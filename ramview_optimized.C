#include <iostream>
#include <string>

#include <TBranch.h>
#include <TTree.h>
#include <TFile.h>
#include <TStopwatch.h>
#include <TString.h>
#include <TTreeIndex.h>
#include <TTreePerfStats.h>

#include "utils.h"
#include "ramrecord.C"


void ramview_optimized(const char* file, const char* query, bool cache = true, 
                      bool perfstats = false, const char* perfstatsfilename = "perf.root") {
    TStopwatch stopwatch;
    stopwatch.Start();

    // Open the file
    auto f = TFile::Open(file);
    if (!f) {
        printf("Failed to open file: %s\n", file);
        return;
    }

    // Get the tree
    auto t = RAMRecord::GetTree(f);
    if (!t) {
        printf("Failed to get tree from file\n");
        f->Close();
        return;
    }

    // Configure cache
    if (!cache) {
        t->SetCacheSize(0);
    } else {
        t->SetCacheSize(30000000);  // 30MB cache
    }

    // Setup performance stats
    TTreePerfStats* ps = nullptr;
    if (perfstats) {
        ps = new TTreePerfStats("ioperf", t);
    }

    // Parse the query string
    std::string region = query;
    int chrDelimiterPos = region.find(":");
    TString rname = region.substr(0, chrDelimiterPos);

    int rangeDelimiterPos = region.find("-");
    int range_start = std::stoi(region.substr(chrDelimiterPos + 1, rangeDelimiterPos - chrDelimiterPos));
    int range_end = std::stoi(region.substr(rangeDelimiterPos + 1, region.size() - rangeDelimiterPos));

    // Convert reference name to ID
    auto refid = RAMRecord::GetRnameRefs()->GetRefId(rname);
    
    // Find row range in index
    auto start_entry = RAMRecord::GetIndex()->GetRow(refid, range_start);
    auto end_entry = RAMRecord::GetIndex()->GetRow(refid, range_end);

    printf("ramview: %s:%d (%lld) - %d (%lld)\n", rname.Data(), range_start, start_entry,
                                                range_end, end_entry);

    // Setup record pointer
    RAMRecord* r = nullptr;
    t->SetBranchAddress("RAMRecord.", &r);
    auto b = t->GetBranch("RAMRecord.");

    // Optimize branch loading
    if (b->GetSplitLevel() > 0) {
        t->SetBranchStatus("*", 0);  // Disable all branches
        t->SetBranchStatus("RAMRecord.v_refid", 1);
        t->SetBranchStatus("RAMRecord.v_pos", 1);
        t->SetBranchStatus("RAMRecord.v_lseq", 1);
    }

    // Binary search for starting entry
    Long64_t low = start_entry;
    Long64_t high = end_entry;
    
    while (low < high) {
        Long64_t mid = low + (high - low) / 2;
        t->GetEntry(mid);
        if (r->GetPOS() + r->GetSEQLEN() <= range_start) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    start_entry = low;
    
    // Re-enable all branches for full record access
    if (b->GetSplitLevel() > 0) {
        t->SetBranchStatus("RAMRecord.*", 1);
    }

    // Process entries in batches
    const int batchSize = 500;
    Long64_t j;
    
    for (j = start_entry; j < end_entry; j += batchSize) {
        Long64_t entriesRemaining = end_entry - j;
        Long64_t entriesToProcess = entriesRemaining < batchSize ? entriesRemaining : batchSize;
        
        for (Long64_t k = 0; k < entriesToProcess; k++) {
            t->GetEntry(j + k);
            // r->Print();  // Commented as in original
        }
    }

    // Process entries after end_entry that might still be in range
    t->GetEntry(j);
    while (r->GetPOS() < range_end) {
        // r->Print();  // Commented as in original
        j++;
        t->GetEntry(j);
    }

    // Report performance
    stopwatch.Print();

    // Clean up
    if (perfstats) {
        ps->SaveAs(perfstatsfilename);
        delete ps;
    }
    
    f->Close();
}