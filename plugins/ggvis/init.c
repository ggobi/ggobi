#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

void
ggvis_init (ggvisd *ggv) {
  arrayd_init_null (&ggv->dist_orig);
  arrayd_init_null (&ggv->dist);
  arrayd_init_null (&ggv->pos_orig);
  arrayd_init_null (&ggv->pos);

  ggv->stressplot_pix = NULL;
  ggv->histogram_pix = NULL;

  ggv->mds_dims = 3;

  ggv->mds_stepsize = 0.02;
  ggv->mds_power = 1.0;
  ggv->mds_dist_power = 1.0;
  ggv->mds_lnorm = 2.0;
  ggv->mds_weight_power = 0.0;

  ggv->mds_isotonic_mix = 1.0;
  ggv->mds_distpow_over_lnorm = 0.5;
  ggv->mds_lnorm_over_distpow = 2.0;
  ggv->mds_within_between = 1.0;
  ggv->mds_rand_select_val = 1.0;
  ggv->mds_rand_select_new = false;
  ggv->mds_perturb_val = 1.0;
  ggv->mds_threshold_high = 0.0;
  ggv->mds_threshold_low = 0.0;

  ggv->metric_nonmetric = metric;
  ggv->KruskalShepard_classic = KruskalShepard;
}

