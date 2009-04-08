#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "plugin.h"
/* glayout.h includes config.h, which defines HAVE_LIBGVC */
#include "glayout.h"

#if defined HAVE_LIBGVC

#include <gvc.h>

#define DOT_LAYOUT   0
#define NEATO_LAYOUT 1
#define FDP_LAYOUT   2 // currently disabled; crashes with memory
                       // errors when running snetwork.xml
#define TWOPI_LAYOUT 3 // currently disabled; redundant with radial,
                       // and not as good.
#define CIRCO_LAYOUT 4

#ifdef DEBUG
static void
test_edge_length (Agraph_t *graph, glayoutd *gl, ggobid *gg)
{
  gint i, a, b;
  GGobiData *d = gl->dsrc;
  GGobiData *e = gl->e;
  Agnode_t *head, *tail;
  Agedge_t *edge;
  gchar *name;
  endpointsd *endpoints = resolveEdgePoints(e, d);

  for (i=0; i<e->edge.n; i++) {
    edge_endpoints_get (i, &a, &b, d, endpoints, e);

    name = (gchar *) g_array_index (d->rowlab, gchar *, a);
    tail = agfindnode (graph, name);

    name = (gchar *) g_array_index (d->rowlab, gchar *, b);
    head = agfindnode (graph, name);

    if (head && tail) {
      edge = agfindedge (graph, tail, head);
      if (edge) {
        if (edge->u.dist > 1)
          g_printerr ("dist: %f\n", edge->u.dist);
      }
    }
  }
}
#endif

void neato_model_cb (GtkWidget *w, PluginInstance *inst)
{
  glayoutd *gl = glayoutFromInst (inst);
  gl->neato_model = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
}
void neato_use_edge_length_cb (GtkToggleButton *button, PluginInstance *inst)
{
  glayoutd *gl = glayoutFromInst (inst);
  gl->neato_use_edge_length_p = button->active;
}

void neato_dim_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  glayoutd *gl = glayoutFromInst (inst);
  gl->neato_dim = (gint) (adj->value);
}

static gint
neato_get_weight_var (Agraph_t *graph, GtkWidget *w, glayoutd *gl, ggobid *gg)
{
  gint weightvar = -1;
  GGobiData *e = gl->e;
  GGobiData *e_clist;
  GtkWidget *tree_view;

  /*-- find the variable which will define the edge lengths --*/
  /*-- first get the list of variables from the 'apply' button --*/
  tree_view = get_tree_view_from_object (G_OBJECT (w));
  if (!tree_view) {
    quick_message ("I can't identify a set of edges", false);
    return -1;
  }
  e_clist = g_object_get_data(G_OBJECT(tree_view), "datad");
  if (e_clist == NULL || e_clist != e) {
    quick_message ("This isn't the same set of edges you're using", false);
    return -1;
  }
  weightvar = get_one_selection_from_tree_view (tree_view, e);
  if (weightvar == -1) {
    quick_message ("Please specify a variable", false);
    return -1;
  }
  return weightvar;
}

static gboolean
neato_apply_edge_length (Agraph_t *graph, gint weightvar, glong *visible,
			 gint nvisible, GGobiData *d, GGobiData *e, ggobid *gg)
{
  gint i, a, b;
  //GGobiData *e_clist;
  Agnode_t *head, *tail;
  Agedge_t *edge;
  gchar *name;
  endpointsd *endpoints;

  /*
  GtkWidget *clist;
  clist = get_clist_from_object (GTK_OBJECT (w));
  if (!clist) {
    quick_message ("I can't identify a set of edges", false);
    return false;
  }
  // I don't think I have an equivalent of this test now.
  e_clist = g_object_get_data(G_OBJECT(clist), "datad");
  if (e_clist == NULL || e_clist != e) {
    quick_message ("This isn't the same set of edges you're using", false);
    return false;
  }
  */

  endpoints = resolveEdgePoints(e, d);
  if (endpoints) {

    // See splot_hidden_edge
    for (i=0; i<e->edge.n; i++) {
      edge_endpoints_get (i, &a, &b, d, endpoints, e);

      name = (gchar *) g_array_index (d->rowlab, gchar *, a);
      tail = agfindnode (graph, name);

      name = (gchar *) g_array_index (d->rowlab, gchar *, b);
      head = agfindnode (graph, name);

      if (head && tail) {
        edge = agfindedge (graph, tail, head);
        if (edge) {
          if (e->tform.vals[i][weightvar] < 1) {
            quick_message ("The minimum length is 1.0; perform a variable transformation before doing the layout.", false);
            return false;
          } else {
            agsafeset (edge, "len", 
              g_strdup_printf("%f", e->tform.vals[i][weightvar]), NULL);
          }
        }
      }
    }
  }
  return true;
}


void dot_neato_layout_cb (GtkWidget *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  glayoutd *gl = glayoutFromInst (inst);
  GGobiData *d = gl->dsrc;
  GGobiData *e = gl->e;
  Agnode_t *node, *head, *tail;
  Agedge_t *edge;
  Agedge_t **edgev;
  gint *intv;
  gchar *name;
  gint kind = AGRAPH;
  gint i, k;
  gint a, b;
#ifdef DEBUG
  FILE* f;
#endif
  Agraph_t *graph;
  gdouble **pos;
  gint layout_type = DOT_LAYOUT;
  static GVC_t *gvc; 
/*-- to add the new datad --*/
  gint m, nvisible, nc;
  InputDescription *desc = NULL;
  GGobiData *dnew;
  gdouble *values;
  gchar **rownames, **colnames, **rowids;
  glong *visible;
  displayd *dspnew;
  gint dim, weightvar = -1, nedges;
  endpointsd *endpoints = resolveEdgePoints(e, d);
  gchar       modelchar = 'd';

  DisplayOptions *opts = NULL;

  if (e == NULL) {
    g_printerr ("Trouble:  no edge set is specified\n");
    return;
  }

  visible = (glong *) g_malloc (d->nrows_in_plot * sizeof (glong));
  nvisible = visible_set (visible, d);

  if (strcmp (gtk_widget_get_name (button), "neato") == 0) {
    layout_type = NEATO_LAYOUT;
    if (gl->neato_use_edge_length_p)
      weightvar = neato_get_weight_var (graph, button, gl, gg);
  } else if (strcmp (gtk_widget_get_name (button), "twopi") == 0) {
    layout_type = TWOPI_LAYOUT;
  } else if (strcmp (gtk_widget_get_name (button), "fdp") == 0) {
    layout_type = FDP_LAYOUT;
  } else if (strcmp (gtk_widget_get_name (button), "circo") == 0) {
    layout_type = CIRCO_LAYOUT;
  }

  aginit();
  if (gvc == NULL)
    gvc = gvContext();

  /*-- create a new empty graph --*/
  graph = agopen("graph", kind);

  /*-- create new nodes, add to graph --*/
  for (i=0; i<nvisible; i++) {
    m = visible[i];
    name = (gchar *) g_array_index (d->rowlab, gchar *, m);
    agnode(graph, name);
  }

  intv = (gint *) g_malloc (e->edge.n * sizeof(gint));
  edgev = (Agedge_t **) g_malloc (e->edge.n * sizeof (Agedge_t *));
  nedges = 0;
  /*-- create new edges, add to graph --*/
  for (i=0; i<e->edge.n; i++) {
    if (e->excluded.els[i])
      continue;

/* still need a test for edge visibility */
    edge_endpoints_get (i, &a, &b, d, endpoints, e);

    name = (gchar *) g_array_index (d->rowlab, gchar *, a);
    tail = agfindnode (graph, name);

    name = (gchar *) g_array_index (d->rowlab, gchar *, b);
    head = agfindnode (graph, name);

    /*-- if head and tail are both in the visible subset --*/
    if (head && tail) {
      edge = agedge(graph, tail, head);
      intv[nedges] = i;
      edgev[nedges] = edge;
      nedges++;
    }
  }

  pos = (gdouble **) g_malloc0 (nvisible * sizeof (gdouble *));
  if (layout_type == DOT_LAYOUT || layout_type == TWOPI_LAYOUT || layout_type == FDP_LAYOUT || layout_type == CIRCO_LAYOUT)
    dim = 2;
  else dim = gl->neato_dim;
  for (i=0; i<nvisible; i++)
    pos[i] = (gdouble *) g_malloc0 (dim * sizeof (gdouble));

  if (layout_type == DOT_LAYOUT) {
    gvLayout(gvc, graph, "dot");

    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      pos[i][0] = ND_coord(node).x;
      pos[i][1] = ND_coord(node).y;
    }

  } else if (layout_type == TWOPI_LAYOUT) {
    Agnode_t *ctr = 0;

    /* Get the rowlab associated with the center node, if defined */
    if (gl->centerNodeIndex >= 0) {
      ctr = agfindnode(graph,
        (gchar *) g_array_index (d->rowlab, gchar *, 
				 (gl->centerNodeIndex)));
    }

    if (ctr)
      agsafeset (graph, "root", ctr->name, NULL);
    gvLayout (gvc, graph, "twopi");

    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      for (k=0; k<dim; k++)
        pos[i][k] = (gdouble) ND_pos(node)[k]; /* node->u.pos[k];*/
    }
   
  } else if (layout_type == FDP_LAYOUT) {  /* could use fdp_dim */
    gvLayout (gvc, graph, "fdp");

    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      for (k=0; k<dim; k++)
        pos[i][k] = (gdouble) ND_pos(node)[k]; /* node->u.pos[k];*/
    }

  } else if (layout_type == NEATO_LAYOUT) {
    attrsym_t*  sym;
    gchar *model;

    if (gl->neato_dim > 2) {
        char buf[20];
        sym = agfindattr(graph,"dim");   
        if (!sym)
          sym = agraphattr(graph,"dim","");
        sprintf (buf, "%d", gl->neato_dim);
        agxset(graph, sym->index, buf);
    }
    /* rather than doing the previous, you can take neato_init_graph
     * apart as you originally did. We should probably put in some
     * mechanism for setting Ndim but obviously wouldn't be in 1.16.
     * -- Emden
     */
    /* There is also a subset model */
    if (gl->neato_model == neato_circuit_resistance) {
      model = "circuit";
      modelchar = 'c';
    } else if (gl->neato_model == neato_shortest_path) {
      model = "shortpath";
      modelchar = 's';
    } else {
      model = "subset";
      modelchar = 'b';
    }
    agsafeset (graph, "model", model, NULL);

    /* In place of MODE_MAJOR, you could use MODE_KK which is the old neato */
    if (gl->neato_use_edge_length_p && weightvar >= 0) {
      neato_apply_edge_length(graph, weightvar, visible, nvisible, d, e, gg);
    }
    gvLayout (gvc, graph, "neato");

    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      for (k=0; k<gl->neato_dim; k++)
        pos[i][k] = (gdouble) ND_pos(node)[k]; /* node->u.pos[k];*/
      /*-- may want to set u.width, u.height to very small values --*/
      /*-- I see no effect, though ... dfs --*/
      node->u.width = node->u.height = .001;
    }
#ifdef DEBUG
    f = fopen ("test.out", "w");
    agwrite (graph, f);
    fclose(f);
#endif

  } else if (layout_type == CIRCO_LAYOUT) {

    gvLayout (gvc, graph, "circo");

    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      for (k=0; k<dim; k++)
        pos[i][k] = (gdouble) ND_pos(node)[k]; /* node->u.pos[k];*/
    }
  }

  gvFreeLayout (gvc, graph);
/*
 * create a new datad with the new variables.  include only
 * those nodes that are visible.  these needs some more testing ...
 * and this code could be more efficient -- writing to one set
 * of arrays, then copying to a matrix is probably unnecessary.
*/
  nc = dim;

  rowids = (gchar **) g_malloc (nvisible * sizeof(gchar *));
  for (m=0; m<nvisible; m++) {
    i = visible[m];
    rowids[m] = g_strdup (d->rowIds[i]);
  }

  values = (gdouble *) g_malloc (nvisible * nc * sizeof(gdouble));
  rownames = (gchar **) g_malloc (nvisible * sizeof(gchar *));
  for (i=0; i<nvisible; i++) {
    rownames[i] = (gchar *) g_array_index (d->rowlab, gchar *, visible[i]);
    for (k=0; k<dim; k++)
      values[i + k*nvisible] = (gdouble) pos[i][k];
  }

  colnames = (gchar **) g_malloc (nc * sizeof(gchar *));
  for (k=0; k<dim; k++)
    colnames[k] = g_strdup_printf ("Pos%d", k);

  /*
   * In case there is no initial scatterplot because the datasets
   * have no variables, we don't want creating a datad to trigger
   * the initialization of this plot.   This takes care of it.
  */
  GGOBI_getSessionOptions()->info->createInitialScatterPlot = false;
  /*-- --*/

  dnew = ggobi_data_new (nvisible, nc);
  switch (layout_type) {
    case DOT_LAYOUT:
      dnew->name =  g_strdup ("dot");
      dnew->nickname = g_strdup ("dot");
      break;
    case NEATO_LAYOUT:
      dnew->name = g_strdup_printf ("neato %dd%c", gl->neato_dim, modelchar);
      dnew->nickname = g_strdup_printf ("nto%dd%c", gl->neato_dim, modelchar);
      break;
    case TWOPI_LAYOUT:
      dnew->name =  g_strdup ("twopi");
      dnew->nickname = g_strdup ("twopi");
      break;
    case FDP_LAYOUT:
      dnew->name =  g_strdup ("fdp");
      dnew->nickname = g_strdup ("fdp");
      break;
    case CIRCO_LAYOUT:
      dnew->name =  g_strdup ("circo");
      dnew->nickname = g_strdup ("circo");
      break;
  }

  GGOBI(setData) (values, rownames, colnames, nvisible, nc, dnew, false,
    gg, rowids, false, desc);

  /*-- copy the color and glyph vectors from d to dnew --*/
  for (i=0; i<nvisible; i++) {
    dnew->color.els[i] = dnew->color_now.els[i] = dnew->color_prev.els[i] =
      d->color.els[visible[i]];
    dnew->glyph.els[i].type = dnew->glyph_now.els[i].type =
      dnew->glyph_prev.els[i].type = d->glyph.els[visible[i]].type;
    dnew->glyph.els[i].size = dnew->glyph_now.els[i].size =
      dnew->glyph_prev.els[i].size = d->glyph.els[visible[i]].size;
  }

/*
 * open a new scatterplot with the new data, and display edges
 * as they're displayed in the current datad.
*/ 

  /* dfs -- in imitation of code in display_ui.c, maybe not needed */
  splot_set_current (gg->current_splot, off, gg);   
  /* */

  opts = GGOBI(getDefaultDisplayOptions)();
  opts->axes_show_p = false;
  opts->edges_undirected_show_p = true;
  dspnew = GGOBI(newScatterplot) (0, 1, true, dnew, gg);
  opts->axes_show_p = true;  /*-- restore it --*/
  opts->edges_undirected_show_p = false;

  display_add(dspnew, gg);
  varpanel_refresh(dspnew, gg);

  setDisplayEdge (dspnew, e);

  for (i=0; i<nvisible; i++) {
    g_free (pos[i]);
  }
  g_free (pos);
  g_free (visible);
  agclose (graph);
}

#endif
