//
// FASTQ to RAM (ROOT Alignment/Map) format converter.
//
// Author: Jose Javier, 31/7/2017
//

#include <TString.h>
#include <TTree.h>
#include <TFile.h>
#include <TClass.h>
#include <TStopwatch.h>
#include <Compression.h>

#include "ramrecord.h"

void stripcrlf(char *tok)
{
   int l = strlen(tok);
   if (l > 0 && tok[l-1] == '\n') {
      if (l > 1 && tok[l-2] == '\r')
         tok[l-2] = '\0';
      else
         tok[l-1] = '\0';
   }
}

void fqtoram(const char *datafile = "fqexample.fq", const char *treefile = "ramexample.root")
{
   // Convert a FASTQ file into a RAM file.

   // open the FASTQ file
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
   tree->Branch("RAMRecord.", &r, 64000, 99);

   int nrecords = 0;

   const int maxl = 10240;
   char line[maxl];
   while (fgets(line, maxl, fp)) {
      
      // Starts with unneded @
      stripcrlf(line+1);
      r->SetQNAME(line+1);

      // Store the SEQ
      fgets(line, maxl, fp);
      r->SetSEQ(line);

      // Ignore + field
      fgets(line, maxl, fp);

      //
      fgets(line, maxl, fp);
      r->SetQUAL(line);

      nrecords++;
      tree->Fill();

   }
   tree->Print();
   tree->Write();

   fclose(fp);
   delete f;

   printf("\nProcessed %d FASTQ records\n", nrecords);

}
