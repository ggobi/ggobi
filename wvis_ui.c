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

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*-- these will probably all become members of gg->wvis */
static GtkWidget *da;
static GdkPixmap *pix;

static gfloat *pct;
static gint npct = 0;
static gint nearest_color = -1;
static gint xmargin = 20;
static gint ymargin = 20;

static gint motion_notify_id = 0;
static icoords mousepos;
static gint nearest_color;
/* */

static void
delete_cb (GtkWidget *w, GdkEventButton *event, gpointer data)
{
  gtk_widget_hide (w);
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

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  if (pos.x != mousepos.x) {
    val = (gfloat) (pos.x - xmargin) /
          (gfloat) (w->allocation.width - 2*xmargin);

    /*-- don't allow it to cross its neighbors' boundaries --*/
    if (val >= pct[nearest_color-1] && val <= pct[nearest_color+1]) {
      pct[nearest_color] = val;
      gtk_signal_emit_by_name (GTK_OBJECT (w), "expose_event",
        "expose_event",
        (gpointer) gg, (gpointer) &rval);
    }
  }

  mousepos.x = pos.x;  

  return true;
}

/*-- when the button is pressed, listen for motion notify events --*/
static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  GdkModifierType state;
  icoords pos;
  gint k, x, y, nearest = -1, d;
  gint hgt = (w->allocation.height - 2*ymargin) / (gg->ncolors - 1);
  gint dist = w->allocation.width*w->allocation.width +
              w->allocation.height*w->allocation.height;

  gdk_window_get_pointer (w->window, &pos.x, &pos.y, &state);

  /*-- find nearest slider --*/
  y = ymargin + 10;
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
    d = (pos.x-x)*(pos.x-x) + (pos.y-y)*(pos.y-y);
    if (d < 100 && d < dist) {
      nearest = k;
      dist = d;
    }
    y += hgt;
  }

  nearest_color = nearest;

  if (nearest_color != -1)
    motion_notify_id = gtk_signal_connect (GTK_OBJECT (w),
                       "motion_notify_event",
                       (GtkSignalFunc) motion_notify_cb,
                       (gpointer) gg);
  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  gtk_signal_disconnect (GTK_OBJECT (w), motion_notify_id);
  motion_notify_id = 0;

  return true;
}

static gint
da_configure_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  /*-- Create new backing pixmaps of the appropriate size --*/
  if (pix != NULL)
    gdk_pixmap_unref (pix);
  pix = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  gtk_widget_queue_draw (w);

  return false;
}

static void
da_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  gint height = w->allocation.height - 2*ymargin;
  gint hgt = height / (gg->ncolors - 1);
  gint k;
  gint x0, x1;
  gint x = xmargin;
  gint y = ymargin;
  GdkColormap *cmap = gdk_colormap_get_system ();
  GdkColor gray1, gray2, gray3;
  gboolean writeable = false, best_match = true, success;
  GdkPoint *points;
/*
 * get the current d and the selected variable 
 *   in the clist in order to draw the labels
*/
  GtkWidget *clist = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(w), "clist");
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  GList *selection = GTK_CLIST (clist)->selection;
  gint selected_var = -1;

  if (selection) selected_var = (gint) selection->data;

  if (npct != gg->ncolors) {
    npct = gg->ncolors;
    pct = (gfloat *) g_realloc (pct, npct * sizeof (npct));
    for (k=0; k<npct; k++) {
      pct[k] = (gfloat) (k+1) /  (gfloat) npct;
    }
  }

  gray1.red = gray1.blue = gray1.green = (guint16) (.3*65535.0);
  gray2.red = gray2.blue = gray2.green = (guint16) (.5*65535.0);
  gray3.red = gray3.blue = gray3.green = (guint16) (.7*65535.0);
  success = gdk_colormap_alloc_color(cmap, &gray1, writeable, best_match);
  success = gdk_colormap_alloc_color(cmap, &gray2, writeable, best_match);
  success = gdk_colormap_alloc_color(cmap, &gray3, writeable, best_match);

  /*-- clear the pixmap --*/
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (pix, gg->plot_GC, TRUE,
                      0, 0, w->allocation.width, w->allocation.height);


  /*-- draw the color bars --*/
  x0 = xmargin;
  for (k=0; k<gg->ncolors; k++) {
    x1 = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
    gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[k]);
    gdk_draw_rectangle (pix, gg->plot_GC,
                        TRUE, x0, ymargin, x1 - x0, height);
    x0 = x1;
  }

  /*-- draw the horizontal lines --*/
  x0 = xmargin; y = ymargin + 10;
  x1 = xmargin + (w->allocation.width - 2*xmargin) - 1;
  gdk_gc_set_foreground (gg->plot_GC, &gray2);
  for (k=0; k<gg->ncolors-1; k++) {
    gdk_draw_line (pix, gg->plot_GC, x0, y, x1, y);
    y += hgt;
  }

  /*-- draw rectangles, 20 x 10 for the moment --*/
  y = ymargin + 10;
  gdk_gc_set_foreground (gg->plot_GC, &gray2);
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
    gdk_draw_rectangle (pix, gg->plot_GC,
                        TRUE, x-10, y-5, 20, 10);
    y += hgt;
  }


  /*-- draw the dark shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->plot_GC, &gray1);
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);

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
    gdk_draw_polygon (pix, gg->plot_GC,
                      TRUE, points, 7);
    gdk_draw_line (pix, gg->plot_GC, x-1, y-4, x-1, y+3);

    y += hgt;
  }

  /*-- draw the light shadows --*/
  y = ymargin + 10;
  points = (GdkPoint *) g_malloc (7 * sizeof (GdkPoint));
  gdk_gc_set_foreground (gg->plot_GC, &gray3);
  for (k=0; k<gg->ncolors-1; k++) {
    x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);

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
    gdk_draw_polygon (pix, gg->plot_GC,
                      TRUE, points, 7);
    gdk_draw_line (pix, gg->plot_GC, x, y-4, x, y+3);

    y += hgt;
  }
/*
  gdk_color_free (&gray1);
  gdk_color_free (&gray2);
  gdk_color_free (&gray3);
*/

  /*-- add the variable limits in the margin --*/
  if (d && selected_var != -1) {
    gfloat min = d->vartable[selected_var].lim_tform.min;
    gfloat max = d->vartable[selected_var].lim_tform.max;
    gfloat val;
    gchar *str;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style = gtk_widget_get_style (da);

    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
    y = ymargin;
    for (k=0; k<gg->ncolors-1; k++) {
      val = min + pct[k] * (max - min);
      str = g_strdup_printf ("%3.3g", val);
      gdk_text_extents (style->font, str, strlen(str),
        &lbearing, &rbearing, &width, &ascent, &descent);
      x = xmargin + pct[k] * (w->allocation.width - 2*xmargin);
      gdk_draw_string (pix, style->font, gg->plot_GC,
        x - width/2,
        y - 2,
        str);
      g_free (str);
    }
  }

  gdk_draw_pixmap (w->window, gg->plot_GC, pix,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

/*-- update the contents of the clist --*/
static void wvis_setdata_cb (GtkWidget *w, datad *d)
{
  ggobid *gg = GGobiFromWidget(w, true);
  gboolean rval = false;
  GtkWidget *clist = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(w), "clist");
  gint j;
  gchar *row[1];

  gtk_clist_clear (GTK_CLIST (clist));

  gtk_clist_freeze (GTK_CLIST (clist));
  for (j=0; j<d->ncols; j++) {
    row[0] = g_strdup_printf (d->vartable[j].collab_tform);
    gtk_clist_append (GTK_CLIST (clist), row);
  }
  gtk_clist_thaw (GTK_CLIST (clist));

  gtk_object_set_data (GTK_OBJECT (clist), "datad", d);
  gtk_signal_emit_by_name (GTK_OBJECT (da), "expose_event",
    "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

void
selection_made_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gboolean rval = false;
  gtk_signal_emit_by_name (GTK_OBJECT (da), "expose_event",
    "expose_event",
    (gpointer) gg, (gpointer) &rval);
}

/*-- I need datad, the variable, and pct[] --*/
static void scale_apply_cb (GtkButton *w, ggobid* gg)
{
  GtkWidget *clist = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(w), "clist");
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  GList *selection = GTK_CLIST (clist)->selection;
  gint selected_var = -1;
  if (selection) selected_var = (gint) selection->data;

  if (d && selected_var != -1) {
    gint i, m, k;
    gfloat min = d->vartable[selected_var].lim_tform.min;
    gfloat max = d->vartable[selected_var].lim_tform.max;
    gfloat val;

    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];

      for (k=0; k<gg->ncolors; k++) {
        val = min + pct[k] * (max - min);
        if (d->tform.vals[i][selected_var] <= val) {
          d->color_now.els[i] = k;
          break;
        }
      }
    }
    displays_plot (NULL, FULL, gg);
  }
}

void
wvis_window_open (ggobid *gg, guint action, GtkWidget *w) {
  GtkWidget *opt, *vbox, *hbox, *hb, *menu, *menuitem;
  GtkWidget *swin, *clist, *btn, *vb;
  gint nd = g_slist_length (gg->d);
  gint j;
  GSList *l;
  datad *d;
  gchar *row[1];

  if (gg->wvis_ui.window == NULL) {

    gg->wvis_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->wvis_ui.window),
      "brushing by weights");
    gtk_signal_connect (GTK_OBJECT (gg->wvis_ui.window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), NULL);

    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
    gtk_container_add (GTK_CONTAINER (gg->wvis_ui.window), vbox);

    /*-- defined here rather than later so that it can be used in the menu --*/
    clist = gtk_clist_new (1);
    /*-- --*/

    if (nd > 0) {

      /*-- option menu to specify the datad --*/
      hbox = gtk_hbox_new (false, 0);
      gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 0);
      hb = gtk_hbox_new (false, 0);
      gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 0);

      gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Dataset:"),
        false, false, 2);

      opt = gtk_option_menu_new ();
      gtk_widget_set_name (opt, "WEIGHTEDVIS:datad_option_menu");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "Specify the dataset to use.",
        NULL);

      menu = gtk_menu_new ();
      for (l = gg->d; l; l = l->next) {
        d = (datad *) l->data;
        menuitem = gtk_menu_item_new_with_label (d->name);
        gtk_menu_append (GTK_MENU (menu), menuitem);
        gtk_widget_show (menuitem) ;
        gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
          GTK_SIGNAL_FUNC (wvis_setdata_cb), d);
        GGobi_widget_set (menuitem, gg, true);
        gtk_object_set_data (GTK_OBJECT (menuitem), "clist", clist);
      }
      gtk_option_menu_set_menu (GTK_OPTION_MENU (opt), menu);
      gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);
    }

    /* Create a scrolled window to pack the CList widget into */
    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_box_pack_start (GTK_BOX (vbox), swin, false, false, 0);

    /*-- variable list --*/
    /*clist = gtk_clist_new (1);*/  /*-- created earlier --*/
    gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_SINGLE);
    gtk_object_set_data (GTK_OBJECT (clist), "datad", gg->d->data);
    gtk_signal_connect (GTK_OBJECT (clist), "select_row",
                       GTK_SIGNAL_FUNC (selection_made_cb),
                       gg);

    /*-- populate with the all variables the first datad --*/
    d = gg->d->data;
    for (j=0; j<d->ncols; j++) {
      row[0] = g_strdup_printf (d->vartable[j].collab_tform);
      gtk_clist_append (GTK_CLIST (clist), row);
    }
    gtk_container_add (GTK_CONTAINER (swin), clist);


    btn = gtk_button_new_with_label ("Update scale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update the scale of symbols", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
/*
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_update_cb), gg);
*/

    /*-- colors, symbols --*/
    /*-- now we get fancy:  draw the scale, with glyphs and colors --*/
    vb = gtk_vbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), vb, true, true, 0);
    da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (da), 400, 200);
    gtk_object_set_data (GTK_OBJECT (da), "clist", clist);
    gtk_box_pack_start (GTK_BOX (vb), da, true, true, 0);

    gtk_signal_connect (GTK_OBJECT (da),
                        "configure_event",
                        (GtkSignalFunc) da_configure_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (da),
                        "expose_event",
                        (GtkSignalFunc) da_expose_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (da),
                        "button_press_event",
                        (GtkSignalFunc) button_press_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (da),
                        "button_release_event",
                        (GtkSignalFunc) button_release_cb,
                        (gpointer) gg);

    gtk_widget_set_events (da, GDK_EXPOSURE_MASK
               | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
               | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

    btn = gtk_button_new_with_label ("Apply");
    gtk_object_set_data (GTK_OBJECT (btn), "clist", clist);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Apply the color scale", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_apply_cb), gg);

    gtk_widget_show_all (gg->wvis_ui.window);
  }

  gdk_window_raise (gg->wvis_ui.window->window);
}
