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
void       show_glayout_window (GtkWidget *widget, PluginInstance *inst);


gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  const gchar *lbl = "Graph layout ...";

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ((gchar *)lbl, gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_glayout_window), inst);
  return(true);
}


void
show_glayout_window (GtkWidget *widget, PluginInstance *inst)
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
    gtk_object_set_data (GTK_OBJECT (gl->window), "glayoutd", gl);

  } else {
    gl = (glayoutd *) inst->data;
    if (window)
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
glayout_datad_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  glayoutd *gl = glayoutFromInst (inst);
  gchar *dname;
  datad *d;
  GSList *l;
  gchar *clname = gtk_widget_get_name (GTK_WIDGET(cl));

  gtk_clist_get_text (GTK_CLIST (cl), row, 0, &dname);
  for (l = gg->d; l; l = l->next) {
    d = l->data;
    if (strcmp (d->name, dname) == 0) {
      if (strcmp (clname, "nodeset") == 0) {
        gl->dsrc = d;
      } else if (strcmp (clname, "edgeset") == 0) {
        gl->e = d;
      }
      break;
    }
  }
  /* Don't free either string; they're just pointers */
}
static gint
glayout_clist_datad_added_cb (ggobid *gg, datad *d, void *clist)
{
  gchar *row[1];
  GtkWidget *swin;
  gchar *clname;

  if (!GTK_IS_OBJECT (clist))
    g_printerr ("(glayout_clist_datad_added_cb) clist is not an object\n");
  if (!GTK_IS_WIDGET (clist))
    g_printerr ("(glayout_clist_datad_added_cb) clist is not a widget\n");

  swin = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (clist), "datad_swin");
  clname = gtk_widget_get_name (GTK_WIDGET(clist));
  g_printerr ("clname = %s\n", clname);

  /*
   * This doesn't look right: a new datad can be only one of a
   * node set and an edge set. ???
   */
  if (strcmp (clname, "nodeset") == 0 && d->rowIds) {
    g_printerr ("adding node set %s\n", d->name);
    row[0] = g_strdup (d->name);
    gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
    g_free (row[0]);
  }
  if (strcmp (clname, "edgeset") == 0) {
    g_printerr ("adding edge set %s\n", d->name);
    if (d->edge.n > 0) {
      g_printerr ("... with %d edges\n", d->edge.n);
      row[0] = g_strdup (d->name);
      gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
      g_free (row[0]);
    }
  }

  gtk_widget_show_all (swin);

  return false;
}

static const gchar *const neato_model_lbl[] = {
  "Shortest path", "Circuit resistance"};
GtkWidget *
create_glayout_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook, *label, *frame, *vbox, *btn;
  GtkWidget *hb, *entry, *hscale, *vb, *opt, *apply_btn;
  GtkTooltips *tips = gtk_tooltips_new ();
  /*-- for lists of datads --*/
  gchar *clist_titles[2] = {"node sets", "edge sets"};
  datad *d;
  GtkWidget *hbox, *swin, *clist, *varnotebook;
  gchar *row[1];
  GSList *l;
  glayoutd *gl = glayoutFromInst (inst);
  GtkObject *adj;

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gl->window = window;

  gtk_window_set_title(GTK_WINDOW(window), "Graph Layout");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_glayout_window), inst);

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
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  clist = gtk_clist_new_with_titles (1, &clist_titles[0]);
  gtk_widget_set_name (GTK_WIDGET(clist), "nodeset");
  gtk_clist_set_selection_mode (GTK_CLIST (clist),
    GTK_SELECTION_SINGLE);
  gtk_object_set_data (GTK_OBJECT (clist), "datad_swin", swin);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
    (GtkSignalFunc) glayout_datad_set_cb, inst);
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) glayout_clist_datad_added_cb, GTK_OBJECT (clist));
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
    (GtkSignalFunc) glayout_datad_set_cb, inst);
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) glayout_clist_datad_added_cb, GTK_OBJECT (clist));
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
  gtk_entry_set_editable (GTK_ENTRY (entry), false);
  gtk_object_set_data (GTK_OBJECT(window), "CENTERNODE", entry);
  if (gl->dsrc)
    gtk_entry_set_text (GTK_ENTRY (entry),
      (gchar *) g_array_index (gl->dsrc->rowlab, gchar *, 0));
  gtk_signal_connect (GTK_OBJECT(gg),
    "sticky_point_added", radial_center_set_cb, inst);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), entry,
    "To reset the center node, use sticky identification in ggobi", NULL);
  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);
  
  btn = gtk_button_new_with_label ("apply");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (radial_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  /*-- highlight the edges connected to nodes with sticky labels --*/
/*
  btn = gtk_button_new_with_label ("highlight edges");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (highlight_edges_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);
*/

  label = gtk_label_new ("Radial");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);
  /*-- --*/

/* 
 * neato tab
*/
  frame = gtk_frame_new ("Neato layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

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

  label = gtk_label_new ("Model:");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);
  opt = gtk_option_menu_new ();
/*
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Fix one of the axes during plot cycling or let them both float", NULL);
*/
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
#ifdef GRAPHVIZ
  populate_option_menu (opt, (gchar**) neato_model_lbl,
    sizeof (neato_model_lbl) / sizeof (gchar *),
    (GtkSignalFunc) neato_model_cb, "PluginInst", inst);
#endif

  /*-- neato scale --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 1);

  label = gtk_label_new ("Dimension:");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (vb), label, false, false, 3);

  adj = gtk_adjustment_new ((gfloat)gl->neato_dim, 2.0, 11.0, 1.0, 1.0, 1.0);
#ifdef GRAPHVIZ
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
    GTK_SIGNAL_FUNC (neato_dim_cb), inst);
#endif
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_widget_set_usize (GTK_WIDGET (hscale), 150, 30);

  gtk_range_set_update_policy (GTK_RANGE (hscale), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_scale_set_value_pos (GTK_SCALE(hscale), GTK_POS_BOTTOM);
  gtk_scale_set_draw_value (GTK_SCALE(hscale), TRUE);

  gtk_scale_set_digits (GTK_SCALE(hscale), 0);
  gtk_box_pack_start (GTK_BOX (vb), hscale, false, false, 3);
  /*-- --*/

  apply_btn = gtk_button_new_with_label ("apply");
  gtk_widget_set_name (apply_btn, "neato");
#ifdef GRAPHVIZ
  gtk_signal_connect (GTK_OBJECT (apply_btn), "clicked",
                      GTK_SIGNAL_FUNC (dot_neato_layout_cb), (gpointer) inst);
#else
  gtk_widget_set_sensitive (apply_btn, false);
#endif
  gtk_box_pack_start (GTK_BOX (vbox), apply_btn, false, false, 3);


  /*-- second child of the neato hbox, to contain the list of variables --*/
  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 0);

  btn = gtk_check_button_new_with_label ("Use edge length");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), btn,
    "Have neato use edge length in determining node positions, and use the selected variable as a source of lengths.  Edge lengths must be >= 1.0.",
    NULL);
#ifdef GRAPHVIZ
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
    GTK_SIGNAL_FUNC (neato_use_edge_length_cb), inst);
#endif
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 2);

  /*-- include only edge sets.  --*/
  varnotebook = create_variable_notebook (vbox,
    GTK_SELECTION_SINGLE, all_vartypes, edgesets_only,
    (GtkSignalFunc) NULL, inst->gg);
  gtk_object_set_data (GTK_OBJECT(apply_btn), "notebook", varnotebook);

  label = gtk_label_new ("Neato");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);


/*
 * Dot tab
*/
  frame = gtk_frame_new ("Dot layout");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_with_label ("apply");
  gtk_widget_set_name (btn, "dot");
#ifdef GRAPHVIZ
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (dot_neato_layout_cb), (gpointer) inst);
#else
  gtk_widget_set_sensitive (btn, false);
#endif
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  label = gtk_label_new ("Dot");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);


  gtk_widget_show_all (window);

  return(window);
}


void close_glayout_window(GtkWidget *w, PluginInstance *inst)
{
  /*
  gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
    (GtkSignalFunc) glayout_clist_datad_added_cb, GTK_OBJECT (clist));
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
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_glayout_window), inst);
    */
    gtk_widget_destroy (gl->window);
  }
}

gint
visible_set (glong *visible, datad *d)
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
