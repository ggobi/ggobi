#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"

#include "glayout.h"

void
glayout_init (glayoutd *gl) {

/*
  gl->running_p = false;
  gl->rescale_p = false;

  gl->dist_type = 0;

  gl->mds_group_ind = deflt;

  gl->mds_stepsize = 0.02;
  gl->mds_power = 1.0;
  gl->mds_distpow = 1.0;
  gl->mds_lnorm = 2.0;
  gl->mds_distpow_over_lnorm = 0.5;
  gl->mds_lnorm_over_distpow = 2.0;
  gl->mds_weightpow = 0.0;
  gl->mds_within_between = 1.0;
  gl->mds_rand_select_val = 1.0;
  gl->mds_rand_select_new = false;
  gl->mds_perturb_val = 1.0;
  gl->mds_threshold_high = 0.0;
  gl->mds_threshold_low = 0.0;
  gl->mds_dims = 3;
  gl->mds_freeze_var = 0;
*/

  gl->config_dist = NULL;
  gl->raw_dist = NULL;
  gl->weights = NULL;
  gl->trans_dist = NULL;
  gl->trans_dist_index = NULL;
  gl->bl = NULL;
  gl->bl_w = NULL;
  gl->rand_sel = NULL;

  arrayd_init_null (&gl->dist_orig);
  arrayd_init_null (&gl->dist);
  arrayd_init_null (&gl->pos_orig);
  arrayd_init_null (&gl->pos);

  gl->metric_nonmetric = METRIC;
  gl->KruskalShepard_classic = KRUSKALSHEPARD;

  gl->radial = NULL;
  gl->graphviz = NULL;
}
