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

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define NCOLS_CLIST 10

#define CLIST_VARNO     0
#define CLIST_VARNAME   1
#define CLIST_TFORM     2
#define CLIST_USER_MIN  3
#define CLIST_USER_MAX  4
#define CLIST_DATA_MIN  5
#define CLIST_DATA_MAX  6
#define CLIST_MEAN      7
#define CLIST_MEDIAN    8
#define CLIST_NMISSING  9

static void delete_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}
static void hide_cb (GtkWidget *w, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}

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

static void
dialog_range_set (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  GtkCList *clist = vartable_clist_get (gg);
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);
  gint j, k;
  gchar *val_str;
  gfloat val;

  /*-- minimum --*/
  val_str = gtk_entry_get_text (GTK_ENTRY (gg->vartable_ui.umin));
  if (strlen (val_str) > 0) {
    val = (gfloat) atof (val_str);
    for (k=0; k<ncols; k++) {
      j = cols[k];

      d->vartable[j].lim_specified_p = true;
      d->vartable[j].lim_specified.min = val;

      gtk_clist_set_text (clist, j, CLIST_USER_MIN,
        g_strdup_printf("%8.3f", val));
    }
  }

  /*-- maximum --*/
  val_str = gtk_entry_get_text (GTK_ENTRY (gg->vartable_ui.umax));
  if (strlen (val_str) > 0) {
    val = (gfloat) atof (val_str);
    for (k=0; k<ncols; k++) {
      j = cols[k];

      d->vartable[j].lim_specified_p = true;
      d->vartable[j].lim_specified.max = val;

      gtk_clist_set_text (clist, j, CLIST_USER_MAX,
        g_strdup_printf("%8.3f", val));
    }
  }

  g_free (cols);
  gtk_widget_destroy (dialog);
}

/*
 * open a dialog with two text entry widgets in it,
 * and fetch the range for the selected variables in
 * dialog_range_set.
*/
void
range_set_cb (GtkWidget *w, ggobid *gg)
{
  GtkWidget *frame, *vb, *hb, *okay_button;

  GtkWidget *dialog = gtk_dialog_new ();

  frame = gtk_frame_new ("Set range");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), frame);

  vb = gtk_vbox_new (true, 5);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  /*-- make an hbox to hold a label and a text entry widget --*/
  hb = gtk_hbox_new (true, 5);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Minimum: "),
    true, true, 2);

  gg->vartable_ui.umin = gtk_entry_new ();
  gtk_widget_set_usize (gg->vartable_ui.umin,
                        gdk_string_width (gg->vartable_ui.umin->style->font,
                        "0000000000"), -1);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->vartable_ui.umin,
    "Range minimum for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (hb), gg->vartable_ui.umin,
    true, true, 2);

  gtk_container_add (GTK_CONTAINER (vb), hb);

  /*-- make another hbox --*/
  hb = gtk_hbox_new (true, 5);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Maximum: "),
    true, true, 2);

  gg->vartable_ui.umax = gtk_entry_new ();
  gtk_widget_set_usize (gg->vartable_ui.umax,
                        gdk_string_width (gg->vartable_ui.umax->style->font,
                        "0000000000"), -1);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->vartable_ui.umax,
    "Range minimum for the selected variable(s)", NULL);
  gtk_box_pack_start (GTK_BOX (hb), gg->vartable_ui.umax,
    true, true, 2);

  gtk_container_add (GTK_CONTAINER (vb), hb);

  /*-- buttons --*/
  okay_button = gtk_button_new_with_label ("Okay");
  gtk_signal_connect (GTK_OBJECT (okay_button), "clicked",
    GTK_SIGNAL_FUNC (dialog_range_set), gg);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    okay_button);

  gtk_widget_show_all (dialog);
}

void range_unset_cb (GtkWidget *w, ggobid *gg)
{
  GtkCList *clist = vartable_clist_get (gg);
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
  gint ncols = selected_cols_get (cols, d, gg);
  gint j, k;

  for (k=0; k<ncols; k++) {
    j = cols[k];
    d->vartable[j].lim_specified_p = false;
    /*-- then null out the two entries in the table --*/
    gtk_clist_set_text (clist, j, CLIST_USER_MIN, g_strdup(""));
    gtk_clist_set_text (clist, j, CLIST_USER_MAX, g_strdup(""));
  }
  g_free ((gchar *) cols);
}

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
      d->vartable[jvar].selected = selected;
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

  gtk_clist_get_text (GTK_CLIST (d->vartable_clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  d->vartable[varno].selected = true;

  return;
}

void
deselection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);

  gtk_clist_get_text (GTK_CLIST (d->vartable_clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  d->vartable[varno].selected = false;

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
  if (column == 1)  /*-- variable name --*/
    gtk_clist_set_compare_func (GTK_CLIST (d->vartable_clist), NULL);
  else
    gtk_clist_set_compare_func (GTK_CLIST (d->vartable_clist),
                                (GtkCListCompareFunc) arithmetic_compare);
  gtk_clist_sort (GTK_CLIST (d->vartable_clist));

  return;
}

void
vartable_row_append (gint j, datad *d, ggobid *gg)
{
  if (d->vartable_clist != NULL) {
    gint k;
    gchar **row;
    row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    if (j == -1) {
      row[0] = g_strdup_printf ("%d", 0);
      row[1] = g_strdup ("");
      row[2] = g_strdup ("");
      row[3] = g_strdup_printf ("%8.3f", 0.0);
      row[4] = g_strdup_printf ("%8.3f", 0.0);
      row[5] = g_strdup_printf ("%8.3f", 0.0);
      row[6] = g_strdup_printf ("%8.3f", 0.0);
      row[7] = g_strdup_printf ("%8.3f", 0.0);
      row[8] = g_strdup_printf ("%8.3f", 0.0);
      row[9] = g_strdup_printf ("%d", 0);
    } else {
      row[0] = g_strdup_printf ("%d", j);
      row[1] = g_strdup (d->vartable[j].collab);
      row[2] = g_strdup ("");
      if (d->vartable[j].lim_specified_p) {
        row[3] = g_strdup_printf ("%8.3f", d->vartable[j].lim_specified.min);
        row[4] = g_strdup_printf ("%8.3f", d->vartable[j].lim_specified.max);
      } else {
        row[3] = g_strdup ("");
        row[4] = g_strdup ("");
      }
      row[5] = g_strdup_printf ("%8.3f", d->vartable[j].lim_raw.min);
      row[6] = g_strdup_printf ("%8.3f", d->vartable[j].lim_raw.max);
      row[7] = g_strdup_printf ("%8.3f", d->vartable[j].mean);
      row[8] = g_strdup_printf ("%8.3f", d->vartable[j].median);
      row[9] = g_strdup_printf ("%d", d->vartable[j].nmissing);
    }

    gtk_clist_append ((GtkCList *) d->vartable_clist, row);

    for (k=0; k<NCOLS_CLIST; k++)
      g_free ((gpointer) row[k]);
    g_free ((gpointer) row);
  }
}

void
vartable_open (ggobid *gg)
{                                  
  gint j, k;
  GtkWidget *vbox, *hbox, *btn;
  GtkWidget *scrolled_window;
  gchar *titles[NCOLS_CLIST] =
    {"varno", "Variable",          /*-- varno will be an invisible column --*/
     "Transformation",
     "Min (user)", "Max (user)",
     "Min (data)", "Max (data)",
     "Mean", "Median",
     "N missing"};
  GSList *l;
  datad *d;
  GtkWidget *labelw;
  gint n;

  if (gg->vartable_ui.window == NULL) {

    gg->vartable_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (gg->vartable_ui.window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), gg);
    gtk_window_set_title (GTK_WINDOW (gg->vartable_ui.window),
      "Variable selection and statistics");

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

    n = 0;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;

      /* Create a scrolled window to pack the CList widget into */
      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
        GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

      labelw = (g_slist_length (gg->d) > 1) ? gtk_label_new (d->name) : NULL;
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
                                scrolled_window, labelw);

      gtk_widget_show (scrolled_window);

      d->vartable_clist = gtk_clist_new_with_titles (NCOLS_CLIST, titles);
      gtk_clist_set_selection_mode (GTK_CLIST (d->vartable_clist),
        GTK_SELECTION_MULTIPLE);

      /*-- right justify all the numerical columns --*/
      for (k=0; k<NCOLS_CLIST; k++)
        gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
          k, GTK_JUSTIFY_RIGHT);

      /*-- make the first column invisible --*/
      gtk_clist_set_column_visibility (GTK_CLIST (d->vartable_clist), 0, false);

      /*-- set the column width automatically --*/
      for (k=0; k<NCOLS_CLIST; k++)
        gtk_clist_set_column_auto_resize (GTK_CLIST (d->vartable_clist),
                                          k, true);

      /*-- populate the table --*/
      for (j=0 ; j<d->ncols ; j++)
        vartable_row_append (j, d, gg);

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
    }

    /*-- 3 = COLUMN_INSET --*/
    gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
      d->vartable_clist->requisition.width + 3 +
      GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
      150);

    hbox = gtk_hbox_new (true, 10);

    btn = gtk_button_new_with_label ("Set range ... ");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Set user min and max for the selected variable(s)", NULL);
    gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 1);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (range_set_cb), gg);


    btn = gtk_button_new_with_label ("Unset range");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Unset user min and max for the selected variable(s)", NULL);
    gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 1);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (range_unset_cb), gg);

    btn = gtk_button_new_with_label ("Select all");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Select all variables", NULL);
    gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 1);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (select_all_cb), gg);

    btn = gtk_button_new_with_label ("Clear selection");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Deselect all variables", NULL);
    gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 1);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (deselect_all_cb), gg);

    btn = gtk_button_new_with_label ("Close");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the window", NULL);
    gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 1);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (hide_cb), gg);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 1);
  }

  gtk_widget_show_all (gg->vartable_ui.window);
  gdk_window_raise (gg->vartable_ui.window->window);
}

void
vartable_tform_set (gint varno, datad *d, ggobid *gg) {

  if (d->vartable_clist != NULL)
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), varno,
      2, d->vartable[varno].collab_tform);
}
