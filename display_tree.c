/*-- display_tree.c --*/
#include <stdlib.h>
#include <string.h>

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

void
update_display_tree_plots_by_variable(DisplayTree *tree, gint whichVar, datad *d, splotd *sp, ggobid *gg, GtkWidget *w)
{
    displayd *dpy = sp->displayptr;
    int i, n;

    GtkWidget *subTree;
    GList *kids;

    n = g_list_length(gg->displays);
    for(i =0; i < n; i++) {
        dpy = (displayd *) g_list_nth_data(gg->displays, i);
	if(sp->displayptr == dpy) 
	    break;
    }

    if(i == n) {
	return;/*XXX give a warning message that this should never have happened! */
    }

      /* 
        Now we know the particular element of the tree corresponding
        to the display. So, this is a container with a single child
        which is a GtkTreeItem object. We ask it for its subtree,
        then we get its children. And now we loop over the splotd's
        in the display and update the label in the corresponding
        child in this containers children. Each child is a GtkTreeItem.
       */    
    kids = gtk_container_children(GTK_CONTAINER(tree->tree));
    subTree = (GtkWidget*) g_list_nth_data(kids, i);
    
    kids = gtk_container_children(GTK_CONTAINER(GTK_TREE_ITEM_SUBTREE(GTK_TREE_ITEM(subTree))));


    n = g_list_length(dpy->splots);
    for(i = 0; i < n; i++) {
	GtkWidget *tmp;
        splotd *sp;
	char *lab;
        sp = (splotd*) g_list_nth_data(dpy->splots, i);
        tmp = (GtkWidget *) g_list_nth_data(kids, i);
        tmp = (GtkWidget *) g_list_nth_data(gtk_container_children(GTK_CONTAINER(tmp)), 0);
	lab = splot_tree_label(sp, i, dpy->displaytype, dpy->d, gg);
        gtk_label_set_text(GTK_LABEL(tmp), lab);
	g_free(lab);
    }
}


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

 GtkWidget *tree, *sw;
 GtkWidget *plot_tree_window;
 gint numItems;

 /* If this is the first time we have called this, 
    create it from scratch. Otherwise, we have to 
    update the display. The easiest way is to remove
    the entire contents of the tree and start rebuilding
    with the current model.
  */
 if(gg->display_tree.tree == NULL) {
  plot_tree_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(plot_tree_window), "GGobi Displays");
  gtk_widget_set_usize(plot_tree_window, 250, 300);

  gtk_signal_connect (GTK_OBJECT(gg->main_window),
    "select_variable",
    (GtkSignalFunc) update_display_tree_plots_by_variable,
    (gpointer) &gg->display_tree);

 } else {
   g_printerr("The display tree is already visible. It should be correct!\n");
   return(NULL);
 }

  tree = gtk_tree_new();

  numItems = 0;
  for (dlist = gg->displays; dlist; dlist = dlist->next, numItems++) {
    GtkWidget *sub;
    display = (displayd *) dlist->data;
    sub = display_add_tree(display, numItems, tree, gg);
  }

 sw = gtk_scrolled_window_new(NULL, NULL);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), tree);
 gtk_container_add(GTK_CONTAINER( plot_tree_window ), sw);
 gtk_widget_show_all(plot_tree_window);


  gtk_signal_connect (GTK_OBJECT (plot_tree_window), "delete_event",
                      GTK_SIGNAL_FUNC (display_tree_delete_cb), gg);

 gg->display_tree.tree = tree;
 gg->display_tree.numItems = numItems;
 gg->display_tree.window = plot_tree_window;

 return(GTK_TREE( tree ));
}


GtkWidget *
display_add_tree(displayd *display, gint entry, GtkWidget *tree, ggobid *gg)
{
  GtkWidget *item, *subTree;
  gchar *label;
  if(tree == NULL)
    return(NULL);

  item = gtk_tree_item_new_with_label(label = display_tree_label(display));
  g_free(label);
  gtk_signal_connect (GTK_OBJECT(item), "select",
                      GTK_SIGNAL_FUNC(display_tree_display_child_select),
                      display);
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
 gtk_widget_destroy (gg->display_tree.window);
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
  gchar *buf;
  
  tree = gtk_tree_new();

/*
gtk_signal_connect (GTK_OBJECT(tree), "select_child",
                             GTK_SIGNAL_FUNC(display_tree_child_select), display);
*/
      /* Here do the plots within the display. */
  for (slist = display->splots; slist ; slist = slist->next, ctr++) {
    sp = (splotd *) slist->data;
    /*-- buf is allocated in splot_tree_label, but freed here --*/
    buf = splot_tree_label (sp, ctr, display->displaytype, d, gg);
    item = gtk_tree_item_new_with_label (buf);
    if(buf) 
      g_free (buf);

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
gchar * 
display_tree_label(displayd *display)
{
 gchar * val, *tmp;

 if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display))
    val = (gchar *) gtk_display_tree_label(display);
 else
  switch(display->displaytype) {
    case scatterplot:
      val = "Scatterplot";
      break;
    case scatmat:
      val = "Scatterplot Matrix";
      break;

    case unknown_display_type:
      val = (gchar *) NULL;
      break;
    default:
      break;
  }

  if(val) {
      tmp = g_malloc(sizeof(gchar *) * (strlen(val) + strlen(display->d->name + 3 + 1)));
      sprintf(tmp, "%s (%s)", val, display->d->name);
  } else
      tmp = val;

 return(tmp);
}

/*
  Computes the label for an element in the tree for the specified
  splot, in position `ctr' and an element in the container of 
  type `type'.
 */
gchar *
splot_tree_label(splotd *splot, gint ctr, enum displaytyped type,  datad *d, ggobid *gg)
{
  gchar *buf = "";
  gint n;
  vartabled *vt, *vtx, *vty;

  if(GTK_IS_GGOBI_EXTENDED_SPLOT(splot)) {
      return(GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(splot)->klass)->tree_label(splot, d, gg));
  }

  switch (type) {
    case scatterplot:
    {
     displayd *display = (displayd *) splot->displayptr; 
     cpaneld *cpanel = &display->cpanel;

     switch (cpanel->projection) {
        case P1PLOT:
        case TOUR1D:
          vt = vartable_element_get (splot->p1dvar, d);
          n = strlen (vt->collab);
          buf = (gchar*) g_malloc(n* sizeof (gchar*));
          sprintf(buf, "%s", vt->collab);
        break;

        case XYPLOT:
          vtx = vartable_element_get (splot->xyvars.x, d);
          vty = vartable_element_get (splot->xyvars.y, d);

          n = strlen (vtx->collab) + strlen (vty->collab) + 5;
          buf = (gchar*) g_malloc (n * sizeof (gchar*));
          sprintf (buf, "%s v %s", vtx->collab, vty->collab);
        break;

        case TOUR2D:
          n = strlen ("in grand tour");
          buf = (gchar*) g_malloc (n * sizeof (gchar*));
          sprintf (buf, "%s", "in grand tour");
        break;

        case COTOUR:
          n = strlen ("in correlation tour");
          buf = (gchar*) g_malloc (n * sizeof (gchar*));
          sprintf (buf, "%s", "in correlation tour");
        break;
        default:
        break;
     }
    }
    break;
    case scatmat:
      vtx = vartable_element_get (splot->xyvars.x, d);
      vty = vartable_element_get (splot->xyvars.y, d);

      n = strlen (vtx->collab) + strlen (vty->collab) + 5;
      buf = (gchar*) g_malloc (n * sizeof (gchar*));
      sprintf (buf, "%s v %s", vtx->collab, vty->collab);
    break;
    case unknown_display_type:
    break;
    default:
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
gint
tree_display_entry_remove(displayd *display, GtkWidget *tree, ggobid *gg)
{
 GList *dlist;
 displayd *tmp;
 gint which = 0;

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
gint  
tree_display_entry_remove_by_index(gint which, GtkWidget *tree)
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
display_tree_display_child_select(GtkWidget *item, displayd *display)
{
  ggobid *gg = GGobiFromDisplay (display);

  if(gg->display_tree.tree == NULL) {
    return;
  }

  if(display != NULL) {
    splotd *sp = gg->current_splot;
    displayd *spd = (displayd *) sp->displayptr;
    /* Top-level of tree, so set that to be the current display. */
    display_set_current(display, gg);

    /* Make sure the current splot is in the current display */
    if (spd != display) {
      sp = (splotd *) g_list_nth_data (display->splots, 0);
      GGOBI(splot_set_current_full) (display, sp, gg);
    }
  }

  gtk_widget_show(GTK_GGOBI_WINDOW_DISPLAY(display)->window);  
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
