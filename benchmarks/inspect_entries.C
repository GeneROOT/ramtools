void inspect_entries(const char* file="output.root")
{
   TFile f(file);
   TTree *t = (TTree*)f.Get("RAM");
   if(!t) {
      printf("ERROR\n");
      return;
   }
   printf("%lld\n", t->GetEntries());
}
