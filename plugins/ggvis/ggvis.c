#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

void       close_ggvis_window(GtkWidget *w, PluginInstance *inst);
GtkWidget *create_ggvis_window(ggobid *gg, PluginInstance *inst);
void       show_ggvis_window (GtkWidget *widget, PluginInstance *inst);


gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ("Graph layout ...", gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_ggvis_window), inst);
  return(true);
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
    ggvisd *ggv = (ggvisd *) g_malloc (sizeof (ggvisd));

    ggvis_init (ggv);

    window = create_ggvis_window (inst->gg, inst);
    gtk_object_set_data (GTK_OBJECT (window), "ggvisd", ggv);
    inst->data = window;  /*-- or this could be the ggvis structure --*/

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);
  }
}

ggvisd *
GGVisFromInst (PluginInstance *inst)
{
  GtkWidget *window = (GtkWidget *) inst->data;
  ggvisd *ggv = (ggvisd *) gtk_object_get_data (GTK_OBJECT(window), "ggvisd");
  return ggv;
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

static void
set_dist_matrix_from_edges (datad *d, datad *e, ggobid *gg, ggvisd *ggv)
{
  gint nNodes = d->nrows;
  gint nedges = e->edge.n;
  endpointsd *endpoints = e->edge.endpoints;

  gint i, j;
  gdouble infinity = (gdouble) (2*nNodes);
  gboolean changing;
  gint end1, end2, end3;
  gdouble d12;  /* weight */
  
  gdouble **dv = ggv->dist.vals;

  if (nNodes < 1 || nedges < 1)
    return;

  /* Ok, we have a nice distance matrix, let's fill it in with infinity. */
  for (i = 0; i < nNodes; i++) {
    for (j = 0; j < nNodes; j++)
      ggv->dist.vals[i][j] = infinity;
    ggv->dist.vals[i][i] = 0.0;
  }

  /* As long as we find a shorter path using the edges, keep going. */
  changing = true;
  while (changing) {
    changing = false;
    for (i = 0; i < nedges; i++) {
      end1 = d->rowid.idv.els[endpoints[i].a];
      end2 = d->rowid.idv.els[endpoints[i].b];
      /*-- we don't have edge weights yet --*/
      d12 = 1.0;
      for (end3 = 0; end3 < nNodes; end3++) {
        /* So we have a direct link from end1 to end2.  Can this be */
        /* used to shortcut a path from end1 to end3 or end2 to end3? */
        if (dv[end1][end3] > d12 + dv[end2][end3]) {
          dv[end3][end1] = dv[end1][end3] = d12 + dv[end2][end3];
          changing = true;
        }
        if (dv[end2][end3] > d12 + dv[end1][end3]) {
          dv[end3][end2] = dv[end2][end3] = d12 + dv[end1][end3];
          changing = true;
        }
      }    /* end3 */
    }    /* end1 and end2 */
  }    /* while changing. */

  scale_array_max (&ggv->dist, nNodes, nNodes);
}

/*-- move this to cmds_ui.c? --*/
static void cmds_cb (GtkButton *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = GGVisFromInst (inst);
  gint i, m, nNodes, nEdges;

  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;
  if (d == NULL || e == NULL)
    return;

  nNodes = d->nrows;
  nEdges = e->edge.n;

  if (nEdges <= 0)
    return;

  /*-- allocate distance matrix, nNodes x nNodes --*/
  if (ggv->dist.vals == NULL || ggv->dist.nrows != nNodes)
    arrayd_alloc (&ggv->dist, nNodes, nNodes);

  /*-- populate distance matrix with link distances --*/
  set_dist_matrix_from_edges (d, e, gg, ggv);

g_printerr ("distance matrix allocated, populated and scaled\n");

  /*-- allocate position matrix, nNodes x nvariables --*/
  if (ggv->pos.vals == NULL || ggv->pos.nrows != nNodes)
    arrayd_alloc (&ggv->pos, nNodes, d->ncols);
  
  cmds (&ggv->dist, &ggv->pos);
g_printerr ("through cmds\n");

/*-- add three variables and put in the new values --*/
  {
    gint k;
    gdouble *x = g_malloc0 (d->nrows * sizeof (gdouble));
    gchar *name;

    for (k=0; k<3; k++) {
      for (i=0; i<d->nrows; i++) {
        x[i] = ggv->pos.vals[i][k];
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
  ggvisd *ggv = GGVisFromInst (inst);
  gint i, j, jvar;
  gchar *name;

  datad *d = gg->current_display->d;
  datad *e = gg->current_display->e;
  if (d == NULL || e == NULL)
    return;

/*-- add a couple of rounds of spring therapy --*/
  spring_once (3, d, e, &ggv->dist, &ggv->pos);
g_printerr ("through spring_once (ten times)\n");

  for (j=0; j<3; j++) {
    name = g_strdup_printf ("Pos%d", j);
    jvar = vartable_index_get_by_name (name, d);
    g_free (name);
    if (jvar >= 0) {
      for (i=0; i<d->nrows; i++) {
        d->raw.vals[i][jvar] = d->tform.vals[i][jvar] = ggv->pos.vals[i][j];
      }
      limits_set_by_var (jvar, true, true, d, gg);
      tform_to_world_by_var (jvar, d, gg);
    }
  }
  displays_tailpipe (REDISPLAY_ALL, FULL, gg);
}


GtkWidget *
create_ggvis_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook, *label, *frame, *vbox, *btn;

  /*-- I will probably have to get hold of this window, after which
       I can name all the other widgets --*/
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggvis");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_ggvis_window), inst);

  main_vbox = gtk_vbox_new (FALSE,1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook),
    GTK_POS_TOP);
  gtk_box_pack_start (GTK_BOX (main_vbox), notebook, false, false, 2);

  /*-- network tab --*/
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

  /*-- highlight the edges connected to nodes with sticky labels --*/
  btn = gtk_button_new_with_label ("highlight edges");
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (highlight_edges_cb), inst);
  gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 3);

  label = gtk_label_new ("Radial");
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
                            frame, label);
  /*-- --*/

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
