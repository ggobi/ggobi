/* subset_ui.c */
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

#define SS_RANDOM 0
#define SS_BLOCK  1
#define SS_EVERYN 2
#define SS_STICKY 3
#define SS_ROWLAB 4


static void
close_window_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->subset_ui.window);
}

static void
rescale_cb (GtkButton *button, ggobid *gg)
{
  datad *d = gg->current_display->d;
  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened subset_ui.window.
  */
  if (gg->subset_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  limits_set (true, true, d, gg);
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (REDISPLAY_ALL, FULL, gg);
}

static void
subset_cb (GtkWidget *w, ggobid *gg)
{
  gint subset_type;
  gchar *sample_str, *rowlab;
  gint bstart, bsize;
  gint estart, estep;
  gboolean redraw;
  datad *d = gg->current_display->d;

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened subset_ui.window.
  */
  if (gg->subset_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  subset_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (gg->subset_ui.notebook));

  switch (subset_type) {
    case SS_RANDOM:
      sample_str = 
        gtk_editable_get_chars (GTK_EDITABLE (gg->subset_ui.random_entry),
                                0, -1);
      d->subset.random_n = atoi (sample_str);
      redraw = subset_random (d->subset.random_n, d, gg);
      break;
    case SS_BLOCK:
      bstart = (gint) d->subset.bstart_adj->value;
      bsize = (gint) d->subset.bsize_adj->value;
      redraw = subset_block (bstart-1, bsize, d, gg);
      break;
    case SS_EVERYN:
      estart = (gint) d->subset.estart_adj->value;
      estep = (gint) d->subset.estep_adj->value;
      redraw = subset_everyn (estart-1, estep, d, gg);
      break;
    case SS_STICKY:
      redraw = subset_sticky (d, gg);
      break;
    case SS_ROWLAB:
      rowlab =
        gtk_editable_get_chars (GTK_EDITABLE (gg->subset_ui.rowlab_entry),
        0, -1);
      redraw = subset_rowlab (rowlab, d, gg);
      break;
  }

  if (redraw)
    subset_apply (d, gg);
}

static void
include_all_cb (GtkWidget *w, ggobid *gg) {
  datad *d = gg->current_display->d;
  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened subset_ui.window.
  */
  if (gg->subset_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  if (d != NULL) {
    subset_include_all (d, gg);
    subset_apply (d, gg);
  }
}

/*------------------------------------------------------------------*/
/*     adjusting the increments for block and everyn subsetting     */
/*------------------------------------------------------------------*/

/*-- Set the increment for block_start --*/
void set_start_incr_cb (GtkAdjustment *adj, GtkSpinButton *spin )
{
/*-- Set the spin widget's adjustment->step_increment to adj->value --*/
  GtkAdjustment *adj_spin = gtk_spin_button_get_adjustment (spin);

  adj_spin->step_increment = adj->value;
  gtk_spin_button_set_adjustment (spin, adj_spin);
}

/*-- Set the increment for block_size --*/
void set_block_incr_cb (GtkAdjustment *adj, GtkSpinButton *spin )
{
/*-- Set the spin widget's adjustment->step_increment to adj->value --*/
  GtkAdjustment *adj_spin = gtk_spin_button_get_adjustment (spin);

  adj_spin->step_increment = adj->value;
  gtk_spin_button_set_adjustment (spin, adj_spin);
}

/*------------------------------------------------------------------*/

void
subset_window_open (ggobid *gg, guint action, GtkWidget *w) {

  GtkWidget *button, *t;
  GtkWidget *vbox, *frame, *hb, *vb;
  GtkWidget *label, *btn;
  datad *d;
  gboolean firsttime = false;  /*-- first time for this d? --*/

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
    return;

  else {

    d = gg->current_display->d;
    gg->subset_ui.d = d;

    /*
     * if this particular datad object hasn't been the active one
     * during subsetting, initialize all its GtkAdjustments.
    */
    if (d->subset.bstart_adj == NULL) {
      gfloat fnr = (gfloat) d->nrows;
      firsttime = true;

      d->subset.bstart_adj = (GtkAdjustment *)
        gtk_adjustment_new (1.0, 1.0, (fnr-2.0), 1.0, 5.0, 0.0);
      d->subset.bsize_adj = (GtkAdjustment *)
        gtk_adjustment_new (fnr/10.0, 1.0, fnr, 1.0, 5.0, 0.0);

      d->subset.bstart_incr_adj = (GtkAdjustment *)
        gtk_adjustment_new (1.0, 1.0, fnr, 1.0, 5.0, 0.0);
      d->subset.bsize_incr_adj = (GtkAdjustment *)
        gtk_adjustment_new (1.0, 1.0, fnr, 1.0, 5.0, 0.0);

      d->subset.estart_adj = (GtkAdjustment *)
        gtk_adjustment_new (1.0, 1.0, fnr-2.0, 1.0, 5.0, 0.0);
      d->subset.estep_adj = (GtkAdjustment *)
        gtk_adjustment_new (fnr/10.0, 1.0, fnr-1, 1.0, 5.0, 0.0);
    }

    if (gg->subset_ui.window == NULL) {
    
      gg->subset_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (gg->subset_ui.window),
        "subset data");
      gtk_signal_connect (GTK_OBJECT (gg->subset_ui.window),
        "delete_event", GTK_SIGNAL_FUNC (close_window_cb), NULL);
  
      gtk_container_set_border_width (GTK_CONTAINER (gg->subset_ui.window), 5);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->subset_ui.window), vbox);
    
      /* Create a new notebook, place the position of the tabs */
      gg->subset_ui.notebook = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->subset_ui.notebook),
        GTK_POS_TOP);
      gtk_box_pack_start (GTK_BOX (vbox), gg->subset_ui.notebook,
        false, false, 2);
    
      /*-- Random sample without replacement --*/
      frame = gtk_frame_new ("Random sample without replacement");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hb = gtk_hbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), hb);

      gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Sample size"),
        false, false, 2);
  
      gg->subset_ui.random_entry = gtk_entry_new ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.random_entry,
        "Type in the desired sample size", NULL);
      gtk_box_pack_start (GTK_BOX (hb), gg->subset_ui.random_entry,
        true, true, 2);

      gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("out of"),
        false, false, 2);

      /*-- data size --*/
      gg->subset_ui.nrows_entry = gtk_entry_new ();
      gtk_entry_set_editable (GTK_ENTRY (gg->subset_ui.nrows_entry), false);
      gtk_box_pack_start (GTK_BOX (hb), gg->subset_ui.nrows_entry,
        true, true, 2);

      label = gtk_label_new ("Random");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
                                frame, label);
      
      /*-----------------------*/
      /*-- Consecutive block --*/
      /*-----------------------*/
      frame = gtk_frame_new ("Consecutive block");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      t = gtk_table_new (2, 2, true);
      gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
      gtk_container_set_border_width (GTK_CONTAINER (t), 5);
      gtk_container_add (GTK_CONTAINER (frame), t);

      /*-- Block subsetting: First case (bstart) --*/
      vb = gtk_vbox_new (false, 3);
      label = gtk_label_new ("First case:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gg->subset_ui.bstart = gtk_spin_button_new (d->subset.bstart_adj, 0, 0);

      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (gg->subset_ui.bstart), false);
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (gg->subset_ui.bstart),
                                       GTK_SHADOW_OUT);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.bstart,
        "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), gg->subset_ui.bstart, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- Block subsetting: blocksize (bsize) --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("Blocksize:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gg->subset_ui.bsize = gtk_spin_button_new (d->subset.bsize_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (gg->subset_ui.bsize), false);
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (gg->subset_ui.bsize),
                                       GTK_SHADOW_OUT);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.bsize,
        "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), gg->subset_ui.bsize, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

      /*-- Block subsetting: First case increment (bstart_incr) --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("Set the increment:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gg->subset_ui.bstart_incr =
        gtk_spin_button_new (d->subset.bstart_incr_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (gg->subset_ui.bstart_incr),
                                false);
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON
                                         (gg->subset_ui.bstart_incr),
                                       GTK_SHADOW_OUT);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.bstart_incr,
        "Specify the increment for the arrows used to increment and decrement the first case, just above",
        NULL);
      gtk_box_pack_start (GTK_BOX (vb), gg->subset_ui.bstart_incr,
                          false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,1,2);

      /*-- Block subsetting: Blocksize increment (bsize_incr) --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("Set the increment:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gg->subset_ui.bsize_incr =
        gtk_spin_button_new (d->subset.bsize_incr_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (gg->subset_ui.bsize_incr),
                                false);
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON
                                         (gg->subset_ui.bsize_incr),
                                       GTK_SHADOW_OUT);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.bsize_incr,
        "Specify the increment for the arrows used to increment and decrement the blocksize, just above",
        NULL);
      gtk_box_pack_start (GTK_BOX (vb), gg->subset_ui.bsize_incr,
                          false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,1,2);

      label = gtk_label_new ("Block");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
                                frame, label);

      /*--------------------*/
      /*-- Every nth case --*/
      /*--------------------*/
      frame = gtk_frame_new ("Every nth case");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      t = gtk_table_new (1, 2, true);
      gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
      gtk_container_set_border_width (GTK_CONTAINER (t), 5);
      gtk_container_add (GTK_CONTAINER (frame), t);

      /*-- everyn subsetting: start --*/
      vb = gtk_vbox_new (false, 3);
      label = gtk_label_new ("First case:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gg->subset_ui.estart = gtk_spin_button_new (d->subset.estart_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (gg->subset_ui.estart), false);
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (gg->subset_ui.estart),
                                       GTK_SHADOW_OUT);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.estart,
        "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), gg->subset_ui.estart, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- everyn subsetting: stepsize --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("N:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gg->subset_ui.estep = gtk_spin_button_new (d->subset.estep_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (gg->subset_ui.estep), false);
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (gg->subset_ui.estep),
                                       GTK_SHADOW_OUT);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->subset_ui.estep,
        "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), gg->subset_ui.estep, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

      label = gtk_label_new ("Every n");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*-------------------------------------------------------*/
      /*-- Cases whose row label is one of the sticky labels --*/
      /*-------------------------------------------------------*/
      frame = gtk_frame_new ("Cases whose row label is sticky");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
      gtk_widget_set_usize (frame, 100, 75);
      label = gtk_label_new ("Sticky");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*------------------------------------------------------*/
      /*-- Cases whose row label is the specified row label --*/
      /*------------------------------------------------------*/
      frame = gtk_frame_new ("Cases with specified row label");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hb = gtk_hbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), hb);

      label = gtk_label_new ("Row label:");
      gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
  
      gg->subset_ui.rowlab_entry = gtk_entry_new ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        gg->subset_ui.rowlab_entry,
        "Type in the label shared by the cases you want in the subset",
        NULL);
      gtk_box_pack_start (GTK_BOX (hb), gg->subset_ui.rowlab_entry,
                          true, true, 2);

      label = gtk_label_new ("Row label");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
                                frame, label);

      /*-- hbox to hold a few buttons --*/
      hb = gtk_hbox_new (true, 2);

      gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

      button = gtk_button_new_with_label ("Subset");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Draw a new subset and update all plots", NULL);
      gtk_signal_connect (GTK_OBJECT (button),
                          "clicked",
                          GTK_SIGNAL_FUNC (subset_cb),
                          (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);

      button = gtk_button_new_with_label ("Rescale");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Rescale the data after choosing a new subset", NULL);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (rescale_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);
    
      button = gtk_button_new_with_label ("Include all");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Stop subsetting: include all cases and update all plots", NULL);
      gtk_signal_connect (GTK_OBJECT (button),
                          "clicked",
                          GTK_SIGNAL_FUNC (include_all_cb),
                          (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (hb), button, true, true, 2);

      /*-- Close button --*/
      gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(), false, true, 2);
      hb = gtk_hbox_new (false, 2);
      gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

      btn = gtk_button_new_with_label ("Close");
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                          GTK_SIGNAL_FUNC (close_window_cb), (ggobid *) gg);
      gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 0);

    }  /*-- if window == NULL --*/

    /*
     * if this particular datad object is being initialized, 
     * add its callbacks.
    */
    if (firsttime) {
      gtk_signal_connect (GTK_OBJECT (d->subset.bstart_incr_adj),
                          "value_changed",
                          GTK_SIGNAL_FUNC (set_start_incr_cb),
                          (gpointer) gg->subset_ui.bstart);
      gtk_signal_connect (GTK_OBJECT (d->subset.bsize_incr_adj),
                          "value_changed",
                          GTK_SIGNAL_FUNC (set_block_incr_cb),
                          (gpointer) gg->subset_ui.bsize);
    }

    /*
     * In case this is a different d than was used the last time
     * the subset panel was opened, attach the right adjustments
     * to the spin_buttons.
    */
    gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (gg->subset_ui.bstart),
                                    d->subset.bstart_adj);
    gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (gg->subset_ui.bsize),
                                    d->subset.bsize_adj);
                                   
    gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (gg->subset_ui.bstart_incr),
                                    d->subset.bstart_incr_adj);
    gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (gg->subset_ui.bsize_incr),
                                    d->subset.bsize_incr_adj);

    gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (gg->subset_ui.estart),
                                    d->subset.estart_adj);
    gtk_spin_button_set_adjustment (GTK_SPIN_BUTTON (gg->subset_ui.estep),
                                    d->subset.estep_adj);

    /*-- ... and set the values of the text entries, too --*/
    gtk_entry_set_text (GTK_ENTRY (gg->subset_ui.random_entry),
      g_strdup_printf ("%d", d->subset.random_n));
    gtk_entry_set_text (GTK_ENTRY (gg->subset_ui.nrows_entry),
      g_strdup_printf ("%d", d->nrows));
    /*-- --*/

    gtk_widget_show_all (gg->subset_ui.window);
  }

  gdk_window_raise (gg->subset_ui.window->window);
}
