//
// Index for a RAM and save it to another file
//
// Author: Jose Javier Gonzalez Ortiz, 7/8/2017
//

#include <iostream>

#include <TTree.h>
#include <TTreeIndex.h>
#include <TFile.h>
#include <TString.h>

#include "ramrecord.h"

void ramclonewithindex(const char *file, const char *indexed_file){
    auto f = TFile::Open(file);
    auto t = (TTree *)f->Get("RAM");

    t->BuildIndex("v_rnamehash", "v_pos");

    auto indexed_f = TFile::Open(indexed_file, "RECREATE");
    indexed_f->SetCompressionLevel(f->GetCompressionLevel());
    indexed_f->SetCompressionAlgorithm(f->GetCompressionAlgorithm());
    TTree *indexed_t = t->CloneTree();
    indexed_f->Write();

    delete f;
    delete indexed_f;

}
