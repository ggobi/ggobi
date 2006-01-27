/* vartable_ui.c */ 
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

/* interface code for the variable statistics table: dialogs and buttons */

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

static void close_btn_cb (GtkWidget *w, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}

static void
clone_vars_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);

  if (ncols > 0)
    clone_vars (cols, ncols, d, gg);

  g_free (cols);
}


/* not implemented
static void
delete_vars_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);

  if (ncols > 0)
    delete_vars (cols, ncols, d, gg);

  g_free (cols);
}
*/

/*-------------------------------------------------------------------------*/
/*--------------- Setting and clearing variable ranges --------------------*/
/*-------------------------------------------------------------------------*/

static void
limits_type_cb (GtkToggleButton *w, ggobid *gg) 
{
  gg->lims_use_visible = w->active;
}

static void
dialog_range_set (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = w;
  GtkWidget *umin_entry, *umax_entry;
  //GtkWidget *tree_view = vartable_tree_view_get (gg);
  GtkTreeModel *model;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);
  gint j, k;
  gchar *val_str;
  gfloat min_val, max_val;
  gboolean min_p = false, max_p = false;
  vartabled *vt;

  umin_entry = widget_find_by_name (GTK_DIALOG(dialog)->vbox, "umin_entry");
  if (umin_entry == NULL || !GTK_IS_ENTRY(umin_entry)) {
    g_printerr ("found the wrong widget; bail out\n");
    return;
  }
  umax_entry = widget_find_by_name (GTK_DIALOG(dialog)->vbox, "umax_entry");
  if (umax_entry == NULL || !GTK_IS_ENTRY(umax_entry)) {
    g_printerr ("found the wrong widget; bail out\n");
    return;
  }

  /*-- minimum --*/
  val_str = gtk_editable_get_chars (GTK_EDITABLE (umin_entry),
    0, -1);
  if (val_str != NULL && strlen (val_str) > 0) {
    min_val = (gfloat) atof (val_str);
    g_free (val_str);
    min_p = true;
  }

  /*-- maximum --*/
  val_str = gtk_editable_get_chars (GTK_EDITABLE (umax_entry),
    0, -1);
  if (val_str != NULL && strlen (val_str) > 0) {
    max_val = (gfloat) atof (val_str);
    g_free (val_str);
    max_p = true;
  }

  /*-- require setting both, and make sure the values are consistent --*/
  if (!min_p || !max_p || (min_p && max_p && max_val<min_val)) {
    range_unset (gg);
  } else {

    for (k=0; k<ncols; k++) {
	  GtkTreeIter iter;
	  
      j = cols[k];
	  vt = vartable_element_get (j, d);

	  vartable_iter_from_varno(j, d, &model, &iter);
      
      vt->lim_specified.min = vt->lim_specified_tform.min = min_val;
      vt->lim_specified.max = vt->lim_specified_tform.max = max_val;

	  gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
	  	VT_REAL_USER_MIN, min_val, VT_REAL_USER_MAX, max_val, -1);
      
      vt->lim_specified_p = min_p && max_p;
    }

    /*
     * the first function could be needed if transformation has been
     * going on, because lim_tform could be out of step.
    */
    limits_set (false, false, d, gg);  
    vartable_limits_set (d);
    vartable_stats_set (d);

    tform_to_world (d, gg);
    displays_tailpipe (FULL, gg);
  }

  g_free (cols);
}

static void
range_unset_cb (GtkWidget *w, ggobid *gg)
{
  range_unset (gg);
}

static void rescale_cb (GtkWidget *w, ggobid *gg) {
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);

  limits_set (true, true, d, gg);  
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);
}


/*
 * open a dialog with two text entry widgets in it,
 * and fetch the range for the selected variables in
 * dialog_range_set.
*/
static void
open_range_set_dialog (GtkWidget *w, ggobid *gg)
{
  GtkWidget *frame, *vb, *hb, *btn, *lbl;
  GtkWidget *dialog, *umin, *umax;
  GtkWidget *radio1, *radio2;
  GSList *group;
  gint k;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);
  gboolean ok = true;
  vartabled *vt;

  for (k=0; k<ncols; k++) {
    vt = vartable_element_get (cols[k], d);
    if (vt->tform0 != NO_TFORM0 ||
        vt->tform1 != NO_TFORM1 ||
        vt->tform2 != NO_TFORM2)
    {
      ok = false;
      quick_message ("Sorry, can't set the range for a transformed variable\n",
        false);
      break;
    }
  }
  g_free (cols);
  if (!ok)
	  return;

  dialog = gtk_dialog_new_with_buttons ("Range Dialog", NULL, 0, 
  			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

  /*-- frame for a pair of radio buttons --*/
  frame = gtk_frame_new ("Define rescaling behavior");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), frame);

  vb = gtk_vbox_new (true, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  radio1 = gtk_radio_button_new_with_mnemonic (NULL, "Use _visible points");
  GTK_TOGGLE_BUTTON (radio1)->active = TRUE;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "When rescaling, use only the cases that are visible: ie, not hidden by brushing and not excluded by subsampling",
    NULL);
  g_signal_connect (G_OBJECT (radio1), "toggled",
                      G_CALLBACK (limits_type_cb), gg);
  gtk_box_pack_start (GTK_BOX (vb), radio1, false, false, 0);

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_mnemonic (group, "Use _all points");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "When rescaling, use all cases",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio2, TRUE, TRUE, 0);
  /*-- --*/


  /*-- frame for setting the user-specified limits --*/
  frame = gtk_frame_new ("Override default limits");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), frame);

  vb = gtk_vbox_new (true, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  /*-- make an hbox to hold a label and a text entry widget --*/
  hb = gtk_hbox_new (true, 5);
  lbl = gtk_label_new_with_mnemonic ("M_inimum: ");
  gtk_box_pack_start (GTK_BOX (hb), lbl, true, true, 0);

  umin = gtk_entry_new ();
  gtk_entry_set_width_chars(GTK_ENTRY(umin), 10);
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), umin);
  
  gtk_widget_set_name (umin, "umin_entry");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), umin,
    "Minimum for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (hb), umin, true, true, 2);

  gtk_container_add (GTK_CONTAINER (vb), hb);

  /*-- make another hbox --*/
  hb = gtk_hbox_new (true, 5);
  lbl = gtk_label_new_with_mnemonic ("M_aximum: ");
  gtk_box_pack_start (GTK_BOX (hb), lbl,
    true, true, 2);

  umax = gtk_entry_new ();
  gtk_entry_set_width_chars(GTK_ENTRY(umin), 10);
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), umax);
  
  gtk_widget_set_name (umax, "umax_entry");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), umax,
    "Maximum for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (hb), umax, true, true, 2);

  gtk_container_add (GTK_CONTAINER (vb), hb);
  /*-- --*/

  /*-- frame for the unset range button --*/
  frame = gtk_frame_new ("Restore default limits");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), frame);
  vb = gtk_vbox_new (true, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  btn = gtk_button_new_with_mnemonic ("_Clear user limits");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Unset user min and max for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (vb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (range_unset_cb), gg);
  /*-- --*/

  gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	  dialog_range_set(dialog, gg);

  gtk_widget_destroy(dialog);
}

void range_unset (ggobid *gg)
{
  GtkTreeModel *model;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);
  gint j, k;
  vartabled *vt;

  for (k=0; k<ncols; k++) {
	  GtkTreeIter iter;
	  
    j = cols[k];
    vt = vartable_element_get (j, d);
	
	vartable_iter_from_varno(j, d, &model, &iter);
	  
    vt->lim_specified_p = false;
    /*-- then null out the two entries in the table --*/
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
	  	VT_REAL_USER_MIN, 0.0, VT_REAL_USER_MAX, 0.0, -1);
  }
  g_free ((gchar *) cols);


  /*-- these 4 lines the same as in dialog_range_set --*/
  limits_set (false, false, d, gg);  
  vartable_limits_set (d);
  vartable_stats_set (d);

  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);
}


/*-------------------------------------------------------------------------*/
/*------- Adding derived variables (other than cloning, for now) ----------*/
/*-------------------------------------------------------------------------*/

static void
dialog_newvar_add (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = w;
  GtkWidget *entry, *radio_brush;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint vtype;
  gchar *vname;

  /*-- retrieve the radio button for the brushing groups --*/
  radio_brush = widget_find_by_name (GTK_DIALOG(dialog)->vbox, "radio_brush");
  if (radio_brush == NULL || !GTK_IS_RADIO_BUTTON(radio_brush)) {
    g_printerr ("found the wrong widget; bail out\n");
    return;
  }
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio_brush)))
    vtype = ADDVAR_BGROUP;
  else
    vtype = ADDVAR_ROWNOS;

  /*-- retrieve the entry widget and variable name --*/
  entry = widget_find_by_name (GTK_DIALOG(dialog)->vbox, "newvar_entry");
  if (entry == NULL || !GTK_IS_ENTRY(entry)) {
    g_printerr ("found the wrong widget; bail out\n");
/**/return;
  }
  vname = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
  if (vname != NULL && strlen(vname) > 0) {
     newvar_add_with_values(vtype == ADDVAR_BGROUP ? 
       (gdouble *) &AddVarBrushGroup : (gdouble *) &AddVarRowNumbers,
       d->nrows, vname, real,
       0, NULL, NULL, NULL, d, gg); 

/* I think we still want to do this ... */
#ifdef FORMERLY
    /*-- scroll to the bottom to highlight the new variable --*/
    nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
    indx = gtk_notebook_get_current_page (nb);
    /*-- each notebook page's child is a scrolled window --*/
    swin = gtk_notebook_get_nth_page (nb, indx);
    adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (swin));
    adj->value += adj->page_increment;
    gtk_adjustment_value_changed (adj);
#endif

  }
}

static void
open_newvar_dialog (GtkWidget *w, ggobid *gg)
{
  GtkWidget *dialog;
  GtkWidget *frame, *vb, *hb, *lbl;
  GtkWidget *radio1, *radio2, *entry;
  GSList *radio_group;

  dialog = gtk_dialog_new_with_buttons ("Add New Variable", NULL, 0, GTK_STOCK_OK,
  				GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
  frame = gtk_frame_new ("Variable values");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame,
    false, false, 2);

  /*-- make a vb to hold the radio buttons --*/
  vb = gtk_vbox_new (false, 2);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  radio1 = gtk_radio_button_new_with_mnemonic (NULL, "1:_n");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "Add a variable whose values run from 1 to the number of cases",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio1, false, false, 2);

  radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_mnemonic (radio_group, "Brushed _groups");
  gtk_widget_set_name (radio2, "radio_brush");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "Add a variable whose values are based on the groups defined by brushing",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio2, false, false, 2);

  /*-- label and entry --*/
  hb = gtk_hbox_new (false, 2);
  lbl = gtk_label_new_with_mnemonic ("Variable _name: ");
  gtk_box_pack_start (GTK_BOX (hb), lbl,
    true, true, 2);
  entry = gtk_entry_new();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry);
  gtk_entry_set_text (GTK_ENTRY (entry), "foo");
  gtk_widget_set_name (entry, "newvar_entry");

  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hb,
    false, false, 2);

	gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		dialog_newvar_add(dialog, gg);
	
  gtk_widget_destroy(dialog);
}


/*-------------------------------------------------------------------------*/
/*                         Rename one variable                             */
/*-------------------------------------------------------------------------*/

static void
dialog_rename_var (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = w;
  GtkWidget *entry;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gchar *vname;
  gint *selected_vars, nselected_vars = 0;
  gint jvar;
  vartabled *vt;

  /*-- find out what variables are selected in the var statistics panel --*/
  selected_vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_vars = selected_cols_get (selected_vars, d, gg);
  if (nselected_vars == 0)
    return;

  /*-- retrieve the entry widget and variable name --*/
  entry = widget_find_by_name (GTK_DIALOG(dialog)->vbox, "rename_entry");
  if (entry == NULL || !GTK_IS_ENTRY(entry)) {
    g_printerr ("found the wrong widget; bail out\n");
    return;
  }

  jvar = selected_vars[0];
  vname = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
  if (vname != NULL && strlen(vname) > 1) {
    vt = vartable_element_get (jvar, d);
    vt->collab = g_strdup (vname);
    vt->nickname = g_strndup (vname, 2);

    vartable_collab_set_by_var (jvar, d);
    tform_label_update (jvar, d, gg);
  }
}

static void
open_rename_dialog (GtkWidget *w, ggobid *gg)
{
  GtkWidget *dialog, *hb, *entry, *lbl;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *selected_vars, nselected_vars = 0;
  vartabled *vt;

  /*-- find out what variables are selected in the var statistics panel --*/
  selected_vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_vars = selected_cols_get (selected_vars, d, gg);

  if (nselected_vars == 0) {
    gchar *message = g_strdup_printf ("You must select one variable.\n");
    quick_message (message, false);
    g_free (message);
	return;
  }

  dialog = gtk_dialog_new_with_buttons ("Rename One Variable", NULL, 0, 
  			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
 
  /*-- label and entry --*/
  hb = gtk_hbox_new (false, 2);
  lbl = gtk_label_new ("Variable _name: ");
  gtk_box_pack_start (GTK_BOX (hb), lbl,
    true, true, 2);
  entry = gtk_entry_new();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), entry);
  /*-- label it with the name of the variable being renamed --*/
  vt = vartable_element_get (selected_vars[0], d);
  gtk_entry_set_text (GTK_ENTRY (entry), vt->collab);
  gtk_widget_set_name (entry, "rename_entry");

  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hb,
    false, false, 2);
	
  gtk_widget_show_all(GTK_DIALOG(dialog)->vbox);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	  dialog_rename_var(dialog, gg);

  gtk_widget_destroy(dialog);
}

/*-------------------------------------------------------------------------*/

void select_all_cb (GtkWidget *w, ggobid *gg)
{
  GtkWidget *tree_view = vartable_tree_view_get (gg);
  GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_select_all(tree_sel);
}
void deselect_all_cb (GtkWidget *w, ggobid *gg)
{
  GtkWidget *tree_view = vartable_tree_view_get (gg);
  GtkTreeSelection *tree_sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_unselect_all(tree_sel);
}

GtkWidget *
vartable_buttonbox_build (ggobid *gg) {
  GtkWidget *hbox, *hb, *btn;

  /*-- hbox for the buttons along the bottom --*/
  hbox = gtk_hbox_new (false, 12);

  /*-- Make and clear selections --*/
  hb = gtk_hbox_new (false, 2);

  btn = gtk_button_new_with_mnemonic ("_Select all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Select all variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (select_all_cb), gg);

  btn = gtk_button_new_with_mnemonic ("Clear s_election");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Deselect all variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (deselect_all_cb), gg);

  gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);
  /*-- --*/

  /*-- Set and apply limits --*/
  hb = gtk_hbox_new (false, 2);

  /*-- set and clear variable ranges --*/
  btn = gtk_button_new_with_mnemonic ("_Limits ... ");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Set user min and max for the selected variable(s), and define rescaling behavior", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (open_range_set_dialog), gg);

  /*-- rescale after resetting variable ranges --*/
  btn = gtk_button_new_with_mnemonic ("Resc_ale");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Rescale plots using specified limits and scaling behavior", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (rescale_cb), gg);

  gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);
  /*--  --*/

  /*-- Clone, new, delete ... --*/
  hb = gtk_hbox_new (false, 2);
  /*-- Clone or delete selected variables --*/

  btn = gtk_button_new_with_mnemonic ("Cl_one");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Clone selected variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (clone_vars_cb), gg);

  /*-- New variable: index, derived from brushing, ... --*/
  btn = gtk_button_new_with_mnemonic ("_New");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Add a new variable", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (open_newvar_dialog), gg);
  /*-- --*/

/*
 * not yet implemented
  btn = gtk_button_new_with_label ("Delete");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Delete selected variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (delete_vars_cb), gg);
  gtk_widget_set_sensitive (btn, false);
*/

  gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);
  /*-- --*/

  /*-- Rename one variable ... --*/
  btn = gtk_button_new_with_mnemonic ("Rena_me ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Rename one variable -- one variable must be selected", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (open_rename_dialog), gg);
  /*-- --*/

  btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Close the window", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, false, 1);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (close_btn_cb), gg);

  return hbox;
}
