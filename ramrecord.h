//
// RAMRecord class describes the SAM data fields in RAM format.
// It copies packing features from the BAM format.
//
// Author: Fons Rademakers, 29/6/2017
//

#ifndef RAMRecord_h
#define RAMRecord_h

#include <TObject.h>
#include <TString.h>
#include <iostream>



class RAMRecord : public TObject {

public:
   static const int mincol = 11, maxcol = 14, nopt = maxcol - mincol;

   enum EQualCompressionBits {
      kPhred33          = BIT(14),   // Default Phred+33 quality score
      kIlluminaBinning  = BIT(15),   // Illumina binning
      kDrop             = BIT(16)    // Drop quality score
   };

private:
   TString         v_qname;          // Query template NAME
   UShort_t        v_flag;           // Bitwise FLAG
   TString         v_rname;          // Reference sequence NAME
   UInt_t          v_pos;            // 1-based left most mapping POSition
   UChar_t         v_mapq;           // MAPing Quality
   Int_t           v_ncigar_op;      // Number of CIGAR operands
   UInt_t         *v_cigar;          //[v_ncigar_op] (op_len<<4|op. "MIDNSHP=X" -> "012345678")
   TString         v_rnext;          // Reference name of the mate/next read
   UInt_t          v_pnext;          // Position of the mate/next read
   Int_t           v_tlen;           // Observed Template LENgth
   Int_t           v_lseq;           // Length of segment SEQuence
   Int_t           v_lseq2;          // (Length+1)/2 of segment SEQuence
   UChar_t        *v_seq;            //[v_lseq2] segment SEQuence
   UChar_t        *v_qual;           //[v_lseq] ASCII of Phred-scaled base QUALity+33
   TString         v_opt[nopt];      // Optional fields

public:
   RAMRecord() : v_flag(0), v_pos(0), v_mapq(0), v_ncigar_op(0), v_cigar(nullptr),
                 v_pnext(0), v_tlen(0), v_lseq(0),
                 v_lseq2(0), v_seq(nullptr), v_qual(nullptr) { }
   RAMRecord(const RAMRecord &rec);
   RAMRecord &operator=(const RAMRecord &rhs);
   virtual ~RAMRecord() { delete [] v_cigar; v_cigar = nullptr;
                          delete [] v_seq;   v_seq   = nullptr;
                          delete [] v_qual;  v_qual  = nullptr; }

   void SetQNAME(const char *qname) { v_qname = qname; }
   void SetFLAG(UShort_t f) { v_flag = f; }
   void SetRNAME(const char *rname) { v_rname = rname; }
   void SetPOS(UInt_t pos) { v_pos = pos; }
   void SetMAPQ(UChar_t mapq) { v_mapq = mapq; }
   void SetCIGAR(const char *cigar);
   void SetRNEXT(const char *rnext) { v_rnext = rnext; }
   void SetPNEXT(UInt_t pnext) { v_pnext = pnext; }
   void SetTLEN(Int_t tlen) { v_tlen = tlen; }
   void SetSEQ(const char *seq);
   void SetQUAL(const char *qual);
   void SetOPT(const char *opt, Int_t idx) { v_opt[idx] = opt; }

   const char *GetQNAME() const { return v_qname; }
   UInt_t      GetFLAG() const { return v_flag; }
   const char *GetRNAME() const { return v_rname; }
   UInt_t      GetPOS() const { return v_pos; }
   UInt_t      GetMAPQ() const { return v_mapq; }
   Int_t       GetNCIGAROP() { return v_ncigar_op; }
   Int_t       GetCIGAROPLEN(Int_t idx);
   Int_t       GetCIGAROP(Int_t idx);
   const char *GetCIGAR() const;
   const char *GetRNEXT() const { return v_rnext; }
   UInt_t      GetPNEXT() const { return v_pnext; }
   Int_t       GetTLEN() const { return v_tlen; }
   Int_t       GetSEQLEN() const { return v_lseq; }
   const char *GetSEQ() const;
   const char *GetQUAL() const;
   const char *GetOPT(Int_t idx) const { return v_opt[idx]; }

   void        Print(Option_t *option="") const;

   ClassDef(RAMRecord,1)
};

// Return values of GetCIGAROP()
const UChar_t RAM_CIGAR_M = 0;
const UChar_t RAM_CIGAR_I = 1;
const UChar_t RAM_CIGAR_D = 2;
const UChar_t RAM_CIGAR_N = 3;
const UChar_t RAM_CIGAR_S = 4;
const UChar_t RAM_CIGAR_H = 5;
const UChar_t RAM_CIGAR_P = 6;
const UChar_t RAM_CIGAR_EQUAL = 7;
const UChar_t RAM_CIGAR_X = 8;

// Illumina binning scheme:
// 1      1
// 2-9    6
// 10-19 15
// 20-24 22
// 25-29 27
// 30-34 33
// 35-39 37
// >=40  40
static const UChar_t illumina_binning[] = {
	0,   1,  6,  6,  6,  6,  6,  6,  6,  6, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 22, 22, 22, 22, 22, 27, 27, 27, 27, 27, 33, 33, 33, 33,
   33, 37, 37, 37, 37, 37, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
	40, 40, 40, 40, 40, 40, 40, 40
};

inline static UChar_t IlluminaBinning(UChar_t qual)
{
   return illumina_binning[qual-33];
}

inline RAMRecord::RAMRecord(const RAMRecord &rec) : TObject(rec)
{
   // RAMRecord copy ctor.

   v_qname     = rec.v_qname;
   v_flag      = rec.v_flag;
   v_rname     = rec.v_rname;
   v_pos       = rec.v_pos;
   v_mapq      = rec.v_mapq;
   v_cigar     = rec.v_cigar;
   v_ncigar_op = rec.v_ncigar_op;
   v_cigar     = nullptr;
   if (rec.v_cigar != nullptr) {
      v_cigar = new UInt_t[v_ncigar_op];
      memcpy(v_cigar, rec.v_cigar, v_ncigar_op*sizeof(UInt_t));
   }
   v_rnext     = rec.v_rnext;
   v_pnext     = rec.v_pnext;
   v_tlen      = rec.v_tlen;
   v_lseq      = rec.v_lseq;
   v_lseq2     = rec.v_lseq2;
   v_seq       = nullptr;
   if (rec.v_seq != nullptr) {
      v_seq = new UChar_t[v_lseq2];
      memcpy(v_seq, rec.v_seq, v_lseq2);
   }
   v_qual      = nullptr;
   if (rec.v_qual != nullptr) {
      v_qual = new UChar_t[v_lseq];
      memcpy(v_qual, rec.v_qual, v_lseq);
   }
   for (int i = 0; i < nopt; i++)
      v_opt[i] = rec.v_opt[i];
}

inline RAMRecord &RAMRecord::operator=(const RAMRecord &rhs)
{
   // RAMRecord assignment operator.

   if (this != &rhs) {
      TObject::operator=(rhs);
      v_qname     = rhs.v_qname;
      v_flag      = rhs.v_flag;
      v_rname     = rhs.v_rname;
      v_pos       = rhs.v_pos;
      v_mapq      = rhs.v_mapq;
      v_ncigar_op = rhs.v_ncigar_op;
      if (v_cigar != nullptr) {
         delete [] v_cigar;
         v_cigar = nullptr;
      }
      if (rhs.v_cigar != nullptr) {
         v_cigar = new UInt_t[v_ncigar_op];
         memcpy(v_cigar, rhs.v_cigar, v_ncigar_op*sizeof(UInt_t));
      }
      v_rnext     = rhs.v_rnext;
      v_pnext     = rhs.v_pnext;
      v_tlen      = rhs.v_tlen;
      v_lseq      = rhs.v_lseq;
      v_lseq2     = rhs.v_lseq2;
      if (v_seq != nullptr) {
         delete [] v_seq;
         v_seq = nullptr;
      }
      if (rhs.v_seq != nullptr) {
         v_seq = new UChar_t[v_lseq2];
         memcpy(v_seq, rhs.v_seq, v_lseq2);
      }
      if (v_qual != nullptr) {
         delete [] v_qual;
         v_qual = nullptr;
      }
      if (rhs.v_qual != nullptr) {
         v_qual = new UChar_t[v_lseq];
         memcpy(v_qual, rhs.v_qual, v_lseq);
      }
      for (int i = 0; i < nopt; i++)
         v_opt[i] = rhs.v_opt[i];
   }
   return *this;
}


static const char *codetoseq = "=ACMGRSVTWYHKDBN";

inline void RAMRecord::SetSEQ(const char *seq)
{
   // Use BAM like encoding for the segment SEQuence. This uses about half
   // the space compared to an ASCII string as the allowed character set is limited
   // (fits in 4 instead of 8 bits).

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

   if (v_seq != nullptr && v_lseq2 > maxlseq2) {
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


inline const char *RAMRecord::GetSEQ() const
{
   // Decode segment SEQuence from BAM like encoded format. See SetSEQ().

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

   // in case column v_seq is not read
   if (!v_seq)
      return "";

   if (seq != nullptr && v_lseq > maxlseq) {
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

inline void RAMRecord::SetQUAL(const char *qual)
{
   // Set QUALity string. This is in Phred+33 scale, same as in BAM.

   static Int_t maxlqual = 0;

   if (v_qual != nullptr && v_lseq > maxlqual) {
      delete [] v_qual;
      v_qual = nullptr;
   }

   if (v_qual == nullptr && v_lseq) {
      maxlqual = v_lseq;
      v_qual = new UChar_t[v_lseq];
   }

   if (TestBit(RAMRecord::kPhred33)) {
      memcpy(v_qual, qual, v_lseq);
   } else if (TestBit(RAMRecord::kIlluminaBinning)) {
      for (int i = 0; i < v_lseq; i++)
         v_qual[i] = IlluminaBinning(qual[i]);
   } else if (TestBit(RAMRecord::kDrop)) {
      memset(v_qual, 0, v_lseq);
   } else
      memcpy(v_qual, qual, v_lseq);
}

inline const char *RAMRecord::GetQUAL() const
{
   // Decode QUALity. See also SetQUAL().

   static int maxlqual = 0;
   static char *qual = nullptr;

   // in case column v_qual is not read
   if (!v_qual)
      return "";

   if (qual != nullptr && v_lseq > maxlqual) {
      delete [] qual;
      qual = nullptr;
   }

   if (qual == nullptr && v_lseq) {
      maxlqual = v_lseq;
      qual = new char[v_lseq+1];
      qual[v_lseq] = '\0';
   }

   if (TestBit(RAMRecord::kPhred33)) {
      memcpy(qual, v_qual, v_lseq);
   } else if (TestBit(RAMRecord::kIlluminaBinning)) {
      for (int i = 0; i < v_lseq; i++)
         qual[i] = v_qual[i]+33;   // make printable
   } else if (TestBit(RAMRecord::kDrop)) {
      strlcpy(qual, "*", v_lseq+1);
   } else
      memcpy(qual, v_qual, v_lseq);

   return qual;
}


static const char *codetocigar = "MIDNSHP=X";

inline void RAMRecord::SetCIGAR(const char *cigar)
{
   // Use BAM like encoding for the CIGAR code.

   static UChar_t cigartocode[256];
   static UChar_t iscigarcode[256];
   static bool init = false;
   if (!init) {
      memset(cigartocode, 0, 256);
      memset(iscigarcode, 0, 256);
      for (int i = 0; i < 9; i++) {
         cigartocode[codetocigar[i]] = i;
         iscigarcode[codetocigar[i]] = 1;
      }
      init = true;
   }

   static int maxops = 1024;   // more than enough
   if (v_cigar == nullptr) {
      v_cigar = new UInt_t[maxops];
   }

   int len = strlen(cigar);
   char cig[1024];             // more than enough
   strlcpy(cig, cigar, 1024);
   v_ncigar_op = 0;
   char *c = cig;
   for (int i = 0; i < len; i++) {
      if (iscigarcode[cig[i]] == 1) {
         UChar_t op = cigartocode[cig[i]];
         cig[i] = 0;
         UInt_t oplen = atoi(c);
         v_cigar[v_ncigar_op] = (oplen << 4) | op;
         v_ncigar_op++;
         if (v_ncigar_op > maxops) {
            Error("SetCIGAR", "please increase maxops, currently %d", maxops);
            return;
         }
         c = &cig[i+1];
      }
   }
}

inline Int_t RAMRecord::GetCIGAROPLEN(Int_t idx)
{
   // Return the length of the CIGAR operation specified by idx.

   if (idx >= v_ncigar_op) {
      Error("GetCIGAROPLEN", "idx=%d out of range, max=%d", idx, v_ncigar_op);
      return 0;
   }

   return v_cigar[idx] >> 4;
}

inline Int_t RAMRecord::GetCIGAROP(Int_t idx)
{
   // Return opcode of the CIGAR operation specified by idx.

   if (idx >= v_ncigar_op) {
      Error("GetCIGAROPLEN", "idx=%d out of range, max=%d", idx, v_ncigar_op);
      return 0;
   }

   return v_cigar[idx] & 0xf;
}

inline const char *RAMRecord::GetCIGAR() const
{
   // Rebuild the CIGAR string.

   static char cigar[1024];

   // in case column v_cigar is not read
   if (!v_cigar)
      return "";

   int l = 0;
   for (int i = 0; i < v_ncigar_op; i++) {
      l += snprintf(cigar+l, 1024, "%u%c", v_cigar[i] >> 4, codetocigar[v_cigar[i] & 0xf]);
      if (l > 1024-11) {  // 9 decimals in 28 bits + 1 for op + 1 for trailing 0
         Error("GetCIGAR", "please increase cigar string, currently %d", 1024);
         return cigar;
      }
   }
   return cigar;
}

inline void RAMRecord::Print(Option_t *) const
{
   // Print a single record, in SAM format.

   std::cout << GetQNAME() << "\t" << GetFLAG() << "\t" << GetRNAME() << "\t"
             << GetPOS() << "\t" << GetMAPQ() << "\t" << GetCIGAR() << "\t"
             << GetRNEXT() << "\t" << GetPNEXT() << "\t" << GetTLEN() << "\t"
             << GetSEQ() << "\t" << GetQUAL();
   for (int i = 0; i < nopt; i++)
      std::cout << "\t" << GetOPT(i);
   std::cout << endl;
}

#ifdef __ROOTCLING__
#pragma link C++ class RAMRecord+;
#endif

#endif
