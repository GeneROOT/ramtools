//
// Read a RAM file.
//
// Author: Fons Rademakers, 29/6/2017
//

#include <TFile.h>
#include <TTree.h>

#include "ramrecord.h"

void ramreader(const char *file = "ramexample.root")
{
   auto f = TFile::Open(file);
   auto t = (TTree*) f->Get("RAM");

   RAMRecord *r = new RAMRecord;

   t->SetBranchAddress("RAMRecord", &r);

   printf("The file contains %lld RAMRecords\n\n", t->GetEntries());

   // access sequentially first 10 records
   printf("Sequentially access the first 10 records from the file:\n");
   for (int i = 0; i < 10; i++) {
      t->GetEvent(i);
      printf("%2d QNAME: %s\n", i, r->GetQNAME());
      printf("%2d SEQ:   %s\n", i, r->GetSEQ());
      printf("%2d QUAL:  %s\n", i, r->GetQUAL());
   }

   printf("\nFull print of last RAMRecord:\n");
   r->Print();

   // Randomly access 10 records
   printf("\nRandomly access 10 records from the file:\n");
   for (int i = 0; i < 10; i++) {
      int n = gRandom->Rndm()*1000.;
      t->GetEvent(n);
      printf("%2d SEQ:  %s\n", n, r->GetSEQ());
      printf("%2d QUAL: %s\n", n, r->GetQUAL());
   }

   RAMRecord r2 = *r;
   printf("\nFull print of copied last RAMRecord:\n");
   r2.Print();
}
