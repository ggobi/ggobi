/* vartable_ui.c */ 
/* interface code for the variable statistics table */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define NCOLS_CLIST 9


void delete_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->app.vardata_window);
}

void
vartable_select_var (gint jvar, gboolean selected, ggobid *gg)
{
  gint j, varno;
  gchar *varno_str;

  /*-- loop over the rows in the table, looking for jvar --*/
  for (j=0; j<gg->ncols; j++) {
    if (gg->app.clist != NULL) {
      gtk_clist_get_text (GTK_CLIST (gg->app.clist), j, 0, &varno_str);
      varno = (gint) atoi (varno_str);
    } else varno = j;
    
    if (varno == jvar) {
      if (gg->app.clist != NULL) {
        if (selected)
          gtk_clist_select_row (GTK_CLIST (gg->app.clist), jvar, 1);
        else
          gtk_clist_unselect_row (GTK_CLIST (gg->app.clist), jvar, 1);
      }
      gg->vardata[jvar].selected = selected;
    }
  }
}

void
vartable_unselect_all (ggobid *gg) 
{
  gint j;

  if (gg->app.clist != NULL)
    gtk_clist_unselect_all (GTK_CLIST (gg->app.clist));

  for (j=0; j<gg->ncols; j++)
    gg->vardata[j].selected = false;
}

void
selection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;

  gtk_clist_get_text (GTK_CLIST (gg->app.clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  gg->vardata[varno].selected = true;

  return;
}
void
deselection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;

  gtk_clist_get_text (GTK_CLIST (gg->app.clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  gg->vardata[varno].selected = false;
  g_printerr ("deselected row= %d, varno= %d\n", row, varno);

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
  gtk_clist_set_sort_column (GTK_CLIST (gg->app.clist), column);
  if (column == 1)  /*-- variable name --*/
    gtk_clist_set_compare_func (GTK_CLIST (gg->app.clist), NULL);
  else
    gtk_clist_set_compare_func (GTK_CLIST (gg->app.clist),
                                (GtkCListCompareFunc) arithmetic_compare);
  gtk_clist_sort (GTK_CLIST (gg->app.clist));

  return;
}

void
vartable_row_append (gint j, ggobid *gg)
{
  if (gg->app.clist != NULL) {
    gint k;
    gchar **row;
    row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    row[0] = g_strdup_printf ("%d", j);
    row[1] = g_strdup (gg->vardata[j].collab);
    row[2] = g_strdup_printf ("%d", gg->vardata[j].groupid);
    row[3] = g_strdup ("");
    row[4] = g_strdup_printf ("%8.3f", gg->vardata[j].lim_raw.min);
    row[5] = g_strdup_printf ("%8.3f", gg->vardata[j].lim_raw.max);
    row[6] = g_strdup_printf ("%8.3f", gg->vardata[j].mean);
    row[7] = g_strdup_printf ("%8.3f", gg->vardata[j].median);
    row[8] = g_strdup_printf ("%d", gg->vardata[j].nmissing);

    gtk_clist_append ((GtkCList *) gg->app.clist, row);

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

  if (gg->app.vardata_window == NULL) {

    gg->app.vardata_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (gg->app.vardata_window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), gg);
    gtk_window_set_title (GTK_WINDOW (gg->app.vardata_window),
      "Variable statistics");

    vbox = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_container_add (GTK_CONTAINER (gg->app.vardata_window), vbox);
    gtk_widget_show (vbox);
    
    /* Create a scrolled window to pack the CList widget into */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_window);

    gg->app.clist = gtk_clist_new_with_titles (NCOLS_CLIST, titles);
    gtk_clist_set_selection_mode (GTK_CLIST (gg->app.clist),
      GTK_SELECTION_MULTIPLE);

    /*-- left justify all the numerical columns --*/
    gtk_clist_set_column_justification (GTK_CLIST (gg->app.clist),
      2, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (gg->app.clist),
      4, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (gg->app.clist),
      5, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (gg->app.clist),
      6, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (gg->app.clist),
      7, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (gg->app.clist),
      8, GTK_JUSTIFY_RIGHT);

    /*-- make the first column invisible --*/
    gtk_clist_set_column_visibility (GTK_CLIST (gg->app.clist), 0, false);

    /*-- set the column width automatically --*/
    for (k=0; k<NCOLS_CLIST; k++)
      gtk_clist_set_column_auto_resize (GTK_CLIST (gg->app.clist), k, true);

    /*-- populate the table --*/
    for (j=0 ; j<gg->ncols ; j++)
      vartable_row_append (j, gg);

    /*-- track selections --*/
    gtk_signal_connect (GTK_OBJECT (gg->app.clist), "select_row",
                       GTK_SIGNAL_FUNC (selection_made),
                       gg);
    gtk_signal_connect (GTK_OBJECT (gg->app.clist), "unselect_row",
                       GTK_SIGNAL_FUNC (deselection_made),
                       gg);

    /*-- re-sort when receiving a mouse click on a column header --*/
    gtk_signal_connect (GTK_OBJECT (gg->app.clist), "click_column",
                       GTK_SIGNAL_FUNC (sortbycolumn_cb),
                       gg);

    /* It isn't necessary to shadow the border, but it looks nice :) */
    gtk_clist_set_shadow_type (GTK_CLIST (gg->app.clist), GTK_SHADOW_OUT);

    gtk_container_add (GTK_CONTAINER (scrolled_window), gg->app.clist);
    gtk_widget_show (gg->app.clist);

    gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
      gg->app.clist->requisition.width + 3 /*-- COLUMN_INSET --*/ +
      GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
      150);

    gtk_widget_show_all (gg->app.vardata_window);
  }

  gdk_window_raise (gg->app.vardata_window->window);
}



void
vartable_tform_set (gint varno, ggobid *gg) {

  if (gg->app.clist != NULL)
    gtk_clist_set_text (GTK_CLIST (gg->app.clist), varno,
      3, gg->vardata[varno].collab_tform);
}
