/* schemes_ui.c */
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
 * It is my understanding that I'm supposed to use gdk_colormap_free_colors
 * to free anything I've allocated with gdk_colormap_alloc_color(s), but
 * it just isn't working to free all the svis.color_map colors.   
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

GtkWidget *createColorSchemeTree (int numTypes, gchar * schemeTypes[],
                                  ggobid * gg);
static void entry_set_scheme_name (ggobid * gg);

/*-------------------------------------------------------------------*/
/*             Using colorschemed objects                            */
/*-------------------------------------------------------------------*/

void
colorscheme_set_cb (GtkTreeSelection * sel, GtkTreeView * tree_view)
{
  ggobid *gg = GGobiFromWidget (GTK_WIDGET (tree_view), true);
  gboolean rval = false;
  GtkTreeModel *model;
  GGobiStage *d;
  colorschemed *scheme;
  GtkTreeIter iter;

/*
 * gg->svis sometimes has its own scheme, and then we'll use it.
 * If it's null, we use gg->activeColorScheme.  We update
 * gg->activeColorScheme to the value of scheme when the user asks.
*/

  if (!gtk_tree_selection_get_selected (sel, &model, &iter))
    return;

  gtk_tree_model_get (model, &iter, 1, &scheme, -1);

  if (scheme) {
    gg->svis.scheme = scheme;
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
  tree_view = gtk_tree_selection_get_tree_view (sel);
  if (tree_view != NULL) {
    d = (GGobiStage *) g_object_get_data (G_OBJECT (tree_view), "datad");
  }
  else {
    d = (GGobiStage *) g_slist_nth_data (gg->d, 0);
  }

  g_signal_emit_by_name (G_OBJECT (gg->svis.da), "expose_event",
                         (gpointer) gg, (gpointer) & rval);
}


/*-------------------------------------------------------------------------*/

/*-- called when closed from the close button --*/
static void
close_btn_cb (GtkWidget * w, ggobid * gg)
{
  gtk_widget_hide (gg->svis.window);
}

/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget * w, GdkEventButton * event, ggobid * gg)
{
  gtk_widget_hide (gg->svis.window);
}

static gint
da_configure_cb (GtkWidget * w, GdkEventConfigure * event, ggobid * gg)
{
  /*-- Create new backing pixmaps of the appropriate size --*/
  if (gg->svis.pix != NULL)
    gdk_pixmap_unref (gg->svis.pix);
  gg->svis.pix = gdk_pixmap_new (w->window,
                                 w->allocation.width, w->allocation.height,
                                 -1);

  gtk_widget_queue_draw (w);

  return false;
}

/*
 * Set the bin boundaries (the values of svis.pct[]) by
 * simply dividing the range of the data into
 * scheme->n equal-sized pieces
*/
static void
bin_boundaries_set (GGobiStage * d, ggobid * gg)
{
  gint k;

  /*
   * These numbers are the upper boundaries of each interval.
   * By default, they start at .1 and end at 1.0.
   */
  for (k = 0; k < gg->svis.npct; k++) {
    gg->svis.pct[k] = (gfloat) (k + 1) / (gfloat) gg->svis.npct;
  }
}

static void
da_expose_cb (GtkWidget * w, GdkEventExpose * event, ggobid * gg)
{
  gint height = w->allocation.height - 2 * ymargin;
  gint x0, x1, k, hgt;
  colorschemed *scheme = (gg->svis.scheme != NULL) ?
    gg->svis.scheme : gg->activeColorScheme;
  GGobiStage *d = NULL;
  GdkPixmap *pix = gg->svis.pix;

  if (gg->svis.GC == NULL)
    gg->svis.GC = gdk_gc_new (w->window);

  hgt = height / (scheme->n - 1);

  if (gg->svis.npct != scheme->n) {
    gg->svis.npct = scheme->n;
    gg->svis.pct = (gfloat *) g_realloc (gg->svis.pct,
                                         gg->svis.npct * sizeof (gfloat));
    bin_boundaries_set (d, gg);
  }

  /*-- clear the pixmap --*/
  gdk_gc_set_foreground (gg->svis.GC, &scheme->rgb_bg);
  gdk_draw_rectangle (pix, gg->svis.GC, TRUE,
                      0, 0, w->allocation.width, w->allocation.height);


  /*-- draw the color bars --*/
  x0 = xmargin;
  for (k = 0; k < scheme->n; k++) {
    x1 = xmargin + gg->svis.pct[k] * (w->allocation.width - 2 * xmargin);
    gdk_gc_set_foreground (gg->svis.GC, &scheme->rgb[k]);
    gdk_draw_rectangle (pix, gg->svis.GC, TRUE, x0, ymargin, x1 - x0, height);
    x0 = x1;
  }

  gdk_draw_pixmap (w->window, gg->svis.GC, pix,
                   0, 0, 0, 0, w->allocation.width, w->allocation.height);
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
gboolean
colors_remap (colorschemed * scheme, gboolean force, ggobid * gg)
{
  gint i, k;
  gboolean all_colors_p[MAXNCOLORS];
  GSList *l;
  GGobiStage *d;
  gushort colors_used[MAXNCOLORS];
  gint maxcolorid, ncolors_used;
  gboolean remap_ok = true;

  for (k = 0; k < MAXNCOLORS; k++)
    all_colors_p[k] = false;

  /*-- find out all the colors (indices) are currently in use --*/
  for (l = gg->d; l; l = l->next) {
    d = (GGobiStage *) l->data;
    datad_colors_used_get (&ncolors_used, colors_used, d, gg);
    for (k = 0; k < ncolors_used; k++)
      all_colors_p[colors_used[k]] = true;
  }

  /*-- find out how many colors are currently in use --*/
  ncolors_used = 0;
  for (k = 0; k < MAXNCOLORS; k++)
    if (all_colors_p[k])
      ncolors_used++;

  /*-- find the largest color index currently in use --*/
  maxcolorid = -1;
  for (k = MAXNCOLORS - 1; k > 0; k--) {
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
    quick_message
      ("The number of colors now in use is greater than than\nthe number of colors in the chosen color scheme.  Please choose a color scheme with more colours, or use less colors in the plot.",
       false);

    remap_ok = false;
  }
  else if (maxcolorid >= scheme->n) {
    /*-- build the vector that will be used to reset the current indices --*/
    gint *newind = (gint *) g_malloc ((maxcolorid + 1) * sizeof (gint));
    gint n = 0;

    for (k = 0; k <= maxcolorid; k++) {
      if (all_colors_p[k]) {
        newind[k] = n;

        /*
         * try to achieve a decent spread of the color values,
         * which is helpful in most color maps
         */
        n += ((scheme->n + 1) / ncolors_used);
        /*-- make sure we haven't gone too far --*/
        if (n >= scheme->n - 1)
          n = scheme->n - 1;

      }
    }

    for (l = gg->d; l; l = l->next) {
      d = (GGobiStage *) l->data;
      GGOBI_STAGE_ATTR_INIT_ALL(d);  
      for (i = 0; i < d->n_rows; i++) {
        GGOBI_STAGE_SET_ATTR_COLOR(d, i, newind[ GGOBI_STAGE_GET_ATTR_COLOR(d, i)], ATTR_SET_PERSISTENT);
        //d->color.els[i] = newind[d->color.els[i]];
        //d->color_now.els[i] = newind[d->color_now.els[i]];
        /*-- what about color_prev?  --*/
      }
    }
    g_free (newind);

  }
  else {
    g_printerr ("nothing else should possibly happen, no?\n");
  }

  return remap_ok;
}

static void
scale_set_cb (GtkWidget * w, ggobid * gg)
{
  GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT (w));
  GGobiStage *d = NULL;
  gboolean rval = false;

  if (tree_view)
    d = (GGobiStage *) g_object_get_data (G_OBJECT (tree_view), "datad");

  /*
   * If we've been using gg->svis.scheme, set gg->activeColorScheme
   * to the current scheme.
   */
  if (gg->svis.scheme) {
    colorschemed *scheme = gg->svis.scheme;

    /*-- if no current color index is too high, continue --*/
    if (!colors_remap (scheme, false, gg))
      return;

    gg->activeColorScheme = scheme;
    gg->svis.scheme = NULL;
  }

  displays_plot (NULL, FULL, gg);
  g_signal_emit_by_name (G_OBJECT (gg->svis.da), "expose_event",
                         (gpointer) gg, (gpointer) & rval);

  entry_set_scheme_name (gg);

  symbol_window_redraw (gg);
  cluster_table_update (d, gg);
}


static void
entry_set_scheme_name (ggobid * gg)
{
  gtk_entry_set_text (GTK_ENTRY (gg->svis.entry_preview),
                      (gg->svis.scheme != NULL) ? gg->svis.scheme->name :
                      gg->activeColorScheme->name);

  gtk_entry_set_text (GTK_ENTRY (gg->svis.entry_applied),
                      gg->activeColorScheme->name);
}

void
svis_window_open (ggobid * gg)
{
  GtkWidget *vbox;
  GtkWidget *hb;
  GtkWidget *btn, *label;

  /*-- for colorscales --*/
  GtkWidget *hpane, *tr, *sw;
  static gchar *colorscaletype_lbl[UNKNOWN_COLOR_TYPE] = {
    "<b>Diverging</b>",
    "<b>Sequential</b>",
    "<b>Spectral</b>",
    "<b>Qualitative</b>"
  };

  if (gg->svis.window == NULL) {

    gg->svis.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->svis.window),
                          "Choose Color Scheme");
    g_signal_connect (G_OBJECT (gg->svis.window),
                      "delete_event", G_CALLBACK (close_wmgr_cb), gg);

    hpane = gtk_hpaned_new ();
    //gtk_paned_set_position (GTK_PANED(hpane), 150);
    gtk_container_add (GTK_CONTAINER (gg->svis.window), hpane);

    /* Color scheme tree */
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                         GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (hpane), sw);

    tr = createColorSchemeTree (UNKNOWN_COLOR_TYPE, colorscaletype_lbl, gg);
    gtk_widget_set_size_request (sw, 150, 20);
    gtk_container_add (GTK_CONTAINER (sw), tr);


    /* 
     * Right half of window
     */
    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_box_set_spacing (GTK_BOX (vbox), 5);
    gtk_container_add (GTK_CONTAINER (hpane), vbox);

    /* Name currently in use */
    hb = gtk_hbox_new (true, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hb, true, true, 5);
    label = gtk_label_new ("Color scheme in use");
    gtk_misc_set_alignment (GTK_MISC (label), 0, .5);
    gtk_box_pack_start (GTK_BOX (hb), label, true, true, 0);
    gg->svis.entry_applied = gtk_entry_new ();
    gtk_editable_set_editable (GTK_EDITABLE (gg->svis.entry_applied), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->svis.entry_applied,
                          "The name of the currently active color scheme.",
                          NULL);
    gtk_box_pack_start (GTK_BOX (hb), gg->svis.entry_applied, true, true, 0);
     /**/
      /* preview scheme */
      hb = gtk_hbox_new (true, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hb, true, true, 5);
    label = gtk_label_new ("Color scheme  in preview");
    gtk_misc_set_alignment (GTK_MISC (label), 0, .5);
    gtk_box_pack_start (GTK_BOX (hb), label, true, true, 0);

    gg->svis.entry_preview = gtk_entry_new ();
    gtk_editable_set_editable (GTK_EDITABLE (gg->svis.entry_preview), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->svis.entry_preview,
                          "The name of the color scheme whose colors are displayed below.",
                          NULL);
    gtk_box_pack_start (GTK_BOX (hb), gg->svis.entry_preview, true, true, 0);


    /* Drawing area */
    gg->svis.da = gtk_drawing_area_new ();
    gtk_widget_set_double_buffered (gg->svis.da, false);
    gtk_widget_set_size_request (GTK_WIDGET (gg->svis.da), 300, 150);
    gtk_box_pack_start (GTK_BOX (vbox), gg->svis.da, false, false, 0);

    g_signal_connect (G_OBJECT (gg->svis.da),
                      "configure_event",
                      G_CALLBACK (da_configure_cb), (gpointer) gg);
    g_signal_connect (G_OBJECT (gg->svis.da),
                      "expose_event",
                      G_CALLBACK (da_expose_cb), (gpointer) gg);

    gtk_widget_set_events (gg->svis.da, GDK_EXPOSURE_MASK);

    /* Initializes both entries */
    entry_set_scheme_name (gg);

    /*-- add a close button --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (),
                        false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    /* Apply button */
    btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Make this the current color scheme for brushing in ggobi, preserving current color groups.  If the number of colors in the new scheme is less than the number of colors currently in use, this won't work.",
                          NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 2);
    g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (scale_set_cb), gg);

    btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 2);
    g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (close_btn_cb), gg);
  }

  gtk_widget_show_all (gg->svis.window);
  gdk_window_raise (gg->svis.window->window);
}

GtkWidget *createSchemeColorsTree (colorschemed * scheme);

/**
 Create the tree displaying the colorscheme information.
 This displays the different levels:
    types of schemes, 
    different schemes within each type, 
    colors within each scheme.
 
 */
GtkWidget *
createColorSchemeTree (gint numTypes, gchar * schemeTypes[], ggobid * gg)
{
  //GtkWidget *item;
  //GtkWidget **trees, *top;
  /*GtkWidget *tree; */
  GtkWidget *tree_view;
  GtkTreeStore *model;
  GtkTreeIter *iters;
  gint n;
  GList *l;
  colorschemed *scheme;

  model = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);

  iters = g_new (GtkTreeIter, numTypes);
  for (n = 0; n < numTypes; n++) {
    gtk_tree_store_append (GTK_TREE_STORE (model), &iters[n], NULL);
    gtk_tree_store_set (GTK_TREE_STORE (model), &iters[n], 0, schemeTypes[n],
                        1, NULL, -1);
  }

  for (l = gg->colorSchemes; l; l = l->next) {
    GtkTreeIter iter;
    scheme = (colorschemed *) l->data;
    gtk_tree_store_append (GTK_TREE_STORE (model), &iter,
                           &iters[scheme->type]);
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, 0, scheme->name, 1,
                        scheme, -1);
  }

  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  ggobi_widget_set (tree_view, gg, true);

  populate_tree_view (tree_view, NULL, 1, false, GTK_SELECTION_SINGLE,
                      G_CALLBACK (colorscheme_set_cb), tree_view);

  return (tree_view);
}
