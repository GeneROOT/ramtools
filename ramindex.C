//
// Build an index for a RAM file
//
// Author: Jose Javier Gonzalez Ortiz, 17/7/2017
//

#include <fstream>
#include <iostream>
#include <string>

#include <TEventList.h>
#include <TStopwatch.h>

#include "ramrecord.h"

void ramindex(const char *file)
{
   TStopwatch stopwatch;
   stopwatch.Start();

   // Create index file
   ofstream indexfile;
   std::string indexfilename = file;
   indexfilename += ".idx";
   indexfile.open(indexfilename);

   // Open the file and load tree and reader
   auto f = TFile::Open(file);
   auto t = (TTree *)f->Get("RAM");
   RAMRecord *r = 0;

   t->SetBranchAddress("RAMRecord.", &r);

   // Assume RNAME are chunked together
   // We look only at the RNAME column
   t->SetBranchStatus("RAMRecord.*", 0);
   t->SetBranchStatus("RAMRecord.v_rname", 1);
   t->SetBranchStatus("RAMRecord.v_pos", 1);
   
   int i = 0;
   t->GetEvent(0);
   indexfile << "RNAME,begin,beginPOS,end,endPOS" << endl;
   
   indexfile << r->GetRNAME() << ",";
   indexfile << i << ",";
   indexfile << r->GetPOS() << ",";
   
   TString current_rname = r->GetRNAME();

   t->SetBranchStatus("RAMRecord.*", 0);
   t->SetBranchStatus("RAMRecord.v_rname", 1);

   for (i = 0; i < t->GetEntries(); i++) {
      t->GetEvent(i);
      if (!current_rname.EqualTo(r->GetRNAME())) {
         t->SetBranchStatus("RAMRecord.v_pos", 1);
      
         t->GetEvent(i-1);
         indexfile << i-1 << ",";
         indexfile << r->GetPOS() << endl;

         t->GetEvent(i);
         indexfile << r->GetRNAME() << ",";
         indexfile << i << ",";
         indexfile << r->GetPOS() << ",";
         current_rname = r->GetRNAME();
         
         t->SetBranchStatus("RAMRecord.v_pos", 0);

      }
   }

   t->SetBranchStatus("RAMRecord.v_pos", 1);
   t->GetEvent(i-1);
   indexfile << i-1 << ",";
   indexfile << r->GetPOS() << endl;
   
   indexfile.close();

   stopwatch.Print();

}
