#include <stdio.h>

static const char *const argHelp[] = {
#include "CmdArgHelp.c"
};

void
showHelp ()
{
  int i, n;

  n = sizeof (argHelp) / sizeof (argHelp[0]);
  if (n == 0) {
    fprintf (stdout,
             "There is no command line help for this particular compilation of GGobi. This is probably because CmdArgHelp.c was not created correctly\n");
  }
  for (i = 0; i < n; i += 2) {
    fprintf (stdout, "   %-20s\t%s\n", argHelp[i], argHelp[i + 1]);
  }
}
