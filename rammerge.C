//
// Merge two or more RAM files into another
//
// Author: Jose Javier Gonzalez Ortiz, 31/7/2017
//

#include <TList.h>
#include <TFile.h>
#include <TTree.h>

#include "ramrecord.h"


void rammerge(const char* outfile, const char* infile1, const char* infile2){
    
    TList *list = new TList;

    auto *in1 = TFile::Open(infile1);
    auto t1 = (TTree *)in1->Get("RAM");
    list->Add(t1);

    auto *in2 = TFile::Open(infile2);
    auto t2 = (TTree *)in2->Get("RAM");
    list->Add(t2);
   
    TFile *out = new TFile(outfile,"RECREATE");
    TTree *newtree = TTree::MergeTrees(list);
    newtree->SetName("RAM");
    newtree->Write();

    out->Close();
    in1->Close();
    in2->Close();

    delete list;

}