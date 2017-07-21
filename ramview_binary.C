//
// View a region of a RAM file.
//
// Author: Jose Javier Gonzalez Ortiz, 5/7/2017
//

#include <fstream>
#include <iostream>
#include <string>

#include <TEventList.h>
#include <TStopwatch.h>

#include "ramrecord.h"

void ramview_binary(const char *file, const char *query)
{

   TStopwatch stopwatch;
   stopwatch.Start();

   // Open the file and load tree and reader
   auto f = TFile::Open(file);
   auto t = (TTree *)f->Get("RAM");
   RAMRecord *r = 0;

   t->SetBranchAddress("RAMRecord.", &r);

   // Parse queried region string
   string region = query;
   int chrDelimiterPos = region.find(":");
   TString rname = region.substr(0, chrDelimiterPos);

   int rangeDelimiterPos = region.find("-");

   UInt_t rangeStart = stoi(region.substr(chrDelimiterPos + 1, rangeDelimiterPos - chrDelimiterPos));
   UInt_t rangeEnd = stoi(region.substr(rangeDelimiterPos + 1, region.size() - rangeDelimiterPos));

   
   // Parse index .root.idx file
   string indexfilename = file;
   indexfilename = indexfilename + ".idx";
   ifstream indexfile(indexfilename);

   string buffer;
   int begin, beginPOS, end, endPOS;
   while(indexfile.good())
   {
      getline(indexfile, buffer, ',');

      if(!rname.EqualTo(buffer)){
         getline(indexfile, buffer, '\n');
         continue;
      }
      getline(indexfile, buffer, ',');
      begin = stoi(buffer);
      getline(indexfile, buffer, ',');
      beginPOS = stoi(buffer);
      getline(indexfile, buffer, ',');
      end = stoi(buffer);
      getline(indexfile, buffer, '\n');
      endPOS = stoi(buffer);

      // cout << beginPOS << " " << endPOS << endl;
   }

   int first_row = -1;

   t->SetBranchStatus("RAMRecord.*", 0);
   t->SetBranchStatus("RAMRecord.v_pos", 1);

   if(rangeStart < beginPOS){
      first_row = begin;
   }
   else if(rangeStart > endPOS){
      first_row = end;
   }
   else{
      int top = begin;
      int bottom = end;
      int topPOS = beginPOS;
      int bottomPOS = endPOS;
      int middle;

      while(first_row < 0){
         middle = (top+bottom)/2;
         t->GetEntry(middle);
         int middlePOS = r->GetPOS();
         // cout << top << "\t" << middle << "\t" << bottom << "\t" << " || ";
         // cout << topPOS << "\t" << middlePOS << "\t" << bottomPOS << "\t" << endl;
         if(middlePOS < rangeStart){
            if(top == middle){
               first_row = bottom;
            }
            top = middle;
            topPOS = middlePOS;
         }
         else if(middlePOS > rangeStart){
            bottom = middle;
            bottomPOS = middlePOS;
         }
         else{
            first_row = middle;
         }
      }

   }

   t->SetBranchStatus("RAMRecord.v_lseq", 1);
   t->GetEntry(first_row);
   
   // cout << r->GetPOS() << '\t' << r->GetPOS()+r->GetSEQLEN() << '\t' << rangeStart << endl;
   
   while(r->GetPOS() + r->GetSEQLEN() >= rangeStart){
      first_row--;
      t->GetEntry(first_row);
      if(first_row < begin){
         break;
      }
      // cout << r->GetPOS() << '\t' << r->GetPOS()+r->GetSEQLEN() << '\t' << rangeStart << endl;
   }
   first_row++;
   // cout << first_row << endl;
   
   t->SetBranchStatus("RAMRecord.*", 1);
   
   for(int i=first_row; i < end; i++){
      t->GetEntry(i);
      if(r->GetPOS() > rangeEnd){
         break;
      }
      r->Print();
   }
   
   stopwatch.Print();
}
