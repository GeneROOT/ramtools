//
// View a region of a RAM file.
//
// Author: Fons Rademakers, 7/12/2017
//

#include <iostream>
#include <cstring>

#include <TBranch.h>
#include <TTree.h>
#include <TFile.h>
#include <TStopwatch.h>
#include <TString.h>
#include <TTreeIndex.h>
#include <TTreePerfStats.h>

#include "utils.h"

#include "ramrecord.C"


void ramview(const char *file, const char *query, bool cache = true, bool perfstats = false,
             const char *perfstatsfilename = "perf.root")
{
   TStopwatch stopwatch;
   stopwatch.Start();

   // Open the file and load tree and reader
   auto f = TFile::Open(file);
   if (!f) {
      printf("ramview: failed to open file %s\n", file);
      return;
   }
   auto t = RAMRecord::GetTree(f);

   RAMRecord *r = 0;

   if (!cache)
      t->SetCacheSize(0);

   TTreePerfStats *ps = 0;
   if (perfstats)
      ps = new TTreePerfStats("ioperf", t);

   t->SetBranchAddress("RAMRecord.", &r);

   TBranch *b = t->GetBranch("RAMRecord.");

   // Parse queried region string (rname:pos1-pos2): chr1:5000-6000
   std::string region = query;
   int chrDelimiterPos = region.find(":");
   TString rname = region.substr(0, chrDelimiterPos);

   int rangeDelimiterPos = region.find("-");

   Int_t range_start = std::stoi(region.substr(chrDelimiterPos + 1, rangeDelimiterPos - chrDelimiterPos));
   Int_t range_end = std::stoi(region.substr(rangeDelimiterPos + 1, region.size() - rangeDelimiterPos));

   // Convert rname to refid
   auto refid = RAMRecord::GetRnameRefs()->GetRefId(rname);

   // Find starting row in index
   auto start_entry = RAMRecord::GetIndex()->GetRow(refid, range_start);
   auto end_entry   = RAMRecord::GetIndex()->GetRow(refid, range_end);

   printf("ramview: %s:%d (%lld) - %d (%lld)\n", rname.Data(), range_start, start_entry,
                                                 range_end, end_entry);

   if (b->GetSplitLevel() > 0)
      t->SetBranchStatus("RAMRecord.*", 0);

   if (b->GetSplitLevel() > 0) {
      t->SetBranchStatus("RAMRecord.v_refid", 1);
      t->SetBranchStatus("RAMRecord.v_pos", 1);
      t->SetBranchStatus("RAMRecord.v_lseq", 1);
   }

   for (; start_entry < end_entry; start_entry++) {
      t->GetEntry(start_entry);
      if (r->GetPOS() + r->GetSEQLEN() > range_start) {
         // First valid position for printing
         break;
      }
   }

   if (b->GetSplitLevel() > 0)
      t->SetBranchStatus("RAMRecord.*", 1);

   Long64_t j;
   for (j = start_entry; j < end_entry; j++) {
      t->GetEntry(j);
//      r->Print();
//      Add optimized printing here. Like in samview where they print
//      using sprintf and write per 4096 bytes to stdout.
   }

   t->GetEntry(j);
   while (r->GetPOS() < range_end) {
//      r->Print();
//      Add optimized printing here. Like in samview where they print
//      using sprintf and write per 4096 bytes to stdout.
      j++;
      t->GetEntry(j);
   }

   stopwatch.Print();

   if (perfstats) {
      ps->SaveAs(perfstatsfilename);
      delete ps;
   }
}
