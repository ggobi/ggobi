/* impute_ui.c */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static void
delete_cb (GtkWidget *w, GdkEventButton *event, gpointer data) {
  gtk_widget_hide (w);
}

static void rescalep_cb (GtkToggleButton *button, ggobid *gg)
{
  gg->impute.rescale_p = button->active;
}
static void group_cb (GtkToggleButton *button, ggobid *gg)
{
  gg->impute.vgroup_p = button->active;
}

static void
impute_cb (GtkWidget *w, ggobid *gg) {
  gint impute_type;
  gboolean redraw = true;
  datad *d = gg->current_display->d;

  impute_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (gg->impute.notebook));

  switch (impute_type) {
    case IMP_RANDOM:
      impute_random (d, gg);
      break;
    case IMP_FIXED:
    case IMP_BELOW:
    case IMP_ABOVE:
      redraw = impute_fixed (impute_type, d, gg);
      break;
  }

  if (redraw) {
    if (gg->impute.rescale_p)
      vardata_lim_update (d, gg);
    tform_to_world (d, gg);
    displays_tailpipe (REDISPLAY_ALL, gg);
  }
}

/*------------------------------------------------------------------*/

static gchar *whichvars_lbl[] =
  {"All variables", "Selected variables"};
void whichvars_set (gint which, ggobid *gg) { gg->impute.whichvars = which; }
static void whichvars_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  whichvars_set (GPOINTER_TO_INT (cbd), gg);
}

void
impute_window_open (ggobid *gg) {

  GtkWidget *button, *tgl, *opt;
  GtkWidget *vbox, *frame, *hb;
  GtkWidget *label;
  datad *d = gg->current_display->d;

  if (d == NULL)  /*-- if used before we have data --*/
    return;

  if (gg->impute.window == NULL) {
    
    gg->impute.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->impute.window),
      "impute data");
    gtk_signal_connect (GTK_OBJECT (gg->impute.window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), NULL);
  
    gtk_container_set_border_width (GTK_CONTAINER (gg->impute.window), 5);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->impute.window), vbox);

    /*-- option menu to specify variables --*/
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "For which variables should imputation be performed?",
      NULL);
    populate_option_menu (opt, whichvars_lbl,
      sizeof (whichvars_lbl) / sizeof (gchar *), whichvars_set_cb, gg);
    gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 2);
    

    /*-- Create a new notebook, place the position of the tabs --*/
    gg->impute.notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->impute.notebook),
      GTK_POS_TOP);
    gtk_box_pack_start (GTK_BOX (vbox), gg->impute.notebook,
      false, false, 2);
    
/*
 * Random imputation
*/
    frame = gtk_frame_new ("Random imputation");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    /*-- Add a toggle button, condition on group or not --*/
    tgl = gtk_check_button_new_with_label ("Condition on symbol and color");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Condition the random imputation on the symbol and color; these groups can be seen in the hide/exclude window",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                       GTK_SIGNAL_FUNC (group_cb), (gpointer) gg);
    gtk_container_add (GTK_CONTAINER (frame), tgl);

    label = gtk_label_new ("Random");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->impute.notebook),
      frame, label);
      
/*
 * Fixed: some fixed value
*/
    frame = gtk_frame_new ("Fixed value");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    hb = gtk_hbox_new (false, 3);
    /*-- this border assignment ensures that all hb's have enough border --*/
    gtk_container_set_border_width (GTK_CONTAINER (hb), 10);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    label = gtk_label_new ("fixed value:");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
    gg->impute.entry_val = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hb), gg->impute.entry_val, false, false, 2);

    label = gtk_label_new ("Fixed");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->impute.notebook),
      frame, label);

/*
 * Fixed: a percentage below the minimum
*/
    frame = gtk_frame_new ("Percentage below the minimum");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    hb = gtk_hbox_new (false, 3);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    gg->impute.entry_below = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hb), gg->impute.entry_below, false, false, 2);
    label = gtk_label_new ("% below min");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);

    label = gtk_label_new ("Below");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->impute.notebook),
      frame, label);

/*
 * Fixed: some fixed value
*/
    frame = gtk_frame_new ("Percentage above the maximum");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    hb = gtk_hbox_new (false, 3);
    gtk_container_add (GTK_CONTAINER (frame), hb);

    gg->impute.entry_above = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hb), gg->impute.entry_above, false, false, 2);
    label = gtk_label_new ("% above max");
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);

    label = gtk_label_new ("Above");
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->impute.notebook),
      frame, label);

/* 
 * hbox to hold a few buttons
*/
    
    hb = gtk_hbox_new (true, 2);

    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

    button = gtk_button_new_with_label ("Impute");
    gtk_signal_connect (GTK_OBJECT (button),
                        "clicked",
                        GTK_SIGNAL_FUNC (impute_cb),
                        (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);

    button = gtk_check_button_new_with_label ("Rescale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
                          "Rescale the data when imputing", NULL);
    gtk_signal_connect (GTK_OBJECT (button), "toggled",
                        GTK_SIGNAL_FUNC (rescalep_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);
  }

  gtk_widget_show_all (gg->impute.window);
}
