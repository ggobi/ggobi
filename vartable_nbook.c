/* vartable_nbook.c */ 
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
/* interface code for the variable statistics table: notebook only */

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

#include <string.h> /* for strcmp() */

extern GtkWidget * vartable_buttonbox_build (ggobid *gg);

static void vartable_subwindow_init (datad *d, ggobid *gg);

static void close_wmgr_cb (GtkWidget *cl, GdkEventButton *event, ggobid *gg)
{
  gtk_widget_hide (gg->vartable_ui.window);
}
static void destroyit (ggobid *gg)
{
  gtk_widget_destroy (gg->vartable_ui.window);
  gg->vartable_ui.window = NULL;
}

static void 
vartable_notebook_adddata_cb (ggobid *gg, datad *d, void *notebook)
{
  vartable_subwindow_init (d, gg);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (GTK_WIDGET(notebook)),
    g_slist_length (gg->d) > 1);
}
CHECK_EVENT_SIGNATURE(vartable_notebook_adddata_cb, datad_added_f)

/*
 * Clear all selected rows from notebook pages
 * when they're de-selected.
*/
void
vartable_switch_page_cb (GtkNotebook *notebook, GtkNotebookPage *page,
  gint page_num, ggobid *gg)
{
  gint prev_page = gtk_notebook_get_current_page (notebook);
  GtkWidget *swin, *clist;
  GList *children;

  if (prev_page > -1) {
    swin = gtk_notebook_get_nth_page (notebook, prev_page);
    children = gtk_container_children (GTK_CONTAINER (swin));
    clist = g_list_nth_data (children, 0);
    gtk_clist_unselect_all (GTK_CLIST (clist));
  }
}

GtkCList *
vartable_clist_get (ggobid *gg) {
  GtkNotebook *nb, *subnb;
  gint indx, subindx;
  GtkWidget *swin;
  GList *children;
/*
 * Each page of vartable_ui.notebook has one child, which is
 * another notebook.
 * That notebook has two children, two scrolled windows, and
 * each scrolled window has one child, a clist
*/
/*
  vartable_ui.notebook
    page 0: datad 0
      nbook
        page 0: swin -> real
        page 1: swin -> categorical
    page n: datad n
      nbook
        page 0: swin -> real
        page 1: swin -> categorical
*/

  nb = GTK_NOTEBOOK (gg->vartable_ui.notebook);
  indx = gtk_notebook_get_current_page (nb);
  /*-- get the current page of the vartable notebook --*/
  subnb = (GtkNotebook *) gtk_notebook_get_nth_page (nb, indx);
  subindx = gtk_notebook_get_current_page (subnb);
  /*-- get the current page of the variable type notebook --*/
  swin = gtk_notebook_get_nth_page (subnb, subindx);
  children = gtk_container_children (GTK_CONTAINER (swin));

  return ((GtkCList *) g_list_nth_data (children, 0));
/*
  swin = gtk_notebook_get_nth_page (nb, indx);
  GList *swin_children = gtk_container_children (GTK_CONTAINER (swin));
*/
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

gint
vartable_rownum_from_varno (gint jvar, vartyped vartype, datad *d)
{
  GList *l;
  gint irow = -1;
  GtkCListRow *row;
  gchar *varstr;
  gint rownum = 0;
  vartyped type = (vartype == categorical) ? categorical : real;

  if (d->vartable_clist[type] != NULL) {
    l = GTK_CLIST(d->vartable_clist[type])->row_list;
    while (l) {
      row = GTK_CLIST_ROW (l);
      varstr = GTK_CELL_TEXT(row->cell[0])->text; /* 0th column for all types */
      if (varstr != NULL && strlen (varstr) > 0) {
        irow = atoi (varstr);
        if (irow == jvar)
          return rownum;
        rownum++;
      }
      l = l->next;
    }
  }
  return -1;
}

gint
vartable_varno_from_rownum (gint rownum, vartyped vartype, datad *d)
{
  GList *l;
  gint irow = -1;
  GtkCListRow *row;
  gchar *varstr;

  l = g_list_last (GTK_CLIST(d->vartable_clist[vartype])->row_list);
  while (l) {
    row = GTK_CLIST_ROW (l);
    varstr = GTK_CELL_TEXT(row->cell[0])->text;   /* var index always first */
    if (varstr != NULL && strlen (varstr) > 0) {
      irow = atoi (varstr);
      if (irow != -1)
        return irow;
    }
    l = l->prev;
  }

  return irow;
}

/*
void
vartable_select_var (gint jvar, gboolean selected, datad *d, ggobid *gg)
{
  gint j, varno;
  gchar *varno_str;
  vartabled *vt;
  vartyped vartype;

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    vartype = (vt->vartype == categorical) ? categorical : real;
    rownum = vartable_rownum_from_varno (j, vt->vartype, d);

    if (d->vartable_clist[vartype] != NULL) {
      while (rownum >= 0) {
        gtk_clist_get_text (GTK_CLIST (d->vartable_clist[vartype]), rownum, 0,
          &varno_str);
        varno = (gint) atoi (varno_str);
        if (varno >= 0)
          break;
        rownum--;
      }
    } else varno = j;

    if (varno == jvar) {
      if (d->vartable_clist[vartype] != NULL) {
        if (selected)
          gtk_clist_select_row (GTK_CLIST (d->vartable_clist[vartype]),
            jvar, 1);
        else
          gtk_clist_unselect_row (GTK_CLIST (d->vartable_clist[vartype]),
            jvar, 1);
      }
      vt = vartable_element_get (jvar, d);
      vt->selected = selected;
    }
  }
}
*/

void
selection_made (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  gint varno;
  gchar *varno_str;
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
  vartabled *vt;

  while (row >= 0) {
    gtk_clist_get_text (GTK_CLIST (cl), row, 0, &varno_str);
    varno = (gint) atoi (varno_str);
    if (varno >= 0) {
      vt = vartable_element_get (varno, d);
      vt->selected = true;
      break;
    } else row--;
  }

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

  while (row >= 0) {
    gtk_clist_get_text (GTK_CLIST (cl), row, 0, &varno_str);
    varno = (gint) atoi (varno_str);
    if (varno >= 0) {
      vt = vartable_element_get (varno, d);
      vt->selected = false;
      break;
    } else row--;
  }

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
/*
  datad *d = datad_get_from_notebook (gg->vartable_ui.notebook, gg);
*/

  gtk_clist_set_sort_column (GTK_CLIST (cl), column);

/*
   If column is already sorted in forward order, it would be useful to
   sort it in reverse order, but how do I determine its sort order?
   I can either keep an integer vector and keep track of each column's
   sort order, or I can just reset the sort order for the whole clist.
   The lists and trees are so different in gtk 1.3 that it doesn't
   seem worthwhile to work on this now.
*/

  if (column >= 1 && column <= 3)  /*-- name, cat?, tform --*/
    gtk_clist_set_compare_func (GTK_CLIST (cl), NULL);
  else
    gtk_clist_set_compare_func (GTK_CLIST (cl),
      (GtkCListCompareFunc) arithmetic_compare);
  gtk_clist_sort (GTK_CLIST (cl));

  return;
}

static void
vartable_row_assemble (gint jvar, vartyped type, gchar **row,
  datad *d, ggobid *gg)
{
/*-- a new (empty) row will be appended --*/
  switch (type) {
    case counter:
    case integer:
    case real:
      row[REAL_CLIST_VARNO] = g_strdup_printf ("%d", jvar);
      row[REAL_CLIST_VARNAME] = g_strdup ("");
      row[REAL_CLIST_TFORM] = g_strdup ("");
      row[REAL_CLIST_USER_MIN] = g_strdup ("");
      row[REAL_CLIST_USER_MAX] = g_strdup ("");
      row[REAL_CLIST_DATA_MIN] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_DATA_MAX] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_MEAN] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_MEDIAN] = g_strdup_printf ("%8.3f", 0.0);
      row[REAL_CLIST_NMISSING] = g_strdup_printf ("%d", 0);
    break;
    case categorical:
      row[CAT_CLIST_VARNO] = g_strdup_printf ("%d", jvar);
      row[CAT_CLIST_VARNAME] = g_strdup ("");
      row[CAT_CLIST_NLEVELS] = g_strdup ("");
      row[CAT_CLIST_LEVEL_NAME] = g_strdup ("");
      row[CAT_CLIST_LEVEL_VALUE] = g_strdup ("");
      row[CAT_CLIST_LEVEL_COUNT] = g_strdup ("");
      row[CAT_CLIST_USER_MIN] = g_strdup ("");
      row[CAT_CLIST_USER_MAX] = g_strdup ("");
      row[CAT_CLIST_DATA_MIN] = g_strdup ("");
      row[CAT_CLIST_DATA_MAX] = g_strdup ("");
      row[CAT_CLIST_NMISSING] = g_strdup ("");
    break;
    case all_vartypes:
      g_printerr ("(vartable_row_assemble) %d: illegal variable type %d\n",
        jvar, all_vartypes);
    break;
  }
}

void
vartable_row_append (gint jvar, datad *d, ggobid *gg)
{
  vartabled *vt = vartable_element_get (jvar, d);
  vartyped type = vt->vartype;
  gint k;
  gchar **row;
  gint ncolumns;
  if (type == categorical) {
    ncolumns = NCOLS_CLIST_CAT;
  } else {
    ncolumns = NCOLS_CLIST_REAL;
    type = real;
  }

  if (d->vartable_clist[type] != NULL) {
    row = (gchar **) g_malloc (ncolumns * sizeof (gchar *));

    vartable_row_assemble (jvar, type, row, d, gg);

    gtk_clist_freeze (GTK_CLIST (d->vartable_clist[type]));
    gtk_clist_append (GTK_CLIST (d->vartable_clist[type]), row);
    gtk_clist_thaw (GTK_CLIST (d->vartable_clist[type]));

    for (k=0; k<ncolumns; k++)
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
  gchar *titles[NCOLS_CLIST_REAL] = {
    "varno",          /*-- varno will be an invisible column --*/
    "Variable",
    "Transform",
    "Min (user)", "Max (user)",
    "Min (data)", "Max (data)",
    "Mean", "Median",
    "N NAs"};
  gchar *titles_cat[NCOLS_CLIST_CAT] = {
    "varno",          /*-- varno will be an invisible column --*/
    "Variable",
    "N Levels",
    "Level",
    "Value",
    "Count",
    "Min (user)", "Max (user)",
    "Min (data)", "Max (data)",
    "N NAs",
  };
  GtkWidget *nbook = gtk_notebook_new ();
  vartabled *vt;

  gtk_signal_connect (GTK_OBJECT (nbook), "switch-page",
    GTK_SIGNAL_FUNC (vartable_switch_page_cb), gg);

  lbl = datasetName (d, gg);
  /*
   * We're showing all datasets for now, whether they have variables
   * or not.  That could change.
  */
  gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
    nbook, gtk_label_new (lbl));
  g_free (lbl);


  /* Pack each clist into a scrolled window */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

/*
 * Page for real, counter and integer variables
*/
  d->vartable_clist[real] = gtk_clist_new_with_titles (NCOLS_CLIST_REAL,
    titles);

  gtk_clist_set_selection_mode (GTK_CLIST (d->vartable_clist[real]),
    GTK_SELECTION_EXTENDED);
  /*-- right justify all the numerical columns --*/
  for (k=0; k<NCOLS_CLIST_REAL; k++)
    gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist[real]),
      k, GTK_JUSTIFY_RIGHT);
  /*-- make the first column invisible --*/
  gtk_clist_set_column_visibility (GTK_CLIST (d->vartable_clist[real]),
    REAL_CLIST_VARNO, false);
  /*-- set the column width automatically --*/
  for (k=0; k<NCOLS_CLIST_REAL; k++)
    gtk_clist_set_column_auto_resize (GTK_CLIST (d->vartable_clist[real]),
                                      k, true);

  gtk_container_add (GTK_CONTAINER (scrolled_window), d->vartable_clist[real]);
  gtk_notebook_append_page (GTK_NOTEBOOK (nbook),
    scrolled_window, gtk_label_new ("real"));
  gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
    d->vartable_clist[real]->requisition.width + 3 +
    GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
    150);
  gtk_widget_show (scrolled_window);


  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
/*
 * Page for categorical variables
*/
  d->vartable_clist[categorical] = gtk_clist_new_with_titles (NCOLS_CLIST_CAT,
    titles_cat);

  gtk_clist_set_selection_mode (GTK_CLIST (d->vartable_clist[categorical]),
    GTK_SELECTION_EXTENDED);
  /*-- right justify all the numerical columns --*/
  for (k=0; k<NCOLS_CLIST_CAT; k++)
    gtk_clist_set_column_justification (GTK_CLIST (d->vartable_clist[categorical]),
      k, GTK_JUSTIFY_RIGHT);
  /*-- make the first column invisible --*/
  gtk_clist_set_column_visibility (GTK_CLIST (d->vartable_clist[categorical]),
    CAT_CLIST_VARNO, false);
  /*-- set the column width automatically --*/
  for (k=0; k<NCOLS_CLIST_CAT; k++)
    gtk_clist_set_column_auto_resize (GTK_CLIST (d->vartable_clist[categorical]),
                                      k, true);

  gtk_container_add (GTK_CONTAINER (scrolled_window),
    d->vartable_clist[categorical]);
  gtk_notebook_append_page (GTK_NOTEBOOK (nbook),
    scrolled_window, gtk_label_new ("categorical"));
  /*-- 3 = COLUMN_INSET --*/
  gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
    d->vartable_clist[categorical]->requisition.width + 3 +
    GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
    150);
  gtk_widget_show (scrolled_window);



  /*-- populate the tables --*/
  for (j=0 ; j<d->ncols ; j++) {
    vt = vartable_element_get (j, d);

    vartable_row_append (j, d, gg);    /*-- append a generic row --*/
    if (vt->vartype == categorical)
      for (k=0; k<vt->nlevels; k++)
        vartable_row_append (j, d, gg);

    vartable_cells_set_by_var (j, d);  /*-- then opulate --*/
  }
  

/* reals (etc) */
  /*-- track selections --*/
  gtk_signal_connect (GTK_OBJECT (d->vartable_clist[real]),
    "select_row", GTK_SIGNAL_FUNC (selection_made), gg);
  gtk_signal_connect (GTK_OBJECT (d->vartable_clist[real]),
    "unselect_row", GTK_SIGNAL_FUNC (deselection_made), gg);
  /*-- re-sort when receiving a mouse click on a column header --*/
  gtk_signal_connect (GTK_OBJECT (d->vartable_clist[real]),
    "click_column", GTK_SIGNAL_FUNC (sortbycolumn_cb), gg);

/* categoricals */
  /*-- track selections --*/
  gtk_signal_connect (GTK_OBJECT (d->vartable_clist[categorical]),
    "select_row", GTK_SIGNAL_FUNC (selection_made), gg);
  gtk_signal_connect (GTK_OBJECT (d->vartable_clist[categorical]),
    "unselect_row", GTK_SIGNAL_FUNC (deselection_made), gg);
  /*-- re-sort when receiving a mouse click on a column header --*/
/*  no: because this is a goofy sort of hierarchical display
  gtk_signal_connect (GTK_OBJECT (d->vartable_clist[categorical]),
    "click_column", GTK_SIGNAL_FUNC (sortbycolumn_cb), gg);
*/


  /* It isn't necessary to shadow the border, but it looks nice :) */
/*
  gtk_clist_set_shadow_type (GTK_CLIST (d->vartable_clist[real]),
    GTK_SHADOW_OUT);
*/


  gtk_widget_show_all (nbook);

}

void
vartable_open (ggobid *gg)
{                                  
  GtkWidget *vbox, *hbox;
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

  /*-- listen for datad_added events --*/
  gtk_signal_connect (GTK_OBJECT (gg),
    "datad_added", GTK_SIGNAL_FUNC (vartable_notebook_adddata_cb),
     GTK_OBJECT (gg->vartable_ui.notebook));

  hbox = vartable_buttonbox_build (gg);
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
  vartabled *vt = vartable_element_get (j, d);
  gint rownum, k;

  if (vt) {
    rownum = vartable_rownum_from_varno (j, vt->vartype, d);

    switch (vt->vartype) {
      case categorical:
        if (d->vartable_clist[categorical] != NULL) {
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
            rownum, CAT_CLIST_VARNAME, vt->collab);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
            rownum, CAT_CLIST_NLEVELS,
            g_strdup_printf ("%d", vt->nlevels));
          /*-- set the level fields --*/
          for (k=0; k<vt->nlevels; k++) {
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
              k+1+rownum, CAT_CLIST_VARNO, "-1");
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
              k+1+rownum, CAT_CLIST_LEVEL_NAME,
              vt->level_names[k]);
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
              k+1+rownum, CAT_CLIST_LEVEL_VALUE,
              g_strdup_printf ("%d", vt->level_values[k]));
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
              k+1+rownum, CAT_CLIST_LEVEL_COUNT,
              g_strdup_printf ("%d", vt->level_counts[k]));
          }
        }
      break;
      case integer:
      case counter:
      case real:
        if (d->vartable_clist[real] != NULL) {
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), rownum,
            REAL_CLIST_VARNAME, vt->collab);
        }
      break;
      case all_vartypes:
        g_printerr ("(vartable_collab_set_by_var) illegal variable type %d\n", all_vartypes);
      break;
    }
  }
}

/*-- sets the name of the transformed variable --*/
void
vartable_collab_tform_set_by_var (gint j, datad *d)
{
  vartabled *vt;

  if (d->vartable_clist[real] != NULL) {
    vt = vartable_element_get (j, d);
    if (vt->tform0 == NO_TFORM0 &&
        vt->tform1 == NO_TFORM1 &&
        vt->tform2 == NO_TFORM2)
    {
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), j,
        REAL_CLIST_TFORM, g_strdup(""));
    } else {
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), j,
        REAL_CLIST_TFORM, vt->collab_tform);
    }
  }
}

/*-- sets the limits for a variable --*/
void
vartable_limits_set_by_var (gint j, datad *d)
{
  gchar *stmp;
  vartabled *vt = vartable_element_get (j, d);
  if (vt) {
    gint rownum = vartable_rownum_from_varno (j, vt->vartype, d);

    switch (vt->vartype) {
      case integer:
      case counter:
      case real:
        if (d->vartable_clist[real] != NULL) {

          stmp = g_strdup_printf ("%8.3f", (gfloat) vt->lim_display.min);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), rownum,
            REAL_CLIST_DATA_MIN, stmp);
          g_free (stmp);

          stmp = g_strdup_printf ("%8.3f", (gfloat) vt->lim_display.max);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), rownum,
            REAL_CLIST_DATA_MAX, stmp);
          g_free (stmp);

          if (vt->lim_specified_p) {
            stmp = g_strdup_printf ("%8.3f",
              (gfloat) vt->lim_specified_tform.min);
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), rownum,
              REAL_CLIST_USER_MIN, stmp);
            g_free (stmp);

            stmp = g_strdup_printf ("%8.3f",
              (gfloat) vt->lim_specified_tform.max);
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]), rownum,
              REAL_CLIST_USER_MAX, stmp);
            g_free (stmp);
          }
        }
      break;

      case categorical:
        if (d->vartable_clist[categorical] != NULL) {

          stmp = g_strdup_printf ("%d", (gint) vt->lim_display.min);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
            rownum, CAT_CLIST_DATA_MIN, stmp);
          g_free (stmp);

          stmp = g_strdup_printf ("%d", (gint) vt->lim_display.max);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
            rownum, CAT_CLIST_DATA_MAX, stmp);
          g_free (stmp);

          if (vt->lim_specified_p) {
            stmp = g_strdup_printf ("%d", (gint) vt->lim_specified_tform.min);
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
              rownum, CAT_CLIST_USER_MIN, stmp);
            g_free (stmp);

            stmp = g_strdup_printf ("%d", (gint) vt->lim_specified_tform.max);
            gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
              rownum, CAT_CLIST_USER_MAX, stmp);
            g_free (stmp);
          }
        }
      break;
      case all_vartypes:
        g_printerr ("(vartable_limits_set_by_var) %d: illegal variable type %d\n",
          j, all_vartypes);
      break;
    }
  }
}
void
vartable_limits_set (datad *d) 
{
  gint j;
  if (d->vartable_clist[real] != NULL || d->vartable_clist[categorical] != NULL)
    for (j=0; j<d->ncols; j++)
      vartable_limits_set_by_var (j, d);
}

/*-- sets the mean, median, and number of missings for a variable --*/
void
vartable_stats_set_by_var (gint j, datad *d) {
  vartabled *vt = vartable_element_get (j, d);
  gchar *stmp;
  vartyped type;

  if (vt) {
    gint rownum = vartable_rownum_from_varno (j, vt->vartype, d);
    switch (vt->vartype) {
      case categorical:
        if (d->vartable_clist[categorical] != NULL) {
          stmp = g_strdup_printf ("%d", vt->nmissing);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[categorical]),
            rownum, CAT_CLIST_NMISSING, stmp);
          g_free (stmp);
        }

      case integer:
      case counter:
      case real:
        type = real;
        if (d->vartable_clist[real] != NULL) {

          /*-- for counter variables, don't display the mean --*/
          stmp = (vt->vartype == counter) ?
            g_strdup("") : g_strdup_printf ("%8.3f", vt->mean);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]),
            rownum, REAL_CLIST_MEAN, stmp);
          g_free (stmp);

          /*-- for counter variables, don't display the median --*/
          stmp = (vt->vartype == counter) ?
            g_strdup("") : g_strdup_printf ("%8.3f", vt->median);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]),
            rownum, REAL_CLIST_MEDIAN, stmp);
          g_free (stmp);

          stmp = g_strdup_printf ("%d", vt->nmissing);
          gtk_clist_set_text (GTK_CLIST (d->vartable_clist[real]),
            rownum, REAL_CLIST_NMISSING, stmp);
          g_free (stmp);
        }
      break;
      case all_vartypes:
        g_printerr ("(vartable_stats_set_by_var) %d: illegal variable type %d\n",
          j, vt->vartype);
      break;
    }
  }
}

void
vartable_stats_set (datad *d) {
  gint j;

  if (d->vartable_clist[real] != NULL)
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
