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

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "vartable.h"

#include <string.h> /* for strcmp() */

#ifdef MULTIPLE_CLISTS
#define NCOLS_CAT 9
#endif

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


GtkCList *
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
#ifdef MULTIPLE_CLISTS
  GtkWidget *vb = gtk_vbox_new(false, 5);
#endif

  lbl = datasetName (d, gg);

#ifndef MULTIPLE_CLISTS
  /* Create a scrolled window to pack the CList widget into */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  /*
   * We're showing all datasets for now, whether they have variables
   * or not.  That could change.
  */
  gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
                            scrolled_window, gtk_label_new (lbl));
  gtk_widget_show (scrolled_window);
#else
  gtk_notebook_append_page (GTK_NOTEBOOK (gg->vartable_ui.notebook),
                            vb, gtk_label_new (lbl));
#endif
  g_free (lbl);


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

#ifndef MULTIPLE_CLISTS
  gtk_container_add (GTK_CONTAINER (scrolled_window), d->vartable_clist);
  gtk_widget_show (d->vartable_clist);

  /*-- 3 = COLUMN_INSET --*/
  gtk_widget_set_usize (GTK_WIDGET (scrolled_window),
    d->vartable_clist->requisition.width + 3 +
    GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar->requisition.width,
    150);
#else
  gtk_box_pack_start (GTK_BOX (vb), d->vartable_clist,
    true, true, 2);

/* add another clist, just to see how it would look */
{
  GtkWidget *clist_cat;
  gchar *titles_cat[NCOLS_CAT] = {
    "varno",          /*-- varno will be an invisible column --*/
    "Variable",
    "Level",
    "Count",
    "Min (user)", "Max (user)",
    "Min (data)", "Max (data)"
    "N NAs",
  };
  clist_cat = gtk_clist_new_with_titles (NCOLS_CAT, titles_cat);
  gtk_box_pack_start (GTK_BOX (vb), clist_cat,
    true, true, 2);

  gtk_clist_set_column_visibility (GTK_CLIST (clist_cat), 0, false);
}

  gtk_widget_show_all (vb);
#endif

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
  gchar *ind;
  vartabled *vt;

  if (d->vartable_clist != NULL) {
    vt = vartable_element_get (j, d);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_VARNAME, vt->collab);
    ind = (vt->vartype == categorical) ?
      g_strdup_printf ("y:%d", vt->nlevels) :
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

    stmp = g_strdup_printf ("%8.3f", (gfloat) vt->lim_display.min);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_DATA_MIN, stmp);
    g_free (stmp);

    stmp = g_strdup_printf ("%8.3f", (gfloat) vt->lim_display.max);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_DATA_MAX, stmp);
    g_free (stmp);

    if (vt->lim_specified_p) {
      stmp = g_strdup_printf ("%8.3f", (gfloat) vt->lim_specified_tform.min);
      gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
        CLIST_USER_MIN, stmp);
      g_free (stmp);

      stmp = g_strdup_printf ("%8.3f", (gfloat) vt->lim_specified_tform.max);
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

    /*-- for counter variables, don't display the mean --*/
    stmp = (vt->vartype == counter) ?
      g_strdup("") : g_strdup_printf ("%8.3f", vt->mean);
    gtk_clist_set_text (GTK_CLIST (d->vartable_clist), j,
      CLIST_MEAN, stmp);
    g_free (stmp);

    /*-- for categorical and counter variables, don't display the median --*/
    stmp = (vt->vartype == categorical || vt->vartype == counter) ?
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
