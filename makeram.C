//
// SAM to RAM (ROOT Alignment/Map) format converter.
//
// Author: Fons Rademakers, 6/6/2017
//

#include <TString.h>
#include <TTree.h>
#include <TFile.h>
#include <TClass.h>
#include <Compression.h>

#include "ramrecord.h"



void makeram(const char *datafile = "samexample.sam", const char *treefile = "ramexample.root",
             Bool_t repBadData = kTRUE)
{
   FILE *fp = fopen(datafile, "r");
   if (!fp) {
      printf("file %s not found\n", datafile);
      return;
   }

   int coln, nlines, i;
   TString line;
   TObjArray *toks;
   TObjString *s;

   // open ROOT file
   auto f = TFile::Open(treefile, "RECREATE");
   f->SetCompressionLevel(9);     // 0 - no compression, 1..9 - min to max compression
   f->SetCompressionAlgorithm(ROOT::kLZMA);  // ROOT::kZLIB, ROOT::kLZMA, ROOT::kLZ4

   // create the TTree
   auto tree = new TTree("SAM", datafile);

   // create a branch for a RAMRecord
   // don't stream TObject info, but still nice to have a TObject derived class
   auto *r = new RAMRecord;
   RAMRecord::Class()->IgnoreTObjectStreamer();
   tree->Branch("RAMRecord", &r, 64000, 1);

   nlines = 0;

   while (line.Gets(fp)) {

      toks = line.Tokenize("\t");
      coln = toks->GetEntriesFast();

      if (line[0] == '@') {
         // skip header lines for the time being

         // Create header objects

      } else {
         if (coln < RAMRecord::mincol) {
            printf("[%d]: error -- %d data values read instead of %d\n",
                   nlines, coln, RAMRecord::mincol);
            continue;
         }
         if (coln > RAMRecord::maxcol) {
            printf("[%d]: warning -- %d data values read, %d value(s) truncated\n",
                   nlines, coln, coln - RAMRecord::maxcol);
            coln = RAMRecord::maxcol;
         }

         // qname
         s = (TObjString *) toks->At(0);
         r->SetQNAME(s->String());

         // flag
         s = (TObjString *) toks->At(1);
         if (s->String().IsDigit())
            r->SetFLAG(s->String().Atoi());
         else {
            if (repBadData)
               printf("[%d,2]: does not look like integer [%s]\n", nlines, s->GetName());
            r->SetFLAG(0);
         }

         // rname
         s = (TObjString *) toks->At(2);
         r->SetRNAME(s->String());

         // pos
         s = (TObjString *) toks->At(3);
         if (s->String().IsDigit())
            r->SetPOS(s->String().Atoi());
         else {
            if (repBadData)
               printf("[%d,2]: does not look like integer [%s]\n", nlines, s->GetName());
            r->SetPOS(0);
         }

         // mapq
         s = (TObjString *) toks->At(4);
         if (s->String().IsDigit())
            r->SetMAPQ(s->String().Atoi());
         else {
            if (repBadData)
               printf("[%d,2]: does not look like integer [%s]\n", nlines, s->GetName());
            r->SetMAPQ(0);
         }

         // cigar
         s = (TObjString *) toks->At(5);
         r->SetCIGAR(s->String());

         // rnext
         s = (TObjString *) toks->At(6);
         r->SetRNEXT(s->String());

         // pnext
         s = (TObjString *) toks->At(7);
         if (s->String().IsDigit())
            r->SetPNEXT(s->String().Atoi());
         else {
            if (repBadData)
               printf("[%d,2]: does not look like integer [%s]\n", nlines, s->GetName());
            r->SetPNEXT(0);
         }

         // tlen
         s = (TObjString *) toks->At(8);
         if (s->String().IsDigit())
            r->SetTLEN(s->String().Atoi());
         else {
            if (repBadData)
               printf("[%d,2]: does not look like integer [%s]\n", nlines, s->GetName());
            r->SetTLEN(0);
         }

         // seq
         s = (TObjString *) toks->At(9);
         r->SetSEQ(s->String());

         // qual
         s = (TObjString *) toks->At(10);
         r->SetQUAL(s->String());

         // opt's
         for (i = 0; i < RAMRecord::nopt; i++) {
            r->SetOPT("", i);
            if (coln >= RAMRecord::mincol+i+1) {
               s = (TObjString *) toks->At(RAMRecord::mincol+i);
               r->SetOPT(s->String(), i);
            }
         }

         tree->Fill();
      }
      nlines++;
      delete toks;
   }
   tree->Print();
   tree->Write();

   fclose(fp);
   delete f;
}
