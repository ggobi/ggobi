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

void
ggv_scramble (ggvisd *ggv, ggobid *gg)
{
  gint i, j;
/*
 * Fill in all data with normal random values ...
*/

  for (i = 0; i < ggv->pos.nrows; i++)
    for (j = 0; j < ggv->mds_dims; j++)
      ggv->pos.vals[i][j] = ggv_randvalue(UNIFORM);

  ggv_center_scale_pos_all (ggv);
  update_ggobi (ggv, gg);
}

void
ggv_datad_create (datad *dsrc, datad *e, displayd *dsp, ggvisd *ggv, ggobid *gg)
{
  gint i, j;
  gint nc = ggv->mds_dims;   /*-- default:  3d --*/
  glong *rowids;
  gchar **rownames, **colnames;
  gdouble *values;
  datad *dnew;
  InputDescription *desc = NULL;
  displayd *dspnew;
  gboolean edges_displayed;
  vartabled *vt;
  gdouble range;

  rowids = (glong *) g_malloc (dsrc->nrows * sizeof(glong));
  for (i=0; i<dsrc->nrows; i++) {
    rowids[i] = (glong) dsrc->rowid.id.els[i];
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
  colnames[0] = "Pos1";
  colnames[1] = "Pos2";
  colnames[2] = "Pos3";
  dnew = datad_create (dsrc->nrows, nc, gg);
  dnew->name = g_strdup ("MDS");

  GGOBI(setData) (values, rownames, colnames, dsrc->nrows, nc, dnew,
    false, gg, rowids, desc);

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
/* setDisplayEdge (dspnew, e); */
  edges_displayed = display_copy_edge_options (dsp, dspnew);
  if (!edges_displayed) {
    /*GGOBI(setShowLines)(dspnew, true);*/
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

  display_tailpipe (dspnew, FULL, gg);

  ggv->dpos = dnew;

  g_free(values);
  g_free(colnames);
  g_free(rownames);
  g_free(rowids);
}

static void
ggv_center_scale_pos_all (ggvisd *ggv)
{
  gint i, j;

  if (ggv->pos_mean.nels < ggv->mds_dims)
    vectord_realloc (&ggv->pos_mean, ggv->mds_dims);
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
/*
  gint i, j, m;
  datad *dpos = ggv->dpos;
  gdouble **pos = ggv->pos.vals;

  for (m=0; m<dpos->nrows_in_plot; m++) {
    i = dpos->rows_in_plot[m];
    for (j=0; j<dpos->ncols; j++) {
      pos[i][j] = dpos->tform.vals[i][j] ;
    }
  }
*/
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
  for (j=0; j<ggv->mds_dims; j++) {
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
  gint i, j, IJ;;

  vectord_realloc (&ggv->trans_dist, ggv->ndistances);
  for (i=0; i<ggv->Dtarget.nrows; i++) {
    for (j=0; j<ggv->Dtarget.nrows; j++) {
      IJ = i*ggv->Dtarget.ncols+j;
      if (ggv->KruskalShepard_classic == KruskalShepard) {
        ggv->trans_dist.els[IJ]  = ggv->Dtarget.vals[i][j];
      } else { /* CLASSIC */
        ggv->trans_dist.els[IJ]  = -ggv->Dtarget.vals[i][j]*
                                    ggv->Dtarget.vals[i][j];
      }
    }
  }

}


/*
 * This code should actually be moved out of the callback
 * and stashed someplace else.
*/
void ggv_compute_Dtarget_cb (GtkWidget *button, PluginInstance *inst)
{
/*
  GtkWidget *notebook = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(button), "notebook");
*/
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  GtkWidget *clist;
  gint i, j, selected_var;
  gdouble infinity;
  gboolean changing;
  gint end1, end2, end3;
  gdouble d12;
  gdouble **Dvals;
  endpointsd *endpoints;
  datad *dsrc, *e;
  displayd *dsp = gg->current_display;
  GtkWidget *window, *entry;
  gchar *lbl;
  gdouble dtmp;

  /*
   * this is awkward -- it allows dsrc to be initialized once
   *   but never reset
  */
  if (!ggv->dsrc)
    ggv->dsrc = dsp->d;
  dsrc = ggv->dsrc;
 
/*
How big is this distance matrix?  if (ggv->complete_Dtarget), then
it can be larger than e->edge.n.    Perhaps it should be of
dimension d->nrows x d->nrows.
Sometimes it will be extremely sparse, but then we probably
aren't running ggvis on extremely large data, so maybe the
large size of it isn't important.
*/

  /*-- allocate Dtarget --*/
  arrayd_alloc (&ggv->Dtarget, dsrc->nrows, dsrc->nrows);
  /*-- initalize Dtarget --*/
  infinity = (gdouble) (2 * dsrc->nrows);
  for (i=0; i<dsrc->nrows; i++) {
    for (j=0; j<dsrc->nrows; j++)
      ggv->Dtarget.vals[i][j] = infinity;
    ggv->Dtarget.vals[i][i] = 0.0;
  }

  ggv->e = gg->current_display->e;
  if (ggv->Dtarget_source == VarValues) {
    clist = get_clist_from_object (GTK_OBJECT (button));
    if (!clist) {
      quick_message ("I can't identify a set of edges", false);
      return;
    }
    /*-- this is the edgeset for distance computations, not
         necessarily the edgeset to be displayed (eg, morsecode) --*/
    ggv->e = gtk_object_get_data (GTK_OBJECT(clist), "datad");
    if (!ggv->e) {
      quick_message ("I can't identify a set of edges", false);
      return;
    }
    selected_var = get_one_selection_from_clist (clist, ggv->e);
  }
  if (!ggv->e) {
    if (!edgeset_add (dsp)) {
      quick_message ("Please specify an edge set", false);
      return;
    } else {
      ggv->e = dsp->e;
    }
  }
  e = ggv->e;

  Dvals = ggv->Dtarget.vals;
  endpoints = e->edge.endpoints;

  /*-- populate --*/
  if (!ggv->complete_Dtarget) {
    for (i = 0; i < e->edge.n; i++) {
      end1 = dsrc->rowid.idv.els[endpoints[i].a];
      end2 = dsrc->rowid.idv.els[endpoints[i].b];
      Dvals[end1][end2] = (ggv->Dtarget_source == VarValues) ?
        e->tform.vals[i][selected_var] : 1.0;
    }
  } else {  /*-- complete Dtarget using a shortest path algorithm --*/

    changing = true;
    while (changing) {
      changing = false;
      for (i = 0; i < e->edge.n; i++) {
        end1 = dsrc->rowid.idv.els[endpoints[i].a];
        end2 = dsrc->rowid.idv.els[endpoints[i].b];
        d12 = (ggv->Dtarget_source == VarValues) ?
          e->tform.vals[i][selected_var] : 1.0;

        for (end3 = 0; end3 < dsrc->nrows; end3++) {
          /* So we have a direct link from end1 to end2.  Can this be */
          /* used to shortcut a path from end1 to end3 or end2 to end3? */
          if (Dvals[end1][end3] > d12 + Dvals[end2][end3]) {
            Dvals[end3][end1] = Dvals[end1][end3] = d12 + Dvals[end2][end3];
            changing = true;
          }
          if (Dvals[end2][end3] > d12 + Dvals[end1][end3]) {
            Dvals[end3][end2] = Dvals[end2][end3] = d12 + Dvals[end1][end3];
            changing = true;
          }
        }    /* end3 */
      }    /* end1 and end2 */
    }    /* while changing. */
  }
  ggv->ndistances = ggv->Dtarget.nrows * ggv->Dtarget.ncols;

  ggv->Dtarget_max = DBL_MIN;  ggv->Dtarget_min = DBL_MAX;
  for (i=0; i<ggv->Dtarget.nrows; i++) {
    for (j=0; j<ggv->Dtarget.ncols; j++) {
      dtmp = ggv->Dtarget.vals[i][j]; 
      if (dtmp < 0) {
        g_printerr ("negative dissimilarity: i=%d j=%d diss=%3.6f -> NA\n",
          i, j, dtmp);
        dtmp = ggv->Dtarget.vals[i][j] = DBL_MAX;
      }
      if(dtmp != DBL_MAX) {
        if (dtmp > ggv->Dtarget_max) ggv->Dtarget_max = dtmp;
        if (dtmp < ggv->Dtarget_min) ggv->Dtarget_min = dtmp;
      }
    }
  }
  ggv->mds_threshold_low =  ggv->Dtarget_min;
  ggv->mds_threshold_high = ggv->Dtarget_max;

  /*-- update the entry to let people know Dtarget has been computed --*/
  window = (GtkWidget *) inst->data;
  entry = (GtkWidget *) gtk_object_get_data (GTK_OBJECT(window),
    "DTARGET_ENTRY");
  lbl = g_strdup_printf ("%d x %d", ggv->Dtarget.nrows, ggv->Dtarget.ncols);
  gtk_entry_set_text (GTK_ENTRY (entry), lbl);
  g_free (lbl);

  trans_dist_init_defaults (ggv);
}

/*-- --*/

void mds_run_cb (GtkToggleButton *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;
  gboolean state = btn->active;

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
  if (!ggv->dpos) {
    /*-- initialize, allocate and populate dpos --*/
    ggv_datad_create (ggv->dsrc, ggv->e, gg->current_display, ggv, gg);
    ggv_pos_init (ggv);
  }
  mds_once (true, ggv, gg);
  update_ggobi (ggv, gg);
}
void mds_reinit_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
  }
  if (!ggv->dpos) {
    /*-- initialize, allocate and populate dpos --*/
    ggv_datad_create (ggv->dsrc, ggv->e, gg->current_display, ggv, gg);
    ggv_pos_init (ggv);
  }
  ggv_pos_reinit (ggv);
  update_ggobi (ggv, gg);
}
void mds_scramble_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggobid *gg = inst->gg;

  if (ggv->Dtarget.nrows == 0) {
    quick_message ("I can't identify a distance matrix", false);
    return;
  }
  if (!ggv->dpos) {
    /*-- initialize, allocate and populate dpos --*/
    ggv_datad_create (ggv->dsrc, ggv->e, gg->current_display, ggv, gg);
    ggv_pos_init (ggv);
  }
  ggv_scramble (ggv, gg);
  update_ggobi (ggv, gg);
}


void ggv_stepsize_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_stepsize = adj->value;
 g_printerr ("mds_stepsize = %f\n", ggv->mds_stepsize);
}

void ggv_dims_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_dims = (gint) (adj->value);
}
void ggv_dist_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_dist_power = adj->value;

  ggv->mds_lnorm_over_dist_power = ggv->mds_lnorm/ggv->mds_dist_power;
  ggv->mds_dist_power_over_lnorm = ggv->mds_dist_power/ggv->mds_lnorm;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0)
    return;

  mds_once (false, ggv, gg);
  ggv_Dtarget_histogram_update (ggv, gg);
}

/*
   This callback is attached to two adjustments:
     one controls mds_Dtarget_power  (range: 1:6)
     the other controls mds_isotonic_mix (range: 0:1)
*/
void ggv_Dtarget_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  ggvisd *ggv = ggvisFromInst (inst);

  /*-- I'm trusting that the adjustment is in sync with the option menu --*/
  if (ggv->metric_nonmetric == metric) {
    ggv->mds_Dtarget_power = adj->value;
  } else {
    ggv->mds_isotonic_mix = adj->value / 100.0;
  }

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0)
    return;

  mds_once (false, ggv, gg);
  ggv_Dtarget_histogram_update (ggv, gg);
}
void ggv_lnorm_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_lnorm = adj->value;
}
void ggv_weight_power_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_weight_power = adj->value;
}

void ggv_metric_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);
  GtkWidget *label, *hscale;
  GtkAdjustment *Dtarget_adj, *isotonic_mix_adj;

  ggv->metric_nonmetric = (MDSMetricInd) GPOINTER_TO_INT (cbd);

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
void ggv_kruskal_cb (GtkWidget *w, gpointer cbd)
{
  PluginInstance *inst = (PluginInstance *)
    gtk_object_get_data (GTK_OBJECT(w), "PluginInst");
  ggvisd *ggv = ggvisFromInst (inst);

  ggv->KruskalShepard_classic = (MDSKSInd) GPOINTER_TO_INT (cbd);
 g_printerr ("KruskalShepardInd = %d\n", ggv->KruskalShepard_classic);
}


void ggv_groups_cb (GtkWidget *w, PluginInstance *inst)
{
}

void
ggv_perturb_adj_cb (GtkAdjustment *adj, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  ggv->mds_perturb_val = adj->value;
}

void
ggv_perturb_btn_cb (GtkWidget *btn, PluginInstance *inst)
{
  ggvisd *ggv = ggvisFromInst (inst);
  gint i, k;
  ggobid *gg = inst->gg;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0)
    return;

  for (i = 0; i < ggv->pos.nrows; i++)
    for (k = ggv->mds_freeze_var; k < ggv->mds_dims; k++) {
      ggv->pos.vals[i][k] = (1.0-ggv->mds_perturb_val) * ggv->pos.vals[i][k] +
        (ggv->mds_perturb_val) * ggv_randvalue(NORMAL);
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

  ggv->mds_rand_select_val = adj->value;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0)
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

  ggv->mds_rand_select_new = true;

  /*-- sanity check before execution --*/
  if (ggv->Dtarget.nrows == 0)
    return;

  mds_once (true, ggv, gg);
  update_ggobi (ggv, gg);
}

void ggv_constrained_cb (GtkWidget *w, PluginInstance *inst)
{
}
