#include <ggobi.h>

gint GGOBI(main)(gint argc, gchar *argv[], gboolean processEvents);

gint 
main(gint argc, gchar *argv[])
{ 
 GGOBI(main)(argc, argv, true);
 return (0);
}
