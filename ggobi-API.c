/*-- ggobi-API.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "GGobiAPI.h"

#include "ggobi.h"
#include "datad.h"
#include "types.h"
#include "vars.h"
#include "externs.h"
#include "display.h"

extern const gchar *const GlyphNames[];

/*
extern const gchar *GlyphNames[];
*/


void warning(const char *msg);

#ifdef __cplusplus
extern "C" {
#endif
void GGOBI(displays_release)(ggobid *gg);
void GGOBI(display_release)(displayd *display, ggobid *gg);
void GGOBI(splot_release)(splotd *sp, displayd *display, ggobid *gg);
void GGOBI(data_release)(datad *, ggobid *gg);
void GGOBI(vartable_free)(datad *, ggobid *gg);
void GGOBI(vardatum_free)(vartabled *var, ggobid *gg);

#ifdef __cplusplus
}
#endif

const gchar *
GGOBI(getFileName) (ggobid *gg)
{
  return(gg->input->fileName);
}

const gchar *
GGOBI(setFileName) (const gchar *fileName, DataMode data_mode, ggobid *gg)
{
 const gchar *old = g_strdup(GGOBI(getFileName)(gg));
 fileset_read_init(fileName, data_mode, gg);
 return(old);
}


DataMode
GGOBI(getDataMode) (ggobid *gg)
{
  return(gg->input->mode);
}

DataMode
GGOBI(setDataMode) (DataMode newMode, ggobid *gg)
{
  DataMode old = gg->input->mode;
  sessionOptions->data_mode = newMode;
  gg->input->mode = newMode;
  return(old);
}

const gchar * const 
GGOBI(getDataModeDescription)(DataMode mode)
{
 extern const gchar * const DataModeNames[];
 return(DataModeNames[mode]);
}

const gchar *const *
GGOBI(getDataModeNames)(int *n)
{
 extern const gchar * const DataModeNames[];
 if(n)
  *n = num_data_modes;
 return(DataModeNames);
}


gchar **
GGOBI(getVariableNames)(gint transformed, datad *d, ggobid *gg)
{
  gchar **names;
  gint nc = d->ncols, i;
  vartabled *form;

  names = (gchar**) g_malloc (sizeof(gchar*)*nc);

  for (i = 0; i < nc; i++) {
    form = d->vartable + i;
    names[i] = transformed ? form->collab_tform : form->collab;
  }

  return (names);
}


void
GGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed,
  datad *d, ggobid *gg)
{

  if (!transformed)
    g_free (d->vartable[jvar].collab);
  g_free (d->vartable[jvar].collab_tform);

  if (transformed)
    d->vartable[jvar].collab_tform = g_strdup(name);
  else {
    extern GtkWidget *checkbox_get_nth (gint, datad *);
    d->vartable[jvar].collab = g_strdup(name);
    d->vartable[jvar].collab_tform = g_strdup(name);
    gtk_object_set (GTK_OBJECT(checkbox_get_nth (jvar, d)),
      "label", name, NULL);
   }
}


/*
  Closes the specified display
 */
void 
GGOBI(destroyCurrentDisplay)(ggobid *gg)
{
  display_free (gg->current_display, false, gg);
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
GGOBI(setData)(gdouble *values, gchar **rownames, gchar **colnames,
  gint nr, gint nc, datad *d, gboolean cleanup, ggobid *gg, InputDescription *desc)
{
  gint i, j;
  gchar *lbl;

  GGOBI(displays_release)(gg);
  GGOBI(data_release)(d, gg);

  d->input = desc;
  if(d->name == NULL)
    d->name = g_strdup(desc->fileName);
  if(gg->input == NULL)
    gg->input = desc;

  d->ncols = nc;
  d->nrows = nr;

  d->nrows_in_plot = d->nrows;  /*-- for now --*/
  d->nrgroups = 0;              /*-- for now --*/

  d->rows_in_plot = NULL;

  arrayf_alloc(&d->raw, nr, nc);

  rowlabels_alloc (d, gg);

  vartable_alloc (d);
  vartable_init (d);

  br_glyph_ids_alloc (d);
  br_glyph_ids_init (d, gg);

  br_color_ids_alloc (d, gg);
  br_color_ids_init (d, gg);


  hidden_alloc (d);

  for (j = 0; j < nc ; j++) {
   d->vartable[j].collab = g_strdup(colnames[j]);
   d->vartable[j].collab_tform = g_strdup(colnames[j]);
   for (i = 0; i < nr ; i++) {
     if (j == 0) {
       lbl = g_strdup (rownames[i]);
       g_array_append_val (d->rowlab, lbl);
       g_free (lbl);
     }

     d->raw.vals[i][j] = values[i + j*nr];
   }
  }


  /* Now recompute and display the top plot. */
  if (datad_init (d, gg, cleanup) != NULL) {
      /* Have to patch up the displays list since we removed
         every entry and that makes for meaningless entries.
       */
     gg->displays->next = NULL;

     display_menu_build (gg);
  }
}


/* These are all for freeing the currently held data. */

void
GGOBI(displays_release)(ggobid *gg)
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
 int num = g_list_length(gg->displays);

 for(dlist = gg->displays; dlist != NULL; dlist = dlist->next, num--) {
  if(num == 0)
   break;
  display = (displayd *) dlist->data;
  /*  display_release(display, gg); */
  display_free(display, true, gg);
 }
}

void
GGOBI(display_release)(displayd *display, ggobid *gg)
{
   display_free(display, true, gg);
}


void
GGOBI(splot_release)(splotd *sp, displayd *display, ggobid *gg)
{
 splot_free(sp, display, gg);
}

/* Not in the API for the moment. A "protected" routine. */
void
GGOBI(data_release)(datad *d, ggobid *gg)
{
  if(d == NULL)
    return;
 if (d->rowlab) {
    rowlabels_free (d, gg);
    d->rowlab = NULL;
  }

  GGOBI(vartable_free)(d, gg);
}

void
GGOBI(vartable_free)(datad *d, ggobid *gg)
{
 int i;

  for(i = 0; i < d->ncols ; i++) {
    GGOBI(vardatum_free)(d->vartable+i, gg);
  }
  g_free (d->vartable);
  d->vartable = NULL;
}

void 
GGOBI(vardatum_free)(vartabled *var, ggobid *gg)
{
 if (var->collab)
  g_free (var->collab);
 if (var->collab_tform)
  g_free (var->collab_tform);
}


const gchar * const*
GGOBI(getViewTypes)(int *n)
{
 *n = NDISPLAYTYPES;
 return (ViewTypes);
}

const gint *
GGOBI(getViewTypeIndices)(gint *n)
{
  extern gint ViewTypeIndices[];
  *n = NDISPLAYTYPES;
  return (ViewTypeIndices);
}


displayd *
GGOBI(newScatterplot) (gint ix, gint iy, datad *d, ggobid *gg)
{
 displayd *display = NULL;
 splotd* sp;

 display = display_alloc_init (scatterplot, false, d, gg);
 sp = splot_new (display, 400, 400, gg);

 sp->xyvars.x = ix;
 sp->xyvars.y = iy;

 display = scatterplot_new (false, sp, d, gg);
 display_add (display, gg);

 return (display);
}

displayd *
GGOBI(newScatmat) (gint *rows, gint *columns, gint nr, gint nc,
  datad *d, ggobid *gg)
{
  displayd *display = display_alloc_init (scatmat, false, d, gg);

  display = scatmat_new (false, nr, rows, nc, columns, d, gg);
  display_add (display, gg);

  return (display);
}

displayd *
GGOBI(newParCoords)(gint *vars, gint numVars, datad *d, ggobid *gg)
{
  displayd *display = NULL;

  display = display_alloc_init (parcoords, false, d, gg);
  display = parcoords_new (false, numVars, vars, d, gg);
  display_add (display, gg);

  return (display);
}

displayd* 
GGOBI(createPlot)(int type, char **varnames)
{
 displayd *display = NULL;
 /*
   display_new(type);
 */
 return(display);
}


const gchar* 
GGOBI(getCurrentDisplayType)(ggobid *gg)
{
 return(GGOBI(getViewTypeName)(gg->current_display->displaytype));
}

const gchar *
GGOBI(getViewTypeName)(enum displaytyped type)
{
 gint n, i;
 const gint *types = GGOBI(getViewTypeIndices)(&n);
 const gchar * const *names = GGOBI(getViewTypes)(&n);

 for (i = 0; i < n; i++) {
   if (types[i] == type) {
    return (names[i]);
   }
 }

 return (NULL);
}


/*
  Pointer to the raw data managed by GGobi.
  Don't touch this.
 */
const gfloat** 
GGOBI(getRawData)(datad *d, ggobid *gg)
{
  return((const gfloat**) d->raw.vals);
}

/*
  Pointer to the second transformation of the data managed by GGobi.
  Don't touch this.
 */
const gfloat** 
GGOBI(getTFormData)(datad *d, ggobid *gg)
{
  return ((const gfloat **) d->tform.vals);
}


/*
  Returns a reference to the labels used to identify
  the observations.
  Do not change this as it is not a copy.
 */
const gchar **
GGOBI(getCaseNames)(datad *d, ggobid *gg)
{
  gchar **rowlab = (gchar **) g_malloc (sizeof(gchar*) * d->nrows);
  gint i;
  for (i=0; i<d->nrows; i++)
    rowlab[i] = (gchar *) g_array_index (d->rowlab, gchar *, i);

  return ((const gchar **) rowlab);
}

/*
 This does not copy the label, so it is assumed
 that the caller has already allocated the space
 using the appropriate GTK memory model.
 If it is apparent that this is being called from 
 contexts that use a very different memory model (e.g. S/R),
 we can change the behaviour to copy this.

 Similarly, this can be modified to return the previous
 value, but the caller will have to free that pointer.
 */
void
GGOBI(setCaseName)(gint index, const gchar *label, datad *d, ggobid *gg)
{
  gchar *old;
  if (index < 0 || index >= d->nrows) {
    warning("Index is out of range of observations in setCaseName");
    return; 
  }

  old = g_array_index (d->rowlab, gchar *, index);
  g_free (old);

  g_array_insert_val (d->rowlab, index, label);
}


void
warning (const gchar *msg)
{
 fprintf(stderr, "%s\n", msg);
 fflush(stderr);
}


/*-------------------------------------------------------------------------*/
/*     setting and getting point glyph types and sizes                     */
/*-------------------------------------------------------------------------*/

gint *
GGOBI(getGlyphTypes)(int *n)
{
  static gint *glyphIds = NULL;
  *n = UNKNOWN_GLYPH-1; /* -1 since we start at 1 */

  if(glyphIds == NULL){
    gint i;
    glyphIds = (gint*) g_malloc(*n * sizeof(gint));
    for(i = 0; i < *n ; i++) {
      glyphIds[i] = mapGlyphName(GlyphNames[i]);
    }
  }

  return(glyphIds);
}

const gchar **const
GGOBI(getGlyphTypeNames)(gint *n)
{
  *n = UNKNOWN_GLYPH-1; /* -1 since we start at 1 */
  return ((const gchar **const) GlyphNames);
}


gchar const*
GGOBI(getGlyphTypeName)(gint type)
{
  gchar const *ans;
  ans = GlyphNames[type-1];

  return (ans);
}


gint *
GGOBI(getCaseGlyphTypes)(gint *ids, gint n, datad *d, ggobid *gg)
{
  gint i;
  gint *ans = (gint *) g_malloc (n * sizeof(gint));

  for (i = 0; i < n ; i++)
    ans[i] = GGOBI(getCaseGlyphType)(ids[i], d, gg);

  return (ids);
}

gint 
GGOBI(getCaseGlyphType)(gint id, datad *d, ggobid *gg)
{
  gint index = d->rows_in_plot[id];
  return (d->glyph_now[index].type);
}

gint *
GGOBI(getCaseGlyphSizes)(gint *ids, gint n, datad *d, ggobid *gg)
{
  gint i;
  gint *ans = (gint *) g_malloc (n * sizeof(gint));

  for (i = 0; i < n ; i++)
    ans[i] = GGOBI(getCaseGlyphSize)(ids[i], d, gg);

  return (ids);
}

gint 
GGOBI(getCaseGlyphSize)(gint id, datad *d, ggobid *gg)
{
  gint index = d->rows_in_plot[id];

  return (d->glyph_now[index].size);
}


void 
GGOBI(setCaseGlyph)(gint index, gint type, gint size, datad *d, ggobid *gg)
{
  if (size > -1)
    d->glyph_ids[index].size = d->glyph_now[index].size = size;
  if (type > -1)
    d->glyph_ids[index].type = d->glyph_now[index].type = type;
}

void 
GGOBI(setCaseGlyphs)(gint *ids, gint n, gint type, gint size,
  datad *d, ggobid *gg)
{
  gint i;
  for(i = 0; i < n ; i++)
    GGOBI(setCaseGlyph)(ids[i], type, size, d, gg);
}

/*-------------------------------------------------------------------------*/
/*               setting and getting point colors                          */
/*-------------------------------------------------------------------------*/

void 
GGOBI(setCaseColor)(gint pt, gint colorIndex, datad *d, ggobid *gg)
{
  d->color_ids.els[pt] = d->color_now.els[pt] = colorIndex;
}

void 
GGOBI(setCaseColors)(gint *pts, gint howMany, gint colorIndex,
  datad *d, ggobid *gg)
{
  gint i;
  for (i = 0; i < howMany ; i++)
    d->color_ids.els[pts[i]] = d->color_now.els[pts[i]] = colorIndex;
}


gint 
GGOBI(getCaseColor) (gint pt, datad *d, ggobid *gg)
{
  return (d->color_now.els[pt]);
}

gint *
GGOBI(getCaseColors)(gint *pts, gint howMany, datad *d, ggobid *gg)
{
  gint i;
  gint *ans = (gint*) g_malloc(howMany * sizeof(gint));

  for (i = 0; i < howMany ; i++)
   ans[i] = GGOBI(getCaseColor)(pts[i], d, gg);

  return(ans);
}

/*-------------------------------------------------------------------------*/
/*        setting and getting the point hidden state                       */
/*-------------------------------------------------------------------------*/

void 
GGOBI(setCaseHidden)(gint pt, gboolean hidden_p, datad *d, ggobid *gg)
{
  d->hidden.els[pt] = d->hidden_now.els[pt] = hidden_p;
  /*-- don't replot --*/
}

void 
GGOBI(setCaseHiddens)(gint *pts, gint howMany, gboolean hidden_p, datad *d, ggobid *gg)
{
  gint i;
  for (i = 0; i < howMany ; i++)
    GGOBI(setCaseHidden)(pts[i], hidden_p, d, gg);
  displays_plot (NULL, FULL, gg);
}

gboolean
GGOBI(getCaseHidden) (gint pt, datad *d, ggobid *gg)
{
  return (d->hidden_now.els[pt]);
}

gboolean *
GGOBI(getCaseHiddens)(gint *pts, gint howMany, datad *d, ggobid *gg)
{
 gint i;
 gboolean *ans = (gboolean *) g_malloc (howMany * sizeof(gboolean));

 for(i = 0; i < howMany ; i++)
  ans[i] = GGOBI(getCaseHidden)(pts[i], d, gg);

 return (ans);
}

/*-------------------------------------------------------------------------*/
/*        setting and getting edges                                        */
/*-------------------------------------------------------------------------*/


gboolean 
GGOBI(isConnectedEdge)(gint a, gint b, datad *d, ggobid *gg)
{
  gint tmp, i;

  if(a > b) {
     tmp = a;
     a = b;
     b = tmp;
  }

  for(i = 0; i < d->nedges ; i++) {
    
    if(d->edge_endpoints[i].a == a && d->edge_endpoints[i].b == b)
       return(true);

    if(d->edge_endpoints[i].a > a) {
      return(false);
    } 
  }

 return(false);
}

/*
  The additional argument update allows one to pre-allocate
  an entire block for edge_endpoints and then write into
  it, rather than reallocate the vector for each new edge.
  
  To do this, the value of update should be false.
 */
void
GGOBI(setObservationEdge)(gint x, gint y, datad *d, ggobid *gg, gboolean update)
{
  if (GGOBI(isConnectedEdge)(x, y, d, gg) == false) {
    if (update)
      edges_alloc (d->nedges+1, d, gg);
    d->edge_endpoints[d->nedges].a = x;
    d->edge_endpoints[d->nedges].b = y;
    d->nedges++;
  }
}


gboolean 
GGOBI(getShowLines)()
{
 return(GGOBI(getDefaultDisplayOptions)()->edges_directed_show_p);
}


gboolean GGOBI(setShowLines)(gboolean val)
{
 gboolean old = GGOBI(getShowLines)();
 GGOBI(getDefaultDisplayOptions)()->edges_directed_show_p = val;

 return(old);
}

DisplayOptions *
GGOBI(getDefaultDisplayOptions)()
{
 return(&DefaultDisplayOptions);
}


displayd *
GGOBI(getDisplay)(gint which, ggobid *gg)
{
  displayd *display = NULL;

  if (which < g_list_length (gg->displays))
   display = (displayd *) g_list_nth_data(gg->displays, which);

 return(display);
}

DisplayOptions *
GGOBI(getDisplayOptions)(int displayNum, ggobid *gg)
{
 DisplayOptions *options = NULL;
  if(displayNum < 0)
   options =  GGOBI(getDefaultDisplayOptions)();
  else {
   displayd *display;
    display = GGOBI(getDisplay)(displayNum, gg);
    if(display)
      options = &(display->options);
  }

 return(options);
}


displayd *
GGOBI(getCurrentDisplay)(ggobid *gg)
{
 return(gg->current_display);
}

gint
GGOBI(getCurrentDisplayIndex)(ggobid *gg)
{
 return(g_list_index(gg->displays, gg->current_display));
}

gint
GGOBI(getCurrentPlotIndex)(ggobid *gg)
{
 int val = -1;
 displayd *d;
  if(gg->current_splot) {
    d = GGOBI(getCurrentDisplay)(gg);
    val = g_list_index(d->splots, gg->current_splot);
  }
 
 return(val);
}

displayd *
GGOBI(setCurrentDisplay)(int which, ggobid *gg)
{
 displayd *d;

 d = GGOBI(getDisplay)(which, gg);

 if(d != NULL)
   display_set_current(d, gg);

 return(d);
}


splotd *
GGOBI(getPlot)(displayd *display, int which)
{
  splotd *sp = (splotd *) g_list_nth_data(display->splots, which);
  return(sp);
}


gint
GGOBI(getNumGGobis)()
{
 extern gint num_ggobis;
 return (num_ggobis);
}


gboolean
GGOBI(setColorMap)(double *vals, int nr, ggobid *gg)
{
 int i;

 gg->default_color_table = (GdkColor*) g_realloc(gg->default_color_table, sizeof(GdkColor) * nr);
 gg->ncolors = nr;

 for(i = 0; i < nr; i++) {
   gg->default_color_table[i].red = vals[i];
   gg->default_color_table[i].green = vals[i + nr];
   gg->default_color_table[i].blue = vals[i + 2*nr];
 }

 return(GGOBI(registerColorMap)(gg));
}



gboolean
GGOBI(registerColorMap)(ggobid *gg)
{
  gboolean *success;
  GdkColormap *cmap = gdk_colormap_get_system ();

  success = (gboolean *) g_malloc(sizeof(gboolean) * gg->ncolors);
  gdk_colormap_alloc_colors (cmap, gg->default_color_table, gg->ncolors,
    false, true, success);

  g_free(success);

 return(true);
}

/*
  Whether to destory the window or not.
  If this is being called from an event handler
  in response to the window being destroyed, we would
  get a circularity. However, when called programmatically
  from within the process (or from e.g. R) we need to force
  it to be closed.
 */
gboolean
GGOBI(close)(ggobid *gg, gboolean closeWindow)
{
  if (gg->close_pending)
    return (false);

  gg->close_pending = true;
  display_free_all (gg);

  if (closeWindow && gg->main_window)
    gtk_widget_destroy (gg->main_window);

  if (gg->display_tree.window)
    gtk_widget_destroy (gg->display_tree.window);
  if (gg->vartable_ui.window)
    gtk_widget_destroy (gg->vartable_ui.window);

  gg->close_pending = false;
  /* Now fix up the list of ggobi's */
  return (ggobi_remove(gg) != -1);
}

void
GGOBI(setIdentifyHandler)(IdentifyProc proc,  void *data, ggobid *gg)
{
  gg->identify_handler.handler = proc;
  gg->identify_handler.user_data = data;
}


void
GGOBI(getBrushSize)(gint *w, gint *h, datad *d, ggobid *gg)
{
  *w = ABS(d->brush_pos.x1 - d->brush_pos.x2);
  *h = ABS(d->brush_pos.y1 - d->brush_pos.y2);
}

void
GGOBI(getBrushLocation)(gint *x, gint *y, datad *d, ggobid *gg)
{
  *x = MIN(d->brush_pos.x1, d->brush_pos.x2);
  *y = MIN(d->brush_pos.y1, d->brush_pos.y2);
}

void
redraw (datad *d, ggobid *gg)
{
  /*
 active_paint_lines(gg);
 active_paint_lines(gg);
  */
  brush_once (true, d, gg);
  display_plot (gg->current_display, FULL, gg);
}


void GGOBI(setBrushSize)(int w, int h, datad *d, ggobid *gg)
{
  d->brush_pos.x1 = MIN(d->brush_pos.x1, d->brush_pos.x2);
  d->brush_pos.y1 = MIN(d->brush_pos.y1, d->brush_pos.y2);

  d->brush_pos.x2 =  d->brush_pos.x1 + w;
  d->brush_pos.y2 =  d->brush_pos.y1 + h;

  brush_once (true, d, gg);
  redraw (d, gg);
  display_plot (gg->current_display, FULL, gg);
}


void GGOBI(setBrushLocation)(gint x, gint y, datad *d, ggobid *gg)
{
 gint wd, ht;
 GGOBI(getBrushSize)(&wd, &ht, d, gg);
 
 d->brush_pos.x1 = x;
 d->brush_pos.y1 = y;

 d->brush_pos.x2 = x + wd;
 d->brush_pos.y2 = y + ht;

 /*
 GGOBI(setBrushSize)(wd, ht, gg);
 */

  brush_once (true, d, gg);

  redraw (d, gg);
}

gboolean
GGOBI(setBrushGlyph)(gint type, gint size, ggobid *gg)
{
  if(type > -1)
    gg->glyph_id.type = type;
  if(size > -1)
    gg->glyph_id.size = size;

  return(true); /* Should be true iff there is a change. */
}

void 
GGOBI(getBrushGlyph)(gint *type, gint *size, ggobid *gg)
{
  *type = gg->glyph_id.type;
  *size = gg->glyph_id.size;
}


/*
  Returns the dimensions of the specified
  splot in pixels which can then be used for
  specifying.
 */
void
GGOBI(getPlotPixelSize)(gint *w, gint *h, splotd *sp)
{
  /* Temp */
  *w = -1; *h = -1;
}


splotd *
GGOBI(getSPlot)(gint which, displayd *display)
{
  splotd *sp = (splotd*) g_list_nth_data(display->splots, which);
  return(sp);
}


gint
GGOBI(setMode)(const gchar *name, ggobid *gg)
{
  gint old = mode_get(gg);
  gint newMode =   GGOBI(getModeId)(name);
  if(newMode > -1)
    GGOBI(full_mode_set)(newMode, gg);

  return(old);
}

gint
GGOBI(getModeId)(const gchar *name)
{
  gint n, i;
  const gchar *const *names = GGOBI(getOpModeNames)(&n);
 
  for(i = 0; i < n; i++) {
    if(strcmp(names[i],name) == 0)
      return(i);
  }

  return(-1);
}

const gchar *
GGOBI(getModeName)(int which)
{ 
  int n;
  const gchar *const *names = GGOBI(getOpModeNames)(&n);
  return(names[which]);
}

gint 
GGOBI(setBrushColor)(gint cid, ggobid *gg)
{
  gint old = gg->color_id;
  if(cid > -1 && cid < gg->ncolors)
    gg->color_id = cid;

  return(old);
}

gint 
GGOBI(getBrushColor)(ggobid *gg)
{
  return(gg->color_id);
}

const gchar *const 
GGOBI(getColorName)(gint cid, ggobid *gg, gboolean inDefault)
{
  if(gg->colorNames && cid > -1 && cid < gg->ncolors) {
    return(gg->colorNames[cid]);
  }

 return(NULL);
}


gint
GGOBI(addVariable)(gdouble *vals, gint num, gchar *name, gboolean update, 
  datad *d, ggobid *gg)
{
  if (d->ncols < 1) {
    gint i;
    gchar ** rnames = (gchar **)g_malloc(sizeof(gchar*) * num);
    for (i = 0; i < num; i++) {
      rnames[i] = (gchar *) g_malloc (sizeof (gchar)*7);
      rnames[i] = g_strdup_printf ("%d", i+1);
/*    sprintf(rnames[i],"%d",i+1);*/
    }
    /* May want the final false here to be true as it causes the 
       creation of a plot. Probably not, but just mention it here
       so we don't forget.
     */
    GGOBI(setData)(vals, rnames, &name, num, 1, d, false, gg, d->input);
    for (i = 0; i < num; i++)
      g_free (rnames[i]);
    g_free (rnames);
  } else {
    gint which = d->ncols-1;
    gint *cols = (gint *) g_malloc (sizeof (gint));
    gint ncols = 1;
    if (num > d->nrows) {
      num =  d->nrows;
      /* Add a warning here. */
    }

    /*-- we have to first clone a variable and then
         overwrite its name and variables.  --*/
    cols[0] = 0;
    clone_vars (cols, ncols, d, gg);
    g_free (cols);
/*
 *  variable_clone (0, name, true, d, gg);
*/
    which++;
    GGOBI(setVariableName)(which, name, false, d, gg);
    GGOBI(setVariableValues)(which, vals, num, update, d, gg);
  }

  if (update)
    gdk_flush();

  return (d->ncols - 1);  /*-- incremented by clone_vars --*/
}

/*
  The idea of the update argument is that we can defer recomputing
  the statistics for all the variables and then the transformations.
  This is useful when we know we will be adding more variables before
  redisplaying.
  For example,
    for(i = 0; i < n; i++)
      GGOBI(setVariableValues)(i, values[i], gg->nrows, i == n-1, gg);
  causes the update to be done only for the last variable.
 */
gboolean
GGOBI(setVariableValues)(gint whichVar, gdouble *vals, gint num,
  gboolean update, datad *d, ggobid *gg)
{
  gint i;
  for (i = 0; i < num; i++) {
    d->raw.vals[i][whichVar] = d->tform.vals[i][whichVar] = vals[i];
  }

  if(update) {
    GGOBI(update_data)(d, gg);
  }


  return(true);    
}

void
GGOBI(update_data)(datad *d, ggobid *gg)
{
/*
  vartable_stats_set (d, gg);
  vartable_lim_update (d, gg);
*/
  limits_set (true, true, d);  
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
}

gint
GGOBI(removeVariable)(gchar *name, datad *d, ggobid *gg)
{
  gint which = GGOBI(getVariableIndex)(name, d, gg);
  if (which > -1 && which < d->ncols)
    return (GGOBI(removeVariableByIndex)(which, d, gg));

 return(-1);
}

gint
GGOBI(removeVariableByIndex)(gint which, datad *d, ggobid *gg)
{
  gint i, j;
  for (i = 0; i < d->nrows; i++) {
   for (j = which+1; j < d->ncols; j++) {
   }
  }

  d->ncols--;

  return (-1);
}


gint 
GGOBI(getVariableIndex)(const gchar *name, datad *d, ggobid *gg)
{
  gint i;
  for (i = 0; i < d->ncols; i++)
    if (strcmp (d->vartable[i].collab, name) == 0)
      return (i);

  return(-1);
}

void
GGOBI(setPlotRange)(double *x, double *y, int plotNum, displayd *display, gboolean pixels, ggobid *gg)
{
  splotd *sp;

  sp = GGOBI(getPlot)(display, plotNum);

  if(pixels) {
    
  } else {

    splot_zoom(sp, *x, *y, gg);
  }

 /*
  fcoords tfmin, tfmax;
  tfmin.x = x[0];
  tfmin.y = y[0];
  tfmax.x = x[1];
  tfmax.y = y[1];

  if (GTK_WIDGET_VISIBLE (display->hrule)) {
    if (((gfloat) GTK_EXT_RULER (display->hrule)->lower != tfmin.x) ||
        ((gfloat) GTK_EXT_RULER (display->hrule)->upper != tfmax.x))
    {
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->hrule),
                               (gdouble) tfmin.x, (gdouble) tfmax.x);
    }
  }

  if (GTK_WIDGET_VISIBLE (display->vrule)) {
    if (((gfloat) GTK_EXT_RULER (display->vrule)->upper != tfmin.y) ||
        ((gfloat) GTK_EXT_RULER (display->vrule)->lower != tfmax.y))
    {
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->vrule),
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
GGOBI(raiseWindow)(int which, gboolean raiseOrIcon, gboolean up, ggobid *gg)
{
  displayd *display;
  gboolean ok = false;
  int start, end, i;

  if(which < 0) {
    start = 0;
    end = g_list_length(gg->displays);
  } else {
    end = which+1;
    start = which;
  }

     for(i = start; i < end; i++) {
      display = (displayd *) g_list_nth_data(gg->displays, i);
      if(raiseOrIcon) {
        if(up) 
          gdk_window_raise(display->window->window); 
        else
          gdk_window_lower(display->window->window); 
      } else {
        if(up) 
          gtk_widget_hide_all(display->window); 
        else
          gtk_widget_show_all(display->window); 

      }
     }

     ok = true;


  gdk_flush();
  return(ok);
}

gchar *
GGOBI(getDescription)(ggobid *gg)
{
  return(g_strdup(gg->input->fileName));
}

/*
  Finds the index of the dataset named `name'
  in the specified ggobid object.
 */
int 
GGOBI(datasetIndex)(const char *name,  ggobid *gg)
{
  datad *d;
  int ctr = 0;
  GSList *tmp = gg->d;

  while(tmp) {
    d =(datad *) tmp->data;
    if(strcmp(name, d->name) == 0)
      return(ctr);
    ctr++;
    tmp = tmp->next;
  }

  return(-1);
}

/*
  Returns the names of the different datasets
  maintained in the specified ggobid object.
 */
gchar **
GGOBI(getDatasetNames)(int *n, ggobid *gg)
{
  int i;
  datad *d;
  gchar **names;
  GSList *tmp = gg->d;
  *n = g_slist_length(gg->d);
  names = (gchar **) g_malloc(sizeof(gchar *) * (*n));
  for(i = 0; i < *n ; i++) {
    d =(datad *) tmp->data;
    names[i] = g_strdup(d->name);
    tmp = tmp->next;
  }

  return(names);
}

/*
 Added to the API and to avoid breaking code (e.g. in RSggobi)
 we add it here with a new name GGOBI(ggobi_get).
*/
ggobid * 
GGOBI(ggobi_get)(gint which)
{
  return(ggobi_get(which));
}

gint
GGOBI(ncols)(datad *data)
{
 return(data->ncols);
}

gint
GGOBI(nrecords)(datad *data)
{
 return(data->nrows);
}


/*
 This is the routine one uses to register a handler for key press events
 for the numbered keys, i.e. 0, 1, ..., 9
 One can specify null values for each of these to remove the handler and have
 these events discarded.

 See notes/NumberedKeys.*, splot.c and ggobi.h also for more details 
 */
KeyEventHandler *
GGOBI(registerNumberedKeyEventHandler)(KeyEventHandlerFunc routine, void *userData, char *description, ReleaseData *releaseData, ggobid *gg, ProgrammingLanguage lang)
{
  KeyEventHandler *old = gg->NumberedKeyEventHandler;
  KeyEventHandler *newValue;
  if(routine == NULL)
    newValue = NULL;
  else {
    newValue = g_malloc(1 * sizeof(KeyEventHandler));
    newValue->handlerRoutine = routine;
    newValue->userData = userData;
    newValue->description = g_strdup(description);
    newValue->language = lang;
    newValue->releaseData = releaseData;
  }

  gg->NumberedKeyEventHandler = newValue;

  return(old);
}

KeyEventHandler *
GGOBI(removeNumberedKeyEventHandler)(ggobid *gg)
{
 return(GGOBI(registerNumberedKeyEventHandler(NULL, NULL, NULL, NULL, gg, C)));
}

#ifdef HAVE_CONFIG_H
#include "config.h"
static gchar *version_date = GGOBI_RELEASE_DATE;
int GgobiVersionNumbers[] = {MAJOR_VERSION, MINOR_VERSION, PATCH_LEVEL};
static gchar *version_string =  GGOBI_VERSION_STRING;
#else
static gchar *version_date = "October 10, 2000";
int GgobiVersionNumbers[] = {-1,-1,-1};
static gchar *version_string = "developer";
#endif

char * const 
GGOBI(getVersionDate)()
{
  return(version_date);
}

char * const 
GGOBI(getVersionString)()
{
  return(version_string);
}

int* const 
GGOBI(getVersionNumbers)()
{
  return(GgobiVersionNumbers);
}


