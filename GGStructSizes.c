/* GGStructSizes.c */

#ifndef GGStructSizes_C
#define GGStructSizes_C

typedef struct {
  unsigned int size;
  const char * const name;
} GGobi_StructSize;

#ifdef USE_DBMS
#include "dbms.h"
#endif

#define GG_StructEntry(type) {sizeof(type), #type}

static const GGobi_StructSize  ggobiStructs[] = {
	GG_StructEntry(ggobid),
	GG_StructEntry(datad),
	GG_StructEntry(displayd),
	GG_StructEntry(splotd), 
	GG_StructEntry(vartabled),
	GG_StructEntry(GGobiOptions)
#ifdef USE_DBMS
	,GG_StructEntry(DBMSLoginInfo)
#endif
};

extern const GGobi_StructSize *GGOBI(getStructs)(int *n);

const GGobi_StructSize *
#ifdef GGOBI_MAIN
GGOBI(getStructs)(int *n)
#else
GGOBI(getGGobiStructs)(int *n)
#endif
{
   *n = sizeof(ggobiStructs)/sizeof(ggobiStructs[0]);
   return(ggobiStructs);
}


#ifndef GGOBI_MAIN

#include <string.h>

gboolean
checkGGobiStructSizes(const GGobiPluginDetails *plugin)
{
  const GGobi_StructSize * local, * internal;
  int nlocal, ninternal;
  int i,j;
  gboolean ok;

  local = GGOBI(getStructs)(&nlocal);
  internal = GGOBI(getGGobiStructs)(&ninternal);

  if(nlocal != ninternal)
    g_printerr("<%s />Different number of structures in table (%d != %d)!\n", plugin->name, nlocal, ninternal);

  for(i = 0; i < nlocal; i++) {
     for(j = 0; j < ninternal ; j++)
       if(strcmp(local[i].name, internal[j].name) == 0) {
          if(local[i].size != internal[j].size) {
  	    g_printerr("<%s/> Inconsistent struct sizes for %s: %d != %d\n", plugin->name, local[i].name, local[i].size, internal[j].size);
	  }
       ok = true;
          break;
       }
     if(j == ninternal) {
       g_printerr("<%s />No entry for `%s' struct in the internals\n", plugin->name, local[i].name);
       ok = false;
     }
  }
  return(ok);
}
#endif


#endif

