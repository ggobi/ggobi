/*-- identify_ui.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"


static void
id_remove_labels_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = gg->current_display->d;
  g_slist_free (d->sticky_ids);
  d->sticky_ids = (GSList *) NULL;
  displays_plot (NULL, QUICK, gg);
}
static void
id_all_sticky_cb (GtkWidget *w, ggobid *gg)
{
  gint i, m;
  datad *d = gg->current_display->d;

  /*-- clear the list before adding to avoid redundant entries --*/
  g_slist_free (d->sticky_ids);
  d->sticky_ids = (GSList *) NULL;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    d->sticky_ids = g_slist_append (d->sticky_ids, GINT_TO_POINTER (i));
  }
  displays_plot (NULL, QUICK, gg);
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  
/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/

  return true;
}

void
displays_add_point_labels (splotd *splot, gint k, ggobid *gg) {
  GList *dlist, *slist;
  displayd *display;
  splotd *sp;
  GtkWidget *w;

  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    for (slist = display->splots; slist; slist = slist->next) {
      sp = (splotd *) slist->data;
      if (sp != splot) {
        w = sp->da;
        splot_redraw (sp, QUICK, gg);
      }
    }
  }
}


static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gint k;
  ggobid *gg = GGobiFromSPlot(sp);
  datad *d = gg->current_display->d;
  gboolean button1_p, button2_p;
  gint nd = g_slist_length (gg->d);
  extern void identify_link_by_id (gint k, datad *source_d, ggobid *gg);

/*
 * w = sp->da
 * sp = gtk_object_get_data (GTK_OBJECT (w), "splotd"));
*/

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  k = find_nearest_point (&sp->mousepos, sp, d, gg);
  d->nearest_point = k;

  /*-- link by id --*/
  if (nd > 1) identify_link_by_id (k, d, gg);
  /*-- --*/

  if (k != d->nearest_point_prev) {
    displays_plot (NULL, QUICK, gg);

    if (gg->identify_handler.handler) {
      (gg->identify_handler.handler)(gg->identify_handler.user_data,
        k, sp, w, gg);
    }

    d->nearest_point_prev = k;
  }

  return true;  /* no need to propagate the event */
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
/*
 * If nearest_point is a member of gg->sticky_ids, remove it; if
 * it isn't, add it.
*/

  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = sp->displayptr;
  datad *d = display->d;
  extern void sticky_id_toggle (datad *, ggobid *);
  sticky_id_toggle (d, gg);

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  return true;
}

void
identify_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;
  
  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                        "motion_notify_event",
                                        (GtkSignalFunc) motion_notify_cb,
                                        (gpointer) sp);
  } else {
    if (sp->key_press_id) {
      gtk_signal_disconnect (GTK_OBJECT (display->window), sp->key_press_id);
      sp->key_press_id = 0;
    }
    if (sp->press_id) {
      gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
      sp->press_id = 0;
    }
    if (sp->release_id) {
      gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
      sp->release_id = 0;
    }
    if (sp->motion_id) {
      gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);
      sp->motion_id = 0;
    }
  }

}


/*----------------------------------------------------------------------*/
/*                   Resetting the main menubar                         */
/*----------------------------------------------------------------------*/

void
identify_menus_make (ggobid *gg) {
/*
 * Options menu
*/
  gg->menus.options_item = submenu_make ("_Options", 'O',
    gg->main_accel_group);
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->mode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
  submenu_insert (gg->menus.options_item, gg->main_menubar, OPTIONS_MENU_POS);
}

void
cpanel_identify_make(ggobid *gg) {
  GtkWidget *btn;
  
  gg->control_panel[IDENT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER(gg->control_panel[IDENT]), 5);

/*
 * button for removing all labels
*/
  btn = gtk_button_new_with_label ("Remove labels");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
                        btn, "Remove all labels", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (id_remove_labels_cb),
                      gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
                      btn, false, false, 1);

/*
 * button for making all labels sticky
*/
  btn = gtk_button_new_with_label ("Make all sticky");
  gtk_tooltips_set_tip (GTK_TOOLTIPS(gg->tips),
                        btn, "Make all labels sticky, or persistent", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (id_all_sticky_cb),
                      (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[IDENT]),
                      btn, false, false, 1);

  gtk_widget_show_all (gg->control_panel[IDENT]);
}


