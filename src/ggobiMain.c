#include "ggobi.h"

gint ggobi_main (gint argc, gchar * argv[], gboolean processEvents);

gint
main (gint argc, gchar * argv[])
{
  ggobi_main (argc, argv, true);
  return (0);
}
