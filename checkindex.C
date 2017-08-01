#include <iostream>


#include <TTree.h>
#include <TTreeIndex.h>
#include <TFile.h>
#include <TString.h>
#include <cstring>

#include "ramrecord.h"
#include "utils.h"

void checkindex(const char *file="samexample.root"){
    auto f = TFile::Open(file);
    auto t = (TTree *)f->Get("RAM");
    RAMRecord *r = 0;
    // t->BuildIndex("v_rname", "v_pos");
    t->SetBranchAddress("RAMRecord.", &r);

    // auto i = TTreeIndex(t, "v_rname", "v_pos");

    t->Print();

    auto i  = t->GetTreeIndex();

    std::cout << i->GetMajorName() << std::endl;
    std::cout << i->GetMinorName() << std::endl;
    // i->Print();

    // std::cout << t->GetEntryNumberWithBestIndex(TString::Hash("chr1", std::strlen("chr1")), 10152) << std::endl;

}
