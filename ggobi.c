#define XGOBIINTERN

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

static gchar *version_date = "January 5, 2000";

const gchar * const ViewTypes[] = {"Scatterplot", "Scatterplot Matrix", "Parallel Coordinates"};
const gint ViewTypeIndeces[] = { scatterplot, scatmat, parcoords};           

gint
parse_command_line (gint *argc, gchar **av)
{
  gboolean stdin_p = false;
  

/*
 * Now parse the command line.
*/
  for ( ; *argc>1 && av[1][0]=='-'; (*argc)--,av++) {

    /*
     * -s:  xgobi initiated from inside S
    */
    if (strcmp (av[1], "-s") == 0)
      xg.data_mode = Sprocess;

    /*
     * -:  look to stdin for the input data
    */
    else if (strcmp (av[1], "-") == 0) {
      stdin_p = true;
    }

    /*
     * -std:  look for one of mmx (default), msd, or mmd
    */
    else if (strcmp (av[1], "-std") == 0) {
      if (strcmp (av[2], "mmx") == 0) {
        xg.std_type = 0;
        av++; (*argc)--;
      }
      else if (strcmp (av[2], "msd") == 0) {
        xg.std_type = 1;
        av++; (*argc)--;
      }
      else if (strcmp (av[2], "mmd") == 0) {
        xg.std_type = 2;
        av++; (*argc)--;
      }
    }

    /*
     * -subset:  Sample size n; look for an integer.
    */
    else if (strcmp (av[1], "-subset") == 0) {
      gint n = atoi (av[2]);
      if (n > 1) {
        xg.nrows_in_plot = n;
        g_printerr ("Starting xgobi with a sample of size %d\n", n);
      }
	  av++; (*argc)--;
    }

    /*
     * -only n/N:  Draw a sample of n assuming there are N rows in the xg.
     * -only a,n:  Starting with row a, read only n rows
    */
    else if (strcmp (av[1], "-only") == 0) {
      gint n1 = -1, n2 = -1;
      gchar spec[128];
      xg.file_read_type = read_all;

      /*
       * we normally start at the first row of the file, and read until
       * reaching EOF
      */
      xg.file_start_row = 0;
      xg.file_length = 0;
      xg.file_sample_size = 0;

      strcpy (spec, av[2]);

      if (strchr ((const gchar *) spec, '/') != NULL)
        xg.file_read_type = draw_sample;
      else if (strchr ((const gchar *) spec, ',') != NULL)
        xg.file_read_type = read_block;

      if (xg.file_read_type == read_all)
        exit(0);

      n1 = atoi (strtok (spec, ",/"));
      n2 = atoi (strtok ((gchar *) NULL, ",/"));

      if (n1 == -1 || n2 == -1)
        exit(0);

      if (xg.file_read_type == draw_sample) {
        xg.file_sample_size = n1;
        xg.file_length = n2;
        g_printerr ("drawing a sample of %d of %d\n", n1, n2);
      }
      else if (xg.file_read_type == read_block) {
        xg.file_start_row = n1 - 1;
        xg.file_sample_size = n2;
        g_printerr ("reading %d rows, starting with %d\n", n2, n1);
      }

	  av++; (*argc)--;
    }

    /*
     * -version:  print version date, return
    */
    else if (strcmp (av[1], "-version") == 0) {
      g_printerr ("This version of XGobi is dated %s\n", version_date);
      exit (0);
    }

  }

  (*argc)--;
  av++;


/*
 * Test the values
*/

  if (xg.std_type != 0 && xg.std_type != 1 && xg.std_type != 2) {
    g_printerr ("std: Standardization type not valid; aborting.\n");
    exit(0);
  }

  /* (xg.data_mode == ascii || xg.data_mode == binary) */
  if (*argc == 0)
    xg.data_in = (stdin_p) ? g_strdup_printf ("stdin") : NULL;
  else
    xg.data_in = g_strdup_printf (av[0]);

  return 1;
}

gint XGOBI(main)(gint argc, gchar *argv[], gboolean processEvents);

gint main (gint argc, gchar *argv[])
{ 
 return (XGOBI(main)(argc, argv, true));
}

  /* Available so that we can call this from R
     without any confusion between which main().
   */
gint XGOBI (main)(gint argc, gchar *argv[], gboolean processEvents)
{
  extern void make_ggobi (gchar *, gboolean);
  GdkVisual *vis;


/*  g_thread_init (NULL);*/

  gtk_init (&argc, &argv);

  vis = gdk_visual_get_system ();

  xg.mono_p = (vis->depth == 1 ||
               vis->type == GDK_VISUAL_STATIC_GRAY ||
               vis->type == GDK_VISUAL_GRAYSCALE);

  g_print ("progname = %s\n", g_get_prgname());

  xg.std_type = 0;
  xg.data_mode = ascii;
  xg.nrows_in_plot = -1;
  xg.file_read_type = read_all;

  parse_command_line (&argc, argv);
  g_print ("data_in = %s\n", xg.data_in);

  make_ggobi (xg.data_in, processEvents);

  g_free (xg.data_in);
  return (0);
}
