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

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *da;

static void
delete_cb (GtkWidget *w, GdkEventButton *event, gpointer data)
{
  gtk_widget_hide (w);
}

static gint
da_configure_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  return true;
}

/*

In practice, these will have to be drawn at locations that
can change.

*/

static void
da_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  gint xmargin = 20;
  gint ymargin = 20;
  gint wid = (w->allocation.width - 2*xmargin)/gg->ncolors;
  gint height = w->allocation.height - 2*ymargin;
  gint hgt = height / (gg->ncolors - 1);
  gint k;
  gint x = xmargin;
  gint y = ymargin;

  for (k=0; k<gg->ncolors; k++) {
    gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[k]);
    gdk_draw_rectangle (w->window, gg->plot_GC,
                        TRUE, x, ymargin, wid, height);
    x += wid;
  }

  /*-- draw rectangles, 20 x 10 for the moment --*/
  gdk_gc_set_line_attributes (gg->plot_GC,
    0, GDK_LINE_ON_OFF_DASH, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  x = xmargin + wid; y = ymargin + 10;
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  for (k=1; k<gg->ncolors; k++) {
    gdk_draw_line (w->window, gg->plot_GC,
      xmargin, y, w->allocation.width - xmargin, y);
    gdk_draw_rectangle (w->window, gg->plot_GC,
                        TRUE, x-10, y-5, 20, 10);
    x += wid;
    y += hgt;
  }
  gdk_gc_set_line_attributes (gg->plot_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}

static const gchar *const nclasses_lbl[] = {"5", "6", "7"};
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
    gg->wvis_ui.d = (datad *) gg->d->data;  /*-- arbitrary --*/

    gg->wvis_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->wvis_ui.window),
      "brushing by weights");
    gtk_signal_connect (GTK_OBJECT (gg->wvis_ui.window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), NULL);

    vbox = gtk_vbox_new (false, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
    gtk_container_add (GTK_CONTAINER (gg->wvis_ui.window), vbox);

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
/*
        gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
          GTK_SIGNAL_FUNC (wvis_setdata), GINT_TO_POINTER (i));
*/
        GGobi_widget_set (menuitem, gg, true);
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
    clist = gtk_clist_new (1);
    gtk_clist_set_selection_mode (GTK_CLIST (clist),
      GTK_SELECTION_EXTENDED);

    /*-- populate with the all variables in wvis_ui.d (for now) --*/
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
    gtk_box_pack_start (GTK_BOX (vb), da, true, true, 0);

    gtk_signal_connect (GTK_OBJECT (da),
                        "configure_event",
                        (GtkSignalFunc) da_configure_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (da),
                        "expose_event",
                        (GtkSignalFunc) da_expose_cb,
                        (gpointer) gg);

    gtk_widget_show_all (gg->wvis_ui.window);
  }

  gdk_window_raise (gg->wvis_ui.window->window);
}
