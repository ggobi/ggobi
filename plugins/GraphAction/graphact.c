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
void       show_graphact_window (GtkWidget *widget, PluginInstance *inst);


gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  const gchar *lbl = "Graph manipulation ...";

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ((gchar *)lbl, gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_graphact_window), inst);
  return(true);
}


void
show_graphact_window (GtkWidget *widget, PluginInstance *inst)
{
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    GtkWidget *window;
    graphactd *ga = (graphactd *) g_malloc (sizeof (graphactd));

    graphact_init (ga);
    inst->data = ga;

    window = create_graphact_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (window), "graphactd", ga);

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);
  }
}

graphactd *
graphactFromInst (PluginInstance *inst)
{
  graphactd *ga = (graphactd *) inst->data;
  return ga;
}

static void
graphact_datad_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  graphactd *ga = graphactFromInst (inst);
  gchar *dname;
  datad *d;
  GSList *l;
  gchar *clname = gtk_widget_get_name (GTK_WIDGET(cl));
  gboolean changed = false;

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
  /* Don't free either string; they're just pointers */


  if (ga->d != NULL && ga->e != NULL) {
    init_edge_vectors (changed, inst);
  }
}

static void 
graphact_clist_datad_added_cb (ggobid *gg, datad *d, void *clist)
{
  gchar *row[1];
  GtkWidget *swin = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT (clist), "datad_swin");
  gchar *clname = gtk_widget_get_name (GTK_WIDGET(clist));

  if (strcmp (clname, "nodeset") == 0 && d->rowid.idv.nels > 0) {
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

static const gchar *const neighborhood_depth_lbl[] = {
  " 1 ", " 2 "};
GtkWidget *
create_graphact_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook, *label, *frame, *vbox, *btn, *opt;
  GtkTooltips *tips = gtk_tooltips_new ();
  /*-- for lists of datads --*/
  gchar *clist_titles[2] = {"node sets", "edge sets"};
  datad *d;
  GtkWidget *hbox, *swin, *clist;
  gchar *row[1];
  GSList *l;
  graphactd *ga = graphactFromInst (inst);

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  ga->window = window;

  gtk_window_set_title(GTK_WINDOW(window), "Graph Action");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_graphact_window), inst);

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
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  clist = gtk_clist_new_with_titles (1, &clist_titles[0]);
  gtk_widget_set_name (GTK_WIDGET(clist), "nodeset");
  gtk_clist_set_selection_mode (GTK_CLIST (clist),
    GTK_SELECTION_SINGLE);
  gtk_object_set_data (GTK_OBJECT (clist), "datad_swin", swin);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
    (GtkSignalFunc) graphact_datad_set_cb, inst);
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) graphact_clist_datad_added_cb, GTK_OBJECT (clist));
  /*-- --*/

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d->rowid.idv.nels != 0) {  /*-- node sets --*/
      row[0] = g_strdup (d->name);
      gtk_clist_append (GTK_CLIST (clist), row);
      g_free (row[0]);
    }
  }
  gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_box_pack_start (GTK_BOX (hbox), swin, false, false, 2);

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
    (GtkSignalFunc) graphact_datad_set_cb, inst);
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) graphact_clist_datad_added_cb, GTK_OBJECT (clist));
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

  label = gtk_label_new ("Specify datasets");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

  /*-- --*/
  frame = gtk_frame_new ("Show/hide leaf nodes");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

  vbox = gtk_vbox_new (false, 1);
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);
  btn = gtk_button_new_with_label ("Hide");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (ga_leaf_hide_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);
  btn = gtk_button_new_with_label ("Show");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (ga_nodes_show_cb), inst);  /*-- show all nodes --*/
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);

  label = gtk_label_new ("Leaf");
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
 button: label all nodes and edges?
*/

  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);

  btn = gtk_check_button_new_with_label ("Show neighbors");
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
    GTK_SIGNAL_FUNC (show_neighbors_toggle_cb), inst);
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);

  btn = gtk_button_new_with_label ("Show all");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (ga_nodes_show_cb), inst);  /*-- show all nodes --*/
  gtk_box_pack_start (GTK_BOX (hbox), btn, false, false, 2);


  hbox = gtk_hbox_new (true, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);
  label = gtk_label_new ("Depth:");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_box_pack_start (GTK_BOX (hbox), label, false, false, 0);
  opt = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt, false, false, 0);
  populate_option_menu (opt, (gchar**) neighborhood_depth_lbl,
    sizeof (neighborhood_depth_lbl) / sizeof (gchar *),
    (GtkSignalFunc) neighborhood_depth_cb, "PluginInst", inst);

  label = gtk_label_new ("Neighbors");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
  /*-- --*/


  gtk_widget_show_all (window);

  return(window);
}


void close_graphact_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_graphact_window), inst);
    gtk_widget_destroy((GtkWidget*) inst->data);
  }
}

gint
visible_set (glong *visible, datad *d)
{
  gint i, m;
  gint nvisible = 0;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    if (!d->hidden.els[i]) {
      visible[nvisible++] = i;
    }
  }

  return nvisible;
}
