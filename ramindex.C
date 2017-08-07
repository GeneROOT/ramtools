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

void ramindex(const char *file, const char *index_file, bool clone = true){
    auto f = TFile::Open(file);
    auto t = (TTree *)f->Get("RAM");

    t->BuildIndex("v_rname", "v_pos");

    if(clone){
        auto index_f = TFile::Open(index_file, "RECREATE");
        index_f->SetCompressionLevel(f->GetCompressionLevel());
        index_f->SetCompressionAlgorithm(f->GetCompressionAlgorithm());
        TTree *indexed_t = t->CloneTree();
        indexed_t->Print();
        index_f->Write();
    }
    else{
        auto i  = t->GetTreeIndex();
        if(!i){
            std::out << "Incorrect Index" << std::endl;
            exit(1);
        }
    }


}
