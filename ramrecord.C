#include "ramrecord.h"

// statics
RAMRecord::RefMap *RAMRecord::fgRefMap = 0;
int                RAMRecord::fgNextId = 0;
int                RAMRecord::fgLastId = 0;
std::string        RAMRecord::fgLastName;


int RAMRecord::GetRefId(const char *rname, bool check_sort)
{
   if (rname[0] == '*')
      return -1;

   if (rname[0] == '=')
      return fgLastId;
   
   if (fgLastName == rname)
      return fgLastId;

   if (!fgRefMap)
      fgRefMap = new RefMap;
   if (check_sort) {
      if (fgRefMap->insert(std::make_pair(rname, fgNextId)).second == false) {
         printf("Element with key %s not inserted because already existing\n", rname);
      }
   } else
      (*fgRefMap)[rname] = fgNextId;
   fgLastId = fgNextId;
   fgLastName = rname;
   fgNextId++;
   
   return fgLastId;
}

const char *RAMRecord::GetRefName(int rid, bool next)
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

   std::map<std::string,int>::iterator it = fgRefMap->begin();
   while (it != fgRefMap->end()) {
      if (it->second == rid) {
         lastid = rid;
         lastname = it->first;
         return lastname.c_str();
      }
      it++;
   }
   return "";
}

void RAMRecord::WriteRefMap()
{
   if (gFile)
      gFile->WriteObjectAny(fgRefMap, "RAMRecord::RefMap", "fgRefMap");
   else
      printf("No file open\n");
}

void RAMRecord::ReadRefMap()
{
   RefMap *fgRefMap = 0;
   if (gFile)
      fgRefMap = (RefMap*) gFile->Get("fgRefMap");
   else
      printf("No file open\n");
}

void RAMRecord::PrintRefMap()
{
   RefMap::iterator it = fgRefMap->begin();
   while (it != fgRefMap->end()) {
      printf("%s: %d\n", it->first.c_str(), it->second);
      it++;
   }
}