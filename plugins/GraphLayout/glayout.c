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
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    GtkWidget *window;
    glayoutd *gl = (glayoutd *) g_malloc (sizeof (glayoutd));

    glayout_init (gl);

    window = create_glayout_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (window), "glayoutd", gl);
    inst->data = window;  /*-- or this could be the glayout structure --*/

/*-- Can't do this here until I have an agnostic highlight function --*/
/*
void highlight_edges_cb (GtkButton *button, PluginInstance *inst);
  gtk_signal_connect (GTK_OBJECT(inst->gg),
    "sticky_point_added", highlight_sticky_edges, inst);
  gtk_signal_connect (GTK_OBJECT(inst->gg),
    "sticky_point_removed", highlight_sticky_edges, inst);
*/


  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);
  }
}

glayoutd *
glayoutFromInst (PluginInstance *inst)
{
  GtkWidget *window = (GtkWidget *) inst->data;
  glayoutd *gl = (glayoutd *) gtk_object_get_data (GTK_OBJECT(window), "glayoutd");
  return gl;
}

void
scale_array_max (array_d *dist, gint nr, gint nc)
{
  /*extern gdouble delta;*/  /* in mds.c */
  gdouble max;
  gint i, j;
  gdouble **d = dist->vals;

  if (dist->nrows < nr || dist->ncols < nc)
    g_printerr ("This array is smaller than nr or nc\n");
  else {

    max = 0.0;
    for (j=0; j<nc; j++) {
      for (i=0; i<nr; i++) {
        if (d[i][j] != DBL_MAX) {
          if (d[i][j] > max) max = d[i][j];
          if (d[i][j] < 0.0) 
            g_printerr ("Negative value %e in dist array at i=%d, j=%d\n",
              d[i][j], i, j);
        }
      }
    }

    if (max < 1E-10) 
      printf("Range of dist array too small: max=%e\n", max);

    for (j=0; j<nc; j++) {
      for (i=0; i<nr; i++) {
        if(d[i][j] != DBL_MAX)
          d[i][j] /= max;
      }
    }
  }
}

#ifdef CMDS
/*-- move this to cmds_ui.c? --*/
static void cmds_cb (GtkButton *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  glayoutd *gl = glayoutFromInst (inst);
  gint i, nNodes, nEdges;

  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;
  if (d == NULL || e == NULL)
    return;

  nNodes = d->nrows;
  nEdges = e->edge.n;

  if (nEdges <= 0)
    return;

  /*-- allocate distance matrix, nNodes x nNodes --*/
  if (gl->dist.vals == NULL || gl->dist.nrows != nNodes)
    arrayd_alloc (&gl->dist, nNodes, nNodes);

  /*-- populate distance matrix with link distances --*/
  set_dist_matrix_from_edges (d, e, gg, gl);

g_printerr ("distance matrix allocated, populated and scaled\n");

  /*-- allocate position matrix, nNodes x nvariables --*/
  if (gl->pos.vals == NULL || gl->pos.nrows != nNodes)
    arrayd_alloc (&gl->pos, nNodes, d->ncols);
  
  cmds (&gl->dist, &gl->pos);
g_printerr ("through cmds\n");

/*-- add three variables and put in the new values --*/
  {
    gint k;
    gdouble *x = g_malloc0 (d->nrows * sizeof (gdouble));
    gchar *name;

    for (k=0; k<2; k++) {
      for (i=0; i<d->nrows; i++) {
        x[i] = gl->pos.vals[i][k];
      }
      name = g_strdup_printf ("Pos%d", k);
      newvar_add_with_values (x, d->nrows, name, d, gg);
      g_free (name);
    }
    g_free (x);
  }
}

/*-- move this to cmds_ui.c? --*/
static void mds_spring_cb (GtkButton *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  glayoutd *gl = glayoutFromInst (inst);
  gint i, j, jvar;
  gchar *name;
  gint ndims = 2;

  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;
  if (d == NULL || e == NULL)
    return;

/*-- add a couple of rounds of spring therapy --*/
  spring_once (ndims, d, e, &gl->dist, &gl->pos);
g_printerr ("through spring_once (ten times)\n");

  for (j=0; j<ndims; j++) {
    name = g_strdup_printf ("Pos%d", j);
    jvar = vartable_index_get_by_name (name, d);
    g_free (name);
    if (jvar >= 0) {
      for (i=0; i<d->nrows; i++) {
        d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = gl->pos.vals[i][j];
      }
      limits_set_by_var (jvar, true, true, d, gg);
      tform_to_world_by_var (jvar, d, gg);
    }
  }
  displays_tailpipe (FULL, gg);
}
#endif


GtkWidget *
create_glayout_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook, *label, *frame, *vbox, *btn;

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "glayout");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_glayout_window), inst);

  main_vbox = gtk_vbox_new (FALSE,1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook),
    GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

#ifdef CMDS
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
#endif

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

  /*-- graphviz tab: dot and neato --*/
  frame = gtk_frame_new ("Graphviz layouts");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

  vbox = gtk_vbox_new (false, 5);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 5); 
  gtk_container_add (GTK_CONTAINER(frame), vbox);

  btn = gtk_button_new_with_label ("dot");
  gtk_widget_set_name (btn, "dot");
#ifdef GRAPHVIZ
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (dot_neato_layout_cb), (gpointer) inst);
#endif
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  btn = gtk_button_new_with_label ("neato");
  gtk_widget_set_name (btn, "neato");
#ifdef GRAPHVIZ
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (dot_neato_layout_cb), (gpointer) inst);
#endif
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  label = gtk_label_new ("Graphviz");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);

  gtk_widget_show_all (window);

  return(window);
}


void close_glayout_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_glayout_window), inst);
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
