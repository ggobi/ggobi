/* vartable_ui.c */ 
/* interface code for the variable statistics table */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define NCOLS_CLIST 9


void delete_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->vardata_window);
}

void
vartable_select_var (gint jvar, gboolean selected, datad *d, ggobid *gg)
{
  gint j, varno;
  gchar *varno_str;

  /*-- loop over the rows in the table, looking for jvar --*/
  for (j=0; j<d->ncols; j++) {
    if (d->vardata_clist != NULL) {
      gtk_clist_get_text (GTK_CLIST (d->vardata_clist), j, 0, &varno_str);
      varno = (gint) atoi (varno_str);
    } else varno = j;
    
    if (varno == jvar) {
      if (d->vardata_clist != NULL) {
        if (selected)
          gtk_clist_select_row (GTK_CLIST (d->vardata_clist), jvar, 1);
        else
          gtk_clist_unselect_row (GTK_CLIST (d->vardata_clist), jvar, 1);
      }
      d->vardata[jvar].selected = selected;
    }
  }
}

void
vartable_unselect_all (ggobid *gg) 
{
  gint j;
  GSList *l;
  datad *d;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;

    if (d->vardata_clist != NULL)
      gtk_clist_unselect_all (GTK_CLIST (d->vardata_clist));

    for (j=0; j<d->ncols; j++)
      d->vardata[j].selected = false;
  }
}

void
selection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;
  datad *d = gg->current_display->d;

  gtk_clist_get_text (GTK_CLIST (d->vardata_clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  d->vardata[varno].selected = true;

  return;
}

void
deselection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;
  datad *d = gg->current_display->d;

  gtk_clist_get_text (GTK_CLIST (d->vardata_clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  d->vardata[varno].selected = false;

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
  datad *d = gg->current_display->d;

  gtk_clist_set_sort_column (GTK_CLIST (d->vardata_clist), column);
  if (column == 1)  /*-- variable name --*/
    gtk_clist_set_compare_func (GTK_CLIST (d->vardata_clist), NULL);
  else
    gtk_clist_set_compare_func (GTK_CLIST (d->vardata_clist),
                                (GtkCListCompareFunc) arithmetic_compare);
  gtk_clist_sort (GTK_CLIST (d->vardata_clist));

  return;
}

void
vartable_row_append (gint j, datad *d, ggobid *gg)
{
  if (d->vardata_clist != NULL) {
    gint k;
    gchar **row;
    row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    row[0] = g_strdup_printf ("%d", j);
    row[1] = g_strdup (d->vardata[j].collab);
    row[2] = g_strdup_printf ("%d", d->vardata[j].groupid);
    row[3] = g_strdup ("");
    row[4] = g_strdup_printf ("%8.3f", d->vardata[j].lim_raw.min);
    row[5] = g_strdup_printf ("%8.3f", d->vardata[j].lim_raw.max);
    row[6] = g_strdup_printf ("%8.3f", d->vardata[j].mean);
    row[7] = g_strdup_printf ("%8.3f", d->vardata[j].median);
    row[8] = g_strdup_printf ("%d", d->vardata[j].nmissing);

    gtk_clist_append ((GtkCList *) d->vardata_clist, row);

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
  datad *d = gg->current_display->d;

  if (gg->vardata_window == NULL) {

    gg->vardata_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (gg->vardata_window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), gg);
    gtk_window_set_title (GTK_WINDOW (gg->vardata_window),
      "Variable statistics");

    vbox = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_container_add (GTK_CONTAINER (gg->vardata_window), vbox);
    gtk_widget_show (vbox);

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;

      /* Create a scrolled window to pack the CList widget into */
      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
      gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
      gtk_widget_show (scrolled_window);

      d->vardata_clist = gtk_clist_new_with_titles (NCOLS_CLIST, titles);
      gtk_clist_set_selection_mode (GTK_CLIST (d->vardata_clist),
        GTK_SELECTION_MULTIPLE);

      /*-- left justify all the numerical columns --*/
      gtk_clist_set_column_justification (GTK_CLIST (d->vardata_clist),
        2, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vardata_clist),
        4, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vardata_clist),
        5, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vardata_clist),
        6, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vardata_clist),
        7, GTK_JUSTIFY_RIGHT);
      gtk_clist_set_column_justification (GTK_CLIST (d->vardata_clist),
        8, GTK_JUSTIFY_RIGHT);

      /*-- make the first column invisible --*/
      gtk_clist_set_column_visibility (GTK_CLIST (d->vardata_clist), 0, false);

      /*-- set the column width automatically --*/
      for (k=0; k<NCOLS_CLIST; k++)
        gtk_clist_set_column_auto_resize (GTK_CLIST (d->vardata_clist), k, true);

      /*-- populate the table --*/
      for (j=0 ; j<d->ncols ; j++)
        vartable_row_append (j, d, gg);

      /*-- track selections --*/
      gtk_signal_connect (GTK_OBJECT (d->vardata_clist), "select_row",
                         GTK_SIGNAL_FUNC (selection_made),
                         gg);
      gtk_signal_connect (GTK_OBJECT (d->vardata_clist), "unselect_row",
                         GTK_SIGNAL_FUNC (deselection_made),
                         gg);

      /*-- re-sort when receiving a mouse click on a column header --*/
      gtk_signal_connect (GTK_OBJECT (d->vardata_clist), "click_column",
                         GTK_SIGNAL_FUNC (sortbycolumn_cb),
                         gg);

      /* It isn't necessary to shadow the border, but it looks nice :) */
      gtk_clist_set_shadow_type (GTK_CLIST (d->vardata_clist), GTK_SHADOW_OUT);

   /* gtk_box_pack_start (GTK_BOX (vbox), d->vardata_clist, TRUE, TRUE, 0);*/
      gtk_container_add (GTK_CONTAINER (scrolled_window), d->vardata_clist);
      gtk_widget_show (d->vardata_clist);
    }

    /*-- 3 = COLUMN_INSET --*/
    gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
      d->vardata_clist->requisition.width + 3 +
      GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
      150);

    gtk_widget_show_all (gg->vardata_window);
  }

  gdk_window_raise (gg->vardata_window->window);
}


void
vartable_tform_set (gint varno, datad *d, ggobid *gg) {

  if (d->vardata_clist != NULL)
    gtk_clist_set_text (GTK_CLIST (d->vardata_clist), varno,
      3, d->vardata[varno].collab_tform);
}
