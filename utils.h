//
// Auxiliary common functions
//
// Author: Jose Javier Gonzalez Ortiz, 6/6/2017
//


void stripcrlf(char *tok);
UInt_t djb2_hash(const char *str);


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

// DJB2 hash for char*
UInt_t djb2_hash(const char *str)
{
    UInt_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}