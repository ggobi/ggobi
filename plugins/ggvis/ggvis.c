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
  { "/_View",         NULL,     NULL,             0, "<Branch>" },
  { "/View/Shepard Plot",
       NULL,    
       (GtkItemFactoryCallback) NULL,  
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

static const gchar *const dsource_lbl[] = {"Unweighted", "D from variable"};
static const gchar *const metric_lbl[] = {"Metric MDS", "Nonmetric MDS"};
static const gchar *const kruskal_lbl[] = {"Kruskal", "Classic"};
static const gchar *const groups_lbl[] = {
  "Scale all",
  "Within groups",
  "Between groups",
  "Anchors scale",
  "Anchors fixed"};
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

    ggvis_init (ggv);
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

void
create_ggvis_window(ggvisd *ggv, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *vbox_params;
  GtkWidget *notebook, *varnotebook, *opt;
  GtkWidget *label, *frame, *btn, *vbox, *hbox, *vb, *hscale, *table, *hb;
  GtkObject *adj;
  gint top;
  GtkAccelGroup *ggv_accel_group;
  GtkWidget *menubar;
  GtkTooltips *tips;
  GtkWidget *entry;

  tips = gtk_tooltips_new ();

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

/*-- notebook for distance matrix, run controls, and starting layout --*/

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

/*-- "Definition of D" controls --*/

  hbox = gtk_hbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 2);

  /*-- Option menu: Use edge distances or an edge variable --*/
  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), opt,
    "To define the distance matrix D, use unweighted edges (typical for graphs) or the edge variable selected at right (typical for MDS data and weighted graphs)",
    NULL);
  populate_option_menu (opt, (gchar**) dsource_lbl,
    sizeof (dsource_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_dsource_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 2);

  btn = gtk_check_button_new_with_label ("Complete distances ");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Fill in a missing D[i,j] using a shortest path algorithm when a path exists from i to j; by default, D[i,j] is treated as missing.",
    NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
    GTK_SIGNAL_FUNC (ggv_complete_distances_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  /*-- include only edge sets.  --*/
  varnotebook = create_variable_notebook (hbox,
    GTK_SELECTION_SINGLE, all_vartypes, edgesets_only,
    (GtkSignalFunc) NULL, inst->gg);

  /*
   * create this button after the varnotebook, because it's
   *   necessary to get hold of the notebook in the button callback
  */
  btn = gtk_button_new_with_label ("Compute D");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Compute the new target distance matrix D.",
    NULL);
  gtk_object_set_data (GTK_OBJECT(btn), "notebook", varnotebook);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (ggv_compute_Dtarget_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  /*-- Report on D --*/
  hb = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 2);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Dimension of D"),
    false, false, 2);

  entry = gtk_entry_new ();
  gtk_object_set_data (GTK_OBJECT(window), "DTARGET_ENTRY", entry);
  gtk_entry_set_editable (GTK_ENTRY (entry), false);
  gtk_entry_set_text (GTK_ENTRY (entry), "(uninitialized)");
  gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);

  label = gtk_label_new ("Definition of D");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

/*-- Run controls --*/

  vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  hbox = gtk_hbox_new (true, 1);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);

  /* Run, step and reinit in an hbox */
  /*-- run --*/
  btn = gtk_check_button_new_with_label ("Run MDS");
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (mds_run_cb), inst);

  btn = gtk_button_new_with_label ("Step");
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (mds_step_cb), inst);

  btn = gtk_button_new_with_label ("Reinit");
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (mds_reinit_cb), inst);

  /*-- stepsize --*/
  table = gtk_table_new (1, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 2);

  adj = gtk_adjustment_new (0.01, 0.0001, 0.2, 0.02, 0.2, 0.10);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_stepsize_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 4);
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, 0, 1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Stepsize");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Run");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

  /*-- Reset initial layout --*/

  hbox = gtk_hbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);

  vbox = gtk_vbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 2);

  btn = gtk_check_button_new_with_label (" Scramble ");
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);
  btn = gtk_check_button_new_with_label (" Copy selected vars ");
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  /*-- exclude edge sets --*/
  varnotebook = create_variable_notebook (hbox,
    GTK_SELECTION_EXTENDED, all_vartypes, no_edgesets,
    (GtkSignalFunc) NULL, inst->gg);
  gtk_object_set_data (GTK_OBJECT (window),
    "notebook", varnotebook);

  label = gtk_label_new ("Starting pos");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);
  /*-- --*/


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
  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) metric_lbl,
    sizeof (metric_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_metric_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (hbox), opt, false, false, 2);

  /*-- Kruskal/Shepard vs Classic --*/
  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) kruskal_lbl,
    sizeof (kruskal_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_kruskal_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (hbox), opt, false, false, 2);

  /*-- table --*/
  table = gtk_table_new (1, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 2);

  top = 0;

  /*-- MDS Dimension --*/
  label = gtk_label_new ("Dimension (k)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new ((gfloat)ggv->mds_dims, 1.0, 10.0, 0.1, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_dims_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);


  frame = gtk_frame_new ("Data (D): Histogram, power, weights");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), frame, false, false, 2);

  vb = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  ggv->histogram_da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (ggv->histogram_da),
    HISTOGRAM_WIDTH, HISTOGRAM_HEIGHT);
  gtk_signal_connect (GTK_OBJECT (ggv->histogram_da), "expose_event",
    GTK_SIGNAL_FUNC(ggv_histogram_expose_cb), inst);
  gtk_signal_connect (GTK_OBJECT (ggv->histogram_da), "configure_event",
    GTK_SIGNAL_FUNC(ggv_histogram_configure_cb), inst);
  gtk_widget_set_events (ggv->histogram_da, GDK_EXPOSURE_MASK);
  gtk_box_pack_start (GTK_BOX (vb), ggv->histogram_da, true, true, 2);


  /*-- Data power, weight power --*/
  table = gtk_table_new (2, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 2);
  gtk_box_pack_start (GTK_BOX (vb), table, false, false, 2);

  top = 0;

  /*-- Data power --*/
  label = gtk_label_new ("Data power (D^p)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (1.0, 0.0, 6.0, 0.1, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_Dtarget_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Weight power --*/
  top++;
  label = gtk_label_new ("Weight (w=D^r)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (0.0, -4.0, 4.0, 0.1, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_weight_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

/*-- notebook --*/

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

  /*-- MDS parameters tab --*/
  vbox_params = gtk_vbox_new (false, 0);  /* 2 children, each a frame */

  frame = gtk_frame_new ("MDS distance parameters");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start (GTK_BOX (vbox_params), frame, false, false, 2);

  vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  /*-- table --*/
  table = gtk_table_new (2, 2, false);
  gtk_container_set_border_width (GTK_CONTAINER (table), 5);
  gtk_box_pack_start (GTK_BOX (vbox), table, false, false, 2);

  top = 0;

  /*-- Dist power --*/
  top++;
  label = gtk_label_new ("Dist power (d^q)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (1.0, 0.0, 6.0, 0.1, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_dist_power_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  /*-- Minkowski --*/
  top++;
  label = gtk_label_new ("Minkowski (m)");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_table_attach (GTK_TABLE (table), label, 1, 2, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (2.0, 1.0, 6.0, 0.1, 1.0, 1.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (ggv_lnorm_cb), inst);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);
  ggvis_scale_set_default_values (GTK_SCALE(hscale));
  gtk_table_attach (GTK_TABLE (table), hscale, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  label = gtk_label_new ("Distance");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox_params, label);

  /*-- Groups tab --*/
  frame = gtk_frame_new ("Groups");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_check_button_new_with_label ("Use brush groups");
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  opt = gtk_option_menu_new ();
  populate_option_menu (opt, (gchar**) groups_lbl,
    sizeof (groups_lbl) / sizeof (gchar *),
    (GtkSignalFunc) ggv_groups_cb, "PluginInst", inst);
  gtk_box_pack_start (GTK_BOX (vbox), opt, false, false, 2);

  label = gtk_label_new ("Groups");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

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
  gtk_table_attach (GTK_TABLE (table), btn, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL), 
    (GtkAttachOptions) (GTK_FILL),
    2, 2);

  adj = gtk_adjustment_new (1.0, 0.0, 1.0, 0.2, 0.2, 0.10);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
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
  gtk_table_attach (GTK_TABLE (table), btn, 0, 1, top, top+1,
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
    2, 2);

  adj = gtk_adjustment_new (0.0, 0.0, 1.0, 0.2, 0.2, 0.10);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
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

  gtk_widget_show_all (window);
}


void close_ggvis_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_ggvis_window), inst);
    gtk_widget_destroy((GtkWidget*) inst->data);
  }
}
