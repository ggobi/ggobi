#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>
#include <math.h>

#include "plugin.h"
#include "defines.h"
#include "ggvis.h"

static void ggv_center_scale_pos_all (ggvisd *ggv);
void mds_open_display (PluginInstance *inst);

void
ggv_scramble (ggvisd *ggv, ggobid *gg)
{
  gint i, j;
/*
 * Fill in all data with normal random values ...
*/

  for (i = 0; i < ggv->pos.nrows; i++)
    for (j = 0; j < ggv->dim; j++)
      ggv->pos.vals[i][j] = ggv_randvalue(UNIFORM);

  ggv_center_scale_pos_all (ggv);
  update_ggobi (ggv, gg);
}

void
ggv_datad_create (datad *dsrc, datad *e, displayd *dsp, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst); 
  ggobid *gg = inst->gg;
  gint i, j;
  gint nc = ggv->dim;   /*-- default:  3d --*/
  gchar **rownames, **colnames, **rowids;
  gdouble *values;
  datad *dnew;
  InputDescription *desc = NULL;
  displayd *dspnew;
  vartabled *vt;
  gdouble range;

  rowids = (gchar **) g_malloc (dsrc->nrows * sizeof(gchar *));
  for (i=0; i<dsrc->nrows; i++) {
    rowids[i] = g_strdup (dsrc->rowIds[i]);
  }

  values = (gdouble *) g_malloc (dsrc->nrows * nc * sizeof(gdouble));
  rownames = (gchar **) g_malloc (dsrc->nrows * sizeof(gchar *));
  /* allocating pos; populating pos and values at the same time */
  if (ggv->pos.nrows < dsrc->nrows) {
    arrayd_alloc (&ggv->pos, dsrc->nrows, nc);
    for (j=0; j<nc; j++) {
      if (j < dsrc->ncols) {
        vt = vartable_element_get (j, dsrc);
        range = vt->lim_tform.max - vt->lim_tform.min;
        for (i=0; i<dsrc->nrows; i++)
          ggv->pos.vals[i][j] = values[i + j*dsrc->nrows] =
              (dsrc->tform.vals[i][j] - vt->lim_tform.min) / range;
      } else {
        for (i=0; i<dsrc->nrows; i++)
          ggv->pos.vals[i][j] = values[i + j*dsrc->nrows] =
              ggv_randvalue(UNIFORM);
      }
    }
  } else if (ggv->pos.ncols < nc) {
    gint nc_prev = ggv->pos.ncols;
    arrayd_add_cols (&ggv->pos, nc);
    for (j=nc_prev; j<nc; j++) {
      if (j < dsrc->ncols) {
        vt = vartable_element_get (j, dsrc);
        range = vt->lim_tform.max - vt->lim_tform.min;
        for (i=0; i<dsrc->nrows; i++)
          ggv->pos.vals[i][j] = values[i + j*dsrc->nrows] =
            (dsrc->tform.vals[i][j] - vt->lim_tform.min) / range;
      } else {
        for (i=0; i<dsrc->nrows; i++)
          ggv->pos.vals[i][j] = values[i + j*dsrc->nrows] =
              ggv_randvalue(UNIFORM);
      }
    }
  }
/* */

  for (i=0; i<dsrc->nrows; i++)
    rownames[i] = (gchar *) g_array_index (dsrc->rowlab, gchar *, i);

  colnames = (gchar **) g_malloc (nc * sizeof(gchar *));
  for (j=0; j<nc; j++)
    colnames[j] = g_strdup_printf ("Pos%d", j+1);

  /*
   * In case there is no initial scatterplot because the datasets
   * have no variables, we don't want creating a datad to trigger
   * the initialization of this plot.   This takes care of it.
  */

  GGOBI_getSessionOptions()->info->createInitialScatterPlot = false;

  /*-- --*/

  dnew = datad_create (dsrc->nrows, nc, gg);
  dnew->name = g_strdup ("MDS");
  dnew->nickname = g_strdup ("MDS");

  GGOBI(setData) (values, rownames, colnames, dsrc->nrows, nc, dnew,
    false, gg, rowids, false, desc);

  /*-- copy the color and glyph vectors from d to dnew --*/
  for (i=0; i<dsrc->nrows; i++) {
    dnew->color.els[i] = dnew->color_now.els[i] = dnew->color_prev.els[i] =
      dsrc->color.els[i];
    dnew->glyph.els[i].type = dnew->glyph_now.els[i].type =
      dnew->glyph_prev.els[i].type = dsrc->glyph.els[i].type;
    dnew->glyph.els[i].size = dnew->glyph_now.els[i].size =
      dnew->glyph_prev.els[i].size = dsrc->glyph.els[i].size;
  }

/*
 * open a new scatterplot with the new data, and display edges
 * as they're displayed in the current datad ... or not
*/
  dspnew = GGOBI(newScatterplot) (0, 1, dnew, gg);
  
  display_tailpipe (dspnew, FULL, gg);

  ggv->dpos = dnew;
  clusters_set (ggv->dpos, gg);

  g_free(values);
  g_free(colnames);
  g_free(rownames);
/*
  for (i=0; i<dsrc->nrows; i++)
    g_free (rowids[i]);
  g_free(rowids);
*/
}

static void
ggv_center_scale_pos_all (ggvisd *ggv)
{
  gint i, j;

  if (ggv->pos_mean.nels < ggv->dim)
    vectord_realloc (&ggv->pos_mean, ggv->dim);
  vectord_zero (&ggv->pos_mean);

  /* find center */
  for (j=0; j<ggv->pos.ncols; j++) {
    for (i=0; i<ggv->pos.nrows; i++)
      ggv->pos_mean.els[j] += ggv->pos.vals[i][j];
    ggv->pos_mean.els[j] /= ggv->pos.nrows;
  }

  /* find scale */
  ggv->pos_scl = 0.;
  for(i=0; i<ggv->pos.nrows; i++)
    for(j=0; j<ggv->pos.ncols; j++)
      ggv->pos_scl += fabs(ggv->pos.vals[i][j] - ggv->pos_mean.els[j]);
  ggv->pos_scl = ggv->pos_scl/(gdouble)ggv->pos.nrows/(gdouble)ggv->pos.ncols;

  /* center & scale */
  for (i=0; i<ggv->pos.nrows; i++)
    for (j=0; j<ggv->pos.ncols; j++)
      ggv->pos.vals[i][j] = (ggv->pos.vals[i][j] - ggv->pos_mean.els[j])/
                            ggv->pos_scl;

  vectord_zero (&ggv->pos_mean);
  ggv->pos_scl = 1.;
}

void
printminmax (gchar *cmt, ggvisd *ggv)
{
  gint i, j;
  gfloat min, max;
  max = min = ggv->pos.vals[0][0];
  for (i=0; i<ggv->pos.nrows; i++) {
    for (j=0; j<ggv->pos.ncols; j++) {
      min = MIN (ggv->pos.vals[i][j], min);
      max = MAX (ggv->pos.vals[i][j], max);
    }
  }
  g_printerr("%s min %f max %f\n", cmt, min, max);
}

/* Is this needed? */
void
ggv_pos_init (ggvisd *ggv)
{
  ggv_center_scale_pos_all (ggv);
}

void
ggv_pos_reinit (ggvisd *ggv)
{
  datad *dsrc = ggv->dsrc;
  gint i, j;
  gdouble min, range;
  vartabled *vt;

/*-- populates pos; what about dpos?  --*/
  for (j=0; j<ggv->dim; j++) {
    if (j < dsrc->ncols) {
      vt = vartable_element_get (j, dsrc);
      min = vt->lim_tform.min;
      range = vt->lim_tform.max - vt->lim_tform.min;
      for (i=0; i<dsrc->nrows; i++)
        ggv->pos.vals[i][j] = (dsrc->tform.vals[i][j] - min) / range;
    } else {
      for (i=0; i<dsrc->nrows; i++)
        ggv->pos.vals[i][j] = ggv_randvalue(UNIFORM);
    }
  }

  ggv_center_scale_pos_all (ggv);
}

void ggv_task_cb (GtkToggleButton *button, PluginInstance *inst)
{
  GtkWidget *w, *window;

  ggvisd *ggv = ggvisFromInst (inst); 
  window = (GtkWidget *) inst->data;

  if (button->active) {
    if (strcmp (gtk_widget_get_name (GTK_WIDGET(button)), "MDS") == 0) {
      ggv->mds_task = DissimAnalysis;
    } else {
      ggv->mds_task = GraphLayout;
    }

    w = widget_find_by_name (window, "MDS_WEIGHTS");
    gtk_widget_set_sensitive (w, ggv->mds_task == GraphLayout);
    w = widget_find_by_name (window, "MDS_COMPLETE");
    gtk_widget_set_sensitive (w, ggv->mds_task == GraphLayout);

    if (ggv->mds_task == DissimAnalysis)
      gtk_clist_select_row (ggv->clist_dist, 0, 0);
  }
}

void ggv_complete_distances_cb (GtkToggleButton *button, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->complete_Dtarget = button->active;
}
void ggv_edge_weights_cb (GtkToggleButton *button, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->Dtarget_source = (MDSDtargetSource) button->active;
  if (ggv->Dtarget_source == VarValues)
    gtk_clist_select_row (ggv->clist_dist, 0, 0);
}

/*
 * Required so that the histogram is properly drawn before mds_once
 * has executed.
*/
static void
trans_dist_init_defaults (ggvisd *ggv)
{
  gint i, j;

  vectord_realloc (&ggv->trans_dist, ggv->ndistances);
  for (i=0; i<ggv->Dtarget.nrows; i++) {
    for (j=0; j<ggv->Dtarget.nrows; j++) {
      /*IJ = i*ggv->Dtarget.ncols+j;*/
      if (ggv->KruskalShepard_classic == KruskalShepard) {
        ggv->trans_dist.els[IJ]  = ggv->Dtarget.vals[i][j];
      } else { /* CLASSIC */
        ggv->trans_dist.els[IJ]  = -ggv->Dtarget.vals[i][j]*
                                    ggv->Dtarget.vals[i][j];
      }
    }
  }
}

void
mds_open_display (PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  GtkWidget *window, *w;

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
  }

  if (!ggv->dpos) {
    gint j;
    vartabled *vt;
    /*-- initialize, allocate and populate dpos --*/
    ggv_datad_create (ggv->dsrc, ggv->e, gg->current_display, inst);
    ggv_pos_init (ggv);
    /*-- update limits here? could force them to be -1, 1  --*/
    /*limits_set (true, true, ggv->dpos, gg);*/

    /*
     * The initial data is on [-1, 1]; force the limits to be on [-2,2].
     * Rationale:  since the average distance from the mean is 1, we want
     * to guarantee that we have space for 2*1, ie, [mean-1, mean+1]
    */
    for (j=0; j<ggv->dpos->ncols; j++) {
      vt = vartable_element_get (j, ggv->dpos);
      vt->lim_tform.min = vt->lim_raw.min = vt->lim_display.min =
        vt->lim.min = -2.0;
      vt->lim_tform.max = vt->lim_raw.max = vt->lim_display.max =
        vt->lim.max = 2.0;
    }
  }
  
  window = (GtkWidget *) inst->data;
  w = widget_find_by_name (window, "Step");
  gtk_widget_set_sensitive (w, true);
}

void mds_open_display_cb (GtkWidget *btn, PluginInstance *inst)
{
  mds_open_display (inst);
}

void mds_run_cb (GtkToggleButton *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  gboolean state = btn->active;
  gint selected_var = -1;
  datad *dsrc;
  ggobid *gg = inst->gg;

  if (state) {

    gboolean first_time = false;
    gboolean new_dsrc = false;  /* node set */
    gboolean new_weights = false;


    /*
     * make sure we have a node set, an edge set, and (if needed)
     * a source of distances or edge weights 
    */

    /* Make sure the node set is present and can support edges */
    if (ggv->dsrc == NULL || ggv->dsrc->rowIds == NULL) {
      g_printerr ("node set not correctly specified\n");
      return;
    }
    dsrc = ggv->dsrc;
    /* Make sure there's an edge set */
    ggv->e = gtk_object_get_data (GTK_OBJECT(ggv->clist_dist), "datad");
    if (ggv->e == NULL || ggv->e->edge.n == 0) {
      g_printerr ("edge set not correctly specified\n");
      return;
    }
    /*
     * If we are using a distance vector or a vector of edge weights, make
     * sure a variable has been selected.  (Do the variable values have
     * to be non-negative?)
    */
    if (ggv->mds_task == DissimAnalysis || ggv->Dtarget_source == VarValues) {
      selected_var = get_one_selection_from_clist (GTK_WIDGET(ggv->clist_dist),
        ggv->e);
      if (selected_var == -1) {
        quick_message ("Please specify a variable", false);
        return;
      }
    }

    /* 
     * Check the settings on a few flags so we'll know what has to
     * be initialized or re-initialized
     */
    if (ggv->Dtarget.nrows == 0)
      first_time = true;
    else if (ggv->Dtarget.nrows != ggv->dsrc->nrows)
      new_dsrc = true;
    /* possible bug:  if the user resets this variable while mds is
       running, the change won't show up here */
    if (ggv->mds_task == DissimAnalysis || ggv->Dtarget_source == VarValues) {
      if (selected_var != ggv->weight_var) {
        new_weights = true;
        ggv->weight_var = selected_var;
      }
    }

    /*
     * If this is the first time through or the node set changed,
     * allocate space for the distance matrix. (There's no need
     * to free it before allocating; arrayd_alloc handles that.)
     */
    if (first_time || new_dsrc)
      arrayd_alloc (&ggv->Dtarget, dsrc->nrows, dsrc->nrows);

    /*
     * If anything changed, construct a distance matrix, and
     * make sure we were successful.  Initialize (or reinitialize)
     * some variables.  [We'll need more reinitialization here
     * if new_weights or new_dsrc -- see what's initialized in
     * init.c]
    */
    if (new_weights || first_time || new_dsrc) {
      ggv_init_Dtarget (ggv->weight_var, ggv);  /* populate with INF */
      ggv_compute_Dtarget (ggv->weight_var, ggv);
      if (ggv->Dtarget.nrows == 0) {
        quick_message ("I can't identify a distance matrix", false);
        return;
      }
      g_printerr ("%d x %d\n", ggv->Dtarget.nrows, ggv->Dtarget.ncols);

      trans_dist_init_defaults (ggv);
    }

    if (first_time) {
      /*-- open display --*/
      mds_open_display (inst);
    }

    ggv_Dtarget_histogram_update (ggv, gg);
  }

  mds_func (state, inst);
}

void mds_step_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
  }

  mds_once (true, ggv, gg);
  update_ggobi (ggv, gg);
}
void mds_reinit_cb (PluginInstance *inst, guint action, GtkWidget *w)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
  }
  if (ggv->pos.nrows == 0) {
    quick_message ("First, open a display", false);
    return;
  }

  ggv_pos_reinit (ggv);
  update_ggobi (ggv, gg);
}

void mds_scramble_cb (PluginInstance *inst, guint action, GtkWidget *w)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
  }
  if (ggv->pos.nrows == 0) {
    quick_message ("First, open a display", false);
    return;
  }

  ggv_scramble (ggv, gg);
  update_ggobi (ggv, gg);
}

void
mds_reset_params_cb (PluginInstance *inst, guint action, GtkWidget *widget)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  GtkWidget *window, *w;
  GtkAdjustment *adj;

  window = (GtkWidget *) inst->data;

/*
 * I don't know if it's necessary to reset all these parameters.
*/

  ggv->KruskalShepard_classic = KruskalShepard;  /*-- an option menu --*/
  w = widget_find_by_name (window, "kruskalshepard_classic_opt");
  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
    (gint) ggv->KruskalShepard_classic);
  
  ggv->stepsize = 0.02;     /*-- reset a slider --*/
  w = widget_find_by_name (window, "stepsize_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->stepsize);

  ggv->dist_power = 1.0;    /*-- reset a slider --*/
  w = widget_find_by_name (window, "dist_power_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->dist_power);

/*
 * an option menu, which has control over a slider behaves,
 * because it determines which adjustment is attached to it.
 * That's the Dtarget_power_scale.
*/
  ggv->metric_nonmetric = metric;
  w = widget_find_by_name (window, "metric_opt");
  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
    (gint) ggv->metric_nonmetric);
{  /*-- make sure the appropriate adjustment is attached to the hscale --*/
  GtkWidget *menu = gtk_option_menu_get_menu (GTK_OPTION_MENU(w));
  GList *children = gtk_container_children (GTK_CONTAINER(menu));
  GtkWidget *item = (GtkWidget *) children->data;  /*-- first one --*/
  ggv_metric (item, 0);
}

/*
   We don't reset the other adjustment that may be associated
   with this scale, the isotonic_mix_adj.  See ggv_metric_cb.
*/
  ggv->Dtarget_power = 1.0;  /*-- reset a slider --*/
  w = widget_find_by_name (window, "Dtarget_power_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->Dtarget_power);


  ggv->lnorm = 2.0;
  w = widget_find_by_name (window, "lnorm_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->lnorm);

  ggv->weight_power = 0.0;
  w = widget_find_by_name (window, "weight_power_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->weight_power);

/*
  ggv->isotonic_mix = 1.0;
  ggv->within_between = 1.0;
*/
  ggv->rand_select_new = false;
  ggv->rand_select_val = 1.0;  /* selection probability */
  w = widget_find_by_name (window, "selection_prob_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->rand_select_val);

  ggv->perturb_val = 1.0;
  w = widget_find_by_name (window, "perturbation_scale");
  adj = gtk_range_get_adjustment (GTK_RANGE(w));
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), ggv->perturb_val);

  /*ggv->group_p = false;*/     /*-- toggle button --*/
  /*ggv->group_ind = within;*/  /*-- option menu --*/

  if (ggv->Dtarget.nrows == 0 || ggv->pos.nrows == 0) {
    return;
  }

  update_ggobi (ggv, gg);
}

void ggv_stepsize_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->stepsize = adj->value;
}

void ggv_dims_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  gint dim = (gint) adj->value;
  gint i, j;
  gchar *vname;
  gboolean running = ggv->running_p;
  datad *d = ggv->dpos;
  gdouble *dtmp;

  if (ggv->dpos) {

    if (ggv->running_p) mds_func (false, inst);
  
    if (dim > ggv->maxdim) {  /*-- add variables as needed --*/
      arrayd_add_cols (&ggv->pos, dim);
      vectord_realloc (&ggv->pos_mean, dim);

      dtmp = (gdouble *) g_malloc0 (d->nrows * sizeof (gdouble));
      for (i=0; i<d->nrows; i++)
        dtmp[i] = 2 * (randvalue() - .5);  /* on [-1, 1] */
      for (j=ggv->dim; j<dim; j++) {
        vname = g_strdup_printf ("Pos%d", j+1);
        newvar_add_with_values (dtmp, d->nrows, vname,
          real, 0, (gchar **) NULL, (gint *) NULL, (gint *) NULL,
        d, gg);
        g_free (vname);
      }
      g_free (dtmp);
      ggv->maxdim = MAX(ggv->dim, dim);
    }
    ggv->dim = dim;
  }

  if (ggv->dpos) {
    if (running) mds_func (true, inst);
  }
}

void ggv_dist_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->dist_power = adj->value;

  ggv->lnorm_over_dist_power = ggv->lnorm/ggv->dist_power;
  ggv->dist_power_over_lnorm = ggv->dist_power/ggv->lnorm;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0 || ggv->pos.nrows == 0)
    return;

  mds_once (false, ggv, gg);
  ggv_Dtarget_histogram_update (ggv, gg);
}

/*
   This callback is attached to two adjustments:
     one controls Dtarget_power  (range: 1:6)
     the other controls isotonic_mix (range: 0:1)
*/
void ggv_Dtarget_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);

  /*-- I'm trusting that the adjustment is in sync with the option menu --*/
  if (ggv->metric_nonmetric == metric) {
    ggv->Dtarget_power = adj->value;
  } else {
    ggv->isotonic_mix = adj->value / 100.0;
  }

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0 || ggv->pos.nrows == 0)
    return;

  mds_once (false, ggv, gg);
  ggv_Dtarget_histogram_update (ggv, gg);
}
void ggv_lnorm_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->lnorm = adj->value;
}
void ggv_weight_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->weight_power = adj->value;
}

void ggv_metric (GtkWidget *w, gint param)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);
  GtkWidget *label, *hscale;
  GtkAdjustment *Dtarget_adj, *isotonic_mix_adj;

  ggv->metric_nonmetric = (MDSMetricInd) param;

  label = gtk_object_get_data (GTK_OBJECT(w), "label");
  hscale = gtk_object_get_data (GTK_OBJECT(w), "hscale");
  Dtarget_adj = gtk_object_get_data (GTK_OBJECT(w), "Dtarget_adj");
  isotonic_mix_adj = gtk_object_get_data (GTK_OBJECT(w), "isotonic_mix_adj");

  if (ggv->metric_nonmetric == metric) {
    if (GTK_RANGE(hscale)->adjustment != Dtarget_adj) {
      /*
       * add to the ref count of the adjust to be removed, because
       * gtk_range_set_adjustment will decrease its ref count.
      */
      gtk_object_ref (GTK_OBJECT(isotonic_mix_adj));
      gtk_range_set_adjustment (GTK_RANGE(hscale), Dtarget_adj);
      gtk_label_set_text (GTK_LABEL(label), "Data power (D^p)");
    }
  } else {
    if (GTK_RANGE(hscale)->adjustment != isotonic_mix_adj) {
      gtk_object_ref (GTK_OBJECT(Dtarget_adj));
      gtk_range_set_adjustment (GTK_RANGE(hscale), isotonic_mix_adj);
      gtk_label_set_text (GTK_LABEL(label), "Isotonic(D) (%)");
    }
  }

}

void ggv_metric_cb (GtkWidget *w, gpointer cbd)
{
  gint param = (MDSMetricInd) GPOINTER_TO_INT (cbd);
  ggv_metric (w, param);
}
void ggv_kruskal_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->KruskalShepard_classic = (MDSKSInd) GPOINTER_TO_INT (cbd);
}


void ggv_groups_cb (GtkToggleButton *button,  gpointer cbd)
{
  /* all_distances, within, between */
  if (button->active) {
    PluginInstance *inst = (PluginInstance *)
      gtk_object_get_data (GTK_OBJECT(button), "PluginInst");
    ggvisd *ggv = ggvisFromInst (inst);

    ggv->group_ind = GPOINTER_TO_INT (cbd);

    /* If we just turned groups on, make sure anchor is off */
    if (ggv->group_ind != 0) {
      if (ggv->anchor_ind != 0) {
        GtkWidget *window = (GtkWidget *) inst->data;
        GtkWidget *w = widget_find_by_name (window, "ANCHOR_OFF");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);
      }
    }
  }
}
void ggv_anchor_cb (GtkToggleButton *button, gpointer cbd)
{
  /* no_anchor, scaled, fixed */
  if (button->active) {
    PluginInstance *inst = (PluginInstance *)
      gtk_object_get_data (GTK_OBJECT(button), "PluginInst");
    ggvisd *ggv = ggvisFromInst (inst);

    ggv->anchor_ind = GPOINTER_TO_INT (cbd);

    /* If we just turned anchor on, make sure groups is off */
    if (ggv->anchor_ind != 0) {
      if (ggv->group_ind != 0) {
        GtkWidget *window = (GtkWidget *) inst->data;
        GtkWidget *w = widget_find_by_name (window, "GROUPS_OFF");
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);
      }
    }
  }
}

void
ggv_perturb_adj_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->perturb_val = adj->value;
}

void
ggv_perturb_btn_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  gint i, k;
  ggobid *gg = inst->gg;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0 || ggv->pos.nrows == 0)
    return;

  for (i = 0; i < ggv->pos.nrows; i++)
    for (k = ggv->freeze_var; k < ggv->dim; k++) {
      ggv->pos.vals[i][k] = (1.0-ggv->perturb_val) * ggv->pos.vals[i][k] +
        (ggv->perturb_val) * ggv_randvalue(NORMAL);
    }

  ggv_center_scale_pos (ggv);
  update_ggobi (ggv, gg);

  update_stress (ggv, gg);
}

void
ggv_selection_prob_adj_cb (GtkAdjustment *adj, PluginInstance *inst)
{
/*
 * Adjust the probability of random selection of a dist/diss
*/
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->rand_select_val = adj->value;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0 || ggv->pos.nrows == 0)
    return;

  mds_once (true, ggv, gg);
  update_ggobi (ggv, gg);
}
void
ggv_selection_prob_btn_cb (GtkWidget *btn, PluginInstance *inst)
{
/*
 * Call for new random selection vector
*/
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->rand_select_new = true;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0 || ggv->pos.nrows == 0)
    return;

  mds_once (true, ggv, gg);
  update_ggobi (ggv, gg);
}

void ggv_constrained_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
     gtk_object_get_data (GTK_OBJECT (w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);
  /*-- 0: no variables frozen        --*/
  /*-- 1: first variable frozen      --*/
  /*-- 2: first two variables frozen --*/
  ggv->freeze_var = GPOINTER_TO_INT (cbd);
}
