#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

#include "GGStructSizes.c"

void       close_ggvis_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_ggvis_window(ggobid *gg, PluginInstance *inst);
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

void
ggvis_init (ggvisd *ggv) {
/*
  arrayd_init_null (&ggv->dist_orig);
  arrayd_init_null (&ggv->dist);
  arrayd_init_null (&ggv->pos_orig);
  arrayd_init_null (&ggv->pos);
*/
}


void
show_ggvis_window (GtkWidget *widget, PluginInstance *inst)
{
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    GtkWidget *window;
    ggvisd *gl = (ggvisd *) g_malloc (sizeof (ggvisd));

    ggvis_init (gl);

    window = create_ggvis_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (window), "ggvisd", gl);
    inst->data = window;  /*-- or this could be the ggvis structure --*/

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);
  }
}

ggvisd *
ggvisFromInst (PluginInstance *inst)
{
  GtkWidget *window = (GtkWidget *) inst->data;
  ggvisd *gl = (ggvisd *) gtk_object_get_data (GTK_OBJECT(window), "ggvisd");
  return gl;
}


GtkWidget *
create_ggvis_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *run_vbox, *stress_vbox;
  GtkWidget *notebook;
  GtkWidget *frame, *btn, *vbox, *hbox, *hb, *hscale;
  GtkObject *adj;

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggvis");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
    GTK_SIGNAL_FUNC (close_ggvis_window), inst);

  main_vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  hbox = gtk_hbox_new (false, 1);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, false, false, 2);

/*-- Run controls --*/
  frame = gtk_frame_new ("");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (hbox), frame, false, false, 2);

  run_vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), run_vbox);

  btn = gtk_button_new_with_label ("Run MDS");
  gtk_box_pack_start (GTK_BOX (run_vbox), btn, false, false, 2);
  /*-- for stepsize, use a range widget --*/
  btn = gtk_button_new_with_label ("Step");
  gtk_box_pack_start (GTK_BOX (run_vbox), btn, false, false, 2);
  btn = gtk_button_new_with_label ("Reinit");
  gtk_box_pack_start (GTK_BOX (run_vbox), btn, false, false, 2);


/*-- Stress controls: parameters --*/
  frame = gtk_frame_new ("");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (hbox), frame, false, false, 2);

  stress_vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), stress_vbox);


  /*-- Data Power, Dist Power, Minkowski, Weight power --*/
  /* value, lower, upper, step_increment, page_increment, page_size */
  adj = gtk_adjustment_new (0.0, 0.0, 101.0, 0.1, 1.0, 1.0);
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 100, 30);
  scale_set_default_values (GTK_SCALE (hscale));
  gtk_box_pack_start (GTK_BOX (stress_vbox), hscale, TRUE, TRUE, 0);


#ifdef NOTEBOOK
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook),
    GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

  /*-- network tab: cmds --*/
  frame = gtk_frame_new ("Network layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_with_label ("cmds");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (cmds_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  btn = gtk_button_new_with_label ("spring");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (mds_spring_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  label = gtk_label_new ("Network");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  /*-- radial tab --*/
  frame = gtk_frame_new ("Radial layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_with_label ("apply");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (radial_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);


  label = gtk_label_new ("Radial");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);
  /*-- --*/

  /*-- graphviz tab: dot and neato --*/
  frame = gtk_frame_new ("Graphviz layouts");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_with_label ("dot");
  gtk_widget_set_name (btn, "dot");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  btn = gtk_button_new_with_label ("neato");
  gtk_widget_set_name (btn, "neato");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (dot_neato_layout_cb), (gpointer) inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  label = gtk_label_new ("Graphviz");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);
#endif

  gtk_widget_show_all (window);

  return(window);
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
