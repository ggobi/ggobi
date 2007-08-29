/* display_tree.h */
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

#ifndef DISPLAY_TREE_H
#define DISPLAY_TREE_H 1

#include <gtk/gtk.h>

#include "session.h"
#include "vars.h"

/* Global variables for the window containing a tree listing
   plots within displays */
typedef struct {

  GtkWidget *window;
  GtkWidget *tree;
  GtkTreeModel *model;

} DisplayTree;

extern DisplayTree display_tree;

void display_add_tree(displayd *display);

gchar *display_tree_label(displayd *display);

GtkTreeView *plot_tree_display(GGobiSession *gg);

void show_display_tree_cb (GtkWidget *widget, GGobiSession *);
void show_display_tree (GGobiSession *gg, GtkWidget *widget);

void splot_add_tree(displayd *display, GtkTreeIter *parent);
gchar *splot_tree_label (splotd *, GGobiStage *, GGobiSession *);

void display_tree_delete_cb(GtkWidget *w, GdkEvent *event, GGobiSession *gg);

gboolean  tree_display_entry_remove(displayd *display, GtkWidget *w, GGobiSession *gg);

void display_tree_child_select(GtkTreeSelection *, gpointer cbd);

#endif
