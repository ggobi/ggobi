#include <stdio.h>

#include "ggobi.h"
#include "externs.h"

void
testRun(ggobid *gg)
{
  datad *d;
  GGobiOptions *opts;
  int i;
  fprintf(stderr, "In testRun in testDLL\n"); fflush(stderr);
  d = datad_new(NULL, gg);
  fprintf(stderr, "Datad %p\n", (void *) d); fflush(stderr);

  opts = getSessionOptions();
  fprintf(stderr, "Session options %p, %d\n", (void *) opts, opts->numArgs); fflush(stderr);
  for(i = 0; i < opts->numArgs; i++) {
    fprintf(stderr, "%d) %s\n", i, opts->cmdArgs[i]);
  }
}

