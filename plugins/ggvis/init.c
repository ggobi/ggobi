#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

void
ggvis_init (ggvisd *ggv)
{
  /*-- initialize the datad pointers --*/
  ggv->dsrc = NULL;
  ggv->dpos = NULL;
  ggv->e = NULL;

  ggv->mds_running = false;
  ggv->idle_id = 0;

  arrayd_init_null (&ggv->Dtarget);
  arrayd_init_null (&ggv->pos);

  ggv->stressplot_pix = NULL;
  ggv->nstressvalues = 0;
  vectord_init_null (&ggv->stressvalues);
  vectord_alloc (&ggv->stressvalues, NSTRESSVALUES);

  ggv->histogram_pix = NULL;

  ggv->mds_dims = 3;

  ggv->mds_stepsize = 0.02;
  ggv->mds_dist_power = 1.0;
  ggv->mds_Dtarget_power = 1.0;
  ggv->mds_lnorm = 2.0;
  ggv->mds_dist_power_over_lnorm = 1.0;
  ggv->mds_weight_power = 0.0;

  ggv->mds_isotonic_mix = 1.0;
  ggv->mds_dist_power_over_lnorm = 0.5;
  ggv->mds_lnorm_over_dist_power = 2.0;
  ggv->mds_within_between = 1.0;
  ggv->mds_rand_select_val = 1.0;
  ggv->mds_rand_select_new = false;
  ggv->mds_perturb_val = 1.0;
  ggv->mds_threshold_high = 0.0;
  ggv->mds_threshold_low = 0.0;

  ggv->metric_nonmetric = metric;
  ggv->KruskalShepard_classic = KruskalShepard;

  ggv->Dtarget_source = LinkDist;
  ggv->complete_Dtarget = false;

  ggv->mds_group_ind = deflt;

  /*-- used in mds.c --*/
  vectord_init_null (&ggv->pos_mean);
  vectord_init_null (&ggv->weights);
  vectord_init_null (&ggv->rand_sel);
  vectord_init_null (&ggv->trans_dist);
  vectord_init_null (&ggv->config_dist);
  vectori_init_null (&ggv->point_status);
  vectori_init_null (&ggv->trans_dist_index);
  vectori_init_null (&ggv->bl);
  vectord_init_null (&ggv->bl_w);
  arrayd_init_null (&ggv->gradient);

  ggv->pos_scl = 0.0;
  ggv->mds_rand_select_val = 1.0;
  ggv->mds_freeze_var = 0;
  ggv->dist_max = 0.0;
  ggv->prev_nonmetric_active_dist = 0;
  /* */
}

