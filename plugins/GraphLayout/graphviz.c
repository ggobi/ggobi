#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "glayout.h"

#ifdef GRAPHVIZ
#include <dotneato.h>

#define DOT_LAYOUT   0
#define NEATO_LAYOUT 1

#define DATE    "June 2002"
#define VERSION "huh?"
char *Info[] = {
    "glayout",            /* Program */
    VERSION,              /* Version */
    DATE                  /* Build Date */
};

#ifdef DEBUG
static void
test_edge_length (Agraph_t *graph, glayoutd *gl, ggobid *gg)
{
  gint i, a, b;
  datad *d = gl->dsrc;
  datad *e = gl->e;
  Agnode_t *head, *tail;
  Agedge_t *edge;
  gchar *name;
  endpointsd *endpoints = resolveEdgePoints(e, d);

  for (i=0; i<e->edge.n; i++) {
    edge_endpoints_get (i, &a, &b, d, endpoints, e);

    /*a = d->rowid.idv.els[e->edge.endpoints[i].a];*/
    name = (gchar *) g_array_index (d->rowlab, gchar *, a);
    tail = agfindnode (graph, name);

    /*b = d->rowid.idv.els[e->edge.endpoints[i].b];*/
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

void neato_model_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *) gtk_object_get_data (GTK_OBJECT (w),
    "PluginInst");
  glayoutd *gl = glayoutFromInst (inst);
  gl->neato_model = GPOINTER_TO_INT (cbd);
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
  gint weightvar;
  datad *e = gl->e;
  datad *e_clist;
  GtkWidget *clist;

  /*-- find the variable which will define the edge lengths --*/
  /*-- first get the list of variables from the 'apply' button --*/
  clist = get_clist_from_object (GTK_OBJECT (w));
  if (!clist) {
    quick_message ("I can't identify a set of edges", false);
    return false;
  }
  e_clist = gtk_object_get_data (GTK_OBJECT(clist), "datad");
  if (e_clist == NULL || e_clist != e) {
    quick_message ("This isn't the same set of edges you're using", false);
    return false;
  }
  weightvar = get_one_selection_from_clist (clist, e);
  if (weightvar == -1) {
    quick_message ("Please specify a variable", false);
    return false;
  }
  return weightvar;
}

/*
static gboolean
neato_apply_edge_length (Agraph_t *graph, GtkWidget *w,
  glayoutd *gl, ggobid *gg)
{
  gint i, a, b, selected_var;
  datad *d = gl->dsrc;
  datad *e = gl->e;
  datad *e_clist;
  Agnode_t *head, *tail;
  Agedge_t *edge;
  gchar *name;
  GtkWidget *clist;

  clist = get_clist_from_object (GTK_OBJECT (w));
  if (!clist) {
    quick_message ("I can't identify a set of edges", false);
    return false;
  }
  e_clist = gtk_object_get_data (GTK_OBJECT(clist), "datad");
  if (e_clist == NULL || e_clist != e) {
    quick_message ("This isn't the same set of edges you're using", false);
    return false;
  }
  selected_var = get_one_selection_from_clist (clist, e);
  if (selected_var == -1) {
    quick_message ("Please specify a variable", false);
    return false;
  }

  for (i=0; i<e->edge.n; i++) {
    a = d->rowid.idv.els[e->edge.endpoints[i].a];
    name = (gchar *) g_array_index (d->rowlab, gchar *, a);
    tail = agfindnode (graph, name);

    b = d->rowid.idv.els[e->edge.endpoints[i].b];
    name = (gchar *) g_array_index (d->rowlab, gchar *, b);
    head = agfindnode (graph, name);

    if (head && tail) {
      edge = agfindedge (graph, tail, head);
      if (edge) {
        if (e->tform.vals[i][selected_var] < 1) {
          quick_message ("The minimum length is 1.0; perform a variable transformation before doing the layout.", false);
          return false;
        } else {
          edge->u.dist = e->tform.vals[i][selected_var];
        }
      }
    }
  }
  return true;
}
*/


void dot_neato_layout_cb (GtkWidget *button, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  glayoutd *gl = glayoutFromInst (inst);
  displayd *dsp = gg->current_display;
  datad *d = gl->dsrc;
  datad *e = gl->e;
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
/*-- to add the new datad --*/
  gint m, nvisible, nc;
  InputDescription *desc = NULL;
  datad *dnew;
  gdouble *values;
  gchar **rownames, **colnames, **rowids;
  glong *visible;
  displayd *dspnew;
  gboolean edges_displayed;
  gint dim, weightvar = -1, nedges;
  endpointsd *endpoints = resolveEdgePoints(e, d);

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
  }

  aginit();

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

    /*a = d->rowid.idv.els[e->edge.endpoints[i].a];*/
    name = (gchar *) g_array_index (d->rowlab, gchar *, a);
    tail = agfindnode (graph, name);

    /*b = d->rowid.idv.els[e->edge.endpoints[i].b];*/
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
  if (layout_type == DOT_LAYOUT)
    dim = 2;
  else dim = gl->neato_dim;
  for (i=0; i<nvisible; i++)
    pos[i] = (gdouble *) g_malloc0 (dim * sizeof (gdouble));

  if (layout_type == DOT_LAYOUT) {
    graph_init(graph);
    graph->u.drawing->engine = DOT;
    dot_init_node_edge(graph);  /*-- this might muck with width, height --*/
    dot_rank(graph);
    dot_mincross(graph);
    dot_position(graph);

    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      pos[i][0] = (gdouble) node->u.coord.x;
      pos[i][1] = (gdouble) node->u.coord.y;
    }
    dot_cleanup (graph);

  } else {  /* if layout_type == NEATO */
    gint         nG;
    attrsym_t*  sym;
   
    /* setting rankdir=LR is currently undefined in neato,
     * but having it set causes has effects in the common routines.
     * So, we turn it off.
     */
    sym = agfindattr(graph,"rankdir");
    if (sym)
      agxset (graph, sym->index, "");

    graph_init(graph);

	graph->u.ndim = gl->neato_dim;
	Ndim = graph->u.ndim = MIN(graph->u.ndim,MAXDIM);

    graph->u.drawing->engine = NEATO;
    neato_init_node_edge(graph);
    nG = scan_graph(graph);

    if (weightvar >= 0) {
      for (i=0; i<nedges; i++) {
        if (e->tform.vals[intv[i]][weightvar] < 1) {
          quick_message ("Can't use weights: the minimum length is 1.0; perform a variable transformation before doing the layout.", false);
          g_printerr ("len: %f\n", e->tform.vals[intv[i]][weightvar]);
          /* reset weights */
          for (k=0; i<i; k++)
            edgev[intv[k]]->u.dist = 1.0;
          break;  /*-- free arrays and quit?  --*/
        }
        edgev[intv[i]]->u.dist = e->tform.vals[intv[i]][weightvar];
      }
      g_free (intv);
      g_free (edgev);
    }

    if (Nop) {
      initial_positions(graph, nG);
    }
    else {
      if (gl->neato_model == neato_circuit_resistance) {
         circuit_model(graph,nG);
      } else shortest_path(graph, nG);
      initial_positions(graph, nG);
      diffeq_model(graph, nG);
      solve_model(graph, nG);
      final_energy(graph, nG); 
      adjustNodes(graph);
    }
    /*-- add variables generated by the layout algorithm --*/
    for (i=0; i<nvisible; i++) {  /*-- nrows = nnodes --*/
      m = visible[i];
      name = (gchar *) g_array_index (d->rowlab, gchar *, m);
      node = agfindnode (graph, name);
      for (k=0; k<gl->neato_dim; k++)
        pos[i][k] = (gdouble) node->u.pos[k];
      /*-- may want to set u.width, u.height to very small values --*/
      /*-- I see no effect, though ... dfs --*/
      node->u.width = node->u.height = .001;
    }
#ifdef DEBUG
    f = fopen ("test.out", "w");
    agwrite (graph, f);
    fclose(f);
#endif
    neato_cleanup (graph);
  }

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
  sessionOptions->info->createInitialScatterPlot = false;
  /*-- --*/

  dnew = datad_create (nvisible, nc, gg);
  dnew->name = (layout_type == DOT_LAYOUT) ?
    g_strdup ("dot") :
    g_strdup_printf ("neato %dd%c", gl->neato_dim,
     (gl->neato_model == neato_circuit_resistance) ? 'c' : 's');
  dnew->nickname = (layout_type == DOT_LAYOUT) ?
    g_strdup ("dot") :
    g_strdup_printf ("nto%dd%c", gl->neato_dim,
     (gl->neato_model == neato_circuit_resistance) ? 'c' : 's');

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
  dspnew = GGOBI(newScatterplot) (0, 1, dnew, gg);
  setDisplayEdge (dspnew, e);
  if (dsp)
    edges_displayed = display_copy_edge_options (dsp, dspnew);
  if (!edges_displayed) {
    GGOBI(setShowLines)(dspnew, true);
/*
    GtkWidget *item;
    dspnew->options.edges_undirected_show_p = true;
    item = widget_find_by_name (dspnew->edge_menu,
            "DISPLAY MENU: show directed edges");
    if (item)
      gtk_check_menu_item_set_active ((GtkCheckMenuItem *) item,
        dspnew->options.edges_directed_show_p);
*/
  }

  for (i=0; i<nvisible; i++) {
    g_free (pos[i]);
    /*g_free (rowids[i]);*/
  }
  /*g_free (rowids);*/
  g_free (pos);
  g_free (visible);
  agclose (graph);
}

#endif
