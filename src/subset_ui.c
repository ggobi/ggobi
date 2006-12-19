/* subset_ui.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "ggobi-stage-subset.h"

typedef enum { 
  GGOBI_SUBSET_RANDOM, GGOBI_SUBSET_BLOCK, GGOBI_SUBSET_RANGE,
  GGOBI_SUBSET_EVERYN, GGOBI_SUBSET_STICKY, GGOBI_SUBSET_ROWLAB 
} GGobiSubsetType;

static void
subset_ui_add_data (GtkTreeModel *model, GGobiStage *d)
{
  GtkTreeIter iter;
  GGobiStage *s = ggobi_stage_find(d, GGOBI_MAIN_STAGE_SUBSET);
  gfloat fnr = (gfloat) s->n_rows;  
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, 1, 0, 2, d->n_rows,
    3, gtk_adjustment_new (1.0, 1.0, (fnr-2.0), 1.0, 5.0, 0.0),
    4, gtk_adjustment_new (fnr/10.0, 1.0, fnr, 1.0, 5.0, 0.0),
    5, gtk_adjustment_new (1.0, 1.0, fnr-2.0, 1.0, 5.0, 0.0),
    6, gtk_adjustment_new (fnr/10.0, 1.0, fnr-1, 1.0, 5.0, 0.0),
    7, s, -1);
}


/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, GtkWidget *win) {
  gtk_widget_hide (win);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, GtkWidget *win) {
  gtk_widget_hide (win);
}

static GGobiStageSubset *
subset_ui_get_selected_stage(GtkTreeSelection *tree_sel)
{
  GGobiStageSubset *s = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  if (gtk_tree_selection_get_selected(tree_sel, &model, &iter))
    gtk_tree_model_get(model, &iter, 7, &s, -1);
  return s;
}

static const gchar *const substr_lbl[] = {
 "Is identical to the string",
 "Includes the string",
 "Begins with the string",
 "Ends with the string",
 "Does not include the string",
};
static void subset_ui_string_pos_cb (GtkWidget *w, GtkTreeSelection *tree_sel)
{
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  
  if (gtk_tree_selection_get_selected(tree_sel, &model, &iter))
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, 
      gtk_combo_box_get_active(GTK_COMBO_BOX(w)), -1);
}

static void
set_adjustment(GtkTreeModel *model, GtkTreeIter *iter, gint col, const gchar* wid_key)
{
  GtkAdjustment *adj;
  GtkWidget *spinbtn = g_object_get_data(G_OBJECT(model), wid_key);
  /* skip the name and two integers */
  gtk_tree_model_get(model, iter, col + 3, &adj, -1);
  gtk_spin_button_set_adjustment(GTK_SPIN_BUTTON(spinbtn), adj);
}

static void
subset_ui_display_update (GtkTreeModel *model, GtkTreeIter *iter)
{
  GtkWidget *entry;
  gint n;
  GGobiStageSubset *stage;
  gchar *txt;
  
  /* set/restore adjustments for the dataset at the given row */
  set_adjustment (model, iter, 0, "ggobi-subset-bstart");
  set_adjustment (model, iter, 1, "ggobi-subset-bsize");
  set_adjustment (model, iter, 2, "ggobi-subset-estart");
  set_adjustment (model, iter, 3, "ggobi-subset-estep");
  
  /*-- ... and set the values of the text entries, too --*/
  entry = g_object_get_data(G_OBJECT(model), "ggobi-subset-random");
  gtk_tree_model_get(model, iter, 2, &n, -1);
  txt = g_strdup_printf ("%d", n);
  gtk_entry_set_text (GTK_ENTRY (entry), txt);
  g_free (txt);
  
  entry = g_object_get_data(G_OBJECT(model), "ggobi-subset-nrows");
  gtk_tree_model_get(model, iter, 7, &stage, -1);
  txt = g_strdup_printf ("%d", GGOBI_STAGE(stage)->n_rows);
  gtk_entry_set_text (GTK_ENTRY (entry), txt);
  g_free (txt);
}


static void
subset_ui_datad_set_cb (GtkTreeSelection *tree_sel, ggobid *gg)
{
  /*-- Assume that all datad's are included --*/
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  
  if (gtk_tree_selection_get_selected(tree_sel, &model, &iter))
      subset_ui_display_update (model, &iter);
}

static void
rescale_cb (GtkWidget *w, GtkTreeSelection *tree_sel)
{
  GGobiStageSubset *subset = subset_ui_get_selected_stage(tree_sel);
  GGobiStage *d = GGOBI_STAGE(subset);
  if (d) {
    // FIXME: everything is rescaled by default with the new pipeline, if
    // we want to support subsetting without rescaling, we'll need a special
    // option that links the scales to the limits at a stage before "subset"
    /*limits_set (d, d->gg->lims_use_visible);
    vartable_limits_set (d);
    vartable_stats_set (d);

    tform_to_world(d);
    displays_tailpipe (FULL, d->gg);*/
  }
}

static void
subset_cb (GtkWidget *w, GtkTreeSelection *tree_sel)
{
  gint subset_type;
  gchar *sample_str, *substr;
  GtkAdjustment *bstart, *bsize;
  GtkAdjustment *estart, *estep;
  gint string_pos;
  gboolean redraw = false;
  GtkTreeModel *model;
  GGobiStageSubset *d = subset_ui_get_selected_stage(tree_sel);
  GtkWidget *entry, *tgl;
  GtkTreeIter iter;

  if (!d)
    return;

  gtk_tree_selection_get_selected(tree_sel, &model, &iter);
  
  subset_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (g_object_get_data(G_OBJECT(model), 
      "ggobi-subset-notebook")));

  switch (subset_type) {
    case GGOBI_SUBSET_RANDOM:
      entry = g_object_get_data(G_OBJECT(model), "ggobi-subset-random");
      sample_str = 
        gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 2, atoi (sample_str), -1);
      redraw = ggobi_stage_subset_random (d, atoi (sample_str));
    break;
    case GGOBI_SUBSET_BLOCK:
      gtk_tree_model_get(model, &iter, 3, &bstart, 4, &bsize, -1);
      redraw = ggobi_stage_subset_block (d, gtk_adjustment_get_value(bstart)-1, 
        gtk_adjustment_get_value(bsize));
    break;
    case GGOBI_SUBSET_RANGE:
      redraw = ggobi_stage_subset_range (d);
    break;
    case GGOBI_SUBSET_EVERYN:
      gtk_tree_model_get(model, &iter, 5, &estart, 6, &estep, -1);
      redraw = ggobi_stage_subset_everyn (d, gtk_adjustment_get_value(estart)-1, 
        gtk_adjustment_get_value(estep));
    break;
    case GGOBI_SUBSET_STICKY:
      redraw = ggobi_stage_subset_sticky (d);
    break;
    case GGOBI_SUBSET_ROWLAB:
      /* use a toggle widget to specify whether to ignore case or not */
      entry = (GtkWidget *)
        g_object_get_data(G_OBJECT(model), "ggobi-subset-rowlab");
      tgl = (GtkWidget *)
        g_object_get_data(G_OBJECT(model), "ggobi-subset-casefold");
      substr = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
      gtk_tree_model_get(model, &iter, 1, &string_pos, -1);
      redraw = ggobi_stage_subset_rowlab (d, substr, string_pos,
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(tgl)));
    break;
  }

  if (redraw)
    ggobi_stage_subset_apply (d);
}

static void
include_all_cb (GtkWidget *w, GtkTreeSelection *tree_sel) {
  GGobiStageSubset *d = subset_ui_get_selected_stage(tree_sel);

  if (d != NULL) {
    ggobi_stage_filter_set_included_all (GGOBI_STAGE_FILTER(d), true);
    ggobi_stage_subset_apply (d);
  }
}


static void 
subset_ui_tree_view_datad_added_cb (ggobid *gg, GGobiStage *d, GtkTreeModel *model)
{
  GtkWidget *swin = (GtkWidget *)
    g_object_get_data(G_OBJECT (model), "ggobi-subset-swin");
  subset_ui_add_data(model, d);
  gtk_widget_show_all (swin);
}

CHECK_EVENT_SIGNATURE(subset_ui_tree_view_datad_added_cb,datad_added_f)

/*------------------------------------------------------------------*/

void
subset_window_open (ggobid *gg) {

  GtkWidget *button, *t, *nbook;
  GtkWidget *vbox, *frame, *hb, *vb, *button_hbox, *close_hbox;
  GtkWidget *label, *btn, *spinbtn, *entry, *opt;
  GGobiStageSubset *d;
  GtkTreeSelection *tree_sel;
  static gchar *tree_view_titles[1] = {"datasets"};
  GtkAdjustment *adj;
  GtkWidget *swin, *tree_view;
  GtkListStore *model;
  GtkTreeIter first;
  GSList *l;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
    return;

  else {

    d = gg->d->data;

    if (gg->subset_ui.window == NULL) {
    
      gg->subset_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (gg->subset_ui.window),
        "Subset Data");
      g_signal_connect (G_OBJECT (gg->subset_ui.window),
        "delete_event", G_CALLBACK (close_wmgr_cb), gg->subset_ui.window);
  
      gtk_container_set_border_width (GTK_CONTAINER (gg->subset_ui.window), 5);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->subset_ui.window), vbox);


      /* Create a scrolled window to pack the list widget into */
      swin = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

      model = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, 
        GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT, 
        GTK_TYPE_ADJUSTMENT, GGOBI_TYPE_STAGE_SUBSET);
      tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
      tree_sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
      
      g_object_set_data(G_OBJECT (model), "ggobi-subset-swin", swin);
      g_signal_connect (G_OBJECT (gg), "datad_added",
        G_CALLBACK(subset_ui_tree_view_datad_added_cb), model);
      /*-- --*/

      /*-- All datad's are included. This assumption is used in two places. */
      for (l = gg->d; l; l = l->next) 
        subset_ui_add_data(GTK_TREE_MODEL(model), GGOBI_STAGE(l->data));
      
      gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &first, NULL, 0);
      gtk_tree_selection_select_iter(tree_sel, &first);
      
      gtk_container_add (GTK_CONTAINER (swin), tree_view);
      gtk_box_pack_start (GTK_BOX (vbox), swin, true, true, 2);
    
      /* Create a new notebook, place the position of the tabs */
      nbook = gtk_notebook_new ();
      g_object_set_data(G_OBJECT(model), "ggobi-subset-notebook", nbook);
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (nbook), GTK_POS_TOP);
      gtk_box_pack_start (GTK_BOX (vbox), nbook, false, false, 2);
    
      /*-- Random sample without replacement --*/
      frame = gtk_frame_new ("Random sample without replacement");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hb = gtk_hbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), hb);

	    label = gtk_label_new_with_mnemonic ("Sample si_ze");
      gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);
  
      /*-- entry: random sample size --*/
      entry = gtk_entry_new ();
	    gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);
      g_object_set_data(G_OBJECT(gg->subset_ui.window), 
        "ggobi-subset-random", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in the desired sample size", NULL);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

	    label = gtk_label_new_with_mnemonic ("_out of");
      gtk_box_pack_start (GTK_BOX (hb), label,
        false, false, 2);

      /*-- entry: data size --*/
      entry = gtk_entry_new ();
	    gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);
      g_object_set_data(G_OBJECT(gg->subset_ui.window),
        "ggobi-subset-nrows", entry);
      gtk_editable_set_editable (GTK_EDITABLE (entry), false);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

      label = gtk_label_new_with_mnemonic ("R_andom");
      gtk_notebook_append_page (GTK_NOTEBOOK (nbook), frame, label);
      
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
      label = gtk_label_new_with_mnemonic ("_First case:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gtk_tree_model_get(GTK_TREE_MODEL(model), &first, 3, &adj, -1);
      spinbtn = gtk_spin_button_new (adj, 0, 0);
  	  gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinbtn);
      g_object_set_data(G_OBJECT(model), "ggobi-subset-bstart", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        spinbtn, "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- Block subsetting: blocksize (bsize) --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new_with_mnemonic ("Blocksi_ze:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gtk_tree_model_get(GTK_TREE_MODEL(model), &first, 4, &adj, -1);
      spinbtn = gtk_spin_button_new (adj, 0, 0);
	    gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinbtn);
      g_object_set_data(G_OBJECT(model), "ggobi-subset-bsize", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        spinbtn, "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb),
        spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);


      label = gtk_label_new_with_mnemonic ("_Block");
      gtk_notebook_append_page (GTK_NOTEBOOK (nbook), frame, label);

      /*---------------------------*/
      /*-- Points within a range --*/
      /*---------------------------*/
      frame = gtk_frame_new ("Variable limits");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      vb = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), vb);

      gtk_box_pack_start (GTK_BOX (vb),
        gtk_label_new ("Exclude data outside the user limits\nin the variable manipulation table"),
        false, false, 0);

      label = gtk_label_new_with_mnemonic ("_Limits");
      gtk_notebook_append_page (GTK_NOTEBOOK (nbook), frame, label);

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
      label = gtk_label_new_with_mnemonic ("_First case:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gtk_tree_model_get(GTK_TREE_MODEL(model), &first, 5, &adj, -1);
      spinbtn = gtk_spin_button_new (adj, 0, 0);
	    gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinbtn);
      g_object_set_data(G_OBJECT(model), "ggobi-subset-estart", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinbtn,
        "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- everyn subsetting: stepsize --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new_with_mnemonic ("_N:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      gtk_tree_model_get(GTK_TREE_MODEL(model), &first, 6, &adj, -1);
      spinbtn = gtk_spin_button_new (adj, 0, 0);
      gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinbtn);
      g_object_set_data(G_OBJECT(model), "ggobi-subset-estep", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinbtn,
        "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

      label = gtk_label_new_with_mnemonic ("_Every n");
      gtk_notebook_append_page (GTK_NOTEBOOK (nbook), frame, label);

      /*-------------------------------------------------------*/
      /*-- Cases whose row label is one of the sticky labels --*/
      /*-------------------------------------------------------*/
      frame = gtk_frame_new ("Cases whose row label is sticky");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
      //gtk_widget_set_usize (frame, 100, 75);

      vb = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), vb);

      gtk_box_pack_start (GTK_BOX (vb),
        gtk_label_new ("Include only those cases with a sticky label"),
        false, false, 0);

      label = gtk_label_new_with_mnemonic ("S_ticky");
      gtk_notebook_append_page (GTK_NOTEBOOK (nbook), frame, label);

      /*---------------------------------------------------------*/
      /*-- Cases whose row label includes the specified string --*/
      /*---------------------------------------------------------*/
      frame = gtk_frame_new ("Cases with specified row label");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      vb = gtk_vbox_new (false, 5);
      gtk_container_add (GTK_CONTAINER (frame), vb);

      hb = gtk_hbox_new (false, 5);
      gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 5);

      label = gtk_label_new_with_mnemonic ("S_ubstring:");
      gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);

      entry = gtk_entry_new ();
      gtk_label_set_mnemonic_widget(GTK_LABEL(label), entry);
      g_object_set_data(G_OBJECT(gg->subset_ui.window),
        "ggobi-filter-rowlab", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in a string to specify the cases you want in the subset",
        NULL);
      gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);

      hb = gtk_hbox_new (false, 5);
      gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 5);

      opt = gtk_combo_box_new_text ();
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "Specify the position in the row labels to check for the substring",
        NULL);
      gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);
      populate_combo_box (opt, (gchar**) substr_lbl, G_N_ELEMENTS(substr_lbl),
        G_CALLBACK(subset_ui_string_pos_cb), tree_sel);

      btn = gtk_check_button_new_with_mnemonic ("_Ignore case");
      g_object_set_data(G_OBJECT(gg->subset_ui.window),
        "ggobi-subset-casefold", btn);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(btn), true);
      gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 0);

      label = gtk_label_new_with_mnemonic ("R_ow label");
      gtk_notebook_append_page (GTK_NOTEBOOK (nbook),
        frame, label);

      /*-- hbox to hold a few buttons --*/
      button_hbox = gtk_hbox_new (true, 2);

      gtk_box_pack_start (GTK_BOX (vbox), button_hbox, false, false, 2);

      button = gtk_button_new_with_mnemonic ("_Subset");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Draw a new subset and update all plots", NULL);
      g_object_set_data(G_OBJECT (button), "datad_tree_view", tree_view);
      g_signal_connect (G_OBJECT (button), "clicked",
                          G_CALLBACK (subset_cb), tree_sel);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);

      button = gtk_button_new_with_mnemonic ("_Rescale");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Rescale the data after choosing a new subset", NULL);
      g_object_set_data(G_OBJECT (button), "datad_tree_view", tree_view);
      g_signal_connect (G_OBJECT (button), "clicked",
                          G_CALLBACK (rescale_cb), tree_sel);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);
    
      button = gtk_button_new_with_mnemonic ("Include _all");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Stop subsetting: include all cases and update all plots", NULL);
      g_object_set_data(G_OBJECT (button), "datad_tree_view", tree_view);
      g_signal_connect (G_OBJECT (button), "clicked",
                          G_CALLBACK (include_all_cb), tree_sel);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);

      /*-- Separator --*/
      gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
        false, true, 2);

      /*-- Close button --*/
      close_hbox = gtk_hbox_new (false, 2);
      gtk_box_pack_start (GTK_BOX (vbox), close_hbox, false, false, 1);

      btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
      g_signal_connect (G_OBJECT (btn), "clicked",
                          G_CALLBACK (close_btn_cb), gg->subset_ui.window);
      gtk_box_pack_start (GTK_BOX (close_hbox), btn, true, false, 0);

      /*-- initialize display --*/
      populate_tree_view(tree_view, tree_view_titles, G_N_ELEMENTS(tree_view_titles), 
        true, GTK_SELECTION_BROWSE, G_CALLBACK(subset_ui_datad_set_cb), gg);
      //subset_ui_display_update (GTK_TREE_MODEL(model), &first);

      if (g_slist_length (gg->d) > 1)
        gtk_widget_show_all (swin);
      gtk_widget_show (vbox);
      gtk_widget_show_all (button_hbox);
      gtk_widget_show_all (close_hbox);
      gtk_widget_show_all (nbook);
    }  /*-- if window == NULL --*/

    gtk_widget_show (gg->subset_ui.window);
    gdk_window_raise (gg->subset_ui.window->window);
  }
}
