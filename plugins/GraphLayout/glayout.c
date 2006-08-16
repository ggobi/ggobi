#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "glayout.h"

#include "GGStructSizes.c"

void       close_glayout_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_glayout_window(ggobid *gg, PluginInstance *inst);
void       show_glayout_window (GtkAction *action, PluginInstance *inst);

gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  static GtkActionEntry entry = {
	"GraphLayout", NULL, "Graph _Layout", NULL, "Layout graphs using graphviz", 
		G_CALLBACK (show_glayout_window)
  };  
  GGOBI(addToolAction)(&entry, (gpointer)inst, gg);
  return(true);
}


void
show_glayout_window (GtkAction *action, PluginInstance *inst)
{
  glayoutd *gl;

  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    gl = (glayoutd *) g_malloc (sizeof (glayoutd));

    glayout_init (gl);
    inst->data = gl;

    create_glayout_window (inst->gg, inst);
    g_object_set_data(G_OBJECT (gl->window), "glayoutd", gl);

  } else {
    gl = (glayoutd *) inst->data;
    if (gl->window)
      gtk_widget_show_now ((GtkWidget*) gl->window);
  }
}

glayoutd *
glayoutFromInst (PluginInstance *inst)
{
  glayoutd *gl = (glayoutd *) inst->data;
  return gl;
}

static void
glayout_datad_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  glayoutd *gl = glayoutFromInst (inst);
  GGobiData *d;
  const gchar *clname = gtk_widget_get_name(GTK_WIDGET(gtk_tree_selection_get_tree_view(tree_sel)));
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  if (!gtk_tree_selection_get_selected(tree_sel, &model, &iter))
	  return;
  gtk_tree_model_get(model, &iter, 1, &d, -1);
  if (strcmp (clname, "nodeset") == 0)
	  gl->dsrc = d;
  else if (strcmp (clname, "edgeset") == 0)
	  gl->e = d;
  /* Don't free either string; they're just pointers */
}
static gint
glayout_tree_view_datad_added_cb (ggobid *gg, GGobiData *d, GtkWidget *tree_view)
{
  GtkWidget *swin = (GtkWidget *)
    g_object_get_data(G_OBJECT (tree_view), "datad_swin");
  const gchar *clname = gtk_widget_get_name (GTK_WIDGET(tree_view));
  GtkTreeIter iter;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
  
  if (strcmp (clname, "nodeset") == 0 && d->rowIds != NULL) 
  {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, 1, d, -1);
  }
  if (strcmp (clname, "edgeset") == 0 && d->edge.n > 0)
  {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, 1, d, -1);
  }
  
  gtk_widget_show_all (swin);

  return false;
}

static const gchar *const neato_model_lbl[] = {
  "Shortest path", "Circuit resistance", "Subset"};
GtkWidget *
create_glayout_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook, *label, *frame, *vbox, *btn;
  GtkWidget *hb, *entry;
#if defined HAVE_LIBGVC || defined GRAPHVIZ
  GtkWidget *hscale, *vb, *opt, *apply_btn, *varnotebook;
  GtkObject *adj;
#endif
  GtkTooltips *tips = gtk_tooltips_new ();
  /*-- for lists of datads --*/
  gchar *tree_view_titles[2] = {"node sets", "edge sets"};
  GGobiData *d;
  GtkWidget *hbox, *swin, *tree_view;
  GSList *l;
  GtkListStore *model;
  GtkTreeIter iter;
  glayoutd *gl = glayoutFromInst (inst);

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gl->window = window;

  gtk_window_set_title(GTK_WINDOW(window), "Graph Layout");
  g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (close_glayout_window), inst);

  main_vbox = gtk_vbox_new (FALSE,1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook),
    GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

/*-- "Specify datasets" list widgets --*/
/*-- this is exactly the same code that appears in ggvis.c --*/

  hbox = gtk_hbox_new (true, 10);
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
		G_CALLBACK(glayout_datad_set_cb), inst);
  gtk_widget_set_name (GTK_WIDGET(tree_view), "nodeset");
  g_object_set_data(G_OBJECT (tree_view), "datad_swin", swin);
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(glayout_tree_view_datad_added_cb), GTK_OBJECT (tree_view));
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
  gtk_box_pack_start (GTK_BOX (hbox), swin, true, true, 2);

/*
 * edge sets
*/
  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new(2, G_TYPE_STRING, GGOBI_TYPE_DATA);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, tree_view_titles, 1, true, GTK_SELECTION_SINGLE,
		G_CALLBACK(glayout_datad_set_cb), inst);
  gtk_widget_set_name (GTK_WIDGET(tree_view), "edgeset");
  g_object_set_data(G_OBJECT (tree_view), "datad_swin", swin);
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(glayout_tree_view_datad_added_cb), GTK_OBJECT (tree_view));
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

  label = gtk_label_new ("Specify datasets");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

/*
 * radial tab
*/
  frame = gtk_frame_new ("Radial layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  /*-- Label of the center node: passive display --*/
  hb = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Center node"),
    false, false, 2);
  entry = gtk_entry_new ();
  gtk_editable_set_editable (GTK_EDITABLE (entry), false);
  g_object_set_data(G_OBJECT(window), "CENTERNODE", entry);
  if (gl->dsrc)
    gtk_entry_set_text (GTK_ENTRY (entry),
      (gchar *) g_array_index (gl->dsrc->rowlab, gchar *, 0));
  g_signal_connect (G_OBJECT(gg),
    "sticky_point_added", G_CALLBACK(radial_center_set_cb), inst);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), entry,
    "To reset the center node, use sticky identification in ggobi", 
    NULL);
  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  /*-- checkbox: automatically update the center node when
    responding to identify events --*/
  btn = gtk_check_button_new_with_mnemonic("_Automatically update layout when center node is reset");  
  g_signal_connect (G_OBJECT (btn), "toggled",
    G_CALLBACK (radial_auto_update_cb), (gpointer) inst);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(btn),
    gl->radialAutoUpdate);
  gtk_widget_set_name (btn, "RADIAL:autoupdate");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Automatically update the layout when a new sticky label is assigned in Identify mode, or wait until the apply button is pressed", 
    NULL);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  /*-- checkbox: create new datad and display: this has to
    be on and insensitive initially.  It should become sensitive
    after the first layout has been generated. --*/
  btn = gtk_check_button_new_with_mnemonic("_Create new data and display when updating layout");
  g_signal_connect (G_OBJECT (btn), "toggled",
    G_CALLBACK (radial_new_data_cb), (gpointer) inst);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(btn),
    gl->radialNewData);
  gtk_widget_set_sensitive (btn, false);
  gtk_widget_set_name (btn, "RADIAL:newdata");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Create new data and display when pressing the apply button, or re-use existing resources", 
    NULL);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (radial_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  /*-- highlight the edges connected to nodes with sticky labels --*/
/*
  btn = gtk_button_new_with_label ("highlight edges");
  g_signal_connect (G_OBJECT (btn), "clicked",
                      G_CALLBACK (highlight_edges_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);
*/

  label = gtk_label_new_with_mnemonic ("_Radial");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);
  /*-- --*/

/* 
 * neato tab
*/
  frame = gtk_frame_new ("Neato layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
#if defined HAVE_LIBGVC || defined GRAPHVIZ

  hbox = gtk_hbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(hbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), hbox);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 0);

/*
Add an option:  Model either 'circuit resistance' or 'shortest path'
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  label = gtk_label_new_with_mnemonic ("_Model:");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);
  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), opt);
/*
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Fix one of the axes during plot cycling or let them both float", NULL);
*/
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, (gchar**) neato_model_lbl, G_N_ELEMENTS(neato_model_lbl),
    G_CALLBACK(neato_model_cb), inst);

  /*-- neato scale --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

  label = gtk_label_new_with_mnemonic ("_Dimension:");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vb), label, false, false, 3);

  adj = gtk_adjustment_new ((gfloat)gl->neato_dim, 2.0, 11.0, 1.0, 1.0, 1.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (neato_dim_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), opt);
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);

  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_scale_set_value_pos (GTK_SCALE(hscale), GTK_POS_BOTTOM);
  gtk_scale_set_draw_value (GTK_SCALE(hscale), TRUE);

  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_box_pack_start (GTK_BOX (vb), hscale, false, false, 3);
  /*-- --*/

  apply_btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_widget_set_name (apply_btn, "neato");
  g_signal_connect (G_OBJECT (apply_btn), "clicked",
                      G_CALLBACK (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), apply_btn, false, false, 3);


  /*-- second child of the neato hbox, to contain the list of variables --*/
  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 0);

  btn = gtk_check_button_new_with_mnemonic ("Use _edge length");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Have neato use edge length in determining node positions, and use the selected variable as a source of lengths.  Edge lengths must be >= 1.0.",
    NULL);
  g_signal_connect (G_OBJECT (btn), "toggled",
    G_CALLBACK (neato_use_edge_length_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  /*-- include only edge sets.  --*/
  varnotebook = create_variable_notebook (vbox,
    GTK_SELECTION_SINGLE, all_vartypes, edgesets_only,
    G_CALLBACK(NULL), (gpointer) NULL, inst->gg);
  g_object_set_data(G_OBJECT(apply_btn), "notebook", varnotebook);
#else
  gtk_container_add (GTK_CONTAINER(frame), gtk_label_new ("Not enabled"));
#endif

  label = gtk_label_new_with_mnemonic ("_Neato");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);


/*
 * Dot tab
*/
  frame = gtk_frame_new ("Dot layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

#if defined HAVE_LIBGVC || defined GRAPHVIZ
  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_widget_set_name (btn, "dot");
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);
#else
  gtk_container_add (GTK_CONTAINER(frame), gtk_label_new ("Not enabled"));
#endif

  label = gtk_label_new_with_mnemonic ("_Dot");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

/*
 * fdp tab -- disable for a while.  It crashes on snetwork.xml, and
 * I don't know why.
*/
/*
  frame = gtk_frame_new ("fdp layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

#if defined HAVE_LIBGVC || GRAPHVIZ
  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_widget_set_name (btn, "fdp");
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);
#else
  gtk_container_add (GTK_CONTAINER(frame), gtk_label_new ("Not enabled"));
#endif

  label = gtk_label_new_with_mnemonic ("_FDP");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
 */

/*
 * twopi tab
*/
/* This doesn't work as well as my 'radial' in any case; disable.
  frame = gtk_frame_new ("twopi layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

#if defined HAVE_LIBGVC || GRAPHVIZ
  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_widget_set_name (btn, "twopi");
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);
#else
  gtk_container_add (GTK_CONTAINER(frame), gtk_label_new ("Not enabled"));
#endif

  label = gtk_label_new_with_mnemonic ("_Twopi");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
 */

/*
 * circo tab
*/
  frame = gtk_frame_new ("circo layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

#if defined HAVE_LIBGVC || defined GRAPHVIZ
  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_widget_set_name (btn, "circo");
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);
#else
  gtk_container_add (GTK_CONTAINER(frame), gtk_label_new ("Not enabled"));
#endif

  label = gtk_label_new_with_mnemonic ("_Circo");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

  gtk_widget_show_all (window);

  return(window);
}


void close_glayout_window(GtkWidget *w, PluginInstance *inst)
{
  /*
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(glayout_clist_datad_added_cb), GTK_OBJECT (clist));
  */
  if (inst->data) {
    glayoutd *gl = glayoutFromInst (inst);
    gtk_widget_destroy (gl->window);
  }

  inst->data = NULL;
}

void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  if (inst->data) {
    glayoutd *gl = glayoutFromInst (inst);
    /* I don't remember what this code is for -- dfs
    g_signal_handlers_disconnect_by_func(G_OBJECT(inst->data),
      G_CALLBACK (close_glayout_window), inst);
    */
    gtk_widget_destroy (gl->window);
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
