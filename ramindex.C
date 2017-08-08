//
// Index for a RAM and save it to same file or external one
//
// Author: Jose Javier Gonzalez Ortiz, 7/8/2017
//

#include <iostream>
#include <cstring>

#include <TTree.h>
#include <TTreeIndex.h>
#include <TFile.h>
#include <TString.h>

#include "ramrecord.h"

void ramindex(const char *file, bool update = true, std::string indexfile = ""){
    auto f = TFile::Open(file, "UPDATE");
    auto t = (TTree *)f->Get("RAM");

    t->BuildIndex("v_rnamehash", "v_pos");

    if(update){
        t->Write();
    }
    else{
        auto i  = t->GetTreeIndex();
        if( indexfile.empty() ){
            indexfile = file;
            indexfile.append(".rai");
        }

        auto index_f = TFile::Open(indexfile.c_str(), "RECREATE");
        i->Write("INDEX");
        std:cout << "Saving in separate indexfile " << indexfile << std::endl;
        std::cout << "Size: " << f->GetSize() << std::endl;
        delete index_f;
    }

    delete f;
    std::cout << "Index generated for " << file << std::endl;
}
