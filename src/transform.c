/* transform.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

/*
 * I'm trying to apply each transformation to the user-specified
 * limits along with the data, and for most transformations, that
 * isn't too hard to do.  However, rank, normal score, and z-score
 * seem to raise a problem.  Perhaps the user-specified limits
 * should be turned off in those cases.  --dfs
*/

#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define SIGNUM(x) (((x)<0.0)?(-1.0):(((x)>0.0)?(1.0):(0.0)))

static const gchar * const domain_error_message = "Data outside the domain of function.";
static const gchar * const ldomain_error_message = "Limits outside the domain of function.";

#ifdef __cplusplus
extern "C" {
#endif
gfloat no_change (gfloat x, gfloat incr) { return x; }
gfloat negate (gfloat x, gfloat incr)    { return -x; }
gfloat raise_min_to_0 (gfloat x, gfloat incr) { return (x - incr); }
gfloat raise_min_to_1 (gfloat x, gfloat incr) { return (x - incr + 1.0); }
gfloat inv_raise_min_to_0 (gfloat x, gfloat incr) { return (x + incr); }
gfloat inv_raise_min_to_1 (gfloat x, gfloat incr) { return (x + incr - 1.0); }
#ifdef __cplusplus
}
#endif

static void
mean_stddev (gdouble *x, gfloat *mean, gfloat *stddev, gint j, 
  GGobiStage *d, ggobid *gg)
/*
 * Find the minimum and maximum values of a column 
 * scaling by mean and std_width standard deviations.
 * Use the function pointer to domain_adj.
*/
{
  gint i;
  gdouble sumxi = 0.0, sumxisq = 0.0;
  gdouble dmean, dvar, dstddev;
  gdouble dn = (gdouble) d->n_rows;

  for (i=0; i<d->n_rows; i++) {
    sumxi = sumxi + x[i];
    sumxisq = sumxisq + (x[i] * x[i]);
  }

  dmean = sumxi / dn;
  dvar = (sumxisq / dn) - (dmean * dmean);
  dstddev = sqrt (dvar);

  *mean = (gfloat) dmean;
  *stddev = (gfloat) dstddev;
}

gfloat
median (gfloat **data, gint jcol, GGobiStage *d, ggobid *gg)
{
/*
 * Find the minimum and maximum values of each column,
 * scaling by median and largest distance
*/
  gint i, np = d->n_rows;
  gfloat *x;
  gdouble dmedian = 0;


  x = (gfloat *) g_malloc (d->n_rows * sizeof (gfloat));
  for (i=0; i<np; i++) {
    x[i] = data[i][jcol];
  }

  qsort ((void *) x, np, sizeof (gfloat), fcompare);
  dmedian = ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;

  g_free ((gpointer) x);
  return (gfloat) dmedian;
}

/* adapted from Ratfor code used in S */
gdouble
qnorm (gdouble pr)
{
  gdouble p, eta, term,
    f1 = .010328,
    f2 = .802853,
    f3 = 2.515517,
    f4 = .001308,
    f5 = .189269,
    f6 = 1.432788;

  if (pr <= 0. || pr >= 1.) 
    g_printerr ("Probability out of range (0,1): %f", pr);
  p = pr;
  if (p > 0.5) p = 1.0 - pr;

  /*  depending on the size of pr this may error in log or sqrt */
  eta  = sqrt (-2.0 * log (p));
  term = ((f1*eta+f2) * eta + f3) / (((f4*eta+f5)*eta+f6) * eta + 1.0);
  if (pr <= .5)
    return (term - eta);
  else return (eta - term);
}


gboolean
transform_values_compare (gint jfrom, gint jto, GGobiStage *d)
{
  gboolean same = true;
  GGobiVariable *varf = ggobi_stage_get_variable(d, jfrom);
  GGobiVariable *vart = ggobi_stage_get_variable(d, jto);

  same = (
    vart->tform1 == varf->tform1 &&
    vart->tform2 == varf->tform2 &&
    vart->domain_incr == varf->domain_incr &&
    vart->param == varf->param &&
    vart->domain_adj == varf->domain_adj &&
    vart->inv_domain_adj == varf->inv_domain_adj);

  return same;
}

void
transform0_values_set (gint tform0, gint j, GGobiStage *d, ggobid *gg)
{
  gfloat domain_incr;
  gfloat (*domain_adj) (gfloat x, gfloat incr) = no_change;
  gfloat (*inv_domain_adj) (gfloat x, gfloat incr) = no_change;
  GGobiVariable *var = ggobi_stage_get_variable(d, j);

  switch (tform0) {

    case NO_TFORM0:  /*-- no domain adjustment --*/
      domain_incr = 0;
      domain_adj = no_change;
      inv_domain_adj = no_change;
    break;

    case RAISE_MIN_TO_0:
      domain_incr = var->lim_raw.min;
      domain_adj = raise_min_to_0;
      inv_domain_adj = inv_raise_min_to_0;
    break;

    case RAISE_MIN_TO_1:
      domain_incr = var->lim_raw.min;
      domain_adj = raise_min_to_1;
      inv_domain_adj = inv_raise_min_to_1;
    break;

    case NEGATE:
      domain_incr = 0.0;
      domain_adj = negate;
      inv_domain_adj = negate;
    break;

    default:
      domain_incr = 0;
      domain_adj = no_change;
      inv_domain_adj = no_change;
  }

  var->tform0 = tform0;
  var->domain_incr = domain_incr;
  var->domain_adj = domain_adj;
  var->inv_domain_adj = inv_domain_adj;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform0_combo_box_set_value (j, false/*transform*/, d, gg);
}

void
transform1_values_set (gint tform1, gfloat expt, gint j, 
  GGobiStage *d, ggobid *gg)
{
  GGobiVariable *var = ggobi_stage_get_variable(d, j);

  var->tform1 = tform1;
  var->param = expt;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform1_combo_box_set_value (j, false, d, gg);
}

gboolean 
transform1_apply (gint j, GGobiStage *s, ggobid *gg)
{
  gint i;
  gfloat min, max, diff;
  gfloat ref, ftmp;
  gboolean tform_ok = true;
  gdouble dtmp;
  lims slim, slim_tform;  /*-- specified limits --*/
  slim_tform.min = 0;
  slim_tform.max = 0;
  slim.min = 0;
  slim.max = 0;
  GtkWidget *stage1_cbox;
  gint tform1;
  gfloat boxcoxparam = gg->tform_ui.boxcox_adj->value;
  GGobiVariable *var = ggobi_stage_get_variable(s, j);
  gfloat incr = var->domain_incr;
  gfloat (*domain_adj) (gfloat x, gfloat incr) = var->domain_adj;
  GGobiData *d = GGOBI_DATA(ggobi_stage_get_root(s));

  stage1_cbox = widget_find_by_name (gg->tform_ui.window,
    "TFORM:stage1_options");

  if (!stage1_cbox)
    return false;

  tform1 = gtk_combo_box_get_active (GTK_COMBO_BOX (stage1_cbox));

  /*-- adjust the transformed value of the user-supplied limits --*/
  if (var->lim_specified_p) {
    slim.min = var->lim_specified.min;
    slim.max = var->lim_specified.max;
  }

  switch (tform1)
  {
    case NO_TFORM1:    /*-- Apply the stage0 transformation --*/
      for (i=0; i<s->n_rows; i++) {
        s->tform.vals[i][j] = (*domain_adj)(d->raw.vals[i][j], incr);
      }
      /*-- apply the same transformation to the specified limits --*/
      if (var->lim_specified_p) {
        slim_tform.min = (*domain_adj)(slim.min, incr);
        slim_tform.max = (*domain_adj)(slim.max, incr);
      }
    break;

    case BOXCOX:  /* Box-Cox power transform family */
      if (fabs (boxcoxparam-0) < .001) {       /* Natural log */
        for (i=0; i<s->n_rows; i++) {
          if ((*domain_adj)(d->raw.vals[i][j], incr) <= 0) {
            g_printerr ("%f %f\n",
              d->raw.vals[i][j],
              (*domain_adj)(d->raw.vals[i][j], incr));
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
        }
        /*-- apply the same domain test to the specified limits --*/
        if (tform_ok && var->lim_specified_p) {
          if (((*domain_adj)(slim_tform.min, incr) <= 0) ||
              ((*domain_adj)(slim_tform.max, incr) <= 0))
          {
            quick_message (ldomain_error_message, false);
            tform_ok = false;
          }
        }

        if (tform_ok) {  /*-- if all values are in the domain of log --*/
          for (i=0; i<s->n_rows; i++) {
            s->tform.vals[i][j] = (gfloat)
              log ((gdouble) ((*domain_adj)(d->raw.vals[i][j], incr)));
          }

          /*-- apply the same transformation to the specified limits --*/
          if (var->lim_specified_p) {
            slim_tform.min = (gfloat)
              log ((gdouble) ((*domain_adj)(slim.min, incr)));
            slim_tform.max = (gfloat)
              log ((gdouble) ((*domain_adj)(slim.max, incr)));
          }
        }
      }

      else {  /*-- if the exponent is outisde (-.001, .001) --*/

        for (i=0; i<s->n_rows; i++) {

          dtmp = pow ((gdouble) (*domain_adj)(d->raw.vals[i][j], incr),
                      boxcoxparam);
          dtmp = (dtmp - 1.0) / boxcoxparam;

          /* If dtmp no good, return */
          if (!isfinite (dtmp)) {
            g_printerr ("%f %f %f (breaking, i=%d)\n",
              d->raw.vals[i][j],
              (*domain_adj)(d->raw.vals[i][j], incr),
              dtmp, i);
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          } else {
            s->tform.vals[i][j] = (gfloat) dtmp;
          }
        }

        /*-- apply the same transformation to the specified limits --*/
        if (tform_ok && var->lim_specified_p) {
          dtmp = pow ((gdouble) (*domain_adj)(slim.min, incr), boxcoxparam);
          if (isfinite (dtmp)) {
            quick_message (ldomain_error_message, false);
            tform_ok = false;
          }
          slim_tform.min = (gfloat) (dtmp - 1.0) / boxcoxparam;
          dtmp = pow ((gdouble) (*domain_adj)(slim.max, incr), boxcoxparam);
          if (isfinite (dtmp)) {
            quick_message (ldomain_error_message, false);
            tform_ok = false;
          }
          slim_tform.max = (gfloat) (dtmp - 1.0) / boxcoxparam;
        }
      }
    break;

    case LOG10:    /* Base 10 log */
      for (i=0; i<s->n_rows; i++) {
        if ((*domain_adj)(d->raw.vals[i][j], incr) <= 0) {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }
      /*-- apply the same domain test to the specified limits --*/
      if (tform_ok && var->lim_specified_p) {
        if (((*domain_adj)(slim_tform.min, incr) <= 0) ||
            ((*domain_adj)(slim_tform.max, incr) <= 0))
        {
          quick_message (ldomain_error_message, false);
          tform_ok = false;
        }
      }

      if (tform_ok) {  /*-- if all values are in the domain of log10 --*/
        for (i=0; i<s->n_rows; i++) {
          s->tform.vals[i][j] = (gfloat)
            log10 ((gdouble) (*domain_adj)(d->raw.vals[i][j], incr));
        }
        /*-- apply the same transformation to the specified limits --*/
        if (var->lim_specified_p) {
          slim_tform.min = (gfloat)
            log10 ((gdouble) (*domain_adj)(slim.min, incr));
          slim_tform.max = (gfloat)
            log10 ((gdouble) (*domain_adj)(slim.max, incr));
        }
      }
    break;

    case INVERSE:    /* 1/x: require all data to be of the same sign */
      for (i=0; i<s->n_rows-1; i++) {
        if (SIGNUM((*domain_adj)(d->raw.vals[i][j], incr)) !=
            SIGNUM((*domain_adj)(d->raw.vals[i+1][j], incr)))
        {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }
      /*-- apply the same domain test to the specified limits --*/
      if (tform_ok && var->lim_specified_p) {
        if (SIGNUM((*domain_adj)(slim_tform.min, incr)) !=
            SIGNUM((*domain_adj)(slim_tform.max, incr)))
        {
          quick_message (ldomain_error_message, false);
          tform_ok = false;
        }
      }

      if (tform_ok) {
        for (i=0; i<s->n_rows; i++) {
          s->tform.vals[i][j] = (gfloat)
            pow ((gdouble) (*domain_adj)(d->raw.vals[i][j], incr),
              (gdouble) (-1.0));
        }

        /*-- apply the same transformation to the specified limits --*/
        if (var->lim_specified_p) {
          slim_tform.min = (gfloat)
            pow ((gdouble) (*domain_adj)(slim.min, incr), (gdouble) (-1.0));
          slim_tform.max = (gfloat)
            pow ((gdouble) (*domain_adj)(slim.max, incr), (gdouble) (-1.0));
        }
      }
    break;

    case ABSVALUE:
      for (i=0; i<s->n_rows; i++) {
        ftmp = (*domain_adj)(d->raw.vals[i][j], incr);
        s->tform.vals[i][j] = (ftmp >= 0 ? ftmp : -1 * ftmp);
      }
      /*-- apply the same transformation to the specified limits --*/
      if (var->lim_specified_p) {
        ftmp = (*domain_adj)(slim.min, incr);
        slim_tform.min = (ftmp >= 0 ? ftmp : -1 * ftmp);

        ftmp = (*domain_adj)(slim.max, incr);
        slim_tform.max = (ftmp >= 0 ? ftmp : -1 * ftmp);

        if (slim_tform.min > slim_tform.max) {
          ftmp = slim_tform.min;
          slim_tform.min = slim_tform.max;
          slim_tform.max = ftmp;
        }
      }
    break;


    case SCALE_AB:    /* Map onto [a,b] */
    {
      gfloat scale_get_a (ggobid *);
      gfloat scale_get_b (ggobid *);
      gfloat a = scale_get_a (gg);
      gfloat b = scale_get_b (gg);
      gfloat bminusa = b - a;
      gfloat ftmp;

      /*-- Either use user-defined limits, or data min and max --*/
      if (var->lim_specified_p) {
        min = slim_tform.min;
        max = slim_tform.max;
      } else {
        min = max = (*domain_adj)(d->raw.vals[0][j], incr);
        for (i=0; i<s->n_rows; i++) {
          ref = (*domain_adj)(d->raw.vals[i][j], incr);
          if (ref < min) min = ref;
          if (ref > max) max = ref;
        }
      }

      limits_adjust (&min, &max);
      diff = max - min;

      for (i=0; i<s->n_rows; i++) {
        ftmp = ((*domain_adj)(d->raw.vals[i][j], incr) - min)/diff;
        s->tform.vals[i][j] = (ftmp * bminusa) + a;
      }
    }
    break;

    default:
    break;
  }

  if (tform_ok && var->lim_specified_p) {
    var->lim_specified_tform.min = slim_tform.min;
    var->lim_specified_tform.max = slim_tform.max;
  }

  return (tform_ok);
}

void
transform2_values_set (gint tform2, gint j, GGobiStage *d, ggobid *gg)
{
  GGobiVariable *var = ggobi_stage_get_variable(d, j);

  var->tform2 = tform2;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform2_combo_box_set_value (j, true, d, gg);
}

gboolean 
transform2_apply (gint jcol, GGobiStage *d, ggobid *gg)
{
  gint i, m;
  gboolean tform_ok = true;
  GtkWidget *stage2_cbox;
  gint tform2;

  stage2_cbox = widget_find_by_name (gg->tform_ui.window,
                                            "TFORM:stage2_options");
  if (!stage2_cbox)
    return false;

  tform2 = gtk_combo_box_get_active (GTK_COMBO_BOX (stage2_cbox));

  switch (tform2)
  {
    case NO_TFORM2:  /* Restore the values from transformation, stage 2 */
    break;

    case STANDARDIZE:    /* (x-mean)/sigma */
    {
      gfloat mean, stddev;
      gdouble *x;
      x = (gdouble *) g_malloc (d->n_rows * sizeof (gdouble));
      for (i=0; i<d->n_rows; i++) {
        x[i] = (gdouble) d->tform.vals[i][jcol];
      }

      mean_stddev (x, &mean, &stddev, jcol, d, gg);
      if (stddev == 0) {
        quick_message (domain_error_message, false);
      } else {
        for (i=0; i<d->n_rows; i++) {
          d->tform.vals[i][jcol] = (x[i] - mean)/stddev;
        }
      }
    }
    break;

    case SORT:
    case RANK:
    case NORMSCORE:  /*-- normscore = qnorm applied to rank --*/
    {
      paird *pairs = (paird *)
        g_malloc (d->n_rows * sizeof (paird));
    
      for (i=0; i<d->n_rows; i++) {
        pairs[i].f = d->tform.vals[i][jcol];
        pairs[i].indx = i;
      }
      qsort ((gchar *) pairs, d->n_rows, sizeof (paird), pcompare);

      if (tform2 == SORT) {
        for (i=0; i<d->n_rows; i++) {
          m = pairs[i].indx;
          d->tform.vals[m][jcol] = pairs[i].f;
        }
      } else if (tform2 == RANK) {
        for (i=0; i<d->n_rows; i++) {
          m = pairs[i].indx;
          d->tform.vals[m][jcol] = i;
        }
      } else if (tform2 == NORMSCORE) {
        for (i=0; i<d->n_rows; i++) {
          m = pairs[i].indx;
          d->tform.vals[m][jcol] = 
            qnorm ((gfloat) (i+1) / (gfloat) (d->n_rows+1));
        }
      }
      g_free ((gpointer) pairs);
    }
    break;

    case ZSCORE:
    {
      gdouble *zscore_data;
      gdouble zmean=0, zvar=0;
      gdouble dtmp;

      /* Allocate array for z scores */
      zscore_data = (gdouble *) g_malloc (d->n_rows * sizeof (gdouble));

      for (i=0; i<d->n_rows; i++) {
        dtmp = (gdouble) d->tform.vals[i][jcol];
        zscore_data[i] = dtmp;
        zmean += dtmp;
        zvar += (dtmp * dtmp);
      }
      zmean /= (gdouble) d->n_rows;
      zvar = sqrt (zvar / (gdouble) d->n_rows - zmean*zmean);
      for (i=0; i<d->n_rows; i++) {
        zscore_data[i] = (zscore_data[i] - zmean) / zvar;
      }

      for (i=0; i<d->n_rows; i++) {
        if (zscore_data[i] > 0)
          zscore_data[i] =
            erf (zscore_data[i]/sqrt (2.)) / 2.8284271+0.5;
        else if (zscore_data[i]<0)
          zscore_data[i] = 0.5 - erf (fabs
            (zscore_data[i])/sqrt (2.))/2.8284271;
        else 
          zscore_data[i] = 0.5;
      }
        
      for (i=0; i<d->n_rows; i++) {
        d->tform.vals[i][jcol] = (gfloat) zscore_data[i]; 
      }

      g_free ((gpointer) zscore_data);
    }
    break;

    case DISCRETE2:    /* x>median */
    {
      gfloat ref, fmedian, min, max;

      /* refuse to discretize if all values are the same */
      gboolean allequal = true;
      ref = d->tform.vals[0][jcol];
      for (i=0; i<d->n_rows; i++) {
        if (d->tform.vals[i][jcol] != ref) {
          allequal = false;
          break;
        }
      }

      if (allequal) {
        quick_message (domain_error_message, false);
        tform_ok = false;
        break;
      }

      /* First find median */

      fmedian = median (d->tform.vals, jcol, d, gg);

      /* Then find the true min and max */
      min = max = d->tform.vals[0][jcol];
      for (i=0; i<d->n_rows; i++) {
        ref = d->tform.vals[i][jcol];
        if (ref < min) min = ref;
        if (ref > max) max = ref;
      }

      /* This prevents the collapse of the data in a special case */
      if (max == fmedian)
        fmedian = (min + max)/2.0;

      for (i=0; i<d->n_rows; i++) {
        d->tform.vals[i][jcol] =
          (d->tform.vals[i][jcol] > fmedian) ? 1.0 : 0.0;
      }
    }
    break;

    default:
      fprintf (stderr, "Unhandled switch-case in transform2_apply\n");
  }

  return tform_ok;
}

/*---------------------------------------------------------------------*/

/*
 * stage = 0,1,2
 * tform_type depends on the stage
 * param is the box-cox exponent, only used in stage 1
*/
gboolean
transform_variable (gint stage, gint tform_type, gfloat param, gint jcol,
  GGobiStage *d, ggobid *gg)
{
  gboolean success = true;

  switch (stage) {
    case 0:

      transform0_values_set (tform_type, jcol, d, gg);

      /*-- apply tform1 to the new domain, using pre-existing parameters --*/
      /*-- if it fails, reset tform1 to NULL --*/
      if (!transform1_apply (jcol, d, gg)) {
        transform1_values_set (NO_TFORM1, 0.0, jcol, d, gg);
        //transform1_apply (jcol, d, gg);  // not needed?
        success = false;
      }

      /*-- try to apply tform2 to the new values of tform1 --*/
      /*-- if it fails, reset tform2 to NULL --*/
      if (!transform2_apply (jcol, d, gg)) {
        transform2_values_set (NO_TFORM2, jcol, d, gg);
        success = false;
      }
      break;

    case 1:
    case 2:
      /*-- run the stage1 transform --*/
      if (stage == 1)
        transform1_values_set (tform_type, param, jcol, d, gg);
      if (!transform1_apply (jcol, d, gg)) {
        transform1_values_set (NO_TFORM1, 0., jcol, d, gg);
        transform1_apply (jcol, d, gg);
        success = false;
      }

      /*-- then run the stage2 transform --*/
      if (stage == 2)
        transform2_values_set (tform_type, jcol, d, gg);
      if (!transform2_apply (jcol, d, gg)) {
        transform2_values_set (NO_TFORM2, jcol, d, gg);
        success = false;
      }
    break;
  }
  g_signal_emit_by_name(d, "col_name_changed", jcol);
  ggobi_stage_update_col(d, jcol);

  return success;
}

void
transform (gint stage, gint tform_type, gfloat param, gint *vars, gint nvars,
  GGobiStage *d, ggobid *gg) 
{
  guint k;
  gboolean ok = true;
  gint completed = nvars;

  for (k=0; k<nvars; k++) {
    ok = transform_variable (stage, tform_type, param, vars[k], d, gg);
    if (!ok) {
      completed = k;
      break;
    }
  }
}
