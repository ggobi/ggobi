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

static void
delete_cb (GtkWidget *w, GdkEventButton *event, gpointer data)
{
  gtk_widget_hide (w);
}

static void
nclasses_set_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
}

static const gchar *const nclasses_lbl[] = {"5", "6", "7", "8"};
void
wvis_window_open (ggobid *gg, guint action, GtkWidget *w) {
  GtkWidget *opt, *vbox, *hbox, *hb, *menu, *menuitem;
  GtkWidget *swin, *clist, *btn, *entry;
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


    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 0);

    /*-- button: delete selected variables --*/
    btn = gtk_button_new_with_label ("Remove selected");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update the list, removing the selected variables", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 1);
/*
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (remove_selected_cb), gg);
*/

    /*-- button: retain only the selected variables --*/
    btn = gtk_button_new_with_label ("Retain selected");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update the list, retaining only the selected variables", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 1);
/*
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (retain_selected_cb), gg);
*/

    /*-- min, center, max --*/
    hb = gtk_hbox_new (false, 0);

    gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Min:"),
      false, false, 2);
    entry = gtk_entry_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "Minimum value", NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry,
      true, true, 2);

    gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Center:"),
      false, false, 2);
    entry = gtk_entry_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "Center value.  If this is filled in, the scale will be double-ended",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry,
      true, true, 2);

    gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Max:"),
      false, false, 2);
    entry = gtk_entry_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "Maximum value", NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry,
      true, true, 2);

    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    /*-- number of classes; single- or double-headed --*/
    hbox = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 0);
    hb = gtk_hbox_new (false, 0);
    gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);

    /*-- n classes option menu --*/
    gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("N classes:"),
      false, false, 2);
    opt = gtk_option_menu_new ();
    gtk_widget_set_name (opt, "WEIGHTEDVIS:nclasses_option_menu");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Choose the number of classes.",
      NULL);
    populate_option_menu (opt, (gchar**) nclasses_lbl,
                          sizeof (nclasses_lbl) / sizeof (gchar *),
                          (GtkSignalFunc) nclasses_set_cb, gg);
    gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);

    btn = gtk_button_new_with_label ("Update scale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update the scale of symbols", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 0);
/*
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scale_update_cb), gg);
*/

    /*-- colors, symbols --*/
    /*-- now we get fancy:  draw the scale, with glyphs and colors --*/

    gtk_widget_show_all (gg->wvis_ui.window);
  }

  gdk_window_raise (gg->wvis_ui.window->window);
}
