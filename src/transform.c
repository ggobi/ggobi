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

#ifdef WIN32
#include <float.h>
extern gint _finite (gdouble);
#endif


#define SIGNUM(x) (((x)<0.0)?(-1.0):(((x)>0.0)?(1.0):(0.0)))

/* */
#ifdef __cplusplus
extern "C" {
#endif
extern gint finite (gdouble);  /*-- not defined on all unixes --*/
extern gdouble erf (gdouble);  /*-- not defined on all unixes --*/
#ifdef __cplusplus
}
#endif
/* */

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
  GGobiData *d, ggobid *gg)
/*
 * Find the minimum and maximum values of a column 
 * scaling by mean and std_width standard deviations.
 * Use the function pointer to domain_adj.
*/
{
  gint i;
  gdouble sumxi = 0.0, sumxisq = 0.0;
  gdouble dmean, dvar, dstddev;
  gdouble dn = (gdouble) d->nrows_in_plot;

  for (i=0; i<d->nrows_in_plot; i++) {
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
median (gfloat **data, gint jcol, GGobiData *d, ggobid *gg)
{
/*
 * Find the minimum and maximum values of each column,
 * scaling by median and largest distance
*/
  gint i, m, np = d->nrows_in_plot;
  gfloat *x;
  gdouble dmedian = 0;


  x = (gfloat *) g_malloc (d->nrows_in_plot * sizeof (gfloat));
  for (i=0; i<np; i++) {
    m = d->rows_in_plot.els[i];
    x[m] = data[m][jcol];
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

void
vt_init(vartabled *vt)
{
  vt->tform0 = NO_TFORM0;
  vt->tform1 = NO_TFORM1;
  vt->tform2 = NO_TFORM2;
  vt->domain_incr = 0.;
  vt->param = 0.;
  vt->domain_adj = no_change;
  vt->inv_domain_adj = no_change;
}
void
transform_values_init (vartabled *vt) 
{
  vt_init(vt);
}

void
vt_copy(vartabled *vtf, vartabled *vtt)
{
  vtt->tform0 = vtf->tform0;
  vtt->tform1 = vtf->tform1;
  vtt->tform2 = vtf->tform2;
  vtt->domain_incr = vtf->domain_incr;
  vtt->param = vtf->param;
  vtt->domain_adj = vtf->domain_adj;
  vtt->inv_domain_adj = vtf->inv_domain_adj;
}

void
transform_values_copy (gint jfrom, gint jto, GGobiData *d)
{
  vartabled *vtf = vartable_element_get (jfrom, d);
  vartabled *vtt = vartable_element_get (jto, d);
  vt_copy (vtf, vtt);
}
gboolean
transform_values_compare (gint jfrom, gint jto, GGobiData *d)
{
  gboolean same = true;
  vartabled *vtf = vartable_element_get (jfrom, d);
  vartabled *vtt = vartable_element_get (jto, d);

  same = (
    vtt->tform1 == vtf->tform1 &&
    vtt->tform2 == vtf->tform2 &&
    vtt->domain_incr == vtf->domain_incr &&
    vtt->param == vtf->param &&
    vtt->domain_adj == vtf->domain_adj &&
    vtt->inv_domain_adj == vtf->inv_domain_adj);

  return same;
}

void
transform0_values_set (gint tform0, gint j, GGobiData *d, ggobid *gg)
{
  gfloat domain_incr;
  gfloat (*domain_adj) (gfloat x, gfloat incr) = no_change;
  gfloat (*inv_domain_adj) (gfloat x, gfloat incr) = no_change;
  vartabled *vt = vartable_element_get (j, d);

  switch (tform0) {

    case NO_TFORM0:  /*-- no domain adjustment --*/
      domain_incr = 0;
      domain_adj = no_change;
      inv_domain_adj = no_change;
    break;

    case RAISE_MIN_TO_0:
      domain_incr = vt->lim_raw.min;
      domain_adj = raise_min_to_0;
      inv_domain_adj = inv_raise_min_to_0;
    break;

    case RAISE_MIN_TO_1:
      domain_incr = vt->lim_raw.min;
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

  vt->tform0 = tform0;
  vt->domain_incr = domain_incr;
  vt->domain_adj = domain_adj;
  vt->inv_domain_adj = inv_domain_adj;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform0_combo_box_set_value (j, false/*transform*/, d, gg);
}

void
transform1_values_set (gint tform1, gfloat expt, gint j, 
  GGobiData *d, ggobid *gg)
{
  vartabled *vt = vartable_element_get (j, d);

  vt->tform1 = tform1;
  vt->param = expt;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform1_combo_box_set_value (j, false, d, gg);
}

gboolean 
transform1_apply (gint j, GGobiData *d, ggobid *gg)
{
  gint i, m, n;
  gfloat min, max, diff;
  gfloat ref, ftmp;
  gboolean tform_ok = true;
  gdouble dtmp;
  lims slim, slim_tform;  /*-- specified limits --*/
  GtkWidget *stage1_cbox;
  gint tform1;
  gfloat boxcoxparam = gg->tform_ui.boxcox_adj->value;
  vartabled *vt = vartable_element_get (j, d);
  gfloat incr = vt->domain_incr;
  gfloat (*domain_adj) (gfloat x, gfloat incr) = vt->domain_adj;

  stage1_cbox = widget_find_by_name (gg->tform_ui.window,
    "TFORM:stage1_options");

  if (!stage1_cbox)
    return false;

  tform1 = gtk_combo_box_get_active (GTK_COMBO_BOX (stage1_cbox));

  /*-- adjust the transformed value of the user-supplied limits --*/
  if (vt->lim_specified_p) {
    slim.min = vt->lim_specified.min;
    slim.max = vt->lim_specified.max;
  }

  switch (tform1)
  {
    case NO_TFORM1:    /*-- Apply the stage0 transformation --*/
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        d->tform.vals[m][j] = (*domain_adj)(d->raw.vals[m][j], incr);
      }
      /*-- apply the same transformation to the specified limits --*/
      if (vt->lim_specified_p) {
        slim_tform.min = (*domain_adj)(slim.min, incr);
        slim_tform.max = (*domain_adj)(slim.max, incr);
      }
    break;

    case BOXCOX:  /* Box-Cox power transform family */
      if (fabs (boxcoxparam-0) < .001) {       /* Natural log */
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          if ((*domain_adj)(d->raw.vals[m][j], incr) <= 0) {
            g_printerr ("%f %f\n",
              d->raw.vals[m][j],
              (*domain_adj)(d->raw.vals[m][j], incr));
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
        }
        /*-- apply the same domain test to the specified limits --*/
        if (tform_ok && vt->lim_specified_p) {
          if (((*domain_adj)(slim_tform.min, incr) <= 0) ||
              ((*domain_adj)(slim_tform.max, incr) <= 0))
          {
            quick_message (ldomain_error_message, false);
            tform_ok = false;
          }
        }

        if (tform_ok) {  /*-- if all values are in the domain of log --*/
          for (i=0; i<d->nrows_in_plot; i++) {
            m = d->rows_in_plot.els[i];
            d->tform.vals[m][j] = (gfloat)
              log ((gdouble) ((*domain_adj)(d->raw.vals[m][j], incr)));
          }

          /*-- apply the same transformation to the specified limits --*/
          if (vt->lim_specified_p) {
            slim_tform.min = (gfloat)
              log ((gdouble) ((*domain_adj)(slim.min, incr)));
            slim_tform.max = (gfloat)
              log ((gdouble) ((*domain_adj)(slim.max, incr)));
          }
        }
      }

      else {  /*-- if the exponent is outisde (-.001, .001) --*/

        for (i=0; i<d->nrows_in_plot; i++) {

          m = d->rows_in_plot.els[i];
          dtmp = pow ((gdouble) (*domain_adj)(d->raw.vals[m][j], incr),
                      boxcoxparam);
          dtmp = (dtmp - 1.0) / boxcoxparam;

          /* If dtmp no good, return */
#ifdef WIN32
          if (!_finite (dtmp)) {
#else
          if (!finite (dtmp)) {
#endif
            g_printerr ("%f %f %f (breaking, i=%d)\n",
              d->raw.vals[m][j],
              (*domain_adj)(d->raw.vals[m][j], incr),
              dtmp, i);
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          } else {
            d->tform.vals[m][j] = (gfloat) dtmp;
          }
        }

        /*-- apply the same transformation to the specified limits --*/
        if (tform_ok && vt->lim_specified_p) {
          dtmp = pow ((gdouble) (*domain_adj)(slim.min, incr), boxcoxparam);
#ifdef WIN32
          if (!_finite (dtmp)) {
#else
          if (!finite (dtmp)) {
#endif
            quick_message (ldomain_error_message, false);
            tform_ok = false;
          }
          slim_tform.min = (gfloat) (dtmp - 1.0) / boxcoxparam;
          dtmp = pow ((gdouble) (*domain_adj)(slim.max, incr), boxcoxparam);
#ifdef WIN32
          if (!_finite (dtmp)) {
#else
          if (!finite (dtmp)) {
#endif
            quick_message (ldomain_error_message, false);
            tform_ok = false;
          }
          slim_tform.max = (gfloat) (dtmp - 1.0) / boxcoxparam;
        }
      }
    break;

    case LOG10:    /* Base 10 log */
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        if ((*domain_adj)(d->raw.vals[m][j], incr) <= 0) {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }
      /*-- apply the same domain test to the specified limits --*/
      if (tform_ok && vt->lim_specified_p) {
        if (((*domain_adj)(slim_tform.min, incr) <= 0) ||
            ((*domain_adj)(slim_tform.max, incr) <= 0))
        {
          quick_message (ldomain_error_message, false);
          tform_ok = false;
        }
      }

      if (tform_ok) {  /*-- if all values are in the domain of log10 --*/
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          d->tform.vals[m][j] = (gfloat)
            log10 ((gdouble) (*domain_adj)(d->raw.vals[m][j], incr));
        }
        /*-- apply the same transformation to the specified limits --*/
        if (vt->lim_specified_p) {
          slim_tform.min = (gfloat)
            log10 ((gdouble) (*domain_adj)(slim.min, incr));
          slim_tform.max = (gfloat)
            log10 ((gdouble) (*domain_adj)(slim.max, incr));
        }
      }
    break;

    case INVERSE:    /* 1/x: require all data to be of the same sign */
      for (i=0; i<d->nrows_in_plot-1; i++) {
        m = d->rows_in_plot.els[i];
        n = d->rows_in_plot.els[i+1];
        if (SIGNUM((*domain_adj)(d->raw.vals[m][j], incr)) !=
            SIGNUM((*domain_adj)(d->raw.vals[n][j], incr)))
        {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }
      /*-- apply the same domain test to the specified limits --*/
      if (tform_ok && vt->lim_specified_p) {
        if (SIGNUM((*domain_adj)(slim_tform.min, incr)) !=
            SIGNUM((*domain_adj)(slim_tform.max, incr)))
        {
          quick_message (ldomain_error_message, false);
          tform_ok = false;
        }
      }

      if (tform_ok) {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          d->tform.vals[m][j] = (gfloat)
            pow ((gdouble) (*domain_adj)(d->raw.vals[m][j], incr),
              (gdouble) (-1.0));
        }

        /*-- apply the same transformation to the specified limits --*/
        if (vt->lim_specified_p) {
          slim_tform.min = (gfloat)
            pow ((gdouble) (*domain_adj)(slim.min, incr), (gdouble) (-1.0));
          slim_tform.max = (gfloat)
            pow ((gdouble) (*domain_adj)(slim.max, incr), (gdouble) (-1.0));
        }
      }
    break;

    case ABSVALUE:
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        ftmp = (*domain_adj)(d->raw.vals[m][j], incr);
        d->tform.vals[m][j] = (ftmp >= 0 ? ftmp : -1 * ftmp);
      }
      /*-- apply the same transformation to the specified limits --*/
      if (vt->lim_specified_p) {
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
      if (vt->lim_specified_p) {
        min = slim_tform.min;
        max = slim_tform.max;
      } else {
        min = max = (*domain_adj)(d->raw.vals[0][j], incr);
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          ref = (*domain_adj)(d->raw.vals[m][j], incr);
          if (ref < min) min = ref;
          if (ref > max) max = ref;
        }
      }

      limits_adjust (&min, &max);
      diff = max - min;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        ftmp = ((*domain_adj)(d->raw.vals[m][j], incr) - min)/diff;
        d->tform.vals[m][j] = (ftmp * bminusa) + a;
      }
    }
    break;

    default:
    break;
  }

  if (tform_ok && vt->lim_specified_p) {
    vt->lim_specified_tform.min = slim_tform.min;
    vt->lim_specified_tform.max = slim_tform.max;
  }

  return (tform_ok);
}

void
transform2_values_set (gint tform2, gint j, GGobiData *d, ggobid *gg)
{
  vartabled *vt = vartable_element_get (j, d);

  vt->tform2 = tform2;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform2_combo_box_set_value (j, true, d, gg);
}

gboolean 
transform2_apply (gint jcol, GGobiData *d, ggobid *gg)
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
      x = (gdouble *) g_malloc (d->nrows_in_plot * sizeof (gdouble));
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        x[i] = (gdouble) d->tform.vals[m][jcol];
      }

      mean_stddev (x, &mean, &stddev, jcol, d, gg);
      if (stddev == 0) {
        quick_message (domain_error_message, false);
      } else {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          d->tform.vals[m][jcol] = (x[i] - mean)/stddev;
        }
      }
    }
    break;

    case SORT:
    case RANK:
    case NORMSCORE:  /*-- normscore = qnorm applied to rank --*/
    {
      paird *pairs = (paird *)
        g_malloc (d->nrows_in_plot * sizeof (paird));
    
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        pairs[i].f = d->tform.vals[m][jcol];
        pairs[i].indx = m;
      }
      qsort ((gchar *) pairs, d->nrows_in_plot, sizeof (paird), pcompare);

      if (tform2 == SORT) {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = pairs[i].indx;
          d->tform.vals[m][jcol] = pairs[i].f;
        }
      } else if (tform2 == RANK) {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = pairs[i].indx;
          d->tform.vals[m][jcol] = i;
        }
      } else if (tform2 == NORMSCORE) {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = pairs[i].indx;
          d->tform.vals[m][jcol] = 
            qnorm ((gfloat) (i+1) / (gfloat) (d->nrows_in_plot+1));
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
      zscore_data = (gdouble *) g_malloc (d->nrows_in_plot * sizeof (gdouble));

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        dtmp = (gdouble) d->tform.vals[m][jcol];
        zscore_data[i] = dtmp;
        zmean += dtmp;
        zvar += (dtmp * dtmp);
      }
      zmean /= (gdouble) d->nrows_in_plot;
      zvar = sqrt (zvar / (gdouble) d->nrows_in_plot - zmean*zmean);
      for (i=0; i<d->nrows_in_plot; i++) {
        zscore_data[i] = (zscore_data[i] - zmean) / zvar;
      }

      for (i=0; i<d->nrows_in_plot; i++) {
        if (zscore_data[i] > 0)
          zscore_data[i] =
            erf (zscore_data[i]/sqrt (2.)) / 2.8284271+0.5;
        else if (zscore_data[i]<0)
          zscore_data[i] = 0.5 - erf (fabs
            (zscore_data[i])/sqrt (2.))/2.8284271;
        else 
          zscore_data[i] = 0.5;
      }
        
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        d->tform.vals[m][jcol] = (gfloat) zscore_data[i]; 
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
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        if (d->tform.vals[m][jcol] != ref) {
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
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        ref = d->tform.vals[m][jcol];
        if (ref < min) min = ref;
        if (ref > max) max = ref;
      }

      /* This prevents the collapse of the data in a special case */
      if (max == fmedian)
        fmedian = (min + max)/2.0;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        d->tform.vals[m][jcol] =
          (d->tform.vals[m][jcol] > fmedian) ? 1.0 : 0.0;
      }
    }
    break;

    default:
      fprintf (stderr, "Unhandled switch-case in transform2_apply\n");
  }

  return tform_ok;
}

/*
 * update the labels <after> transform has completed, so that
 * we're ready for any sort of success or failure
*/
void
collab_tform_update (gint j, GGobiData *d, ggobid *gg)
{
  gchar *lbl0, *lbl1;
  vartabled *vt = vartable_element_get (j, d);

  g_free ((gpointer) vt->collab_tform);

  /*-- skip the stage0 changes except negation --*/
  switch (vt->tform0) {
    case NEGATE:
      lbl0 = g_strdup_printf ("-%s", vt->collab);
      break;
    default:
      lbl0 = g_strdup (vt->collab);
      break;
  }

  switch (vt->tform1) {
    case NO_TFORM1:
      lbl1 = g_strdup (lbl0);
      break;
    case BOXCOX:
      lbl1 = g_strdup_printf ("B-C(%s,%.2f)", lbl0, vt->param);
      break;
    case LOG10:
      lbl1 = g_strdup_printf ("log10(%s)", lbl0);
      break;
    case INVERSE:
      lbl1 = g_strdup_printf ("1/%s", lbl0);
      break;
    case ABSVALUE:
      lbl1 = g_strdup_printf ("abs(%s)", lbl0);
      break;
    case SCALE_AB:
      lbl1 = g_strdup_printf ("%s [a,b]", lbl0);
      break;
  }

  switch (vt->tform2) {
    case NO_TFORM2:
      vt->collab_tform = g_strdup (lbl1);
    break;
    case STANDARDIZE:
      vt->collab_tform = g_strdup_printf ("(%s-m)/s", lbl1);
    break;
    case SORT:
      vt->collab_tform = g_strdup_printf ("sort(%s)", lbl1);
    break;
    case RANK:
      vt->collab_tform = g_strdup_printf ("rank(%s)", lbl1);
    break;
    case NORMSCORE:
      vt->collab_tform = g_strdup_printf ("normsc(%s)", lbl1);
    break;
    case ZSCORE:
      vt->collab_tform = g_strdup_printf ("zsc(%s)", lbl1);
    break;
    case DISCRETE2:
      vt->collab_tform = g_strdup_printf ("%s:0,1", lbl1);
    break;
  }

  g_free ((gpointer) lbl0);
  g_free ((gpointer) lbl1);
}

void tform_label_update (gint j, GGobiData *d, ggobid *gg)
{
  /*-- update the values of the variable labels --*/
  collab_tform_update (j, d, gg);

  /*-- update the displayed checkbox label --*/
  /*varlabel_set (j, d);*/
  varpanel_label_set (j, d);

  /*-- update the displayed variable circle labels --*/
  varcircle_label_set (j, d);

  /*-- update the variable statistics table --*/
  vartable_collab_tform_set_by_var (j, d);
}

/*---------------------------------------------------------------------*/

/*
 * stage = 0,1,2
 * tform_type depends on the stage
 * param is the box-cox exponent, only used in stage 1
*/
gboolean
transform_variable (gint stage, gint tform_type, gfloat param, gint jcol,
  GGobiData *d, ggobid *gg)
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

  tform_label_update (jcol, d, gg);

  return success;
}

void
transform (gint stage, gint tform_type, gfloat param, gint *vars, gint nvars,
  GGobiData *d, ggobid *gg) 
{
  gint k;
  gboolean ok = true;
  gint completed = nvars;

  for (k=0; k<nvars; k++) {
    ok = transform_variable (stage, tform_type, param, vars[k], d, gg);
    if (!ok) {
      completed = k;
      break;
    }
  }
  
  limits_set (false, true, d, gg);  
  for (k=0; k<completed; k++) {
    vartable_limits_set_by_var (vars[k], d);
    vartable_stats_set_by_var (vars[k], d);
    tform_to_world_by_var (vars[k], d, gg);
  }


  /*
   * there's no need to reproject if the variables just transformed
   * are not currently displayed, but we're doing it anyway.
  */
  /*-- do not redisplay the missing values displays --*/
  displays_tailpipe (FULL, gg);
}
