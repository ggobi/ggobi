#ifndef DISPLAY_TREE_H
#define DISPLAY_TREE_H 1

#include <gtk/gtk.h>

#include "ggobi.h"
#include "vars.h"

/* Global variable for the */
typedef struct {

  GtkWidget *window;
  GtkWidget *tree;
  int numItems;

} DisplayTree;

extern DisplayTree display_tree;

GtkWidget *display_add_tree(displayd *display, int entry, GtkWidget *tree, ggobid *gg);

char *display_tree_label(displayd *display);
GtkTree *plot_tree_display(ggobid *);

/*
void show_display_tree (gpointer cbd, guint action, GtkWidget *widget);
*/
void show_display_tree (ggobid *gg, GtkWidget *widget);

GtkWidget *splot_subtree_create(displayd *display, ggobid *gg);
char *splot_tree_label (splotd *, gint, enum displaytyped, datad *, ggobid *);

void display_tree_delete_cb(GtkWidget *w, GdkEvent *event, ggobid *gg);

int  tree_display_entry_remove(displayd *display, GtkWidget *w, ggobid *gg);
int  tree_display_entry_remove_by_index(int which, GtkWidget *tree);

void display_tree_display_child_select(GtkWidget *root_tree,displayd *display);

void display_tree_splot_child_select(GtkWidget *root_tree, splotd *plot);

#endif
