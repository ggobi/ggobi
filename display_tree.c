#include <stdlib.h>

#include "ggobi.h"
/* #include "display_tree.h" */
#include "externs.h"
#include "vars.h"

/*

  Manipulates a separate window for displaying
  a tree representing the open windows being 
  managed by this ggobi session/application.
  The sub-elements within each display node are the 
  plots within that node.

 */



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
GtkTree*
plot_tree_display(ggobid *gg)
{
 GList *dlist;
 displayd *display;

 GtkWidget *tree;
 GtkWidget *plot_tree_window;
 int numItems;

 /* If this is the first time we have called this, 
    create it from scratch. Otherwise, we have to 
    update the display. The easiest way is to remove
    the entire contents of the tree and start rebuilding
    with the current model.
  */
 if(gg->display_tree.tree == NULL) {
  plot_tree_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(plot_tree_window), "GGobi Displays");
 } else {

  fprintf(stderr, "The display tree is already visible. It should be correct!\n");fflush(stderr);
  return(NULL);

  /* 
   This can attempt to remove the contents of the window,
   and allow them to be rebuilt so as to be consistent with the
   current display list.
  tree = gg->display_tree.tree;
  gtk_container_remove(GTK_CONTAINER(gtk_widget_get_ancestor(plot_tree_window, GTK_TYPE_WINDOW)), tree);
  */
 }

  tree = gtk_tree_new();

  numItems = 0;
  for (dlist = gg->displays; dlist; dlist = dlist->next, numItems++) {
    display = (displayd *) dlist->data;
    display_add_tree(display, numItems, tree, gg);
  }

  /*
 gtk_signal_connect (GTK_OBJECT(tree), "select_child",
                      GTK_SIGNAL_FUNC(display_tree_child_select), NULL);
  */
 gtk_container_add(GTK_CONTAINER( plot_tree_window ), tree);
 gtk_widget_show_all(plot_tree_window);


  gtk_signal_connect (GTK_OBJECT (plot_tree_window), "delete_event",
                      GTK_SIGNAL_FUNC (display_tree_delete_cb), gg);

 gg->display_tree.tree = tree;
 gg->display_tree.numItems = numItems;
 gg->display_tree.window = plot_tree_window;

 return(GTK_TREE( tree ));
}


GtkWidget *
display_add_tree(displayd *display, int entry, GtkWidget *tree, ggobid *gg)
{
  GtkWidget *item, *subTree;

   if(tree == NULL)
    return(NULL);

    item = gtk_tree_item_new_with_label(display_tree_label(display));
gtk_signal_connect (GTK_OBJECT(item), "select",
                                 GTK_SIGNAL_FUNC(display_tree_display_child_select), display);
    gtk_tree_append(GTK_TREE( tree ), item);
    gtk_widget_show(item);


    subTree = splot_subtree_create(display, gg);
    gtk_tree_set_view_mode (GTK_TREE(subTree), GTK_TREE_VIEW_ITEM);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM( item ), subTree);

 return(item);
}


/*
  Called when the display tree window is closed.
 */
void
display_tree_delete_cb(GtkWidget *w, GdkEvent *event, ggobid *gg) 
{
 gg->display_tree.tree = NULL;
 gg->display_tree.numItems = -1;
}



/*
   Create a sub-tree for the specified 
   window/display which may be a parallel coords plot,
   scatterplot, or scatterplot matrix.
 */

GtkWidget *
splot_subtree_create (displayd *display, ggobid *gg)
{
  GList *slist;
  splotd *sp;
  GtkWidget *tree, *item;
  gint ctr = 0;
  datad *d = display->d;
  
  tree = gtk_tree_new();

/*
gtk_signal_connect (GTK_OBJECT(tree), "select_child",
                             GTK_SIGNAL_FUNC(display_tree_child_select), display);
*/
      /* Here do the plots within the display. */
  for (slist = display->splots; slist ; slist = slist->next, ctr++) {
    sp = slist->data;
    item = gtk_tree_item_new_with_label (splot_tree_label (sp,
      ctr, display->displaytype, d, gg));

    gtk_signal_connect (GTK_OBJECT(item), "select",
                        GTK_SIGNAL_FUNC(display_tree_splot_child_select), sp);
    gtk_widget_show (item);

    gtk_tree_append (GTK_TREE (tree), item);
  }

   return (tree);
}

/*
   Use the windows title.
   Need to find out how to get the title from a GtkWindow.
   For now, use a string associated with the displaytype.
 */
char *
display_tree_label(displayd *display)
{
 char *val;

  switch(display->displaytype) {
    case scatterplot:
      val = "Scatterplot";
     break;
    case scatmat:
      val = "Scatterplot Matrix";
     break;
    case parcoords:
      val = "Parallel Coordinates";
     break;
  }

 return(val);
}

/*
  Computes the label for an element in the tree for the specified
  splot, in position `ctr' and an element in the container of 
  type `type'.
 */
gchar *
splot_tree_label(splotd *splot, gint ctr, enum displaytyped type,
  datad *d, ggobid *gg)
{
  gchar *buf;
  gint n;

  switch (type) {
    case scatterplot:
    case scatmat:
      n = strlen (d->vartable[splot->xyvars.x].collab) +
          strlen (d->vartable[splot->xyvars.y].collab) + 5;
      buf = (gchar*) g_malloc (n * sizeof (gchar*));
      sprintf (buf, "%s v %s",
        d->vartable[splot->xyvars.x].collab,
        d->vartable[splot->xyvars.y].collab);
     break;
    case parcoords:
      n = strlen (d->vartable[splot->p1dvar].collab);
      buf = (gchar*) g_malloc(n* sizeof (gchar*));
      sprintf(buf, "%s", d->vartable[splot->p1dvar].collab);
     break;
  }

  return (buf);
}


/*
   Callback for a menu item, etc. to create and show
   the display and plot hierarchy.
 */
void
show_display_tree (ggobid *gg, GtkWidget *widget)
{
  plot_tree_display(gg);
}


/*
 Identify the index of the given display and remove
 the corresponding node in the display_tree.
 */
int
tree_display_entry_remove(displayd *display, GtkWidget *tree, ggobid *gg)
{
 GList *dlist;
 displayd *tmp;
 int which = 0;

  if(tree == NULL)
    return(-1);

  for (dlist = gg->displays; dlist; dlist = dlist->next, which++) {
    tmp = (displayd *) dlist->data;
    if(tmp == display)
      return(tree_display_entry_remove_by_index(which, tree));

  } 

 return(-1);
}

/**
  Remove the specified entry from the top-level of the 
  given tree.
 */
int  
tree_display_entry_remove_by_index(int which, GtkWidget *tree)
{
fprintf(stderr, "Removing display %d\n", which); fflush(stderr);
  gtk_tree_clear_items(GTK_TREE(tree), which, which+1);

 return(which);
}



/*
 Called when a node corresponding to a window/display is
 selected in the display tree.
 This sets that display to be the current or active one.
 */
void
display_tree_display_child_select(GtkWidget *item, displayd *display, ggobid *gg)
{

  if(gg->display_tree.tree == NULL) {
    return;
  }

  if(display != NULL) {
    /* Top-level of tree, so set that to be the current display. */
    display_set_current(display, gg);
  }  
}

/*
 Called when a node corresponding to an splot is
 selected in the display tree.
 This makes the corresponding splot the current
 one.
 */
void
display_tree_splot_child_select(GtkWidget *item, splotd *plot)
{
  ggobid *gg = GGobiFromSPlot(plot);

  if( &(plot->displayptr->ggobi->display_tree) == NULL) {
    return;
  }

  if(plot != NULL) {
    /* Set the associated splot to be the current one. */
   splot_set_current(plot, on, gg);
  }  
}
