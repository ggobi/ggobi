/* impute_ui.c */  /*-- should be called missing_ui.c --*/
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

/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, ggobid *gg) {
  gtk_widget_hide (gg->impute.window);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
  gtk_widget_hide (gg->impute.window);
}

static void rescale_cb (GtkButton *button, ggobid *gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->impute.window));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");

  limits_set (true, true, d, gg);
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);
}
static void group_cb (GtkToggleButton *button, ggobid *gg)
{
  gg->impute.bgroup_p = button->active;
}
static void 
show_missings_cb (GtkToggleButton *button, ggobid *gg)
{
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->impute.window));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");

  d->missings_show_p = button->active;
  displays_tailpipe (FULL, gg);
}

static void
impute_cb (GtkWidget *w, ggobid *gg) {
  gint impute_type;
  gboolean redraw = true;
  GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->impute.window));
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  gint *vars = (gint *) g_malloc (d->ncols * sizeof(gint));
  gint nvars = get_selections_from_clist (d->ncols, vars, clist);

  impute_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (gg->impute.notebook));

  switch (impute_type) {
    case IMP_RANDOM:
      impute_random (d, nvars, vars, gg);
    break;
    case IMP_FIXED:
    case IMP_BELOW:
    case IMP_ABOVE:
      redraw = impute_fixed (impute_type, nvars, vars, d, gg);
    break;
  }

  if (redraw) {
    tform_to_world (d, gg);
    displays_tailpipe (FULL, gg);
  }

  g_free (vars);
}

/*------------------------------------------------------------------*/

void
impute_window_open (ggobid *gg)
{
  GtkWidget *frame0, *vb;
  GtkWidget *btn, *tgl, *notebook;
  GtkWidget *vbox, *frame, *hb;
  GtkWidget *label;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
/**/return;

  if (gg->impute.window == NULL) {
    
    gg->impute.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->impute.window),
      "Missing values");
    gtk_signal_connect (GTK_OBJECT (gg->impute.window),
      "delete_event", GTK_SIGNAL_FUNC (close_wmgr_cb), NULL);
  
    gtk_container_set_border_width (GTK_CONTAINER (gg->impute.window), 5);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->impute.window), vbox);

    /*-- Add a toggle button, show missings or not --*/
    tgl = gtk_check_button_new_with_label ("Show missing values");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(tgl), on);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Draw the missing values when plotting displays; if there are multiple datasets, this applies only to the current dataset",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
      GTK_SIGNAL_FUNC (show_missings_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (vbox), tgl, true, true, 2);


    /*-- add a button to generate a new datad --*/
    btn = gtk_button_new_with_label ("Add missings as new dataset");
    gtk_signal_connect (GTK_OBJECT (btn),
      "clicked", GTK_SIGNAL_FUNC (missings_datad_cb), (gpointer) gg);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Generate a new dataset from the 1's and 0's representing missingness",
      NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn, true, true, 2);

    /*-- add a frame to contain the "imputation" widgets --*/
    frame0 = gtk_frame_new ("Assign values");
    gtk_container_set_border_width (GTK_CONTAINER (frame0), 2);
    gtk_box_pack_start (GTK_BOX (vbox), frame0, true, true, 2);

    vb = gtk_vbox_new (false, 2);
    /*-- this has the effect of setting an internal border inside the frame --*/
    gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
    gtk_container_add (GTK_CONTAINER (frame0), vb);
    
    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vb,
      GTK_SELECTION_EXTENDED,
      (GtkSignalFunc) NULL, gg);

    /*-- Create a new notebook, place the position of the tabs --*/
    gg->impute.notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->impute.notebook),
      GTK_POS_TOP);
    gtk_box_pack_start (GTK_BOX (vb), gg->impute.notebook,
      false, false, 2);
    
/*
 * Random imputation
*/
    frame = gtk_frame_new ("Random imputation");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

    /*-- Add a toggle button, condition on group or not --*/
    tgl = gtk_check_button_new_with_label ("Condition on symbol and color");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
      "Condition the random imputation on the symbol and color; these groups can be seen in the case clusters window",
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
 * Fixed: percentage above the max
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

   /*-- hbox to hold a few buttons --*/
    hb = gtk_hbox_new (true, 2);

    gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 2);

    btn = gtk_button_new_with_label ("Impute");
    gtk_signal_connect (GTK_OBJECT (btn),
                        "clicked",
                        GTK_SIGNAL_FUNC (impute_cb),
                        (gpointer) gg);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Impute or assign values to missings", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 2);

    btn = gtk_button_new_with_label ("Rescale");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
                          "Rescale the data after imputing", NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (rescale_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, true, 2);

    /*-- add a close button --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_with_label ("Close");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 2);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (close_btn_cb), gg);

    gtk_object_set_data (GTK_OBJECT (gg->impute.window),
      "notebook", notebook);
    gtk_widget_show_all (gg->impute.window);
  }

  gdk_window_raise (gg->impute.window->window);
}
