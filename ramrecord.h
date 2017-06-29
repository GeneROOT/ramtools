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

   void SetQNAME(const TString &qname) { v_qname = qname; }
   void SetFLAG(UShort_t f) { v_flag = f; }
   void SetRNAME(const TString &rname) { v_rname = rname; }
   void SetPOS(UInt_t pos) { v_pos = pos; }
   void SetMAPQ(UChar_t mapq) { v_mapq = mapq; }
   void SetCIGAR(const TString &cigar) { v_cigar = cigar; }
   void SetRNEXT(const TString &rnext) { v_rnext = rnext; }
   void SetPNEXT(UInt_t pnext) { v_pnext = pnext; }
   void SetTLEN(Int_t tlen) { v_tlen = tlen; }
   void SetSEQ(const TString &seq);
   void SetQUAL(const TString &qual) { v_qual = qual; }
   void SetOPT(const TString &opt, Int_t idx) { v_opt[idx] = opt; }

   const TString &GetQNAME() const { return v_qname; }
   UInt_t         GetFLAG() const { return v_flag; }
   const TString &GetRNAME() const { return v_rname; }
   UInt_t         GetPOS() const { return v_pos; }
   UInt_t         GetMAPQ() const { return v_mapq; }
   const TString &GetCIGAR() const { return v_cigar; }
   const TString &GetRNEXT() const { return v_rnext; }
   UInt_t         GetPNEXT() const { return v_pnext; }
   Int_t          GetTLEN() const { return v_tlen; }
   const TString &GetSEQ() const;
   const TString &GetQUAL() const { return v_qual; }
   const TString &GetOPT(Int_t idx) const { return v_opt[idx]; }

   ClassDef(RAMRecord,1)
};

void RAMRecord::SetSEQ(const TString &seq)
{
   // Use BAM like encoding for the segment SEQuence. This uses about half
   // the space compared to an ASCII string as the allowed character set is limited
   // (fits in 4 instead of 8 bits).

   static TString seqcode = "=ACMGRSVTWYHKDBN";
   static bool mapfilled = false;
   static map<char,UChar_t> seqmap;
   if (!mapfilled) {
      for (int i = 0; i < seqcode.Length(); i++)
         seqmap[seqcode[i]] = i;
      mapfilled = true;
   }

   static Int_t maxlseq2 = 0;
   v_lseq = seq.Length();
   Int_t newlseq2 = (v_lseq + 1)/2;

   if (newlseq2 > maxlseq2) {
      delete [] v_seq;
      v_seq = nullptr;
   }

   if (v_seq == nullptr && newlseq2) {
      v_lseq2 = newlseq2;
      maxlseq2 = v_lseq2;
      v_seq = new UChar_t[v_lseq2];
   } else {
      v_lseq2 = newlseq2;
   }

   for (int i = 0; i < v_lseq2; i++) {
      char c = seq[2*i];
      if (!seqcode.Contains(c))
         c = 'N';
      UChar_t nib1 = seqmap[c];
      UChar_t nib2 = 0;
      if (2*i+1 < v_lseq) {
         c = seq[2*i+1];
         if (!seqcode.Contains(c))
            c = 'N';
         nib2 = seqmap[c];
      }
      v_seq[i] = nib1 << 4 | nib2;
   }
}

const TString &RAMRecord::GetSEQ() const
{
   // Decode segment SEQuence from BAM like encoded format. See SetSEQ().

   static TString seqcode = "=ACMGRSVTWYHKDBN";
   static bool mapfilled = false;
   static map<UChar_t,char> seqmap;
   if (!mapfilled) {
      for (int i = 0; i < seqcode.Length(); i++)
         seqmap[i] = seqcode[i];
      mapfilled = true;
   }

   static TString seq;
   seq = "";
   for (int i = 0; i < v_lseq2; i++) {
      UChar_t nib = v_seq[i] >> 4;
      seq += seqmap[nib];
      if (2*i+1 < v_lseq) {
         nib = v_seq[i] & 0xf;
         seq += seqmap[nib];
      }
   }

   return seq;
}
