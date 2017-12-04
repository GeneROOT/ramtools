#include "ramrecord.h"

RAMRefs  *RAMRecord::fgRefs  = 0;
RAMIndex *RAMRecord::fgIndex = 0;


TTree *RAMRecord::GetTree(TFile *file, const char *treeName)
{
   if (!file) {
      ::Error("RAMRecord::GetTree", "file was not opened");
      return 0;
   }
   TTree *t = (TTree *) file->Get(treeName);
   ReadRefs();
   ReadIndex();
   return t;
}

void RAMRecord::WriteRefs()
{
   if (gFile) {
      if (fgRefs)
         gFile->WriteObjectAny(fgRefs, "RAMRefs", "Refs");
   } else
      ::Error("RAMRecord::WriteRefs", "no file open");
}

void RAMRecord::ReadRefs()
{
   if (gFile) {
      auto refs = (RAMRefs*) gFile->Get("Refs");
      if (fgRefs && refs->Size() > 0) {
         delete fgRefs;
         fgRefs = refs;
      }
   } else
      ::Error("RAMRecord::ReadRefs", "no file open");
}

void RAMRecord::WriteIndex()
{
   if (gFile) {
      if (fgIndex)
         gFile->WriteObjectAny(fgIndex, "RAMIndex", "Index");
   } else
      ::Error("RAMRecord::WriteIndex", "no file open");
}

void RAMRecord::ReadIndex()
{
   if (gFile) {
      auto index = (RAMIndex*) gFile->Get("Index");
      if (fgIndex && index->Size() > 0) {
         delete fgIndex;
         fgIndex = index;
      }
   } else
      ::Error("RAMRecord::ReadIndex", "no file open");
}


RAMRefs::RAMRefs()
{
   fLastId  = 0;
   fMaxId   = 100;
   fRefVec.reserve(fMaxId);
}

int RAMRefs::GetRefId(const char *rname, bool check_sort)
{
   if (rname[0] == '*')
      return -1;

   if (rname[0] == '=')
      return fLastId;

   if (fLastName == rname)
      return fLastId;

   if (check_sort) {
      auto it = std::find(fRefVec.begin(), fRefVec.end(), rname);
      if (it != fRefVec.end()) {
         printf("rname %s already inserted, file not sorted\n", rname);
         auto index = std::distance(fRefVec.begin(), it);
         fLastId = (int) index;
         fLastName = rname;
         return fLastId;
      }
   }   

   if (fLastId+1 >= fMaxId) {
      fMaxId *= 2;
      fRefVec.reserve(fMaxId);
   }

   fRefVec.push_back(rname);

   fLastId   = fRefVec.size()-1;
   fLastName = rname;

   return fLastId;
}

const char *RAMRefs::GetRefName(int rid, bool next)
{
   // When next is true, then we're called for RNEXT and return "="
   // in case rid is same as previous.

   if (rid == -1)
      return "*";

   static int lastid = -1;
   static string lastname;
   if (lastid > -1 && rid == lastid && next) {
      if (next)
         return "=";
      else
         return lastname.c_str();
   }

   if (rid >= (int) fRefVec.size())
      return "";

   lastname = fRefVec[rid];
   lastid   = rid;
   return lastname.c_str();
}

void RAMRefs::Print() const
{
   int size = fRefVec.size();
   printf("RAMRefs vector:\n");
   for (int i = 0; i < size; i++)
      printf("%d: %s\n", i, fRefVec[i].c_str());
}


void RAMIndex::AddItem(int refid, int pos, Long64_t row)
{
   Key_t key = std::make_pair(refid, pos);
   fIndex[key] = row;
}

Long64_t RAMIndex::GetRow(int refid, int pos)
{
   Index_t::iterator low;
   Key_t key = std::make_pair(refid, pos);
   low = fIndex.lower_bound(key);

   if (low == fIndex.end()) {
      return -1;   // nothing found
   } else if (low == fIndex.begin()) {
       std::cout << "low=(" << low->first.first << "," << low->first.second << ")\n";
       return low->second;
   } else {
      if ((low->first.first == refid) && (low->first.second == pos)) {
         std::cout << "low=(" << low->first.first << "," << low->first.second << ")\n";
         return low->second;
      } else {
         --low;
         std::cout << "low=(" << low->first.first << "," << low->first.second << ")\n";
         return low->second;
      }         
   }
}

void RAMIndex::Print() const
{
   Index_t::const_iterator it = fIndex.begin();
   printf("RAMIndex map:\n");
   while (it != fIndex.end()) {
      printf("%lld: refid=%d, pos=%d\n", it->second, it->first.first, it->first.second);
      ++it;
   }
}
