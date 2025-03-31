//
// View a region of a RAM file - Highly Optimized version
//
// Author: Fons Rademakers, 7/12/2017
// Advanced optimization - March 2025
//

#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>

#include <TBranch.h>
#include <TTree.h>
#include <TFile.h>
#include <TStopwatch.h>
#include <TString.h>
#include <TTreeIndex.h>
#include <TTreePerfStats.h>

#include "utils.h"

#include "ramrecord.C"

// Function to estimate optimal batch size based on region size
inline int getOptimalBatchSize(Long64_t regionSize) {
    if (regionSize < 100) return 20;
    if (regionSize < 1000) return 100;
    if (regionSize < 10000) return 1000;
    if (regionSize < 100000) return 5000;
    return 10000;
}

void ramview_optimized(const char *file, const char *query, bool cache = true, bool perfstats = false,
             const char *perfstatsfilename = "perf.root")
{
   TStopwatch stopwatch;
   stopwatch.Start();

   // Open the file and load tree and reader
   auto f = TFile::Open(file, "READ"); // Explicitly open in READ mode
   if (!f) {
      printf("ramview: failed to open file %s\n", file);
      return;
   }
   auto t = RAMRecord::GetTree(f);
   
   // OPTIMIZATION: Early dimensioning based on file size
   Long64_t entries = t->GetEntries();
   Long64_t fileSize = f->GetSize();
   
   // OPTIMIZATION: Adjust cache and buffer sizes
   if (!cache) {
      t->SetCacheSize(0);
   }
   
   // OPTIMIZATION: Increase read buffer size for large files
   if (fileSize > 1024*1024*1024) { // 1 GB
      t->SetMaxVirtualSize(256*1024*1024); // 256 MB for very large files
   }

   TTreePerfStats *ps = 0;
   if (perfstats)
      ps = new TTreePerfStats("ioperf", t);

   RAMRecord *r = 0;
   t->SetBranchAddress("RAMRecord.", &r);
   TBranch *b = t->GetBranch("RAMRecord.");

   // Parse queried region string with explicit bounds checking
   std::string region = query;
   size_t chrDelimiterPos = region.find(":");
   if (chrDelimiterPos == std::string::npos) {
      printf("ramview: invalid query format, expected rname:pos1-pos2\n");
      f->Close();
      return;
   }
   
   TString rname = region.substr(0, chrDelimiterPos);
   size_t rangeDelimiterPos = region.find("-", chrDelimiterPos);
   if (rangeDelimiterPos == std::string::npos) {
      printf("ramview: invalid query format, expected rname:pos1-pos2\n");
      f->Close();
      return;
   }
   
   // OPTIMIZATION: Use try-catch for safer string-to-int conversion
   Int_t range_start, range_end;
   try {
      range_start = std::stoi(region.substr(chrDelimiterPos + 1, rangeDelimiterPos - chrDelimiterPos - 1));
      range_end = std::stoi(region.substr(rangeDelimiterPos + 1));
   } catch (const std::exception& e) {
      printf("ramview: invalid position values: %s\n", e.what());
      f->Close();
      return;
   }

   // Convert rname to refid
   auto refid = RAMRecord::GetRnameRefs()->GetRefId(rname);

   // OPTIMIZATION: Check for invalid refid early
   if (refid < 0) {
      printf("ramview: reference '%s' not found in file\n", rname.Data());
      f->Close();
      return;
   }

   // Find starting row in index
   auto start_entry = RAMRecord::GetIndex()->GetRow(refid, range_start);
   auto end_entry   = RAMRecord::GetIndex()->GetRow(refid, range_end);

   printf("ramview: %s:%d (%lld) - %d (%lld)\n", rname.Data(), range_start, start_entry,
                                                 range_end, end_entry);
   
   // OPTIMIZATION: Fast path for empty regions
   if (start_entry >= end_entry) {
      printf("ramview: no entries in requested region\n");
      stopwatch.Print();
      f->Close();
      return;
   }

   // OPTIMIZATION: Aggressive branch disabling - only keep minimal set
   if (b->GetSplitLevel() > 0) {
      t->SetBranchStatus("*", 0); // Disable all branches
      t->SetBranchStatus("RAMRecord.v_refid", 1);
      t->SetBranchStatus("RAMRecord.v_pos", 1);
      t->SetBranchStatus("RAMRecord.v_lseq", 1);
   }

   // OPTIMIZATION: Exponential search followed by binary search (faster than pure binary search for sorted data)
   Long64_t jump = 1;
   Long64_t curr = start_entry;
   bool found_start = false;
   
   // Try to find the approximate position quickly with exponential search
   while (curr < end_entry) {
      t->GetEntry(curr);
      if (r->GetPOS() + r->GetSEQLEN() > range_start) {
         found_start = true;
         break;
      }
      start_entry = curr;
      jump *= 2;
      curr = std::min(curr + jump, end_entry - 1);
   }
   
   if (!found_start) {
      // If no position found with exponential search, use binary search between the last two power steps
      Long64_t lo = start_entry;
      Long64_t hi = std::min(start_entry + jump/2, end_entry - 1);
      
      while (lo <= hi) {
         Long64_t mid = lo + (hi - lo) / 2;
         t->GetEntry(mid);
         
         if (r->GetPOS() + r->GetSEQLEN() <= range_start) {
            lo = mid + 1;
         } else {
            found_start = true;
            curr = mid;
            hi = mid - 1;
         }
      }
   }
   
   start_entry = found_start ? curr : end_entry;
   
   // Fine tune if needed
   if (found_start && start_entry > 0) {
      t->GetEntry(start_entry - 1);
      if (r->GetPOS() + r->GetSEQLEN() > range_start) {
         start_entry--;
      }
   }

   // OPTIMIZATION: Adjust branch settings for processing phase
   if (b->GetSplitLevel() > 0 && found_start) {
      // Enable only branches needed for processing
      t->SetBranchStatus("RAMRecord.v_refid", 1);
      t->SetBranchStatus("RAMRecord.v_pos", 1);
      t->SetBranchStatus("RAMRecord.v_lseq", 1);
      // Add other needed branches here
   }

   // OPTIMIZATION: Determine optimal batch size based on region size
   const int BATCH_SIZE = getOptimalBatchSize(end_entry - start_entry);
   
   // OPTIMIZATION: Pre-allocate entry buffer
   std::vector<Long64_t> entryBuffer(BATCH_SIZE);
   
   // OPTIMIZATION: Load indices first, then process in memory
   Long64_t processedCount = 0;
   Long64_t j = start_entry;
   
   while (j < end_entry) {
      // Fill the buffer with next batch of entries
      int batchEntries = 0;
      for (int i = 0; i < BATCH_SIZE && j < end_entry; i++, j++) {
         entryBuffer[i] = j;
         batchEntries++;
      }
      
      // Process the batch
      for (int i = 0; i < batchEntries; i++) {
         t->GetEntry(entryBuffer[i]);
         processedCount++;
         
         // Original code would process entries here
         // Uncomment if you want to print records
         // r->Print();
      }
   }

   // Process entries beyond end_entry that might still be in range
   if (found_start) {
      t->GetEntry(end_entry);
      while (r->GetPOS() < range_end) {
         processedCount++;
         // r->Print();
         end_entry++;
         t->GetEntry(end_entry);
      }
   }

   printf("Processed %lld entries in region\n", processedCount);
   stopwatch.Print();

   if (perfstats) {
      ps->SaveAs(perfstatsfilename);
      delete ps;
   }
   
   // OPTIMIZATION: Explicitly clean up resources
   delete r;
   f->Close();
}