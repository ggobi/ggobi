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

#include "read_init.h"
#include "colorscheme.h"

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

   /* Needs to be connected to MAXNVARS in scatmat.c
      and MAXNPCPLOTS in  parcoords.c
    */
#define MAXNVARS 4 
#define MAXNPCPLOTS 5
#define MAXNTSPLOTS 6 

#include "ggobiClass.h"

const GtkTypeLoad typeLoaders[] = {
                               	   gtk_ggobi_scatterplot_display_get_type,
                               	   gtk_ggobi_scatmat_display_get_type,
                               	   gtk_ggobi_par_coords_display_get_type,
                                   gtk_ggobi_time_series_display_get_type,
                                   gtk_ggobi_barchart_display_get_type
                                  };

const gchar * const ViewTypes[] =
  {"Scatterplot", "Scatterplot Matrix", 
#ifdef PARCOORDS_BUILTIN
"Parallel Coordinates",
#endif
};
const gint ViewTypeIndices[] = {};

const gchar *const DataModeNames[num_data_modes] =
  {"ASCII", "binary", "R/S data", "XML", "MySQL", "URL", "Unknown"};


void initSessionOptions();

gchar *
getOptValue(const gchar * const name, const gchar * const value)
{
  const gchar * ptr = (const gchar *) value;

  if(ptr[0] != '-' || ptr[1] != '-')
    return(NULL);

  if(strncmp(name, value + 2, strlen(name)) == 0) {
    ptr = value + strlen(name) + 2;
    if(ptr[0] != '=' || ptr[1] == '\0') {
      g_printerr("--%s must be given a value in the form --%s=value\n", name, name);
    fflush(stderr);          
    ptr = NULL;
  } else
    ptr = g_strdup(ptr+1);
  } else
    ptr = NULL;

  return((gchar *)ptr);
}

gint
parse_command_line (gint *argc, gchar **av)
{
  gboolean stdin_p = false;
  gchar *ptr;

/*
 * Now parse the command line.
*/
  for ( ; *argc>1 && av[1][0]=='-'; (*argc)--,av++) {
    /*
     * -s:  ggobi initiated from inside S
    */
    if (strcmp (av[1], "--help") == 0 || strcmp (av[1], "-help") == 0) {
      showHelp();
      exit(0);
    }
    else if (strcmp (av[1], "-s") == 0)
      sessionOptions->data_mode = Sprocess_data;
    else if (strcmp (av[1], "-ascii") == 0) {
      sessionOptions->data_mode = ascii_data;
    }
    else if (strcmp (av[1], "-xml") == 0) {
      sessionOptions->data_mode = xml_data;
    } else if (strcmp (av[1], "-v") == 0 || strcmp (av[1], "--validate") == 0) {
      extern gint xmlDoValidityCheckingDefaultValue;
      xmlDoValidityCheckingDefaultValue = 1;
   }

   else if(strcmp(av[1], "--verbose") == 0  || strcmp(av[1], "-verbose") == 0  || strcmp(av[1], "-V") == 0) {
      sessionOptions->verbose = GGOBI_VERBOSE;
   } 

   else if(strcmp(av[1], "--silent") == 0  || strcmp(av[1], "-silent") == 0)  {
      sessionOptions->verbose = GGOBI_SILENT;
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
    } else if (strcmp (av[1], "--version") == 0) {
      g_printerr ("%s\n", GGOBI(getVersionString()));
      exit (0);
    } else if(strcmp(av[1], "-init") == 0) {
#ifdef SUPPORT_INIT_FILES
      sessionOptions->initializationFile = g_strdup(av[2]);
#else
      g_printerr ("-init not supported without XML\n");fflush(stderr);
#endif
      (*argc)--; av++;
    } else if((ptr = getOptValue("init", av[1]))) {
#ifdef SUPPORT_INIT_FILES
      sessionOptions->initializationFile = ptr;
#endif
    } else if(strcmp(av[1], "-noinit") == 0) {
      sessionOptions->initializationFile = g_strdup("");
    } else if(strcmp(av[1],"-colorschemes") == 0) {
      sessionOptions->info->colorSchemeFile = av[2];
      /* read_colorscheme(av[2], &(sessionOptions->colorSchemes)); */
      (*argc)--; av++;
    } else if((ptr = getOptValue("colorschemes", av[1]))) {
       sessionOptions->info->colorSchemeFile = ptr;
    } else if(strcmp(av[1],"-activeColorScheme") == 0) {
      sessionOptions->activeColorScheme = g_strdup(av[2]);
      (*argc)--; av++;
    } else if((ptr = getOptValue("activeColorScheme", av[1]))) {
      sessionOptions->activeColorScheme = ptr;
    }  else if(strcmp(av[1], "-datamode") == 0) {
      sessionOptions->data_type = g_strdup(av[2]);
      (*argc)--; av++;
    }  else if((ptr = getOptValue("datamode", av[1]))) {
      sessionOptions->data_type = ptr;
    }  else if(strcmp(av[1], "--keepalive") == 0) {
      sessionOptions->info->quitWithNoGGobi = !sessionOptions->info->quitWithNoGGobi;
    } else if(strcmp(av[1], "-restore") == 0) {
      sessionOptions->restoreFile = g_strdup(av[2]);
      (*argc)--; av++;
    } else if((ptr = getOptValue("restore", av[1]))) {
      sessionOptions->restoreFile = ptr;
    } else if((ptr = getOptValue("plugin", av[1]))) {
      sessionOptions->pluginFiles = g_slist_append(sessionOptions->pluginFiles, g_strdup(ptr));
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
    sessionOptions->data_in = g_strdup(av[0]);

  return 1;
}

gint GGOBI(main)(gint argc, gchar *argv[], gboolean processEvents);

gint 
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
  gint numDatasets, i;

  /* Move all the entries after the one being removed
     down by one in the array to compact it.
   */
  if(which < num_ggobis -1) {
     memcpy(all_ggobis + which,
            all_ggobis + which +
            1,
            sizeof(ggobid*)*(num_ggobis-which-1));
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
  static gint count = 0;
  fprintf(stderr, "Key press event (count = %d): key = %d, data = %s\n", 
                     count, (gint)keyval, (gchar *)userData);
  fflush(stderr);

  if(++count == 4) {
    count = 0;
    GGOBI(removeNumberedKeyEventHandler)(gg);
  }
 return(true);
}
#endif

/**
  Find the color scheme element in the list with the specified
  name.
 */
colorschemed *
findColorSchemeByName(GList *schemes, const gchar *name)
{
  colorschemed *s;
  gint i, n;

  n = g_list_length(schemes);
  for(i = 0; i < n; i++) {
   s = (colorschemed *)g_list_nth_data(schemes, i);
   if(strcmp(name, s->name) == 0)
       return(s);
  }
  return(NULL);
}

ggobid* /*XXX should be void. Change when gtk-object setup settles. */
ggobi_alloc(ggobid *tmp)
{
  if(tmp == NULL) {
      /* Should never happen in new GtkObject-based version. */
      tmp = (ggobid*) g_malloc (sizeof (ggobid));
      memset (tmp, '\0', sizeof (ggobid)); 
  }

  tmp->firsttime = true;
  tmp->brush.firsttime = true;

  tmp->d = NULL;
  tmp->displays = NULL;

  /*-- initialize to NULLMODE and check for ncols later --*/
  tmp->viewmode = NULLMODE;
  tmp->prev_viewmode = NULLMODE;
  tmp->projection = NULLMODE;
  tmp->prev_projection = NULLMODE;
  /*-- --*/

  /*-- initialize main window, tool windows to NULL --*/
  tmp->main_window = NULL;
  tmp->display_tree.window = NULL;
  tmp->vartable_ui.window = NULL;
  tmp->sphere_ui.window = NULL;
  tmp->cluster_ui.window = NULL;
  tmp->color_ui.symbol_window = NULL;
  /*-- --*/

  tmp->color_ui.margin = 10;
  tmp->tour2d.idled = 0;
  tmp->tour1d.idled = 0;
  tmp->tourcorr.idled = 0;
  tmp->tour1d.fade_vars = true;
  tmp->tour2d.fade_vars = true;
  tmp->tourcorr.fade_vars = true;
  tmp->tour1d.all_vars = false;
  tmp->tour2d.all_vars = false;
  tmp->brush.updateAlways_p = true;

  tmp->tour2d3.idled = 0;

  tmp->printOptions = NULL;
  tmp->pluginInstances = NULL;

  tmp->plot_GC = NULL;


  tmp->colorSchemes = sessionOptions->colorSchemes;
  if (sessionOptions->activeColorScheme)
    tmp->activeColorScheme = findColorSchemeByName(tmp->colorSchemes,
      sessionOptions->activeColorScheme);
  else {
    /*-- use "Set1 9" by default, if it's present --*/
    sessionOptions->activeColorScheme = "Set1 9";
    tmp->activeColorScheme = findColorSchemeByName(tmp->colorSchemes,
      sessionOptions->activeColorScheme);
    if (!tmp->activeColorScheme)
      tmp->activeColorScheme = (colorschemed *)
        g_list_nth_data(tmp->colorSchemes, 0);
  }
  if (!tmp->activeColorScheme) {
    g_printerr ("failed to find color scheme\n");
    exit(0);
  } else colorscheme_init (tmp->activeColorScheme);
    /*
     * the number of colors in use will be tested against the
     * scheme->n the first time we plot, and the color ids will
     * be adjusted if necessary.
    */

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

void
ggobiInit(int *argc, char **argv[])
{
  if(ExtendedDisplayTypes) 
    return;

  gtk_init (argc, argv);
 
  GTK_TYPE_GGOBI;
  registerDisplayTypes((GtkTypeLoad *) typeLoaders,
		       sizeof(typeLoaders)/sizeof(typeLoaders)[0]);
}

  /* Available so that we can call this from R
     without any confusion between which main().
   */
gint 
GGOBI(main)(gint argc, gchar *argv[], gboolean processEvents)
{
  GdkVisual *vis;
  ggobid *gg;
  initSessionOptions();
  sessionOptions->cmdArgs = argv;
  sessionOptions->numArgs = argc;

  ggobiInit(&argc, &argv);

  vis = gdk_visual_get_system ();

  parse_command_line (&argc, argv);


#ifdef SUPPORT_INIT_FILES
  process_initialization_files();
#endif

  if(sessionOptions->verbose == GGOBI_VERBOSE)
    g_printerr("progname = %s\n", g_get_prgname());

  if(sessionOptions->verbose == GGOBI_VERBOSE)
    g_printerr("data_in = %s\n", sessionOptions->data_in);

  if(DefaultPrintHandler.callback == NULL)
    setStandardPrintHandlers();

  if(sessionOptions->info->colorSchemeFile && sessionOptions->colorSchemes == NULL) {
      read_colorscheme(sessionOptions->info->colorSchemeFile, &sessionOptions->colorSchemes);
  }

  if(sessionOptions->colorSchemes == NULL) {
    colorschemed *scheme = default_scheme_init();
    sessionOptions->colorSchemes = g_list_append(sessionOptions->colorSchemes, scheme);
    sessionOptions->activeColorScheme = scheme->name;
  }
  

  gg = gtk_type_new(GTK_TYPE_GGOBI); 

  gg->mono_p = (vis->depth == 1 ||
                vis->type == GDK_VISUAL_STATIC_GRAY ||
                vis->type == GDK_VISUAL_GRAYSCALE);

  make_ggobi (sessionOptions, processEvents, gg);

  /* g_free (sessionOptions->data_in); */

  return (num_ggobis);
}

/*XX
 This might usefully be changed to workd directly from the
 XML tree and avoid having the GGobiDisplayDescription.
 As we include more information (e.g. brushing information)
 we end up copying it for little reason. 
 However, this works for the restore file, but not necessarily
 the initialization file as we won't have created the ggobid
 when we read that. We could just re-parse the file and find the
 appropriate node. Probably the wisest.
*/
gboolean
processRestoreFile(const gchar * const fileName, ggobid *gg)
{
  xmlDocPtr doc;
  xmlNodePtr node;
  GGobiDescription desc;
  GList *el;
  doc = xmlParseFile(fileName); 
  if(!doc)
    return(false);

  node = xmlDocGetRootElement(doc);

  if(!node)
    return(false);

  getPreviousDisplays(node, &desc);

  el = desc.displays;
  while(el) {
    displayd *dpy;
    GGobiDisplayDescription *dpyDesc;
    dpyDesc = (GGobiDisplayDescription *) el->data;
    dpy = createDisplayFromDescription(gg, dpyDesc);    
    /*XX free dpyDesc here and remove from list. */
    el = el->next;
  }

  xmlFreeDoc(doc);

  return(true);
}


void
initSessionOptions()
{
  sessionOptions = &sessionoptions;
  sessionOptions->data_mode = unknown_data;

  sessionOptions->showControlPanel = true;
  sessionOptions->verbose = GGOBI_CHATTY;


  sessionOptions->info = (GGobiInitInfo*) g_malloc(sizeof(GGobiInitInfo));
  memset(sessionOptions->info, '\0', sizeof(GGobiInitInfo));
  sessionOptions->info->glyph.size = sessionOptions->info->glyph.type = -1;
  sessionOptions->info->createInitialScatterPlot = true;
  sessionOptions->info->allowCloseLastDisplay = false;
  sessionOptions->info->quitWithNoGGobi = true; 
  sessionOptions->info->numScatMatrixVars = MAXNVARS;
  sessionOptions->info->numParCoordsVars = MAXNPCPLOTS;
  sessionOptions->info->numTimePlotVars = MAXNTSPLOTS;
}


/*
  Called in response to a window being destroyed.
  Be careful not to use the event or the object
  as they are not guaranteed to be correct. We are 
  abusing the callback marshalling system a little.
 */
gboolean
ggobi_close (ggobid *gg, GdkEvent *ev, GtkObject *w)
{
  GGOBI(close)(gg, true);
  return(true);
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
  gint i;
  for(i = 0; i < num_ggobis ; i++) {
    if(all_ggobis[i] == gg)
      return(i);
  }

 return(-1);
}

datad *
GGobi_get_data(gint which, const ggobid * const gg)
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
 extern gint num_ggobis;
  gint i;
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
  gint i, n;
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
  gint i, n;
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
        gchar *v;
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
     
  if(fileName && fileName[0] && canRead(fileName)) {
    info = read_init_file(fileName, sessionOptions->info);
    /* sessionOptions->info = info; */
  }
  if(sessionOptions->pluginFiles) {
    GSList *el = sessionOptions->pluginFiles;
    while(el) {
       readPluginFile((char *) el->data, sessionOptions->info);
       el = el->next;
    }
  }
}


/* This includes code that provides information about the 
   sizes of the data structures in GGobi when it was compiled.
*/

#define GGOBI_MAIN 1
#include "GGStructSizes.c"

