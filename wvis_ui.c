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

static gint xmargin = 20;
static gint ymargin = 20;

static void bin_counts_reset (gint jvar, datad *d, ggobid *gg);

extern void color_table_init_from_scheme (colorschemed *scheme,
  GdkColor *color_table, GdkColor *bg_color, GdkColor *accent_color);
extern void color_table_init_from_default (GdkColor *color_table,
  GdkColor *bg_color, GdkColor *accent_color);


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
 * gg->wvis sometimes has its own color_table.  If it's null, use
 * gg->color_table; else use the local one.  Then only initialize
 * the global one (gg->color_table) when Apply is clicked.
*/
  if (gg->wvis.color_table) {
/*
    gdk_colormap_free_colors (gdk_colormap_get_system(),
      gg->wvis.color_table, gg->wvis.ncolors);
    gdk_colormap_free_colors (gdk_colormap_get_system(),
                              &gg->wvis.bg_color, 1);
    gdk_colormap_free_colors (gdk_colormap_get_system(),
                              &gg->wvis.accent_color, 1);
*/
    g_free (gg->wvis.color_table);
    gg->wvis.color_table = NULL;
    gg->wvis.scheme = NULL;
  }

  if (scheme) {
    gg->wvis.scheme = scheme;
    gg->wvis.ncolors = scheme->n;
    gg->wvis.color_table = (GdkColor *)
      g_malloc (scheme->n * sizeof (GdkColor));
    color_table_init_from_scheme (scheme, gg->wvis.color_table,
      &gg->wvis.bg_color, &gg->wvis.accent_color);
  }
  else {
    gg->wvis.scheme = NULL;
    gg->wvis.ncolors = MAXNCOLORS;
    gg->wvis.color_table = (GdkColor *)
      g_malloc (MAXNCOLORS * sizeof (GdkColor));
    color_table_init_from_default (gg->wvis.color_table,
      &gg->wvis.bg_color, &gg->wvis.accent_color);
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
  d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  selected_var = get_one_selection_from_clist (clist);
  if (d && selected_var != -1) {
    gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
      (gpointer) gg, (gpointer) &rval);
    bin_counts_reset (selected_var, d, gg);
  }
  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

static void
colorscheme_add_to_menu (GtkWidget *menu, colorschemed *scheme,
  gpointer ptr, ggobid *gg)
{
  GtkWidget *menuitem;

  if (scheme) {
    menuitem = gtk_menu_item_new_with_label (scheme->name);
  } else {
    menuitem = gtk_menu_item_new_with_label ("Default");
  }

  gtk_object_set_data (GTK_OBJECT (menuitem), "notebook", ptr);

  gtk_menu_append (GTK_MENU (menu), menuitem);
  gtk_widget_show (menuitem) ;
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
    colorscheme_set_cb, scheme);
  GGobi_widget_set (menuitem, gg, true);
}


/*-------------------------------------------------------------------------*/

static void
bin_counts_reset (gint jvar, datad *d, ggobid *gg)
{
  gint i, k, m;
  gfloat val;
  vartabled *vt = vartable_element_get (jvar, d);
  gfloat min = vt->lim_tform.min;
  gfloat max = vt->lim_tform.max;
  gint ncolors;

  if (gg->wvis.color_table == NULL) {
    ncolors = gg->ncolors;
  } else {
    ncolors = gg->wvis.ncolors;
  }

  for (k=0; k<gg->wvis.npct; k++)
    gg->wvis.n[k] = 0;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    for (k=0; k<ncolors; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      if (d->tform.vals[i][jvar] <= val) {
        gg->wvis.n[k]++;
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
  gint selected_var = get_one_selection_from_clist (clist);

  icoords *mousepos = &gg->wvis.mousepos;
  gint nearest_color = gg->wvis.nearest_color;

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  if (pos.x != mousepos->x) {
    val = (gfloat) (pos.x - xmargin) /
          (gfloat) (w->allocation.width - 2*xmargin);

    /*-- don't allow it to cross its neighbors' boundaries --*/
    if (val >= gg->wvis.pct[nearest_color-1] &&
        val <= gg->wvis.pct[nearest_color+1])
    {
      gg->wvis.pct[nearest_color] = val;

      if (selected_var != -1 && selected_var < d->ncols)
        bin_counts_reset (selected_var, d, gg);

      gtk_signal_emit_by_name (GTK_OBJECT (w), "expose_event",
        (gpointer) gg, (gpointer) &rval);
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
  gint ncolors;
  GdkColor *color_table;

  gfloat *pct = gg->wvis.pct;
  gint *nearest_color = &gg->wvis.nearest_color;
  gint hgt;

  if (gg->wvis.color_table == NULL) {
    ncolors = gg->ncolors;
    color_table = gg->color_table;
  } else {
    ncolors = gg->wvis.ncolors;
    color_table = gg->wvis.color_table;
  }
  hgt = (w->allocation.height - 2*ymargin) / (ncolors - 1);

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  /*-- find nearest slider --*/
  y = ymargin + 10;
  for (k=0; k<ncolors-1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
    d = (pos.x-x)*(pos.x-x) + (pos.y-y)*(pos.y-y);
    if (d < 100 && d < dist) {
      nearest = k;
      dist = d;
    }
    y += hgt;
  }

  *nearest_color = nearest;

  if (*nearest_color != -1)
    gg->wvis.motion_notify_id = gtk_signal_connect (GTK_OBJECT (w),
                                "motion_notify_event",
                                (GtkSignalFunc) motion_notify_cb,
                                (gpointer) gg);
  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  if (gg->wvis.motion_notify_id) {
    gtk_signal_disconnect (GTK_OBJECT (w), gg->wvis.motion_notify_id);
    gg->wvis.motion_notify_id = 0;
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
  gint ncolors;
  GdkColor *color_table, *bg_color, *accent_color;

  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist);

  GtkWidget *da = gg->wvis.da;
  GdkPixmap *pix = gg->wvis.pix;

  if (gg->wvis.GC == NULL)
    gg->wvis.GC = gdk_gc_new (w->window);

  if (gg->wvis.color_table == NULL) {
    ncolors = gg->ncolors;
    color_table = gg->color_table;
    bg_color = &gg->bg_color;
    accent_color = &gg->accent_color;
  } else {
    ncolors = gg->wvis.ncolors;
    color_table = gg->wvis.color_table;
    bg_color = &gg->wvis.bg_color;
    accent_color = &gg->wvis.accent_color;
  }
  hgt = height / (ncolors - 1);

  if (gg->wvis.npct != ncolors) {
    gg->wvis.npct = ncolors;
    gg->wvis.pct = (gfloat *) g_realloc (gg->wvis.pct,
                                         gg->wvis.npct * sizeof (gfloat));
    gg->wvis.n = (gint *) g_realloc (gg->wvis.n,
                                     gg->wvis.npct * sizeof (gint));
    for (k=0; k<gg->wvis.npct; k++) {
      gg->wvis.pct[k] = (gfloat) (k+1) /  (gfloat) gg->wvis.npct;
      gg->wvis.n[k] = 0;
    }
  }

  /*-- clear the pixmap --*/
  gdk_gc_set_foreground (gg->wvis.GC, bg_color);
  gdk_draw_rectangle (pix, gg->wvis.GC, TRUE,
                      0, 0, w->allocation.width, w->allocation.height);


  /*-- draw the color bars --*/
  x0 = xmargin;
  for (k=0; k<ncolors; k++) {
    x1 = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    gdk_gc_set_foreground (gg->wvis.GC, &color_table[k]);
    gdk_draw_rectangle (pix, gg->wvis.GC,
                        TRUE, x0, ymargin, x1 - x0, height);
    x0 = x1;
  }

  /*-- draw the horizontal lines --*/
  x0 = xmargin; y = ymargin + 10;
  x1 = xmargin + (w->allocation.width - 2*xmargin) - 1;
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray2);
  for (k=0; k<ncolors-1; k++) {
    gdk_draw_line (pix, gg->wvis.GC, x0, y, x1, y);
    y += hgt;
  }

  /*-- draw rectangles, 20 x 10 for the moment --*/
  y = ymargin + 10;
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray2);
  for (k=0; k<ncolors-1; k++) {
    x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
    gdk_draw_rectangle (pix, gg->wvis.GC,
                        TRUE, x-10, y-5, 20, 10);
    y += hgt;
  }


  /*-- draw the dark shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray1);
  for (k=0; k<ncolors-1; k++) {
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

  /*-- draw the light shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->wvis.GC, &gg->wvis.gray3);
  for (k=0; k<ncolors-1; k++) {
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

  /*-- add the variable limits in the top margin --*/
  if (d && selected_var != -1) {
    gfloat min, max;
    gfloat val;
    gchar *str;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style = gtk_widget_get_style (da);

    vt = vartable_element_get (selected_var, d);
    min = vt->lim_tform.min;
    max = vt->lim_tform.max;

    gdk_gc_set_foreground (gg->wvis.GC, accent_color);
    y = ymargin;
    for (k=0; k<ncolors-1; k++) {

      val = min + gg->wvis.pct[k] * (max - min);
      str = g_strdup_printf ("%3.3g", val);
      gdk_text_extents (style->font, str, strlen(str),
        &lbearing, &rbearing, &width, &ascent, &descent);
      x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
      gdk_draw_string (pix, style->font, gg->wvis.GC,
        x - width/2,
        y - 2,
        str);
      g_free (str);
    }

    /*-- ... and the counts in the bottom margin --*/
    for (k=0; k<ncolors; k++) {
      val = min + gg->wvis.pct[k] * (max - min);
      str = g_strdup_printf ("%d", gg->wvis.n[k]);
      gdk_text_extents (style->font, str, strlen(str),
        &lbearing, &rbearing, &width, &ascent, &descent);
      x = xmargin + gg->wvis.pct[k] * (w->allocation.width - 2*xmargin);
      diff = (k == 0) ? gg->wvis.pct[k] : gg->wvis.pct[k]-gg->wvis.pct[k-1]; 
      x -= diff/2 * (w->allocation.width - 2*xmargin);
      gdk_draw_string (pix, style->font, gg->wvis.GC,
        x - width/2,
        (w->allocation.height - ymargin) + ascent + descent + 2,
        str);
      g_free (str);
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

  bin_counts_reset (row, d, gg);
  gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

static void scale_apply_cb (GtkWidget *w, ggobid* gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT (w));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint selected_var = get_one_selection_from_clist (clist);
  vartabled *vt;
  extern void symbol_window_redraw (ggobid *);

  if (d && selected_var != -1) {
    gint i, m, k;
    gfloat min, max;
    gfloat val;
    gboolean rval = false;

    /*
     * If we've been using gg->wvis.color_table, init gg->color_table
     * using the current scheme.  Then free wvis.color_table, since it
     * is now redundant.
    */
    if (gg->wvis.color_table) {  /* ie, wvis.color_table != gg->color_table */
/*
      gdk_colormap_free_colors (gdk_colormap_get_system(),
        gg->color_table, gg->ncolors);
      gdk_colormap_free_colors (gdk_colormap_get_system(),
                                &gg->bg_color, 1);
      gdk_colormap_free_colors (gdk_colormap_get_system(),
                                &gg->accent_color, 1);
*/
      g_free (gg->color_table);

      if (gg->wvis.scheme) {
        colorschemed *scheme = gg->wvis.scheme;
        gg->ncolors = scheme->n;
        gg->color_table = (GdkColor *)
          g_malloc (scheme->n * sizeof (GdkColor));
        color_table_init_from_scheme (scheme, gg->color_table,
          &gg->bg_color, &gg->accent_color);
      } else {
        gg->ncolors = MAXNCOLORS;
        gg->color_table = (GdkColor *)
          g_malloc (MAXNCOLORS * sizeof (GdkColor));
        color_table_init_from_default (gg->color_table,
          &gg->bg_color, &gg->accent_color);
      }

/*
      gdk_colormap_free_colors (gdk_colormap_get_system(),
        gg->wvis.color_table, gg->wvis.ncolors);
      gdk_colormap_free_colors (gdk_colormap_get_system(),
                                &gg->wvis.bg_color, 1);
      gdk_colormap_free_colors (gdk_colormap_get_system(),
                                &gg->wvis.accent_color, 1);
*/
      g_free (gg->wvis.color_table);
      gg->wvis.color_table = NULL;
      gg->wvis.scheme = NULL;
      gg->wvis.ncolors = 0;
    }

    vt = vartable_element_get (selected_var, d);
    min = vt->lim_tform.min;
    max = vt->lim_tform.max;

    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];

      for (k=0; k<gg->ncolors; k++) {
        val = min + gg->wvis.pct[k] * (max - min);
        if (d->tform.vals[i][selected_var] <= val) {
          d->color.els[i] = d->color_now.els[i] = k;
          break;
        }
      }
    }
    clusters_set (d, gg);
    displays_plot (NULL, FULL, gg);

    bin_counts_reset (selected_var, d, gg);
    gtk_signal_emit_by_name (GTK_OBJECT (gg->wvis.da), "expose_event",
      (gpointer) gg, (gpointer) &rval);

    symbol_window_redraw (gg);
    cluster_table_update (d, gg);
  }
}

void
wvis_window_open (ggobid *gg) {
  GtkWidget *vbox, *hb;
  GtkWidget *notebook;
  GtkWidget *btn, *vb;
  /*-- for colorscales --*/
  GtkWidget *frame, *opt, *menu;
  GList *l;
  colorschemed *scheme;
  /* */

  if (gg->wvis.window == NULL) {

    gg->wvis.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->wvis.window),
      "color by variable");
    gtk_signal_connect (GTK_OBJECT (gg->wvis.window),
      "delete_event", GTK_SIGNAL_FUNC (close_wmgr_cb), gg);

    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
    gtk_container_add (GTK_CONTAINER (gg->wvis.window), vbox);

    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vbox, GTK_SELECTION_SINGLE,
      (GtkSignalFunc) selection_made_cb, gg);

  /*
   * section on choosing new colormap
  */
    frame = gtk_frame_new ("Choose colormap");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, false, 1);

    opt = gtk_option_menu_new ();
    menu = gtk_menu_new ();

    colorscheme_add_to_menu (menu, NULL, notebook, gg);
    for (l = sessionOptions->colorSchemes; l; l = l->next) {
      scheme = (colorschemed *) l->data;
      colorscheme_add_to_menu (menu, scheme, notebook, gg);
/*
      menuitem = gtk_menu_item_new_with_label (scheme->name);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_show (menuitem) ;
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
        colorscheme_set_cb, scheme);
      GGobi_widget_set (menuitem, gg, true);
*/
    }
    gtk_option_menu_set_menu (GTK_OPTION_MENU (opt), menu);

    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Choose a color scale", NULL);
    gtk_container_add (GTK_CONTAINER (frame), opt);
  /**/

    /*-- colors, symbols --*/
    /*-- now we get fancy:  draw the scale, with glyphs and colors --*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, true, true, 0);
    gg->wvis.da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->wvis.da), 400, 200);
    gtk_object_set_data (GTK_OBJECT (gg->wvis.da), "notebook", notebook);
    gtk_box_pack_start (GTK_BOX (vb), gg->wvis.da, true, true, 0);

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

    btn = gtk_button_new_with_label ("Apply");
    gtk_object_set_data (GTK_OBJECT (btn), "notebook", notebook);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Apply the color scale", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_apply_cb), gg);

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

