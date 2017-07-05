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



void makeram(const char *datafile = "samexample.sam", const char *treefile = "ramexample.root")
{
   FILE *fp = fopen(datafile, "r");
   if (!fp) {
      printf("file %s not found\n", datafile);
      return;
   }

   // open ROOT file
   auto f = TFile::Open(treefile, "RECREATE");
   f->SetCompressionLevel(1);     // 0 - no compression, 1..9 - min to max compression
   f->SetCompressionAlgorithm(ROOT::kLZMA);  // ROOT::kZLIB, ROOT::kLZMA, ROOT::kLZ4

   // create the TTree
   auto tree = new TTree("RAM", datafile);

   // create a branch for a RAMRecord
   // don't stream TObject info, but still nice to have a TObject derived class
   auto *r = new RAMRecord;
   RAMRecord::Class()->IgnoreTObjectStreamer();
   tree->Branch("RAMRecord", &r, 64000, 1);

   int nlines = 0;
   int nrecords = 0;

   const int maxl = 10240;
   char line[maxl];
   while (fgets(line, maxl, fp)) {

      bool header = false;
      int ntok = 0;
      char *tok;
      while ((tok = strtok(ntok ? 0 : line, "\t"))) {

         if ((ntok == 0 && tok[0] == '@') || header) {
            // skip header lines for the time being

            // Create header objects
            header = true;
            break;

         } else {

            // qname
            if (ntok == 0)
               r->SetQNAME(tok);

            // flag
            if (ntok == 1)
               r->SetFLAG(atoi(tok));

            // rname
            if (ntok == 2)
               r->SetRNAME(tok);

            // pos
            if (ntok == 3)
               r->SetPOS(atoi(tok));

            // mapq
            if (ntok == 4)
               r->SetMAPQ(atoi(tok));

            // cigar
            if (ntok == 5)
               r->SetCIGAR(tok);

            // rnext
            if (ntok == 6)
               r->SetRNEXT(tok);

            // pnext
            if (ntok == 7)
               r->SetPNEXT(atoi(tok));

            // tlen
            if (ntok == 8)
               r->SetTLEN(atoi(tok));

            // seq
            if (ntok == 9)
               r->SetSEQ(tok);

            // qual
            if (ntok == 10) {
               r->SetQUAL(tok);
               for (int i = 0; i < RAMRecord::nopt; i++)
                  r->SetOPT("", i);
            }

            // opt's
            if (ntok >= 11) {
               if (ntok-11 < RAMRecord::nopt)
                  r->SetOPT(tok, ntok-11);
            }
         }
         ntok++;
      }

      if (ntok < RAMRecord::mincol && !header) {
         printf("[%d]: error -- %d data values read instead of %d\n",
                nlines, ntok, RAMRecord::mincol);
         // skip record
         continue;
      }
      if (ntok > RAMRecord::maxcol && !header) {
         printf("[%d]: warning -- %d data values read, %d value(s) truncated\n",
                nlines, ntok, ntok - RAMRecord::maxcol);
      }

      if (!header) {
         nrecords++;
         tree->Fill();
      }
      nlines++;
   }
   tree->Print();
   tree->Write();

   fclose(fp);
   delete f;

   printf("\nProcessed %d SAM records\n", nrecords);
}
