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
#define SS_RANGE  2
#define SS_EVERYN 3
#define SS_STICKY 4
#define SS_ROWLAB 5

static void
selection_made_cb (GtkWidget *clist, gint row, gint column, GdkEventButton *event, ggobid *);

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
      gint kd = get_one_selection_from_clist (clist, d);  /* in utils_ui.c */
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
  GtkWidget *spinbtn, *entry;
  /*
   * If this is a different d than was used the last time
   * the subset panel was opened, attach the right adjustments
   * to the spin_buttons.
  */
  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.bstart_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.bstart_adj);
  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.bsize_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.bsize_adj);

  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.estart_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.estart_adj);
  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.estep_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.estep_adj);

  /*-- ... and set the values of the text entries, too --*/
  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:RANDOM_ENTRY");
  if (entry) {
    gchar *txt = g_strdup_printf ("%d", d->subset.random_n);
    gtk_entry_set_text (GTK_ENTRY (entry), txt);
    g_free (txt);
  }
  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:NROWS_ENTRY");
  if (entry) {
    gchar *txt = g_strdup_printf ("%d", d->nrows);
    gtk_entry_set_text (GTK_ENTRY (entry), txt);
    g_free (txt);
  }
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
    displays_tailpipe (FULL, gg);
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
  GtkWidget *entry;
  gchar *str;
  greal min, max;
  gboolean proceed = true;

  if (!d)
    return;

  subset_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (gg->subset_ui.notebook));

  switch (subset_type) {
    case SS_RANDOM:
      entry = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:RANDOM_ENTRY");
      sample_str = 
        gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
      d->subset.random_n = atoi (sample_str);
      redraw = subset_random (d->subset.random_n, d, gg);
    break;
    case SS_BLOCK:
      bstart = (gint) d->subset.bstart_adj->value;
      bsize = (gint) d->subset.bsize_adj->value;
      redraw = subset_block (bstart-1, bsize, d, gg);
    break;
    case SS_RANGE:

      entry = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:MIN_ENTRY");
      if (entry) {
        str = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
        min = atof (str);
      } else proceed = false;

      entry = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:MAX_ENTRY");
      if (entry) {
        str = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
        max = atof (str);
      } else proceed = false;
      if (proceed && d->subset.jvar >= 0)
        redraw = subset_range (min, max, d->subset.jvar, d, gg);
      else redraw = false;
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
      entry = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:ROWLAB");
      rowlab = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
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


static void 
subset_clist_datad_added_cb (ggobid *gg, datad *d, void *clist)
{
  gchar *row[1];
  GtkWidget *swin = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT (clist), "datad_swin");

  subset_init (d, gg);
  row[0] = g_strdup (d->name);
  gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
  g_free (row[0]);

  gtk_widget_show_all (swin);
}

CHECK_EVENT_SIGNATURE(subset_clist_datad_added_cb,datad_added_f)

/*------------------------------------------------------------------*/

void
subset_window_open (ggobid *gg, guint action, GtkWidget *w) {

  GtkWidget *button, *t;
  GtkWidget *vbox, *frame, *hb, *vb, *button_hbox, *close_hbox;
  GtkWidget *label, *btn, *spinbtn, *entry;
  GtkWidget *varnotebook;
  datad *d;
  gchar *clist_titles[1] = {"datasets"};

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

      clist = gtk_clist_new_with_titles (1, clist_titles);
      gtk_clist_set_selection_mode (GTK_CLIST (clist),
        GTK_SELECTION_SINGLE);
      gtk_object_set_data (GTK_OBJECT (clist), "datad_swin", swin);
      gtk_signal_connect (GTK_OBJECT (clist), "select_row",
        (GtkSignalFunc) subset_datad_set_cb, gg);
      gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
        (GtkSignalFunc) subset_clist_datad_added_cb, GTK_OBJECT (clist));
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
  
      /*-- entry: random sample size --*/
      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:RANDOM_ENTRY", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in the desired sample size", NULL);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

      gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("out of"),
        false, false, 2);

      /*-- entry: data size --*/
      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:NROWS_ENTRY", entry);
      gtk_entry_set_editable (GTK_ENTRY (entry), false);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

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

      spinbtn = gtk_spin_button_new (d->subset.bstart_adj, 0, 0);
      gtk_object_set_data (GTK_OBJECT(d->subset.bstart_adj), "WIDGET", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        spinbtn, "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- Block subsetting: blocksize (bsize) --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("Blocksize:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      spinbtn = gtk_spin_button_new (d->subset.bsize_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        spinbtn, "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb),
        spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);


      label = gtk_label_new ("Block");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
                                frame, label);

      /*---------------------------*/
      /*-- Points within a range --*/
      /*---------------------------*/
      frame = gtk_frame_new ("Variable range");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hb = gtk_hbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), hb);

      varnotebook = create_variable_notebook (hb,
        GTK_SELECTION_SINGLE, all_vartypes, all_datatypes,
        (GtkSignalFunc) selection_made_cb, gg);
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:RANGE_NOTEBOOK", varnotebook);

      t = gtk_table_new (2, 2, true);
      /*gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);*/
      gtk_container_set_border_width (GTK_CONTAINER (t), 5);
      gtk_box_pack_start (GTK_BOX (hb), t, false, false, 0);

      /*-- min label and entry --*/
      gtk_table_attach_defaults (GTK_TABLE (t),
        gtk_label_new ("Minimum"), 0,1,0,1);

      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:MIN_ENTRY", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in the minimum value for the selected variable (using the transformed data if applicable)",
        NULL);
      gtk_table_attach_defaults (GTK_TABLE (t), entry, 1,2, 0,1);

      /*-- max label and entry --*/
      gtk_table_attach_defaults (GTK_TABLE (t),
        gtk_label_new ("Maximum"), 0,1, 1,2);

      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:MAX_ENTRY", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in the maximum value for the selected variable (using the transformed data if applicable)",
        NULL);
      gtk_table_attach_defaults (GTK_TABLE (t), entry, 1,2, 1,2);


      label = gtk_label_new ("Range");
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

      spinbtn = gtk_spin_button_new (d->subset.estart_adj, 0, 0);
      gtk_object_set_data (GTK_OBJECT(d->subset.estart_adj), "WIDGET", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinbtn,
        "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- everyn subsetting: stepsize --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("N:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      spinbtn = gtk_spin_button_new (d->subset.estep_adj, 0, 0);
      gtk_object_set_data (GTK_OBJECT(d->subset.estep_adj), "WIDGET", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinbtn,
        "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
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
  
      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:ROWLAB", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in the label shared by the cases you want in the subset",
        NULL);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

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

      /*-- initialize display --*/
      subset_display_update (gg->d->data, gg);

      if (g_slist_length (gg->d) > 1)
        gtk_widget_show_all (swin);
      gtk_widget_show (vbox);
      gtk_widget_show_all (button_hbox);
      gtk_widget_show_all (close_hbox);
      gtk_widget_show_all (gg->subset_ui.notebook);
    }  /*-- if window == NULL --*/

    gtk_widget_show (gg->subset_ui.window);
    gdk_window_raise (gg->subset_ui.window->window);
  }
}

/*------------  range setting ---------------------------*/

static void
selection_made_cb (GtkWidget *clist, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  vartabled *vt = vartable_element_get (row, d);
  GtkWidget *entry;
  gchar *txt;

  d->subset.jvar = row;

  /*-- update the values in the min and max entries in the 'range' tab --*/
  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:MIN_ENTRY");
  if (entry && vt) {
    txt = g_strdup_printf ("%g", vt->lim_tform.min);
    gtk_entry_set_text (GTK_ENTRY (entry), txt);
    g_free (txt);
  }

  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:MAX_ENTRY");
  if (entry && vt) {
    txt = g_strdup_printf ("%g", vt->lim_tform.max);
    gtk_entry_set_text (GTK_ENTRY (entry), txt);
    g_free (txt);
  }
}
