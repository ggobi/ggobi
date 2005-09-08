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

#include "ggobi.h"
#include "vars.h"

/* Global variables for the window containing a tree listing
   plots within displays */
typedef struct {

  GtkWidget *window;
  GtkWidget *tree;
  gint numItems;

} DisplayTree;

extern DisplayTree display_tree;

GtkWidget *display_add_tree(displayd *display, gint entry, GtkWidget *tree, ggobid *gg);

gchar *display_tree_label(displayd *display);

/* For Gtk 2.4, need to set -DGTK_ENABLE_BROKEN as GtkTree will not be defined otherwise. */
GtkTree *plot_tree_display(ggobid *gg);

void show_display_tree_cb (GtkWidget *widget, ggobid *);
void show_display_tree (ggobid *gg, GtkWidget *widget);

GtkWidget *splot_subtree_create(displayd *display, ggobid *gg);
gchar *splot_tree_label (splotd *, gint, datad *, ggobid *);

void display_tree_delete_cb(GtkWidget *w, GdkEvent *event, ggobid *gg);

gint  tree_display_entry_remove(displayd *display, GtkWidget *w, ggobid *gg);
gint  tree_display_entry_remove_by_index(gint which, GtkWidget *tree);

void display_tree_display_child_select(GtkWidget *root_tree,displayd *display);

void display_tree_splot_child_select(GtkWidget *root_tree, splotd *plot);

#endif
