/*-- display_tree.c --*/
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

#include <stdlib.h>
#include <string.h>

#include "ggobi.h"
/* #include "display_tree.h" */
#include "externs.h"
#include "vars.h"

enum
{ DISPTREE_LABEL, DISPTREE_DATASET, DISPTREE_PMODE, DISPTREE_IMODE,
  DISPTREE_OBJECT, DISPTREE_NCOLS
};


gboolean
display_tree_get_iter_for_object (GtkTreeModel * model, gpointer obj,
                                  GtkTreeIter * iter)
{
  gboolean found = false, exists;
  gpointer tmp_obj;

  exists = gtk_tree_model_get_iter_first (model, iter);
  while (!found && exists) {
    gtk_tree_model_get (model, iter, DISPTREE_OBJECT, &tmp_obj, -1);
    if (tmp_obj == obj)
      found = true;
    else
      exists = gtk_tree_model_iter_next (model, iter);
  }
  return (exists);
}

/*

  Manipulates a separate window for displaying
  a tree representing the open windows being 
  managed by this ggobi session/application.
  The sub-elements within each display node are the 
  plots within that node.

 */

static void
update_display_tree_plots_by_variable (ggobid * gg, GGobiData * d,
                                       gint whichVar, splotd * sp,
                                       GtkTreeModel * model)
{
  GtkTreeIter iter;
  gchar *label;

  g_return_if_fail (GTK_IS_TREE_STORE (model));

  display_tree_get_iter_for_object (model, sp, &iter);

  label = splot_tree_label (sp, d, gg);
  gtk_tree_store_set (GTK_TREE_STORE (model), &iter, DISPTREE_LABEL, label,
                      -1);
  /* dfs: don't free
  if (label)
    g_free (label);
  */
}

CHECK_EVENT_SIGNATURE (update_display_tree_plots_by_variable,
                       select_variable_f)

     static gchar *disptree_lbl[] =
       { "Label", "Dataset", "View", "Interaction" };

/*
  Create a window displaying a hierarchical
  view of the current displays managed by this 
  Ggobi, along with their sub-plots.

  This can be used for selecting the current
  or active display, or plot.
  Also, one can add menus here to allow different
  operations on a group of selected items in the tree
  for things such as printing, deleting, editing, saving,
  etc.

  When this window is open, we need to add new windows 
  as they are created, and also remove those that are deleted.
  This is arranged by adding appropriate calls to the routines
  here from display_new() and display_free(), respectively.
 */
GtkTreeView *
plot_tree_display (ggobid * gg)
{
  GList *dlist;
  displayd *display;

  GtkWidget *tree, *sw;
  GtkWidget *plot_tree_window;
  GtkTreeModel *model;

  /* If this is the first time we have called this, 
     create it from scratch. Otherwise, we have to 
     update the display. The easiest way is to remove
     the entire contents of the tree and start rebuilding
     with the current model.
   */

  g_return_val_if_fail (gg->display_tree.tree == NULL, NULL);

  plot_tree_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (plot_tree_window), "GGobi Displays");
  gtk_window_set_default_size (GTK_WINDOW (plot_tree_window), 450, 200);

  model = GTK_TREE_MODEL (gtk_tree_store_new (DISPTREE_NCOLS, G_TYPE_STRING,
                                              G_TYPE_STRING, G_TYPE_STRING,
                                              G_TYPE_STRING, G_TYPE_OBJECT));

  g_signal_connect (G_OBJECT (gg),
                    "select_variable",
                    G_CALLBACK (update_display_tree_plots_by_variable),
                    (gpointer) model);

  gg->display_tree.model = model;
  for (dlist = gg->displays; dlist; dlist = dlist->next) {
    display = (displayd *) dlist->data;
    display_add_tree (display);
  }

  tree = gtk_tree_view_new_with_model (model);
  gg->display_tree.tree = tree;
  populate_tree_view (tree, disptree_lbl, G_N_ELEMENTS (disptree_lbl), true,
                      GTK_SELECTION_SINGLE,
                      G_CALLBACK (display_tree_child_select), NULL);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (sw), tree);
  gtk_container_add (GTK_CONTAINER (plot_tree_window), sw);
  gtk_widget_show_all (plot_tree_window);


  g_signal_connect (G_OBJECT (plot_tree_window), "delete_event",
                    G_CALLBACK (display_tree_delete_cb), gg);

  gg->display_tree.window = plot_tree_window;

  return (GTK_TREE_VIEW (tree));
}

void
display_add_tree (displayd * display)
{
  gchar *label;
  const gchar *dataset, *pmode, *imode;
  ggobid *gg = display->ggobi;
  GtkTreeIter disp_iter;
  GtkTreeModel *tree = gg->display_tree.model;
  if (tree == NULL)
    return;

  label = display_tree_label (display);
  dataset = display->d->name;
  imode = GGOBI (getIModeScreenName) (display->cpanel.imode, display);
  pmode = GGOBI (getPModeScreenName) (display->cpanel.pmode, display);

  gtk_tree_store_append (GTK_TREE_STORE (tree), &disp_iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (tree), &disp_iter,
                      DISPTREE_LABEL, label, DISPTREE_DATASET, dataset,
                      DISPTREE_IMODE, imode, DISPTREE_PMODE, pmode,
                      DISPTREE_OBJECT, display, -1);

  splot_add_tree (display, &disp_iter);
}


/*
  Called when the display tree window is closed.
 */
void
display_tree_delete_cb (GtkWidget * w, GdkEvent * event, ggobid * gg)
{
  gtk_widget_destroy (gg->display_tree.window);
  gg->display_tree.tree = NULL;
}



/*
   Create a sub-tree for the specified 
   window/display which may be a parallel coords plot,
   scatterplot, or scatterplot matrix.
 */

void
splot_add_tree (displayd * display, GtkTreeIter * parent)
{
  ggobid *gg = display->ggobi;
  GList *slist;
  splotd *sp;
  GGobiData *d = display->d;
  gchar *buf;
  GtkTreeIter iter;
  GtkTreeModel *model = gg->display_tree.model;

/*
g_signal_connect (G_OBJECT(tree), "select_child",
                             G_CALLBACK(display_tree_child_select), display);
*/
  /* Here do the plots within the display. */
  for (slist = display->splots; slist; slist = slist->next) {
    sp = (splotd *) slist->data;
    /*-- buf is allocated in splot_tree_label, but freed here --*/
    buf = splot_tree_label (sp, d, gg);
    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, parent);
    gtk_tree_store_set (GTK_TREE_STORE (model), &iter, DISPTREE_LABEL, buf,
                        DISPTREE_OBJECT, sp, -1);
    /* dfs: don't free
    if (buf)
      g_free (buf);
    */
  }
}

/*
   Use the windows title.
   Need to find out how to get the title from a GtkWindow.
   For now, use a string associated with the displaytype.
 */
gchar *
display_tree_label (displayd * display)
{
  gchar *val = NULL;

  if (GGOBI_IS_EXTENDED_DISPLAY (display))
    val = (gchar *) ggobi_display_tree_label (display);

  return (val);
}

/*
  Computes the label for an element in the tree for the specified
  splot, in position `ctr' and an element in the container of 
  type `type'.
 */
gchar *
splot_tree_label (splotd * splot, GGobiData * d, ggobid * gg)
{
  if (GGOBI_IS_EXTENDED_SPLOT (splot)) {
    return (GGOBI_EXTENDED_SPLOT_GET_CLASS (splot)->
            tree_label (splot, d, gg));
  }

  return (NULL);
}

/*
   Callback for a menu item, etc. to create and show
   the display and plot hierarchy.
 */
void                            /* used when DisplayTree is part of the ItemFactory in the main
                                   menubar */
show_display_tree (ggobid * gg, GtkWidget * widget)
{
  plot_tree_display (gg);
}

void                            /* used when DisplayTree is part of the Display menu */
show_display_tree_cb (GtkWidget * w, ggobid * gg)
{
  plot_tree_display (gg);
}

/*
 Identify the index of the given display and remove
 the corresponding node in the display_tree.
 */
gboolean
tree_display_entry_remove (displayd * display, GtkWidget * tree, ggobid * gg)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (!tree)
    return false;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree));
  display_tree_get_iter_for_object (model, display, &iter);

  return (gtk_tree_store_remove (GTK_TREE_STORE (model), &iter));
}

/*
 Called when a node corresponding to a window/display is
 selected in the display tree.
 This sets that display to be the current or active one.
 */
void
display_tree_child_select (GtkTreeSelection * sel, gpointer data)
{
  displayd *display;
  splotd *splot = NULL;
  ggobid *gg;
  gpointer obj;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected (sel, &model, &iter))
    return;
  gtk_tree_model_get (model, &iter, DISPTREE_OBJECT, &obj, -1);

  if (GGOBI_IS_SPLOT (obj)) {
    splot = GGOBI_SPLOT (obj);
    display = splot->displayptr;
  }
  else if (GGOBI_IS_DISPLAY (obj)) {
    display = GGOBI_DISPLAY (obj);
  }
  else
    return;

  gg = GGobiFromDisplay (display);
  g_return_if_fail (gg->display_tree.tree != NULL);
  /* Top-level of tree, so set that to be the current display. */
  if (!splot && gg->current_splot->displayptr != display) {
    splot = (splotd *) g_list_nth_data (display->splots, 0);
  }
  if (splot)
    GGOBI (splot_set_current_full) (display, splot, gg);

  gtk_widget_show (GGOBI_WINDOW_DISPLAY (display)->window);
  /* And now make certain the window comes to the top. */
  gdk_window_raise (GGOBI_WINDOW_DISPLAY (display)->window->window);
}
