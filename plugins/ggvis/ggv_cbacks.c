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
ggv_datad_create (datad *dsrc, datad *e, displayd *dsp, ggvisd *ggv, ggobid *gg)
{
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
  sessionOptions->info->createInitialScatterPlot = false;
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

/*
 * Definition of D
*/
void ggv_dsource_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
     gtk_object_get_data (GTK_OBJECT (w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->Dtarget_source = (MDSDtargetSource) GPOINTER_TO_INT (cbd);
}
void ggv_complete_distances_cb (GtkToggleButton *button, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->complete_Dtarget = button->active;
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

void ggv_compute_Dtarget_cb (GtkWidget *button, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  GtkWidget *clist;
  gint selected_var = -1;
  datad *dsrc;
  GtkWidget *window, *entry;
  gchar *lbl;

  /* This was set in the first clist in the preceding tab */
  if (ggv->dsrc == NULL || ggv->dsrc->rowIds == NULL) {
    g_printerr ("node set not correctly specified\n");
    return;
  }
  if (ggv->e == NULL || ggv->e->edge.n == 0) {
    g_printerr ("edge set not correctly specified\n");
    return;
  }

  dsrc = ggv->dsrc; 
 
  /*-- allocate Dtarget --*/
  arrayd_alloc (&ggv->Dtarget, dsrc->nrows, dsrc->nrows);

  if (ggv->Dtarget_source == VarValues) {
    clist = get_clist_from_object (GTK_OBJECT (button));
    if (!clist) {
      quick_message ("I can't identify a set of edges", false);
      return;
    }
    /*-- this is the edgeset for distance computations, not
         necessarily the edgeset to be displayed (eg, morsecode) --*/
/* can override the setting of the second clist in the preceding tab */
    ggv->e = gtk_object_get_data (GTK_OBJECT(clist), "datad");
g_printerr ("e name = %s\n", ggv->e->name);
    if (ggv->e == NULL) {
g_printerr ("e is null\n");
      quick_message ("I can't identify a set of edges", false);
      return;
    }
    selected_var = get_one_selection_from_clist (clist, ggv->e);
    if (selected_var == -1) {
      quick_message ("Please specify a variable", false);
      return;
    }
  }

/*
  if (ggv->e == NULL) {
    if (!edgeset_add (dsp)) {
      quick_message ("Please specify an edge set", false);
      return;
    } else {
      ggv->e = dsp->e;
    }
  }
*/

  if (dsrc->rowIds == NULL) {
    g_printerr ("Make sure the current display is a plot of the nodes.\n");
    g_printerr ("  Currently nodes: %s edges: %s\n", dsrc->name, ggv->e->name);
    return;
  }

  /*-- initalize Dtarget --*/
  ggv_init_Dtarget (selected_var, ggv);
/*
  infinity = (gdouble) (2 * dsrc->nrows);
  for (i=0; i<dsrc->nrows; i++) {
    for (j=0; j<dsrc->nrows; j++)
      ggv->Dtarget.vals[i][j] = infinity;
    ggv->Dtarget.vals[i][i] = 0.0;
  }
*/

  ggv_compute_Dtarget (selected_var, ggv);


  /*-- update the entry to let people know Dtarget has been computed --*/
  window = (GtkWidget *) inst->data;
  entry = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(window),
    "DTARGET_ENTRY");
  lbl = g_strdup_printf ("%d x %d", ggv->Dtarget.nrows, ggv->Dtarget.ncols);
  gtk_entry_set_text (GTK_ENTRY (entry), lbl);
  g_free (lbl);

  trans_dist_init_defaults (ggv);

  /*-- open display --*/
  mds_open_display (inst);
}

/*-- --*/

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
    ggv_datad_create (ggv->dsrc, ggv->e, gg->current_display, ggv, gg);
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
  w = widget_find_by_name (window, "RunMDS");
  gtk_widget_set_sensitive (w, true);
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

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
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
/*void mds_scramble_cb (GtkWidget *btn, PluginInstance *inst)*/
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
 g_printerr ("stepsize = %f\n", ggv->stepsize);
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
  
    if (dim > ggv->dim) {  /*-- add variables as needed --*/

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

    }
  }

  ggv->dim = dim;

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
 g_printerr ("KruskalShepardInd = %d\n", ggv->KruskalShepard_classic);
}


void ggv_brush_groupsp_cb (GtkToggleButton *w, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->group_p = w->active;
}
void ggv_brush_groups_opt_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  /* within, between, anchorscales, anchorfixed */
  ggv->group_ind = GPOINTER_TO_INT (cbd);
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
