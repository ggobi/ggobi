/* vartable_ui.c */ 

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
dialog_range_cancel (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  gtk_widget_destroy (dialog);
}

static void
limits_type_cb (GtkToggleButton *w, ggobid *gg) 
{
  gg->lims_use_visible = w->active;
}

static void
dialog_range_set (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  GtkWidget *umin_entry, *umax_entry;
  GtkCList *clist = vartable_clist_get (gg);
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
      j = cols[k];
      vt = vartable_element_get (j, d);

      vt->lim_specified.min = vt->lim_specified_tform.min = min_val;
      vt->lim_specified.max = vt->lim_specified_tform.max = max_val;

      gtk_clist_set_text (clist, j, REAL_CLIST_USER_MIN,
        g_strdup_printf("%8.3f", min_val));
      gtk_clist_set_text (clist, j, REAL_CLIST_USER_MAX,
        g_strdup_printf("%8.3f", max_val));

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
  gtk_widget_destroy (dialog);
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
  GtkWidget *frame, *vb, *hb, *btn, *okay_btn, *cancel_btn;
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
/**/return;

  dialog = gtk_dialog_new ();

  /*-- frame for a pair of radio buttons --*/
  frame = gtk_frame_new ("Define rescaling behavior");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), frame);

  vb = gtk_vbox_new (true, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  radio1 = gtk_radio_button_new_with_label (NULL, "Use visible points");
  GTK_TOGGLE_BUTTON (radio1)->active = TRUE;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "When rescaling, use only the cases that are visible: ie, not hidden by brushing and not excluded by subsampling",
    NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (limits_type_cb), gg);
  gtk_box_pack_start (GTK_BOX (vb), radio1, false, false, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_label (group, "Use all points");
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
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Minimum: "),
    true, true, 0);

  umin = gtk_entry_new ();
#if GTK_MAJOR_VERSION == 2
  gtk_widget_set_usize (umin,
    gdk_string_width (gtk_style_get_font (umin->style), 
      "0000000000"), -1);
#else
  gtk_widget_set_usize (umin,
    gdk_string_width (umin->style->font,
      "0000000000"), -1);
#endif

  gtk_widget_set_name (umin, "umin_entry");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), umin,
    "Minimum for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (hb), umin, true, true, 2);

  gtk_container_add (GTK_CONTAINER (vb), hb);

  /*-- make another hbox --*/
  hb = gtk_hbox_new (true, 5);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Maximum: "),
    true, true, 2);

  umax = gtk_entry_new ();
#if GTK_MAJOR_VERSION == 2
  gtk_widget_set_usize (umax,
    gdk_string_width (gtk_style_get_font (umax->style), 
      "0000000000"), -1);
#else
  gtk_widget_set_usize (umax,
    gdk_string_width (umax->style->font,
      "0000000000"), -1);
#endif

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

  btn = gtk_button_new_with_label ("Clear user limits");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Unset user min and max for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (vb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (range_unset_cb), gg);
  /*-- --*/

  /*-- ok button --*/
  okay_btn = gtk_button_new_with_label ("Okay");
  gtk_signal_connect (GTK_OBJECT (okay_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_range_set), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    okay_btn);

  /*-- cancel button --*/
  cancel_btn = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (cancel_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_range_cancel), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    cancel_btn);

  gtk_widget_show_all (dialog);
}

void range_unset (ggobid *gg)
{
  GtkCList *clist = vartable_clist_get (gg);
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);
  gint j, k;
  vartabled *vt;

  for (k=0; k<ncols; k++) {
    j = cols[k];
    vt = vartable_element_get (j, d);
    vt->lim_specified_p = false;
    /*-- then null out the two entries in the table --*/
    gtk_clist_set_text (clist, j, REAL_CLIST_USER_MIN, g_strdup(""));
    gtk_clist_set_text (clist, j, REAL_CLIST_USER_MAX, g_strdup(""));
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
dialog_newvar_cancel (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  gtk_widget_destroy (dialog);
}

static void
dialog_newvar_add (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
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

    /*-- destroy the dialog widget --*/
    gtk_widget_destroy (dialog);
  }
}

static void
open_newvar_dialog (GtkWidget *w, ggobid *gg)
{
  GtkWidget *dialog;
  GtkWidget *frame, *vb, *hb, *okay_btn, *cancel_btn;
  GtkWidget *radio1, *radio2, *entry;
  GSList *radio_group;

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), "Add new variable");
  frame = gtk_frame_new ("Variable values");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), frame,
    false, false, 2);

  /*-- make a vb to hold the radio buttons --*/
  vb = gtk_vbox_new (false, 2);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  radio1 = gtk_radio_button_new_with_label (NULL, "1:n");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "Add a variable whose values run from 1 to the number of cases",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio1, false, false, 2);

  radio_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_label (radio_group, "Brushed groups");
  gtk_widget_set_name (radio2, "radio_brush");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "Add a variable whose values are based on the groups defined by brushing",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio2, false, false, 2);

  /*-- label and entry --*/
  hb = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Variable name: "),
    true, true, 2);
  entry = gtk_entry_new();
  gtk_entry_set_text (GTK_ENTRY (entry), "foo");
  gtk_widget_set_name (entry, "newvar_entry");

  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hb,
    false, false, 2);

  /*-- ok button --*/
  okay_btn = gtk_button_new_with_label ("Okay");
  gtk_signal_connect (GTK_OBJECT (okay_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_newvar_add), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    okay_btn);

  /*-- cancel button --*/
  cancel_btn = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (cancel_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_newvar_cancel), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    cancel_btn);

  gtk_widget_show_all (dialog);
}


/*-------------------------------------------------------------------------*/
/*                         Rename one variable                             */
/*-------------------------------------------------------------------------*/

static void
dialog_rename_var (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
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
  GtkWidget *dialog, *hb, *entry;
  GtkWidget *okay_btn, *cancel_btn;
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
/**/return;
  }

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), "Rename one variable");

  /*-- label and entry --*/
  hb = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Variable name: "),
    true, true, 2);
  entry = gtk_entry_new();

  /*-- label it with the name of the variable being renamed --*/
  vt = vartable_element_get (selected_vars[0], d);
  gtk_entry_set_text (GTK_ENTRY (entry), vt->collab);
  gtk_widget_set_name (entry, "rename_entry");

  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hb,
    false, false, 2);

  /*-- ok button --*/
  okay_btn = gtk_button_new_with_label ("Okay");
  gtk_signal_connect (GTK_OBJECT (okay_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_rename_var), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    okay_btn);

  /*-- cancel button --*/
  cancel_btn = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (cancel_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_newvar_cancel), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    cancel_btn);

  gtk_widget_show_all (dialog);

}

/*-------------------------------------------------------------------------*/

void select_all_cb (GtkWidget *w, ggobid *gg)
{
  GtkCList *clist = vartable_clist_get (gg);
  gtk_clist_select_all (clist);
}
void deselect_all_cb (GtkWidget *w, ggobid *gg)
{
  GtkCList *clist = vartable_clist_get (gg);
  gtk_clist_unselect_all (clist);
}

GtkWidget *
vartable_buttonbox_build (ggobid *gg) {
  GtkWidget *hbox, *hb, *btn;

  /*-- hbox for the buttons along the bottom --*/
  hbox = gtk_hbox_new (false, 12);

  /*-- Make and clear selections --*/
  hb = gtk_hbox_new (false, 2);

  btn = gtk_button_new_with_label ("Select all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Select all variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (select_all_cb), gg);

  btn = gtk_button_new_with_label ("Clear selection");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Deselect all variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (deselect_all_cb), gg);

  gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);
  /*-- --*/

  /*-- Set and apply limits --*/
  hb = gtk_hbox_new (false, 2);

  /*-- set and clear variable ranges --*/
  btn = gtk_button_new_with_label ("Limits ... ");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Set user min and max for the selected variable(s), and define rescaling behavior", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_range_set_dialog), gg);

  /*-- rescale after resetting variable ranges --*/
  btn = gtk_button_new_with_label ("Rescale");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Rescale plots using specified limits and scaling behavior", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (rescale_cb), gg);

  gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);
  /*--  --*/

  /*-- Clone, new, delete ... --*/
  hb = gtk_hbox_new (false, 2);
  /*-- Clone or delete selected variables --*/

  btn = gtk_button_new_with_label ("Clone");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Clone selected variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (clone_vars_cb), gg);

  /*-- New variable: index, derived from brushing, ... --*/
  btn = gtk_button_new_with_label ("New ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Add a new variable", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_newvar_dialog), gg);
  /*-- --*/

/*
 * not yet implemented
  btn = gtk_button_new_with_label ("Delete");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Delete selected variables", NULL);
  gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (delete_vars_cb), gg);
  gtk_widget_set_sensitive (btn, false);
*/

  gtk_box_pack_start (GTK_BOX (hbox), hb, true, false, 1);
  /*-- --*/

  /*-- Rename one variable ... --*/
  btn = gtk_button_new_with_label ("Rename ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Rename one variable -- one variable must be selected", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_rename_dialog), gg);
  /*-- --*/

  btn = gtk_button_new_with_label ("Close");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Close the window", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, false, 1);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (close_btn_cb), gg);

  return hbox;
}
