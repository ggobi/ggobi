#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"

#include "ggvis.h"

void
ggvis_init (ggvisd *ggv) {

/*
  ggv->running_p = false;
  ggv->rescale_p = false;

  ggv->dist_type = 0;

  ggv->mds_group_ind = deflt;

  ggv->mds_stepsize = 0.02;
  ggv->mds_power = 1.0;
  ggv->mds_distpow = 1.0;
  ggv->mds_lnorm = 2.0;
  ggv->mds_distpow_over_lnorm = 0.5;
  ggv->mds_lnorm_over_distpow = 2.0;
  ggv->mds_weightpow = 0.0;
  ggv->mds_within_between = 1.0;
  ggv->mds_rand_select_val = 1.0;
  ggv->mds_rand_select_new = false;
  ggv->mds_perturb_val = 1.0;
  ggv->mds_threshold_high = 0.0;
  ggv->mds_threshold_low = 0.0;
  ggv->mds_dims = 3;
  ggv->mds_freeze_var = 0;
*/

  ggv->config_dist = NULL;
  ggv->raw_dist = NULL;
  ggv->weights = NULL;
  ggv->trans_dist = NULL;
  ggv->trans_dist_index = NULL;
  ggv->bl = NULL;
  ggv->bl_w = NULL;
  ggv->rand_sel = NULL;

  arrayd_init_null (&ggv->dist_orig);
  arrayd_init_null (&ggv->dist);
  arrayd_init_null (&ggv->pos_orig);
  arrayd_init_null (&ggv->pos);

  ggv->metric_nonmetric = METRIC;
  ggv->KruskalShepard_classic = KRUSKALSHEPARD;

  ggv->radial = NULL;
  ggv->graphviz = NULL;
}
