//
// View a region of a RAM file.
//
// Author: Jose Javier Gonzalez Ortiz, 5/7/2017
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

#include "ramrecord.h"
#include "utils.h"

void ramview_index(const char *file, const char *query, bool perfstats=false, const char* perfstatsfilename="perf.root")
{

    TStopwatch stopwatch;
    stopwatch.Start();

    // Open the file and load tree and reader
    auto f = TFile::Open(file);
    auto t = (TTree *)f->Get("RAM");
    RAMRecord *r = 0;

    // Check for TTreeIndex in TTree and in external file (in that order)

    auto i = t->GetTreeIndex();
    if(!i){
        std::string indexfile = file;
        indexfile.append(".rai");
        std::FILE *fp = std::fopen(indexfile.c_str(), "r");
        if (fp) {
            auto f2 = TFile::Open(indexfile.c_str());
            i = (TTreeIndex *)f2->Get("INDEX");
        }

        if(i){
            t->SetTreeIndex(i);
        }
        else{
            std::cout << "Index missing please call ramindex first" << std::endl;
            exit(1);
        }
    }


    TTreePerfStats *ps = 0;

    if(perfstats){
       ps = new TTreePerfStats("ioperf", t);
    }


    t->SetBranchAddress("RAMRecord.", &r);

    TBranch *b = t->GetBranch("RAMRecord.");

    // Parse queried region string
    std::string region = query;
    int chrDelimiterPos = region.find(":");
    TString rname = region.substr(0, chrDelimiterPos);

    int rangeDelimiterPos = region.find("-");

    UInt_t range_start = std::stoi(region.substr(chrDelimiterPos + 1, rangeDelimiterPos - chrDelimiterPos));
    UInt_t range_end   = std::stoi(region.substr(rangeDelimiterPos + 1, region.size() - rangeDelimiterPos));

    // Look into the TTree Index
    UInt_t hashed_rname = TString::Hash(rname, std::strlen(rname));
    Long64_t start_entry =  t->GetEntryNumberWithBestIndex(hashed_rname, range_start);
    Long64_t end_entry   =  t->GetEntryNumberWithBestIndex(hashed_rname, range_end);

    if(end_entry == -1){
        std::cout << "Invalid Region " << region << std::endl;
        exit(1);
    }

    if(b->GetSplitLevel() > 0){
        t->SetBranchStatus("RAMRecord.*", 0);
    }

    UInt_t start_pos = range_start;
    while(start_entry == -1){
        start_entry = t->GetEntry(start_pos++);
    }

    if(b->GetSplitLevel() > 0){
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

    if(b->GetSplitLevel() > 0){
       t->SetBranchStatus("RAMRecord.*", 1);
    }

    Long64_t j;
    for ( j = start_entry ; j <= end_entry; j++) {
        t->GetEntry(j);
        r->Print();
    }

    t->GetEntry(j++);
    while(r->GetPOS() <= range_end){
        t->GetEntry(j++);
        r->Print();
    }

    stopwatch.Print();

    if(perfstats){
       ps->SaveAs(perfstatsfilename);
       delete ps;
    }

  
}
