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

   int coln, nlines, nrecords, i;
   TString line;
   TObjArray *toks;
   TObjString *s;

   // open ROOT file
   auto f = TFile::Open(treefile, "RECREATE");
   f->SetCompressionLevel(1);     // 0 - no compression, 1..9 - min to max compression
   f->SetCompressionAlgorithm(ROOT::kLZMA);  // ROOT::kZLIB, ROOT::kLZMA, ROOT::kLZ4

   // create the TTree
   auto tree = new TTree("SAM", datafile);

   // create a branch for a RAMRecord
   // don't stream TObject info, but still nice to have a TObject derived class
   auto *r = new RAMRecord;
   RAMRecord::Class()->IgnoreTObjectStreamer();
   tree->Branch("RAMRecord", &r, 64000, 1);

   nlines = 0;
   nrecords = 0;

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
         r->SetFLAG(s->String().Atoi());

         // rname
         s = (TObjString *) toks->At(2);
         r->SetRNAME(s->String());

         // pos
         s = (TObjString *) toks->At(3);
         r->SetPOS(s->String().Atoi());

         // mapq
         s = (TObjString *) toks->At(4);
         r->SetMAPQ(s->String().Atoi());

         // cigar
         s = (TObjString *) toks->At(5);
         r->SetCIGAR(s->String());

         // rnext
         s = (TObjString *) toks->At(6);
         r->SetRNEXT(s->String());

         // pnext
         s = (TObjString *) toks->At(7);
         r->SetPNEXT(s->String().Atoi());

         // tlen
         s = (TObjString *) toks->At(8);
         r->SetTLEN(s->String().Atoi());

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
         nrecords++;
      }
      nlines++;
      delete toks;
   }
   tree->Print();
   tree->Write();

   fclose(fp);
   delete f;

   printf("\nProcessed %d SAM records\n", nrecords);
}
