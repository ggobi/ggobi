/* barchart.c */
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
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "gtkext.h"
#include "externs.h"

#include "barchartDisplay.h"

#define WIDTH   370
#define HEIGHT  370

gfloat barchart_sort_index(gfloat * yy, gint ny, ggobid * gg,
                           barchartSPlotd * sp);
void barchart_init_categorical(barchartSPlotd * sp, datad * d);
void barchart_set_initials(barchartSPlotd * sp, datad * d);
void rectangle_inset(gbind * bin);
void barchart_allocate_structure(barchartSPlotd * sp, datad * d);
void button_draw_with_shadows(GdkPoint * region, GdkDrawable * drawable,
                              ggobid * gg);

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static const GtkItemFactoryEntry menu_items[] = {
  {"/_File",
   NULL,
   NULL,
   0,
   "<Branch>"},
  {"/File/Print",
   "",
   (GtkItemFactoryCallback) display_print_cb,
   0,
   "<Item>"},
  {"/File/sep",
   NULL,
   NULL,
   0,
   "<Separator>"},
  {"/File/Close",
   "",
   (GtkItemFactoryCallback) display_close_cb,
   0,
   "<Item>"},
};


displayd *createBarchart(gboolean missing_p, splotd * sp, gint var, datad * d, ggobid * gg);

displayd *
barchart_new(gboolean missing_p, splotd * sp, datad * d, ggobid * gg)
{
  return(createBarchart(missing_p, sp, -1, d, gg));
}

displayd *
barchart_new_with_vars(gboolean missing_p, gint nvars, gint *vars, datad * d, ggobid * gg)
{
 return(createBarchart(missing_p, NULL, vars ? vars[0] : 0, d, gg));
}



displayd *
createBarchart(gboolean missing_p, splotd * sp, gint var, datad * d,
  ggobid * gg)
{
  GtkWidget *table, *vbox;
  displayd *display;

  if (d == NULL || d->ncols < 1)
    return (NULL);

  if (sp == NULL || sp->displayptr == NULL) {
    /* Use GTK_TYPE_GGOBI_BARCHART_DISPLAY, or the regular extended display
       and set the titleLabel immediately afterward. If more goes into barchart,
       we will do the former. And that's what we do.
       The alternative is.
       display = gtk_type_new(GTK_TYPE_GGOBI_EXTENDED_DISPLAY);
       GTK_GGOBI_EXTENDED_DISPLAY(display)->titleLabel = "BarChart";
     */
    display = gtk_type_new(GTK_TYPE_GGOBI_BARCHART_DISPLAY);
    display_set_values(display, d, gg);
  } else {
    display = (displayd *) sp->displayptr;
    display->d = d;
  }

  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
     display->options.axes_center_p = true;
   */

  barchart_cpanel_init(&display->cpanel, gg);

  display_window_init(GTK_GGOBI_WINDOW_DISPLAY(display), 3, gg);   /*-- 3 = width = any small int --*/

  /*-- Add the main menu bar --*/
  vbox = GTK_WIDGET(display);  
  gtk_container_border_width(GTK_CONTAINER(vbox), 1);
  gtk_container_add(GTK_CONTAINER
                    (GTK_GGOBI_WINDOW_DISPLAY(display)->window), vbox);

  gg->app.sp_accel_group = gtk_accel_group_new();
  get_main_menu((GtkItemFactoryEntry *) menu_items,
                sizeof(menu_items) / sizeof(menu_items[0]),
                gg->app.sp_accel_group,
                GTK_GGOBI_WINDOW_DISPLAY(display)->window,
                &display->menubar, (gpointer) display);
  /*
   * After creating the menubar, and populating the file menu,
   * add the other menus manually
   */
  barchart_display_menus_make(display, gg->app.sp_accel_group,
                              (GtkSignalFunc) display_options_cb, gg);

  gtk_box_pack_start(GTK_BOX(vbox), display->menubar, false, true, 0);


  /*-- Initialize a single splot --*/
  if (sp == NULL) {
    sp = gtk_barchart_splot_new(display, WIDTH, HEIGHT, gg);
  }


  display->splots = NULL;
  display->splots = g_list_append(display->splots, (gpointer) sp);

  table = gtk_table_new(3, 2, false);   /* rows, columns, homogeneous */
  gtk_box_pack_start(GTK_BOX(vbox), table, true, true, 0);
  gtk_table_attach(GTK_TABLE(table),
                   sp->da, 1, 2, 0, 1,
                   (GtkAttachOptions) (GTK_SHRINK | GTK_EXPAND | GTK_FILL),
                   (GtkAttachOptions) (GTK_SHRINK | GTK_EXPAND | GTK_FILL),
                   0, 0);



  /*
   * The horizontal ruler goes on top. As the mouse moves across the
   * drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
   */
  display->hrule = gtk_ext_hruler_new();
#ifndef GTK_2_0
  gtk_signal_connect_object(GTK_OBJECT(sp->da), "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD(display->hrule, motion_notify_event),
    GTK_OBJECT(display->hrule));
#else
  display->hrule = gtk_hruler_new ();
   /* What about the events above. */
#endif

  gtk_table_attach(GTK_TABLE(table),
                   display->hrule, 1, 2, 1, 2,
                   (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                   (GtkAttachOptions) GTK_FILL, 0, 0);


  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
   */
  display->vrule = gtk_ext_vruler_new();
#ifndef GTK_2_0
  gtk_signal_connect_object(GTK_OBJECT(sp->da),
    "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD(display->vrule, motion_notify_event),
    GTK_OBJECT(display->vrule));
#else
  display->vrule = gtk_vruler_new ();
   /* What about the events above. */
#endif

  gtk_table_attach(GTK_TABLE(table),
                   display->vrule, 0, 1, 0, 1,
                   (GtkAttachOptions) GTK_FILL,
                   (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                   0, 0);


  gtk_widget_show_all(GTK_GGOBI_WINDOW_DISPLAY(display)->window);

  /*-- hide any extraneous rulers --*/

  display->p1d_orientation = VERTICAL;
  scatterplot_show_rulers(display, P1PLOT);
  ruler_ranges_set(true, display, sp, gg);

  return display;
}

void barchart_clean_init(barchartSPlotd * sp)
{
  displayd *display;
  datad *d;
  gint i, j;

  display = (displayd *) GTK_GGOBI_SPLOT(sp)->displayptr;
  d = display->d;

  sp->bar->nbins = -1;

  sp->bar->new_nbins = -1;
  barchart_allocate_structure(sp, d);

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->bins[i].count = 0;
    for (j = 0; j < sp->bar->ncolors; j++) {
      sp->bar->cbins[i][j].count = 0;
      sp->bar->cbins[i][j].rect.width = 1;
    }
  }
  for (i = 0; i < sp->bar->nbins + 2; i++)
    sp->bar->bar_hit[i] = false;
/* */

  barchart_set_initials(sp, d);
  sp->bar->offset = 0;
  GTK_GGOBI_SPLOT(sp)->pmid.y = 0;

  if (sp->bar->index_to_rank) {
    g_free((gpointer) sp->bar->index_to_rank);
  }
  sp->bar->index_to_rank =
    (gint *) g_malloc(d->nrows_in_plot * sizeof(gint));
  barchart_init_categorical(sp, d);
}

static void
barchart_recalc_group_counts(barchartSPlotd * sp, datad * d, ggobid * gg)
{
  gint i, j, m, bin;
/* dfs */
  vartabled *vtx = vartable_element_get(GTK_GGOBI_SPLOT(sp)->p1dvar, d);
/* --- */

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
  for (i = 0; i < d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];

    /*-- skip missings?  --*/
    if (d->nmissing > 0 && !d->missings_show_p
        && MISSING_P(m, GTK_GGOBI_SPLOT(sp)->p1dvar))
      continue;

    /*-- skip hiddens? --*/
    if (d->hidden_now.els[m]) {
      continue;
    }

    bin = GTK_GGOBI_SPLOT(sp)->planar[m].x;
/* dfs */
    if (vtx->vartype == categorical)
      bin = sp->bar->index_to_rank[i];
/* --- */
    if ((bin >= 0) && (bin < sp->bar->nbins)) {
      sp->bar->cbins[bin][d->color_now.els[m]].count++;
    }
    if (bin == -1) {
      sp->bar->col_low_bin[d->color_now.els[m]].count++;
    } else if (bin == sp->bar->nbins) {
      sp->bar->col_high_bin[d->color_now.els[m]].count++;
    }
  }

  barchart_recalc_group_dimensions(sp, gg);
}



void barchart_recalc_group_dimensions(barchartSPlotd * sp, ggobid * gg)
{
  gint colorwidth, i, j, xoffset;

  for (i = 0; i < sp->bar->nbins; i++) {
    xoffset = sp->bar->bins[i].rect.x;

/* first color in all bins is the current color */
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
      rectangle_inset(&sp->bar->cbins[i][j]);
    }
    xoffset += colorwidth;

/* then all other colors follow in the order of the colortable */
    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth = 0;
        if (sp->bar->bins[i].count > 0)
          colorwidth =
              (gint) ((gfloat) sp->bar->cbins[i][j].count /
                      sp->bar->bins[i].count *
                      sp->bar->bins[i].rect.width);
        sp->bar->cbins[i][j].rect.x = xoffset;
        sp->bar->cbins[i][j].rect.y = sp->bar->bins[i].rect.y;
        sp->bar->cbins[i][j].rect.height = sp->bar->bins[i].rect.height;

        sp->bar->cbins[i][j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset(&sp->bar->cbins[i][j]);
        }
        xoffset += colorwidth;
      }
    }
  }

/* now eliminate rounding problems - last color in each bin gets adjusted */
  for (i = 0; i < sp->bar->nbins; i++) {
    gboolean stop = FALSE;
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
      rectangle_inset(&sp->bar->col_high_bin[j]);
    }
    xoffset += colorwidth;

    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth =
            (gint) ((gfloat) sp->bar->col_high_bin[j].count /
                    sp->bar->high_bin->count *
                    sp->bar->high_bin->rect.width);
        sp->bar->col_high_bin[j].rect.x = xoffset;
        sp->bar->col_high_bin[j].rect.y = sp->bar->high_bin->rect.y;
        sp->bar->col_high_bin[j].rect.height =
            sp->bar->high_bin->rect.height;
        sp->bar->col_high_bin[j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset(&sp->bar->col_high_bin[j]);
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
      rectangle_inset(&sp->bar->col_low_bin[j]);
    }
    xoffset += colorwidth;

    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth =
            (gint) ((gfloat) sp->bar->col_low_bin[j].count /
                    sp->bar->low_bin->count *
                    sp->bar->low_bin->rect.width);
        sp->bar->col_low_bin[j].rect.x = xoffset;
        sp->bar->col_low_bin[j].rect.y = sp->bar->low_bin->rect.y;
        sp->bar->col_low_bin[j].rect.height =
            sp->bar->low_bin->rect.height;
        sp->bar->col_low_bin[j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset(&sp->bar->col_low_bin[j]);
        }
        xoffset += colorwidth;

      }
    }
  }


}

void rectangle_inset(gbind * bin)
{
/* works around the gdk convention, that the areas of filled and framed rectangles differ
   by one pixel in each dimension */

  bin->rect.height += 1;
  bin->rect.x += 1;
  bin->rect.width += 1;
}

void barchart_init_vectors(barchartSPlotd * sp)
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

void barchart_free_structure(barchartSPlotd * sp)
{
  gint i;

/* free all previously allocated pointers */
  if (sp->bar->bins)
    g_free((gpointer) (sp->bar->bins));

  if (sp->bar->cbins) {
    gint nbins = sp->bar->nbins;

    for (i = 0; i < nbins; i++)
      if (sp->bar->cbins[i])
        g_free((gpointer) (sp->bar->cbins[i]));
    g_free((gpointer) (sp->bar->cbins));
  }

  if (sp->bar->breaks)
    g_free((gpointer) sp->bar->breaks);

  if (sp->bar->high_bin)
    g_free((gpointer) sp->bar->high_bin);

  if (sp->bar->low_bin)
    g_free((gpointer) sp->bar->low_bin);

  if (sp->bar->col_high_bin)
    g_free((gpointer) sp->bar->col_high_bin);

  if (sp->bar->col_low_bin)
    g_free((gpointer) sp->bar->col_low_bin);

  if (sp->bar->bar_hit)
    g_free((gpointer) sp->bar->bar_hit);

  if (sp->bar->old_bar_hit)
    g_free((gpointer) sp->bar->old_bar_hit);

  barchart_init_vectors(sp);
}

void barchart_allocate_structure(barchartSPlotd * sp, datad * d)
{
  vartabled *vtx;
  gint i, nbins;
  splotd *rawsp = GTK_GGOBI_SPLOT(sp);
  ggobid *gg = GGobiFromSPlot(rawsp);
  colorschemed *scheme = gg->activeColorScheme;

  vtx = vartable_element_get(rawsp->p1dvar, d);

  if (sp->bar->new_nbins < 0) {
    if (vtx->vartype == categorical) {
/* dfs */
      nbins = (vtx->nmissing) ? vtx->nlevels+1 : vtx->nlevels;
/* --- */
#ifdef BEFORE
      nbins = vtx->nlevels;
#endif
      sp->bar->is_histogram = FALSE;
    } else {
      nbins = 10;               /* replace by a more sophisticated rule */
      sp->bar->is_histogram = TRUE;
    }
  } else
    nbins = sp->bar->new_nbins;
  sp->bar->new_nbins = -1;

  if (vtx->lim_specified_p) {
    rawsp->p1d.lim.min = vtx->lim_specified.min;
    rawsp->p1d.lim.max = vtx->lim_specified.max;
  } else {
    rawsp->p1d.lim.min = vtx->lim.min;
    rawsp->p1d.lim.max = vtx->lim.max;
/* dfs */
    if (vtx->vartype == categorical) {
      rawsp->p1d.lim.min = MIN (rawsp->p1d.lim.min,
                                vtx->level_values[0]);
      rawsp->p1d.lim.max = MAX (rawsp->p1d.lim.max,
                                vtx->level_values[vtx->nlevels-1]);
    }
/* --- */
  }

  if (sp->bar->nbins && nbins == sp->bar->nbins)
    return;                     /* nothing else to be done */


/* free all previously allocated pointers */
  barchart_free_structure(sp);

  sp->bar->nbins = nbins;

/* allocate space */
  sp->bar->bins = (gbind *) g_malloc(nbins * sizeof(gbind));
  sp->bar->cbins = (gbind **) g_malloc(nbins * sizeof(gbind *));
  sp->bar->ncolors = scheme->n;
  sp->bar->bar_hit = (gboolean *) g_malloc((nbins + 2) * sizeof(gboolean));
  sp->bar->old_bar_hit =
      (gboolean *) g_malloc((nbins + 2) * sizeof(gboolean));

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->cbins[i] =
        (gbind *) g_malloc(sp->bar->ncolors * sizeof(gbind));
  }

  sp->bar->breaks = (gfloat *) g_malloc((nbins + 1) * sizeof(nbins));
}

void barchart_init_categorical(barchartSPlotd * sp, datad * d)
{
  splotd *rawsp = GTK_GGOBI_SPLOT(sp);
  gint i, jvar = rawsp->p1dvar;
  ggobid *gg = GGobiFromSPlot(rawsp);
  vartabled *vtx = vartable_element_get(rawsp->p1dvar, d);
  gfloat mindist, maxheight;
  gfloat min, max;

  gfloat *yy;
  yy = (gfloat *) g_malloc(d->nrows_in_plot * sizeof(gfloat));
  for (i = 0; i < d->nrows_in_plot; i++)
    yy[i] = d->tform.vals[d->rows_in_plot[i]][jvar];
  mindist = barchart_sort_index(yy, d->nrows_in_plot, gg, sp);
  g_free((gpointer) yy);

  min = vtx->lim_tform.min;
  max = vtx->lim_tform.max;
/* dfs */
  if (vtx->vartype == categorical) {
    min = MIN (min, vtx->level_values[0]);
    max = MAX (max, vtx->level_values[vtx->nlevels-1]);
  }
/* --- */

  maxheight = max - min;

  rawsp->scale.y = SCALE_DEFAULT * maxheight / (maxheight + mindist);
}


gboolean
barchart_redraw(splotd * rawsp, datad * d, ggobid * gg, gboolean binned)
{
  gint i, j;
  colorschemed *scheme = gg->activeColorScheme;
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(rawsp);

/* dfs */
/*
  In case we're passively responding to hide brushing, adding this
  line works but is unappealing.  It recalculates the counts, and
  then rescales using the max of the counts.  It would look quite
  nice if the rescaling didn't occur, probably.  So it should scale
  using the maximum counts if no cases were hidden, but then draw
  using the count of visible cases.
*/
  barchart_recalc_counts(sp, d, gg);
/*-- --*/

  barchart_recalc_group_counts(sp, d, gg);

  for (j = 0; j < sp->bar->ncolors; j++) {
    gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb[j]);

    for (i = 0; i < sp->bar->nbins; i++) {
      if (sp->bar->cbins[i][j].count > 0)
        gdk_draw_rectangle(rawsp->pixmap0, gg->plot_GC, TRUE,
                           sp->bar->cbins[i][j].rect.x,
                           sp->bar->cbins[i][j].rect.y,
                           sp->bar->cbins[i][j].rect.width,
                           sp->bar->cbins[i][j].rect.height);
    }
  }

/* draw overflow bins if necessary */
  if (sp->bar->high_pts_missing) {
    for (j = 0; j < sp->bar->ncolors; j++) {
      gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb[j]);

      if (sp->bar->col_high_bin[j].count > 0) {
        gdk_draw_rectangle(rawsp->pixmap0, gg->plot_GC, TRUE,
                           sp->bar->col_high_bin[j].rect.x,
                           sp->bar->col_high_bin[j].rect.y,
                           sp->bar->col_high_bin[j].rect.width,
                           sp->bar->col_high_bin[j].rect.height);
      }
    }
  }
  if (sp->bar->low_pts_missing) {
    for (j = 0; j < sp->bar->ncolors; j++) {
      gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb[j]);

      if (sp->bar->col_low_bin[j].count > 0) {
        gdk_draw_rectangle(rawsp->pixmap0, gg->plot_GC, TRUE,
                           sp->bar->col_low_bin[j].rect.x,
                           sp->bar->col_low_bin[j].rect.y,
                           sp->bar->col_low_bin[j].rect.width,
                           sp->bar->col_low_bin[j].rect.height);
      }
    }

  }


/* mark empty bins with a small circle */
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
  for (i = 0; i < sp->bar->nbins; i++) {
    if (sp->bar->bins[i].count == 0) {
      gint radius = sp->bar->bins[i].rect.height / 4;
      gdk_draw_line(rawsp->pixmap0, gg->plot_GC,
                    sp->bar->bins[i].rect.x, sp->bar->bins[i].rect.y,
                    sp->bar->bins[i].rect.x,
                    sp->bar->bins[i].rect.y +
                    sp->bar->bins[i].rect.height);
      gdk_draw_arc(rawsp->pixmap0, gg->plot_GC, FALSE,
                   sp->bar->bins[i].rect.x - radius / 2,
                   sp->bar->bins[i].rect.y +
                   sp->bar->bins[i].rect.height / 2 - radius / 2, radius,
                   radius, 0, 64 * 360);
    }
  }

  return (false);
}

void
barchart_splot_add_plot_labels(splotd * sp, GdkDrawable * drawable,
                               ggobid * gg)
{
  GtkStyle *style = gtk_widget_get_style(sp->da);
  gint lbearing, rbearing, width, ascent, descent;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  vartabled *vtx;

  vtx = vartable_element_get(sp->p1dvar, d);

#if GTK_MAJOR_VERSION == 2
  gdk_text_extents(gtk_style_get_font(style),
    vtx->collab_tform, strlen(vtx->collab_tform),
    &lbearing, &rbearing, &width, &ascent, &descent);
#else
  gdk_text_extents(style->font,
     vtx->collab_tform, strlen(vtx->collab_tform),
     &lbearing, &rbearing, &width, &ascent, &descent);
#endif

#if GTK_MAJOR_VERSION == 2
  gdk_draw_string(drawable, gtk_style_get_font(style),
    gg->plot_GC, sp->max.x - width - 5, /*-- right justify --*/
    sp->max.y - 5, vtx->collab_tform);
#else
  gdk_draw_string(drawable, style->font,
    gg->plot_GC, sp->max.x - width - 5, /*-- right justify --*/
    sp->max.y - 5, vtx->collab_tform);
#endif

  if (vtx->vartype == categorical) {
    gint i;
    gchar *catname;
    barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);

/* dfs */
    gint level;
    for (i = 0; i < bsp->bar->nbins; i++) {
      level = checkLevelValue (vtx, (gdouble) bsp->bar->bins[i].value);
      catname = g_strdup_printf ("%s",
        (level == -1) ? "missing" : vtx->level_names[level]);
/* --- */
#ifdef PREV
    for (i = 0; i < vtx->nlevels; i++) {
      catname = g_strdup (vtx->level_names[i]);
#endif

#if GTK_MAJOR_VERSION == 2
      gdk_draw_string(drawable, gtk_style_get_font(style), gg->plot_GC,
        bsp->bar->bins[i].rect.x + 2,
        bsp->bar->bins[i].rect.y +
        bsp->bar->bins[i].rect.height / 2 + 2, catname);
#else
      gdk_draw_string(drawable, style->font, gg->plot_GC,
        bsp->bar->bins[i].rect.x + 2,
        bsp->bar->bins[i].rect.y +
        bsp->bar->bins[i].rect.height / 2 + 2, catname);
#endif
      g_free (catname);
    }
  }
}

void barchart_set_breakpoints(gfloat width, barchartSPlotd * sp, datad * d)
{
  gfloat rdiff;
  gint i, nbins;
  splotd *rawsp = GTK_GGOBI_SPLOT(sp);

  rdiff = rawsp->p1d.lim.max - rawsp->p1d.lim.min;

  nbins = (gint) (rdiff / width + 1);

  sp->bar->new_nbins = nbins;
  barchart_allocate_structure(sp, d);

  for (i = 0; i <= sp->bar->nbins; i++) {
    sp->bar->breaks[i] = rawsp->p1d.lim.min + width * i;
  }

}

void barchart_set_initials(barchartSPlotd * sp, datad * d)
{
  splotd *rawsp = GTK_GGOBI_SPLOT(sp);
  vartabled *vtx = vartable_element_get(rawsp->p1dvar, d);

  if (vtx->vartype == categorical) {
/* dfs */
    if (vtx->nlevels > 1) {
      gint i, level;
      gfloat missing_val;
      gboolean add_level = false;
      if (vtx->nmissing) {
        for (i=0; i<d->nrows_in_plot; i++) {
          if (MISSING_P(d->rows_in_plot[i], rawsp->p1dvar)) {
            missing_val = d->tform.vals[i][rawsp->p1dvar];
            break;
          }
        }
        /* If the currently "imputed" value for missings is not one
           of the levels we already have, then we need an extra bin
           for the missings.
        */
        if (checkLevelValue (vtx, missing_val) == -1) {
          add_level = true;
          level = 0;
          for (i=0; i<sp->bar->nbins; i++) {
            if (add_level &&
                (gint) missing_val < vtx->level_values[level])
            {
              sp->bar->bins[i].value = (gint) missing_val;
              add_level = false;
            } else {
              sp->bar->bins[i].value = vtx->level_values[level++];
            }
          }
          if (add_level &&
              (gint) missing_val > vtx->level_values[vtx->nlevels-1])
            sp->bar->bins[sp->bar->nbins-1].value = missing_val;
        } else {
          for (i=0; i<vtx->nlevels; i++)
            sp->bar->bins[i].value = vtx->level_values[i];
          sp->bar->nbins -= 1;

          sp->bar->bins = (gbind *) g_realloc (sp->bar->bins,
            sp->bar->nbins * sizeof(gbind));
          sp->bar->bar_hit = (gboolean *) g_realloc(sp->bar->bar_hit,
            (sp->bar->nbins + 2) * sizeof(gboolean));
          sp->bar->old_bar_hit = (gboolean *) g_realloc(sp->bar->old_bar_hit,
            (sp->bar->nbins + 2) * sizeof(gboolean));

          g_free((gpointer) (sp->bar->cbins[ sp->bar->nbins ]));
          sp->bar->cbins = (gbind **) g_realloc (sp->bar->cbins,
            sp->bar->nbins * sizeof(gbind *));
        }
      } else {
        for (i=0; i<vtx->nlevels; i++)
          sp->bar->bins[i].value = vtx->level_values[i];
      }
    }
/* --- */
  } else {
    gint i;
    gfloat rdiff = rawsp->p1d.lim.max - rawsp->p1d.lim.min;

    for (i = 0; i < sp->bar->nbins; i++) {
      sp->bar->breaks[i] = rawsp->p1d.lim.min + rdiff / sp->bar->nbins * i;
    }
    sp->bar->breaks[sp->bar->nbins] = rawsp->p1d.lim.max;
  }
}

void barchart_recalc_counts(barchartSPlotd * sp, datad * d, ggobid * gg)
{
  gfloat yy;
  gint i, bin, m;
  splotd *rawsp = GTK_GGOBI_SPLOT(sp);
  vartabled *vtx = vartable_element_get(rawsp->p1dvar, d);

  if (!vtx->vartype == categorical)
    rawsp->scale.y = SCALE_DEFAULT;
  for (i = 0; i < sp->bar->nbins; i++)
    sp->bar->bins[i].count = 0;

  sp->bar->high_pts_missing = sp->bar->low_pts_missing = FALSE;

  if (vtx->vartype == categorical) {

    for (i = 0; i < d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];

      /*-- skip missings?  --*/
      if (d->nmissing > 0 && !d->missings_show_p
          && MISSING_P(m, rawsp->p1dvar))
        continue;

      /*-- skip hiddens? --*/
      if (d->hidden_now.els[m])
        continue;

      bin = sp->bar->index_to_rank[i];
      if ((bin >= 0) && (bin < sp->bar->nbins)) {
        sp->bar->bins[bin].count++;
      }
/* dfs */
      rawsp->planar[m].x = (greal) sp->bar->bins[bin].value;
/* --- */
#ifdef PREV
      rawsp->planar[m].x = bin;
#endif
    }
  } else {  /* all vartypes but categorical */
    gint index, rank = 0;

    index = sp->bar->index_to_rank[rank];
    /*yy = d->tform.vals[index][rawsp->p1dvar];*/ /* maybe not, dfs */
    yy = d->tform.vals[ d->rows_in_plot[index] ][rawsp->p1dvar];

    while ((yy < sp->bar->breaks[0] + sp->bar->offset) &&
           (rank < d->nrows_in_plot - 1)) {

      /*-- skip hiddens? --*/
      if (d->hidden_now.els[ d->rows_in_plot[index] ]) {
        rank++;
        continue;
      }

      /*rawsp->planar[index].x = -1;*/ /* maybe not, dfs */
      rawsp->planar[ d->rows_in_plot[index] ].x = -1;
      rank++;
      index = sp->bar->index_to_rank[rank];
      /*yy = d->tform.vals[index][rawsp->p1dvar];*/ /* maybe not, dfs */
      yy = d->tform.vals[ d->rows_in_plot[index] ][rawsp->p1dvar];
    }

    if (rank > 0) {
      sp->bar->low_pts_missing = TRUE;
      if (sp->bar->low_bin == NULL)
        sp->bar->low_bin = (gbind *) g_malloc(sizeof(gbind));
      if (sp->bar->col_low_bin == NULL)
        sp->bar->col_low_bin =
            (gbind *) g_malloc(sp->bar->ncolors * sizeof(gbind));
      sp->bar->low_bin->count = rank;
    }
    bin = 0;
    while (rank < d->nrows_in_plot) {
      index = sp->bar->index_to_rank[rank];
      /*yy = d->tform.vals[index][rawsp->p1dvar];*/ /* maybe not, dfs*/

      /*-- skip hiddens? --*/
      if (d->hidden_now.els[ d->rows_in_plot[index] ]) {
        rank++;
        continue;
      }

      yy = d->tform.vals[ d->rows_in_plot[index] ][rawsp->p1dvar];
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
        } else {
          if (sp->bar->high_pts_missing == FALSE) {
            sp->bar->high_pts_missing = TRUE;
            if (sp->bar->high_bin == NULL)
              sp->bar->high_bin = (gbind *) g_malloc(sizeof(gbind));
            if (sp->bar->col_high_bin == NULL) {
              sp->bar->col_high_bin = (gbind *)
                  g_malloc(sp->bar->ncolors * sizeof(gbind));
            }
            sp->bar->high_bin->count = 0;
          }
          sp->bar->high_bin->count++;
        }
      } else {
        sp->bar->bins[bin].count++;
      }
      /*rawsp->planar[index].x = bin;*/ /* maybe not, dfs */
      rawsp->planar[ d->rows_in_plot[index] ].x = bin;
      rank++;
    }
  }
  if (sp->bar->low_pts_missing == FALSE) {
    if (sp->bar->low_bin != NULL)
      g_free((gpointer) (sp->bar->low_bin));
    if (sp->bar->col_low_bin != NULL)
      g_free((gpointer) (sp->bar->col_low_bin));
    sp->bar->low_bin = NULL;
    sp->bar->col_low_bin = NULL;
  }
  if (sp->bar->high_pts_missing == FALSE) {
    if (sp->bar->high_bin != NULL)
      g_free((gpointer) (sp->bar->high_bin));
    if (sp->bar->col_high_bin != NULL)
      g_free((gpointer) (sp->bar->col_high_bin));
    sp->bar->high_bin = NULL;
    sp->bar->col_high_bin = NULL;
  }

  barchart_recalc_dimensions(GTK_GGOBI_SPLOT(sp), d, gg);
}

void barchart_recalc_dimensions(splotd * rawsp, datad * d, ggobid * gg)
{
  gint i, maxbincount = 0, maxbin = -1;
  gfloat precis = PRECISION1;
  vartabled *vtx;

  gfloat scale_y;
  gint index;
  gint minwidth;
  gfloat rdiff, ftmp;

  GdkRectangle *rect;
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(rawsp);

  scale_y = rawsp->scale.y;

  /*
   * Calculate is, a scale factor.  Scale so as to use the entire
   * plot window (well, as much of the plot window as scale.x and
   * scale.y permit.)
   */
  vtx = vartable_element_get(rawsp->p1dvar, d);

  rdiff = rawsp->p1d.lim.max - rawsp->p1d.lim.min;
  index = 0;
  for (i = 0; i < sp->bar->nbins; i++) {
    if (sp->bar->bins[i].count > maxbincount) {
      maxbincount = sp->bar->bins[i].count;
      maxbin = i;
    }

    sp->bar->bins[i].planar.x = -1;
    if (vtx->vartype == categorical) {
/* dfs */
      gfloat ftmp;
      ftmp = -1.0 + 2.0*((greal)sp->bar->bins[i].value - rawsp->p1d.lim.min)
        / rdiff;
      sp->bar->bins[i].planar.y = (greal) (PRECISION1 * ftmp);
/* --- */
#ifdef PREV
      index = sp->bar->bins[i].index;
      if (index >= 0)
        sp->bar->bins[i].planar.y =
          (glong) d->world.vals[index][rawsp->p1dvar];
#endif
    } else {
      ftmp =
          -1.0 + 2.0 * (sp->bar->breaks[i] - sp->bar->breaks[0]) / rdiff;
      sp->bar->bins[i].planar.y = (glong) (precis * ftmp);
    }
  }
  sp->bar->maxbincounts = maxbincount;

  if (!sp->bar->is_spine) {
    greal precis = (greal) PRECISION1;
    greal gtmp;

    scale_y /= 2;

    rawsp->iscale.y = (greal) (-1 * (gfloat) rawsp->max.y * scale_y);

    minwidth = rawsp->max.y;
    for (i = 0; i < sp->bar->nbins; i++) {

      rect = &sp->bar->bins[i].rect;
      gtmp = sp->bar->bins[i].planar.y - rawsp->pmid.y;
      rect->y = (gint) (gtmp * rawsp->iscale.y / precis);

      rect->x = 10;
      rect->y += (rawsp->max.y / 2);
      if (i == 0)
        minwidth = 2 * (rawsp->max.y - rect->y);
      if (i > 0) {
        minwidth =
            MIN(minwidth, sp->bar->bins[i - 1].rect.y - rect->y - 2);
        sp->bar->bins[i - 1].rect.height =
            sp->bar->bins[i - 1].rect.y - rect->y - 2;
      }

      rect->width = MAX(1, (gint) ((gfloat) (rawsp->max.x - 2 * rect->x)
                                   * sp->bar->bins[i].count /
                                   sp->bar->maxbincounts));

    }
    sp->bar->bins[sp->bar->nbins - 1].rect.height =
        sp->bar->bins[sp->bar->nbins - 2].rect.y -
        sp->bar->bins[sp->bar->nbins - 1].rect.y - 1;

/* set overflow bins to the left and right */
    if (sp->bar->low_pts_missing) {
      sp->bar->low_bin->rect.height = minwidth;
      sp->bar->low_bin->rect.x = 10;
      sp->bar->low_bin->rect.width =
          MAX(1,
              (gint) ((gfloat)
                      (rawsp->max.x - 2 * sp->bar->low_bin->rect.x)
                      * sp->bar->low_bin->count / sp->bar->maxbincounts));
      sp->bar->low_bin->rect.y = sp->bar->bins[0].rect.y + 2;
    }
    if (sp->bar->high_pts_missing) {
      sp->bar->high_bin->rect.height = minwidth;
      sp->bar->high_bin->rect.x = 10;
      sp->bar->high_bin->rect.width =
          MAX(1,
              (gint) ((gfloat)
                      (rawsp->max.x - 2 * sp->bar->high_bin->rect.x)
                      * sp->bar->high_bin->count / sp->bar->maxbincounts));
      i = sp->bar->nbins - 1;
      sp->bar->high_bin->rect.y =
          sp->bar->bins[i].rect.y - 2 * sp->bar->bins[i].rect.height - 1;
    }


    minwidth = (gint) (0.9 * minwidth);
    for (i = 0; i < sp->bar->nbins; i++) {
      if (!vtx->vartype == categorical)
        sp->bar->bins[i].rect.y -= sp->bar->bins[i].rect.height;
      else {
        sp->bar->bins[i].rect.height = minwidth;
        sp->bar->bins[i].rect.y -= minwidth / 2;
      }
    }
  } else {                      /* spine plot representation */
    GdkRectangle *rect;
    gint bindist = 2;           /* distance between two bins */
    gint maxheight;
    gint yoffset;
    gint n = d->nrows_in_plot;

    /*-- ignore hiddens here? --*/
/*
    for (i=0; i<d->nrows_in_plot; i++)
      if (d->hidden_now.els[d->rows_in_plot[i]])
        n--;
*/

    scale_y = SCALE_DEFAULT;
    maxheight = (rawsp->max.y - (sp->bar->nbins - 1) * bindist) * scale_y;
    yoffset = (gint) (rawsp->max.y * .5 * (1 + scale_y));

    for (i = 0; i < sp->bar->nbins; i++) {
      rect = &sp->bar->bins[i].rect;
      rect->x = 10;
      rect->width = rawsp->max.x - 2 * rect->x;

      rect->height =
          (gint) ((gfloat) sp->bar->bins[i].count / n * maxheight);
      rect->y = yoffset;
      yoffset -= (rect->height + bindist);
    }

    minwidth = (gint) (0.9 * minwidth);
    for (i = 0; i < sp->bar->nbins; i++) {
      sp->bar->bins[i].rect.y -= sp->bar->bins[i].rect.height;
    }

/* draw overflow bins */
    if (sp->bar->high_pts_missing) {
      sp->bar->high_bin->rect.width = rawsp->max.x - 2 * rect->x;
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
      sp->bar->low_bin->rect.width = rawsp->max.x - 2 * rect->x;
      sp->bar->low_bin->rect.height =
          (gint) ((gfloat) sp->bar->low_bin->count / n * maxheight);
      sp->bar->low_bin->rect.y =
          (gint) (rawsp->max.y * .5 * (1 + scale_y)) + 2;
    }
  }
}

gboolean barchart_active_paint_points(splotd * rawsp, datad * d)
{
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(rawsp);
  brush_coords *brush_pos = &rawsp->brush_pos;
  gint i, m, indx;
  GdkRectangle brush_rect;
  GdkRectangle dummy;
  gint x1 = MIN(brush_pos->x1, brush_pos->x2);
  gint x2 = MAX(brush_pos->x1, brush_pos->x2);
  gint y1 = MIN(brush_pos->y1, brush_pos->y2);
  gint y2 = MAX(brush_pos->y1, brush_pos->y2);
  gboolean *hits;
  vartabled *vtx = vartable_element_get(rawsp->p1dvar, d);

  hits = (gboolean *) g_malloc((sp->bar->nbins + 2) * sizeof(gboolean));

  brush_rect.x = x1;
  brush_rect.y = y1;
  brush_rect.width = x2 - x1;
  brush_rect.height = y2 - y1;


  for (i = 0; i < sp->bar->nbins; i++) {
    hits[i + 1] =
        gdk_rectangle_intersect(&sp->bar->bins[i].rect, &brush_rect,
                                &dummy);
  }
  if (sp->bar->high_pts_missing)
    hits[sp->bar->nbins + 1] =
        gdk_rectangle_intersect(&sp->bar->high_bin->rect, &brush_rect,
                                &dummy);
  else
    hits[sp->bar->nbins + 1] = FALSE;

  if (sp->bar->low_pts_missing)
    hits[0] =
        gdk_rectangle_intersect(&sp->bar->low_bin->rect, &brush_rect,
                                &dummy);
  else
    hits[0] = FALSE;

  d->npts_under_brush = 0;

  for (i = 0; i < d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];

    /*-- skip missings?  --*/
    if (d->nmissing > 0 && !d->missings_show_p
        && MISSING_P(m, rawsp->p1dvar))
      continue;

    /*-- skip hiddens? --*/
    if (d->hidden_now.els[m]) {
      continue;
    }

    /*-- dfs -- this seems to assume that the values of planar begin at 0,
         which may not be true ... this change makes it work for categorical,
         but breaks it otherwise --*/
    if (vtx->vartype == categorical) {
      indx = (gint) (rawsp->planar[m].x - rawsp->p1d.lim.min + 1);
    } else {
      indx = (gint) (rawsp->planar[m].x + 1);
    }

    d->pts_under_brush.els[m] = hits[indx];
    if (hits[indx])
      d->npts_under_brush++;
#ifdef PREV
      d->pts_under_brush.els[m] = hits[(gint)rawsp->planar[m].x + 1];
      if (hits[(gint)rawsp->planar[m].x + 1])
        d->npts_under_brush++;
#endif
  }

  g_free((gpointer) hits);

  return d->npts_under_brush;
}

static ggobid *CurrentGGobi = NULL;

gint barpsort(const void *arg1, const void *arg2)
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
barchart_sort_index(gfloat * yy, gint ny, ggobid * gg, barchartSPlotd * sp)
{
  gint i, *indx;
  gint rank;
  gfloat mindist = 0.0;

  indx = (gint *) g_malloc(ny * sizeof(gint));

/*
 * gy is needed solely for the psort routine:  psort is used by
 * qsort to put an index vector in the order that yy will assume.
*/
  gg->p1d.gy = (gfloat *) g_malloc(ny * sizeof(gfloat));
  for (i = 0; i < ny; i++) {
    indx[i] = i;
    gg->p1d.gy[i] = yy[i];
  }
  CurrentGGobi = gg;

  qsort((void *) indx, (size_t) ny, sizeof(gint), barpsort);

  CurrentGGobi = NULL;
/*
 * Bug here:  this is screwy if ny < 4.
*/
  if (sp->bar->is_histogram) {  /* vartype != categorical */
    mindist = 0;

    for (i = 0; i < ny; i++) {
      sp->bar->index_to_rank[i] = indx[i];
    }

  } else {  /* vartype = categorical */

/* dfs */
    /* XXX
       Later, when labelling, if a value doesn't match one of the
       level_values, label it 'missing'
    */
/* assumption:  there exist at least two bins */
    mindist = sp->bar->bins[1].value - sp->bar->bins[0].value;
    for (i = 1; i < sp->bar->nbins; i++)
      mindist = MIN (mindist,
                     sp->bar->bins[i].value - sp->bar->bins[i-1].value);

    rank = 0;
    /*-- there are bin values that don't exist in the data --*/
    while (yy[indx[0]] > sp->bar->bins[rank].value)
      rank++;

    for (i=0; i<sp->bar->nbins; i++)
      sp->bar->bins[i].index = -1;

    for (i=0; i<ny; i++) {

      if (i > 0) {
        if (yy[indx[i]] != yy[indx[i - 1]]) {
          rank++;

          while (yy[indx[i]] > sp->bar->bins[rank].value) {
            rank++;
          }

          sp->bar->bins[rank].index = indx[i]; /* do I care? */
        }
      }

      /* This takes me from index to bin -- dfs */
      sp->bar->index_to_rank[indx[i]] = rank;
    }

/* --- */
#ifdef PREV
    rank = 0;
    for (i=0; i<sp->bar->nbins; i++)
      sp->bar->bins[i].index = -1;

    mindist = yy[indx[ny - 1]] - yy[indx[0]];
    sp->bar->bins[rank].index = indx[0];
    for (i = 0; i < ny; i++) {
      if (i > 0) {
        if (yy[indx[i]] != yy[indx[i - 1]]) {
          rank++;
          mindist = MIN(yy[indx[i]] - yy[indx[i - 1]], mindist);
          sp->bar->bins[rank].index = indx[i];
        }
      }
      sp->bar->index_to_rank[indx[i]] = rank;
    }
#endif

  }

  g_free((gpointer) (gg->p1d.gy));
  g_free((gpointer) (indx));

  return mindist;
}

void
barchart_scaling_visual_cues_draw(splotd * rawsp, GdkDrawable * drawable,
                                  ggobid * gg)
{
  vartabled *vtx;
  displayd *display = gg->current_display;
  datad *d = display->d;
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(rawsp);
  vtx = vartable_element_get(GTK_GGOBI_SPLOT(sp)->p1dvar, d);

  if (!vtx->vartype == categorical) {
/* calculate & draw anchor_rgn */
    gint y = sp->bar->bins[0].rect.y + sp->bar->bins[0].rect.height;
    gint x = sp->bar->bins[0].rect.x;
    gint halfwidth = sp->bar->bins[0].rect.height / 2 - 2;


    sp->bar->anchor_rgn[0].x = sp->bar->anchor_rgn[1].x = x - 5;
    sp->bar->anchor_rgn[2].x = x;
    sp->bar->anchor_rgn[0].y = y + halfwidth;
    sp->bar->anchor_rgn[1].y = y - halfwidth;
    sp->bar->anchor_rgn[2].y = y;

    button_draw_with_shadows(sp->bar->anchor_rgn, drawable, gg);

/* calculate & draw offset_rgn */
    y = sp->bar->bins[0].rect.y;
    sp->bar->offset_rgn[0].x = sp->bar->offset_rgn[1].x = x - 5;
    sp->bar->offset_rgn[2].x = x;
    sp->bar->offset_rgn[0].y = y + halfwidth;
    sp->bar->offset_rgn[1].y = y - halfwidth;
    sp->bar->offset_rgn[2].y = y;

    button_draw_with_shadows(sp->bar->offset_rgn, drawable, gg);


  }

}

void
button_draw_with_shadows(GdkPoint * region, GdkDrawable * drawable,
                         ggobid * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  gdk_gc_set_foreground(gg->plot_GC, &gg->wvis.gray3);
  gdk_draw_polygon(drawable, gg->plot_GC, TRUE, region, 3);

/* dark shadows */
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_bg);

  gdk_draw_polygon(drawable, gg->plot_GC, FALSE, region, 3);
  gdk_draw_line(drawable, gg->plot_GC, region[0].x, region[2].y,
                region[2].x, region[2].y);

/* light shadows */
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);

  gdk_draw_line(drawable, gg->plot_GC, region[0].x, region[0].y,
                region[1].x, region[1].y);
  gdk_draw_line(drawable, gg->plot_GC, region[1].x, region[1].y,
                region[2].x, region[2].y);
  gdk_draw_line(drawable, gg->plot_GC, region[0].x, region[2].y + 1,
                region[2].x, region[2].y + 1);
}

void
barchart_display_menus_make(displayd * display,
                            GtkAccelGroup * accel_group,
                            GtkSignalFunc func, ggobid * gg)
{
  GtkWidget *topmenu, *options_menu;
  GtkWidget *item;

  display->edge_item = NULL;
  display->edge_menu = NULL;
  scatterplot_display_edge_menu_update(display, accel_group, func, gg);

  /*-- Options menu --*/
  topmenu = submenu_make("_Options", 'H', accel_group);
  /*-- add a tooltip --*/
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), topmenu,
                       "Options menu for this display (barchart)", NULL);

  options_menu = gtk_menu_new();

  item = CreateMenuCheck(options_menu, "Show points",
                         func, GINT_TO_POINTER(DOPT_POINTS), on, gg);
  gtk_object_set_data(GTK_OBJECT(item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem(options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuCheck(options_menu, "Show axes",
                         func, GINT_TO_POINTER(DOPT_AXES), on, gg);
  gtk_object_set_data(GTK_OBJECT(item), "display", (gpointer) display);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(topmenu), options_menu);
  submenu_append(topmenu, display->menubar);
  gtk_widget_show(topmenu);
}

gboolean pt_in_rect(icoords pt, GdkRectangle rect)
{
  return ((pt.x >= rect.x) && (pt.x <= rect.x + rect.width)
          && (pt.y >= rect.y) && (pt.y <= rect.y + rect.height));
}

gboolean
barchart_identify_bars(icoords mousepos, splotd * rawsp, datad * d,
                       ggobid * gg)
{
/* returns 0 if nothing has changed from the last time */
/*         1 if different bars are hit */
  gint i, nbins;
  gboolean stop;
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(rawsp);
  nbins = sp->bar->nbins;

  /* check, which bars are hit */
  if (sp->bar->low_pts_missing)
    sp->bar->bar_hit[0] = pt_in_rect(mousepos, sp->bar->high_bin->rect);
  else
    sp->bar->bar_hit[0] = FALSE;

  for (i = 0; i < sp->bar->nbins; i++) {
    sp->bar->bar_hit[i + 1] = pt_in_rect(mousepos, sp->bar->bins[i].rect);
  }

  if (sp->bar->high_pts_missing)
    sp->bar->bar_hit[nbins + 1] =
        pt_in_rect(mousepos, sp->bar->high_bin->rect);
  else
    sp->bar->bar_hit[nbins + 1] = FALSE;


/* are those bars the same as last time? */
  stop = FALSE;
  for (i = 0; (i < nbins + 2) && !stop; i++)
    stop = (sp->bar->bar_hit[i] != sp->bar->old_bar_hit[i]);

  sp->bar->same_hits = !stop;

  if (!stop)
    return FALSE;               /* nothing else needs to be changed */

/* set old bar hits to match the new results */
  for (i = 0; i < nbins + 2; i++)
    sp->bar->old_bar_hit[i] = sp->bar->bar_hit[i];

  return TRUE;
}

void
barchart_add_bar_cues(splotd * rawsp, GdkDrawable * drawable, ggobid * gg)
{
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(rawsp);
  GtkStyle *style = gtk_widget_get_style(rawsp->da);
  gint i, nbins;
  gchar string[100];
  icoords mousepos = rawsp->mousepos;
  colorschemed *scheme = gg->activeColorScheme;
  PipelineMode mode = viewmode_get (gg);
  gint level;
  gint j;

  if (mode != IDENT)
    return;

  nbins = sp->bar->nbins;
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);


  if (sp->bar->low_pts_missing && sp->bar->bar_hit[0]) {
    sprintf(string, "%ld point%s < %.2f", sp->bar->low_bin->count,
            sp->bar->low_bin->count == 1 ? "" : "s",
            sp->bar->breaks[0] + sp->bar->offset);

    gdk_draw_rectangle(drawable, gg->plot_GC, FALSE,
                       sp->bar->low_bin->rect.x, sp->bar->low_bin->rect.y,
                       sp->bar->low_bin->rect.width,
                       sp->bar->low_bin->rect.height);
    gdk_draw_string(drawable,
#if GTK_MAJOR_VERSION == 2
                    gtk_style_get_font(style),
#else
                    style->font,
#endif
                    gg->plot_GC, mousepos.x, mousepos.y, string);
  }
  for (i = 1; i < nbins + 1; i++) {
    if (sp->bar->bar_hit[i]) {
      if (sp->bar->is_histogram) {
        sprintf(string, "%ld point%s in (%.2f,%.2f)",
                sp->bar->bins[i - 1].count,
                sp->bar->bins[i - 1].count == 1 ? "" : "s",
                sp->bar->breaks[i - 1] + sp->bar->offset,
                sp->bar->breaks[i] + sp->bar->offset);
      } else {
        gchar *levelName;
        vartabled *var;
        var =
            (vartabled *) g_slist_nth_data(rawsp->displayptr->d->vartable,
                                           rawsp->p1dvar);
/* dfs */
        j = i-1;
        level = checkLevelValue (var, (gdouble) sp->bar->bins[j].value);

        if (level == -1) {
          sprintf(string, "%ld point%s missing",
                sp->bar->bins[j].count,
                sp->bar->bins[j].count == 1 ? "" : "s");
        } else {
          levelName = var->level_names[level];
          sprintf(string, "%ld point%s for level %s",
                sp->bar->bins[j].count,
                sp->bar->bins[j].count == 1 ? "" : "s", levelName);
        }
/* --- */
#ifdef PREV
        levelName = var->level_names[i - 1];
        sprintf(string, "%ld point%s for level %s",
                sp->bar->bins[i - 1].count,
                sp->bar->bins[i - 1].count == 1 ? "" : "s", levelName);
#endif
      }
      gdk_draw_rectangle(drawable, gg->plot_GC, FALSE,
                         sp->bar->bins[i - 1].rect.x,
                         sp->bar->bins[i - 1].rect.y,
                         sp->bar->bins[i - 1].rect.width,
                         sp->bar->bins[i - 1].rect.height);
      gdk_draw_string(drawable,
#if GTK_MAJOR_VERSION == 2
                      gtk_style_get_font(style),
#else
                      style->font,
#endif
                      gg->plot_GC, mousepos.x, mousepos.y, string);
    }
  }

  if (sp->bar->high_pts_missing && sp->bar->bar_hit[nbins + 1]) {
    sprintf(string, "%ld point%s > %.2f", sp->bar->high_bin->count,
            sp->bar->high_bin->count == 1 ? "" : "s",
            sp->bar->breaks[nbins] + sp->bar->offset);

    gdk_draw_rectangle(drawable, gg->plot_GC, FALSE,
                       sp->bar->high_bin->rect.x,
                       sp->bar->high_bin->rect.y,
                       sp->bar->high_bin->rect.width,
                       sp->bar->high_bin->rect.height);
    gdk_draw_string(drawable,
#if GTK_MAJOR_VERSION == 2
                    gtk_style_get_font(style),
#else
                    style->font,
#endif
                    gg->plot_GC, mousepos.x, mousepos.y, string);

  }

}

splotd *gtk_barchart_splot_new(displayd * dpy, gint width, gint height,
                               ggobid * gg)
{
  barchartSPlotd *bsp;
  splotd *sp;

  bsp = gtk_type_new(GTK_TYPE_GGOBI_BARCHART_SPLOT);
  sp = GTK_GGOBI_SPLOT(bsp);

  splot_init(sp, dpy, width, height, gg);
  barchart_clean_init(bsp);
  barchart_recalc_counts(bsp, dpy->d, gg);

  return (sp);
}

/**
 Called when we create the barchart.
*/
void barchart_cpanel_init(cpaneld * cpanel, ggobid * gg)
{
  cpanel->viewmode = EXTENDED_DISPLAY_MODE;
  cpanel->projection = P1PLOT;  /*-- does it need a projection? --*/
  cpanel->barchart_display_mode = 0;    /*dfs-barchart */

  /*-- 1d plots --*/
  cpanel_p1d_init(cpanel, gg);

  /*-- available modes --*/
  cpanel_brush_init(cpanel, gg);
  cpanel_identify_init(cpanel, gg);
}
