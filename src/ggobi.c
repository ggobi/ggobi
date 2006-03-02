/*-- ggobi.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#define GGOBIINTERN
#define GGOBI_C

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "GGobiApp.h"

#include "ggobi.h"

#include "vars.h"
#include "externs.h"

#include "config.h"

#include "print.h"

#include "read_init.h"
#include "colorscheme.h"

#ifdef WIN32
#include <windows.h>
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif


#include "plugin.h" /* For registerDefaultPlugin. */


GGobiApp *ggobiApp;

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

const GTypeLoad typeLoaders[] = {
  ggobi_scatterplot_display_get_type,
  ggobi_scatmat_display_get_type,
  ggobi_par_coords_display_get_type,
  ggobi_time_series_display_get_type,
  ggobi_barchart_display_get_type
};

const gchar * const ViewTypes[] = {
  "Scatterplot", 
  "Scatterplot Matrix", 
};
const gint ViewTypeIndices[];

static gchar *computeGGobiHome(char *str);

GGobiApp *
getGGobiApp()
{
   return(ggobiApp);
}


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
    else if (strcmp (av[1], "-s") == 0) {
      sessionOptions->data_mode = Sprocess_data;
      sessionOptions->data_type = g_strdup(av[1]+1);
    }
    else if (strcmp (av[1], "-csv") == 0) {
      sessionOptions->data_mode = csv_data;
      sessionOptions->data_type = g_strdup(av[1]+1);
    } else if (strcmp (av[1], "-ascii") == 0) {
      sessionOptions->data_mode = ascii_data;
      sessionOptions->data_type = g_strdup(av[1]+1);
    }
    else if (strcmp (av[1], "-xml") == 0) {
      sessionOptions->data_mode = xml_data;
      sessionOptions->data_type = g_strdup(av[1]+1);
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
	  sessionOptions->info->quitWithNoGGobi = false;
    } else if(strcmp(av[1], "-restore") == 0) {
      sessionOptions->restoreFile = g_strdup(av[2]);
      (*argc)--; av++;
    } else if((ptr = getOptValue("restore", av[1]))) {
      sessionOptions->restoreFile = ptr;
    } else if((ptr = getOptValue("tourSpeed", av[1]))) {
      sessionOptions->defaultTourSpeed = atof(ptr);
    } else if((ptr = getOptValue("tour1dSpeed", av[1]))) {
      sessionOptions->defaultTour1dSpeed = atof(ptr);
    } else if((ptr = getOptValue("plugin", av[1]))) {
      sessionOptions->pluginFiles = g_slist_append(sessionOptions->pluginFiles, g_strdup(ptr));
    } else if(strcmp(av[1], "-home") == 0 || strcmp(av[1], "--home") == 0) {
      fprintf(stdout, "%s\n", computeGGobiHome(av[0]));fflush(stdout);
      exit(0);
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

  g_object_unref(G_OBJECT(gg));
  /* gtk_object_destroy(GTK_OBJECT(gg)); */
/*  g_free (gg); */

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
      /* Should never happen in new GtkObject-based version. 
	 tmp = (ggobid*) g_malloc (sizeof (ggobid));
	 memset (tmp, '\0', sizeof (ggobid)); 
      */
	tmp = g_object_new(GGOBI_TYPE_GGOBI, NULL);
  }

  tmp->firsttime = true;
  tmp->brush.firsttime = true;

  tmp->d = NULL;
  tmp->displays = NULL;
  tmp->current_display = NULL;

  /*-- initialize to NULLMODE and check for ncols later --*/
  tmp->pmode = NULL_PMODE;
  tmp->pmode_prev = NULL_PMODE;
  tmp->imode = NULL_IMODE;
  tmp->imode_prev = NULL_IMODE;
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
  // tmp->brush.updateAlways_p = true;

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
    g_error ("failed to find color scheme");
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

  g_signal_emit_by_name(G_OBJECT(ggobiApp), "new_ggobi", tmp);

  return (tmp);
}

void
ggobiInit(int *argc, char **argv[])
{


  if(ExtendedDisplayTypes) 
    return;

  gtk_init (argc, argv);

  ggobiApp = g_object_new(GGOBI_TYPE_APP, NULL);

#ifdef TEST_GGOBI_APPP
/*XXX FIX */
  GGOBI(registerNumberedKeyEventHandler)(DummyKeyTest,
    g_strdup("A string for the key handler"),"Test handler", NULL, tmp, C);
#endif

  initSessionOptions(*argc, *argv);
 
  GGOBI_TYPE_GGOBI;
  registerDisplayTypes((GTypeLoad *) typeLoaders,
		       sizeof(typeLoaders)/sizeof(typeLoaders)[0]);

  registerDefaultPlugins(sessionOptions->info);
}


  /* Available so that we can call this from R
     without any confusion between which main().
   */
gint 
GGOBI(main)(gint argc, gchar *argv[], gboolean processEvents)
{
  GdkVisual *vis;
  ggobid *gg;

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
  

  gg = g_object_new(GGOBI_TYPE_GGOBI, NULL); 

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


#ifdef WIN32
gchar *
getGGobiHomeFromRegistry()
{
  HRESULT hr;
  gchar *tmp;
  char *key;
  BYTE *buf;
  DWORD bufSize, type;
  HKEY lkey;

  key = "Software\\GGobi";

  hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_ALL_ACCESS, &lkey);
  if(hr != ERROR_SUCCESS) {
    fprintf(stderr, "GGobi not in registry: %s (%d)\n", key, (int) hr);fflush(stderr);
    return(NULL);
  }

  RegQueryValueEx(lkey, "Home", NULL, NULL, NULL, &bufSize);
  buf = (BYTE*) g_malloc(bufSize * sizeof(BYTE));

  RegQueryValueEx(lkey, "Home", NULL, &type, buf, &bufSize);
  RegCloseKey(lkey);

  if(type == REG_SZ) {
     tmp = (gchar*) buf;
     fprintf(stderr, "Registry value of GGobi home: %s\n", tmp);
  }

  return(tmp);
}
#endif


/*
 Computes where GGobi directory is located.
 Ensures that there is a trailing / at the end.
*/
static gchar *
computeGGobiHome(char *str)
{
  gchar *dir, *dirPtr, *tmp;

  tmp = getenv("GGOBI_HOME");

#ifdef WIN32
  if(!tmp || !tmp[0]) 
    tmp = getGGobiHomeFromRegistry(); 
#endif

  if(!tmp) {
    dir = str;
    dirPtr = strrchr(dir, G_DIR_SEPARATOR);

    if(!dirPtr) {
     return(g_strdup(""));
    }

    /*
    This can't be right, based on the assignment a few characters
    down.   -- dfs
    tmp = (char *) g_malloc( ((dirPtr - dir) + 1)*sizeof(char));
    */
    tmp = (char *) g_malloc( ((dirPtr - dir) + 2)*sizeof(char));
    strncpy(tmp, dir, dirPtr-dir + 1);
    tmp[(dirPtr - dir) + 1] = '\0';
  }

  if(tmp[strlen(tmp)-1] == G_DIR_SEPARATOR)
     dir = g_strdup(tmp);
  else {
       /* Make certain there is a / at the end of the string. */
     dir = (char *) g_malloc( (strlen(tmp) + 2)*sizeof(char));
     sprintf(dir, "%s%c", tmp, G_DIR_SEPARATOR);
  }

  return(dir);
}

void
initSessionOptions(int argc, char **argv)
{
  gchar *tmp;
  sessionOptions = &sessionoptions;
  sessionOptions->data_mode = unknown_data;

  sessionOptions->showControlPanel = true;
  sessionOptions->verbose = GGOBI_CHATTY;

  sessionOptions->cmdArgs = argv;
  sessionOptions->numArgs = argc;

  sessionOptions->ggobiHome = computeGGobiHome(argv[0]);


  sessionOptions->info = (GGobiInitInfo*) g_malloc(sizeof(GGobiInitInfo));
  memset(sessionOptions->info, '\0', sizeof(GGobiInitInfo));
  sessionOptions->info->glyph.size = sessionOptions->info->glyph.type = -1;
  sessionOptions->info->createInitialScatterPlot = true;
  sessionOptions->info->allowCloseLastDisplay = false;
  sessionOptions->info->quitWithNoGGobi = true; 
  sessionOptions->info->numScatMatrixVars = MAXNVARS;
  sessionOptions->info->numParCoordsVars = MAXNPCPLOTS;
  sessionOptions->info->numTimePlotVars = MAXNTSPLOTS;

  sessionOptions->useRadioMenuItems = false;

  tmp = g_malloc(sizeof(gchar) * (strlen(sessionOptions->ggobiHome) + strlen("data/colorschemes.xml") + 2));
  sprintf(tmp, "%s%s%c%s", sessionOptions->ggobiHome,
	                   "share", G_DIR_SEPARATOR, "colorschemes.xml");
  sessionOptions->info->colorSchemeFile = tmp;

  sessionOptions->defaultTourSpeed = 50.0;  
  sessionOptions->defaultTour1dSpeed = 40.0;  
}

gboolean
ggobi_close (ggobid *gg)
{
  GGOBI(close)(gg, true);
  return(true);
}


/*
   Key for storing a reference to a ggobid instance in a widget
   so that we can retrieve it within a callback.
*/
static const gchar * GGobiGTKey = "GGobi";

const gchar* key_get (void) {
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
  ggobid *gg = NULL;
  GObject *obj;
  obj = g_object_get_data(G_OBJECT(w), GGobiGTKey);
  if (obj) {
    gg = (ggobid*) obj;
    ValidateGGobiRef (gg, true);
  }

  return (gg);
}

ggobid* GGobiFromWindow (GdkWindow *win)
{
  ggobid *gg = NULL;
  GObject *obj;
  obj = g_object_get_data(G_OBJECT(win), GGobiGTKey);
  if (obj) {
    gg = (ggobid*) obj;
    ValidateGGobiRef (gg, true);
  }

  return(gg);
}

ggobid* 
GGobiFromSPlot(splotd *sp)
{
  ggobid *gg = NULL;
  displayd *display = NULL;
  if ((sp) && sp->displayptr) {
    display = (displayd *) sp->displayptr;
    if (display) {
      gg = ValidateGGobiRef(display->ggobi, false);
    }
  }
  return gg;
}

ggobid* 
GGobiFromDisplay(displayd *display)
{
  return (display->ggobi);
}

void
GGobi_widget_set (GtkWidget *w, ggobid *gg, gboolean asIs)
{
  GtkWidget *wid = w;
  if (!asIs)
	  wid = GTK_WIDGET (gtk_widget_get_parent_window (wid));

  g_object_set_data (G_OBJECT(wid), GGobiGTKey, gg);
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
 static gchar *error_msg = "Incorrect reference to ggobid.";
 extern ggobid** all_ggobis;
 extern gint num_ggobis;
  gint i;
  for(i = 0; i < num_ggobis ; i++) {
   if(all_ggobis[i] == gg)
    return(gg);
  }

  if (fatal) {
     g_error(error_msg);
  } else g_critical(error_msg);

 return(NULL);
}

datad *
ValidateDatadRef(datad *d, ggobid *gg, gboolean fatal)
{
  static gchar *error_msg = "Incorrect reference to datad.";
  gint i, n;
  n = g_slist_length(gg->d);
  for(i = 0; i < n ; i++) {
   if(g_slist_nth_data(gg->d, i) == d)
    return(d);
  }

 if (fatal)
  g_error(error_msg);
 else g_critical(error_msg);
 
 return(NULL); 
}



displayd *
ValidateDisplayRef(displayd *d, ggobid *gg, gboolean fatal)
{
  static gchar *error_msg = "Incorrect reference to display.";
  gint i, n;
  n = g_list_length(gg->displays);
  for(i = 0; i < n ; i++) {
   if(g_list_nth_data(gg->displays, i) == d)
    return(d);
  }

 if (fatal)
  g_error(error_msg);
 else g_critical(error_msg);
 
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
	if(canRead(buf))
          fileName = buf;
        else 
          fileName = NULL;
      }

      if(!fileName) {
        sprintf(buf, "%sggobirc",sessionOptions->ggobiHome);
        fileName = buf;
        /*g_free(tmp);*/ /* purify objects to this */
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

GGobiOptions *
GGOBI_getSessionOptions()
{
  return(sessionOptions);
}


/* This includes code that provides information about the 
   sizes of the data structures in GGobi when it was compiled.
*/

#define GGOBI_MAIN 1
#include "GGStructSizes.c"

