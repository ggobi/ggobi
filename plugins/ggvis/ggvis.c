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
void       show_ggvis_window (GtkWidget *widget, PluginInstance *inst);

gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  const gchar *lbl = "ggvis (MDS) ...";

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ((gchar *)lbl, gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_ggvis_window), inst);
  return(true);
}

static GtkItemFactoryEntry menu_items[] = {
/*
  { "/_IO",            NULL,     NULL,             0, "<Branch>" },
  { "/IO/Save distance matrix ...",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/_Reset",         NULL,     NULL,             0, "<Branch>" },
  { "/Reset/Center and scale",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
*/
  { "/_View",         NULL,     NULL,             0, "<Branch>" },
  { "/View/Shepard Plot",
       NULL,    
       (GtkItemFactoryCallback) create_shepard_data_cb,  
       0 },

  { "/_Reset",        NULL,     NULL,             0, "<Branch>" },
  { "/Reset/Reinit layout",
       NULL,    
       (GtkItemFactoryCallback) mds_reinit_cb,  
       0 },
  { "/Reset/Scramble layout",
       NULL,
       (GtkItemFactoryCallback) mds_scramble_cb,  
       0 },
  { "/Reset/Reset MDS parameters",
       NULL,    
       (GtkItemFactoryCallback) mds_reset_params_cb,  
       0 },

  { "/_Help",         NULL,     NULL,             0, "<LastBranch>" },
  { "/Help/MDS Background",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/Help/MDS Controls",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/Help/Formula for Kruskal-Shepard distance scaling",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  { "/Help/Formula for Torgerson-Gower dot-product scaling (classic)",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
       0 },
  };

static const gchar *const dsource_lbl[] = {
  "Unweighted graph dist", 
  "D from variable"
};
static const gchar *const metric_lbl[] = {"Metric MDS", "Nonmetric MDS"};
static const gchar *const kruskal_lbl[] = {"Kruskal", "Classic"};
static const gchar *const groups_lbl[] = {
  "Use all distances",
  "Use distances within groups",
  "Use distances between groups",
};
static const gchar *const anchor_lbl[] = {
  "No anchor",
  "Scaled",
  "Fixed",
};
static const gchar *const constrained_lbl[] = {
  "No variables frozen",
  "First variable frozen",
  "First two variables frozen"};

void
show_ggvis_window (GtkWidget *widget, PluginInstance *inst)
{
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
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
    ggv = (ggvisd *) gtk_object_get_data (GTK_OBJECT(window), "ggvisd");

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
ggv_datad_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  gchar *dname;
  datad *d;
  GSList *l;
  gchar *clname = gtk_widget_get_name (GTK_WIDGET(cl));
  gint k;

  gtk_clist_get_text (GTK_CLIST (cl), row, 0, &dname);
  for (l = gg->d; l; l = l->next) {
    d = l->data;
    if (strcmp (d->name, dname) == 0) {
      if (strcmp (clname, "nodeset") == 0) {
        ggv->dsrc = d;

        /* Once dsrc has been defined, the anchor groups can be
	   initialized */
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
ggv_clist_datad_added_cb (ggobid *gg, datad *d, void *clist)
{
  gchar *row[1];
  GtkWidget *swin = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT (clist), "datad_swin");
  gchar *clname = gtk_widget_get_name (GTK_WIDGET(clist));

  if (strcmp (clname, "nodeset") == 0 && d->rowIds != NULL) {
    row[0] = g_strdup (d->name);
    gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
    g_free (row[0]);
  }
  if (strcmp (clname, "edgeset") == 0 && d->edge.n > 0) {
    row[0] = g_strdup (d->name);
    gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
    g_free (row[0]);
  }

  gtk_widget_show_all (swin);
}

void
create_ggvis_window(ggvisd *ggv, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *vbox_params;
  GtkWidget *notebook, *varnotebook, *opt, *metric_opt;
  GtkWidget *label, *frame, *btn, *vbox, *hbox, *vb, *hscale, *table, *hb;
  GtkWidget *menu, *child, *radio, *radio1, *radio2;
  GList *children, *list;
  GSList *group;
  GtkObject *adj, *Dtarget_adj, *isotonic_mix_adj;
  gint i, top;
  GtkAccelGroup *ggv_accel_group;
  GtkWidget *menubar;
  /*-- for lists of datads --*/
  gchar *clist_titles[2] = {"node sets", "edge sets"};
  datad *d;
  ggobid *gg = inst->gg;
  GtkWidget *swin, *clist;
  gchar *row[1];
  GSList *l;

  ggv->tips = gtk_tooltips_new ();

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window), "ggvisd", ggv);
  inst->data = window;  /*-- or this could be the ggvis structure --*/

  gtk_window_set_title(GTK_WINDOW(window),
    "ggvis: multidimensional scaling");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
    GTK_SIGNAL_FUNC (close_ggvis_window), inst);

  main_vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  /* main menu bar */
  ggv_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items,
    sizeof (menu_items) / sizeof (menu_items[0]),
    ggv_accel_group, window,
    &menubar, (gpointer) inst);
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
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  clist = gtk_clist_new_with_titles (1, &clist_titles[0]);
  gtk_widget_set_name (GTK_WIDGET(clist), "nodeset");
  gtk_clist_set_selection_mode (GTK_CLIST (clist),
    GTK_SELECTION_SINGLE);
  gtk_object_set_data (GTK_OBJECT (clist), "datad_swin", swin);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
    (GtkSignalFunc) ggv_datad_set_cb, inst);
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) ggv_clist_datad_added_cb, GTK_OBJECT (clist));
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d->rowIds != NULL) {  /*-- node sets --*/
      row[0] = g_strdup (d->name);
      gtk_clist_append (GTK_CLIST (clist), row);
      g_free (row[0]);
    }
  }
  gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_box_pack_start (GTK_BOX (hbox), swin, true, true, 2);

/*
 * edge sets
*/
  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  clist = gtk_clist_new_with_titles (1, &clist_titles[1]);
  gtk_widget_set_name (GTK_WIDGET(clist), "edgeset");
  gtk_clist_set_selection_mode (GTK_CLIST (clist),
    GTK_SELECTION_SINGLE);
  gtk_object_set_data (GTK_OBJECT (clist), "datad_swin", swin);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
    (GtkSignalFunc) ggv_datad_set_cb, inst);
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) ggv_clist_datad_added_cb, GTK_OBJECT (clist));
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d->edge.n != 0) {  /*-- edge sets --*/
      row[0] = g_strdup (d->name);
      gtk_clist_append (GTK_CLIST (clist), row);
      g_free (row[0]);
    }
  }
  gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_box_pack_start (GTK_BOX (hbox), swin, true, true, 2);

  label = gtk_label_new ("Datasets");
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

  radio1 = gtk_radio_button_new_with_label (NULL, "Dissimilarity analysis");
  gtk_widget_set_name (GTK_WIDGET(radio1), "MDS");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), radio1,
    "Perform multidimensional scaling (MDS) for the purpose of dissimilarity analysis; dissimilarities (distances) are provided as an edge variable.",
    NULL);
  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_label (group, "Graph layout");
  gtk_widget_set_name (GTK_WIDGET(radio2), "GRAPH_LAYOUT");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), radio2,
    "Perform multidimensional scaling (MDS) for the purpose of laying out a graph.",
    NULL);

  GTK_TOGGLE_BUTTON(radio1)->active = (ggv->mds_task == DissimAnalysis);
  GTK_TOGGLE_BUTTON(radio2)->active = (ggv->mds_task == GraphLayout);

  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
    GTK_SIGNAL_FUNC (ggv_task_cb), inst);
  gtk_signal_connect (GTK_OBJECT (radio2), "toggled",
    GTK_SIGNAL_FUNC (ggv_task_cb), inst);

  gtk_box_pack_start (GTK_BOX (vb), radio1, true, true, 2);
  gtk_box_pack_start (GTK_BOX (vb), radio2, true, true, 2);

  frame = gtk_frame_new ("Graph Layout Options");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  btn = gtk_check_button_new_with_label ("Use edge weights");
  gtk_widget_set_name (GTK_WIDGET(btn), "MDS_WEIGHTS");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), btn,
    "The distance matrix for a graph is the minimum number of edges connecting any pair of nodes.  These distances can be weighted if an edge variable is supplied.",
    NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), false);
  gtk_widget_set_sensitive (btn, ggv->mds_task == GraphLayout);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
    GTK_SIGNAL_FUNC (ggv_edge_weights_cb), inst);
  gtk_box_pack_start (GTK_BOX (vb), btn, true, true, 2);

  btn = gtk_check_button_new_with_label ("Complete graph distances");
  gtk_widget_set_name (GTK_WIDGET(btn), "MDS_COMPLETE");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), btn,
    "Fill in a missing D[i,j] using a shortest path algorithm when a path exists from i to j; if not checked, D[i,j] is treated as missing.",
    NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(btn),
    ggv->Dtarget_source == LinkDist);
  gtk_widget_set_sensitive (btn, (ggv->mds_task == GraphLayout));
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
    GTK_SIGNAL_FUNC (ggv_complete_distances_cb), inst);
  gtk_box_pack_start (GTK_BOX (vb), btn, true, true, 2);

  label = gtk_label_new ("Task");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

/*-- "Definition of D" controls --*/

  hbox = gtk_hbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 2);

  /*-- Option menu: Use edge distances or an edge variable --*/

  /*-- include only edge sets.  --*/
  varnotebook = create_variable_notebook (hbox,
    GTK_SELECTION_SINGLE, all_vartypes, edgesets_only,
    (GtkSignalFunc) NULL, inst->gg);
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (varnotebook), 0);
  ggv->clist_dist = (GtkCList *) GTK_BIN(swin)->child;
  /* Initialize, selecting first variable */
  if (ggv->mds_task == DissimAnalysis)
    gtk_clist_select_row (ggv->clist_dist, 0, 0);

  /*-- Report on D --*/
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);

  label = gtk_label_new ("Dist");
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
  label = gtk_label_new ("Dimension (k)");
  /*gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);*/
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new ((gfloat)ggv->dim, 1.0, 10.0, 1.0, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_dims_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  top++;

  /* Stepsize */
  label = gtk_label_new ("Step size");
  /*gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);*/
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (0.01, 0.0001, 0.2, 0.02, 0.2, 0.10);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_stepsize_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (hscale, "stepsize_scale");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (ggv->tips), hscale,
    "Stepsize", NULL);
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
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
  btn = gtk_check_button_new_with_label ("Run MDS");
  gtk_widget_set_name (btn, "RunMDS");

  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (mds_run_cb), inst);

  btn = gtk_button_new_with_label ("Step once");
  gtk_widget_set_name (btn, "Step");
  gtk_widget_set_sensitive (btn, false); /* make sensitive after running */
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (mds_step_cb), inst);

  label = gtk_label_new ("Run");
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
  gtk_signal_connect (GTK_OBJECT (ggv->stressplot_da), "expose_event",
    GTK_SIGNAL_FUNC(ggv_stressplot_expose_cb), inst);
  gtk_signal_connect (GTK_OBJECT (ggv->stressplot_da), "configure_event",
    GTK_SIGNAL_FUNC(ggv_stressplot_configure_cb), inst);
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
  metric_opt = gtk_option_menu_new ();
  gtk_widget_set_name (metric_opt, "metric_opt");
  populate_option_menu (metric_opt, (gchar**) metric_lbl,
    sizeof (metric_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_metric_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (hbox), metric_opt, false, false, 2);
  /* attach the label, hscale and two adjustments to this widget */

  /*-- Kruskal/Shepard vs Classic --*/
  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "kruskalshepard_classic_opt");
  populate_option_menu (opt, (gchar**) kruskal_lbl,
    sizeof (kruskal_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_kruskal_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (hbox), opt, false, false, 2);


  frame = gtk_frame_new ("Data (D): Histogram, power, weights");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, false, false, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 1);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  ggv->dissim->da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (ggv->dissim->da),
    HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);
  gtk_widget_set_events (ggv->dissim->da, GDK_EXPOSURE_MASK
             | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
             | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
  gtk_signal_connect (GTK_OBJECT (ggv->dissim->da), "expose_event",
    GTK_SIGNAL_FUNC(ggv_histogram_expose_cb), inst);
  gtk_signal_connect (GTK_OBJECT (ggv->dissim->da), "configure_event",
    GTK_SIGNAL_FUNC(ggv_histogram_configure_cb), inst);
  gtk_signal_connect (GTK_OBJECT (ggv->dissim->da), "motion_notify_event",
    GTK_SIGNAL_FUNC(ggv_histogram_motion_cb), inst);
  gtk_signal_connect (GTK_OBJECT (ggv->dissim->da), "button_press_event",
    GTK_SIGNAL_FUNC(ggv_histogram_button_press_cb), inst);
  gtk_signal_connect (GTK_OBJECT (ggv->dissim->da), "button_release_event",
    GTK_SIGNAL_FUNC(ggv_histogram_button_release_cb), inst);

  gtk_box_pack_start (GTK_BOX (vb), ggv->dissim->da, true, true, 1);


  /*-- Data power, weight power --*/
  table = gtk_table_new (2, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 1);
  gtk_box_pack_start (GTK_BOX (vb), table, false, false, 1);

  top = 0;

  /*-- Data (Dtarget) power --*/
/* Add a second adjustment for isotonic_mix, [0:1] */
  label = gtk_label_new ("Data power (D^p)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  Dtarget_adj = gtk_adjustment_new (1.0, 0.0, 7.0, 0.02, 0.01, 1.0);
  gtk_signal_connect (GTK_OBJECT (Dtarget_adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_Dtarget_power_cb), inst);

  hscale = gtk_hscale_new (GTK_ADJUSTMENT (Dtarget_adj));
  gtk_widget_set_name (hscale, "Dtarget_power_scale");
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  /*-- additional adjustment for isotonic_mix */
  isotonic_mix_adj = gtk_adjustment_new (100.0, 0.0, 101.0, 1.0, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (isotonic_mix_adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_Dtarget_power_cb), inst);
  menu = gtk_option_menu_get_menu (GTK_OPTION_MENU(metric_opt));
  children = gtk_container_children (GTK_CONTAINER(menu));

  for (list = children; list; list = list->next) {
    child = (GtkWidget *) list->data;
    gtk_object_set_data (GTK_OBJECT(child), "label", label); 
    gtk_object_set_data (GTK_OBJECT(child), "hscale", hscale);
    gtk_object_set_data (GTK_OBJECT(child),
      "isotonic_mix_adj", isotonic_mix_adj); 
    gtk_object_set_data (GTK_OBJECT(child),
      "Dtarget_adj", Dtarget_adj); 
  }

  /*-- Weight power --*/
  top++;
  label = gtk_label_new ("Weight (w=D^r)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (0.0, -4.0, 5.0, 0.02, 0.01, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_weight_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (hscale, "weight_power_scale");
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
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
  label = gtk_label_new ("Dist power (d^q)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (1.0, 0.0, 7.0, 0.02, 0.01, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_dist_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (hscale, "dist_power_scale");
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  /*-- Minkowski --*/
  top++;
  label = gtk_label_new ("Minkowski (m)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  adj = gtk_adjustment_new (2.0, 1.0, 7.0, 0.02, 0.01, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_lnorm_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (hscale, "lnorm_scale");
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    1, 1);

  label = gtk_label_new ("Distance");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox_params, label);

  /*-- Groups tab --*/
  frame = gtk_frame_new ("Groups");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  for (i=0; i<3; i++) {
    if (i == 0) {
      radio = gtk_radio_button_new_with_label (NULL, groups_lbl[i]);
      GTK_TOGGLE_BUTTON (radio)->active = TRUE;
      gtk_widget_set_name (radio, "GROUPS_OFF");
    } else {
      group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio));
      radio = gtk_radio_button_new_with_label (group, groups_lbl[i]);
    }
    gtk_object_set_data (GTK_OBJECT(radio), "PluginInst", inst);
    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
      GTK_SIGNAL_FUNC (ggv_groups_cb), GINT_TO_POINTER(i));
    gtk_box_pack_start (GTK_BOX (vbox), radio, TRUE, TRUE, 0);
  }

  label = gtk_label_new ("Groups");
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
      radio = gtk_radio_button_new_with_label (NULL, anchor_lbl[i]);
      gtk_widget_set_name (radio, "ANCHOR_OFF");
      GTK_TOGGLE_BUTTON (radio)->active = TRUE;
    } else {
      group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio));
      radio = gtk_radio_button_new_with_label (group, anchor_lbl[i]);
    }
    gtk_object_set_data (GTK_OBJECT(radio), "PluginInst", inst);
    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
      GTK_SIGNAL_FUNC (ggv_anchor_cb), GINT_TO_POINTER(i));
    gtk_box_pack_start (GTK_BOX (vbox), radio, TRUE, TRUE, 0);
  }

  /* Add symbols */
  ggv->anchor_frame =  gtk_frame_new ("Symbols in use");
  gtk_box_pack_start (GTK_BOX (hbox), ggv->anchor_frame, TRUE, TRUE, 0);
  ggv_anchor_table_build (inst);

  label = gtk_label_new ("Anchor");
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
  btn = gtk_button_new_with_label ("Resample");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (ggv_selection_prob_btn_cb), inst);
  gtk_table_attach (GTK_TABLE (table), btn, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL), 
    (GtkAttachOptions) (GTK_FILL),
    2, 2);

  adj = gtk_adjustment_new (ggv->rand_select_val, 0.0, 1.0, .01, .01, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_selection_prob_adj_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (GTK_WIDGET(hscale), "selection_prob_scale");
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Selection prob.");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);
  /*--        --*/

  /*-- perturbation slider and button --*/
  top++;
  btn = gtk_button_new_with_label ("Reperturb");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (ggv_perturb_btn_cb), inst);
  gtk_table_attach (GTK_TABLE (table), btn, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (ggv->perturb_val, 0.0, 1.0, 0.01, 0.01, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_perturb_adj_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_name (GTK_WIDGET(hscale), "perturbation_scale");
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Perturbation");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 2, 3, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);
  /*--        --*/


  label = gtk_label_new ("Sensitivity");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  /*-- Constrained MDS --*/
  frame = gtk_frame_new ("Constrained MDS");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) constrained_lbl,
    sizeof (constrained_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_constrained_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 0);

  label = gtk_label_new ("Constraints");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  gtk_signal_connect (GTK_OBJECT(gg),
    "clusters_changed", clusters_changed_cb, inst);

  gtk_widget_show_all (window);
}


void close_ggvis_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  if (inst->data) {
    ggvisd *ggv = ggvisFromInst (inst);
    /*
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_ggvis_window), inst);
    */
    gtk_widget_destroy ((GtkWidget *) inst->data);
  }
}

