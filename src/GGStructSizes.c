/* GGStructSizes.c */

#ifndef GGStructSizes_C
#define GGStructSizes_C

typedef struct
{
  unsigned int size;
  const char *const name;
} ggobi_StructSize;

#ifdef USE_DBMS
#include "dbms.h"
#endif

#define GG_StructEntry(type) {sizeof(type), #type}

static const ggobi_StructSize ggobiStructs[] = {
  GG_StructEntry (ggobid),
  GG_StructEntry (GGobiStage),
  GG_StructEntry (displayd),
  GG_StructEntry (splotd),
  GG_StructEntry (GGobiVariable),
  GG_StructEntry (GGobiOptions)
#ifdef USE_DBMS
    , GG_StructEntry (DBMSLoginInfo)
#endif
};

extern const ggobi_StructSize *ggobi_getStructs (int *n);

const ggobi_StructSize *
#ifdef GGOBI_MAIN
  ggobi_getStructs (int *n)
#else
  ggobi_getGGobiStructs (int *n)
#endif
{
  *n = sizeof (ggobiStructs) / sizeof (ggobiStructs[0]);
  return (ggobiStructs);
}


#ifndef GGOBI_MAIN

#include <string.h>

gboolean
checkGGobiStructSizes ()
{
  const ggobi_StructSize *local, *internal;
  int nlocal, ninternal;
  int i, j;
  gboolean ok = false;

  local = ggobi_getStructs (&nlocal);
  internal = ggobi_getGGobiStructs (&ninternal);

  if (nlocal != ninternal)
    g_printerr ("Different number of structures in table (%d != %d)!\n",
                nlocal, ninternal);

  for (i = 0; i < nlocal; i++) {
    for (j = 0; j < ninternal; j++)
      if (strcmp (local[i].name, internal[j].name) == 0) {
        if (local[i].size != internal[j].size) {
          g_printerr ("Inconsistent struct sizes for %s: %d != %d\n",
                      local[i].name, local[i].size, internal[j].size);
        }
        ok = true;
        break;
      }
    if (j == ninternal) {
      g_printerr ("No entry for `%s' struct in the internals\n",
                  local[i].name);
      ok = false;
    }
  }
  return (ok);
}
#endif


#endif
