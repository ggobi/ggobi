/* wvis_ui.c */
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

/*
 * Something here should respond to a variable transformation event,
 * I think.  We don't yet have such an event, so I won't spend the 
 * time now to work out what the response should be.  dfs 9/10/2002
*/

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "colorscheme.h"

static gint xmargin = 20;
static gint ymargin = 20;

static void bin_counts_reset (gint jvar, GGobiData *d, ggobid *gg);
static void selection_made_cb (GtkTreeSelection *tree_sel, ggobid *gg);

/*----------------------------------------------------------------*/
/*      Notebook containing the variable list for each datad      */
/*----------------------------------------------------------------*/

/*
 * Apparently I have to override these functions from utils_ui.c
 * so that I can add a signal appropriately to the new page in
 * the notebook.
 * Only one line is different:
    GtkSignalFunc func = selection_made_cb;
*/

static void 
wvis_variable_notebook_adddata_cb (ggobid *gg, GGobiData *d, void *notebook)
{
  GtkWidget *swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), 0);
  if (swin) {
    GtkWidget *tree_view;
    GtkSelectionMode mode = GTK_SELECTION_SINGLE;
    GCallback func = G_CALLBACK(selection_made_cb);

    tree_view = GTK_BIN (swin)->child;
    if (tree_view) {
      GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
      mode = gtk_tree_selection_get_mode(tree_sel);
      /* um is something supposed to happen here? mfl */
      /*
       * should also be possible to retrieve the signal function that
       * responds to "select_row" signal
      */
    }

    variable_notebook_subwindow_add (d, func, NULL, GTK_WIDGET(notebook),
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
  GGobiData *d;

  /* Create a notebook, set the position of the tabs */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), nd > 1);
  gtk_box_pack_start (GTK_BOX (box), notebook, true, true, 2);
  g_object_set_data(G_OBJECT(notebook), "SELECTION", (gpointer) mode);
  g_object_set_data(G_OBJECT(notebook), "selection-func", func);
  g_object_set_data(G_OBJECT(notebook), "selection-func-data", NULL);
  g_object_set_data(G_OBJECT(notebook), "vartype", (gpointer) all_vartypes);
  g_object_set_data(G_OBJECT(notebook), "datatype", (gpointer) all_datatypes);

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (g_slist_length (d->vartable)) {
      variable_notebook_subwindow_add (d, func, NULL, notebook,
        all_vartypes, all_datatypes, gg);
    }
  }

  /*-- listen for variable_added and _list_changed events on main_window --*/
  /*-- ... list_changed would be adequate --*/
  g_signal_connect (G_OBJECT (gg),
    "varcircle_label_set",
     G_CALLBACK (variable_notebook_varchange_cb),
     GTK_OBJECT (notebook));
  g_signal_connect (G_OBJECT (gg),
    "variable_list_changed",
     G_CALLBACK (variable_notebook_list_changed_cb),
     GTK_OBJECT (notebook));

  /*-- listen for variable_added events on main_window --*/
  g_signal_connect (G_OBJECT (gg),
    "datad_added", G_CALLBACK (wvis_variable_notebook_adddata_cb),
     GTK_OBJECT (notebook));

  return notebook;
}

/*-------------------------------------------------------------------------*/

static void
bin_counts_reset (gint jvar, GGobiData *d, ggobid *gg)
{
  gint i, k, m;
  gfloat val;
  vartabled *vt;
  gfloat min, max;
  colorschemed *scheme = gg->activeColorScheme;

  if (jvar == -1)
    return;

  vt = vartable_element_get (jvar, d);
  min = vt->lim_tform.min;
  max = vt->lim_tform.max;

  for (k=0; k<gg->wvis.npct; k++)
    gg->wvis.n[k] = 0;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
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
record_colors_reset (gint selected_var, GGobiData *d, ggobid *gg)
{
  gint i, k, m;
  gint nd = g_slist_length(gg->d);
  vartabled *vt;
  gfloat min, max, val;
  colorschemed *scheme = gg->activeColorScheme;

  if (selected_var < 0)
    return;

  vt = vartable_element_get (selected_var, d);
  min = vt->lim_tform.min;
  max = vt->lim_tform.max;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    for (k=0; k<scheme->n; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      if (d->tform.vals[i][selected_var] <= val) {
        d->color.els[i] = d->color_now.els[i] = k;
        break;
      }
    }
    if (nd > 1 && !gg->linkby_cv)
      symbol_link_by_id (true, i, d, gg);  /*-- true = force persistent --*/
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

  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT (w));
  GGobiData *d = NULL;
  gint selected_var = -1;

  icoords *mousepos = &gg->wvis.mousepos;
  gint color = gg->wvis.nearest_color;

  if(tree_view) {
    d = (GGobiData *) g_object_get_data(G_OBJECT (tree_view), "datad");
    selected_var = get_one_selection_from_tree_view (tree_view, d);
  }

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  if (pos.x != mousepos->x) {
    val = (gfloat) (pos.x - xmargin) /
          (gfloat) (w->allocation.width - 2*xmargin);

    /*-- don't allow it to cross its neighbors' boundaries --*/
    if ((color == 0 && val <= gg->wvis.pct[color+1] && val >= 0) ||
        (color == gg->wvis.npct-1 && val >= gg->wvis.pct[color+1]) ||
        (val >= gg->wvis.pct[color-1] && val <= gg->wvis.pct[color+1]))
    {
      gg->wvis.pct[color] = val;

      if (selected_var != -1 && selected_var < d->ncols)
        bin_counts_reset (selected_var, d, gg);

      g_signal_emit_by_name(G_OBJECT (w), "expose_event",
        (gpointer) gg, (gpointer) &rval);

      if (gg->wvis.update_method == WVIS_UPDATE_CONTINUOUSLY) {
        record_colors_reset (selected_var, d, gg);
        clusters_set(d);
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
    gg->wvis.motion_notify_id = g_signal_connect (G_OBJECT (w),
      "motion_notify_event", G_CALLBACK(motion_notify_cb), (gpointer) gg);
  }

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT (w));
  GGobiData *d = NULL; 
  gint selected_var = -1;

  if(tree_view) {
      d = (GGobiData *) g_object_get_data(G_OBJECT (tree_view), "datad");
      selected_var = get_one_selection_from_tree_view (tree_view, d);
  }

  if (gg->wvis.motion_notify_id) {
    g_signal_handler_disconnect (G_OBJECT (w), gg->wvis.motion_notify_id);
    gg->wvis.motion_notify_id = 0;
  }

  if (selected_var >= 0 && selected_var <= d->ncols) {
    record_colors_reset (selected_var, d, gg);
    clusters_set(d);
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
bin_boundaries_set (gint selected_var, GGobiData *d, ggobid *gg)
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
      pairs[i].f = d->tform.vals[d->rows_in_plot.els[i]][varno];
      pairs[i].indx = d->rows_in_plot.els[i];
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

static void binning_method_cb (GtkWidget *w, ggobid *gg)
{
  gboolean rval = false;
  gg->wvis.binning_method = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

  gg->wvis.npct = 0;  /*-- force bin_boundaries_set to be called --*/
  g_signal_emit_by_name(G_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

static void update_method_cb (GtkWidget *w, ggobid *gg)
{
  gg->wvis.update_method = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
}

static void alloc_pct (ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  if (gg->wvis.npct != scheme->n) {
    gg->wvis.npct = scheme->n;
    gg->wvis.pct = (gfloat *) g_realloc (gg->wvis.pct,
                                         gg->wvis.npct * sizeof (gfloat));
    gg->wvis.n = (gint *) g_realloc (gg->wvis.n,
                                     gg->wvis.npct * sizeof (gint));
  }
}

static void
da_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  gint height = w->allocation.height - 2*ymargin;
  gint x0, x1, k, hgt;
  gint x = xmargin;
  gint y = ymargin;
  gfloat diff;
  vartabled *vt;
  colorschemed *scheme = gg->activeColorScheme;

  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT (w));
  GGobiData *d = NULL;
  gint selected_var = -1;

  GtkWidget *da = gg->wvis.da;
  GdkPixmap *pix = gg->wvis.pix;

  if(tree_view) {
    d = (GGobiData *) g_object_get_data(G_OBJECT (tree_view), "datad");
    selected_var = get_one_selection_from_tree_view (tree_view, d);
    if (selected_var == -1) {
      selected_var = 0;
      select_tree_view_row (tree_view, selected_var);
    }
  }

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
  gdk_gc_set_foreground (gg->wvis.GC, &gg->mediumgray);
  for (k=0; k<scheme->n-1; k++) {
    gdk_draw_line (pix, gg->wvis.GC, x0, y, x1, y);
    y += hgt;
  }

  /*-- draw rectangles, 20 x 10 --*/
  y = ymargin + 10;
  for (k=0; k<scheme->n-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    draw_3drectangle (w, pix, x, y, 20, 10, gg);
    y += hgt;
  }

  /*-- add the variable limits in the top margin --*/
  if (d && selected_var != -1) {
    gfloat min, max;
    gfloat val;
    gchar *str;
    PangoRectangle rect;
    PangoLayout *layout = gtk_widget_create_pango_layout(da, NULL);

    vt = vartable_element_get (selected_var, d);
    if (vt) {
      min = vt->lim_tform.min;
      max = vt->lim_tform.max;

      gdk_gc_set_foreground (gg->wvis.GC, &scheme->rgb_accent);
      y = ymargin;
      for (k=0; k<scheme->n-1; k++) {

        val = min + gg->wvis.pct[k] * (max - min);
        str = g_strdup_printf ("%3.3g", val);
        layout_text(layout, str, &rect);
        x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
        gdk_draw_layout(pix, gg->wvis.GC, x - rect.width/2, y - 2 - rect.height, layout);
        g_free (str);
      }

      /*-- ... and the counts in the bottom margin --*/
      for (k=0; k<scheme->n; k++) {
        val = min + gg->wvis.pct[k] * (max - min);
        str = g_strdup_printf ("%d", gg->wvis.n[k]);
        layout_text(layout, str, &rect);
        x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
        diff = (k == 0) ? gg->wvis.pct[k] : gg->wvis.pct[k]-gg->wvis.pct[k-1]; 
        x -= diff/2 * (w->allocation.width - 2*xmargin);
        gdk_draw_layout(pix, gg->wvis.GC, 
          x - rect.width/2,
          (w->allocation.height - ymargin) + 2,
          layout);
        g_free (str);
      }
    }
    g_object_unref(G_OBJECT(layout));
  }

  gdk_draw_pixmap (w->window, gg->wvis.GC, pix,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

void
selection_made_cb (GtkTreeSelection *tree_sel, ggobid *gg)
{
  gboolean rval = false;
  GtkTreeView *tree_view = gtk_tree_selection_get_tree_view(tree_sel);
  GGobiData *d = (GGobiData *) g_object_get_data(G_OBJECT (tree_view), "datad");
  GtkWidget *btn;
  gint row;

  row = tree_selection_get_selected_row(tree_sel);
  if (row == -1)
    return;
  
  bin_boundaries_set (row, d, gg);  /*-- in case the method changed --*/
  bin_counts_reset (row, d, gg);
  g_signal_emit_by_name(G_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);

  /*-- get the apply button, make it sensitive --*/
  btn = widget_find_by_name (gg->wvis.window, "WVIS:apply");
  if (btn)
    gtk_widget_set_sensitive (btn, true);
}


static void scale_apply_cb (GtkWidget *w, ggobid* gg)
{
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT (w));
  GGobiData *d = (GGobiData *) g_object_get_data(G_OBJECT (tree_view), "datad");
  gint selected_var = get_one_selection_from_tree_view (tree_view, d);
  colorschemed *scheme = gg->activeColorScheme;

  if (d && selected_var != -1) {
    gboolean rval = false;

    record_colors_reset (selected_var, d, gg);
    clusters_set(d);

    /*-- before calling displays_plot, reset brushing color if needed --*/
    if (gg->color_id >= scheme->n) gg->color_id = scheme->n - 1;

    displays_plot (NULL, FULL, gg);

    bin_counts_reset (selected_var, d, gg);
    g_signal_emit_by_name(G_OBJECT (gg->wvis.da), "expose_event",
      (gpointer) gg, (gpointer) &rval);

    symbol_window_redraw (gg);
    cluster_table_update (d, gg);
  }
}

void
wvis_window_open (ggobid *gg) 
{
  GtkWidget *vbox, *hb;
  GtkWidget *notebook = NULL;
  GtkWidget *btn, *opt;
  static gchar *const binning_method_lbl[] = {
    "Constant bin width",
    "Constant bin count (approx)"};
  static gchar *const update_method_lbl[] = {
    "Update on mouse release",
    "Update continuously"};

  alloc_pct (gg);

  if (gg->wvis.window == NULL) {

    gg->wvis.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->wvis.window),
      "Automatic Brushing by Variable");
    g_signal_connect (G_OBJECT (gg->wvis.window),
      "delete_event", G_CALLBACK (close_wmgr_cb), gg);

    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_box_set_spacing (GTK_BOX(vbox), 5);
    gtk_container_add (GTK_CONTAINER (gg->wvis.window), vbox);    

    /* Create a notebook, set the position of the tabs */
    notebook = wvis_create_variable_notebook (vbox, GTK_SELECTION_SINGLE,
      G_CALLBACK(selection_made_cb), gg);
    gtk_widget_set_sensitive(notebook, true);


    /*-- now we get fancy:  draw the scale, with glyphs and colors --*/
    gg->wvis.da = gtk_drawing_area_new ();
    gtk_widget_set_double_buffered(gg->wvis.da, false);
    gtk_widget_set_size_request (GTK_WIDGET (gg->wvis.da), 400, 200);
    g_object_set_data(G_OBJECT (gg->wvis.da), "notebook", notebook);
    gtk_box_pack_start (GTK_BOX (vbox), gg->wvis.da, false, false, 0);

    g_signal_connect (G_OBJECT (gg->wvis.da),
                        "configure_event",
                        G_CALLBACK(da_configure_cb),
                        (gpointer) gg);
    g_signal_connect (G_OBJECT (gg->wvis.da),
                        "expose_event",
                        G_CALLBACK(da_expose_cb),
                        (gpointer) gg);
    g_signal_connect (G_OBJECT (gg->wvis.da),
                        "button_press_event",
                        G_CALLBACK(button_press_cb),
                        (gpointer) gg);
    g_signal_connect (G_OBJECT (gg->wvis.da),
                        "button_release_event",
                        G_CALLBACK(button_release_cb),
                        (gpointer) gg);

    gtk_widget_set_events (gg->wvis.da, GDK_EXPOSURE_MASK
               | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
               | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

    /*-- hbox to hold the options --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    /*-- option menu for choosing the method of binning --*/
    opt = gtk_combo_box_new_text ();
    gtk_widget_set_name (opt, "WVIS:binning_method");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Select a binning method", NULL);
    gtk_box_pack_start (GTK_BOX (hb), opt,
      false, false, 0);
    populate_combo_box (opt, (gchar**) binning_method_lbl, G_N_ELEMENTS(binning_method_lbl),
      G_CALLBACK(binning_method_cb), gg);

    /*-- option menu for choosing the method of updating --*/
    /* This should be a checkbox, I think ... */
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "How to update the displays in response to movements of the sliders",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), opt, true, true, 0);
    populate_combo_box (opt, (gchar**) update_method_lbl, G_N_ELEMENTS(update_method_lbl),
      G_CALLBACK(update_method_cb), gg);

    /*-- hbox for buttons --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
      false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    /* Apply button */
    btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    g_object_set_data(G_OBJECT (btn), "notebook", notebook);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Apply the color scale, using the values of the variable selected in the list above",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 1);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (scale_apply_cb), gg);
    gtk_widget_set_name (btn, "WVIS:apply");



    btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 2);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (close_btn_cb), gg);
  }

  gtk_widget_show_all (gg->wvis.window);
  gdk_window_raise (gg->wvis.window->window);
}

