#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

#include "GGStructSizes.c"

void       close_ggvis_window(GtkWidget *w, PluginInstance *inst);
void       create_ggvis_window(ggvisd *ggv, PluginInstance *inst);
void       show_ggvis_window (GtkAction *action, PluginInstance *inst);

gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  static GtkActionEntry entry = {
    "GGVis", NULL, "ggvis (MDS)", NULL, "Multi-dimensional scaling tool", 
    G_CALLBACK (show_ggvis_window)
  };
  GGOBI(addToolAction)(&entry, (gpointer)inst, gg);
  return(true);
}

static const gchar *menu_ui =
"<ui>"
"	<menubar>"
"		<menu action='View'>"
"			<menuitem action='ShepardPlot'/>"
"		</menu>"
"		<menu action='Reset'>"
"			<menuitem action='ReinitLayout'/>"
"			<menuitem action='ScrambleLayout'/>"
"			<menuitem action='ResetMDSParameters'/>"
"		</menu>"
#if 0
"		<menu action='Help'>"
"			<menuitem action='MDSBackground'/>"
"			<menuitem action='MDSControls'/>"
"			<menuitem action='KruskalShepardFormula'/>"
"			<menuitem action='TorgersonGowerFormula'/>"
"		</menu>"
#endif
"	</menubar>"
"</ui>";

static GtkActionEntry entries[] = {
  { "View", NULL, "_View" },
  { "ShepardPlot", NULL, "_Shepard Plot", "<control>S", "Display a Shepard Plot", 
    G_CALLBACK(create_shepard_data_cb)
  },
  { "Reset", NULL, "_Reset" },
  { "ReinitLayout", GTK_STOCK_REFRESH, "Reinit _Layout", "<control>L", "Reinitialize the layout",
    G_CALLBACK(mds_reinit_cb)
  },
  { "ScrambleLayout", NULL, "_Scramble Layout", "<control>A", "Scramble the layout",
    G_CALLBACK(mds_scramble_cb)
  },
  { "ResetMDSParameters", NULL, "Reset MDS _Parameters", "<control>P", "Reset the MDS Parameters",
    G_CALLBACK(mds_reset_params_cb)
  },
};

static const gchar *const dsource_lbl[] = {
  "Unweighted graph dist", 
  "D from variable"
};
static const gchar *const metric_lbl[] = {"Metric MDS", "Nonmetric MDS"};
static const gchar *const kruskal_lbl[] = {"Kruskal", "Classic"};
static const gchar *const groups_lbl[] = {
  "Use _all distances",
  "Use distances _within groups",
  "Use distances _between groups",
};
static const gchar *const anchor_lbl[] = {
  "_No anchor",
  "_Scaled",
  "_Fixed",
};
static const gchar *const constrained_lbl[] = {
  "No variables frozen",
  "First variable frozen",
  "First two variables frozen"};

void
show_ggvis_window (GtkAction *action, PluginInstance *inst)
{
  GSList *l;
  GGobiData *d;
  gboolean ok = false;

  /* Before doing anything, make sure there is input data, and that
     it includes an edge set */
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("ggvis: can't initialize without data\n");
    return;
  }
  for (l = inst->gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d->edge.n > 0) {
      ok = true;
      break;
    }
  }
  if (!ok) {
    g_printerr ("ggvis: need an edgeset to define pairwise distances.\n");
    return;
  }

  if (inst->data == NULL) {
    ggvisd *ggv = (ggvisd *) g_malloc (sizeof (ggvisd));

    ggvis_init (ggv, inst->gg);
    ggv_histogram_init (ggv, inst->gg);

    create_ggvis_window (ggv, inst);

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);  /* ie, window */
  }
}

ggvisd *
ggvisFromInst (PluginInstance *inst)
{
  GtkWidget *window = (GtkWidget *) inst->data;
  ggvisd *ggv = NULL;

  if (window)
    ggv = (ggvisd *) g_object_get_data(G_OBJECT(window), "ggvisd");

  return ggv;
}

void ggvis_scale_set_default_values (GtkScale *scale)
{
  gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_digits (scale, 2);
  gtk_scale_set_value_pos (scale, GTK_POS_BOTTOM);
  gtk_scale_set_draw_value (scale, TRUE);
}

static void
ggv_datad_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  gchar *dname;
  GGobiData *d;
  GSList *l;
  const gchar *clname = gtk_widget_get_name (GTK_WIDGET(gtk_tree_selection_get_tree_view(tree_sel)));
  gint k;
  GtkTreeModel *model;
  GtkTreeIter iter;
  
  if (!gtk_tree_selection_get_selected(tree_sel, &model, &iter))
    return;
  
  gtk_tree_model_get(model, &iter, 0, &dname, -1);
  
  for (l = gg->d; l; l = l->next) {
    d = l->data;
    if (strcmp (d->name, dname) == 0) {
      if (strcmp (clname, "nodeset") == 0) {
        ggv->dsrc = d;

        /* Once dsrc has been defined, the anchor groups can be initialized */
        vectorb_realloc (&ggv->anchor_group, d->nclusters);
        for (k=0; k<d->nclusters; k++)
          ggv->anchor_group.els[k] = false;

      } else if (strcmp (clname, "edgeset") == 0) {
        ggv->e = d;
      }
      break;
    }
  }
  /* Don't free either string; they're just pointers */
}
static void 
ggv_tree_view_datad_added_cb (ggobid *gg, GGobiData *d, GtkWidget *tree_view)
{
  GtkWidget *swin;
  const gchar *clname;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
  GtkTreeIter iter;

  if (tree_view == NULL)
    return;

  swin = (GtkWidget *)
    g_object_get_data(G_OBJECT (tree_view), "datad_swin");
  clname = gtk_widget_get_name (tree_view);

  if (strcmp (clname, "nodeset") == 0 && d->rowIds != NULL) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, -1); 
  }
  if (strcmp (clname, "edgeset") == 0 && d->edge.n > 0) {
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, -1);
  }

  gtk_widget_show_all (swin);
}

void
create_ggvis_window(ggvisd *ggv, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *vbox_params;
  GtkWidget *notebook, *opt, *metric_opt;
  GtkWidget *label, *frame, *btn, *vbox, *hbox, *vb, *hscale, *table, *hb;
  GtkWidget *radio1, *radio2;
  GtkWidget *radio = NULL;
  GSList *group;
  GtkObject *adj, *Dtarget_adj, *isotonic_mix_adj;
  gint i, top;
  GtkWidget *menubar;
  /*-- for lists of datads --*/
  gchar *titles[] = {"node sets", "edge sets"};
  GGobiData *d;
  ggobid *gg = inst->gg;
  GtkWidget *swin, *tree_view;
  GSList *l;
  GtkListStore *model;
  GtkUIManager *manager;
  GtkActionGroup *actions;

  ggv->tips = gtk_tooltips_new ();

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT (window), "ggvisd", ggv);
  inst->data = window;  /*-- or this could be the ggvis structure --*/

  gtk_window_set_title(GTK_WINDOW(window),
    "ggvis: multidimensional scaling");
  g_signal_connect (G_OBJECT (window), "destroy",
    G_CALLBACK (close_ggvis_window), inst);

  main_vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  /* main menu bar */
  manager = gtk_ui_manager_new();
  actions = gtk_action_group_new("ggvis");
  gtk_action_group_add_actions(actions, entries, G_N_ELEMENTS(entries), inst);
  gtk_ui_manager_insert_action_group(manager, actions, 0);
  menubar = create_menu_bar (manager, menu_ui, window);
  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, false, false, 0);

/*-- notebook for datads, distance matrix, run controls --*/

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

/*-- "Specify datasets" list widgets --*/

  hbox = gtk_hbox_new (true, 10);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

/*
 * node sets
*/
  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  model = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, titles, 1, true, GTK_SELECTION_SINGLE, 
    G_CALLBACK(ggv_datad_set_cb), inst);

  gtk_widget_set_name (GTK_WIDGET(tree_view), "nodeset");
  g_object_set_data(G_OBJECT (tree_view), "datad_swin", swin);
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(ggv_tree_view_datad_added_cb), 
    (gpointer) tree_view);
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d->rowIds != NULL) {  /*-- node sets --*/
      GtkTreeIter iter;
      gtk_list_store_append(GTK_LIST_STORE(model), &iter);
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, -1);
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

  model = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, &titles[1], 1, true, 
    GTK_SELECTION_SINGLE, 
    G_CALLBACK(ggv_datad_set_cb), inst); 
  gtk_widget_set_name (GTK_WIDGET(tree_view), "edgeset");
  
  g_object_set_data(G_OBJECT (tree_view), "datad_swin", swin);
  g_signal_connect (G_OBJECT (gg), "datad_added",
    G_CALLBACK(ggv_tree_view_datad_added_cb), GTK_OBJECT (tree_view));
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    if (d->edge.n != 0) {  /*-- edge sets --*/
      GtkTreeIter iter;
      gtk_list_store_append(GTK_LIST_STORE(model), &iter);
      gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, -1);
    }
  }
  select_tree_view_row(tree_view, 0);
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (hbox), swin, true, true, 2);

  label = gtk_label_new_with_mnemonic ("_Datasets");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

  /*-- Task controls --*/

  vbox = gtk_hbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  frame = gtk_frame_new ("Task Definition");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  radio1 = gtk_radio_button_new_with_mnemonic (NULL, "_Dissimilarity analysis");
  gtk_widget_set_name (GTK_WIDGET(radio1), "MDS");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), radio1,
    "Perform multidimensional scaling (MDS) for the purpose of dissimilarity analysis; dissimilarities (distances) are provided as an edge variable.",
    NULL);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_mnemonic (group, "Graph _Layout");
  gtk_widget_set_name (GTK_WIDGET(radio2), "GRAPH_LAYOUT");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), radio2,
    "Perform multidimensional scaling (MDS) for the purpose of laying out a graph.",
    NULL);

  GTK_TOGGLE_BUTTON(radio1)->active = (ggv->mds_task == DissimAnalysis);
  GTK_TOGGLE_BUTTON(radio2)->active = (ggv->mds_task == GraphLayout);

  g_signal_connect (G_OBJECT (radio1), "toggled",
    G_CALLBACK (ggv_task_cb), inst);
  g_signal_connect (G_OBJECT (radio2), "toggled",
    G_CALLBACK (ggv_task_cb), inst);

  gtk_box_pack_start (GTK_BOX (vb), radio1, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vb), radio2, true, true, 2);

  frame = gtk_frame_new ("Graph Layout Options");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  btn = gtk_check_button_new_with_mnemonic ("Use edge _weights");
  gtk_widget_set_name (GTK_WIDGET(btn), "MDS_WEIGHTS");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), btn,
    "The distance matrix for a graph is the minimum number of edges connecting any pair of nodes.  These distances can be weighted if an edge variable is supplied.",
    NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), false);
  gtk_widget_set_sensitive (btn, ggv->mds_task == GraphLayout);
  g_signal_connect (G_OBJECT (btn), "toggled",
    G_CALLBACK (ggv_edge_weights_cb), inst);
  gtk_box_pack_start (GTK_BOX (vb), btn, true, true, 2);

  btn = gtk_check_button_new_with_mnemonic ("_Complete graph distances");
  gtk_widget_set_name (GTK_WIDGET(btn), "MDS_COMPLETE");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), btn,
    "Fill in a missing D[i,j] using a shortest path algorithm when a path exists from i to j; if not checked, D[i,j] is treated as missing.",
    NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(btn),
    ggv->Dtarget_source == LinkDist);
  gtk_widget_set_sensitive (btn, (ggv->mds_task == GraphLayout));
  g_signal_connect (G_OBJECT (btn), "toggled",
    G_CALLBACK (ggv_complete_distances_cb), inst);
  gtk_box_pack_start (GTK_BOX (vb), btn, true, true, 2);

  label = gtk_label_new_with_mnemonic ("_Task");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

/*-- "Definition of D" controls --*/

  hbox = gtk_hbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 2);

  /*-- include only edge sets.  --*/
  ggv->varnotebook = create_variable_notebook (hbox,
    GTK_SELECTION_SINGLE, all_vartypes, edgesets_only,
    G_CALLBACK(NULL), (gpointer) NULL, inst->gg);
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (ggv->varnotebook), 0);
  if (swin != NULL) {
    ggv->tree_view_dist = GTK_BIN(swin)->child;
    /* Initialize, selecting first variable */
    if (ggv->mds_task == DissimAnalysis)
      select_tree_view_row(ggv->tree_view_dist, 0);
  } else ggv->tree_view_dist = NULL;

  /*-- Report on D --*/
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

  label = gtk_label_new_with_mnemonic ("D_ist");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

/*-- Run controls --*/

  vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);

  /*-- table --*/
  table = gtk_table_new (2, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 1);

  top = 0;

  /*-- MDS Dimension --*/
  label = gtk_label_new_with_mnemonic ("Dimension (_k)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new ((gfloat)ggv->dim, 1.0, 10.0, 1.0, 1.0, 1.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_dims_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  top++;

  /* Stepsize */
  label = gtk_label_new_with_mnemonic ("_Step size");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (0.01, 0.0001, 0.2, 0.02, 0.2, 0.10);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_stepsize_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_widget_set_name (hscale, "stepsize_scale");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), hscale,
    "Stepsize", NULL);
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 4);
  /*
  gtk_box_pack_start (GTK_BOX (hbox), hscale, true, true, 2);
  */
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  /* Run and step */

  hbox = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);

  /*-- run --*/
  btn = gtk_check_button_new_with_mnemonic ("_Run MDS");
  gtk_widget_set_name (btn, "RunMDS");

  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (mds_run_cb), inst);

  btn = gtk_button_new_with_mnemonic ("Step _once");
  gtk_widget_set_name (btn, "Step");
  gtk_widget_set_sensitive (btn, false); /* make sensitive after running */
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (mds_step_cb), inst);

  label = gtk_label_new_with_mnemonic ("_Run");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

  /*-- Plot of stress function --*/
  frame = gtk_frame_new ("Stress function");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, true, true, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  ggv->stressplot_da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (ggv->stressplot_da),
    STRESSPLOT_WIDTH, STRESSPLOT_HEIGHT);
  g_signal_connect (G_OBJECT (ggv->stressplot_da), "expose_event",
    G_CALLBACK(ggv_stressplot_expose_cb), inst);
  g_signal_connect (G_OBJECT (ggv->stressplot_da), "configure_event",
    G_CALLBACK(ggv_stressplot_configure_cb), inst);
  gtk_widget_set_events (ggv->stressplot_da, GDK_EXPOSURE_MASK);
  gtk_box_pack_start (GTK_BOX (vb), ggv->stressplot_da, true, true, 2);


  /*-- MDS parameters that should stay in view at all times --*/

  frame = gtk_frame_new ("Key MDS Parameters");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, false, false, 2);

  vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);

  /*-- Metric vs Non-metric --*/
  metric_opt = gtk_combo_box_new_text ();
  gtk_widget_set_name (metric_opt, "metric_opt");
  populate_combo_box (metric_opt, (gchar**) metric_lbl, G_N_ELEMENTS(metric_lbl),
    G_CALLBACK(ggv_metric_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), metric_opt, false, false, 2);
  /* attach the label, hscale and two adjustments to this widget */

  /*-- Kruskal/Shepard vs Classic --*/
  opt = gtk_combo_box_new_text ();
  gtk_widget_set_name (opt, "kruskalshepard_classic_opt");
  populate_combo_box (opt, (gchar**) kruskal_lbl, G_N_ELEMENTS(kruskal_lbl),
    G_CALLBACK(ggv_kruskal_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), opt, false, false, 2);


  frame = gtk_frame_new ("Data (D): Histogram, power, weights");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, false, false, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 1);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  ggv->dissim->da = gtk_drawing_area_new ();
  gtk_widget_set_size_request (GTK_WIDGET (ggv->dissim->da),
    HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);
  gtk_widget_set_events (ggv->dissim->da, GDK_EXPOSURE_MASK
             | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
             | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
  g_signal_connect (G_OBJECT (ggv->dissim->da), "expose_event",
    G_CALLBACK(ggv_histogram_expose_cb), inst);
  g_signal_connect (G_OBJECT (ggv->dissim->da), "configure_event",
    G_CALLBACK(ggv_histogram_configure_cb), inst);
  g_signal_connect (G_OBJECT (ggv->dissim->da), "motion_notify_event",
    G_CALLBACK(ggv_histogram_motion_cb), inst);
  g_signal_connect (G_OBJECT (ggv->dissim->da), "button_press_event",
    G_CALLBACK(ggv_histogram_button_press_cb), inst);
  g_signal_connect (G_OBJECT (ggv->dissim->da), "button_release_event",
    G_CALLBACK(ggv_histogram_button_release_cb), inst);

  gtk_box_pack_start (GTK_BOX (vb), ggv->dissim->da, true, true, 1);


  /*-- Data power, weight power --*/
  table = gtk_table_new (2, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 1);
  gtk_box_pack_start (GTK_BOX (vb), table, false, false, 1);

  top = 0;

  /*-- Data (Dtarget) power --*/
/* Add a second adjustment for isotonic_mix, [0:1] */
  label = gtk_label_new_with_mnemonic ("Data _power (D^p)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  Dtarget_adj = gtk_adjustment_new (1.0, 0.0, 7.0, 0.02, 0.01, 1.0);
  g_signal_connect (G_OBJECT (Dtarget_adj), "value_changed",
    G_CALLBACK (ggv_Dtarget_power_cb), inst);

  hscale = gtk_hscale_new (GTK_ADJUSTMENT (Dtarget_adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_widget_set_name (hscale, "Dtarget_power_scale");
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  /*-- additional adjustment for isotonic_mix */
  isotonic_mix_adj = gtk_adjustment_new (100.0, 0.0, 101.0, 1.0, 1.0, 1.0);
  g_signal_connect (G_OBJECT (isotonic_mix_adj), "value_changed",
    G_CALLBACK (ggv_Dtarget_power_cb), inst);
  
    g_object_set_data(G_OBJECT(metric_opt), "label", label); 
    g_object_set_data(G_OBJECT(metric_opt), "hscale", hscale);
    g_object_set_data(G_OBJECT(metric_opt),
      "isotonic_mix_adj", isotonic_mix_adj); 
    g_object_set_data(G_OBJECT(metric_opt),
      "Dtarget_adj", Dtarget_adj); 

  /*-- Weight power --*/
  top++;
  label = gtk_label_new_with_mnemonic ("_Weight (w=D^r)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (0.0, -4.0, 5.0, 0.02, 0.01, 1.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_weight_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_widget_set_name (hscale, "weight_power_scale");
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

/*-- notebook --*/

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

  /*-- MDS parameters tab --*/
  vbox_params = gtk_vbox_new (false, 0);  /* 2 children, each a frame */

  frame = gtk_frame_new ("MDS distance parameters");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (vbox_params), frame, false, false, 2);

  vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  /*-- table --*/
  table = gtk_table_new (2, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 1);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 1);

  top = 0;

  /*-- Dist power --*/
  top++;
  label = gtk_label_new_with_mnemonic ("Dist p_ower (d^q)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (1.0, 0.0, 7.0, 0.02, 0.01, 1.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_dist_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_widget_set_name (hscale, "dist_power_scale");
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  /*-- Minkowski --*/
  top++;
  label = gtk_label_new_with_mnemonic ("_Minkowski (m)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (2.0, 1.0, 7.0, 0.02, 0.01, 1.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_lnorm_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_widget_set_name (hscale, "lnorm_scale");
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  label = gtk_label_new_with_mnemonic ("Di_stance");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox_params, label);

  /*-- Groups tab --*/
  frame = gtk_frame_new ("Groups");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  for (i=0; i<3; i++) {
    if (i == 0) {
      radio = gtk_radio_button_new_with_mnemonic (NULL, groups_lbl[i]);
      GTK_TOGGLE_BUTTON (radio)->active = TRUE;
      gtk_widget_set_name (radio, "GROUPS_OFF");
    } else {
      group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio));
      radio = gtk_radio_button_new_with_mnemonic (group, groups_lbl[i]);
    }
    g_object_set_data(G_OBJECT(radio), "PluginInst", inst);
    g_signal_connect (G_OBJECT (radio), "toggled",
      G_CALLBACK (ggv_groups_cb), GINT_TO_POINTER(i));
    gtk_box_pack_start (GTK_BOX (vbox), radio, TRUE, TRUE, 0);
  }

  label = gtk_label_new_with_mnemonic ("_Groups");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

  /*-- Anchor tab --*/
  frame = gtk_frame_new ("Anchor");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  hbox = gtk_hbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
  gtk_container_add (GTK_CONTAINER(frame), hbox);

  vbox = gtk_vbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 0);

  for (i=0; i<3; i++) {
    if (i == 0) {
      radio = gtk_radio_button_new_with_mnemonic (NULL, anchor_lbl[i]);
      gtk_widget_set_name (radio, "ANCHOR_OFF");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), true);
    } else {
      group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio));
      radio = gtk_radio_button_new_with_mnemonic (group, anchor_lbl[i]);
    }
    g_object_set_data(G_OBJECT(radio), "PluginInst", inst);
    g_signal_connect (G_OBJECT (radio), "toggled",
      G_CALLBACK (ggv_anchor_cb), GINT_TO_POINTER(i));
    gtk_box_pack_start (GTK_BOX (vbox), radio, TRUE, TRUE, 0);
  }

  /* Add symbols */
  ggv->anchor_frame =  gtk_frame_new ("Symbols in use");
  gtk_box_pack_start (GTK_BOX (hbox), ggv->anchor_frame, TRUE, TRUE, 0);
  ggv_anchor_table_build (inst);

  label = gtk_label_new_with_mnemonic ("_Anchor");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

  /*-- Sensitivity Analysis --*/
  frame = gtk_frame_new ("Sensitivity analysis");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  table = gtk_table_new (2, 3, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 2);
  top = 0;

  /*-- selection probability slider and button --*/
  btn = gtk_button_new_with_mnemonic ("_Resample");
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ggv_selection_prob_btn_cb), inst);
  gtk_table_attach (GTK_TABLE (table), btn, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL), 
    (GtkAttachOptions) (GTK_FILL),
    2, 2);

  adj = gtk_adjustment_new (ggv->rand_select_val, 0.0, 1.0, .01, .01, 0.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_selection_prob_adj_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (GTK_WIDGET(hscale), "selection_prob_scale");
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new_with_mnemonic ("_Selection prob.");
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);
  /*--        --*/

  /*-- perturbation slider and button --*/
  top++;
  btn = gtk_button_new_with_mnemonic ("Re_perturb");
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (ggv_perturb_btn_cb), inst);
  gtk_table_attach (GTK_TABLE (table), btn, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (ggv->perturb_val, 0.0, 1.0, 0.01, 0.01, 0.0);
  g_signal_connect (G_OBJECT (adj), "value_changed",
    G_CALLBACK (ggv_perturb_adj_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (GTK_WIDGET(hscale), "perturbation_scale");
  //gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new_with_mnemonic ("P_erturbation");
  gtk_label_set_mnemonic_widget(GTK_LABEL(label), hscale);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);
  /*--        --*/


  label = gtk_label_new_with_mnemonic ("Se_nsitivity");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  /*-- Constrained MDS --*/
  frame = gtk_frame_new ("Constrained MDS");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  opt = gtk_combo_box_new_text ();
  populate_combo_box(opt, (gchar **)constrained_lbl, G_N_ELEMENTS(constrained_lbl),
    G_CALLBACK(ggv_constrained_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);

  label = gtk_label_new_with_mnemonic ("C_onstraints");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  g_signal_connect (G_OBJECT(gg),
    "clusters_changed", G_CALLBACK(clusters_changed_cb), inst);

  gtk_widget_show_all (window);
}

/*
 * This one is called if the ggvis window is closed while
 * ggobi is still running.  It should disconnect signals,
 * because otherwise there's a problem when another routine
 * (eg GraphLayout) adds a datad.
 */
void close_ggvis_window(GtkWidget *w, PluginInstance *inst)
{
  if (inst->data) {
    GtkWidget *window = GTK_WIDGET(inst->data);
    ggobid *gg = inst->gg;
    ggvisd *ggv = ggvisFromInst (inst);

    GtkWidget *tree_view_node = widget_find_by_name (window, "nodeset");
    GtkWidget *tree_view_edge = widget_find_by_name (window, "edgeset");

    if (tree_view_node != NULL && tree_view_edge != NULL) {
      g_signal_handlers_disconnect_matched(G_OBJECT(gg),
        (GSignalMatchType) G_SIGNAL_MATCH_FUNC, 0, 0, NULL, 
        G_CALLBACK(ggv_tree_view_datad_added_cb),
        (gpointer) tree_view_node);
      g_signal_handlers_disconnect_matched(G_OBJECT(gg),
        (GSignalMatchType) G_SIGNAL_MATCH_FUNC, 0, 0, NULL, 
        G_CALLBACK(ggv_tree_view_datad_added_cb),
        (gpointer) tree_view_edge);
    }

    // I would have thought destroying this was enough, but apparently
    // not.  Probably if mds is running, I have to turn it off.

    if (ggv->running_p) mds_func (false, inst);

    variable_notebook_handlers_disconnect (ggv->varnotebook, gg);

    g_signal_handlers_disconnect_by_func (G_OBJECT(gg),
        G_CALLBACK(clusters_changed_cb),
        (gpointer) inst);

    /*
     * This is the window, so it should serve to destroy
     * all the child widgets.
    */
    gtk_widget_destroy ((GtkWidget *) inst->data);

    ggv_free (ggv);
  }
  inst->data = NULL;
}

/*
 * This one is called when ggobi shuts down; it doesn't
 * need to do much of anything, I think.  Should it free
 * its data, though?  Perhaps.  When I try freeing the pixmaps
 * at that time, though, I get an error message.  - dfs
 */
void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  if (inst->data) {
    inst->data = NULL;
  }
}

void
ggv_free (ggvisd *ggv) {
  /* unref the pixmaps */
  /*
  if (GTK_OBJECT(ggv->dissim->pix)->ref_count > 1)
    gdk_pixmap_unref (ggv->dissim->pix);
  if (GTK_OBJECT(ggv->stressplot_pix)->ref_count > 1)
    gdk_pixmap_unref (ggv->stressplot_pix);
  */

  arrayd_free (&ggv->Dtarget, 0, 0);
  arrayd_free (&ggv->pos, 0, 0);

  vectord_free (&ggv->stressvalues);
  vectord_free (&ggv->stressvalues);

  g_free (ggv->dissim);
  vectorb_free (&ggv->dissim->bars_included);
  vectori_free (&ggv->dissim->bins);
  vectorb_free (&ggv->anchor_group);
  vectord_free (&ggv->pos_mean);
  vectord_free (&ggv->weights);
  vectord_free (&ggv->rand_sel);
  vectord_free (&ggv->trans_dist);
  vectord_free (&ggv->config_dist);
  vectori_free (&ggv->point_status);
  vectori_free (&ggv->trans_dist_index);
  vectori_free (&ggv->bl);
  vectord_free (&ggv->bl_w);
  arrayd_free (&ggv->gradient, 0, 0);

  g_free (ggv);
}
