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
#include <TEventList.h>
#include <TStopwatch.h>

#include "ramrecord.h"

void ramview(const char *file, const char *query)
{

   TStopwatch stopwatch;
   stopwatch.Start();

   // Open the file and load tree and reader
   auto f = TFile::Open(file);
   auto t = (TTree *)f->Get("RAM");
   RAMRecord *r = 0;

   t->SetBranchAddress("RAMRecord.", &r);

   TBranch *b = t->GetBranch("RAMRecord.");

   // Parse queried region string
   std::string region = query;
   int chrDelimiterPos = region.find(":");
   TString rname = region.substr(0, chrDelimiterPos);

   int rangeDelimiterPos = region.find("-");

   UInt_t rangeStart = std::stoi(region.substr(chrDelimiterPos + 1, rangeDelimiterPos - chrDelimiterPos));
   UInt_t rangeEnd = std::stoi(region.substr(rangeDelimiterPos + 1, region.size() - rangeDelimiterPos));

   // Default values to ensure correctness
   int rnameStart = -1;
   int posStart = -1;

   // Assume RNAME are chunked together
   // We look only at the RNAME column
   // We can only do this when there are columns
   if(b->GetSplitLevel() > 0){
       t->SetBranchStatus("RAMRecord.*", 0);
       t->SetBranchStatus("RAMRecord.v_rname", 1);
   }

   for (int i = 0; i < t->GetEntries(); i++) {
      t->GetEntry(i);
      if (rname.EqualTo(r->GetRNAME())) {
         rnameStart = i;
         break;
      }
   }

   // If the RNAME was found
   if (rnameStart >= 0) {

      // We need to look both at the leftmost position (v_pos)
      // as well as the length of sequence (v_lseq)
      if(b->GetSplitLevel() > 0){
         t->SetBranchStatus("RAMRecord.v_pos", 1);
         t->SetBranchStatus("RAMRecord.v_lseq", 1);
      }

      for (int i = rnameStart; i < t->GetEntries(); i++) {
         t->GetEntry(i);

         // If the RNAME region ends
         if (!rname.EqualTo(r->GetRNAME())) {
            break;
         } else {
            if (r->GetPOS() + r->GetSEQLEN() > rangeStart) {
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
   }

   stopwatch.Print();
}
