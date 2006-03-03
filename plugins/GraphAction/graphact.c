#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "graphact.h"

#include "GGStructSizes.c"

void       close_graphact_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_graphact_window(ggobid *gg, PluginInstance *inst);
void       show_graphact_window (GtkAction *action, PluginInstance *inst);


gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  static GtkActionEntry entry = {
	"GraphAction", NULL, "Graph _Operations", NULL, "Perform misc operations on a graph", 
		G_CALLBACK (show_graphact_window)
  };
  GGOBI(addToolAction)(&entry, (gpointer)inst, gg);
  return(true);
}


void
show_graphact_window (GtkAction *action, PluginInstance *inst)
{
  graphactd *ga;

  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    ga = (graphactd *) g_malloc (sizeof (graphactd));

    graphact_init (ga);
    inst->data = ga;

    create_graphact_window (inst->gg, inst);
    g_object_set_data(G_OBJECT (ga->window), "graphactd", ga);

  } else {
    ga = (graphactd *) inst->data;
    gtk_widget_show_now ((GtkWidget*) ga->window);
  }
}

graphactd *
graphactFromInst (PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;
  return ga;
}

static void
graphact_datad_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  graphactd *ga = graphactFromInst (inst);
  GGobiData *d, *e;
  const gchar *clname = gtk_widget_get_name(GTK_WIDGET(gtk_tree_selection_get_tree_view(tree_sel)));
  gboolean changed = false;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  if (!gtk_tree_selection_get_selected(tree_sel, &model, &iter))
	  return;
  gtk_tree_model_get(model, &iter, 1, &d, -1);
  if (strcmp (clname, "nodeset") == 0) {
	  changed = ga->d != d;
	  ga->d = d;
  } else if (strcmp (clname, "edgeset") == 0) {
	  changed = ga->e != e;
	  ga->e = e;
  }
  
  #if 0
  gtk_clist_get_text (GTK_CLIST (cl), row, 0, &dname);
  for (l = gg->d; l; l = l->next) {
    d = l->data;
    if (strcmp (d->name, dname) == 0) {
      if (strcmp (clname, "nodeset") == 0) {
        changed = changed || (ga->d != d);
        ga->d = d;
      } else if (strcmp (clname, "edgeset") == 0) {
        changed = changed || (ga->e != d);
        ga->e = d;
      }
      break;
    }
  }
  #endif

  if (ga->d != NULL && ga->e != NULL) {
    init_edge_vectors (changed, inst);
  }
}

static void 
graphact_tree_view_datad_added_cb (ggobid *gg, GGobiData *d, GtkWidget *tree_view)
{
  GtkWidget *swin = (GtkWidget *)
    g_object_get_data(G_OBJECT (tree_view), "datad_swin");
  const gchar *clname = gtk_widget_get_name (GTK_WIDGET(tree_view));
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
  
  if ((strcmp (clname, "nodeset") == 0 && d->rowIds != NULL) ||
  		(strcmp (clname, "edgeset") == 0 && d->edge.n > 0)) 
  {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, 1, d, -1);
  }

  gtk_widget_show_all (swin);
}

static const gchar *const neighborhood_depth_lbl[] = {
  " 1 ", " 2 "};
GtkWidget *
create_graphact_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook, *label, *frame, *vbox, *btn, *opt;
  GtkTooltips *tips = gtk_tooltips_new ();
  /*-- for lists of datads --*/
  gchar *tree_view_titles[2] = {"node sets", "edge sets"};
  GGobiData *d;
  GtkWidget *hbox, *swin, *tree_view;
  GSList *l;
  graphactd *ga = graphactFromInst (inst); 
  GtkTreeIter iter;
  GtkListStore *model;

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  ga->window = window;

  gtk_window_set_title(GTK_WINDOW(window), "Graph operations");
  g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (close_graphact_window), inst);

  main_vbox = gtk_vbox_new (FALSE,1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook),
    GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

/*-- "Specify datasets" list widgets --*/
/*-- this is exactly the same code that appears in ggvis.c --*/

  hbox = gtk_hbox_new (false, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

/*
 * node sets
*/
  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	model = gtk_list_store_new(2, G_TYPE_STRING, GGOBI_TYPE_DATA);
	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	populate_tree_view(tree_view, tree_view_titles, 1, true, GTK_SELECTION_SINGLE,
		G_CALLBACK(graphact_datad_set_cb), inst);
  gtk_widget_set_name (GTK_WIDGET(tree_view), "nodeset");
  g_object_set_data(G_OBJECT (tree_view), "datad_swin", swin);
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(graphact_tree_view_datad_added_cb), tree_view);
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d->rowIds != NULL) {  /*-- node sets --*/
		gtk_list_store_append(model, &iter);
		gtk_list_store_set(model, &iter, 0, d->name, 1, d, -1);
    }
  }
  select_tree_view_row(tree_view, 0);
  
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (hbox), swin, false, false, 2);

/*
 * edge sets
*/
  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new(2, G_TYPE_STRING, GGOBI_TYPE_DATA);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, tree_view_titles+1, 1, true, GTK_SELECTION_SINGLE,
		G_CALLBACK(graphact_datad_set_cb), inst);
  gtk_widget_set_name (GTK_WIDGET(tree_view), "edgeset");
  g_object_set_data(G_OBJECT (tree_view), "datad_swin", swin);
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(graphact_tree_view_datad_added_cb), tree_view);
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d->edge.n != 0) {  /*-- edge sets --*/
      gtk_list_store_append(model, &iter);
	  gtk_list_store_set(model, &iter, 0, d->name, 1, d, -1);
    }
  }
  select_tree_view_row(tree_view, 0);
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (hbox), swin, true, true, 2);

  label = gtk_label_new_with_mnemonic ("Specify _datasets");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

  /*-- --*/

  /*-- Thin the graph in different ways --*/
  frame = gtk_frame_new ("Thin the graph");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);
  btn = gtk_button_new_with_mnemonic ("Shadow _leaves");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Recursively shadow brush leaf nodes and edges", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ga_leaf_hide_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);

  btn = gtk_button_new_with_mnemonic ("Shadow _orphans");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Shadow brush nodes without any edges that are both included and not shadowed",
    NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ga_orphans_hide_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);


  btn = gtk_button_new_with_mnemonic ("Show _all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Show all nodes and edges", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ga_nodes_show_cb), inst);  /*-- show all nodes --*/
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  label = gtk_label_new_with_mnemonic ("_Thin");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
  /*-- --*/


  /*-- --*/
  frame = gtk_frame_new ("Find neighbors");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);
/*
 checkbox:  Neighborhood finder on or off
 button: restore all nodes and edges
 option menu taking the values {1,2} for the radius of the path to be reported
 possibilities:
   button: sticky label everything showing
   button: remove all sticky labels
   color:  draw the rest of the graph in a ghost color, just a
     few shades lighter/darker than the background color.  That
     would necessitate the use of a custom colorscheme.
*/

  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);

  btn = gtk_check_button_new_with_mnemonic ("Show _neighbors");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "To display only a node and its neighbors, turn this on, select 'Identify' in ggobi, and double-click to make a label 'sticky.'", NULL);
  g_signal_connect (G_OBJECT (btn), "toggled",
    G_CALLBACK (show_neighbors_toggle_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);

  btn = gtk_button_new_with_mnemonic ("Show _all");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Show all nodes and edges", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ga_nodes_show_cb), inst);  /*-- show all nodes --*/
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);


  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);
  label = gtk_label_new_with_mnemonic ("_Depth:");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_box_pack_start (GTK_BOX (hbox), label, false, false, 0);
  opt = gtk_combo_box_new_text();
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), opt);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), opt,
    "Select the size of the selected node's neighborhood to show; ie, the number of steps from the node.", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), opt, false, false, 0);
  populate_combo_box (opt, (gchar**) neighborhood_depth_lbl, G_N_ELEMENTS(neighborhood_depth_lbl),
    G_CALLBACK(neighborhood_depth_cb), inst);

  label = gtk_label_new_with_mnemonic ("_Neighbors");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
  /*-- --*/


  /*-- Tidy the graph in different ways --*/
  frame = gtk_frame_new ("Tidy the graph");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);
  btn = gtk_button_new_with_mnemonic ("_Shadow orphaned edges");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Shadow brush edges connected to shadowed nodes", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ga_edge_tidy_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);

  label = gtk_label_new_with_mnemonic ("_Tidy");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
  /*-- --*/


  gtk_widget_show_all (window);

  return(window);
}


void close_graphact_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  if (inst->data) {
  graphactd *ga = graphactFromInst (inst); 
    /* I don't remember what this line is for -- dfs
    g_signal_handlers_disconnect_by_func(G_OBJECT(inst->data),
      G_CALLBACK (close_graphact_window), inst);
    */
    gtk_widget_destroy (ga->window);
  }
}

gint
visible_set (glong *visible, GGobiData *d)
{
  gint i, m;
  gint nvisible = 0;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    if (!d->hidden.els[i]) {
      visible[nvisible++] = i;
    }
  }

  return nvisible;
}
