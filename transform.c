/* transform.c */

/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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

#ifdef G_OS_WIN32
#include <float.h>
extern gint _finite (gdouble);
#endif


#define SIGNUM(x) (((x)<0.0)?(-1.0):(((x)>0.0)?(1.0):(0.0)))

/* */
#ifdef __cplusplus
extern "C" {
#endif
extern gint finite (gdouble);  /*-- defined in math.h, but being ignored --*/
extern gdouble erf (gdouble);  /*-- defined in math.h, but being ignored --*/
#ifdef __cplusplus
}
#endif
/* */

static gchar *domain_error_message = "Data outside the domain of function.";

#ifdef __cplusplus
extern "C" {
#endif
gfloat no_change (gfloat x, gfloat incr) { return x; }
gfloat negate (gfloat x, gfloat incr)    { return -x; }
gfloat raise_min_to_0 (gfloat x, gfloat incr) { return (x + incr); }
gfloat raise_min_to_1 (gfloat x, gfloat incr) { return (x + incr + 1.0); }
gfloat inv_raise_min_to_0 (gfloat x, gfloat incr) { return (x - incr); }
gfloat inv_raise_min_to_1 (gfloat x, gfloat incr) { return (x - incr - 1.0); }
#ifdef __cplusplus
}
#endif

static void
mean_stddev (gdouble *x, gfloat *mean, gfloat *stddev, gint j, 
  datad *d, ggobid *gg)
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
median (gfloat **data, gint jcol, datad *d, ggobid *gg)
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
    m = d->rows_in_plot[i];
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
transform_values_init (gint j, datad *d, ggobid *gg) 
{
  d->vartable[j].tform1 = NO_TFORM1;
  d->vartable[j].tform2 = NO_TFORM2;
  d->vartable[j].domain_incr = 0.;
  d->vartable[j].param = 0.;
  d->vartable[j].domain_adj = no_change;
  d->vartable[j].inv_domain_adj = no_change;
}
void
transform_values_copy (gint jfrom, gint jto, datad *d) 
{
  d->vartable[jto].tform1 = d->vartable[jfrom].tform1;
  d->vartable[jto].tform2 = d->vartable[jfrom].tform2;
  d->vartable[jto].domain_incr = d->vartable[jfrom].domain_incr;
  d->vartable[jto].param = d->vartable[jfrom].param;
  d->vartable[jto].domain_adj = d->vartable[jfrom].domain_adj;
  d->vartable[jto].inv_domain_adj = d->vartable[jfrom].inv_domain_adj;
}

void
transform0_values_set (gint tform0, gint jcol, datad *d, ggobid *gg)
{
  gfloat domain_incr;
  gfloat (*domain_adj) (gfloat x, gfloat incr) = no_change;
  gfloat (*inv_domain_adj) (gfloat x, gfloat incr) = no_change;

  switch (tform0) {

    case NO_TFORM0:  /*-- no domain adjustment --*/
      domain_incr = 0;
      domain_adj = no_change;
      inv_domain_adj = no_change;
      break;

    case RAISE_MIN_TO_0:
      domain_incr = fabs (d->vartable[jcol].lim_raw.min);
      domain_adj = raise_min_to_0;
      inv_domain_adj = inv_raise_min_to_0;
      break;

    case RAISE_MIN_TO_1:
      domain_incr = fabs (d->vartable[jcol].lim_raw.min);
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

  d->vartable[jcol].tform0 = tform0;
  d->vartable[jcol].domain_incr = domain_incr;
  d->vartable[jcol].domain_adj = domain_adj;
  d->vartable[jcol].inv_domain_adj = inv_domain_adj;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform0_opt_menu_set_value (jcol, d, gg);
}

void
transform1_values_set (gint tform1, gfloat expt, gint jcol, 
  datad *d, ggobid *gg)
{
  d->vartable[jcol].tform1 = tform1;
  d->vartable[jcol].param = expt;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform1_opt_menu_set_value (jcol, d, gg);
}

gboolean 
transform1_apply (gint jcol, datad *d, ggobid *gg)
{
  gint i, m;
  gfloat min, max, diff;
  gfloat fmedian, ref;
  gboolean allequal, tform_ok = true;
  gdouble dtmp;
  lims slim, slim_tform;  /*-- specified limits --*/

  gint tform1 = option_menu_index (GTK_OPTION_MENU (gg->tform_ui.stage1_opt));
  gfloat boxcoxparam = gg->tform_ui.boxcox_adj->value;
  gfloat incr = d->vartable[jcol].domain_incr;
  gfloat (*domain_adj) (gfloat x, gfloat incr) = d->vartable[jcol].domain_adj;

  
  /*-- adjust the transformed value of the user-supplied limits --*/
  if (d->vartable[jcol].lim_specified_p) {
    slim.min = d->vartable[jcol].lim_specified.min;
    slim.max = d->vartable[jcol].lim_specified.max;
  }

  switch (tform1)
  {
    case NO_TFORM1:    /*-- Apply the stage0 transformation --*/
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        d->tform.vals[m][jcol] = (*domain_adj)(d->raw.vals[m][jcol], incr);
      }
      /*-- apply the same transformation to the specified limits --*/
      if (d->vartable[jcol].lim_specified_p) {
        slim_tform.min = (*domain_adj)(slim.min, incr);
        slim_tform.max = (*domain_adj)(slim.max, incr);
      }
    break;

    case STANDARDIZE1:    /* (x-mean)/sigma */
    {
      gfloat mean, stddev;
      gdouble *x;
      x = (gdouble *) g_malloc (d->nrows_in_plot * sizeof (gdouble));
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        x[i] = (*domain_adj) (d->raw.vals[m][jcol], incr);
      }

      mean_stddev (x, &mean, &stddev, jcol, d, gg);

      if (stddev == 0) {
        quick_message (domain_error_message, false);
      } else {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot[i];
          d->tform.vals[m][jcol] = ((gfloat) x[i] - mean) / stddev;
        }
      }
      g_free ((gpointer) x);

      /*-- apply the same transformation to the specified limits --*/
      if (d->vartable[jcol].lim_specified_p) {
        slim_tform.min = ((*domain_adj)(slim.min, incr) - mean) / stddev;
        slim_tform.max = ((*domain_adj)(slim.max, incr) - mean) / stddev;
      }

    }
    break;

    case BOXCOX:  /* Box-Cox power transform family */

      if (fabs (boxcoxparam-0) < .001) {       /* Natural log */
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot[i];
          if ((*domain_adj)(d->raw.vals[m][jcol], incr) <= 0) {
            g_printerr ("%f %f\n",
              d->raw.vals[m][jcol],
              (*domain_adj)(d->raw.vals[m][jcol], incr));
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
        }
        if (tform_ok) {  /*-- if all values are in the domain of log --*/
          for (i=0; i<d->nrows_in_plot; i++) {
            m = d->rows_in_plot[i];
            d->tform.vals[m][jcol] = (gfloat)
              log ((gdouble) ((*domain_adj)(d->raw.vals[m][jcol], incr)));
          }

          /*-- apply the same transformation to the specified limits --*/
          if (d->vartable[jcol].lim_specified_p) {
            slim_tform.min = (gfloat)
              log ((gdouble) ((*domain_adj)(slim.min, incr)));
            slim_tform.max = (gfloat)
              log ((gdouble) ((*domain_adj)(slim.max, incr)));
          }
        }
      }

      else {  /*-- if the exponent is outisde (-.001, .001) --*/

        for (i=0; i<d->nrows_in_plot; i++) {

          m = d->rows_in_plot[i];
          dtmp = pow ((gdouble) (*domain_adj)(d->raw.vals[m][jcol], incr),
                      boxcoxparam);
          dtmp = (dtmp - 1.0) / boxcoxparam;

          /* If dtmp no good, return */
#ifdef _WIN32
          if (!_finite (dtmp)) {
#else
          if (!finite (dtmp)) {
#endif
            g_printerr ("%f %f %f (breaking, i=%d)\n",
              d->raw.vals[m][jcol],
              (*domain_adj)(d->raw.vals[m][jcol], incr),
              dtmp, i);
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          } else {
            d->tform.vals[m][jcol] = (gfloat) dtmp;
          }
        }

        /*-- apply the same transformation to the specified limits --*/
        if (d->vartable[jcol].lim_specified_p) {
          dtmp = pow ((gdouble) (*domain_adj)(slim.min, incr), boxcoxparam);
          slim_tform.min = (gfloat) (dtmp - 1.0) / boxcoxparam;
          dtmp = pow ((gdouble) (*domain_adj)(slim.max, incr), boxcoxparam);
          slim_tform.max = (gfloat) (dtmp - 1.0) / boxcoxparam;
        }
      }

    break;

    case ABSVALUE:
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        d->tform.vals[m][jcol] = (d->raw.vals[m][jcol] + incr < 0) ?
          fabs ((gdouble)(*domain_adj)(d->raw.vals[m][jcol], incr)) :
          (*domain_adj)(d->raw.vals[m][jcol], incr);
      }

      /*-- apply the same transformation to the specified limits --*/
      if (d->vartable[jcol].lim_specified_p) {
        slim_tform.min = (slim.min + incr < 0) ?
          fabs ((gdouble)(*domain_adj)(slim.min, incr)) :
          (*domain_adj)(slim.min, incr);
        slim_tform.max = (slim.max + incr < 0) ?
          fabs ((gdouble)(*domain_adj)(slim.max, incr)) :
          (*domain_adj)(slim.max, incr);
      }
    break;

    case INVERSE:    /* 1/x */
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        if ((*domain_adj)(d->raw.vals[m][jcol], incr) == 0) {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }

      if (tform_ok) {
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot[i];
          d->tform.vals[m][jcol] = (gfloat)
            pow ((gdouble) (*domain_adj)(d->raw.vals[m][jcol], incr),
              (gdouble) (-1.0));
        }

        /*-- apply the same transformation to the specified limits --*/
        if (d->vartable[jcol].lim_specified_p) {
          slim_tform.min = (gfloat)
            pow ((gdouble) (*domain_adj)(slim.min, incr), (gdouble) (-1.0));
          slim_tform.max = (gfloat)
            pow ((gdouble) (*domain_adj)(slim.max, incr), (gdouble) (-1.0));
        }
      }
    break;

    case LOG10:    /* Base 10 log */
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        if ((*domain_adj)(d->raw.vals[m][jcol], incr) <= 0) {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }
      if (tform_ok) {  /*-- if all values are in the domain of log10 --*/
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot[i];
          d->tform.vals[m][jcol] = (gfloat)
            log10 ((gdouble) (*domain_adj)(d->raw.vals[m][jcol], incr));
        }

        /*-- apply the same transformation to the specified limits --*/
        if (d->vartable[jcol].lim_specified_p) {
          slim_tform.min = (gfloat)
            log10 ((gdouble) (*domain_adj)(slim.min, incr));
          slim_tform.max = (gfloat)
            log10 ((gdouble) (*domain_adj)(slim.max, incr));
        }
      }
    break;

    case SCALE01:    /* Map onto [0,1] */
      /* First find min and max; they get updated after transformations */

      min = max = (*domain_adj)(d->raw.vals[0][jcol], incr);
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        ref = (*domain_adj)(d->raw.vals[m][jcol], incr);
        if (ref < min) min = ref;
        if (ref > max) max = ref;
      }

      limits_adjust (&min, &max);
      diff = max - min;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        d->tform.vals[m][jcol] =
          ((*domain_adj)(d->raw.vals[m][jcol], incr) - min)/diff;
      }

      /*-- apply the same transformation to the specified limits --*/
      if (d->vartable[jcol].lim_specified_p) {
        slim_tform.min = ((*domain_adj)(slim.min, incr) - min) / diff;
        slim_tform.max = ((*domain_adj)(slim.max, incr) - min) / diff;
      }
    break;


    case DISCRETE2:    /* x>median */
      /* refuse to discretize if all values are the same */
      allequal = true;
      ref = d->raw.vals[0][jcol];
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        if (d->raw.vals[m][jcol] != ref) {
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

      fmedian = median (d->raw.vals, jcol, d, gg);
      fmedian = (*domain_adj)(fmedian, incr);

      /* Then find the true min and max */
      min = max = (*domain_adj)(d->raw.vals[0][jcol], incr);
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        ref = (*domain_adj)(d->raw.vals[m][jcol], incr);
        if (ref < min) min = ref;
        if (ref > max) max = ref;
      }

      /* This prevents the collapse of the data in a special case */
      if (max == fmedian)
        fmedian = (min + max)/2.0;

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        d->tform.vals[m][jcol] =
          ((*domain_adj)(d->raw.vals[m][jcol], incr) > fmedian) ? 1.0 : 0.0;
      }

      /*-- apply the same transformation to the specified limits --*/
      if (d->vartable[jcol].lim_specified_p) {
        slim_tform.min = 0.0;
        slim_tform.max = 1.0;
      }
    break;

    case RANK:
    case NORMSCORE:  /*-- normscore = qnorm applied to rank --*/
    {
      paird *pairs = (paird *)
        g_malloc (d->nrows_in_plot * sizeof (paird));
    
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        pairs[m].f = d->raw.vals[m][jcol];
        pairs[m].indx = m;
      }
      qsort ((gchar *) pairs, d->nrows_in_plot, sizeof (paird), pcompare);
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        d->tform.vals[pairs[m].indx][jcol] =
          (tform1 == RANK) ?
            (gfloat) m :
            qnorm ((gfloat) (m+1) / (gfloat) (d->nrows_in_plot+1));
      }

      /*-- apply the same transformation to the specified limits --*/
      /*-- how? --*/

      g_free ((gpointer) pairs);
    }
      break;

    case ZSCORE:
    {
      gdouble *zscore_data;
      gdouble zmean=0, zvar=0;

      /* Allocate array for z scores */
      zscore_data = (gdouble *) g_malloc (d->nrows_in_plot * sizeof (gdouble));

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        dtmp = (gdouble) (*domain_adj)(d->raw.vals[m][jcol], incr);
        zscore_data[m] = dtmp;
        zmean += dtmp;
        zvar += (dtmp * dtmp);
      }
      zmean /= (gdouble) d->nrows_in_plot;
      zvar = sqrt (zvar / (gdouble) d->nrows_in_plot - zmean*zmean);
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        zscore_data[m] = (zscore_data[m] - zmean) / zvar;
      }

      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        if (zscore_data[m] > 0)
          zscore_data[m] =
            erf (zscore_data[m]/sqrt (2.)) / 2.8284271+0.5;
        else if (zscore_data[m]<0)
          zscore_data[m] = 0.5 - erf (fabs
            (zscore_data[m])/sqrt (2.))/2.8284271;
        else 
          zscore_data[m] = 0.5;
      }
        
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        d->tform.vals[m][jcol] = (gfloat) zscore_data[m]; 
      }

      /*-- apply the same transformation to the specified limits --*/
      /*-- how? --*/

      g_free ((gpointer) zscore_data);
    }
    break;
  }

  if (tform_ok && d->vartable[jcol].lim_specified_p) {
    d->vartable[jcol].lim_specified_tform.min = slim_tform.min;
    d->vartable[jcol].lim_specified_tform.max = slim_tform.max;
  }

  return (tform_ok);
}

void
transform2_values_set (gint tform2, gint jcol, datad *d, ggobid *gg)
{
  d->vartable[jcol].tform2 = tform2;

  /*-- set explicitly in case the routine is not called from the ui --*/
  transform2_opt_menu_set_value (jcol, d, gg);
}

gboolean 
transform2_apply (gint jcol, datad *d, ggobid *gg)
{
  gint i, m;
  gboolean tform_ok = true;
  gint tform2 = option_menu_index (GTK_OPTION_MENU (gg->tform_ui.stage2_opt));

  lims slim, slim_tform;  /*-- specified limits --*/
  if (d->vartable[jcol].lim_specified_p) {
    slim.min = d->vartable[jcol].lim_specified_tform.min;
    slim.max = d->vartable[jcol].lim_specified_tform.max;
  }

  switch (tform2)
  {
    case NO_TFORM2:  /* Restore the values from transformation, stage 2 */
      break;

    case STANDARDIZE2:    /* (x-mean)/sigma */
    {
      gfloat mean, stddev;
      gdouble *x;
      x = (gdouble *) g_malloc (d->nrows_in_plot * sizeof (gdouble));
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        x[i] = (gdouble) d->tform.vals[m][jcol];
      }

      mean_stddev (x, &mean, &stddev, jcol, d, gg);
      if (stddev == 0) {
        quick_message (domain_error_message, false);
      } else {

        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot[i];
          d->tform.vals[m][jcol] = (x[i] - mean)/stddev;
        }

        /*-- apply the same transformation to the specified limits --*/
        if (d->vartable[jcol].lim_specified_p) {
          slim_tform.min = (slim.min - mean) / stddev;
          slim_tform.max = (slim.max - mean) / stddev;
        }

      }
    }
    break;

    default:
      fprintf (stderr, "Unhandled switch-case in transform2_apply\n");
  }

  if (tform_ok && d->vartable[jcol].lim_specified_p) {
    d->vartable[jcol].lim_specified_tform.min = slim_tform.min;
    d->vartable[jcol].lim_specified_tform.max = slim_tform.max;
  }

  return tform_ok;
}

/*
 * update the labels <after> transform has completed, so that
 * we're ready for any sort of success or failure
*/
void
collab_tform_update (gint j, datad *d, ggobid *gg)
{
  gchar *lbl0, *lbl1;

  g_free ((gpointer) d->vartable[j].collab_tform);

  /*-- skip the stage0 changes except negation --*/
  switch (d->vartable[j].tform0) {
    case NEGATE:
      lbl0 = g_strdup_printf ("-%s", d->vartable[j].collab);
      break;
    default:
      lbl0 = g_strdup (d->vartable[j].collab);
      break;
  }

  switch (d->vartable[j].tform1) {
    case NO_TFORM1:
      lbl1 = g_strdup (lbl0);
      break;
    case STANDARDIZE1:
      lbl1 = g_strdup_printf ("(%s-m)/s", lbl0);
      break;
    case BOXCOX:
      lbl1 = g_strdup_printf ("B-C(%s,%.2f)", lbl0, d->vartable[j].param);
      break;
    case ABSVALUE:
      lbl1 = g_strdup_printf ("Abs(%s)", lbl0);
      break;
    case INVERSE:
      lbl1 = g_strdup_printf ("1/%s", lbl0);
      break;
    case LOG10:
      lbl1 = g_strdup_printf ("log10(%s)", lbl0);
      break;
    case SCALE01:
      lbl1 = g_strdup_printf ("%s [0,1]", lbl0);
      break;
    case DISCRETE2:
      lbl1 = g_strdup_printf ("%s:0,1", lbl0);
      break;
    case RANK:
      lbl1 = g_strdup_printf ("rank(%s)", lbl0);
      break;
    case NORMSCORE:
      lbl1 = g_strdup_printf ("normsc(%s)", lbl0);
      break;
    case ZSCORE:
      lbl1 = g_strdup_printf ("zsc(%s)", lbl0);
      break;
  }

  switch (d->vartable[j].tform2) {
    case NO_TFORM2:
      d->vartable[j].collab_tform = g_strdup (lbl1);
      break;
    case STANDARDIZE2:
      d->vartable[j].collab_tform = g_strdup_printf ("(%s-m)/s", lbl1);
      break;
  }

  g_free ((gpointer) lbl0);
  g_free ((gpointer) lbl1);
}

void tform_label_update (gint jcol, datad *d, ggobid *gg)
{
  /*-- update the values of the variable labels --*/
  collab_tform_update (jcol, d, gg);

  /*-- update the displayed checkbox label --*/
  varlabel_set (jcol, d, gg);

  /*-- update the displayed variable circle labels --*/
  /*-- need a routine here --*/

  /*-- update the variable statistics table --*/
  vartable_collab_tform_set_by_var (jcol, d);
}

/*---------------------------------------------------------------------*/

/*
 * stage = 0,1,2
 * tform_type depends on the stage
 * param is the box-cox exponent, only used in stage 1
*/
void
transform_variable (gint stage, gint tform_type, gfloat param, gint jcol,
  datad *d, ggobid *gg)
{
  switch (stage) {
    case 0:

      transform0_values_set (tform_type, jcol, d, gg);

      /*-- apply tform1 to the new domain, using pre-existing parameters --*/
      /*-- if it fails, reset tform1 to NULL --*/
      if (!transform1_apply (jcol, d, gg)) {
        transform1_values_set (NO_TFORM1, 0.0, jcol, d, gg);
        transform1_apply (jcol, d, gg);
      }

      /*-- try to apply tform2 to the new values of tform1 --*/
      /*-- if it fails, reset tform2 to NULL --*/
      if (!transform2_apply (jcol, d, gg)) {
        transform2_values_set (NO_TFORM2, jcol, d, gg);
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
      }

      /*-- then run the stage2 transform --*/
      if (stage == 2)
        transform2_values_set (tform_type, jcol, d, gg);
      if (!transform2_apply (jcol, d, gg)) {
        transform2_values_set (NO_TFORM2, jcol, d, gg);
      }
    break;
  }

  tform_label_update (jcol, d, gg);
}

void
transform (gint stage, gint tform_type, gfloat param, datad *d, ggobid *gg) 
{
  gint k;
  gint ncols;
  gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));

  ncols = selected_cols_get (cols, d, gg);
  if (ncols == 0)
    ncols = plotted_cols_get (cols, d, gg);

  for (k=0; k<ncols; k++)
    transform_variable (stage, tform_type, param, cols[k], d, gg);

  g_free ((gpointer) cols);


  limits_set (false, true, d);  
  for (k=0; k<ncols; k++) {
    vartable_limits_set_by_var (k, d);
    vartable_stats_set_by_var (k, d);
    tform_to_world_by_var (k, d, gg);
  }

  /*
   * there's no need to reproject if the variables just transformed
   * are not currently displayed, but we're doing it anyway.
  */
  /*-- do not redisplay the missing values displays --*/
  displays_tailpipe (REDISPLAY_PRESENT, gg);
}
