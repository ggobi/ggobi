/*-- ggobi.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#define GGOBIINTERN

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "ggobi.h"

#include "vars.h"
#include "externs.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "print.h"

#ifdef USE_XML
#include "read_init.h"
#include "colorscheme.h"
#endif

#ifdef WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

static GGobiOptions sessionoptions;
GGobiOptions *sessionOptions;


ggobid **all_ggobis;
gint num_ggobis;
gint totalNumGGobis;


const gchar * const ViewTypes[] =
  {"Scatterplot", "Scatterplot Matrix", "Parallel Coordinates"};
const gint ViewTypeIndices[] = {scatterplot, scatmat, parcoords};           
const gchar *const DataModeNames[] =
  {"ASCII", "binary", "R/S data", "XML", "MySQL", "Unknown"};


void initSessionOptions();

gint
parse_command_line (gint *argc, gchar **av, ggobid *gg)
{
  gboolean stdin_p = false;
  

/*
 * Now parse the command line.
*/
  for ( ; *argc>1 && av[1][0]=='-'; (*argc)--,av++) {
    /*
     * -s:  ggobi initiated from inside S
    */
    if (strcmp (av[1], "-s") == 0)
      sessionOptions->data_mode = Sprocess_data;
    else if (strcmp (av[1], "-ascii") == 0) {
      sessionOptions->data_mode = ascii_data;
    }
    else if (strcmp (av[1], "-xml") == 0) {
#ifdef USE_XML
      sessionOptions->data_mode = xml_data;
#else
      g_printerr("No xml support compiled for this version, ignoring %s\n", av[1]);
#endif
    } else if (strcmp (av[1], "-v") == 0 || strcmp (av[1], "--validate") == 0) {
#ifdef USE_XML
      extern int xmlDoValidityCheckingDefaultValue;
      xmlDoValidityCheckingDefaultValue = 1;
#else
      g_printerr("No xml support compiled for this version, ignoring %s\n", av[1]);
#endif
   }

   else if(strcmp(av[1], "-verbose") == 0) {
      sessionOptions->verbose = true;
   } 
 
#ifdef USE_MYSQL
    else if (strcmp (av[1], "-mysql") == 0) {
    }
#endif

    /*
     * -:  look to stdin for the input data
    */
    else if (strcmp (av[1], "-") == 0) {
      stdin_p = true;
    }

    /*
     * -version:  print version date, return
    */
    else if (strcmp (av[1], "-version") == 0) {
      g_printerr ("This version of GGobi is dated %s\n", GGOBI(getVersionDate()));
      exit (0);
    }
    else if (strcmp (av[1], "--version") == 0) {
      g_printerr ("%s\n", GGOBI(getVersionString()));
      exit (0);
    } else if(strcmp(av[1], "-init") == 0) {
#ifdef SUPPORT_INIT_FILES
      sessionOptions->initializationFile = g_strdup(av[2]);
#else
      g_printerr ("-init not supported without XML\n");fflush(stderr);
#endif
      (*argc)--; av++;
    } else if(strcmp(av[1],"-colorschemes") == 0) {
#ifdef USE_XML
      sessionOptions->info->colorSchemeFile = av[2];
      /* read_colorscheme(av[2], &(sessionOptions->colorSchemes)); */
#else
      g_printerr ("-colorschemes not supported without XML\n"); fflush(stderr);
#endif
      (*argc)--; av++;

    }
 else if(strcmp(av[1],"-activeColorScheme") == 0) {
#ifdef USE_XML
      sessionOptions->activeColorScheme = g_strdup(av[2]);
#else
      g_printerr ("-colorschemes not supported without XML\n"); fflush(stderr);
#endif
      (*argc)--; av++;
    }
    else if(strcmp(av[1], "-datamode") == 0) {
	sessionOptions->data_type = g_strdup(av[2]);
	(*argc)--; av++;
    }
  }

  (*argc)--;
  av++;

/*
 * Test the values
*/

  if (*argc == 0)
    sessionOptions->data_in = (stdin_p) ? g_strdup_printf ("stdin") : NULL;
  else
    sessionOptions->data_in = g_strdup_printf (av[0]);

  return 1;
}

gint GGOBI(main)(gint argc, gchar *argv[], gboolean processEvents);

int 
main(gint argc, gchar *argv[])
{ 
 GGOBI(main)(argc, argv, true);
 return (0);
}

gint
ggobi_remove (ggobid *gg)
{ 
  gint i;
  for (i = 0; i < num_ggobis; i++) {
    if (all_ggobis[i] == gg) {
      return (ggobi_remove_by_index (gg, i));
    }
  }

  return (-1);
}

/*
      Also, need to close and free the displays associated with the ggobi.
 */
gint
ggobi_remove_by_index (ggobid *gg, gint which)
{
  GSList *l;
  datad *d;
  int numDatasets, i;

  /* Move all the entries after the one being removed
     down by one in the array to compact it.
   */
  if(which < num_ggobis -1) {
     memcpy(all_ggobis + which, all_ggobis + which + 1, sizeof(ggobid*)*(num_ggobis-which-1));
  }
  /* Now patch up the array so that it has the correct number of elements. */
  num_ggobis--;
  if (num_ggobis > 0)
    all_ggobis = (ggobid**)
      g_realloc (all_ggobis, sizeof(ggobid*) * num_ggobis);
  else
    all_ggobis = NULL;

  /* 
      This was crashing in R. Probably because when we exhaust the list
      and remove the final element, we get back garbage.
      This isn't a problem in stand-alone as it never gets called.
   */
  numDatasets = g_slist_length (gg->d);
  for (i=0,l = gg->d; l != NULL && i < numDatasets; i++, l = gg->d) {
    d = (datad *) l->data;
    datad_free (d, gg);
    gg->d = g_slist_remove (gg->d, d);
  }

  g_free (gg);

  return (which);
}

/*
 The code within the TEST_KEYS sections performs a test of handling key presses on 
 the numbered keys. It registers a function
 */
#ifdef TEST_KEYS
gboolean
DummyKeyTest(guint keyval, GtkWidget *w, GdkEventKey *event,  cpaneld *cpanel, splotd *sp, ggobid *gg, void *userData)
{
 static int count = 0;
  fprintf(stderr, "Key press event (count = %d): key = %d, data = %s\n", 
                     count, (int)keyval, (char *)userData);
  fflush(stderr);

  if(++count == 4) {
    count = 0;
    GGOBI(removeNumberedKeyEventHandler)(gg);
  }
 return(true);
}
#endif

ggobid*
ggobi_alloc()
{
 ggobid *tmp;

  tmp = (ggobid*) g_malloc (sizeof (ggobid));

  memset (tmp, '\0', sizeof (ggobid));
  tmp->firsttime = true;
  tmp->brush.firsttime = true;

  /*-- initialize to NULLMODE and check for ncols later --*/
  tmp->viewmode = NULLMODE;
  tmp->prev_viewmode = NULLMODE;
  tmp->projection = NULLMODE;
  tmp->prev_projection = NULLMODE;
  /*-- --*/

  tmp->color_ui.margin = 10;
  tmp->tour2d.idled = 0;
  tmp->tour1d.idled = 0;
  tmp->tourcorr.idled = 0;
  tmp->tour1d.fade_vars = true;
  tmp->tour2d.fade_vars = true;
  tmp->tourcorr.fade_vars = true;
  tmp->brush.updateAlways_p = true;

  tmp->printOptions = NULL;
  tmp->pluginInstances = NULL;

#ifdef USE_XML
  tmp->colorSchemes = sessionOptions->colorSchemes;

  if(sessionOptions->activeColorScheme)
      tmp->activeColorScheme = findColorSchemeByName(tmp->colorSchemes,
        sessionOptions->activeColorScheme);
  else
      tmp->activeColorScheme = (colorschemed *)
        g_list_nth_data(tmp->colorSchemes, 0);
#endif

  totalNumGGobis++;

  all_ggobis = (ggobid **)
    g_realloc (all_ggobis, sizeof(ggobid*)*(num_ggobis+1));
  all_ggobis[num_ggobis] = tmp;
  num_ggobis++;

#ifdef TEST_KEYS
  GGOBI(registerNumberedKeyEventHandler)(DummyKeyTest,
    g_strdup("A string for the key handler"),"Test handler", NULL, tmp, C);
#endif

  return (tmp);
}

  /* Available so that we can call this from R
     without any confusion between which main().
   */
gint 
GGOBI(main)(gint argc, gchar *argv[], gboolean processEvents)
{
  extern void make_ggobi (GGobiOptions *, gboolean, ggobid *);
  GdkVisual *vis;
  ggobid *gg;
  initSessionOptions();
  sessionOptions->cmdArgs = argv;
  sessionOptions->numArgs = argc;

  gtk_init (&argc, &argv);

  vis = gdk_visual_get_system ();

  gg = ggobi_alloc();

  gg->mono_p = (vis->depth == 1 ||
               vis->type == GDK_VISUAL_STATIC_GRAY ||
               vis->type == GDK_VISUAL_GRAYSCALE);

  parse_command_line (&argc, argv, gg);

#ifdef SUPPORT_INIT_FILES
  process_initialization_files();
#endif

  if(sessionOptions->verbose)
    g_printerr("progname = %s\n", g_get_prgname());

  if(sessionOptions->verbose)
    g_printerr("data_in = %s\n", sessionOptions->data_in);

  if(DefaultPrintHandler.callback == NULL)
    setStandardPrintHandlers();

#ifdef USE_XML
  if(sessionOptions->info->colorSchemeFile) {
      read_colorscheme(sessionOptions->info->colorSchemeFile, &sessionOptions->colorSchemes);
  }
#endif

  make_ggobi (sessionOptions, processEvents, gg);

  /* g_free (sessionOptions->data_in); */

  return (num_ggobis);
}

void
initSessionOptions()
{
  sessionOptions = &sessionoptions;
  sessionOptions->data_mode = unknown_data;

  sessionOptions->showControlPanel = true;

  sessionOptions->info = (GGobiInitInfo*) g_malloc(sizeof(GGobiInitInfo));
  memset(sessionOptions->info, '\0', sizeof(GGobiInitInfo));
  sessionOptions->info->glyph.size = sessionOptions->info->glyph.type = -1;
}


/*
  Called in response to a window being destroyed.
 */
void
ggobi_close (GtkObject *w, ggobid *gg)
{
  GGOBI(close)(gg, false);
}


/*
   Key for storing a reference to a ggobid instance in a widget
   so that we can retrieve it within a callback.
*/
const gchar * const GGobiGTKey = "GGobi";

const gchar* const key_get (void) {
  return GGobiGTKey;
}

/*
  Computes the ggobid pointer associated with the specified
  widget. It does so by looking in the window associated with the widget
  and then looking for an entry in the window's association table.
  This assumes that the ggobid reference was stored in the window 
  when it was created.
 */
ggobid*
GGobiFromWidget (GtkWidget *w, gboolean useWindow)
{
  /*
   GdkWindow *win = gtk_widget_get_parent_window(w);
   return(GGobiFromWindow(win));
  */
  ggobid *gg = NULL;
  gg = (ggobid*) gtk_object_get_data (GTK_OBJECT(w), GGobiGTKey);
  ValidateGGobiRef (gg, true);

  return (gg);
}

ggobid* GGobiFromWindow (GdkWindow *win)
{
  ggobid *gg = NULL;
  gg = (ggobid*) gtk_object_get_data(GTK_OBJECT(win), GGobiGTKey);
  ValidateGGobiRef (gg, true);

  return(gg);
}

ggobid* 
GGobiFromSPlot(splotd *sp)
{
 return (sp->displayptr->ggobi);
}

ggobid* 
GGobiFromDisplay(displayd *display)
{
  return (display->ggobi);
}

void
GGobi_widget_set (GtkWidget *w, ggobid *gg, gboolean asIs)
{
  GtkObject *obj;
  if (asIs)
    obj = GTK_OBJECT (w);
  else 
    obj = GTK_OBJECT (gtk_widget_get_parent_window (w));

  gtk_object_set_data (obj, GGobiGTKey, gg);
}


ggobid *
ggobi_get(gint which)
{
 extern ggobid** all_ggobis;
 if(which > -1 && which < num_ggobis)
   return (all_ggobis[which]);
 else
   return(NULL);
}

gint
ggobi_getIndex(ggobid *gg)
{
  int i;
  for(i = 0; i < num_ggobis ; i++) {
    if(all_ggobis[i] == gg)
      return(i);
  }

 return(-1);
}

datad *
GGobi_get_data(int which, const ggobid * const gg)
{
   datad *d;
   d = g_slist_nth_data(gg->d, which);

   return(d);
}

datad *
GGobi_get_data_by_name(const gchar * const name, const ggobid * const gg)
{
   datad *d;
   GSList *l;

   for (l = gg->d; l; l = l->next) {
     d = (datad *) l->data;
     if(strcmp(d->name, name) == 0)
	 return(d);
   }
   return(NULL);
}


ggobid*
ValidateGGobiRef(ggobid *gg, gboolean fatal)
{ 
 extern ggobid** all_ggobis;
 extern int num_ggobis;
  int i;
  for(i = 0; i < num_ggobis ; i++) {
   if(all_ggobis[i] == gg)
    return(gg);
  }

  g_printerr ("Incorrect reference to ggobid.\n");

 if (fatal)
  exit(10);

 return(NULL);
}

datad *
ValidateDatadRef(datad *d, ggobid *gg, gboolean fatal)
{
  int i, n;
  n = g_slist_length(gg->d);
  for(i = 0; i < n ; i++) {
   if(g_slist_nth_data(gg->d, i) == d)
    return(d);
  }

  g_printerr("Incorrect reference to datad.\n");

 if (fatal)
  exit(11);

 return(NULL); 
}



displayd *
ValidateDisplayRef(displayd *d, ggobid *gg, gboolean fatal)
{
  int i, n;
  n = g_list_length(gg->displays);
  for(i = 0; i < n ; i++) {
   if(g_list_nth_data(gg->displays, i) == d)
    return(d);
  }

  g_printerr("Incorrect reference to display.\n");

 if (fatal)
  exit(11);

 return(NULL); 
}

#ifdef USE_XML
/*
  Determines which initialization file to use
  Checks for the one specified by
    1) the -init command line option
    2) the GGOBIRC environment variable
    3) the $HOME/.ggobirc file.
    4) ggobirc in the directory of the executable (argv[0]) 
 */
void
process_initialization_files()
{
  GGobiInitInfo *info;
  gchar buf[100];
  const gchar *fileName = NULL;

  if(sessionOptions->initializationFile)
    fileName = sessionOptions->initializationFile;
  else {
           /* This use of getenv() is not portable. */
    fileName = getenv("GGOBIRC");
    if(!fileName || !fileName[0]) {
      gchar *tmp;
      /* HOME doesn't necessarily make sense in Windows. */
      tmp = getenv("HOME");
      if(tmp) {
        sprintf(buf, "%s/.ggobirc", tmp);
        fileName = buf;
      } else {
	char *v;
        tmp = g_strdup(sessionOptions->cmdArgs[0]);
        v = strrchr(tmp, DIR_SEPARATOR);
        if(v) {
	  v[1] = '\0';
	}
        sprintf(buf, "%sggobirc",tmp);
        fileName = buf;
        g_free(tmp);
      }
    }
    if(fileName)
      sessionOptions->initializationFile = g_strdup(fileName);
  }
     
  if(fileName) {
    info = read_init_file(fileName, sessionOptions->info);
    /* sessionOptions->info = info; */
  }
}
#endif
