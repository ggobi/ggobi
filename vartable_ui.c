/* vartable_ui.c */ 
/* interface code for the variable statistics table */

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"

#define NCOLS_CLIST 9

static GtkWidget *vardata_window = (GtkWidget *) NULL;
static GtkWidget *clist = NULL;

void delete_cb (GtkWidget *cl, GdkEventButton *event, gpointer data)
{
  gtk_widget_hide (vardata_window);
}

void
vartable_select_var (gint jvar, gboolean selected)
{
  gint j, varno;
  gchar *varno_str;

  /*-- loop over the rows in the table, looking for jvar --*/
  for (j=0; j<xg.ncols; j++) {
    if (clist != NULL) {
      gtk_clist_get_text (GTK_CLIST (clist), j, 0, &varno_str);
      varno = (gint) atoi (varno_str);
    } else varno = j;
    
    if (varno == jvar) {
      if (clist != NULL) {
        if (selected)
          gtk_clist_select_row (GTK_CLIST (clist), jvar, 1);
        else
          gtk_clist_unselect_row (GTK_CLIST (clist), jvar, 1);
      }
      xg.vardata[jvar].selected = selected;
    }
  }
}

void
vartable_unselect_all () 
{
  if (clist != NULL)
    gtk_clist_unselect_all (GTK_CLIST (clist));
}

void
selection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, gpointer data)
{
  gint varno;
  gchar *varno_str;

  gtk_clist_get_text (GTK_CLIST (clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  xg.vardata[varno].selected = true;

  return;
}
void
deselection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, gpointer data)
{
  gint varno;
  gchar *varno_str;

  gtk_clist_get_text (GTK_CLIST (clist), row, 0, &varno_str);
  varno = (gint) atoi (varno_str);
  xg.vardata[varno].selected = false;
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

void sortbycolumn_cb (GtkWidget *cl, gint column, gpointer data)
{
  gtk_clist_set_sort_column (GTK_CLIST (clist), column);
  if (column == 1)  /*-- variable name --*/
    gtk_clist_set_compare_func (GTK_CLIST (clist), NULL);
  else
    gtk_clist_set_compare_func (GTK_CLIST (clist),
                                (GtkCListCompareFunc) arithmetic_compare);
  gtk_clist_sort (GTK_CLIST (clist));

  return;
}

void
vartable_row_append (gint j)
{
  if (clist != NULL) {
    gint k;
    gchar **row;
    row = (gchar **) g_malloc (NCOLS_CLIST * sizeof (gchar *));

    row[0] = g_strdup_printf ("%d", j);
    row[1] = g_strdup (xg.vardata[j].collab);
    row[2] = g_strdup_printf ("%d", xg.vardata[j].groupid);
    row[3] = g_strdup ("");
    row[4] = g_strdup_printf ("%8.3f", xg.vardata[j].lim_raw.min);
    row[5] = g_strdup_printf ("%8.3f", xg.vardata[j].lim_raw.max);
    row[6] = g_strdup_printf ("%8.3f", xg.vardata[j].mean);
    row[7] = g_strdup_printf ("%8.3f", xg.vardata[j].median);
    row[8] = g_strdup_printf ("%d", xg.vardata[j].nmissing);

    gtk_clist_append ((GtkCList *) clist, row);

    for (k=0; k<NCOLS_CLIST; k++)
      g_free ((gpointer) row[k]);
    g_free ((gpointer) row);
  }
}


void
vartable_create ()
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

  if (vardata_window == NULL) {

    vardata_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (vardata_window),
      "delete_event", GTK_SIGNAL_FUNC (delete_cb), NULL);
    gtk_window_set_title (GTK_WINDOW (vardata_window), "Variable statistics");

    vbox = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    gtk_container_add (GTK_CONTAINER (vardata_window), vbox);
    gtk_widget_show (vbox);
    
    /* Create a scrolled window to pack the CList widget into */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show (scrolled_window);

    clist = gtk_clist_new_with_titles (NCOLS_CLIST, titles);
    gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_MULTIPLE);

    /*-- left justify all the numerical columns --*/
    gtk_clist_set_column_justification (GTK_CLIST (clist),
      2, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (clist),
      4, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (clist),
      5, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (clist),
      6, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (clist),
      7, GTK_JUSTIFY_RIGHT);
    gtk_clist_set_column_justification (GTK_CLIST (clist),
      8, GTK_JUSTIFY_RIGHT);

    /*-- make the first column invisible --*/
    gtk_clist_set_column_visibility (GTK_CLIST (clist), 0, false);

    /*-- set the column width automatically --*/
    for (k=0; k<NCOLS_CLIST; k++)
      gtk_clist_set_column_auto_resize (GTK_CLIST (clist), k, true);

    /*-- populate the table --*/
    for (j=0 ; j<xg.ncols ; j++)
      vartable_row_append (j);

    /*-- track selections --*/
    gtk_signal_connect (GTK_OBJECT (clist), "select_row",
                       GTK_SIGNAL_FUNC (selection_made),
                       NULL);
    gtk_signal_connect (GTK_OBJECT (clist), "unselect_row",
                       GTK_SIGNAL_FUNC (deselection_made),
                       NULL);

    /*-- re-sort when receiving a mouse click on a column header --*/
    gtk_signal_connect (GTK_OBJECT (clist), "click_column",
                       GTK_SIGNAL_FUNC (sortbycolumn_cb),
                       NULL);

    /* It isn't necessary to shadow the border, but it looks nice :) */
    gtk_clist_set_shadow_type (GTK_CLIST (clist), GTK_SHADOW_OUT);

    gtk_container_add (GTK_CONTAINER (scrolled_window), clist);
    gtk_widget_show (clist);

    gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
      clist->requisition.width + 3 /*-- COLUMN_INSET --*/ +
      GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
      150);

    gtk_widget_show_all (vardata_window);
  }

  gdk_window_raise (vardata_window->window);
}



void
vartable_tform_set (gint varno) {

  if (clist != NULL)
    gtk_clist_set_text (GTK_CLIST (clist), varno,
      3, xg.vardata[varno].collab_tform);
}
