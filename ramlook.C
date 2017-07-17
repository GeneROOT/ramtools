//
// View a region of a RAM file.
//
// Author: Jose Javier Gonzalez Ortiz, 5/7/2017
//

#include <iostream>
#include <string>

#include <TEventList.h>
#include <TStopwatch.h>

#include "ramrecord.h"

void ramlook(const char *file)
{

   TStopwatch stopwatch;
   stopwatch.Start();

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

   

   t->GetEvent(0);
   cout << r->GetRNAME() << " " << r->GetPOS() << " ";
   TString current_rname = r->GetRNAME();

   for (int i = 0; i < t->GetEntries(); i++) {
      t->GetEvent(i);
      if (!current_rname.EqualTo(r->GetRNAME())) {
         t->GetEvent(i-1);
         cout << r->GetPOS() << endl;
         t->GetEvent(i);
         cout << r->GetRNAME() << " " << r->GetPOS() << " ";
         current_rname = r->GetRNAME();
      }
   }

   cout << r->GetPOS()-1 << endl;
   

   stopwatch.Print();
}
