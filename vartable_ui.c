/* vartable_ui.c */ 
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
/* interface code for the variable statistics table */

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

static void vartable_subwindow_init (datad *d, ggobid *gg);

static void close_wmgr_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}
static void close_btn_cb (GtkWidget *w, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}
static void destroyit (ggobid *gg)
{
  gtk_widget_destroy (gg->vartable_ui.window);
  gg->vartable_ui.window = NULL;
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

static void vartable_notebook_adddata_cb (GtkObject *obj, datad *d,
  ggobid *gg, GtkWidget *notebook)
{
  vartable_subwindow_init (d, gg);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook),
    g_slist_length (gg->d) > 1);
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

static GtkCList *
vartable_clist_get (ggobid *gg) {
  GtkNotebook *nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
  gint indx = gtk_notebook_get_current_page (nb);
  /*-- each notebook page's child is a scrolled window --*/
  GtkWidget *swin = gtk_notebook_get_nth_page (nb, indx);
  /*-- each scrolled window has one child, a clist --*/
  GList *swin_children = gtk_container_children (GTK_CONTAINER (swin));
  return ((GtkCList *) g_list_nth_data (swin_children, 0));
}

void
vartable_show_page (displayd *display, ggobid *gg)
{
  GtkNotebook *nb;
  gint page, page_new;
  datad *d = display->d;
  GList *l, *children;
  GtkWidget *child, *tab_label;

  if (display == NULL || gg == NULL || gg->vartable_ui.notebook == NULL)
    return;

  nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
  page = gtk_notebook_get_current_page (nb);

  if (page < 0)
    return;

  page_new = 0;
  children = gtk_container_children (GTK_CONTAINER (gg->vartable_ui.notebook));
  for (l = children; l; l = l->next) {
    child = l->data;
    tab_label = (GtkWidget *) gtk_notebook_get_tab_label (nb, child);
    if (tab_label && GTK_IS_LABEL (tab_label)) {
      if (strcmp (GTK_LABEL (tab_label)->label, d->name) == 0) {
        if (page != page_new) {
          gtk_notebook_set_page (nb, page_new);
          break;
        }
      }
    }
    page_new++;
  }
}
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
    g_free (val_str);
    max_val = (gfloat) atof (val_str);
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

      gtk_clist_set_text (clist, j, CLIST_USER_MIN,
        g_strdup_printf("%8.3f", min_val));
      gtk_clist_set_text (clist, j, CLIST_USER_MAX,
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
  gtk_widget_set_usize (umin,
#if GTK_MAJOR_VERSION == 2
    gdk_string_width (gtk_style_get_font (umin->style), 
#else
    gdk_string_width (umin->style->font,
#endif
      "0000000000"), -1);

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
  gtk_widget_set_usize (umax,
#if GTK_MAJOR_VERSION == 2
    gdk_string_width (gtk_style_get_font (umax->style), 
#else
    gdk_string_width (umax->style->font,
#endif
      "0000000000"), -1);

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
    gtk_clist_set_text (clist, j, CLIST_USER_MIN, g_strdup(""));
    gtk_clist_set_text (clist, j, CLIST_USER_MAX, g_strdup(""));
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
    GtkNotebook *nb;
    gint indx;
    GtkWidget *swin;
    GtkAdjustment *adj;
    newvar_add (vtype, vname, d, gg);

    /*-- scroll to the bottom to highlight the new variable --*/
    nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
    indx = gtk_notebook_get_current_page (nb);
    /*-- each notebook page's child is a scrolled window --*/
    swin = gtk_notebook_get_nth_page (nb, indx);
    adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (swin));
    adj->value += adj->page_increment;
    gtk_adjustment_value_changed (adj);

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


void
vartable_select_var (gint jvar, gboolean selected, datad *d, ggobid *gg)
{
  gint j, varno;
  gchar *varno_str;
  vartabled *vt;

  /*-- loop over the rows in the table, looking for jvar --*/
  for (j=0; j<d->ncols; j++) {
    if (d->vartable_clist != NULL) {
      gtk_clist_get_text (GTK_CLIST (d->vartable_clist), j, 0, &varno_str);
      varno = (gint) atoi (varno_str);
    } else varno = j;
    
    if (varno == jvar) {
      if (d->vartable_clist != NULL) {
        if (selected)
          gtk_clist_select_row (GTK_CLIST (d->vartable_clist), jvar, 1);
        else
          gtk_clist_unselect_row (GTK_CLIST (d->vartable_clist), jvar, 1);
      }
      vt = vartable_element_get (jvar, d);
      vt->selected = selected;
    }
  }
}

void
selection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  vartabled *vt;

  gtk_clist_get_text (GTK_CLIST (d->vartable_clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  vt = vartable_element_get (varno, d);
  vt->selected = true;

  return;
}

void
deselection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  vartabled *vt;

  gtk_clist_get_text (GTK_CLIST (d->vartable_clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  vt = vartable_element_get (varno, d);
  vt->selected = false;

  return;
}

gint
arithmetic_compare (GtkCList *cl, gconstpointer ptr1, gconstpointer ptr2) 
{
  const GtkCListRow *row1 = (const GtkCListRow *) ptr1;
  const GtkCListRow *row2 = (const GtkCListRow *) ptr2;
  gchar *text1 = NULL;
  gchar *text2 = NULL;
  gfloat f1, f2;

  text1 = GTK_CELL_TEXT (row1->cell[cl->sort_column])->text;
  text2 = GTK_CELL_TEXT (row2->cell[cl->sort_column])->text;

  f1 = atof (text1);
  f2 = atof (text2);

  return ((f1 < f2) ? -1 : (f1 > f2) ? 1 : 0);
}

void sortbycolumn_cb (GtkWidget *cl, gint column, ggobid *gg)
{
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);

  gtk_clist_set_sort_column (GTK_CLIST (d->vartable_clist), column);

/*
   If column is already sorted in forward order, it would be useful to
   sort it in reverse order, but how do I determine its sort order?
   I can either keep an integer vector and keep track of each column's
   sort order, or I can just reset the sort order for the whole clist.
   The lists and trees are so different in gtk 1.3 that it doesn't
   seem worthwhile to work on this now.
*/

  if (column >= 1 && column <= 3)  /*-- name, cat?, tform --*/
    gtk_clist_set_compare_func (GTK_CLIST (d->vartable_clist), NULL);
  else
    gtk_clist_set_compare_func (GTK_CLIST (d->vartable_clist),
                                (GtkCListCompareFunc) arithmetic_compare);
  gtk_clist_sort (GTK_CLIST (d->vartable_clist));

  return;
}

static void
vartable_row_assemble (gchar **row, datad *d, ggobid *gg)
{
  /*-- the new row will be appended --*/
  gint nrows = GTK_CLIST (d->vartable_clist)->rows;

  row[CLIST_VARNO] = g_strdup_printf ("%d", nrows);
  row[CLIST_VARNAME] = g_strdup ("");
  row[CLIST_TYPE] = g_strdup ("");
  row[CLIST_TFORM] = g_strdup ("");
  row[CLIST_USER_MIN] = g_strdup ("");
  row[CLIST_USER_MAX] = g_strdup ("");
  row[CLIST_DATA_MIN] = g_strdup_printf ("%8.3f", 0.0);
  row[CLIST_DATA_MAX] = g_strdup_printf ("%8.3f", 0.0);
  row[CLIST_MEAN] = g_strdup_printf ("%8.3f", 0.0);
  row[CLIST_MEDIAN] = g_strdup_printf ("%8.3f", 0.0);
  row[CLIST_NMISSING] = g_strdup_printf ("%d", 0);
}

void
vartable_row_append (datad *d, ggobid *gg)
{
  if (d->vartable_clist != NULL) {
    gint k;
    gchar **row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    vartable_row_assemble (row, d, gg);
    gtk_clist_freeze (GTK_CLIST (d->vartable_clist));
    gtk_clist_append ((GtkCList *) d->vartable_clist, row);
    gtk_clist_thaw (GTK_CLIST (d->vartable_clist));

    for (k=0; k<NCOLS_CLIST; k++)
      g_free ((gpointer) row[k]);
    g_free ((gpointer) row);
  }
}

static void
vartable_subwindow_init (datad *d, ggobid *gg)
{
  gint j, k;
  GtkWidget *scrolled_window;
  gchar *lbl;
  gchar *titles[NCOLS_CLIST] =
    {"varno",          /*-- varno will be an invisible column --*/
     "Variable",
     "Cat?",           /*-- categorical variable? --*/
     "Transform",
     "Min (user)", "Max (user)",
     "Min (data)", "Max (data)",
     "Mean", "Median",
     "N NAs"};

    /* Create a scrolled window to pack the CList widget into */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

    /*
     * We're showing all datasets for now, whether they have variables
     * or not.  That could change.
    */
    lbl = datasetName (d, gg);
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
                              scrolled_window, gtk_label_new (lbl));
    g_free (lbl);

    gtk_widget_show (scrolled_window);

    d->vartable_clist = gtk_clist_new_with_titles (NCOLS_CLIST, titles);
    gtk_clist_set_selection_mode (GTK_CLIST (d->vartable_clist),
      GTK_SELECTION_EXTENDED);

/*-- trying to add tooltips to the headers; it doesn't seem to work --*/
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gtk_clist_get_column_widget (
        GTK_CLIST (d->vartable_clist), CLIST_USER_MIN),
      "User specified minimum; untransformed", NULL);
/*---*/

    /*-- right justify all the numerical columns --*/
    for (k=0; k<NCOLS_CLIST; k++)
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        k, GTK_JUSTIFY_RIGHT);

    /*-- make the first column invisible --*/
    gtk_clist_set_column_visibility (GTK_CLIST (d->vartable_clist),
      CLIST_VARNO, false);

    /*-- set the column width automatically --*/
    for (k=0; k<NCOLS_CLIST; k++)
      gtk_clist_set_column_auto_resize (GTK_CLIST (d->vartable_clist),
                                        k, true);

    /*-- populate the table --*/
    for (j=0 ; j<d->ncols ; j++) {
      vartable_row_append (d, gg);
      vartable_cells_set_by_var (j, d);  /*-- then populate --*/
    }
    
    /*-- track selections --*/
    gtk_signal_connect (GTK_OBJECT (d->vartable_clist), "select_row",
                       GTK_SIGNAL_FUNC (selection_made),
                       gg);
    gtk_signal_connect (GTK_OBJECT (d->vartable_clist), "unselect_row",
                       GTK_SIGNAL_FUNC (deselection_made),
                       gg);

    /*-- re-sort when receiving a mouse click on a column header --*/
    gtk_signal_connect (GTK_OBJECT (d->vartable_clist), "click_column",
                       GTK_SIGNAL_FUNC (sortbycolumn_cb),
                       gg);

    /* It isn't necessary to shadow the border, but it looks nice :) */
    gtk_clist_set_shadow_type (GTK_CLIST (d->vartable_clist), GTK_SHADOW_OUT);

    gtk_container_add (GTK_CONTAINER (scrolled_window), d->vartable_clist);
    gtk_widget_show (d->vartable_clist);

  /*-- 3 = COLUMN_INSET --*/
  gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
    d->vartable_clist->requisition.width + 3 +
    GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
    150);
}

void
vartable_open (ggobid *gg)
{                                  
  GtkWidget *vbox, *hbox, *hb, *btn;
  GSList *l;
  datad *d;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
/**/return;

  /*-- if new datad's have been added, the user has to reopen the window --*/
  if (gg->vartable_ui.window != NULL) {
    destroyit (gg);
  }

  gg->vartable_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (gg->vartable_ui.window),
    "delete_event", GTK_SIGNAL_FUNC (close_wmgr_cb), gg);
  gtk_window_set_title (GTK_WINDOW (gg->vartable_ui.window),
    "Variable manipulation");

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER (gg->vartable_ui.window), vbox);
  gtk_widget_show (vbox);

  /* Create a notebook, set the position of the tabs */
  gg->vartable_ui.notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->vartable_ui.notebook),
    GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->vartable_ui.notebook),
    g_slist_length (gg->d) > 1);
  gtk_box_pack_start (GTK_BOX (vbox), gg->vartable_ui.notebook,
    true, true, 2);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    vartable_subwindow_init (d, gg);
  }

  /*-- listen for datad_added events on main_window --*/
  gtk_signal_connect (GTK_OBJECT (gg->main_window),
    "datad_added", GTK_SIGNAL_FUNC (vartable_notebook_adddata_cb),
     GTK_OBJECT (gg->vartable_ui.notebook));

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

  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 1);

  gtk_widget_show_all (gg->vartable_ui.window);

  /*-- set it to the page corresponding to the current display --*/
  vartable_show_page (gg->current_display, gg);
}

/*-------------------------------------------------------------------------*/
/*                 set values in the table                                 */
/*-------------------------------------------------------------------------*/

/*-- sets the name of the un-transformed variable --*/
void
vartable_collab_set_by_var (gint j, datad *d)
{
  gchar *ind;
  vartabled *vt;

  if (d->vartable_clist != NULL) {
    vt = vartable_element_get (j, d);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_VARNAME, vt->collab);
    ind = (vt->categorical_p) ? g_strdup_printf ("y:%d", vt->nlevels) :
                                g_strdup ("");
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_TYPE, ind);
    g_free (ind);
  }
}

/*-- sets the name of the transformed variable --*/
void
vartable_collab_tform_set_by_var (gint j, datad *d)
{
  vartabled *vt;

  if (d->vartable_clist != NULL) {
    vt = vartable_element_get (j, d);
    if (vt->tform0 == NO_TFORM0 &&
        vt->tform1 == NO_TFORM1 &&
        vt->tform2 == NO_TFORM2)
    {
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
        CLIST_TFORM, g_strdup(""));
    } else {
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
        CLIST_TFORM, vt->collab_tform);
    }
  }
}

/*-- sets the limits for a variable --*/
void
vartable_limits_set_by_var (gint j, datad *d)
{
  vartabled *vt;
  gchar *stmp;

  if (d->vartable_clist != NULL) {
    vt = vartable_element_get (j, d);

    stmp = g_strdup_printf ("%8.3f", vt->lim_display.min);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_DATA_MIN, stmp);
    g_free (stmp);

    stmp = g_strdup_printf ("%8.3f", vt->lim_display.max);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_DATA_MAX, stmp);
    g_free (stmp);

    if (vt->lim_specified_p) {
      stmp = g_strdup_printf ("%8.3f", vt->lim_specified.min);
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
        CLIST_USER_MIN, stmp);
      g_free (stmp);

      stmp = g_strdup_printf ("%8.3f", vt->lim_specified.max);
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
        CLIST_USER_MAX, stmp);
      g_free (stmp);
    }
  }
}
void
vartable_limits_set (datad *d) 
{
  gint j;
  if (d->vartable_clist != NULL)
    for (j=0; j<d->ncols; j++)
    vartable_limits_set_by_var (j, d);
}

/*-- sets the mean, median for a variable --*/
void
vartable_stats_set_by_var (gint j, datad *d) {
  vartabled *vt;
  gchar *stmp;

  if (d->vartable_clist != NULL) {
    vt = vartable_element_get (j, d);

    stmp = g_strdup_printf ("%8.3f", vt->mean);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_MEAN, stmp);
    g_free (stmp);

    /*-- for categorical variables, don't display the median --*/
    stmp = (vt->categorical_p) ?
      g_strdup("") : g_strdup_printf ("%8.3f", vt->median);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_MEDIAN, stmp);
    g_free (stmp);

    stmp = g_strdup_printf ("%d", vt->nmissing);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_NMISSING, stmp);
    g_free (stmp);
  }
}
void
vartable_stats_set (datad *d) {
  gint j;

  if (d->vartable_clist != NULL)
    for (j=0; j<d->ncols; j++)
      vartable_stats_set_by_var (j, d);
}

/*
 * in one routine, populate every cell in a row -- all these
 * functions call gtk_clist_set_text.
*/
void
vartable_cells_set_by_var (gint j, datad *d) 
{
  vartable_stats_set_by_var (j, d);
  vartable_limits_set_by_var (j, d);
  vartable_collab_set_by_var (j, d);
  vartable_collab_tform_set_by_var (j, d);
}
