//
// View a region of a RAM file.
//
// Author: Jose Javier Gonzalez Ortiz, 5/7/2017
//

#include <iostream>
#include <string>

#include <TBranch.h>
#include <TTree.h>
#include <TFile.h>
#include <TStopwatch.h>
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


   int hashed_rname = djb2_hash(rname);

   if(t->GetEntryWithIndex(rname) < 0){
        std::cout << "RNAME " << rname << " not contained in file" << std::endl;
        exit(1);
   }

   Long64_t start_entry =  t->GetEntryNumberWithBestIndex(hashed_rname, range_start);
   Long64_t end_entry   =  t->GetEntryNumberWithBestIndex(hashed_rname, range_end);
   t->GetEntry(start_entry);


   // Assume RNAME are chunked together
   // We look only at the RNAME column
   // We can only do this when there are columns
   if(b->GetSplitLevel() > 0){
       t->SetBranchStatus("RAMRecord.*", 0);
       t->SetBranchStatus("RAMRecord.v_rname", 1);
       t->SetBranchStatus("RAMRecord.v_pos", 1);
       t->SetBranchStatus("RAMRecord.v_lseq", 1);
   }

   int posStart = -1;

    for (int i = start_entry; i < t->GetEntries(); i++) {
      t->GetEntry(i);

      // If the RNAME region ends
      if (!rname.EqualTo(r->GetRNAME())) {
         break;
      } else {
         if (r->GetPOS() + r->GetSEQLEN() > start_entry) {
            // Register first valid position for printing
            posStart = i;
            break;
         }
      }
   }

   // If the position was found
   if (posStart >= 0) {

      // Enable all fields for printing
      if(b->GetSplitLevel() > 0){
         t->SetBranchStatus("RAMRecord.*", 1);
      }
      for (int i = posStart; i < t->GetEntries(); i++) {
         t->GetEntry(i);

         // If the RNAME region ends
         if (!rname.EqualTo(r->GetRNAME())) {
            break;
         } else {
            // Within the region
            if (r->GetPOS() <= rangeEnd) {
               r->Print();
            } else {
               break;
            }
         }
      }
   }

   stopwatch.Print();

   if(perfstats){
      ps->SaveAs(perfstatsfilename);
      delete ps;
   }

  
}
