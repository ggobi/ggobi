/* barchart.c */
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
 *
 * Contributing author of barchart and histogram code:  Heike Hofmann
*/


/* not dealt with: missings, hiddens in overflow bins */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "barchartDisplay.h"

#define WIDTH   370
#define HEIGHT  370

gfloat barchart_sort_index (gfloat * yy, gint ny, ggobid * gg,
                            barchartSPlotd * sp);
void barchart_init_categorical (barchartSPlotd * sp, GGobiStage * d);
void barchart_set_initials (barchartSPlotd * sp, GGobiStage * d);
void rectangle_inset (gbind * bin);
void barchart_allocate_structure (barchartSPlotd * sp, GGobiStage * d);
void button_draw_with_shadows (GdkPoint * region, GdkDrawable * drawable,
                               ggobid * gg);
gboolean rect_intersect (GdkRectangle * rect1, GdkRectangle * rect2,
                         GdkRectangle * dest);
gboolean pt_in_rect (icoords pt, GdkRectangle rect);

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static const gchar *menu_ui =
  "<ui>"
  "	<menubar>"
  "		<menu action='Options'>"
  "			<menuitem action='ShowPoints'/>"
  "			<separator/>"
  "			<menuitem action='ShowAxes'/>" "		</menu>" "	</menubar>" "</ui>";

static void
action_toggle_show_bars (GtkToggleAction * action, displayd * display)
{
  set_display_option (gtk_toggle_action_get_active (action), DOPT_POINTS,
                      display);
}

/* the 'ShowPoints' display action is overridden here for bar display */
static GtkToggleActionEntry toggle_entries[] = {
  {"ShowPoints", NULL, "Show _bars", "<control>B", "Toggle bar display",
   G_CALLBACK (action_toggle_show_bars), true},
};

static guint n_toggle_entries = G_N_ELEMENTS (toggle_entries);

displayd *
barchart_new (gboolean missing_p, splotd * sp, GGobiStage * d, ggobid * gg)
{
  return (createBarchart (NULL, missing_p, sp, -1, d, gg));
}

displayd *
barchart_new_with_vars (gboolean missing_p, gint nvars, gint * vars,
                        GGobiStage * d, ggobid * gg)
{
  return (createBarchart (NULL, missing_p, NULL, vars ? vars[0] : 0, d, gg));
}

displayd *
createBarchart (displayd * display, gboolean missing_p, splotd * sp, gint var,
                GGobiStage * d, ggobid * gg)
{
  GtkWidget *table, *vbox;

  if (d == NULL || !ggobi_stage_has_vars(d))
    return (NULL);

  if (!display) {
    if (sp == NULL || sp->displayptr == NULL) {
      /* Use GGOBI_TYPE_BARCHART_DISPLAY, or the regular extended
         display and set the titleLabel immediately afterward. If more
         goes into barchart, we will do the former. And that's what we
         do.  The alternative is.
         display = g_object_new(GGOBI_TYPE_EXTENDED_DISPLAY);
         GGOBI_EXTENDED_DISPLAY(display)->titleLabel = "BarChart";
       */
      display = g_object_new (GGOBI_TYPE_BARCHART_DISPLAY, NULL);
      display_set_values (display, d, gg);
    }
    else { /* is this code ever used? mfl */
      display = (displayd *) sp->displayptr;
      display->d = d;
    }
  }

  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
     display->options.axes_center_p = true;
   */

  barchart_cpanel_init (&display->cpanel, gg);

  if (GGOBI_IS_WINDOW_DISPLAY (display)
      && GGOBI_WINDOW_DISPLAY (display)->useWindow)
    display_window_init (GGOBI_WINDOW_DISPLAY (display), WIDTH, HEIGHT, 3, gg);  /*-- 3 = width = any small int --*/

  /*-- Add the main menu bar --*/
  vbox = GTK_WIDGET (display);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);
  if (GGOBI_IS_WINDOW_DISPLAY (display)
      && GGOBI_WINDOW_DISPLAY (display)->window) {
    //gg->app.sp_accel_group = gtk_accel_group_new();
    GtkActionGroup *actions = gtk_action_group_new ("BarchartActions");
    gtk_action_group_add_toggle_actions (actions, toggle_entries,
                                         n_toggle_entries, display);
    display->menu_manager = display_menu_manager_create (display);
    gtk_ui_manager_insert_action_group (display->menu_manager, actions, 0);
    g_object_unref (G_OBJECT (actions));
    display->menubar =
      create_menu_bar (display->menu_manager, menu_ui,
                       GGOBI_WINDOW_DISPLAY (display)->window);
    /*
     * After creating the menubar, and populating the file menu,
     * add the other menus manually
     */
    //barchart_display_menus_make(display, gg->app.sp_accel_group,
    //      G_CALLBACK(display_options_cb), gg);

    gtk_container_add (GTK_CONTAINER (GGOBI_WINDOW_DISPLAY (display)->window),
                       vbox);
    gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);
  }

  /*-- Initialize a single splot --*/
  if (sp == NULL) {
    sp = ggobi_barchart_splot_new (display, gg);
  }

  /* Reset sp->p1dvar based on the plotted variables in the current
     display, if appropriate -- it has already been initialized to
     zero in splot_init, a few levels down from
     ggobi_barchart_splot_new(), */
  if (gg->current_display != NULL && gg->current_display != display &&
      ggobi_stage_get_root(gg->current_display->d) == ggobi_stage_get_root(d) &&
      GGOBI_IS_EXTENDED_DISPLAY (gg->current_display)) {
    gint nplotted_vars;
    gint *plotted_vars = (gint *) g_malloc (d->n_cols * sizeof (gint));
    displayd *dsp = gg->current_display;

    nplotted_vars =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (dsp)->plotted_vars_get (dsp,
                                                                plotted_vars,
                                                                d, gg);
    if (nplotted_vars && plotted_vars[0] != 0) {
      sp->p1dvar = plotted_vars[0];
      barchart_clean_init (GGOBI_BARCHART_SPLOT (sp));
      barchart_recalc_counts (GGOBI_BARCHART_SPLOT (sp), display->d, gg);
    }
  }


  display->splots = NULL;
  display->splots = g_list_append (display->splots, (gpointer) sp);

  /*-- Initialize tours if possible --*/
  display_tour1d_init_null (display, gg);
  if (d->n_cols >= MIN_NVARS_FOR_TOUR1D)
    display_tour1d_init (display, gg);

  table = gtk_table_new (3, 2, false);  /* rows, columns, homogeneous */
  gtk_box_pack_start (GTK_BOX (vbox), table, true, true, 0);
  gtk_table_attach (GTK_TABLE (table),
                    sp->da, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK | GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK | GTK_EXPAND | GTK_FILL),
                    0, 0);



  /*
   * The horizontal ruler goes on top. As the mouse moves across the
   * drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
   */
  display->hrule = gtk_hruler_new ();

  gtk_table_attach (GTK_TABLE (table),
                    display->hrule, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    (GtkAttachOptions) GTK_FILL, 0, 0);


  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
   */

  display->vrule = gtk_vruler_new ();

  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_FILL,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                    0, 0);


  if (GGOBI_IS_WINDOW_DISPLAY (display)
      && GGOBI_WINDOW_DISPLAY (display)->useWindow)
    gtk_widget_show_all (GGOBI_WINDOW_DISPLAY (display)->window);
  else
    gtk_widget_show_all (table);

  /*-- hide any extraneous rulers --*/

  display->p1d_orientation = VERTICAL;
  scatterplot_show_rulers (display, P1PLOT);  /* put in pmodeSet? */
  ruler_ranges_set (true, display, sp, gg);

  return display;
}

void
barchart_clean_init (barchartSPlotd * sp)
{
  displayd *display;
  GGobiStage *d;
  gint i, j;

  display = (displayd *) GGOBI_SPLOT (sp)->displayptr;
  d = display->d;

  sp->bar->nbins = -1;

  sp->bar->new_nbins = -1;
  barchart_allocate_structure (sp, d);

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->bins[i].count = 0;
    sp->bar->bins[i].nhidden = 0;
    sp->bar->bar_hit[i] = FALSE;
    sp->bar->old_bar_hit[i] = FALSE;
    for (j = 0; j < sp->bar->ncolors; j++) {
      sp->bar->cbins[i][j].count = 0;
      sp->bar->cbins[i][j].rect.width = 1;
    }
  }
  for (i = 0; i < sp->bar->nbins + 2; i++)
    sp->bar->bar_hit[i] = sp->bar->old_bar_hit[i] = false;
  sp->bar->old_nbins = -1;

/* */

  barchart_set_initials (sp, d);
  sp->bar->offset = 0;
  GGOBI_SPLOT (sp)->pmid.y = 0;

  vectori_realloc (&sp->bar->index_to_rank, d->n_rows);
  barchart_init_categorical (sp, d);
}

static void
barchart_recalc_group_counts (barchartSPlotd * sp, GGobiStage * d, ggobid * gg)
{
  gint i, j, bin;
  
  g_assert (sp->bar->index_to_rank.nels == d->n_rows);

  for (i = 0; i < sp->bar->nbins; i++)
    for (j = 0; j < sp->bar->ncolors; j++)
      sp->bar->cbins[i][j].count = 0;

/*  initialize overflow bins */
  if (sp->bar->high_pts_missing) {
    for (j = 0; j < sp->bar->ncolors; j++)
      sp->bar->col_high_bin[j].count = 0;
  }
  if (sp->bar->low_pts_missing) {
    for (j = 0; j < sp->bar->ncolors; j++)
      sp->bar->col_low_bin[j].count = 0;
  }

/* count points in bins */
  for (i = 0; i < d->n_rows; i++) {
    /*-- skip missings?  --*/
    if (!d->missings_show_p && ggobi_stage_is_missing(d, i, GGOBI_SPLOT (sp)->p1dvar))
      continue;
    
    GGOBI_STAGE_ATTR_INIT_ALL(d); 
    /*-- skip hiddens?  here, yes. --*/
    if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {
      continue;
    }

    bin = GGOBI_SPLOT (sp)->planar[i].x;
/* dfs */
    if (GGOBI_STAGE_IS_COL_CATEGORICAL(d, GGOBI_SPLOT (sp)->p1dvar))
      bin = sp->bar->index_to_rank.els[i];
/* --- */
    if ((bin >= 0) && (bin < sp->bar->nbins)) {
      sp->bar->cbins[bin][ GGOBI_STAGE_GET_ATTR_COLOR(d, i)].count++;
    }
    if (bin == -1) {
      sp->bar->col_low_bin[ GGOBI_STAGE_GET_ATTR_COLOR(d, i)].count++;
    }
    else if (bin == sp->bar->nbins) {
      sp->bar->col_high_bin[ GGOBI_STAGE_GET_ATTR_COLOR(d, i)].count++;
    }
  }

  barchart_recalc_group_dimensions (sp, gg);
}


void
barchart_recalc_group_dimensions (barchartSPlotd * sp, ggobid * gg)
{
  gint colorwidth, i, j, xoffset;

  for (i = 0; i < sp->bar->nbins; i++) {
    xoffset = sp->bar->bins[i].rect.x;

/* first all bins in the current color */
    j = gg->color_id;
    colorwidth = 1;
    if (sp->bar->bins[i].count > 0)
      colorwidth =
        (gint) ((gfloat) sp->bar->cbins[i][j].count /
                sp->bar->bins[i].count * sp->bar->bins[i].rect.width);
    sp->bar->cbins[i][j].rect.x = xoffset;
    sp->bar->cbins[i][j].rect.y = sp->bar->bins[i].rect.y;
    sp->bar->cbins[i][j].rect.height = sp->bar->bins[i].rect.height;

    sp->bar->cbins[i][j].rect.width = colorwidth;
    if (colorwidth) {
      colorwidth++;
      rectangle_inset (&sp->bar->cbins[i][j]);
    }
    xoffset += colorwidth;

/* then all other colors follow in the order of the color table */
    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth = 0;
        if (sp->bar->bins[i].count > 0)
          colorwidth =
            (gint) ((gfloat) sp->bar->cbins[i][j].count /
                    sp->bar->bins[i].count * sp->bar->bins[i].rect.width);
        sp->bar->cbins[i][j].rect.x = xoffset;
        sp->bar->cbins[i][j].rect.y = sp->bar->bins[i].rect.y;
        sp->bar->cbins[i][j].rect.height = sp->bar->bins[i].rect.height;

        sp->bar->cbins[i][j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset (&sp->bar->cbins[i][j]);
        }
        xoffset += colorwidth;
      }
    }
  }

/* now eliminate rounding problems - last color in each bin gets adjusted */
  for (i = 0; i < sp->bar->nbins; i++) {
    gboolean stop = FALSE;

    /*-- dfs:   don't do this if nmissing > 0; leave the shadow in place --*/
    if (sp->bar->bins[i].nhidden)
      continue;

    for (j = sp->bar->ncolors - 1; (j >= 0) && (!stop); j--)
      if (j != gg->color_id)
        if (sp->bar->cbins[i][j].count > 0)
          stop = TRUE;          /* find last color used */

    if (stop) {
      j++;
      sp->bar->cbins[i][j].rect.width =
        sp->bar->bins[i].rect.x + sp->bar->bins[i].rect.width -
        sp->bar->cbins[i][j].rect.x + 2;
    }
  }

/* deal with overflow bins to the left and right now:  */
  if (sp->bar->high_pts_missing) {
    j = gg->color_id;
    xoffset = sp->bar->high_bin->rect.x;
    colorwidth =
      (gint) ((gfloat) sp->bar->col_high_bin[j].count /
              sp->bar->high_bin->count * sp->bar->high_bin->rect.width);
    sp->bar->col_high_bin[j].rect.x = xoffset;
    sp->bar->col_high_bin[j].rect.y = sp->bar->high_bin->rect.y;
    sp->bar->col_high_bin[j].rect.height = sp->bar->high_bin->rect.height;
    sp->bar->col_high_bin[j].rect.width = colorwidth;
    if (colorwidth) {
      colorwidth++;
      rectangle_inset (&sp->bar->col_high_bin[j]);
    }
    xoffset += colorwidth;

    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth =
          (gint) ((gfloat) sp->bar->col_high_bin[j].count /
                  sp->bar->high_bin->count * sp->bar->high_bin->rect.width);
        sp->bar->col_high_bin[j].rect.x = xoffset;
        sp->bar->col_high_bin[j].rect.y = sp->bar->high_bin->rect.y;
        sp->bar->col_high_bin[j].rect.height = sp->bar->high_bin->rect.height;
        sp->bar->col_high_bin[j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset (&sp->bar->col_high_bin[j]);
        }
        xoffset += colorwidth;

      }
    }
  }
  if (sp->bar->low_pts_missing) {
    j = gg->color_id;
    xoffset = sp->bar->low_bin->rect.x;
    colorwidth =
      (gint) ((gfloat) sp->bar->col_low_bin[j].count /
              sp->bar->low_bin->count * sp->bar->low_bin->rect.width);
    sp->bar->col_low_bin[j].rect.x = xoffset;
    sp->bar->col_low_bin[j].rect.y = sp->bar->low_bin->rect.y;
    sp->bar->col_low_bin[j].rect.height = sp->bar->low_bin->rect.height;
    sp->bar->col_low_bin[j].rect.width = colorwidth;
    if (colorwidth) {
      colorwidth++;
      rectangle_inset (&sp->bar->col_low_bin[j]);
    }
    xoffset += colorwidth;

    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth =
          (gint) ((gfloat) sp->bar->col_low_bin[j].count /
                  sp->bar->low_bin->count * sp->bar->low_bin->rect.width);
        sp->bar->col_low_bin[j].rect.x = xoffset;
        sp->bar->col_low_bin[j].rect.y = sp->bar->low_bin->rect.y;
        sp->bar->col_low_bin[j].rect.height = sp->bar->low_bin->rect.height;
        sp->bar->col_low_bin[j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset (&sp->bar->col_low_bin[j]);
        }
        xoffset += colorwidth;
      }
    }
  }


}

void
rectangle_inset (gbind * bin)
{
/* works around the gdk convention, that the areas of filled and
   framed rectangles differ by one pixel in each dimension */

  bin->rect.height += 1;
  if (bin->rect.height < 1)
    bin->rect.height = 1;       /* set minimal height */
  bin->rect.x += 1;
  bin->rect.width += 1;

  if (bin->rect.width < 1)
    bin->rect.width = 1;        /* set minimal width */
}

void
barchart_init_vectors (barchartSPlotd * sp)
{
/* shouldn't be necessary ...*/
  if (sp->bar != NULL) {
    sp->bar->bins = NULL;
    sp->bar->cbins = NULL;
    sp->bar->breaks = NULL;
    sp->bar->high_bin = NULL;
    sp->bar->low_bin = NULL;
    sp->bar->col_high_bin = NULL;
    sp->bar->col_low_bin = NULL;
    sp->bar->bar_hit = NULL;
    sp->bar->old_bar_hit = NULL;
  }
}

void
barchart_free_structure (barchartSPlotd * sp)
{
  gint i;

/* free all previously allocated pointers */
  if (sp->bar->bins)
    g_free ((gpointer) (sp->bar->bins));

  if (sp->bar->cbins) {
    gint nbins = sp->bar->nbins;

    for (i = 0; i < nbins; i++)
      if (sp->bar->cbins[i])
        g_free ((gpointer) (sp->bar->cbins[i]));
    g_free ((gpointer) (sp->bar->cbins));
  }

  if (sp->bar->breaks)
    g_free ((gpointer) sp->bar->breaks);

  if (sp->bar->high_bin)
    g_free ((gpointer) sp->bar->high_bin);

  if (sp->bar->low_bin)
    g_free ((gpointer) sp->bar->low_bin);

  if (sp->bar->col_high_bin)
    g_free ((gpointer) sp->bar->col_high_bin);

  if (sp->bar->col_low_bin)
    g_free ((gpointer) sp->bar->col_low_bin);

  if (sp->bar->bar_hit)
    g_free ((gpointer) sp->bar->bar_hit);

  if (sp->bar->old_bar_hit)
    g_free ((gpointer) sp->bar->old_bar_hit);

  barchart_init_vectors (sp);
}

void
barchart_allocate_structure (barchartSPlotd * sp, GGobiStage * d)
{
  gint i, nbins;
  splotd *rawsp = GGOBI_SPLOT (sp);
  ggobid *gg = GGobiFromSPlot (rawsp);
  colorschemed *scheme = gg->activeColorScheme;
  GGobiVariable *var = ggobi_stage_get_variable(d, rawsp->p1dvar);
  
  if (sp->bar->new_nbins < 0) {
    if (ggobi_variable_get_vartype(var) == GGOBI_VARIABLE_CATEGORICAL) {
      nbins = ggobi_variable_get_n_levels(var);
      sp->bar->is_histogram = FALSE;
    }
    else {
      nbins = 10;               // replace by a more sophisticated rule
      sp->bar->is_histogram = TRUE;
    }
  }
  else
    nbins = sp->bar->new_nbins;
  sp->bar->new_nbins = -1;

  rawsp->p1d.lim.min = ggobi_variable_get_min(var);
  rawsp->p1d.lim.max = ggobi_variable_get_max(var);

  // nothing else to be done
  if (sp->bar->nbins && nbins == sp->bar->nbins)
    return;                     


/* free all previously allocated pointers */
  barchart_free_structure (sp);

  sp->bar->nbins = nbins;

/* allocate space */
  sp->bar->bins = (gbind *) g_malloc (nbins * sizeof (gbind));
  sp->bar->cbins = (gbind **) g_malloc (nbins * sizeof (gbind *));
  sp->bar->ncolors = scheme->n;
  sp->bar->bar_hit = (gboolean *) g_malloc ((nbins + 2) * sizeof (gboolean));
  sp->bar->old_bar_hit =
    (gboolean *) g_malloc ((nbins + 2) * sizeof (gboolean));

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->cbins[i] =
      (gbind *) g_malloc (sp->bar->ncolors * sizeof (gbind));
  }

  sp->bar->breaks = (gfloat *) g_malloc ((nbins + 1) * sizeof (nbins));
}

void
barchart_init_categorical (barchartSPlotd * sp, GGobiStage * d)
{
  splotd *rawsp = GGOBI_SPLOT (sp);
  displayd *display = (displayd *) rawsp->displayptr;
  gint proj = display->cpanel.pmode;
  gint i, j, m, jvar = rawsp->p1dvar;
  ggobid *gg = GGobiFromSPlot (rawsp);
  gfloat mindist, maxheight;
  gfloat min, max;
  GGobiVariable *var = ggobi_stage_get_variable(d, jvar);

  gfloat *yy;
  yy = (gfloat *) g_malloc (d->n_rows * sizeof (gfloat));

  if (proj == TOUR1D) {
    for (m=0; m < d->n_rows; m++) {
      yy[m] = rawsp->planar[m].x = 0;
      rawsp->planar[m].y = 0;
      for (j=0; j<d->n_cols; j++)
      {
        yy[m] += (gfloat)(display->t1d.F.vals[0][j]*d->world.vals[m][j]);
      }
    }    
  } 
  else {
    for (i = 0; i < d->n_rows; i++)
      yy[i] = ggobi_stage_get_raw_value(d, i, jvar);
  }
  mindist = barchart_sort_index (yy, d->n_rows, gg, sp);
  g_free ((gpointer) yy);

  min = ggobi_variable_get_min(var);
  max = ggobi_variable_get_max(var);

  maxheight = max - min;

  rawsp->scale.y =
    (1 - (1.0 - SCALE_DEFAULT) / 2) * maxheight / (maxheight + mindist);
}


gboolean
barchart_redraw (splotd * rawsp, GGobiStage * d, ggobid * gg, gboolean binned)
{
  gint i, j, radius;
  colorschemed *scheme = gg->activeColorScheme;
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (rawsp);
  gbind *bin;

  barchart_recalc_counts (sp, d, gg);
  barchart_recalc_group_counts (sp, d, gg);

/* dfs: if there are hiddens, draw the entire rectangle in the shadow color */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);
  for (i = 0; i < sp->bar->nbins; i++) {
    bin = &sp->bar->bins[i];
    if (bin->nhidden) {
      gdk_draw_rectangle (rawsp->pixmap0, gg->plot_GC, TRUE,
                          bin->rect.x, bin->rect.y, bin->rect.width,
                          bin->rect.height + 1);
    }
  }
/* */
  for (j = 0; j < sp->bar->ncolors; j++) {
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[j]);

    for (i = 0; i < sp->bar->nbins; i++) {
      bin = &sp->bar->cbins[i][j];
      if (bin->count > 0) {
        gdk_draw_rectangle (rawsp->pixmap0, gg->plot_GC, TRUE,
                            bin->rect.x, bin->rect.y, bin->rect.width,
                            bin->rect.height);
      }
    }
  }

/* draw overflow bins if necessary */
  if (sp->bar->high_pts_missing) {
    /*  start with the hiddens */
    if (sp->bar->high_bin->nhidden) {
      bin = sp->bar->high_bin;
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);
      gdk_draw_rectangle (rawsp->pixmap0, gg->plot_GC, TRUE,
                          bin->rect.x, bin->rect.y, bin->rect.width,
                          bin->rect.height + 1);
    }
    for (j = 0; j < sp->bar->ncolors; j++) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[j]);
      bin = &sp->bar->col_high_bin[j];
      if (bin->count > 0)
        gdk_draw_rectangle (rawsp->pixmap0, gg->plot_GC, TRUE,
                            bin->rect.x, bin->rect.y, bin->rect.width,
                            bin->rect.height);
    }
  }
  if (sp->bar->low_pts_missing) {
    /*  start with the hiddens */
    if (sp->bar->low_bin->nhidden) {
      bin = sp->bar->low_bin;
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);
      gdk_draw_rectangle (rawsp->pixmap0, gg->plot_GC, TRUE,
                          bin->rect.x, bin->rect.y, bin->rect.width,
                          bin->rect.height + 1);
    }
    for (j = 0; j < sp->bar->ncolors; j++) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[j]);
      bin = &sp->bar->col_low_bin[j];
      if (bin->count > 0)
        gdk_draw_rectangle (rawsp->pixmap0, gg->plot_GC, TRUE,
                            bin->rect.x, bin->rect.y, bin->rect.width,
                            bin->rect.height);
    }
  }

/* mark empty bins with a small circle */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  for (i = 0; i < sp->bar->nbins; i++) {
    bin = &sp->bar->bins[i];
    if (bin->count == 0) {
      radius = bin->rect.height / 4;
      gdk_draw_line (rawsp->pixmap0, gg->plot_GC,
                     bin->rect.x, bin->rect.y,
                     bin->rect.x, bin->rect.y + bin->rect.height);
      gdk_draw_arc (rawsp->pixmap0, gg->plot_GC, FALSE,
                    bin->rect.x - radius / 2,
                    bin->rect.y + bin->rect.height / 2 - radius / 2,
                    radius, radius, 0, 64 * 360);
    }
  }

  return (false);
}

void
barchart_splot_add_plot_labels (splotd * sp, GdkDrawable * drawable,
                                ggobid * gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gint i = 0;
  GGobiStage *d = display->d;
  PangoLayout *layout =
    gtk_widget_create_pango_layout (GTK_WIDGET (sp->da), NULL);
  PangoRectangle rect;
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);
  GGobiVariable *var = ggobi_stage_get_variable(d, sp->p1dvar);

  layout_text (layout, ggobi_variable_get_name(var), &rect);
  gdk_draw_layout (drawable, gg->plot_GC, sp->max.x - rect.width - 5,
                   sp->max.y - rect.height - 5, layout);
  
  if(!GGOBI_VARIABLE_IS_CATEGORICAL(var)) {
    g_object_unref (G_OBJECT (layout));
    return;
  }

  /* is there enough space for labels? If not - return */
  layout_text (layout, "  ", &rect);
  if (!bsp->bar->is_spine && bsp->bar->bins[1].rect.height < rect.height)
    return;

  for (i = 0; i < bsp->bar->nbins; i++) {
    const gchar* name = ggobi_variable_get_level_name(var, bsp->bar->bins[i].value);
    layout_text (layout, name, NULL);
    gdk_draw_layout (drawable, gg->plot_GC,
                     bsp->bar->bins[i].rect.x + 2,
                     bsp->bar->bins[i].rect.y +
                     bsp->bar->bins[i].rect.height / 2 + 2, layout);

  }
  g_object_unref (G_OBJECT (layout));
}

void
barchart_set_breakpoints (gfloat width, barchartSPlotd * sp, GGobiStage * d)
{
  gfloat rdiff;
  gint i, nbins;
  splotd *rawsp = GGOBI_SPLOT (sp);

  rdiff = rawsp->p1d.lim.max - rawsp->p1d.lim.min;

  nbins = (gint) (rdiff / width + 1);

  sp->bar->new_nbins = nbins;
  barchart_allocate_structure (sp, d);

  for (i = 0; i <= sp->bar->nbins; i++) {
    sp->bar->breaks[i] = rawsp->p1d.lim.min + width * i;
    sp->bar->old_bar_hit[i] = FALSE;
    sp->bar->bar_hit[i] = FALSE;
  }

}

void
barchart_set_initials (barchartSPlotd * sp, GGobiStage * d)
{
  splotd *rawsp = GGOBI_SPLOT (sp);
  gint i, *values;
  GGobiVariable *var = ggobi_stage_get_variable(d, rawsp->p1dvar);

  // if continuous, break up into nbins equal ranges
  if (!GGOBI_VARIABLE_IS_CATEGORICAL(var)) {
    gint i;
    gfloat rdiff = rawsp->p1d.lim.max - rawsp->p1d.lim.min;

    for (i = 0; i < sp->bar->nbins; i++) {
      sp->bar->breaks[i] = rawsp->p1d.lim.min + rdiff / sp->bar->nbins * i;
    }
    sp->bar->breaks[sp->bar->nbins] = rawsp->p1d.lim.max;
    return;
  }
  
  // if categorical, use the level value
  values = ggobi_variable_get_level_values(var);
  for (i = 0; i < ggobi_variable_get_n_levels(var); i++) {
    sp->bar->bins[i].value = values[i];
  }
  g_free(values);
  return;

}

void
barchart_recalc_counts (barchartSPlotd * sp, GGobiStage * d, ggobid * gg)
{
  gfloat yy;
  gint i, bin;
  splotd *rawsp = GGOBI_SPLOT (sp);

  if (sp->bar->index_to_rank.nels != d->n_rows) {
    vectori_realloc (&sp->bar->index_to_rank, d->n_rows);
    barchart_init_categorical (sp, d);
  }

  if (!GGOBI_STAGE_IS_COL_CATEGORICAL(d, rawsp->p1dvar))
    rawsp->scale.y = 1 - (1 - SCALE_DEFAULT) / 2;

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->bins[i].count = 0;
    sp->bar->bins[i].nhidden = 0;
  }

  sp->bar->high_pts_missing = sp->bar->low_pts_missing = FALSE;

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  if (GGOBI_STAGE_IS_COL_CATEGORICAL(d, rawsp->p1dvar)) {

    for (i = 0; i < d->n_rows; i++) {
      /*-- skip missings?  --*/
      if (!d->missings_show_p && ggobi_stage_is_missing(d, i, rawsp->p1dvar))
        continue;

      bin = sp->bar->index_to_rank.els[i];
      if ((bin >= 0) && (bin < sp->bar->nbins)) {
        sp->bar->bins[bin].count++;
        if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, i))
          sp->bar->bins[bin].nhidden++;
      }
      rawsp->planar[i].x = (greal) sp->bar->bins[bin].value;
    }
  } else {                        /* all vartypes but categorical */
    gint index, rank = 0;

    index = sp->bar->index_to_rank.els[rank];
    yy = ggobi_stage_get_raw_value(d, index, rawsp->p1dvar);

    while ((yy < sp->bar->breaks[0] + sp->bar->offset) &&
           (rank < d->n_rows - 1)) {
      rawsp->planar[index].x = -1;
      rank++;
      index = sp->bar->index_to_rank.els[rank];
      yy = ggobi_stage_get_raw_value(d, index, rawsp->p1dvar);
    }

    if (rank > 0) {
      gint k;
      sp->bar->low_pts_missing = TRUE;
      if (sp->bar->low_bin == NULL)
        sp->bar->low_bin = (gbind *) g_malloc (sizeof (gbind));
      if (sp->bar->col_low_bin == NULL)
        sp->bar->col_low_bin =
          (gbind *) g_malloc (sp->bar->ncolors * sizeof (gbind));
      sp->bar->low_bin->count = rank;
      /*-- count the hiddens among the elements in low_bin --*/
      sp->bar->low_bin->nhidden = 0;
      for (k = 0; k < rank; k++) {
        index = sp->bar->index_to_rank.els[k];
        if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, index))
          sp->bar->low_bin->nhidden++;
      }
    }

    bin = 0;
    while (rank < d->n_rows) {
      index = sp->bar->index_to_rank.els[rank];
      yy = ggobi_stage_get_raw_value(d, index, rawsp->p1dvar);
      while ((bin < sp->bar->nbins) &&
             (sp->bar->breaks[bin + 1] + sp->bar->offset < yy)) {
        bin++;
      }

      if (bin > sp->bar->nbins - 1) {
/* check whether the value is the maximum, if so, add it to the last bin -
   slight inconsistency with histograms */
        if (yy == sp->bar->breaks[sp->bar->nbins] + sp->bar->offset) {
          bin--;
          sp->bar->bins[bin].count++;
          if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, index))
            sp->bar->bins[bin].nhidden++;
        }
        else {
          if (sp->bar->high_pts_missing == FALSE) {
            sp->bar->high_pts_missing = TRUE;
            if (sp->bar->high_bin == NULL)
              sp->bar->high_bin = (gbind *) g_malloc (sizeof (gbind));
            if (sp->bar->col_high_bin == NULL) {
              sp->bar->col_high_bin = (gbind *)
                g_malloc (sp->bar->ncolors * sizeof (gbind));
            }
            sp->bar->high_bin->count = 0;
            sp->bar->high_bin->nhidden = 0;
          }
          sp->bar->high_bin->count++;
          if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, index))
            sp->bar->high_bin->nhidden++;
        }
      }
      else {
        sp->bar->bins[bin].count++;
        if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, index))
          sp->bar->bins[bin].nhidden++;
      }
      rawsp->planar[index].x = bin;
      rank++;
    }
  }
  if (sp->bar->low_pts_missing == FALSE) {
    if (sp->bar->low_bin != NULL)
      g_free ((gpointer) (sp->bar->low_bin));
    if (sp->bar->col_low_bin != NULL)
      g_free ((gpointer) (sp->bar->col_low_bin));
    sp->bar->low_bin = NULL;
    sp->bar->col_low_bin = NULL;
  }
  if (sp->bar->high_pts_missing == FALSE) {
    if (sp->bar->high_bin != NULL)
      g_free ((gpointer) (sp->bar->high_bin));
    if (sp->bar->col_high_bin != NULL)
      g_free ((gpointer) (sp->bar->col_high_bin));
    sp->bar->high_bin = NULL;
    sp->bar->col_high_bin = NULL;
  }

  barchart_recalc_dimensions (GGOBI_SPLOT (sp), d, gg);
}

void
barchart_recalc_dimensions (splotd * rawsp, GGobiStage * d, ggobid * gg)
{
  gint i, maxbincount = 0, maxbin = -1;
  gfloat precis = PRECISION1;

  gfloat scale_y;
  gint index;
  gint minwidth = 0;
  gfloat rdiff, ftmp;
  gbind *bin;

  GdkRectangle *rect;
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (rawsp);

  scale_y = rawsp->scale.y;

  /*
   * Calculate is, a scale factor.  Scale so as to use the entire
   * plot window (well, as much of the plot window as scale.x and
   * scale.y permit.)
   */

  rdiff = rawsp->p1d.lim.max - rawsp->p1d.lim.min;
  
  index = 0;
  for (i = 0; i < sp->bar->nbins; i++) {
    bin = &sp->bar->bins[i];
    if (bin->count > maxbincount) {
      maxbincount = bin->count;
      maxbin = i;
    }

    sp->bar->bins[i].planar.x = -1;
    if (GGOBI_STAGE_IS_COL_CATEGORICAL(d, rawsp->p1dvar)) {
      ftmp = -1.0 + 2.0 * ((greal) bin->value - rawsp->p1d.lim.min)
        / rdiff;
      bin->planar.y = (greal) (PRECISION1 * ftmp);
    }
    else {
      ftmp = -1.0 + 2.0 * (sp->bar->breaks[i] - sp->bar->breaks[0]) / rdiff;
      bin->planar.y = (glong) (precis * ftmp);
    }
  }
  sp->bar->maxbincounts = maxbincount;

  if (!sp->bar->is_spine) {
    greal precis = (greal) PRECISION1;
    greal gtmp;
    gbind *binminus;

    scale_y /= 2;

    rawsp->iscale.y = (greal) (-1 * (gfloat) rawsp->max.y * scale_y);

    minwidth = rawsp->max.y;
    for (i = 0; i < sp->bar->nbins; i++) {
      bin = &sp->bar->bins[i];
      rect = &sp->bar->bins[i].rect;

      gtmp = bin->planar.y - rawsp->pmid.y;
      rect->y = (gint) (gtmp * rawsp->iscale.y / precis);

      rect->x = 10;
      rect->y += (rawsp->max.y / 2);
      if (i == 0)
        minwidth = 2 * (rawsp->max.y - rect->y);
      if (i > 0) {
        binminus = &sp->bar->bins[i - 1];
        minwidth = MIN (minwidth, binminus->rect.y - rect->y - 2);
        binminus->rect.height = binminus->rect.y - rect->y - 2;
      }

      rect->width = MAX (1, (gint) ((gfloat) (rawsp->max.x - 2 * rect->x)
                                    * bin->count / sp->bar->maxbincounts));

    }
    sp->bar->bins[sp->bar->nbins - 1].rect.height =
      sp->bar->bins[sp->bar->nbins - 2].rect.y -
      sp->bar->bins[sp->bar->nbins - 1].rect.y - 1;

/* set overflow bins to the left and right */
    if (sp->bar->low_pts_missing) {
      gbind *lbin = sp->bar->low_bin;
      lbin->rect.height = minwidth;
      lbin->rect.x = 10;
      lbin->rect.width = MAX (1, (gint) ((gfloat)
                                         (rawsp->max.x - 2 * lbin->rect.x)
                                         * lbin->count /
                                         sp->bar->maxbincounts));
      lbin->rect.y = sp->bar->bins[0].rect.y + 2;
    }

    if (sp->bar->high_pts_missing) {
      gbind *hbin = sp->bar->high_bin;
      hbin->rect.height = sp->bar->bins[0].rect.height;
      hbin->rect.x = 10;
      hbin->rect.width = MAX (1, (gint) ((gfloat)
                                         (rawsp->max.x - 2 * hbin->rect.x)
                                         * hbin->count /
                                         sp->bar->maxbincounts));
      i = sp->bar->nbins - 1;
      hbin->rect.y =
        sp->bar->bins[i].rect.y - 2 * sp->bar->bins[i].rect.height - 1;
    }

    minwidth = MAX ((gint) (0.9 * minwidth), 0);
    for (i = 0; i < sp->bar->nbins; i++) {
      if (!GGOBI_STAGE_IS_COL_CATEGORICAL(d, rawsp->p1dvar))
        sp->bar->bins[i].rect.y -= sp->bar->bins[i].rect.height;
      else {
        sp->bar->bins[i].rect.height = minwidth;
        sp->bar->bins[i].rect.y -= minwidth / 2;
      }
    }
  }
  else {                        /* spine plot representation */
    GdkRectangle *rect;
    gint bindist = 2;           /* distance between two bins */
    gint maxheight;
    gint yoffset;
    gint n = d->n_rows;

    scale_y = 1 - (1 - SCALE_DEFAULT) / 2;
    maxheight = (rawsp->max.y - (sp->bar->nbins - 1) * bindist) * scale_y;
    yoffset = (gint) (rawsp->max.y * .5 * (1 + scale_y));

    for (i = 0; i < sp->bar->nbins; i++) {
      rect = &sp->bar->bins[i].rect;
      rect->x = 10;
      rect->width = rawsp->max.x - 2 * rect->x;

      rect->height = (gint) ((gfloat) sp->bar->bins[i].count / n * maxheight);
      rect->y = yoffset;
      yoffset -= (rect->height + bindist);
    }

    minwidth = (gint) (0.9 * minwidth);
    for (i = 0; i < sp->bar->nbins; i++) {
      sp->bar->bins[i].rect.y -= sp->bar->bins[i].rect.height;
    }

/* draw overflow bins */

    if (sp->bar->high_pts_missing) {
      sp->bar->high_bin->rect.width = rawsp->max.x - 2 * 10; //10=rect->x;
      sp->bar->high_bin->rect.x = 10;
      sp->bar->high_bin->rect.height =
        (gint) ((gfloat) sp->bar->high_bin->count / n * maxheight);
      i = sp->bar->nbins - 1;
      sp->bar->high_bin->rect.y =
        (gint) (rawsp->max.y * .5 * (1 - scale_y)) -
        sp->bar->high_bin->rect.height - 2;
    }
    if (sp->bar->low_pts_missing) {
      sp->bar->low_bin->rect.x = 10;
      sp->bar->low_bin->rect.width = rawsp->max.x - 2 * 10; //10=rect->x;
      sp->bar->low_bin->rect.height =
        (gint) ((gfloat) sp->bar->low_bin->count / n * maxheight);
      sp->bar->low_bin->rect.y =
        (gint) (rawsp->max.y * .5 * (1 + scale_y)) + 2;
    }
  }
}

gboolean
barchart_active_paint_points (splotd * rawsp, GGobiStage * d, ggobid * gg)
{
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (rawsp);
  brush_coords *brush_pos = &rawsp->brush_pos;
  gint i, indx;
  GdkRectangle brush_rect;
  GdkRectangle dummy;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);
  gboolean *hits;
  cpaneld *cpanel = &gg->current_display->cpanel;

  hits = (gboolean *) g_malloc ((sp->bar->nbins + 2) * sizeof (gboolean));

  brush_rect.x = x1;
  brush_rect.y = y1;
  brush_rect.width = x2 - x1;
  brush_rect.height = y2 - y1;

  for (i = 0; i < sp->bar->nbins; i++) {
    hits[i + 1] = rect_intersect (&sp->bar->bins[i].rect, &brush_rect,
                                  &dummy);
  }
  if (sp->bar->high_pts_missing)
    hits[sp->bar->nbins + 1] =
      rect_intersect (&sp->bar->high_bin->rect, &brush_rect, &dummy);
  else
    hits[sp->bar->nbins + 1] = FALSE;

  if (sp->bar->low_pts_missing)
    hits[0] = rect_intersect (&sp->bar->low_bin->rect, &brush_rect, &dummy);
  else
    hits[0] = FALSE;

  for(guint i = 0; i < d->nrows_under_brush; i++)
    d->rows_under_brush_prev.els[i] = d->rows_under_brush.els[i];
  d->nrows_under_brush_prev = d->nrows_under_brush;
  d->nrows_under_brush = 0;

  for (i = 0; i < d->n_rows; i++) {
    /*-- skip missings?  --*/
    if (!d->missings_show_p && ggobi_stage_is_missing(d, i, rawsp->p1dvar))
      continue;

    GGOBI_STAGE_ATTR_INIT_ALL(d);  
    if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, i) &&
        (cpanel->br.point_targets != br_shadow
         && cpanel->br.point_targets != br_unshadow)) {
      continue;
    }

    /*-- dfs -- this seems to assume that the values of planar begin at 0,
         which may not be true ... this change makes it work for categorical,
         but breaks it otherwise --*/
    if (GGOBI_STAGE_IS_COL_CATEGORICAL(d, rawsp->p1dvar)) {
      indx = (gint) (rawsp->planar[i].x - rawsp->p1d.lim.min + 1);
    }
    else {
      indx = (gint) (rawsp->planar[i].x + 1);
    }

    if (hits[indx])
      d->rows_under_brush.els[d->nrows_under_brush++] = i;
  }

  g_free ((gpointer) hits);

  return d->nrows_under_brush;
}

static ggobid *CurrentGGobi = NULL;

gint
barpsort (const void *arg1, const void *arg2)
{
  ggobid *gg = CurrentGGobi;

  gint val = 0;
  gint *x1 = (gint *) arg1;
  gint *x2 = (gint *) arg2;

  if (gg->p1d.gy[*x1] == gg->p1d.gy[*x2])
    return 0;
/* to speed things up for categorical variables */

  if (gg->p1d.gy[*x1] < gg->p1d.gy[*x2])
    val = -1;
  else if (gg->p1d.gy[*x1] > gg->p1d.gy[*x2])
    val = 1;

  return (val);
}


gfloat
barchart_sort_index (gfloat * yy, gint ny, ggobid * gg, barchartSPlotd * sp)
{
  gint i, *indx;
  gint rank;
  gfloat mindist = 0.0;

  indx = (gint *) g_malloc (ny * sizeof (gint));

/*
 * gy is needed solely for the psort routine:  psort is used by
 * qsort to put an index vector in the order that yy will assume.
*/
  gg->p1d.gy = (gfloat *) g_malloc (ny * sizeof (gfloat));
  for (i = 0; i < ny; i++) {
    indx[i] = i;
    gg->p1d.gy[i] = yy[i];
  }
  CurrentGGobi = gg;

  qsort ((void *) indx, (gsize) ny, sizeof (gint), barpsort);

  CurrentGGobi = NULL;
/*
 * Bug here:  this is screwy if ny < 4.
*/
  if (sp->bar->is_histogram) {  /* vartype != categorical */
    mindist = 0;

    for (i = 0; i < ny; i++) {
      sp->bar->index_to_rank.els[i] = indx[i];
    }

  }
  else {                        /* vartype = categorical */

/* dfs */
    /* XXX 
       Later, when labelling, if a value doesn't match one of the
       level_values, label it 'missing'
     */
/* assumption:  there exist at least two bins */
    mindist = sp->bar->bins[1].value - sp->bar->bins[0].value;
    for (i = 1; i < sp->bar->nbins; i++)
      mindist = MIN (mindist,
                     sp->bar->bins[i].value - sp->bar->bins[i - 1].value);

    rank = 0;
    /*-- there are bin values that don't exist in the data --*/
    while (yy[indx[0]] > sp->bar->bins[rank].value)
      rank++;

    for (i = 0; i < sp->bar->nbins; i++)
      sp->bar->bins[i].index = -1;

    for (i = 0; i < ny; i++) {

      if (i > 0) {
        if (yy[indx[i]] != yy[indx[i - 1]]) {
          rank++;

          while (yy[indx[i]] > sp->bar->bins[rank].value) {
            rank++;
          }

          sp->bar->bins[rank].index = indx[i];  /* do I care? */
        }
      }

      /* This takes me from index to bin -- dfs */
      sp->bar->index_to_rank.els[indx[i]] = rank;
    }


  }

  g_free ((gpointer) (gg->p1d.gy));
  g_free ((gpointer) (indx));

  return mindist;
}

void
barchart_default_visual_cues_draw (splotd * rawsp, GdkDrawable * drawable,
                                   ggobid * gg)
{
  displayd *display = gg->current_display;
  GGobiStage *d = display->d;
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (rawsp);
  GdkPoint btn[4];

  if(GGOBI_STAGE_IS_COL_CATEGORICAL(d, GGOBI_SPLOT(sp)->p1dvar))
    return;

  /* Experiment: ontinue to draw small triangular buttons, but allow
     the regions to grow into long rectangles running along the bars.
     
      */
  
  /* calculate & draw anchor_rgn */
  gint y = sp->bar->bins[0].rect.y + sp->bar->bins[0].rect.height;
  gint x = sp->bar->bins[0].rect.x;
  gint halfwidth = sp->bar->bins[0].rect.height / 2 - 2;

  if (halfwidth <= 0)
      halfwidth = 1;
  
  sp->bar->anchor_rgn[0].x = sp->bar->anchor_rgn[1].x = x - 5;
  sp->bar->anchor_rgn[2].x = x + GGOBI_SPLOT (sp)->max.x; // extend
  sp->bar->anchor_rgn[0].y = y + halfwidth;
  sp->bar->anchor_rgn[1].y = y - halfwidth;
  //sp->bar->anchor_rgn[2].y = y;

  // Rectangle instead of triangle
  sp->bar->anchor_rgn[3].x = sp->bar->anchor_rgn[2].x;
  sp->bar->anchor_rgn[2].y = sp->bar->anchor_rgn[1].y;
  sp->bar->anchor_rgn[3].y = sp->bar->anchor_rgn[0].y;

  btn[0].x = btn[1].x = x - 5;
  btn[2].x = x;
  btn[0].y = y + halfwidth;
  btn[1].y = y - halfwidth;
  btn[2].y = y;
  button_draw_with_shadows (btn, drawable, gg);
  //button_draw_with_shadows(sp->bar->anchor_rgn, drawable, gg);

  /* calculate & draw offset_rgn */
  y = sp->bar->bins[0].rect.y;
  sp->bar->offset_rgn[0].x = sp->bar->offset_rgn[1].x = x - 5;
  sp->bar->offset_rgn[2].x = x + GGOBI_SPLOT (sp)->max.x; // extend
  sp->bar->offset_rgn[0].y = y + halfwidth;
  sp->bar->offset_rgn[1].y = y - halfwidth;
  //sp->bar->offset_rgn[2].y = y;

  // Rectangle instead of triangle -- dfs
  sp->bar->offset_rgn[3].x = sp->bar->offset_rgn[2].x;
  sp->bar->offset_rgn[2].y = sp->bar->offset_rgn[1].y;
  sp->bar->offset_rgn[3].y = sp->bar->offset_rgn[0].y;


  btn[0].x = btn[1].x = x - 5;
  btn[2].x = x;
  btn[0].y = y + halfwidth;
  btn[1].y = y - halfwidth;
  btn[2].y = y;
  button_draw_with_shadows (btn, drawable, gg);
  //button_draw_with_shadows(sp->bar->offset_rgn, drawable, gg);
}

void
button_draw_with_shadows (GdkPoint * region, GdkDrawable * drawable,
                          ggobid * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  /*gdk_gc_set_foreground(gg->plot_GC, &gg->wvis.gray3); */
  gdk_gc_set_foreground (gg->plot_GC, &gg->lightgray);
  gdk_draw_polygon (drawable, gg->plot_GC, TRUE, region, 3);

/* dark shadows */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);

  gdk_draw_polygon (drawable, gg->plot_GC, FALSE, region, 3);
  gdk_draw_line (drawable, gg->plot_GC, region[0].x, region[2].y,
                 region[2].x, region[2].y);

/* light shadows */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);

  gdk_draw_line (drawable, gg->plot_GC, region[0].x, region[0].y,
                 region[1].x, region[1].y);
  gdk_draw_line (drawable, gg->plot_GC, region[1].x, region[1].y,
                 region[2].x, region[2].y);
  gdk_draw_line (drawable, gg->plot_GC, region[0].x, region[2].y + 1,
                 region[2].x, region[2].y + 1);
}

gboolean
rect_intersect (GdkRectangle * rect1, GdkRectangle * rect2,
                GdkRectangle * dest)
{
  gint right, bottom;
  icoords pt;

// horizontal intersection
  pt.x = dest->x = MAX (rect1->x, rect2->x);
  right = MIN (rect1->x + rect1->width, rect2->x + rect2->width);
  dest->width = MAX (0, right - dest->x);

// vertical intersection
  pt.y = dest->y = MAX (rect1->y, rect2->y);
  bottom = MIN (rect1->y + rect1->height, rect2->y + rect2->height);
  dest->height = MAX (0, bottom - dest->y);

  return (pt_in_rect (pt, *rect1) && pt_in_rect (pt, *rect2));
}

gboolean
pt_in_rect (icoords pt, GdkRectangle rect)
{
  return ((pt.x >= rect.x) && (pt.x <= rect.x + rect.width)
          && (pt.y >= rect.y) && (pt.y <= rect.y + rect.height));
}


/* Cues that are drawn in the default mode, indicating that the
 * binwidth and anchor point can be changed. */
void
barchart_add_bar_cues (splotd * rawsp, GdkDrawable * drawable, ggobid * gg)
{
  displayd *display = rawsp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  if (cpanel->imode != DEFAULT_IMODE)
    return;

  barchart_default_visual_cues_draw (rawsp, drawable, gg);
}


gboolean
barchart_identify_bars (icoords mousepos, splotd * rawsp, GGobiStage * d,
                        ggobid * gg)
{
/* returns 0 if nothing has changed from the last time */
/*         1 if different bars are hit */
  gint i, nbins;
  gboolean stop;
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (rawsp);
  nbins = sp->bar->nbins;

  /* check, which bars are hit */
  if (sp->bar->low_pts_missing)
    sp->bar->bar_hit[0] = pt_in_rect (mousepos, sp->bar->high_bin->rect);
  else
    sp->bar->bar_hit[0] = FALSE;

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->bar_hit[i + 1] = pt_in_rect (mousepos, sp->bar->bins[i].rect);
  }

  if (sp->bar->high_pts_missing)
    sp->bar->bar_hit[nbins + 1] =
      pt_in_rect (mousepos, sp->bar->high_bin->rect);
  else
    sp->bar->bar_hit[nbins + 1] = FALSE;


/* are those bars the same as last time? */
  stop = FALSE;

  if (sp->bar->old_nbins == sp->bar->nbins) {
    for (i = 0; (i < nbins + 2) && !stop; i++)
      stop = (sp->bar->bar_hit[i] != sp->bar->old_bar_hit[i]);

  }
  else {
    sp->bar->old_nbins = sp->bar->nbins;
  }

  sp->bar->same_hits = !stop;

  if (!stop)
    return FALSE;               /* nothing else needs to be changed */

/* set old bar hits to match the new results */
  for (i = 0; i < nbins + 2; i++)
    sp->bar->old_bar_hit[i] = sp->bar->bar_hit[i];

  return TRUE;
}

splotd *
ggobi_barchart_splot_new (displayd * dpy, ggobid * gg)
{
  barchartSPlotd *bsp;
  splotd *sp;

  bsp = g_object_new (GGOBI_TYPE_BARCHART_SPLOT, NULL);
  sp = GGOBI_SPLOT (bsp);

  splot_init (sp, dpy, gg);
  barchart_clean_init (bsp);
  barchart_recalc_counts (bsp, dpy->d, gg);

  return (sp);
}

/**
 Called when we create the barchart.
*/
void
barchart_cpanel_init (cpaneld * cpanel, ggobid * gg)
{
  cpanel->imode = DEFAULT_IMODE;
  cpanel->pmode = EXTENDED_DISPLAY_PMODE;
  cpanel->barchart_display_mode = 0;

  /*-- 1d plots --*/
  cpanel_p1d_init (cpanel, gg);

  /*-- available modes --*/
  cpanel_brush_init (cpanel, gg);
  cpanel_identify_init (cpanel, gg);
}
