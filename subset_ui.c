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

extern void subset_init (datad *d, ggobid *gg);

/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, ggobid *gg) {
  gtk_widget_hide (gg->subset_ui.window);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
  gtk_widget_hide (gg->subset_ui.window);
}

static datad *
datad_get_from_widget (GtkWidget *w, ggobid *gg)
{
  datad *d = NULL;

  if (g_slist_length (gg->d) == 0)
    ;
  else if (g_slist_length (gg->d) == 1) 
    d = gg->d->data;
  else {
    GtkWidget *clist = (GtkWidget *)
      gtk_object_get_data (GTK_OBJECT (w), "datad_clist");
    if (clist) {
      gint kd = get_one_selection_from_clist (clist);  /* in utils_ui.c */
      if (kd >= 0) d = (datad *) g_slist_nth_data (gg->d, kd);
    }
  }

  return d;
}

static void
set_adjustment (GtkWidget *w, GtkAdjustment *adj_new)
{
  GtkAdjustment *adj_current;
  GtkSpinButton *btn;
  if (w) {
    btn = GTK_SPIN_BUTTON (w);
    adj_current = gtk_spin_button_get_adjustment (btn);
    if ((gint)adj_current != (gint)adj_new) {
      gtk_object_ref (GTK_OBJECT(adj_current));
      gtk_spin_button_set_adjustment (btn, adj_new);
    }
  }
}

static void
subset_display_update (datad *d, ggobid *gg)
{
  /*
   * If this is a different d than was used the last time
   * the subset panel was opened, attach the right adjustments
   * to the spin_buttons.
  */
  set_adjustment (gg->subset_ui.bstart, d->subset.bstart_adj);
  set_adjustment (gg->subset_ui.bsize, d->subset.bsize_adj);
  set_adjustment (gg->subset_ui.estart, d->subset.estart_adj);
  set_adjustment (gg->subset_ui.estep, d->subset.estep_adj);

  /*-- ... and set the values of the text entries, too --*/
  if (gg->subset_ui.random_entry)
    gtk_entry_set_text (GTK_ENTRY (gg->subset_ui.random_entry),
      g_strdup_printf ("%d", d->subset.random_n));
  if (gg->subset_ui.nrows_entry)
    gtk_entry_set_text (GTK_ENTRY (gg->subset_ui.nrows_entry),
      g_strdup_printf ("%d", d->nrows));
  /*-- --*/
}


static void
subset_datad_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  datad *d = g_slist_nth_data (gg->d, row);
  if (d)
    subset_display_update (d, gg);
}

static void
rescale_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = datad_get_from_widget (w, gg);
  if (d) {
    limits_set (true, true, d, gg);
    vartable_limits_set (d);
    vartable_stats_set (d);

    tform_to_world (d, gg);
    displays_tailpipe (REDISPLAY_ALL, FULL, gg);
  }
}

static void
subset_cb (GtkWidget *w, ggobid *gg)
{
  gint subset_type;
  gchar *sample_str, *rowlab;
  gint bstart, bsize;
  gint estart, estep;
  gboolean redraw;
  datad *d = datad_get_from_widget (w, gg);

  if (!d)
    return;

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
  datad *d = datad_get_from_widget (w, gg);

  if (d != NULL) {
    subset_include_all (d, gg);
    subset_apply (d, gg);
  }
}

/*------------------------------------------------------------------*/

void
subset_window_open (ggobid *gg, guint action, GtkWidget *w) {

  GtkWidget *button, *t;
  GtkWidget *vbox, *frame, *hb, *vb, *button_hbox, *close_hbox;
  GtkWidget *label, *btn;
  datad *d;

  GtkWidget *swin, *clist;
  gchar *row[1];
  GSList *l;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
    return;

  else {

    d = gg->d->data;

    if (gg->subset_ui.window == NULL) {
    
      gg->subset_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (gg->subset_ui.window),
        "subset data");
      gtk_signal_connect (GTK_OBJECT (gg->subset_ui.window),
        "delete_event", GTK_SIGNAL_FUNC (close_wmgr_cb), NULL);
  
      gtk_container_set_border_width (GTK_CONTAINER (gg->subset_ui.window), 5);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->subset_ui.window), vbox);


      /* Create a scrolled window to pack the CList widget into */
      swin = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
        GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

      clist = gtk_clist_new (1);
      gtk_clist_set_selection_mode (GTK_CLIST (clist),
        GTK_SELECTION_SINGLE);
      gtk_signal_connect (GTK_OBJECT (clist), "select_row",
                         subset_datad_set_cb, gg);
      /*-- --*/

      for (l = gg->d; l; l = l->next) {
        d = (datad *) l->data;
        subset_init (d, gg);
        row[0] = g_strdup (d->name);
        gtk_clist_append (GTK_CLIST (clist), row);
        g_free (row[0]);
      }
      gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
      gtk_container_add (GTK_CONTAINER (swin), clist);
      gtk_box_pack_start (GTK_BOX (vbox), swin, false, false, 2);

      d = gg->d->data;
    
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
      button_hbox = gtk_hbox_new (true, 2);

      gtk_box_pack_start (GTK_BOX (vbox), button_hbox, false, false, 2);

      button = gtk_button_new_with_label ("Subset");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Draw a new subset and update all plots", NULL);
      gtk_object_set_data (GTK_OBJECT (button), "datad_clist", clist);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (subset_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);

      button = gtk_button_new_with_label ("Rescale");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Rescale the data after choosing a new subset", NULL);
      gtk_object_set_data (GTK_OBJECT (button), "datad_clist", clist);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (rescale_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);
    
      button = gtk_button_new_with_label ("Include all");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Stop subsetting: include all cases and update all plots", NULL);
      gtk_object_set_data (GTK_OBJECT (button), "datad_clist", clist);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (include_all_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);

      /*-- Separator --*/
      gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
        false, true, 2);

      /*-- Close button --*/
      close_hbox = gtk_hbox_new (false, 2);
      gtk_box_pack_start (GTK_BOX (vbox), close_hbox, false, false, 1);

      btn = gtk_button_new_with_label ("Close");
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                          GTK_SIGNAL_FUNC (close_btn_cb), (ggobid *) gg);
      gtk_box_pack_start (GTK_BOX (close_hbox), btn, true, false, 0);

    }  /*-- if window == NULL --*/

    subset_display_update (d, gg);

    if (g_slist_length (gg->d) > 1) 
      gtk_widget_show_all (swin);
    gtk_widget_show (vbox);
    gtk_widget_show_all (button_hbox);
    gtk_widget_show_all (close_hbox);
    gtk_widget_show_all (gg->subset_ui.notebook);
    gtk_widget_show (gg->subset_ui.window);
  }

  gdk_window_raise (gg->subset_ui.window->window);
}
