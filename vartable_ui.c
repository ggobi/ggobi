/* vartable_ui.c */ 
/* interface code for the variable statistics table */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define NCOLS_CLIST 9


/*
 * this just hides, but maybe it should destroy -- do we allow
 * people to add more datad's when the program is running?
*/
void delete_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
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
      row[2] = g_strdup_printf ("%d", 0);
      row[3] = g_strdup ("");
      row[4] = g_strdup_printf ("%8.3f", 0.0);
      row[5] = g_strdup_printf ("%8.3f", 0.0);
      row[6] = g_strdup_printf ("%8.3f", 0.0);
      row[7] = g_strdup_printf ("%8.3f", 0.0);
      row[8] = g_strdup_printf ("%d", 0);
    } else {
      row[0] = g_strdup_printf ("%d", j);
      row[1] = g_strdup (d->vartable[j].collab);
      row[2] = g_strdup_printf ("%d", d->vartable[j].groupid);
      row[3] = g_strdup ("");
      row[4] = g_strdup_printf ("%8.3f", d->vartable[j].lim_raw.min);
      row[5] = g_strdup_printf ("%8.3f", d->vartable[j].lim_raw.max);
      row[6] = g_strdup_printf ("%8.3f", d->vartable[j].mean);
      row[7] = g_strdup_printf ("%8.3f", d->vartable[j].median);
      row[8] = g_strdup_printf ("%d", d->vartable[j].nmissing);
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
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  gchar *titles[NCOLS_CLIST] =
    {"varno", "Variable",          /*-- varno will be an invisible column --*/
     "Group", "Transformation",
     "Minimum", "Maximum",
     "Mean", "Median",
     "N missing"};
  GSList *l;
  datad *d;
  gchar *label;
  GtkWidget *labelw;
  gint n;
  GtkWidget *hbox, *btn;

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
      false, false, 2);

    n = 0;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;

      /* Create a scrolled window to pack the CList widget into */
      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
        GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

      /*-- use datad->name once it's been defined --*/
      labelw = NULL;
      if (g_slist_length (gg->d) > 1) {
        label = datasetName(d, n++);
        labelw = gtk_label_new (label);
        g_free (label);
      }
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
                                scrolled_window, labelw);

      gtk_widget_show (scrolled_window);

      d->vartable_clist = gtk_clist_new_with_titles (NCOLS_CLIST, titles);
      gtk_clist_set_selection_mode (GTK_CLIST (d->vartable_clist),
        GTK_SELECTION_MULTIPLE);

      /*-- left justify all the numerical columns --*/
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        2, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        4, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        5, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        6, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        7, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist),
        8, GTK_JUSTIFY_RIGHT);

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
                        GTK_SIGNAL_FUNC (delete_cb), gg);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 1);
  }

  gtk_widget_show_all (gg->vartable_ui.window);
  gdk_window_raise (gg->vartable_ui.window->window);
}

void
vartable_tform_set (gint varno, datad *d, ggobid *gg) {

  if (d->vartable_clist != NULL)
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), varno,
      3, d->vartable[varno].collab_tform);
}
