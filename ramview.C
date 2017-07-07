#include <string>
#include <iostream>

#include <TEventList.h>
#include <TStopwatch.h>

#include "ramrecord.h"

void printRow(RAMRecord* r);

void ramview(const char* file, const char *query){
    // String Parsing forma
    
    auto f = TFile::Open(file);
    auto t = (TTree*) f->Get("RAM");
    RAMRecord *r = 0;

    t->SetBranchAddress("RAMRecord.", &r);


    std::string region = query;
    int chrDelimiterPos = region.find(":");
    TString rname =  region.substr(0, chrDelimiterPos);

    int rangeDelimiterPos = region.find("-");
    
    UInt_t rangeStart =  std::stoi( region.substr(chrDelimiterPos+1, rangeDelimiterPos - chrDelimiterPos) );
    UInt_t rangeEnd   =  std::stoi( region.substr(rangeDelimiterPos+1, region.size() - rangeDelimiterPos ) );


    int rnameStart = -1;
    int posStart   = -1;

    t->SetBranchStatus("RAMRecord.*", 0);
    t->SetBranchStatus("RAMRecord.v_rname", 1);

    // Search until rname is found
    for(int i=0 ; i < t->GetEntries() ; i++ ){
        t->GetEvent(i);
        if(rname.EqualTo(r->GetRNAME())){
            rnameStart = i;
            break;
        }
    }


    // Search until rname ends
    if(rnameStart >= 0){

        t->SetBranchStatus("RAMRecord.v_pos", 1);
        t->SetBranchStatus("RAMRecord.v_lseq", 1);

        for(int i=rnameStart ; i < t->GetEntries() ; i++ ){
            t->GetEvent(i);
            if(!rname.EqualTo(r->GetRNAME())){
                break;
            }
            else{
                if(r->GetPOS() + r->GetSEQLEN() > rangeStart){
                    posStart = i;
                    break;
                }
            }
        }

        t->SetBranchStatus("RAMRecord.*", 1);

        if(posStart >= 0){
            for(int i=posStart ; i < t->GetEntries() ; i++ ){
                t->GetEvent(i);
                if(!rname.EqualTo(r->GetRNAME())){
                    break;
                }
                else{
                    if(r->GetPOS() < rangeEnd){
                        printRow(r);
                    }
                    else{
                        break;
                    }
                }
            }
        }
    }
}


void printRow(RAMRecord* r){
    cout << r->GetQNAME()   << "\t";
    cout << r->GetFLAG()    << "\t";
    cout << r->GetRNAME()   << "\t";
    cout << r->GetPOS()     << "\t";
    cout << r->GetMAPQ()    << "\t";
    cout << r->GetCIGAR()   << "\t";
    cout << r->GetRNEXT()   << "\t";
    cout << r->GetPNEXT()   << "\t";
    cout << r->GetTLEN()    << "\t";
    cout << r->GetSEQ()     << "\t";
    cout << r->GetQUAL()    << "\t";
    for (int i = 0; i < RAMRecord::nopt; i++)
        cout << r->GetOPT(i) << "\t";
    cout << endl;
}