/* weightedvis_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

/*
 * It is my understanding that I'm supposed to use gdk_colormap_free_colors
 * to free anything I've allocated with gdk_colormap_alloc_color(s), but
 * it just isn't working to free all the wvis.color_map colors.   
 *    dfs, 10/30/2001
*/

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "colorscheme.h"

static gint xmargin = 20;
static gint ymargin = 20;

static void bin_counts_reset (gint jvar, datad *d, ggobid *gg);

GtkWidget *createColorSchemeTree(int numTypes, gchar *schemeTypes[], ggobid *gg, GtkWidget *notebook);
static void selection_made_cb (GtkWidget *clist, gint row, gint column,
  GdkEventButton *event, ggobid *gg);
static void entry_set_scheme_name (ggobid *gg);

/*--------------------------------------------------------------------*/
/*      Notebook containing the variable list for each datad          */
/*--------------------------------------------------------------------*/

/*
 * Apparently I have to override these functions from utils_ui.c
 * so that I can add a signal appropriately to the new page in
 * the notebook.
 * Only one line is different:
    GtkSignalFunc func = selection_made_cb;
*/

static void 
wvis_variable_notebook_adddata_cb (ggobid *gg, datad *d, void *notebook)
{
  GtkWidget *swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0);
  if (swin) {
    GtkWidget *clist;
    GtkSelectionMode mode = GTK_SELECTION_SINGLE;
    GtkSignalFunc func = (GtkSignalFunc) selection_made_cb;
    clist = GTK_BIN (swin)->child;
    if (clist) {
      mode = GTK_CLIST(clist)->selection_mode;
      /*
       * should also be possible to retrieve the signal function that
       * responds to "select_row" signal
      */
    }
    variable_notebook_subwindow_add (d, func, GTK_WIDGET(notebook),
      all_vartypes, all_datatypes, gg);
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook),
                                g_slist_length (gg->d) > 1);
  }
}

CHECK_EVENT_SIGNATURE(wvis_variable_notebook_adddata_cb, datad_added_f)
CHECK_EVENT_SIGNATURE(variable_notebook_list_changed_cb, variable_list_changed_f)

GtkWidget *
wvis_create_variable_notebook (GtkWidget *box, GtkSelectionMode mode, 
  GtkSignalFunc func, ggobid *gg)
{
  GtkWidget *notebook;
  gint nd = g_slist_length (gg->d);
  GSList *l;
  datad *d;

  /* Create a notebook, set the position of the tabs */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), nd > 1);
  gtk_box_pack_start (GTK_BOX (box), notebook, true, true, 2);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (g_slist_length (d->vartable)) {
      variable_notebook_subwindow_add (d, func, notebook,
        all_vartypes, all_datatypes, gg);
    }
  }

  /*-- listen for variable_added and _list_changed events on main_window --*/
  /*-- ... list_changed would be adequate --*/
  gtk_signal_connect (GTK_OBJECT (gg),
    "variable_added",
     GTK_SIGNAL_FUNC (variable_notebook_varchange_cb),
     GTK_OBJECT (notebook));
  gtk_signal_connect (GTK_OBJECT (gg),
    "variable_list_changed",
     GTK_SIGNAL_FUNC (variable_notebook_list_changed_cb),
     GTK_OBJECT (notebook));

  /*-- listen for variable_added events on main_window --*/
  gtk_signal_connect (GTK_OBJECT (gg),
    "datad_added", GTK_SIGNAL_FUNC (wvis_variable_notebook_adddata_cb),
     GTK_OBJECT (notebook));

  return notebook;
}

/*--------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/*             Using colorschemed objects                            */
/*-------------------------------------------------------------------*/

void
colorscheme_set_cb (GtkWidget *w, colorschemed* scheme)
{
  ggobid *gg = GGobiFromWidget (GTK_WIDGET(w), true);
  gboolean rval = false;
  GtkWidget *clist;
  datad *d;
  gint selected_var;

/*
 * gg->wvis sometimes has its own scheme, and then we'll use it.
 * If it's null, we use gg->activeColorScheme.  We update
 * gg->activeColorScheme to the value of scheme when the user asks.
*/

  if (scheme) {
    gg->wvis.scheme = scheme;
    entry_set_scheme_name (gg);
    colorscheme_init (scheme);
  }

/*-- delete this line once debugging is complete --*/
  displays_plot (NULL, FULL, gg);

  /*-- rebuild the drawing area in this window --*/
/*
 * This is using two expose events, which is odd:  it's something
 * to do with getting the numbers of points in each bin to appear,
 * and there's probably a way to do it better.
*/
  clist = get_clist_from_object (GTK_OBJECT (w));
  if(clist == NULL) {
      d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
      selected_var = get_one_selection_from_clist (clist, d);
  } else {
      d = (datad *) g_slist_nth_data(gg->d, 0);
      selected_var = 0;
  }
  if (d && selected_var != -1) {
      gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
        (gpointer) gg, (gpointer) &rval);
  }
  bin_counts_reset (selected_var, d, gg);
  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);
}


/*-------------------------------------------------------------------------*/

static void
bin_counts_reset (gint jvar, datad *d, ggobid *gg)
{
  gint i, k, m;
  gfloat val;
  vartabled *vt;
  gfloat min, max;
  colorschemed *scheme = (gg->wvis.scheme != NULL) ?
    gg->wvis.scheme : gg->activeColorScheme;

  if (jvar == -1)
    return;

  vt = vartable_element_get (jvar, d);
  min = vt->lim_tform.min;
  max = vt->lim_tform.max;

  for (k=0; k<gg->wvis.npct; k++)
    gg->wvis.n[k] = 0;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    for (k=0; k<scheme->n; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      if (d->tform.vals[i][jvar] <= val) {
        gg->wvis.n[k]++;
        break;
      }
    }
  }

}

static void
record_colors_reset (gint selected_var, datad *d, ggobid *gg)
{
  gint i, k, m;
  vartabled *vt;
  gfloat min;
  gfloat max;
  gfloat val;
  colorschemed *scheme;

  if(selected_var < 0)
      return;

  vt = vartable_element_get (selected_var, d);
  scheme = gg->activeColorScheme;

  min = vt->lim_tform.min;
  max = vt->lim_tform.max;
  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    for (k=0; k<scheme->n; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      if (d->tform.vals[i][selected_var] <= val) {
        d->color.els[i] = d->color_now.els[i] = k;
        break;
      }
    }
  }
}


/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, ggobid *gg) {
  gtk_widget_hide (gg->wvis.window);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->wvis.window);
}

/*
 * Use the horizontal position of the mouse to move the nearest
 * boundary
*/
static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, ggobid *gg)
{
  GdkModifierType state;
  icoords pos;
  gboolean rval = false;
  gfloat val;

  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist, d);

  icoords *mousepos = &gg->wvis.mousepos;
  gint color = gg->wvis.nearest_color;

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  if (pos.x != mousepos->x) {
    val = (gfloat) (pos.x - xmargin) /
          (gfloat) (w->allocation.width - 2*xmargin);

    /*-- don't allow it to cross its neighbors' boundaries --*/
    if ((color == 0 && val <= gg->wvis.pct[color+1]) ||
        (color == gg->wvis.npct-1 && val >= gg->wvis.pct[color+1]) ||
        (val >= gg->wvis.pct[color-1] && val <= gg->wvis.pct[color+1]))
    {
      gg->wvis.pct[color] = val;

      if (selected_var != -1 && selected_var < d->ncols)
        bin_counts_reset (selected_var, d, gg);

      gtk_signal_emit_by_name (GTK_OBJECT (w), "expose_event",
        (gpointer) gg, (gpointer) &rval);

      if (gg->wvis.update_method == WVIS_UPDATE_CONTINUOUSLY) {
        record_colors_reset (selected_var, d, gg);
        clusters_set (d, gg);
        displays_plot (NULL, FULL, gg);
      }
    }
  }

  mousepos->x = pos.x;  

  return true;
}

/*-- when the button is pressed, listen for motion notify events --*/
static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  GdkModifierType state;
  icoords pos;
  gint k, x, y, nearest = -1, d;
  gint dist = w->allocation.width*w->allocation.width +
              w->allocation.height*w->allocation.height;
  colorschemed *scheme = gg->activeColorScheme;

  gfloat *pct = gg->wvis.pct;
  gint *nearest_color = &gg->wvis.nearest_color;
  gint hgt;

  if (gg->wvis.scheme != NULL)
    scheme = gg->wvis.scheme;

  hgt = (w->allocation.height - 2*ymargin) / (scheme->n - 1);

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  /*-- find nearest slider --*/
  y = ymargin + 10;
  for (k=0; k<scheme->n - 1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
    d = (pos.x-x)*(pos.x-x) + (pos.y-y)*(pos.y-y);
    if (d < 100 && d < dist) {
      nearest = k;
      dist = d;
    }
    y += hgt;
  }

  *nearest_color = nearest;

  if (*nearest_color != -1) {
    gg->wvis.motion_notify_id = gtk_signal_connect (GTK_OBJECT (w),
      "motion_notify_event", (GtkSignalFunc) motion_notify_cb, (gpointer) gg);
  }

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist, d);

  if (gg->wvis.motion_notify_id) {
    gtk_signal_disconnect (GTK_OBJECT (w), gg->wvis.motion_notify_id);
    gg->wvis.motion_notify_id = 0;
  }

  if (selected_var >= 0 && selected_var <= d->ncols) {
    record_colors_reset (selected_var, d, gg);
    clusters_set (d, gg);
    displays_plot (NULL, FULL, gg);
  }

  return true;
}

static gint
da_configure_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  /*-- Create new backing pixmaps of the appropriate size --*/
  if (gg->wvis.pix != NULL)
    gdk_pixmap_unref (gg->wvis.pix);
  gg->wvis.pix = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  gtk_widget_queue_draw (w);

  return false;
}

/*
 * Set the bin boundaries (the values of wvis.pct[]) in one
 * of two ways:  simply dividing the range of the data into
 * scheme->n equal-sized pieces, or attempting to set the regions
 * such that they contain equal numbers of points.
*/ 
static void
bin_boundaries_set (gint selected_var, datad *d, ggobid *gg)
{
  gint k;

  if (gg->wvis.binning_method == WVIS_EQUAL_WIDTH_BINS || selected_var == -1) {
    /*
     * These numbers are the upper boundaries of each interval.
     * By default, they start at .1 and end at 1.0.
    */
    for (k=0; k<gg->wvis.npct; k++) {
      gg->wvis.pct[k] = (gfloat) (k+1) /  (gfloat) gg->wvis.npct;
      gg->wvis.n[k] = 0;
    }
  } else if (gg->wvis.binning_method == WVIS_EQUAL_COUNT_BINS) {
    gint i, m;
    gfloat min, max, range, midpt;
    vartabled *vt = vartable_element_get (selected_var, d);
    gint ngroups = gg->wvis.npct;
    gint groupsize = (gint) (d->nrows_in_plot / ngroups);
    paird *pairs = (paird *) g_malloc (d->nrows_in_plot * sizeof (paird));
    guint varno = g_slist_index (d->vartable, vt);

    min = vt->lim_tform.min;
    max = vt->lim_tform.max;
    range = max - min;

    /*-- sort the selected variable --*/
    for (i=0; i<d->nrows_in_plot; i++) {
      pairs[i].f = d->raw.vals[d->rows_in_plot[i]][varno];
      pairs[i].indx = d->rows_in_plot[i];
    }
    qsort ((gchar *) pairs, d->nrows_in_plot, sizeof (paird), pcompare);

    /*
     * determine the boundaries that will result in equal-sized
     * groups (as well as the data permits)
     *
     * This seems to do the right thing except in one case:  the
     * variable is categorical with the number of categories less
     * than the number of groups, and the categories are not of
     * equal size.  It does something legal, but not ideal, since
     * the number of groups it identifies may be too small.
    */
    /*-- initialize the boundaries --*/
    for (k=0; k<ngroups; k++)
      gg->wvis.pct[k] = 1.0;
    i = 0;
    for (k=0; k<ngroups-1; k++) {  /*-- no need to figure out the last one --*/
      m = 0;
      while (m < groupsize || i == 0 || pairs[i].f == pairs[i-1].f) {
        m++;
        i++;
        if (i == d->nrows_in_plot-1) 
          break;
      }
      midpt = (i == d->nrows_in_plot - 1) ?
        max : pairs[i].f + (pairs[i+1].f - pairs[i].f) / 2 ;
      gg->wvis.pct[k] = (midpt - min) / range;
      if (i == d->nrows_in_plot-1) 
        break;
    }

    g_free (pairs);
  }
}

static void binning_method_cb (GtkWidget *w, gpointer cbd)
{
  gboolean rval = false;
  ggobid *gg = GGobiFromWidget (w, true);
  gg->wvis.binning_method = GPOINTER_TO_INT (cbd);

  gg->wvis.npct = 0;  /*-- force bin_boundaries_set to be called --*/
  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

static void update_method_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gg->wvis.update_method = GPOINTER_TO_INT (cbd);
}

static void
da_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  gint height = w->allocation.height - 2*ymargin;
  gint x0, x1, k, hgt;
  gint x = xmargin;
  gint y = ymargin;
  GdkPoint *points;
  gfloat diff;
  vartabled *vt;
  colorschemed *scheme = (gg->wvis.scheme != NULL) ?
    gg->wvis.scheme : gg->activeColorScheme;

  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist, d);

  GtkWidget *da = gg->wvis.da;
  GdkPixmap *pix = gg->wvis.pix;

  if (gg->wvis.GC == NULL)
    gg->wvis.GC = gdk_gc_new (w->window);

  hgt = height / (scheme->n - 1);

  if (gg->wvis.npct != scheme->n) {
    gg->wvis.npct = scheme->n;
    gg->wvis.pct = (gfloat *) g_realloc (gg->wvis.pct,
                                         gg->wvis.npct * sizeof (gfloat));
    gg->wvis.n = (gint *) g_realloc (gg->wvis.n,
                                     gg->wvis.npct * sizeof (gint));
    bin_boundaries_set (selected_var, d, gg);
    bin_counts_reset (selected_var, d, gg);
  }

  /*-- clear the pixmap --*/
  gdk_gc_set_foreground (gg->wvis.GC, &scheme->rgb_bg);
  gdk_draw_rectangle (pix, gg->wvis.GC, TRUE,
                      0, 0, w->allocation.width, w->allocation.height);


  /*-- draw the color bars --*/
  x0 = xmargin;
  for (k=0; k<scheme->n; k++) {
    x1 = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    gdk_gc_set_foreground (gg->wvis.GC, &scheme->rgb[k]);
    gdk_draw_rectangle (pix, gg->wvis.GC,
                        TRUE, x0, ymargin, x1 - x0, height);
    x0 = x1;
  }

  /*-- draw the horizontal lines --*/
  x0 = xmargin; y = ymargin + 10;
  x1 = xmargin + (w->allocation.width - 2*xmargin) - 1;
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray2);
  for (k=0; k<scheme->n-1; k++) {
    gdk_draw_line (pix, gg->wvis.GC, x0, y, x1, y);
    y += hgt;
  }

  /*-- draw rectangles, 20 x 10 for the moment --*/
  y = ymargin + 10;
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray2);
  for (k=0; k<scheme->n-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    gdk_draw_rectangle (pix, gg->wvis.GC,
                        TRUE, x-10, y-5, 20, 10);
    y += hgt;
  }


  /*-- draw the dark shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray1);
  for (k=0; k<scheme->n-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);

    points [0].x = x - 10;
    points [0].y = y + 5;
    points [1].x = x + 10;
    points [1].y = y + 5;
    points [2].x = x + 10;
    points [2].y = y - 5;

    points [3].x = points[2].x - 1;
    points [3].y = points[2].y + 1;
    points [4].x = points[1].x - 1;
    points [4].y = points[1].y - 1;
    points [5].x = points[0].x + 1;
    points [5].y = points[0].y - 1;

    points [6].x = x - 10;
    points [6].y = y + 5;
    gdk_draw_polygon (pix, gg->wvis.GC,
                      TRUE, points, 7);
    gdk_draw_line (pix, gg->wvis.GC, x-1, y-4, x-1, y+3);

    y += hgt;
  }
  g_free (points);

  /*-- draw the light shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray3);
  for (k=0; k<scheme->n-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);

    points [0].x = x - 10;  /*-- lower left --*/
    points [0].y = y + 4;
    points [1].x = x - 10;  /*-- upper left --*/
    points [1].y = y - 5;
    points [2].x = x + 9;  /*-- upper right --*/
    points [2].y = y - 5;

    points [3].x = points[2].x - 1;
    points [3].y = points[2].y + 1;
    points [4].x = points[1].x + 1;
    points [4].y = points[1].y + 1;
    points [5].x = points[0].x + 1;
    points [5].y = points[0].y - 1;

    points [6].x = points[0].x;
    points [6].y = points[0].y;
    gdk_draw_polygon (pix, gg->wvis.GC,
                      TRUE, points, 7);
    gdk_draw_line (pix, gg->wvis.GC, x, y-4, x, y+3);

    y += hgt;
  }
  g_free (points);

  /*-- add the variable limits in the top margin --*/
  if (d && selected_var != -1) {
    gfloat min, max;
    gfloat val;
    gchar *str;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style = gtk_widget_get_style (da);

    vt = vartable_element_get (selected_var, d);
    if (vt) {
      min = vt->lim_tform.min;
      max = vt->lim_tform.max;

      gdk_gc_set_foreground (gg->wvis.GC, &scheme->rgb_accent);
      y = ymargin;
      for (k=0; k<scheme->n-1; k++) {

        val = min + gg->wvis.pct[k] * (max - min);
        str = g_strdup_printf ("%3.3g", val);
        gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
          gtk_style_get_font (style),
#else
          style->font,
#endif
          str, strlen(str),
          &lbearing, &rbearing, &width, &ascent, &descent);
        x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
        gdk_draw_string (pix,
#if GTK_MAJOR_VERSION == 2
          gtk_style_get_font (style),
#else
          style->font,
#endif
          gg->wvis.GC,
          x - width/2,
          y - 2,
          str);
        g_free (str);
      }

      /*-- ... and the counts in the bottom margin --*/
      for (k=0; k<scheme->n; k++) {
        val = min + gg->wvis.pct[k] * (max - min);
        str = g_strdup_printf ("%d", gg->wvis.n[k]);
        gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
          gtk_style_get_font (style),
#else
          style->font,
#endif
          str, strlen(str),
          &lbearing, &rbearing, &width, &ascent, &descent);
        x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
        diff = (k == 0) ? gg->wvis.pct[k] : gg->wvis.pct[k]-gg->wvis.pct[k-1]; 
        x -= diff/2 * (w->allocation.width - 2*xmargin);
        gdk_draw_string (pix,
#if GTK_MAJOR_VERSION == 2
          gtk_style_get_font (style),
#else
          style->font,
#endif
          gg->wvis.GC,
          x - width/2,
          (w->allocation.height - ymargin) + ascent + descent + 2,
          str);
        g_free (str);
      }
    }
  }

  gdk_draw_pixmap (w->window, gg->wvis.GC, pix,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

void
selection_made_cb (GtkWidget *clist, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gboolean rval = false;
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  GtkWidget *btn;

  bin_boundaries_set (row, d, gg);  /*-- in case the method changed --*/
  bin_counts_reset (row, d, gg);
  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);

  /*-- get the apply button, make it sensitive --*/
  btn = widget_find_by_name (gg->wvis.window, "WVIS:apply");
  if (btn)
    gtk_widget_set_sensitive (btn, true);
}

/*
 * Find out whether it's possible to use the new scheme
 * without losing brushing information.  If so, go ahead
 * and change index values if that's required
 *
 * If force is true, remap even if the number of colors
 * is too large.
*/
/*-- move this to color.c --*/
gboolean colors_remap (colorschemed *scheme, gboolean force, ggobid *gg)
{
  gint i, k;
  gboolean all_colors_p[MAXNCOLORS];
  GSList *l;
  datad *d;
  gushort colors_used[MAXNCOLORS];
  gint maxcolorid, ncolors_used;
  gboolean remap_ok = true;

  for (k=0; k<MAXNCOLORS; k++)
    all_colors_p[k] = false;

  /*-- find out all the colors (indices) are currently in use --*/
  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    datad_colors_used_get (&ncolors_used, colors_used, d, gg);
    for (k=0; k<ncolors_used; k++)
      all_colors_p[colors_used[k]] = true;
  }
 
  /*-- find out how many colors are currently in use --*/
  ncolors_used = 0;
  for (k=0; k<MAXNCOLORS; k++)
    if (all_colors_p[k])
      ncolors_used++;

  /*-- find the largest color index currently in use --*/
  maxcolorid = -1;
  for (k=MAXNCOLORS-1; k>0; k--) {
    if (all_colors_p[k]) {
      maxcolorid = k;
      break;
    }
  }

  if (maxcolorid < scheme->n)
    /* no problem, go right ahead */
    ;
  else if (!force && ncolors_used > scheme->n) {

    /* fatal: bail out with a warning */
    quick_message ("The number of colors now in use is greater than than\nthe number of colors in the chosen color scheme.",
      false);

    remap_ok = false;   
  } else if (maxcolorid >= scheme->n) {
    /*-- build the vector that will be used to reset the current indices --*/
    gint *newind = (gint *) g_malloc ((maxcolorid+1) * sizeof (gint));
    gint n = 0;

    for (k=0; k<=maxcolorid; k++) {
      if (all_colors_p[k]) {
        newind[k] = n;

        /*
         * try to achieve a decent spread of the color values,
         * which is helpful in most color maps
        */
        n+= ((scheme->n+1)/ncolors_used);
        /*-- make sure we haven't gone too far --*/
        if (n >= scheme->n-1)  n = scheme->n-1;

      }
    }

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      for (i=0; i<d->nrows; i++) {
        d->color.els[i] = newind[ d->color.els[i] ];
        d->color_now.els[i] = newind[ d->color_now.els[i] ];
        /*-- what about color_prev?  --*/
      }
    }
    g_free (newind);

  } else {
    g_printerr ("nothing else should possibly happen, no?\n");
  }

  return remap_ok;
}

static void scale_set_cb (GtkWidget *w, ggobid* gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gboolean rval = false;

  /*
   * If we've been using gg->wvis.scheme, set gg->activeColorScheme
   * to the current scheme.
  */
  if (gg->wvis.scheme) {
    colorschemed *scheme = gg->wvis.scheme;

    /*-- if no current color index is too high, continue --*/
    if (!colors_remap (scheme, false, gg))
      return;

    gg->activeColorScheme = scheme;
    gg->wvis.scheme = NULL;
  }

  displays_plot (NULL, FULL, gg);
  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);

  entry_set_scheme_name (gg);

  symbol_window_redraw (gg);
  cluster_table_update (d, gg);
}

static void scale_apply_cb (GtkWidget *w, ggobid* gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist, d);
  vartabled *vt;
  colorschemed *scheme = (gg->wvis.scheme != NULL) ?
    gg->wvis.scheme : gg->activeColorScheme;

  if (d && selected_var != -1) {
    gint i, m, k;
    gfloat min, max;
    gfloat val;
    gboolean rval = false;

    /*
     * If we've been using gg->wvis.scheme, set gg->activeColorScheme
     * to this scheme.
    */
    if (gg->wvis.scheme)
      gg->activeColorScheme = gg->wvis.scheme;

    vt = vartable_element_get (selected_var, d);
    min = vt->lim_tform.min;
    max = vt->lim_tform.max;

    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];

      for (k=0; k<scheme->n; k++) {
        val = min + gg->wvis.pct[k] * (max - min);
        if (d->tform.vals[i][selected_var] <= val) {
          d->color.els[i] = d->color_now.els[i] = k;
          break;
        }
      }
    }
    clusters_set (d, gg);

    /*-- before calling displays_plot, reset brushing color if needed --*/
    if (gg->color_id >= scheme->n) gg->color_id = scheme->n - 1;

    displays_plot (NULL, FULL, gg);

    bin_counts_reset (selected_var, d, gg);
    gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
      (gpointer) gg, (gpointer) &rval);

    symbol_window_redraw (gg);
    cluster_table_update (d, gg);
  }
}

static void
entry_set_scheme_name (ggobid *gg)
{
  gtk_entry_set_text (GTK_ENTRY (gg->wvis.entry_preview),
    (gg->wvis.scheme != NULL) ? gg->wvis.scheme->name :
                                gg->activeColorScheme->name);

  gtk_entry_set_text (GTK_ENTRY (gg->wvis.entry_applied),
                                gg->activeColorScheme->name);
}

void
wvis_window_open (ggobid *gg) 
{
  GtkWidget *vbox;
  GtkWidget *frame1, *vb1, *hb;
  GtkWidget *notebook = NULL;
  GtkWidget *btn, *vb, *opt, *label;

  /*-- for colorscales --*/
  GtkWidget *hpane, *tr, *sw;
  GtkWidget *frame2, *vbs;
  static gchar *colorscaletype_lbl[UNKNOWN_COLOR_TYPE] = {
    "DIVERGING",
    "SEQUENTIAL",
    "SPECTRAL",
    "QUALITATIVE"};
  static gchar *const binning_method_lbl[] = {
    "Constant bin width",
    "Constant bin count (approx)"};
  static gchar *const update_method_lbl[] = {
    "Update on mouse release",
    "Update continuously"};

  if (gg->d == NULL || g_slist_length (gg->d) == 0)
    return;

  if (gg->wvis.window == NULL) {

    gg->wvis.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->wvis.window),
      "color schemes");
    gtk_signal_connect (GTK_OBJECT (gg->wvis.window),
      "delete_event", GTK_SIGNAL_FUNC (close_wmgr_cb), gg);

    hpane = gtk_hpaned_new();
    gtk_container_add (GTK_CONTAINER (gg->wvis.window), hpane);

    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add (GTK_CONTAINER (hpane), sw);

    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_box_set_spacing (GTK_BOX(vbox), 5);

    gtk_container_add (GTK_CONTAINER (hpane), vbox);    

/*
 * First the frame for the section on choosing new colormaps,
 * then the graphics, and finally the section on applying
 * the colorscheme by variable -- does this make sense?
 *
 * The awkwardness in sequencing here is that the notebook
 * has to be defined early because it's used by the color
 * scheme section, but I prefer to show it later.
*/

  /*
   * preview of section on choosing new colormap: place the frame
  */
    frame2 = gtk_frame_new ("Choose color scheme");
    gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame2, false, false, 1);

    /*-- preview of section on colors, symbols --*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

    frame1 = gtk_frame_new ("Apply color scheme by variable");
    gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame1, true, true, 1);

/*  */
    vb1 = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vb1), 5);
    gtk_container_add (GTK_CONTAINER (frame1), vb1);

    /* Create a notebook, set the position of the tabs */
    notebook = wvis_create_variable_notebook (vb1, GTK_SELECTION_SINGLE,
      (GtkSignalFunc) selection_made_cb, gg);

    tr = createColorSchemeTree(UNKNOWN_COLOR_TYPE, colorscaletype_lbl,
      gg, notebook);
    gtk_widget_set_usize(sw, 200, 20);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), tr);

    /*-- Insert an option menu for choosing the method of binning --*/
    opt = gtk_option_menu_new ();
    gtk_widget_set_name (opt, "WVIS:binning_method");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Select a binning method", NULL);
    gtk_box_pack_start (GTK_BOX (vb1), opt,
      false, false, 0);
    populate_option_menu (opt, (gchar**) binning_method_lbl,
      sizeof (binning_method_lbl) / sizeof (gchar *),
      (GtkSignalFunc) binning_method_cb, "GGobi", gg);

  /*
   * section on choosing new colormap
  */

    vbs = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbs), 5);
    gtk_container_add (GTK_CONTAINER (frame2), vbs);

#if 0
    opt = gtk_option_menu_new ();
    menu = gtk_menu_new ();

    colorscheme_add_to_menu (menu, "Default", NULL,
      (GtkSignalFunc) colorscheme_set_cb, notebook, gg);
 
    for (n=0; n<ncolorscaletype_lbl; n++, i++) {
      colorscheme_add_to_menu (menu, colorscaletype_lbl[n], NULL,
       NULL, notebook, gg);
      for (l = gg->colorSchemes; l; l = l->next, i++) {
        scheme = (colorschemed *) l->data;
        if (scheme->type == n) {
          colorscheme_add_to_menu (menu, scheme->name, scheme,
            (GtkSignalFunc) colorscheme_set_cb, notebook, gg);
  if(strcmp(scheme->name, gg->activeColorScheme->name) == 0) {
/*XX Fix this - off by some quantity because Debby didn't use trees. :-) */
      currentSelection = i + 2;
  }
}
      }
    }

    gtk_option_menu_set_menu (GTK_OPTION_MENU (opt), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU (opt), currentSelection);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Choose a color scale", NULL);

    gtk_box_pack_start (GTK_BOX (vbs), opt, false, false, 1);
#endif

    hb = gtk_hbox_new (true, 0);
    gtk_box_pack_start (GTK_BOX (vbs), hb, true, true, 5);
    label = gtk_label_new ("Color scheme (preview)");
    gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
    gtk_box_pack_start (GTK_BOX (hb), label, true, true, 0);
    gg->wvis.entry_preview = gtk_entry_new();
    gtk_entry_set_editable (GTK_ENTRY (gg->wvis.entry_preview), false);
    /*entry_set_scheme_name (gg);*/
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->wvis.entry_preview,
      "The name of the color scheme whose colors are displayed below.  This may be the currently active color scheme, or the scheme you're previewing using the tree to the left left.",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), gg->wvis.entry_preview, true, true, 0);

    btn = gtk_button_new_with_label ("Apply color scheme to brushing colors");
    gtk_object_set_data (GTK_OBJECT (btn), "notebook", notebook);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Make this the current color scheme for brushing in ggobi, preserving current color groups.  If the number of colors in the new scheme is less than the number of colors currently in use, this won't work.",
      NULL);
    gtk_box_pack_start (GTK_BOX (vbs), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_set_cb), gg);
    gtk_widget_set_name (btn, "WVIS:setcolorscheme");

    hb = gtk_hbox_new (true, 0);
    gtk_box_pack_start (GTK_BOX (vbs), hb, true, true, 5);
    label = gtk_label_new ("Color scheme (applied)");
    gtk_misc_set_alignment (GTK_MISC(label), 0, .5);
    gtk_box_pack_start (GTK_BOX (hb), label, true, true, 0);
    gg->wvis.entry_applied = gtk_entry_new();
    gtk_entry_set_editable (GTK_ENTRY (gg->wvis.entry_applied), false);
    entry_set_scheme_name (gg);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->wvis.entry_applied,
      "The name of the currently active color scheme.",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), gg->wvis.entry_applied, true, true, 0);
  /**/

    /*-- colors, symbols --*/
    /*-- now we get fancy:  draw the scale, with glyphs and colors --*/
/*
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);
*/
    gg->wvis.da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->wvis.da), 400, 200);
    gtk_object_set_data (GTK_OBJECT (gg->wvis.da), "notebook", notebook);
    gtk_box_pack_start (GTK_BOX (vb), gg->wvis.da, false, false, 0);

    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "configure_event",
                        (GtkSignalFunc) da_configure_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "expose_event",
                        (GtkSignalFunc) da_expose_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "button_press_event",
                        (GtkSignalFunc) button_press_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->wvis.da),
                        "button_release_event",
                        (GtkSignalFunc) button_release_cb,
                        (gpointer) gg);

    gtk_widget_set_events (gg->wvis.da, GDK_EXPOSURE_MASK
               | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
               | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

    /*-- add an hbox to hold a button and a menu --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb1), hb, false, false, 0);

    btn = gtk_button_new_with_label ("Apply");
    gtk_object_set_data (GTK_OBJECT (btn), "notebook", notebook);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Apply the color scale, using the values of the variable selected in the list above",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 0);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_apply_cb), gg);
    gtk_widget_set_name (btn, "WVIS:apply");
    gtk_widget_set_sensitive (btn, false);

    /*-- option menu for choosing the method of updating --*/
    opt = gtk_option_menu_new ();
    gtk_widget_set_name (opt, "WVIS:update_method");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "How to update the displays in response to movements of the sliders (works after a color scheme has been applied by variable)",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), opt, true, true, 0);
    populate_option_menu (opt, (gchar**) update_method_lbl,
      sizeof (update_method_lbl) / sizeof (gchar *),
      (GtkSignalFunc) update_method_cb, "GGobi", gg);

    /*-- add a close button --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
      false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_with_label ("Close");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 2);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (close_btn_cb), gg);
  }

  gtk_widget_show_all (gg->wvis.window);
  gdk_window_raise (gg->wvis.window->window);
}

GtkWidget *createSchemeColorsTree(colorschemed *scheme);

/**
 Create the tree displaying the colorscheme information.
 This displays the different levels:
    types of schemes, 
    different schemes within each type, 
    colors within each scheme.
 
 */
GtkWidget *
createColorSchemeTree(gint numTypes, gchar *schemeTypes[], ggobid *gg,
  GtkWidget *notebook)
{
  GtkWidget *item;
  GtkWidget **trees, *top;
  /*GtkWidget *tree;*/
  gint n;
  GList *l;
  colorschemed *scheme;

  top = gtk_tree_new();
  trees = (GtkWidget **) g_malloc(sizeof(GtkTree*) * numTypes);

  for (n = 0; n < numTypes; n++) {
    trees[n] = gtk_tree_new();
    item = gtk_tree_item_new_with_label(schemeTypes[n]);
    gtk_tree_append(GTK_TREE(top), item);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), trees[n]);
  }

  for(l = gg->colorSchemes; l ; l = l->next) {
   scheme = (colorschemed *) l->data;
   item = gtk_tree_item_new_with_label(scheme->name);
   GGobi_widget_set(item, gg, true);

   gtk_object_set_data (GTK_OBJECT (item), "notebook", notebook);
   gtk_signal_connect(GTK_OBJECT(item), "select",
     (GtkSignalFunc) colorscheme_set_cb, scheme);
   gtk_tree_append(GTK_TREE(trees[scheme->type]), item);
   gtk_widget_show(item);
/*
  Have to read the names properly first.
    tree = createSchemeColorsTree(scheme);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), tree);      
*/
  }
  gtk_widget_show_all(top);

  return(top);
}

GtkWidget *
createSchemeColorsTree(colorschemed *scheme)
{
  GtkWidget *tree, *item;
  gchar *name;
  gint n, i;

  n = scheme->n;
  tree = gtk_tree_new();
  for(i = 0; i < n; i++) {
    name = g_array_index(scheme->colorNames, gchar *, i);
    if(!name)
      name = "missing color name";
    item = gtk_tree_item_new_with_label(name);
    gtk_tree_append(GTK_TREE(tree), item);
    gtk_widget_show(item);
  }

  return(tree);
}
