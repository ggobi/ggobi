/*-- ggobi-API.c --*/
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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "GGobiAPI.h"

#include "ggobi.h"
#include "ggobi-data.h"
#include "types.h"
#include "vars.h"
#include "externs.h"
#include "display.h"
#include "plugin.h"

void warning (const char *msg);

#ifdef __cplusplus
extern "C"
{
#endif
  void ggobi_displays_release (ggobid * gg);
  void ggobi_display_release (displayd * display, ggobid * gg);
  void ggobi_splot_release (splotd * sp, displayd * display, ggobid * gg);
  /*void ggobi_data_release (GGobiData *, ggobid * gg);*/
#ifdef __cplusplus
}
#endif

const gchar *ggobi_getFileName (ggobid * gg)
{
  return (gg->input->fileName);
}

const gchar *ggobi_setFileName (const gchar * fileName, DataMode data_mode,
                                  ggobid * gg)
{
  const gchar *old = g_strdup (ggobi_getFileName (gg));
  fileset_read_init (fileName, NULL, NULL, gg);
  display_menu_build (gg);
  return (old);
}


DataMode ggobi_getDataMode (ggobid * gg)
{
  return (gg->input->mode);
}

DataMode ggobi_setDataMode (DataMode newMode, ggobid * gg)
{
  DataMode old = gg->input->mode;
  sessionOptions->data_mode = newMode;
  gg->input->mode = newMode;
  return (old);
}

/**
 Caller should now free the return value, but not the elements of the
 array.
 This is computed dynamically by querying each of the input plugins 
 for its collection of mode names.
*/
const gchar **ggobi_getDataModeNames (int *n)
{
  int ctr = 0, num, k, i;
  GList *plugins;
  const gchar **ans;
  GGobiPluginInfo *plugin;

  plugins = sessionOptions->info->inputPlugins;
  num = g_list_length (plugins);
  for (i = 0; i < num; i++) {
    plugin = g_list_nth_data (plugins, i);
    ctr += plugin->info.i->numModeNames;
  }

  ans = (const gchar **) g_malloc (sizeof (gchar *) * ctr);
  ctr = 0;
  for (i = 0; i < num; i++) {
    plugin = g_list_nth_data (plugins, i);
    for (k = 0; k < plugin->info.i->numModeNames; k++) {
      ans[ctr++] = plugin->info.i->modeNames[k];
    }
  }

  if (n)
    *n = ctr;

  return (ans);
}


gchar **ggobi_getVariableNames (gint transformed, GGobiData * d,
                                  ggobid * gg)
{
  gchar **names;
  gint nc = GGOBI_STAGE(d)->n_cols, j;

  names = (gchar **) g_malloc (sizeof (gchar *) * nc);

  for (j = 0; j < nc; j++) {
    names[j] = transformed ? ggobi_data_get_transformed_col_name(d, j) : ggobi_stage_get_col_name(GGOBI_STAGE(d), j);
  }

  return (names);
}


/*
  Closes the specified display
 */
void ggobi_destroyCurrentDisplay (ggobid * gg)
{
  display_free (gg->current_display, false, gg);
}


MissingValue_p GGobiMissingValue;

MissingValue_p
ggobi_setMissingValueIdentifier (MissingValue_p f)
{
  MissingValue_p old = GGobiMissingValue;
  GGobiMissingValue = f;
  return (old);
}


/*
  An initial attempt to allow new data to be introduced
  to the Ggobi session, replacing the existing contents.
 
  There are still a few details remaining regarding the scaling
  on the axes, etc. (See ruler_ranges_set in scatterplot.c)
  The reverse pipeline data has not been established correctly
  and the computation is incorrect.
  Specifically, the routine splot_screen_to_tform() is not
  

  When this works, we will take the calls for the different stages 
  and put them in separate routines.
 */
/*-- need two of these now, one to replace and one to append --*/
void
ggobi_setData (gdouble * values, gchar ** rownames, gchar ** colnames,
                 gint nr, gint nc, GGobiData * d, gboolean cleanup,
                 ggobid * gg, gboolean duplicate,
                 InputDescription * desc)
{
  gint i, j;
  gchar *lbl;
  gchar *varname;

  d->input = desc;
  if (ggobi_stage_get_name(GGOBI_STAGE(d)) == NULL)
    ggobi_stage_set_name(GGOBI_STAGE(d), desc->fileName);
  if (gg->input == NULL)
    gg->input = desc;

  ggobi_data_add_rows(d, GGOBI_STAGE(d)->n_rows- nr);
  ggobi_data_add_cols(d, GGOBI_STAGE(d)->n_cols - nc);

  if (values) {
    for (j = 0; j < nc; j++) {
      varname = (colnames) ? colnames[j] : NULL;
      g_debug("Column name %s", colnames[j]);
      ggobi_stage_set_col_name(GGOBI_STAGE(d), j, varname);

      for (i = 0; i < nr; i++) 
        ggobi_stage_set_raw_value(GGOBI_STAGE(d), i, j, values[i + j * nr]);
      
    }
    for (i = 0; i < nr; i++) {
      lbl = (rownames) ? rownames[i] : NULL;
      ggobi_stage_set_row_id(GGOBI_STAGE(d), i, lbl, false);
    }
  }

  ggobi_data_attach(d, gg, false);
}


/* These are all for freeing the currently held data. */
void ggobi_displays_release (ggobid * gg)
{
  GList *dlist;
  displayd *display;

  /* We have to be careful here as we are removing all the elements
     of the singly-linked list. When we remove the last one,
     the ->next value of the dlist becomes non-NULL. Hence
     we are getting garbage. Accordingly, we count down from the total
     number to remove using num and when this is 0, we exit.
     This should leave the slist allocated, but empty.

     We have to patch the list up afterwards.
   */
  gint num = g_list_length (gg->displays);

  for (dlist = gg->displays; dlist != NULL; dlist = dlist->next, num--) {
    if (num == 0)
      break;
    display = (displayd *) dlist->data;
    /*  display_release(display, gg); */
    display_free (display, true, gg);
  }
}

void ggobi_display_release (displayd * display, ggobid * gg)
{
  display_free (display, true, gg);
}


void ggobi_splot_release (splotd * sp, displayd * display, ggobid * gg)
{
  splot_free (sp, display, gg);
}


/**XX*/
const gchar *const *ggobi_getViewTypes (int *n)
{
  *n = NDISPLAYTYPES;
  return (ViewTypes);
}

const gint *ggobi_getViewTypeIndices (gint * n)
{
  extern const gint ViewTypeIndices[];
  *n = NDISPLAYTYPES;
  return (ViewTypeIndices);
}


displayd *ggobi_newScatterplot (gint ix, gint iy, GGobiData * d,
                                  ggobid * gg)
{
  displayd *display = NULL;
  gint vars[2];

  vars[0] = ix;
  vars[1] = iy;
  display = scatterplot_new_with_vars (false, 2, vars, d, gg);
#ifdef FORCE_ADD_DISPLAY
  display_add (display, gg);
#endif


  return (display);
}

displayd *ggobi_newScatmat (gint * rows, gint * columns, gint nr, gint nc,
                              GGobiData * d, ggobid * gg)
{
  displayd *display;

  display = scatmat_new (NULL, false, nr, rows, nc, columns, d, gg);
#ifdef FORCE_ADD_DISPLAY
  display_add (display, gg);    /*XX  the caller should add this display. */
#endif

  return (display);
}

displayd *ggobi_newParCoords (gint * vars, gint numVars, GGobiData * d,
                                ggobid * gg)
{
  displayd *display = NULL;

  display = parcoords_new (display, false, numVars, vars, d, gg);
#ifdef FORCE_ADD_DISPLAY
  display_add (display, gg);    /*XX  the caller should add this display. */
#endif


  return (display);
}

displayd *ggobi_newTimeSeries (gint * yvars, gint numVars, GGobiData * d,
                                 ggobid * gg)
{
  displayd *display = NULL;

  display = tsplot_new (display, false, numVars, yvars, d, gg);
#ifdef FORCE_ADD_DISPLAY
  display_add (display, gg);    /*XX  the caller should add this display. */
#endif

  return (display);
}

displayd *ggobi_createPlot (int type, char **varnames)
{
  displayd *display = NULL;
  /*
     display_new(type);
   */
  return (display);
}


const gchar *ggobi_getCurrentDisplayType (ggobid * gg)
{
/*XX */
  return (ggobi_getViewTypeName (gg->current_display));
}

const gchar *ggobi_getViewTypeName (displayd * dpy)
{
  gchar *val;

  if (!GGOBI_IS_EXTENDED_DISPLAY (dpy))
    return (NULL);

/*
 or use gtk_type_name(GTK_OBJECT_TYPE(dpy))
 */
  val = GGOBI_EXTENDED_DISPLAY_GET_CLASS (dpy)->treeLabel;

  return (val);
}


/*
  Pointer to the raw data managed by GGobi.
  Don't touch this.
 */
const gfloat **ggobi_getRawData (GGobiData * d, ggobid * gg)
{
  return ((const gfloat **) d->raw.vals);
}

/*
  Pointer to the second transformation of the data managed by GGobi.
  Don't touch this.
 */
const gfloat **ggobi_getTFormData (GGobiData * d, ggobid * gg)
{
  return ((const gfloat **) d->tform.vals);
}

void
warning (const gchar * msg)
{
  fprintf (stderr, "%s\n", msg);
  fflush (stderr);
}


/*-------------------------------------------------------------------------*/
/*        setting and getting edges                                        */
/*-------------------------------------------------------------------------*/

gboolean ggobi_getShowLines ()
{
  return (ggobi_getDefaultDisplayOptions ()->edges_undirected_show_p);
}

/* uh.. this takes a boolean value but always shows lines... what's up */
gboolean ggobi_setShowLines (displayd * dsp, gboolean val)
{
  GtkAction *action;
  gboolean old = ggobi_getShowLines ();
  /*ggobi_getDefaultDisplayOptions()->edges_undirected_show_p = val; */
  dsp->options.edges_undirected_show_p = true;

  action = gtk_ui_manager_get_action (dsp->menu_manager,
                                      "/menubar/Edges/ShowUndirectedEdges");
  if (action)
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), true);

  return (old);
}

/* setShowAxes  will require this code:

{
  GtkWidget *topmenu, *menu, *item;
  topmenu = widget_find_by_name (dspnew->menubar, "DISPLAY:options_topmenu");
  if (topmenu) {
    menu = GTK_MENU_ITEM(topmenu)->submenu;
    if (menu) {
      item = widget_find_by_name (menu, "DISPLAY:show_axes");
      if (item) {
        gtk_check_menu_item_set_active ((GtkCheckMenuItem *) item, false);
      }
    }
  }
}


*/

DisplayOptions *ggobi_getDefaultDisplayOptions ()
{
  return (&DefaultDisplayOptions);
}


displayd *ggobi_getDisplay (gint which, ggobid * gg)
{
  displayd *display = NULL;

  if (which < g_list_length (gg->displays))
    display = (displayd *) g_list_nth_data (gg->displays, which);

  return (display);
}

DisplayOptions *ggobi_getDisplayOptions (int displayNum, ggobid * gg)
{
  DisplayOptions *options = NULL;
  if (displayNum < 0)
    options = ggobi_getDefaultDisplayOptions ();
  else {
    displayd *display;
    display = ggobi_getDisplay (displayNum, gg);
    if (display)
      options = &(display->options);
  }

  return (options);
}


displayd *ggobi_getCurrentDisplay (ggobid * gg)
{
  return (gg->current_display);
}

gint ggobi_getCurrentDisplayIndex (ggobid * gg)
{
  return (g_list_index (gg->displays, gg->current_display));
}

gint ggobi_getCurrentPlotIndex (ggobid * gg)
{
  int val = -1;
  displayd *d;
  if (gg->current_splot) {
    d = ggobi_getCurrentDisplay (gg);
    val = g_list_index (d->splots, gg->current_splot);
  }

  return (val);
}

displayd *ggobi_setCurrentDisplay (int which, ggobid * gg)
{
  displayd *d;

  d = ggobi_getDisplay (which, gg);

  if (d != NULL)
    display_set_current (d, gg);

  return (d);
}


splotd *ggobi_getPlot (displayd * display, int which)
{
  splotd *sp = (splotd *) g_list_nth_data (display->splots, which);
  return (sp);
}


gint ggobi_getNumGGobis ()
{
  extern gint num_ggobis;
  return (num_ggobis);
}

#ifdef EXPLICIT_IDENTIFY_HANDLER
void ggobi_setIdentifyHandler (IdentifyProc proc, void *data, ggobid * gg)
{
  gg->identify_handler.handler = proc;
  gg->identify_handler.user_data = data;
}
#endif

void ggobi_getBrushSize (gint * w, gint * h, ggobid * gg)
{
  splotd *sp = gg->current_splot;

  *w = ABS (sp->brush_pos.x1 - sp->brush_pos.x2);
  *h = ABS (sp->brush_pos.y1 - sp->brush_pos.y2);
}

void ggobi_getBrushLocation (gint * x, gint * y, ggobid * gg)
{
  splotd *sp = gg->current_splot;

  *x = MIN (sp->brush_pos.x1, sp->brush_pos.x2);
  *y = MIN (sp->brush_pos.y1, sp->brush_pos.y2);
}

void
redraw (splotd * sp, ggobid * gg)
{
  brush_once (true, sp, gg);
  display_plot (sp->displayptr, FULL, gg);
}


void ggobi_setBrushSize (int w, int h, ggobid * gg)
{
  splotd *sp = gg->current_splot;
  displayd *display = sp->displayptr;

  sp->brush_pos.x1 = MIN (sp->brush_pos.x1, sp->brush_pos.x2);
  sp->brush_pos.y1 = MIN (sp->brush_pos.y1, sp->brush_pos.y2);

  sp->brush_pos.x2 = sp->brush_pos.x1 + w;
  sp->brush_pos.y2 = sp->brush_pos.y1 + h;

  brush_once (true, sp, gg);
  redraw (sp, gg);
  display_plot (display, FULL, gg);
}


void ggobi_setBrushLocation (gint x, gint y, ggobid * gg)
{
  gint wd, ht;
  splotd *sp = gg->current_splot;

  ggobi_getBrushSize (&wd, &ht, gg);

  sp->brush_pos.x1 = x;
  sp->brush_pos.y1 = y;
  sp->brush_pos.x2 = x + wd;
  sp->brush_pos.y2 = y + ht;

  brush_once (true, sp, gg);

  redraw (sp, gg);
}

gboolean ggobi_setBrushGlyph (gint type, gint size, ggobid * gg)
{
  if (type > -1)
    gg->glyph_id.type = type;
  if (size > -1)
    gg->glyph_id.size = size;

  return (true);                /* Should be true iff there is a change. */
}

void ggobi_getBrushGlyph (gint * type, gint * size, ggobid * gg)
{
  *type = gg->glyph_id.type;
  *size = gg->glyph_id.size;
}


/*
  Returns the dimensions of the specified
  splot in pixels which can then be used for
  specifying.
 */
void ggobi_getPlotPixelSize (gint * w, gint * h, splotd * sp)
{
  /* Temp */
  *w = -1;
  *h = -1;
}


splotd *ggobi_getSPlot (gint which, displayd * display)
{
  splotd *sp = (splotd *) g_list_nth_data (display->splots, which);
  return (sp);
}

gint ggobi_setPMode (const gchar * name, ggobid * gg)
{
  ProjectionMode old = pmode_get (gg->current_display, gg);
  ProjectionMode newMode = (ProjectionMode) ggobi_getPModeId (name);
  if (newMode != NULL_PMODE)
    ggobi_full_viewmode_set (newMode, DEFAULT_IMODE, gg);
  return (old);
}

gint ggobi_getPModeId (const gchar * name)
{
  gint n, i;
  const gchar *const *names = ggobi_getPModeNames (&n);

  for (i = 0; i < n; i++) {
    if (strcmp (names[i], name) == 0)
      return (i);
  }

  return (-1);
}

const gchar *ggobi_getPModeName (int which)
{
  int n;
  const gchar *const *names;

  names = ggobi_getPModeNames (&n);
  return (names[which]);
}

const gchar *ggobi_getPModeScreenName (int which, displayd * display)
{
  if (which == EXTENDED_DISPLAY_PMODE) {
    gchar *name;
    GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->imode_control_box (display,
                                                                   &name,
                                                                   display->
                                                                   ggobi);
    return name;
  }
  return (ggobi_getPModeName (which));
}

gint ggobi_setIMode (const gchar * name, ggobid * gg)
{
  InteractionMode old = imode_get (gg);
  InteractionMode newMode = (InteractionMode) ggobi_getIModeId (name);
  if (newMode != NULL_PMODE)
    ggobi_full_viewmode_set (NULL_PMODE, newMode, gg);
  return (old);
}

gint ggobi_getIModeId (const gchar * name)
{
  gint n, i;
  const gchar *const *names = ggobi_getIModeNames (&n);

  for (i = 0; i < n; i++) {
    if (strcmp (names[i], name) == 0)
      return (i);
  }

  return (-1);
}

const gchar *ggobi_getIModeName (int which)
{
  int n;
  const gchar *const *names;

  names = ggobi_getIModeNames (&n);
  return (names[which]);
}

const gchar *ggobi_getIModeScreenName (int which, displayd * display)
{
  if (which == DEFAULT_IMODE)
    return (ggobi_getPModeScreenName (display->cpanel.pmode, display));
  return (ggobi_getIModeName (which));
}

const gchar *ggobi_getPModeKey (int which)
{
  int n;
  const gchar *const *keys = ggobi_getPModeKeys (&n);
  return (keys[which]);
}

/*
gint
ggobi_getModeId(const gchar *name)
{
  gint n, i;
  const gchar *const *names = ggobi_getOpModeNames(&n);
 
  for(i = 0; i < n; i++) {
    if(strcmp(names[i],name) == 0)
      return(i);
  }

  return(-1);
}

const gchar *
ggobi_getModeName(int which)
{ 
  int n;
  const gchar *const *names = ggobi_getOpModeNames(&n);
  return(names[which]);
}
*/

gint ggobi_setBrushColor (gint cid, ggobid * gg)
{
  gint old = gg->color_id;
  if (cid > -1 && cid < gg->activeColorScheme->n)
    gg->color_id = cid;

  return (old);
}

gint ggobi_getBrushColor (ggobid * gg)
{
  return (gg->color_id);
}

const 
gchar *ggobi_getColorName (gint cid, ggobid * gg, gboolean inDefault)
{
  if (cid >= 0 && cid < gg->activeColorScheme->n) {
    return ((gchar *) g_array_index (gg->activeColorScheme->colorNames,
                                     gchar *, cid));
  }

  return (NULL);
}


void
ggobi_setPlotRange (double *x, double *y, int plotNum, displayd * display,
                      gboolean pixels, ggobid * gg)
{
  splotd *sp;

  sp = ggobi_getPlot (display, plotNum);

  if (pixels) {

  }
  else {

    splot_zoom (sp, *x, *y);
  }

  /*
     fcoords tfmin, tfmax;
     tfmin.x = x[0];
     tfmin.y = y[0];
     tfmax.x = x[1];
     tfmax.y = y[1];

     if (GTK_WIDGET_VISIBLE (display->hrule)) {
     if (((gfloat) GTK_RULER (display->hrule)->lower != tfmin.x) ||
     ((gfloat) GTK_RULER (display->hrule)->upper != tfmax.x))
     {
     GTK_RULER_set_range (GTK_RULER (display->hrule),
     (gdouble) tfmin.x, (gdouble) tfmax.x);
     }
     }

     if (GTK_WIDGET_VISIBLE (display->vrule)) {
     if (((gfloat) GTK_RULER (display->vrule)->upper != tfmin.y) ||
     ((gfloat) GTK_RULER (display->vrule)->lower != tfmax.y))
     {
     GTK_RULER_set_range (GTK_RULER (display->vrule),
     (gdouble) tfmax.y, (gdouble) tfmin.y);
     }
     }
   */
}


/*
  This handles the raising and lowering or the iconifying or
  de-iconifying of one or more windows.  If which is negative,
  the operation applies to all the displays with the ggobid
  instance.  Otherwise, the operation applies just to the
  display indexed by which.

  The two logical arguments indicate whether to raise/lower
  or iconify/deiconify.  Within these two operation types, the
  up argument indicates whether to raise or lower, an iconify
  or deiconify.
 */
gboolean
ggobi_raiseWindow (int which, gboolean raiseOrIcon, gboolean up,
                     ggobid * gg)
{
  windowDisplayd *display;
  gboolean ok = false;
  int start, end, i;

  if (which < 0) {
    start = 0;
    end = g_list_length (gg->displays);
  }
  else {
    end = which + 1;
    start = which;
  }

  for (i = start; i < end; i++) {
    display = (windowDisplayd *) g_list_nth_data (gg->displays, i);
    if (GGOBI_IS_WINDOW_DISPLAY (display) == false)
      continue;
    if (raiseOrIcon) {
      if (up)
        gdk_window_raise (display->window->window);
      else
        gdk_window_lower (display->window->window);
    }
    else {
      if (up)
        gtk_widget_hide_all (display->window);
      else
        gtk_widget_show_all (display->window);

    }
  }

  ok = true;


  gdk_flush ();
  return (ok);
}

gchar *ggobi_getDescription (ggobid * gg)
{
  if (!gg->input)
    return (NULL);

  return (g_strdup (gg->input->fileName));
}

/*
  Finds the index of the dataset named `name'
  in the specified ggobid object.
 */
int ggobi_datasetIndex (const char *name, const ggobid * const gg)
{
  GGobiData *d;
  int ctr = 0;
  GSList *tmp = gg->d;

  while (tmp) {
    d = (GGobiData *) tmp->data;
    if (strcmp (name, ggobi_stage_get_name(GGOBI_STAGE(d))) == 0)
      return (ctr);
    ctr++;
    tmp = tmp->next;
  }

  return (-1);
}

/*
  Returns the names of the different datasets
  maintained in the specified ggobid object.
 */
gchar **ggobi_getDatasetNames (gint * n, ggobid * gg)
{
  gint i;
  GGobiData *d;
  gchar **names;
  GSList *tmp = gg->d;
  *n = g_slist_length (gg->d);
  names = (gchar **) g_malloc (sizeof (gchar *) * (*n));
  for (i = 0; i < *n; i++) {
    d = (GGobiData *) tmp->data;
    names[i] = g_strdup (ggobi_stage_get_name(GGOBI_STAGE(d)));
    tmp = tmp->next;
  }

  return (names);
}

/*
 Added to the API and to avoid breaking code (e.g. in RSggobi)
 we add it here with a new name ggobi_ggobi_get.
*/
ggobid *ggobi_ggobi_get (gint which)
{
  return (ggobi_get (which));
}

gint ggobi_ncols (GGobiData * data)
{
  return (GGOBI_STAGE(data)->n_cols);
}

gint ggobi_nrecords (GGobiData * data)
{
  return (GGOBI_STAGE(data)->n_rows);
}


/*
 This is the routine one uses to register a handler for key press events
 for the numbered keys, i.e. 0, 1, ..., 9
 One can specify null values for each of these to remove the handler and have
 these events discarded.

 See notes/NumberedKeys.*, splot.c and ggobi.h also for more details 
 */
KeyEventHandler *ggobi_registerNumberedKeyEventHandler (KeyEventHandlerFunc
                                                          routine,
                                                          void *userData,
                                                          char *description,
                                                          ReleaseData *
                                                          releaseData,
                                                          ggobid * gg,
                                                          ProgrammingLanguage
                                                          lang)
{
  KeyEventHandler *old = gg->NumberedKeyEventHandler;
  KeyEventHandler *newValue;
  if (routine == NULL)
    newValue = NULL;
  else {
    newValue = g_malloc (1 * sizeof (KeyEventHandler));
    newValue->handlerRoutine = routine;
    newValue->userData = userData;
    newValue->description = g_strdup (description);
    newValue->language = lang;
    newValue->releaseData = releaseData;
  }

  gg->NumberedKeyEventHandler = newValue;

  return (old);
}

KeyEventHandler *ggobi_removeNumberedKeyEventHandler (ggobid * gg)
{
  return (ggobi_registerNumberedKeyEventHandler (NULL, NULL, NULL, NULL, gg, C));
}


#include "config.h"
static const gchar *version_date = GGOBI_RELEASE_DATE;
static const int GgobiVersionNumbers[] =
  { MAJOR_VERSION, MINOR_VERSION, MICRO_VERSION };
static const gchar *version_string = PACKAGE_VERSION;

const char *ggobi_getVersionDate ()
{
  return (version_date);
}

const char *ggobi_getVersionString ()
{
  return (version_string);
}

const int *ggobi_getVersionNumbers ()
{
  return (GgobiVersionNumbers);
}


GGobiData *ggobi_data_get (gint which, const ggobid * const gg)
{
  GGobiData *data = NULL;

  if (gg->d != NULL)
    data = g_slist_nth_data (gg->d, which);

  return (data);
}

GGobiData *ggobi_data_get_by_name (const gchar * const name,
                                     const ggobid * const gg)
{
  gint which;
  GGobiData *data = NULL;

  which = ggobi_datasetIndex (name, gg);
  if (which > -1) {
    data = ggobi_data_get (which, gg);
  }

  return (NULL);
}

void
ggobi_setSessionOptions (GGobiOptions * opts)
{
  sessionOptions = opts;
}


/* sets the tour projection matrix, F */
gboolean
ggobi_setTour2DProjectionMatrix (gdouble * Fvalues, gint ncols, gint ndim,
                                   gboolean vals_scaled, ggobid * gg)
{
  ProjectionMode vm = pmode_get (gg->current_display, gg);
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  GGobiData *d = dsp->d;
  gboolean candoit = true;
  gint i, j;

  if ((ncols != GGOBI_STAGE(d)->n_cols) || ndim != 2)
    candoit = false;

  if (candoit) {
    /* Set the scatterplot display mode to be tour2d */
    if (vm != TOUR2D) {
      /* Needs to be filled in */
    }

    /* Pause the tour */
    if (!cpanel->t2d.paused)
      tour2d_pause (cpanel, true, dsp, gg);

    /* Set the projection vector F */
    for (i = 0; i < ndim; i++)
      for (j = 0; j < ncols; j++)
        dsp->t2d.F.vals[i][j] = Fvalues[i + j * 2];

    /* If the values are scaled, then we need to multiply
       them by the tform data, else we multiply them by the 
       world data */
    if (vals_scaled) {
      /* Needs to be filled in */
    }
    else {
      display_tailpipe (dsp, FULL, gg);
      varcircles_refresh (d, gg);
    }
  }

  return (candoit);
}

const gdouble **ggobi_getTour2DProjectionMatrix (gint ncols, gint ndim,
                                                   gboolean vals_scaled,
                                                   ggobid * gg)
{
  displayd *dsp = gg->current_display;
  GGobiData *d = dsp->d;
  gdouble **Fvals;
  gint i, j;

  ncols = GGOBI_STAGE(d)->n_cols;
  ndim = 2;

  Fvals = (gdouble **) g_malloc (sizeof (gdouble *) * ncols);

  if (vals_scaled) {
    /* run the F values through the reverse pipeline */
  }
  else {
    for (i = 0; i < ndim; i++)
      for (j = 0; j < ncols; j++)
        Fvals[i][j] = dsp->t2d.F.vals[i][j];
  }

  return ((const gdouble **) Fvals);
}


guint
getGGobiSignal (GGobiSignalType which)
{
  /*
     XXX  assert(which > -1 && which < MAX_GGOBI_SIGNALS);
   */
  return (GGobiSignals[which]);
}


GSList *ggobi_getExtendedDisplayTypes ()
{
  return (ExtendedDisplayTypes);
}
