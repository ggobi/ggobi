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

#ifdef BARCHART_IMPLEMENTED

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "gtkext.h"
#include "externs.h"

#define WIDTH   370
#define HEIGHT  370

gfloat barchart_sort_index (gfloat *yy, gint ny, ggobid *gg, splotd *sp);
void barchart_clean_init (splotd *sp);
void barchart_init_categorical (splotd *sp, datad *d);
void barchart_set_initials (splotd *sp, datad *d);
void rectangle_inset (gbind *bin);
void barchart_recalc_counts (splotd *sp, datad *d, ggobid *gg);
void barchart_allocate_structure (splotd *sp, datad *d);
void button_draw_with_shadows (GdkPoint *region, GdkDrawable *drawable, ggobid *gg);

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static const GtkItemFactoryEntry menu_items[] = {
  {"/_File",
    NULL,
    NULL,
    0,
    "<Branch>" },
  {"/File/Print",
    "",
    (GtkItemFactoryCallback)display_print_cb,
    0,
    "<Item>" },
  {"/File/sep",
    NULL,
    NULL,
    0,
    "<Separator>" },
  {"/File/Close",
    "",
    (GtkItemFactoryCallback) display_close_cb,
    0,
    "<Item>" },
};

displayd *
barchart_new (gboolean missing_p, splotd *sp, datad *d, ggobid *gg) {
  GtkWidget *table, *vbox;
  displayd *display;
  extern void barchart_display_menus_make (displayd *display,
    GtkAccelGroup *, GtkSignalFunc, ggobid *);

  if (d == NULL || d->ncols < 1)
    return (NULL);

  if (sp == NULL) {
    display = display_alloc_init (barchart, missing_p, d, gg);
  } else {
    display = (displayd*) sp->displayptr;
    display->d = d;
  }

  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
    display->options.axes_center_p = true;
   */

  barchart_cpanel_init (&display->cpanel, gg);

  display_window_init (GTK_GGOBI_WINDOW_DISPLAY(display), 3, gg);  /*-- 3 = width = any small int --*/

  /*-- Add the main menu bar --*/
  vbox = GTK_WIDGET(display); /*XXX gtk_vbox_new (false, 1); */
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (GTK_GGOBI_WINDOW_DISPLAY(display)->window), vbox);

  gg->app.sp_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 gg->app.sp_accel_group, GTK_GGOBI_WINDOW_DISPLAY(display)->window, &display->menubar,
                 (gpointer) display);
  /*
   * After creating the menubar, and populating the file menu,
   * add the other menus manually
  */
  barchart_display_menus_make (display, gg->app.sp_accel_group,
   (GtkSignalFunc) display_options_cb, gg);

  gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);


  /*-- Initialize a single splot --*/
  if (sp == NULL) { 
    sp = splot_new (display, WIDTH, HEIGHT, gg);
  } 
  sp->bar = (barchartd *) g_malloc (1 * sizeof (barchartd)); 
  sp->bar->index_to_rank = NULL;
  sp->bar->is_spine = FALSE;


  barchart_init_vectors(sp);
  barchart_clean_init (sp);
  barchart_recalc_counts (sp,d,gg);



  display->splots = NULL;
  display->splots = g_list_append (display->splots, (gpointer) sp);

  table = gtk_table_new (3, 2, false);  /* rows, columns, homogeneous */
  gtk_box_pack_start (GTK_BOX (vbox), table, true, true, 0);
  gtk_table_attach (GTK_TABLE (table),
                    sp->da, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    0, 0 );



  /*
   * The horizontal ruler goes on top. As the mouse moves across the
   * drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->hrule = gtk_ext_hruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da), "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD (display->hrule, motion_notify_event),
    GTK_OBJECT (display->hrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->hrule, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
                    (GtkAttachOptions) GTK_FILL,
                    0, 0);


  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->vrule = gtk_ext_vruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da),
                             "motion_notify_event",
                             (GtkSignalFunc) EVENT_METHOD (display->vrule,
                                                           motion_notify_event),
                             GTK_OBJECT (display->vrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_FILL,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
                    0, 0 );


  gtk_widget_show_all (GTK_GGOBI_WINDOW_DISPLAY(display)->window);

  /*-- hide any extraneous rulers --*/

  display->p1d_orientation = VERTICAL; 
  scatterplot_show_rulers (display, P1PLOT); 
  ruler_ranges_set (true, display, sp, gg);

  return display;
}

void barchart_clean_init (splotd *sp) {
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gint i, j;

  sp->bar->nbins = -1;

  sp->bar->new_nbins = -1;
  barchart_allocate_structure (sp,d);

  for (i=0; i<sp->bar->nbins; i++) {
    sp->bar->bins[i].count = 0;
    for (j=0; j<sp->bar->ncolors; j++) {
      sp->bar->cbins[i][j].count = 0;
      sp->bar->cbins[i][j].rect.width = 1;
    }
  }
  for (i=0; i<sp->bar->nbins+2; i++)
    sp->bar->bar_hit[i] = false;
/* */

  barchart_set_initials (sp,d);
  sp->bar->offset = 0;
  sp->pmid.y = 0;

  if (sp->bar->index_to_rank) {
    g_free ((gpointer) sp->bar->index_to_rank);
  }
  sp->bar->index_to_rank = (gint *) g_malloc (d->nrows_in_plot * sizeof (gint));
  barchart_init_categorical (sp,d);
 
}

void barchart_recalc_group_counts (splotd *sp, datad *d, ggobid *gg) {
  gint i, j,m,bin;

  for (i=0; i<sp->bar->nbins; i++) 
    for (j=0; j<sp->bar->ncolors; j++)
      sp->bar->cbins[i][j].count = 0;

/*  initialize overflow bins */
  if (sp->bar->high_pts_missing) {
    for (j=0; j<sp->bar->ncolors; j++)
      sp->bar->col_high_bin[j].count = 0;
  }
  if (sp->bar->low_pts_missing) {
    for (j=0; j<sp->bar->ncolors; j++)
      sp->bar->col_low_bin[j].count = 0;
  }

/* count points in bins */
  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];

    /*-- skip missings?  --*/
    if (d->nmissing > 0 && !d->missings_show_p && MISSING_P(m,sp->p1dvar))
      continue;

    bin = sp->planar[m].x;
    if (( bin >= 0) && (bin < sp->bar->nbins)) {
      sp->bar->cbins[bin][d->color_now.els[m]].count++;
    }
    if  (bin == -1) {
      sp->bar->col_low_bin[d->color_now.els[m]].count++;
    }
    if (bin == sp->bar->nbins) {
      sp->bar->col_high_bin[d->color_now.els[m]].count++;
    }
  }

  barchart_recalc_group_dimensions (sp,gg);
}



void barchart_recalc_group_dimensions (splotd *sp, ggobid *gg) {
  gint colorwidth, i, j, xoffset;

  for (i=0; i< sp->bar->nbins; i++) {
    xoffset = sp->bar->bins[i].rect.x;

/* first color in all bins is the current color */ 
    j = gg->color_id;
    colorwidth = 1;
    if (sp->bar->bins[i].count > 0)
      colorwidth = 
        (gint) ((gfloat) sp->bar->cbins[i][j].count/sp->bar->bins[i].count*sp->bar->bins[i].rect.width);
    sp->bar->cbins[i][j].rect.x = xoffset;
    sp->bar->cbins[i][j].rect.y = sp->bar->bins[i].rect.y;
    sp->bar->cbins[i][j].rect.height = sp->bar->bins[i].rect.height;

    sp->bar->cbins[i][j].rect.width = colorwidth;
    if (colorwidth) {
      colorwidth++;
      rectangle_inset (&sp->bar->cbins[i][j]);
    } 
    xoffset +=  colorwidth;

/* then all other colors follow in the order of the colortable */
    for (j=0; j < sp->bar->ncolors; j++) {
      if ( j!= gg->color_id) {
        colorwidth = 0;
        if (sp->bar->bins[i].count > 0)
          colorwidth =
         (gint) ((gfloat) sp->bar->cbins[i][j].count/sp->bar->bins[i].count*sp->bar->bins[i].rect.width);
        sp->bar->cbins[i][j].rect.x = xoffset;
        sp->bar->cbins[i][j].rect.y = sp->bar->bins[i].rect.y;
        sp->bar->cbins[i][j].rect.height = sp->bar->bins[i].rect.height;

        sp->bar->cbins[i][j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset (&sp->bar->cbins[i][j]);
        }
        xoffset +=  colorwidth;
      }
    }
  }

/* now eliminate rounding problems - last color in each bin gets adjusted */
  for (i=0; i< sp->bar->nbins; i++) {
    gboolean stop = FALSE;
    for (j=sp->bar->ncolors-1; (j >= 0) && (!stop); j--) 
      if (j != gg->color_id) 
        if (sp->bar->cbins[i][j].count > 0) 
          stop = TRUE;  /* find last color used */

    if (stop) {
      j++;
      sp->bar->cbins[i][j].rect.width = 
          sp->bar->bins[i].rect.x + sp->bar->bins[i].rect.width - sp->bar->cbins[i][j].rect.x+2;
    }
  }

/* deal with overflow bins to the left and right now:  */
  if (sp->bar->high_pts_missing) {
    j = gg->color_id;
    xoffset = sp->bar->high_bin->rect.x;
    colorwidth =
        (gint) ((gfloat) sp->bar->col_high_bin[j].count/sp->bar->high_bin->count*sp->bar->high_bin->rect.width);
    sp->bar->col_high_bin[j].rect.x = xoffset;
    sp->bar->col_high_bin[j].rect.y = sp->bar->high_bin->rect.y;
    sp->bar->col_high_bin[j].rect.height = sp->bar->high_bin->rect.height;
    sp->bar->col_high_bin[j].rect.width = colorwidth;
    if (colorwidth) {
      colorwidth++;
      rectangle_inset (&sp->bar->col_high_bin[j]);
    }
    xoffset +=  colorwidth;

    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
        colorwidth =
          (gint) ((gfloat) sp->bar->col_high_bin[j].count/sp->bar->high_bin->count*sp->bar->high_bin->rect.width);
        sp->bar->col_high_bin[j].rect.x = xoffset;
        sp->bar->col_high_bin[j].rect.y = sp->bar->high_bin->rect.y;
        sp->bar->col_high_bin[j].rect.height = sp->bar->high_bin->rect.height;
        sp->bar->col_high_bin[j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset (&sp->bar->col_high_bin[j]);
        }
        xoffset +=  colorwidth;

      }
    }
  }
  if (sp->bar->low_pts_missing) {
    j = gg->color_id;
    xoffset = sp->bar->low_bin->rect.x;
    colorwidth =
        (gint) ((gfloat) sp->bar->col_low_bin[j].count/sp->bar->low_bin->count*sp->bar->low_bin->rect.width);
    sp->bar->col_low_bin[j].rect.x = xoffset;
    sp->bar->col_low_bin[j].rect.y = sp->bar->low_bin->rect.y;
    sp->bar->col_low_bin[j].rect.height = sp->bar->low_bin->rect.height;
    sp->bar->col_low_bin[j].rect.width = colorwidth;
    if (colorwidth) {
      colorwidth++;
      rectangle_inset (&sp->bar->col_low_bin[j]);
    }
    xoffset +=  colorwidth;

    for (j = 0; j < sp->bar->ncolors; j++) {
      if (j != gg->color_id) {
            colorwidth =
        (gint) ((gfloat) sp->bar->col_low_bin[j].count/sp->bar->low_bin->count*sp->bar->low_bin->rect.width);
        sp->bar->col_low_bin[j].rect.x = xoffset;
        sp->bar->col_low_bin[j].rect.y = sp->bar->low_bin->rect.y;
        sp->bar->col_low_bin[j].rect.height = sp->bar->low_bin->rect.height;
        sp->bar->col_low_bin[j].rect.width = colorwidth;
        if (colorwidth) {
          colorwidth++;
          rectangle_inset (&sp->bar->col_low_bin[j]);
        }
        xoffset +=  colorwidth;

      }
    }
  }


}

void rectangle_inset (gbind *bin) {
/* works around the gdk convention, that the areas of filled and framed rectangles differ
   by one pixel in each dimension */

  bin->rect.height += 1;
  bin->rect.x += 1; 
  bin->rect.width += 1;
}

void barchart_init_vectors (splotd *sp) {
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

void barchart_free_structure (splotd *sp) {
  gint i;

/* free all previously allocated pointers */
  if (sp->bar->bins)
    g_free ((gpointer) (sp->bar->bins));

  if (sp->bar->cbins) {
    gint nbins = sp->bar->nbins;

    for (i=0; i < nbins; i++)
      if (sp->bar->cbins[i]) g_free ((gpointer) (sp->bar->cbins[i]));
    g_free ((gpointer) (sp->bar->cbins));
  }

  if (sp->bar->breaks)
    g_free ((gpointer) sp->bar->breaks);

  if (sp->bar->high_bin)
    g_free ((gpointer) sp->bar->high_bin);

  if (sp->bar->low_bin)
    g_free ((gpointer) sp->bar->low_bin);

  if ( sp->bar->col_high_bin)
    g_free ((gpointer) sp->bar->col_high_bin);

  if (sp->bar->col_low_bin)
    g_free ((gpointer) sp->bar->col_low_bin);

  if (sp->bar->bar_hit)
    g_free ((gpointer) sp->bar->bar_hit);

  if (sp->bar->old_bar_hit)
    g_free ((gpointer) sp->bar->old_bar_hit);

  barchart_init_vectors (sp); 
}

void barchart_allocate_structure (splotd *sp, datad *d) {
  vartabled *vtx;
  gint i, nbins;
  ggobid *gg = GGobiFromSPlot(sp);
  colorschemed *scheme = gg->activeColorScheme;

  vtx = vartable_element_get (sp->p1dvar, d);

  if (sp->bar->new_nbins < 0) {
    if (vtx->categorical_p) {
      nbins = vtx->nlevels;
      sp->bar->is_histogram = FALSE;
    } else {
      nbins = 10;    /* replace by a more sophisticated rule */
      sp->bar->is_histogram = TRUE;
    }
  } else nbins = sp->bar->new_nbins;
  sp->bar->new_nbins =-1;

  sp->p1d.lim.min = vtx->lim_raw.min;
  sp->p1d.lim.max = vtx->lim_raw.max;

  if (sp->bar->nbins && nbins == sp->bar->nbins) return; /* nothing else to be done */ 


/* free all previously allocated pointers */
  barchart_free_structure (sp);

  sp->bar->nbins = nbins;

/* allocate space */
  sp->bar->bins = (gbind *) g_malloc (nbins * sizeof (gbind));
  sp->bar->cbins = (gbind **) g_malloc (nbins * sizeof (gbind *));
  sp->bar->ncolors = scheme->n;
  sp->bar->bar_hit = (gboolean *) g_malloc ((nbins+2) * sizeof(gboolean));
  sp->bar->old_bar_hit = (gboolean *) g_malloc ((nbins+2) * sizeof(gboolean));


  for (i=0; i < sp->bar->nbins; i++) {
    sp->bar->cbins[i] = (gbind *) g_malloc (sp->bar->ncolors * sizeof (gbind));
  }


  if (!vtx->categorical_p)
    sp->bar->breaks = (gfloat *) g_malloc ((nbins+1)* sizeof(nbins)); 
}

void barchart_init_categorical (splotd *sp, datad *d) {
    gfloat *yy;
    gint i,jvar = sp->p1dvar;
    ggobid *gg = GGobiFromSPlot(sp);
    vartabled *vtx = vartable_element_get (sp->p1dvar, d);
    gfloat mindist, maxheight;

    yy = (gfloat *) g_malloc (d->nrows_in_plot * sizeof (gfloat));
    for (i=0; i<d->nrows_in_plot; i++)
      yy[i] = d->tform.vals[d->rows_in_plot[i]][jvar];

    mindist = barchart_sort_index (yy, d->nrows_in_plot, gg, sp);

    g_free ((gpointer) yy);
    maxheight = vtx->lim_raw.max - vtx->lim_raw.min;
    sp->scale.y = SCALE_DEFAULT * maxheight/(maxheight + mindist);
}


void barchart_redraw (splotd *sp, datad *d, ggobid *gg) {
  gint i, j;
  colorschemed *scheme = gg->activeColorScheme;

  barchart_recalc_group_counts (sp, d, gg);

  for (j=0; j<sp->bar->ncolors; j++) {
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[j]);

    for (i=0; i<sp->bar->nbins; i++) {
      if (sp->bar->cbins[i][j].count>0)
        gdk_draw_rectangle  (sp->pixmap0, gg->plot_GC, TRUE,
          sp->bar->cbins[i][j].rect.x,sp->bar->cbins[i][j].rect.y,
          sp->bar->cbins[i][j].rect.width,sp->bar->cbins[i][j].rect.height);
    }
  }

/* draw overflow bins if necessary */
  if (sp->bar->high_pts_missing) {
    for (j=0; j<sp->bar->ncolors; j++) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[j]);

      if (sp->bar->col_high_bin[j].count>0) {
        gdk_draw_rectangle  (sp->pixmap0, gg->plot_GC, TRUE,
        sp->bar->col_high_bin[j].rect.x,sp->bar->col_high_bin[j].rect.y,
        sp->bar->col_high_bin[j].rect.width,sp->bar->col_high_bin[j].rect.height);
      }
    }
  }
  if (sp->bar->low_pts_missing) {
    for (j=0; j<sp->bar->ncolors; j++) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[j]);

      if (sp->bar->col_low_bin[j].count>0) {
        gdk_draw_rectangle  (sp->pixmap0, gg->plot_GC, TRUE,
        sp->bar->col_low_bin[j].rect.x,sp->bar->col_low_bin[j].rect.y,
        sp->bar->col_low_bin[j].rect.width,sp->bar->col_low_bin[j].rect.height);
      }
    }
 
  }


/* mark empty bins with a small circle */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  for (i=0; i<sp->bar->nbins; i++) {
    if (sp->bar->bins[i].count==0) {
      gint radius = sp->bar->bins[i].rect.height/4;
      gdk_draw_line (sp->pixmap0, gg->plot_GC,
                     sp->bar->bins[i].rect.x,sp->bar->bins[i].rect.y,
                     sp->bar->bins[i].rect.x,sp->bar->bins[i].rect.y+sp->bar->bins[i].rect.height);
      gdk_draw_arc (sp->pixmap0, gg->plot_GC, FALSE,
                    sp->bar->bins[i].rect.x-radius/2, 
                    sp->bar->bins[i].rect.y+sp->bar->bins[i].rect.height/2-radius/2,
                    radius,radius,0,64*360);
    }
  }


}

void barchart_splot_add_plot_labels (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gint lbearing, rbearing, width, ascent, descent;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  vartabled *vtx;

  vtx = vartable_element_get (sp->p1dvar, d);
  gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    vtx->collab_tform, strlen (vtx->collab_tform),
    &lbearing, &rbearing, &width, &ascent, &descent);
  gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    gg->plot_GC,
    sp->max.x - width - 5,  /*-- right justify --*/
    sp->max.y - 5,
    vtx->collab_tform);
 

  if (vtx->categorical_p) {
      gint i;
      gchar catname[100];
      for (i=0; i < vtx->nlevels; i++) {
        strcpy (catname, vtx->level_names[i]);

        gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
          gtk_style_get_font (style),
#else
          style->font,
#endif
          gg->plot_GC,
          sp->bar->bins[i].rect.x+2,
          sp->bar->bins[i].rect.y + sp->bar->bins[i].rect.height/2 + 2,
          catname);
    } 
  }
}

void barchart_set_breakpoints (gfloat width, splotd *sp, datad *d ) {
  gfloat rdiff; 
  gint i, nbins;

  rdiff = sp->p1d.lim.max - sp->p1d.lim.min;
  nbins = (gint) (rdiff/width + 1);


  sp->bar->new_nbins = nbins;
  barchart_allocate_structure (sp, d);

  for (i=0; i <= sp->bar->nbins; i++) {
    sp->bar->breaks[i] = sp->p1d.lim.min + width*i;
  }

}

void barchart_set_initials (splotd *sp, datad *d) {
  vartabled *vtx = vartable_element_get (sp->p1dvar, d);

  
  if (vtx->categorical_p) {
  } else {
    gint i;
    gfloat rdiff = sp->p1d.lim.max - sp->p1d.lim.min;

    for (i=0; i < sp->bar->nbins; i++) {
      sp->bar->breaks[i] = sp->p1d.lim.min + rdiff/sp->bar->nbins*i; 
    }
    sp->bar->breaks[sp->bar->nbins] = sp->p1d.lim.max;

  }
}

void barchart_recalc_counts (splotd *sp, datad *d, ggobid *gg) {
  gfloat yy;
  gint i, bin, m;
  vartabled *vtx = vartable_element_get (sp->p1dvar, d);
  
  if (!vtx->categorical_p) sp->scale.y = SCALE_DEFAULT;
  for (i=0; i<sp->bar->nbins; i++) sp->bar->bins[i].count = 0;

  sp->bar->high_pts_missing = sp->bar->low_pts_missing = FALSE;

  if (vtx->categorical_p) {
    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];

      /*-- skip missings?  --*/
      if (d->nmissing > 0 && !d->missings_show_p && MISSING_P(m,sp->p1dvar))
        continue;

      bin = sp->bar->index_to_rank[i];
      if (( bin >= 0) && (bin < sp->bar->nbins)) {
        sp->bar->bins[bin].count++;
      }
      sp->planar[m].x = bin;
    }
  } else {
    gint index, rank = 0; 

    index = sp->bar->index_to_rank[rank];
    yy = d->tform.vals[index][sp->p1dvar];

    while ((yy < sp->bar->breaks[0]+sp->bar->offset) &&
           (rank < d->nrows_in_plot-1))
    {
      sp->planar[index].x = -1; 
      rank++;
      index = sp->bar->index_to_rank[rank];
      yy = d->tform.vals[index][sp->p1dvar];
    }
    if (rank > 0) {
      sp->bar->low_pts_missing = TRUE;
      if (sp->bar->low_bin == NULL)
        sp->bar->low_bin = (gbind *) g_malloc (sizeof(gbind));
      if (sp->bar->col_low_bin == NULL)
        sp->bar->col_low_bin = (gbind *) g_malloc (sp->bar->ncolors*sizeof(gbind));
      sp->bar->low_bin->count = rank;
    }
    bin = 0;
    while (rank < d->nrows_in_plot) {
      index = sp->bar->index_to_rank[rank];
      yy = d->tform.vals[index][sp->p1dvar];
      while ((bin < sp->bar->nbins) &&
             (sp->bar->breaks[bin+1]+sp->bar->offset < yy)) 
      {
        bin++; 
      }

      if (bin > sp->bar->nbins-1) {
/* check whether the value is the maximum, if so, add it to the last bin - slight inconsistency with histograms */
        if (yy == sp->bar->breaks[sp->bar->nbins] + sp->bar->offset) {
          bin--;
          sp->bar->bins[bin].count++;
        } else  {
          if (sp->bar->high_pts_missing == FALSE) {
            sp->bar->high_pts_missing = TRUE; 
            if (sp->bar->high_bin == NULL)
              sp->bar->high_bin = (gbind *) g_malloc (sizeof(gbind));
            if (sp->bar->col_high_bin == NULL)
              sp->bar->col_high_bin = (gbind *)
                g_malloc (sp->bar->ncolors *sizeof(gbind));
            sp->bar->high_bin->count = 0; 
          }
          sp->bar->high_bin->count++;
        }
      } else { 
        sp->bar->bins[bin].count++; 
      }
      sp->planar[index].x = bin;
      rank++;
    } 
  }
  if (sp->bar->low_pts_missing == FALSE) {
    if (sp->bar->low_bin != NULL) g_free ((gpointer) (sp->bar->low_bin));
    if (sp->bar->col_low_bin != NULL)
      g_free ((gpointer) (sp->bar->col_low_bin));
    sp->bar->low_bin = NULL;
    sp->bar->col_low_bin = NULL;
  }
  if (sp->bar->high_pts_missing == FALSE) {
    if (sp->bar->high_bin != NULL) g_free ((gpointer) (sp->bar->high_bin));
    if (sp->bar->col_high_bin != NULL)
      g_free ((gpointer) (sp->bar->col_high_bin));
    sp->bar->high_bin = NULL;
    sp->bar->col_high_bin = NULL;
  }


  barchart_recalc_dimensions (sp, d, gg);

}

void barchart_recalc_dimensions (splotd *sp, datad *d, ggobid *gg) {
  gint i, maxbincount = 0, maxbin = -1;
  gfloat precis = PRECISION1;
  glong ltmp;
  vartabled *vtx;

  gfloat scale_y = sp->scale.y;
  gint index;
  gint minwidth; 
  gfloat rdiff, ftmp;

  GdkRectangle *rect;

  /*
   * Calculate is, a scale factor.  Scale so as to use the entire
   * plot window (well, as much of the plot window as scale.x and
   * scale.y permit.)
  */
  vtx = vartable_element_get (sp->p1dvar, d);

  rdiff = sp->p1d.lim.max - sp->p1d.lim.min;
  index = 0;
  for (i=0; i < sp->bar->nbins; i++) {
    if (sp->bar->bins[i].count> maxbincount) {
      maxbincount = sp->bar->bins[i].count;
      maxbin = i;
    }

    sp->bar->bins[i].planar.x = -1;
    if (vtx->categorical_p) {
      index = sp->bar->bins[i].index;
      sp->bar->bins[i].planar.y = (glong) d->world.vals[index][sp->p1dvar];
    } else { 
      ftmp = -1.0 + 2.0*(sp->bar->breaks[i]- sp->bar->breaks[0])/rdiff;
      sp->bar->bins[i].planar.y = (glong) (precis * ftmp);
    }
  }
  sp->bar->maxbincounts = maxbincount;

if (!sp->bar->is_spine) {
  scale_y /= 2;

  sp->iscale.y = (glong) (-1 * (gfloat) sp->max.y * scale_y);

  minwidth = sp->max.y; 
  for (i=0; i<sp->bar->nbins; i++) {
    rect = &sp->bar->bins[i].rect;
    ltmp = sp->bar->bins[i].planar.y - sp->pmid.y;
    rect->y = (gint) ((ltmp * sp->iscale.y) >> EXP1);

    rect->x = 10;
    rect->y += (sp->max.y / 2);
    if (i == 0) minwidth = 2*(sp->max.y-rect->y);
    if (i > 0) {
      minwidth = MIN(minwidth,sp->bar->bins[i-1].rect.y - rect->y-2);
      sp->bar->bins[i-1].rect.height = sp->bar->bins[i-1].rect.y - rect->y-2;
    }

    rect->width = MAX(1, (gint) ((gfloat) (sp->max.x -2*rect->x)
                                  * sp->bar->bins[i].count/ sp->bar->maxbincounts));

  }
  sp->bar->bins[sp->bar->nbins-1].rect.height = sp->bar->bins[sp->bar->nbins-2].rect.y -
                                  sp->bar->bins[sp->bar->nbins-1].rect.y - 1;

/* set overflow bins to the left and right */
  if (sp->bar->low_pts_missing) {
    sp->bar->low_bin->rect.height = minwidth;
    sp->bar->low_bin->rect.x = 10;
    sp->bar->low_bin->rect.width = MAX(1, (gint) ((gfloat) (sp->max.x -2*sp->bar->low_bin->rect.x)
                                  * sp->bar->low_bin->count/ sp->bar->maxbincounts));
    sp->bar->low_bin->rect.y = sp->bar->bins[0].rect.y + 2;
  }
  if (sp->bar->high_pts_missing) {
    sp->bar->high_bin->rect.height = minwidth;
    sp->bar->high_bin->rect.x = 10;
    sp->bar->high_bin->rect.width = MAX(1, (gint) ((gfloat) (sp->max.x -2*sp->bar->high_bin->rect.x)
                                  * sp->bar->high_bin->count/ sp->bar->maxbincounts));
    i = sp->bar->nbins-1;
    sp->bar->high_bin->rect.y = sp->bar->bins[i].rect.y - 2*sp->bar->bins[i].rect.height - 1; 
  }

 
  minwidth = (gint) (0.9*minwidth);
  for (i=0; i<sp->bar->nbins; i++) {
    if (!vtx->categorical_p) 
      sp->bar->bins[i].rect.y -= sp->bar->bins[i].rect.height;
    else {
      sp->bar->bins[i].rect.height = minwidth;
      sp->bar->bins[i].rect.y -= minwidth/2;
    }
  }
} else {   /* spine plot representation */
  GdkRectangle *rect;
  gint bindist = 2;  /* distance between two bins */
  gint maxheight; 
  gint yoffset; 
  gint n = d->nrows_in_plot; 

  scale_y = SCALE_DEFAULT;
  maxheight = (sp->max.y-(sp->bar->nbins-1)*bindist)*scale_y;
  yoffset = (gint)(sp->max.y*.5*(1+scale_y));

  for (i=0; i<sp->bar->nbins; i++) {
    rect = &sp->bar->bins[i].rect;
    rect->x = 10;
    rect->width = sp->max.x -2*rect->x;
    
    rect->height = (gint)((gfloat) sp->bar->bins[i].count/n *maxheight);
    rect->y = yoffset;
    yoffset -=  (rect->height + bindist);
  }

  minwidth = (gint) (0.9*minwidth);
  for (i=0; i<sp->bar->nbins; i++) {
    sp->bar->bins[i].rect.y -= sp->bar->bins[i].rect.height;
  }

/* draw overflow bins */
  if (sp->bar->high_pts_missing) {
    sp->bar->high_bin->rect.width = sp->max.x -2*rect->x;
    sp->bar->high_bin->rect.x = 10;
    sp->bar->high_bin->rect.height = (gint)((gfloat) sp->bar->high_bin->count/n *maxheight);
    i = sp->bar->nbins-1;
    sp->bar->high_bin->rect.y = (gint)(sp->max.y*.5*(1-scale_y))-sp->bar->high_bin->rect.height-2; 
  }
  if (sp->bar->low_pts_missing) {
    sp->bar->low_bin->rect.x = 10;
    sp->bar->low_bin->rect.width = sp->max.x -2*rect->x;
    sp->bar->low_bin->rect.height = (gint)((gfloat) sp->bar->low_bin->count/n *maxheight); 
    sp->bar->low_bin->rect.y = (gint)(sp->max.y*.5*(1+scale_y))+2;
  }
}
}

gboolean barchart_active_paint_points (splotd *sp, datad *d) {

  brush_coords *brush_pos = &sp->brush_pos;
  gint i;
  GdkRectangle brush_rect;
  GdkRectangle dummy;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);
  gboolean *hits;

  hits = (gboolean *) g_malloc ((sp->bar->nbins + 2) * sizeof (gboolean));

  brush_rect.x = x1;
  brush_rect.y = y1;
  brush_rect.width = x2-x1;
  brush_rect.height = y2-y1;


  for (i=0; i < sp->bar->nbins; i++) {
    hits [i+1] = gdk_rectangle_intersect(&sp->bar->bins[i].rect, &brush_rect, &dummy);
  }
  if (sp->bar->high_pts_missing) 
    hits [sp->bar->nbins+1] = gdk_rectangle_intersect(&sp->bar->high_bin->rect, &brush_rect, &dummy);
  else 
    hits [sp->bar->nbins+1] = FALSE;

  if (sp->bar->low_pts_missing) 
    hits [0] = gdk_rectangle_intersect(&sp->bar->low_bin->rect, &brush_rect, &dummy);
  else 
    hits [0] = FALSE;



  d->npts_under_brush = 0;

{
    gint m;
    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];

      /*-- skip missings?  --*/
      if (d->nmissing > 0 && !d->missings_show_p && MISSING_P(m,sp->p1dvar))
        continue;

      d->pts_under_brush.els[m] = hits[sp->planar[i].x+1];
      if (hits[sp->planar[i].x+1]) d->npts_under_brush++;
    }
}
  g_free((gpointer) hits);
  
  return d->npts_under_brush;
}

static ggobid *CurrentGGobi = NULL;

gint
barpsort (const void *arg1, const void *arg2)
{
  ggobid *gg = CurrentGGobi;

  gint val = 0;
  gint *x1 = (gint *) arg1;
  gint *x2 = (gint *) arg2;

  if (gg->p1d.gy[*x1] == gg->p1d.gy[*x2]) return 0;
/* to speed things up for categorical variables */

  if (gg->p1d.gy[*x1] < gg->p1d.gy[*x2])
    val = -1;
  else if (gg->p1d.gy[*x1] > gg->p1d.gy[*x2])
    val = 1;

  return (val);
}


gfloat barchart_sort_index (gfloat *yy, gint ny, ggobid *gg, splotd *sp) {
  gint i, *indx;
  gint rank;
  gfloat mindist;

  indx = (gint *) g_malloc (ny * sizeof (gint));

/*
 * gy is needed solely for the psort routine:  psort is used by
 * qsort to put an index vector in the order that yy will assume.
*/
   gg->p1d.gy = (gfloat *) g_malloc (ny * sizeof (gfloat));
  for (i=0; i<ny; i++)
  {
    indx[i] = i;
    gg->p1d.gy[i] = yy[i];
  }
CurrentGGobi = gg;

  qsort ((void *) indx, (size_t) ny, sizeof (gint), barpsort);

CurrentGGobi = NULL;
/*
 * Bug here:  this is screwy if ny < 4.
*/
  if (sp->bar->is_histogram) {
    mindist = 0;

    for (i=0; i < ny; i++) {
      sp->bar->index_to_rank[i] = indx[i];
    }
  } else {
    rank = 0;

    mindist = yy[indx[ny-1]] - yy[indx[0]];
    sp->bar->bins[rank].index = indx[0]; 
    for (i=0; i < ny; i++) {
      if (i > 0) {
        if (yy[indx[i]] != yy[indx[i-1]]) {
          rank++;
          mindist = MIN (yy[indx[i]]-yy[indx[i-1]], mindist);
          sp->bar->bins[rank].index = indx[i];
        }
      }
      sp->bar->index_to_rank[indx[i]] = rank;
    }
  }
  g_free ((gpointer) (gg->p1d.gy));
  g_free ((gpointer) (indx));

  return mindist;
}

void
barchart_scaling_visual_cues_draw (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  vartabled *vtx;
  displayd *display = gg->current_display;
  datad *d = display->d;

  vtx = vartable_element_get (sp->p1dvar, d);

  if (!vtx->categorical_p) { 
/* calculate & draw anchor_rgn */
    gint y = sp->bar->bins[0].rect.y+sp->bar->bins[0].rect.height;   
    gint x = sp->bar->bins[0].rect.x;
    gint halfwidth = sp->bar->bins[0].rect.height/2 -2;


    sp->bar->anchor_rgn[0].x = sp->bar->anchor_rgn[1].x = x-5; 
    sp->bar->anchor_rgn[2].x = x;
    sp->bar->anchor_rgn[0].y = y+halfwidth; 
    sp->bar->anchor_rgn[1].y = y-halfwidth;  
    sp->bar->anchor_rgn[2].y = y;

    button_draw_with_shadows (sp->bar->anchor_rgn, drawable, gg);

/* calculate & draw offset_rgn */
    y = sp->bar->bins[0].rect.y;
    sp->bar->offset_rgn[0].x = sp->bar->offset_rgn[1].x = x-5; 
    sp->bar->offset_rgn[2].x = x;
    sp->bar->offset_rgn[0].y = y+halfwidth;
    sp->bar->offset_rgn[1].y = y-halfwidth;
    sp->bar->offset_rgn[2].y = y;

    button_draw_with_shadows (sp->bar->offset_rgn, drawable, gg);
 

  } 

}

void
button_draw_with_shadows (GdkPoint *region, GdkDrawable *drawable, ggobid *gg) {
  colorschemed *scheme = gg->activeColorScheme;

  gdk_gc_set_foreground (gg->plot_GC, &gg->wvis.gray3);
    gdk_draw_polygon (drawable,gg->plot_GC,TRUE,region,3);

/* dark shadows */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);

  gdk_draw_polygon (drawable,gg->plot_GC,FALSE, region, 3);
  gdk_draw_line (drawable,gg->plot_GC, region[0].x, region[2].y, region[2].x, region[2].y);

/* light shadows */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);

  gdk_draw_line (drawable,gg->plot_GC, region[0].x,region[0].y,region[1].x,region[1].y);
  gdk_draw_line (drawable,gg->plot_GC, region[1].x,region[1].y,region[2].x,region[2].y);
  gdk_draw_line (drawable,gg->plot_GC, region[0].x, region[2].y+1,region[2].x, region[2].y+1);  
}

void
barchart_display_menus_make (displayd *display,
                             GtkAccelGroup *accel_group,
                             GtkSignalFunc func, ggobid *gg)
{
  GtkWidget *topmenu, *options_menu;
  GtkWidget *item;

  display->edge_item = NULL;
  display->edge_menu = NULL;
  scatterplot_display_edge_menu_update (display, accel_group, func, gg);

  /*-- Options menu --*/
  topmenu = submenu_make ("_Options", 'H', accel_group);
  /*-- add a tooltip --*/
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), topmenu,
    "Options menu for this display (barchart)", NULL);

  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuCheck (options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), options_menu);
  submenu_append (topmenu, display->menubar);
  gtk_widget_show (topmenu);
}

gboolean pt_in_rect (icoords pt, GdkRectangle rect) {
  return ((pt.x >= rect.x) && (pt.x <= rect.x+rect.width) && (pt.y >= rect.y) && (pt.y <= rect.y+rect.height));
}

gboolean barchart_identify_bars (icoords mousepos, splotd *sp, datad *d, ggobid *gg) {
/* returns 0 if nothing has changed from the last time */
/*         1 if different bars are hit */
  gint i, nbins;
  gboolean stop;

  nbins = sp->bar->nbins;

  /* check, which bars are hit */
  if (sp->bar->low_pts_missing)
    sp->bar->bar_hit[0] = pt_in_rect(mousepos,sp->bar->high_bin->rect);
  else 
    sp->bar->bar_hit[0] = FALSE;

  for (i=0; i< sp->bar->nbins; i++) {
    sp->bar->bar_hit[i+1] = pt_in_rect(mousepos, sp->bar->bins[i].rect);
  }

  if (sp->bar->high_pts_missing)
    sp->bar->bar_hit [nbins+1] = pt_in_rect(mousepos,sp->bar->high_bin->rect);
  else
    sp->bar->bar_hit [nbins+1] = FALSE;


/* are those bars the same as last time? */
  stop = FALSE;
  for (i=0; (i < nbins+2) && !stop; i++)
    stop = (sp->bar->bar_hit[i] != sp->bar->old_bar_hit[i]);
 
  sp->bar->same_hits = !stop;

  if (!stop) return FALSE; /* nothing else needs to be changed */

/* set old bar hits to match the new results */
  for (i=0; i < nbins+2; i++)
    sp->bar->old_bar_hit[i] = sp->bar->bar_hit[i];

  return TRUE;
}

void barchart_add_bar_cues (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gint i, nbins;
  gchar string[100];
  icoords mousepos = sp->mousepos;
  colorschemed *scheme = gg->activeColorScheme;

  nbins = sp->bar->nbins;
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);


  if (sp->bar->low_pts_missing && sp->bar->bar_hit[0]) {
    sprintf (string,"%ld point%s < %.2f",sp->bar->low_bin->count,
               sp->bar->low_bin->count==1 ? "" : "s",
	       sp->bar->breaks[0]+sp->bar->offset); 

    gdk_draw_rectangle  (drawable, gg->plot_GC, FALSE,
          sp->bar->low_bin->rect.x,sp->bar->low_bin->rect.y,
          sp->bar->low_bin->rect.width,sp->bar->low_bin->rect.height);
    gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      gg->plot_GC,
	  mousepos.x, mousepos.y,
      string);     
  } 
  for (i=1; i<nbins+1; i++) {
    if (sp->bar->bar_hit[i]) {
	if(sp->bar->is_histogram) {
	    sprintf (string,"%ld point%s in (%.2f,%.2f)", sp->bar->bins[i-1].count,
		     sp->bar->bins[i-1].count == 1 ? "" : "s",
		     sp->bar->breaks[i-1]+sp->bar->offset, sp->bar->breaks[i]+sp->bar->offset);
	} else {
	    char *levelName;
            vartabled *var;
	    var = (vartabled *) g_slist_nth_data(sp->displayptr->d->vartable, sp->p1dvar);
	    levelName = var->level_names[i-1];
	    sprintf(string,"%ld point%s for level %s", sp->bar->bins[i-1].count,
		    sp->bar->bins[i-1].count == 1 ? "" : "s", levelName);
	}
      gdk_draw_rectangle(drawable, gg->plot_GC, FALSE,
          sp->bar->bins[i-1].rect.x,sp->bar->bins[i-1].rect.y,
          sp->bar->bins[i-1].rect.width,sp->bar->bins[i-1].rect.height);
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gg->plot_GC, mousepos.x, mousepos.y, string); 
    }
  }
  
  if (sp->bar->high_pts_missing && sp->bar->bar_hit[nbins+1]) {
    sprintf (string,"%ld point%s > %.2f",sp->bar->high_bin->count,
             sp->bar->high_bin->count == 1 ? "" : "s",
	     sp->bar->breaks[nbins]+sp->bar->offset);

    gdk_draw_rectangle  (drawable, gg->plot_GC, FALSE,
          sp->bar->high_bin->rect.x,sp->bar->high_bin->rect.y,
          sp->bar->high_bin->rect.width,sp->bar->high_bin->rect.height);
    gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      gg->plot_GC, mousepos.x, mousepos.y, string);     
 
  }
  
}

#endif
