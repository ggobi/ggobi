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

#include "config.h"

#include "GGobiApp.h"

#include "ggobi.h"

#include "vars.h"
#include "externs.h"

#include "read_init.h"
#include "colorscheme.h"

#include "plugin.h"             /* For registerDefaultPlugin. */

#include "ggobi-intl.h"

#ifdef WIN32
#undef GGOBI_LOCALEDIR
#define GGOBI_LOCALEDIR ggobi_win32_get_localedir()
#undef GGOBI_DATADIR
#define GGOBI_DATADIR NULL
#include <windows.h>
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

const GTypeLoad typeLoaders[] = {
  ggobi_scatterplot_display_get_type,
  ggobi_scatmat_display_get_type,
  ggobi_par_coords_display_get_type,
  ggobi_time_series_display_get_type,
  ggobi_barchart_display_get_type
};

const gchar *const ViewTypes[] = {
  "Scatterplot",
  "Scatterplot Matrix",
};
const gint ViewTypeIndices[];

static gchar *computeGGobiHome (char *str);
static void initSessionOptions (int argc, char **argv);

static gint
parse_command_line (gint * argc, gchar ** av[])
{
  static gboolean print_version = false;
  static gchar *active_color_scheme = NULL;
  static gchar *color_scheme_file = NULL;
  static gchar *data_mode = NULL;
  static gchar *initialization_file = NULL;
  static gboolean quit_with_no_ggobi = true;
  static gint verbosity = GGOBI_CHATTY;
  static gboolean time_ggobi = false;
  static GOptionEntry entries[] = 
  {
    {
      "activeColorScheme", 'c', 0, G_OPTION_ARG_STRING, &active_color_scheme, 
      "name of the default color scheme to use", "scheme" 
    }, { 
      "colorSchemes", 's', 0, G_OPTION_ARG_FILENAME, &color_scheme_file,
      "name of XML file containing color schemes", "file" 
    }, {
      "dataMode", 'd', 0, G_OPTION_ARG_STRING, &data_mode,
      "mode of data supplied on command line", "mode"
    }, {
      "init", 'i', 0, G_OPTION_ARG_FILENAME, &initialization_file,
      "name of initialization file", "file"
    }, {
      "keepalive", 'k', G_OPTION_FLAG_HIDDEN | G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, 
      &quit_with_no_ggobi, "do not quit GGobi if all windows are closed", NULL,
    }, {
      "verbosity", 'l', 0, G_OPTION_ARG_INT, &verbosity,
      "verbosity of GGobi, 0 = silent, 1 = chatty (default), 2 = verbose", "level"
    }, {
      "version", 'v', 0, G_OPTION_ARG_NONE, &print_version,
      "print the GGobi version and exit", NULL
    }, {
      "time", 't', 0, G_OPTION_ARG_NONE, &time_ggobi,
      "run timing tests and exit", NULL
    }, { NULL }
  };
  
  GError *error = NULL;
  
  gtk_init_with_args (argc, av, "- platform for interactive graphics", entries, PACKAGE, &error);
  
  if (error) {
    g_printerr ("Error parsing command line: %s\n", error->message);
    exit(0);
  }
    
  if (print_version) {
    g_printerr ("%s\n", ggobi_getVersionString ());
    exit(0);
  }
  
  sessionOptions->activeColorScheme = active_color_scheme;
  if (color_scheme_file)
    sessionOptions->info->colorSchemeFile = color_scheme_file;
  sessionOptions->data_type = data_mode;
  sessionOptions->initializationFile = initialization_file;
  sessionOptions->info->quitWithNoGGobi = quit_with_no_ggobi;
  sessionOptions->verbose = verbosity;

  sessionOptions->timingp = time_ggobi;

  (*argc)--;
  (*av)++;

/*
 * Test the values
*/

  if (*argc == 0)
    sessionOptions->data_in = NULL;
  else
    sessionOptions->data_in = g_strdup ((*av)[0]);
  
  return 1;
}

gint
ggobi_remove (ggobid * gg)
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
ggobi_remove_by_index (ggobid * gg, gint which)
{
  GSList *l;
  GGobiStage *d;
  gint numDatasets, i;

  /* Move all the entries after the one being removed
     down by one in the array to compact it.
   */
  if (which < num_ggobis - 1) {
    memcpy (all_ggobis + which,
            all_ggobis + which +
            1, sizeof (ggobid *) * (num_ggobis - which - 1));
  }
  /* Now patch up the array so that it has the correct number of elements. */
  num_ggobis--;
  if (num_ggobis > 0)
    all_ggobis = (ggobid **)
      g_realloc (all_ggobis, sizeof (ggobid *) * num_ggobis);
  else
    all_ggobis = NULL;

  /* 
     This was crashing in R. Probably because when we exhaust the list
     and remove the final element, we get back garbage.
     This isn't a problem in stand-alone as it never gets called.
   */
  numDatasets = g_slist_length (gg->d);
  for (i = 0, l = gg->d; l != NULL && i < numDatasets; i++, l = gg->d) {
    d = (GGobiStage *) l->data;
    /* temporarily disabled - broken anyway */
    //ggobi_data_free (d);
    gg->d = g_slist_remove (gg->d, d);
  }

  g_object_unref (G_OBJECT (gg));

  return (which);
}

/*
 The code within the TEST_KEYS sections performs a test of handling key presses on 
 the numbered keys. It registers a function
 */
#ifdef TEST_KEYS
gboolean
DummyKeyTest (guint keyval, GtkWidget * w, GdkEventKey * event,
              cpaneld * cpanel, splotd * sp, ggobid * gg, void *userData)
{
  static gint count = 0;
  fprintf (stderr, "Key press event (count = %d): key = %d, data = %s\n",
           count, (gint) keyval, (gchar *) userData);
  fflush (stderr);

  if (++count == 4) {
    count = 0;
    ggobi_removeNumberedKeyEventHandler (gg);
  }
  return (true);
}
#endif

/**
  Find the color scheme element in the list with the specified
  name.
 */
colorschemed *
findColorSchemeByName (GList * schemes, const gchar * name)
{
  colorschemed *s;
  gint i, n;

  n = g_list_length (schemes);
  for (i = 0; i < n; i++) {
    s = (colorschemed *) g_list_nth_data (schemes, i);
    if (strcmp (name, s->name) == 0)
      return (s);
  }
  return (NULL);
}

ggobid *                        /*XXX should be void. Change when gtk-object setup settles. */
ggobi_alloc (ggobid * tmp)
{
  if (tmp == NULL) {
    /* Should never happen in new GObject-based version. 
       tmp = (ggobid*) g_malloc (sizeof (ggobid));
       memset (tmp, '\0', sizeof (ggobid)); 
     */
    tmp = g_object_new (GGOBI_TYPE_GGOBI, NULL);
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
    tmp->activeColorScheme = findColorSchemeByName (tmp->colorSchemes,
                                                    sessionOptions->
                                                    activeColorScheme);
  else {
    /*-- use "Set1 9" by default, if it's present --*/
    sessionOptions->activeColorScheme = "Set1 9";
    tmp->activeColorScheme = findColorSchemeByName (tmp->colorSchemes,
                                                    sessionOptions->
                                                    activeColorScheme);
    if (!tmp->activeColorScheme)
      tmp->activeColorScheme = (colorschemed *)
        g_list_nth_data (tmp->colorSchemes, 0);
  }
  if (!tmp->activeColorScheme) {
    g_error ("failed to find color scheme");
  }
  else
    colorscheme_init (tmp->activeColorScheme);
  /*
   * the number of colors in use will be tested against the
   * scheme->n the first time we plot, and the color ids will
   * be adjusted if necessary.
   */

  totalNumGGobis++;

  all_ggobis = (ggobid **)
    g_realloc (all_ggobis, sizeof (ggobid *) * (num_ggobis + 1));
  all_ggobis[num_ggobis] = tmp;
  num_ggobis++;

#ifdef TEST_KEYS
  ggobi_registerNumberedKeyEventHandler (DummyKeyTest,
                                           g_strdup
                                           ("A string for the key handler"),
                                           "Test handler", NULL, tmp, C);
#endif

  return (tmp);
}

  /* Available so that we can call this from R
     without any confusion between which main().
   */
gint ggobi_init (gint argc, gchar * argv[], gboolean processEvents)
{
  GdkVisual *vis;
  ggobid *gg;

  bindtextdomain (PACKAGE, GGOBI_LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);

  if (ExtendedDisplayTypes)
    return 1;

  initSessionOptions (argc, argv);
  parse_command_line (&argc, &argv);
  
  plugin_init ();

  GGOBI_TYPE_GGOBI;
  registerDisplayTypes ((GTypeLoad *) typeLoaders,
                        sizeof (typeLoaders) / sizeof (typeLoaders)[0]);

  registerDefaultPlugins (sessionOptions->info);
  
  process_initialization_files ();

  if (sessionOptions->verbose == GGOBI_VERBOSE) {
    g_printerr ("progname = %s\n", g_get_prgname ());
    g_printerr ("data_in = %s\n", sessionOptions->data_in);    
  }

  if (sessionOptions->info->colorSchemeFile
      && sessionOptions->colorSchemes == NULL) {
    read_colorscheme (sessionOptions->info->colorSchemeFile,
                      &sessionOptions->colorSchemes);
  }

  if (sessionOptions->colorSchemes == NULL) {
    colorschemed *scheme = default_scheme_init ();
    sessionOptions->colorSchemes =
      g_list_append (sessionOptions->colorSchemes, scheme);
    sessionOptions->activeColorScheme = scheme->name;
  }


  gg = g_object_new (GGOBI_TYPE_GGOBI, NULL);

  vis = gdk_visual_get_system ();
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
processRestoreFile (const gchar * const fileName, ggobid * gg)
{
  xmlDocPtr doc;
  xmlNodePtr node;
  GGobiDescription desc;
  GList *el;
  doc = xmlParseFile (fileName);
  if (!doc)
    return (false);

  node = xmlDocGetRootElement (doc);

  if (!node)
    return (false);

  getPreviousDisplays (node, &desc);

  el = desc.displays;
  while (el) {
    displayd *dpy;
    GGobiDisplayDescription *dpyDesc;
    dpyDesc = (GGobiDisplayDescription *) el->data;
    dpy = createDisplayFromDescription (gg, dpyDesc);
    /*XX free dpyDesc here and remove from list. */
    el = el->next;
  }

  xmlFreeDoc (doc);

  return (true);
}

/*
 Computes where GGobi directory is located.
 Search order: $GGOBI_HOME or 'str' stripped of file
*/
static gchar *
computeGGobiHome (char *str)
{
  gchar *dir;
  const gchar *env;

  env = g_getenv ("GGOBI_HOME");

  if (env)
    dir = g_strdup (env);
  else
    dir = g_path_get_dirname (str);

  return (dir);
}

static void
initSessionOptions (int argc, char **argv)
{
  gchar *tmp;
  sessionOptions = &sessionoptions;
  sessionOptions->data_mode = unknown_data;

  sessionOptions->showControlPanel = true;
  sessionOptions->verbose = GGOBI_CHATTY;

  sessionOptions->cmdArgs = argv;
  sessionOptions->numArgs = argc;

  sessionOptions->ggobiHome = computeGGobiHome (argv[0]);


  sessionOptions->info = (GGobiInitInfo *) g_malloc0 (sizeof (GGobiInitInfo));
  sessionOptions->info->glyph.size = sessionOptions->info->glyph.type = -1;
  sessionOptions->info->createInitialScatterPlot = true;
  sessionOptions->info->allowCloseLastDisplay = false;
  sessionOptions->info->quitWithNoGGobi = true;
  sessionOptions->info->numScatMatrixVars = MAXNVARS;
  sessionOptions->info->numParCoordsVars = MAXNPCPLOTS;
  sessionOptions->info->numTimePlotVars = MAXNTSPLOTS;

  sessionOptions->useRadioMenuItems = false;

  tmp = g_build_filename("share", "colorschemes.xml", NULL);
  sessionOptions->info->colorSchemeFile = ggobi_find_data_file(tmp);
  g_free(tmp);

  sessionOptions->defaultTourSpeed = 50.0;
  sessionOptions->defaultTour1dSpeed = 40.0;

  sessionOptions->timingp = false;
}

gboolean
ggobi_close (ggobid * gg)
{
  ggobi_close2 (gg, true);
  return (true);
}

/*
  Whether to destory the window or not.  If this is being called from an
  event handler in response to the window being destroyed, we would get
  a circularity. However, when called programmatically from within the
  process (or from e.g. R) we need to force it to be closed. */
gboolean ggobi_close2 (ggobid * gg, gboolean closeWindow)
{
  gboolean val = true;

  if (gg->close_pending)
    return (false);

  gg->close_pending = true;

  /* close plugin instances */
  closePlugins (gg);

  procs_activate (off, gg->pmode, gg->current_display, gg);

  display_free_all (gg);

  if (closeWindow && gg->main_window)
    gtk_widget_destroy (gg->main_window);

  if (gg->display_tree.window)
    gtk_widget_destroy (gg->display_tree.window);
  if (gg->vartable_ui.window)
    gtk_widget_destroy (gg->vartable_ui.window);

  if (gg->color_ui.symbol_window)
    gtk_widget_destroy (gg->color_ui.symbol_window);

  if (gg->wvis.window)
    gtk_widget_destroy (gg->wvis.window);
  if (gg->svis.window)
    gtk_widget_destroy (gg->svis.window);

  gg->close_pending = false;
  /* Now fix up the list of ggobi's */
  val = ggobi_remove (gg) != -1;

  if (ggobi_getNumGGobis () == 0 && sessionOptions->info->quitWithNoGGobi &&
      gtk_main_level () > 0) {
    gtk_main_quit ();
  }

  return (val);
}

/*
   Key for storing a reference to a ggobid instance in a widget
   so that we can retrieve it within a callback.
*/
static const gchar *GGobiGTKey = "GGobi";

const gchar *
key_get (void)
{
  return GGobiGTKey;
}

/*
  Computes the ggobid pointer associated with the specified
  widget. It does so by looking in the window associated with the widget
  and then looking for an entry in the window's association table.
  This assumes that the ggobid reference was stored in the window 
  when it was created.
 */
ggobid *
GGobiFromWidget (GtkWidget * w, gboolean useWindow)
{
  ggobid *gg = NULL;
  GObject *obj;
  obj = g_object_get_data (G_OBJECT (w), GGobiGTKey);
  if (obj) {
    gg = (ggobid *) obj;
    ValidateGGobiRef (gg, true);
  }

  return (gg);
}

ggobid *
GGobiFromWindow (GdkWindow * win)
{
  ggobid *gg = NULL;
  GObject *obj;
  obj = g_object_get_data (G_OBJECT (win), GGobiGTKey);
  if (obj) {
    gg = (ggobid *) obj;
    ValidateGGobiRef (gg, true);
  }

  return (gg);
}

ggobid *
GGobiFromSPlot (splotd * sp)
{
  ggobid *gg = NULL;
  displayd *display = NULL;
  if ((sp) && sp->displayptr) {
    display = (displayd *) sp->displayptr;
    if (display) {
      gg = ValidateGGobiRef (display->ggobi, false);
    }
  }
  return gg;
}

ggobid *
GGobiFromDisplay (displayd * display)
{
  return (display->ggobi);
}

void
ggobi_widget_set (GtkWidget * w, ggobid * gg, gboolean asIs)
{
  GtkWidget *wid = w;
  if (!asIs)
    wid = GTK_WIDGET (gtk_widget_get_parent_window (wid));

  g_object_set_data (G_OBJECT (wid), GGobiGTKey, gg);
}


ggobid *
ggobi_get (gint which)
{
  extern ggobid **all_ggobis;
  if (which > -1 && which < num_ggobis)
    return (all_ggobis[which]);
  else
    return (NULL);
}

gint
ggobi_getIndex (ggobid * gg)
{
  gint i;
  for (i = 0; i < num_ggobis; i++) {
    if (all_ggobis[i] == gg)
      return (i);
  }

  return (-1);
}

GGobiStage *
ggobi_get_data (gint which, const ggobid * const gg)
{
  GGobiStage *d;
  d = g_slist_nth_data (gg->d, which);

  return (d);
}

GGobiStage *
ggobi_get_data_by_name (const gchar * const name, const ggobid * const gg)
{
  GGobiStage *d;
  GSList *l;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    if (strcmp (d->name, name) == 0)
      return (d);
  }
  return (NULL);
}


ggobid *
ValidateGGobiRef (ggobid * gg, gboolean fatal)
{
  static gchar *error_msg = "Incorrect reference to ggobid.";
  extern ggobid **all_ggobis;
  extern gint num_ggobis;
  gint i;
  for (i = 0; i < num_ggobis; i++) {
    if (all_ggobis[i] == gg)
      return (gg);
  }

  if (fatal) 
    g_critical (error_msg);

  return (NULL);
}

GGobiStage *
ValidateDatadRef (GGobiStage * d, ggobid * gg, gboolean fatal)
{
  static gchar *error_msg = "Incorrect reference to datad.";
  gint i, n;
  n = g_slist_length (gg->d);
  for (i = 0; i < n; i++) {
    if (g_slist_nth_data (gg->d, i) == d)
      return (d);
  }

  if (fatal)
    g_error (error_msg);
  else
    g_critical (error_msg);

  return (NULL);
}



displayd *
ValidateDisplayRef (displayd * d, ggobid * gg, gboolean fatal)
{
  static gchar *error_msg = "Incorrect reference to display.";
  gint i, n;
  n = g_list_length (gg->displays);
  for (i = 0; i < n; i++) {
    if (g_list_nth_data (gg->displays, i) == d)
      return (d);
  }

  if (fatal)
    g_error (error_msg);
  else
    g_critical (error_msg);

  return (NULL);
}

static gchar *
ggobi_find_file_in_dir(const gchar *name, const gchar *dir, gboolean ggobi)
{
  gchar *tmp_name = g_build_filename(dir, ggobi ? "ggobi" : "", name, NULL);
  if (file_is_readable(tmp_name))
    return(tmp_name);
  g_free(tmp_name);
  return(NULL);
}

#ifdef WIN32
G_WIN32_DLLMAIN_FOR_DLL_NAME(static, dll_name)

static gchar*
ggobi_win32_get_localedir()
{
  static char *ggobi_localedir = NULL;
  if (ggobi_localedir == NULL) {
    gchar *temp;

    temp = g_win32_get_package_installation_subdirectory (PACKAGE, dll_name, "locale");

    /* ggobi_localedir is passed to bindtextdomain() which isn't
     * UTF-8-aware.
     */
    ggobi_localedir = g_win32_locale_filename_from_utf8 (temp);
    g_free (temp);
  }
  return ggobi_localedir;
}

static gchar*
ggobi_win32_get_packagedir()
{
  static char *ggobi_datadir = NULL;
  if (ggobi_datadir == NULL)
    ggobi_datadir = g_win32_get_package_installation_directory (PACKAGE, dll_name);
  return(ggobi_datadir);
}
#endif

static gchar * 
ggobi_find_file(const gchar *name, const gchar* user, const gchar* const *dirs)
{
  gchar *tmp_name, *cur_dir = g_get_current_dir();
  gint i;
  
  //g_debug("Looking for %s", name);
  if (sessionOptions && sessionOptions->ggobiHome) {
    tmp_name = ggobi_find_file_in_dir(name, sessionOptions->ggobiHome, false);
    if (tmp_name)
      return(tmp_name);
  }
  
  tmp_name = ggobi_find_file_in_dir(name, cur_dir, false);
  g_free(cur_dir);
  if (tmp_name)
    return(tmp_name);
  
  tmp_name = ggobi_find_file_in_dir(name, user, true);
  if (tmp_name)
    return(tmp_name);
  
  for (i = 0; dirs[i]; i++) {
    tmp_name = ggobi_find_file_in_dir(name, dirs[i], true);
    if (tmp_name)
      return(tmp_name);
  }
  
  #ifdef WIN32
  tmp_name = ggobi_find_file_in_dir(name, ggobi_win32_get_packagedir(), false);
  if (tmp_name)
    return(tmp_name);
  #endif
  
  return(NULL);
}

/* Looks in (by default, XDG environment can override some of these):
    $GGOBI_HOME
    Current directory
    $HOME/.local/share/ggobi (Windows: Documents, Application Data for user)
    $prefix/share/ggobi (Windows: GGobi installation directory)
*/
gchar*
ggobi_find_data_file(const gchar *name) 
{
  const gchar* data_dirs[] = { GGOBI_DATADIR, NULL };
  gchar *path = ggobi_find_file(name, g_get_user_data_dir(), data_dirs);
  //g_debug("Found data file: %s", path);
  return(path);
}
/* Looks in (by default, XDG environment can override some of these):
    $GGOBI_HOME
    Current directory
    $HOME/.config/ggobi (Windows: Documents, Application Data for user)
    /etc/xdg/ggobi (Windows: Documents, Application Data for All Users)
    (Windows: GGobi installation directory)
*/
gchar*
ggobi_find_config_file(const gchar *name)
{
  gchar *path = ggobi_find_file(name, g_get_user_config_dir(), g_get_system_config_dirs());
  //g_debug("Found config file: %s", path);
  return(path);
}

/*
  Determines which initialization file to use
  Checks for the one specified by
    1) the -init command line option
    2) the GGOBIRC environment variable
    3) the $HOME/.ggobirc file.
    4) user and system GGobi config dirs
 */
void
process_initialization_files ()
{
  GGobiInitInfo *info;
  gchar *fileName = NULL;

  if (sessionOptions->initializationFile)
    fileName = g_strdup (sessionOptions->initializationFile);
  else {
    fileName = g_strdup (g_getenv ("GGOBIRC"));
    if (!fileName || !fileName[0]) {
      const gchar *tmp;
      tmp = g_get_home_dir ();
      if (tmp) {
        fileName = g_build_filename (tmp, ".ggobirc", NULL);
        if (!file_is_readable (fileName)) {
          g_free (fileName);
          fileName = NULL;
        }
      }
      if (!fileName)
        fileName = ggobi_find_config_file("ggobirc");
    }
    if (fileName)
      sessionOptions->initializationFile = g_strdup (fileName);
  }

  if (fileName && fileName[0] && file_is_readable (fileName)) {
    info = read_init_file (fileName, sessionOptions->info);
    g_free (fileName);
    /* sessionOptions->info = info; */
  }

  if (sessionOptions->pluginFiles) {
    GSList *el = sessionOptions->pluginFiles;
    while (el) {
      readPluginFile ((char *) el->data, sessionOptions->info);
      el = el->next;
    }
  }
}

GGobiOptions *
GGOBI_getSessionOptions ()
{
  return (sessionOptions);
}


/* This includes code that provides information about the 
   sizes of the data structures in GGobi when it was compiled.
*/

#define GGOBI_MAIN 1
#include "GGStructSizes.c"

gint
ndatad_with_vars_get (ggobid *gg)
{
 gint nd;
 GSList *l;
 GGobiStage *d;

 /*-- silly to do this every time, perhaps, but harmless, I think --*/
 if (g_slist_length (gg->d) > 1) {
   nd = 0;
   for (l = gg->d; l; l = l->next) {
     d = (GGobiStage *) l->data;
     if (ggobi_stage_get_n_cols(d))
       nd++;
   }
 }  else nd = 1;

 return nd;
}

