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

  ggv->running_p = false;
  ggv->idle_id = 0;

  arrayd_init_null (&ggv->Dtarget);
  arrayd_init_null (&ggv->pos);

  ggv->stressplot_pix = NULL;
  ggv->nstressvalues = 0;
  vectord_init_null (&ggv->stressvalues);
  vectord_alloc (&ggv->stressvalues, NSTRESSVALUES);

  ggv->dissim = (dissimd *) g_malloc (sizeof (dissimd));
  ggv->dissim->pix = NULL;
  ggv->dissim->low = 0.;
  ggv->dissim->high = 1.;
  ggv->dissim->lgrip_pos = -1;
  ggv->dissim->rgrip_pos = -1;
  ggv->dissim->bars = NULL;
  vectorb_init_null (&ggv->dissim->bars_included);
  vectori_init_null (&ggv->dissim->bins);

  ggv->dim = 3;

  ggv->stepsize = 0.02;
  ggv->dist_power = 1.0;
  ggv->Dtarget_power = 1.0;
  ggv->lnorm = 2.0;
  ggv->dist_power_over_lnorm = 1.0;
  ggv->weight_power = 0.0;

  ggv->isotonic_mix = 1.0;
  ggv->dist_power_over_lnorm = 0.5;
  ggv->lnorm_over_dist_power = 2.0;
  ggv->within_between = 1.0;
  ggv->rand_select_val = 1.0;  /* selection probability */
  ggv->rand_select_new = false;
  ggv->perturb_val = 1.0;
  ggv->threshold_high = 0.0;
  ggv->threshold_low = 0.0;

  ggv->metric_nonmetric = metric;
  ggv->KruskalShepard_classic = KruskalShepard;

  ggv->num_active_dist = 0;

  ggv->Dtarget_source = LinkDist;
  ggv->complete_Dtarget = false;

  ggv->group_p = false;
  ggv->group_ind = within;
  vectorb_init_null (&ggv->anchor_group);

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
  ggv->freeze_var = 0;
  ggv->Dtarget_max = DBL_MAX;
  ggv->Dtarget_min = DBL_MIN;
  ggv->prev_nonmetric_active_dist = 0;
  /* */

  ggv->shepard_iter = 0;
}

