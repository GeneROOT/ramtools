//
// RAMRecord class describes the SAM data fields in RAM format.
// It copies packing features from the BAM format.
//
// Author: Fons Rademakers, 29/6/2017
//

#include <TObject.h>
#include <TString.h>


class RAMRecord : public TObject {

public:
   static const int mincol = 11, maxcol = 14, nopt = maxcol - mincol;

private:
   TString         v_qname;               // Query template NAME
   UShort_t        v_flag;                // Bitwise FLAG
   TString         v_rname;               // Reference sequence NAME
   UInt_t          v_pos;                 // 1-based left most mapping POSition
   UChar_t         v_mapq;                // MAPing Quality
   TString         v_cigar;               // CIGAR string
   TString         v_rnext;               // Reference name of the mate/next read
   UInt_t          v_pnext;               // Position of the mate/next read
   Int_t           v_tlen;                // Observed Template LENgth
   Int_t           v_lseq;                // Length of segment SEQuence
   Int_t           v_lseq2;               // (Length+1)/2 of segment SEQuence
   UChar_t        *v_seq;                 //[v_lseq2] segment SEQuence
   TString         v_qual;                // ASCII of Phred-scaled base QUALity+33
   TString         v_opt[nopt];           // Optional fields

public:
   RAMRecord() : v_flag(0), v_pos(0), v_mapq(0), v_pnext(0), v_tlen(0), v_lseq(0),
                 v_lseq2(0), v_seq(nullptr) { }
   virtual ~RAMRecord() { delete [] v_seq; v_seq = nullptr; }

   void SetQNAME(const char *qname) { v_qname = qname; }
   void SetFLAG(UShort_t f) { v_flag = f; }
   void SetRNAME(const char *rname) { v_rname = rname; }
   void SetPOS(UInt_t pos) { v_pos = pos; }
   void SetMAPQ(UChar_t mapq) { v_mapq = mapq; }
   void SetCIGAR(const char *cigar) { v_cigar = cigar; }
   void SetRNEXT(const char *rnext) { v_rnext = rnext; }
   void SetPNEXT(UInt_t pnext) { v_pnext = pnext; }
   void SetTLEN(Int_t tlen) { v_tlen = tlen; }
   void SetSEQ(const char *seq);
   void SetQUAL(const char *qual) { v_qual = qual; }
   void SetOPT(const char *opt, Int_t idx) { v_opt[idx] = opt; }

   const char *GetQNAME() const { return v_qname; }
   UInt_t      GetFLAG() const { return v_flag; }
   const char *GetRNAME() const { return v_rname; }
   UInt_t      GetPOS() const { return v_pos; }
   UInt_t      GetMAPQ() const { return v_mapq; }
   const char *GetCIGAR() const { return v_cigar; }
   const char *GetRNEXT() const { return v_rnext; }
   UInt_t      GetPNEXT() const { return v_pnext; }
   Int_t       GetTLEN() const { return v_tlen; }
   const char *GetSEQ() const;
   const char *GetQUAL() const { return v_qual; }
   const char *GetOPT(Int_t idx) const { return v_opt[idx]; }

   ClassDef(RAMRecord,1)
};

void RAMRecord::SetSEQ(const char *seq)
{
   // Use BAM like encoding for the segment SEQuence. This uses about half
   // the space compared to an ASCII string as the allowed character set is limited
   // (fits in 4 instead of 8 bits).

   static const char *codetoseq = "=ACMGRSVTWYHKDBN";
   static UChar_t seqtocode[256];
   static bool init = false;
   if (!init) {
      memset(seqtocode, 0, 256);
      for (int i = 1; i < 16; i++) {
         seqtocode[codetoseq[i]] = i;
      }
      init = true;
   }

   static Int_t maxlseq2 = 0;
   v_lseq = strlen(seq);
   v_lseq2 = (v_lseq + 1)/2;

   if (v_lseq2 > maxlseq2) {
      delete [] v_seq;
      v_seq = nullptr;
   }

   if (v_seq == nullptr && v_lseq2) {
      maxlseq2 = v_lseq2;
      v_seq = new UChar_t[v_lseq2];
   }

   int j = 0;
   for (int i = 0; i + 1 < v_lseq; i += 2) {
      v_seq[j] = (seqtocode[seq[i]] << 4) | seqtocode[seq[i+1]];
      j++;
   }
   if (v_lseq % 2) {
      v_seq[j] = seqtocode[seq[v_lseq-1]] << 4;
   }
}

const char *RAMRecord::GetSEQ() const
{
   // Decode segment SEQuence from BAM like encoded format. See SetSEQ().

   static const char *codetoseq = "=ACMGRSVTWYHKDBN";
   static UShort_t codetoseqpair[256];
   static bool init = false;
   if (!init) {
      for (int i = 0; i < 256; i++) {
         codetoseqpair[i] = (codetoseq[i >> 4]) | (codetoseq[i & 0xf] << 8);
      }
      init = true;
   }

   static int maxlseq = 0;
   static char *seq = nullptr;

   if (v_lseq > maxlseq) {
      delete [] seq;
      seq = nullptr;
   }

   if (seq == nullptr && v_lseq) {
      maxlseq = v_lseq;
      seq = new char[v_lseq+1];
      seq[v_lseq] = '\0';
   }

   UShort_t *seqpairs = (UShort_t*) seq;
   int pairs = v_lseq / 2;
   for (int i = 0; i < pairs; i++) {
      seqpairs[i] = codetoseqpair[v_seq[i]];
   }
   if (v_lseq % 2) {
      seq[v_lseq-1] = codetoseq[v_seq[v_lseq / 2] >> 4];
   }

   return seq;
}
