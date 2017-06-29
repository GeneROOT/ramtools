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
   auto t = (TTree*) f->Get("SAM");

   RAMRecord *r = new RAMRecord;

   t->SetBranchAddress("RAMRecord", &r);

   for (int i = 0; i < 10; i++) {
      t->GetEvent(i);
      printf("%2d SEQ: %s\n", i, r->GetSEQ().Data());
   }
}


