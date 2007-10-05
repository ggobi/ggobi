/*
 * Copyright 1986 David W. Scott
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#define true 1
#define false 0
#ifndef iabs
#define iabs(x) ((x) >= 0 ? (x) : -(x))
#endif
#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/* functions */

gint bin1 (gdouble *, gint, gdouble *, gint, gint *);
gint ash1 (gint, gint *, gint, gdouble *, gdouble *, gdouble *,
         gdouble *, gdouble *);

gint
do_ash1d (gdouble * vals, gint nvals, gint nbins, gint n_ashes,
          gdouble * ashed_vals, gdouble * lims_min, gdouble * lims_max,
          gdouble * mean);

/* */

gint
do_ash1d (gdouble * vals, gint nvals, gint nbins, gint n_ashes,
          gdouble * ashed_vals, gdouble * lims_min, gdouble * lims_max,
          gdouble * mean)
{
  gint i, k, icheck;
  gint *bins;
  gdouble min, max, ab[2];
  gdouble sum;

  /* for computing nicerange -- extending the range */
  gdouble del, beta = 0.2;

  /* for ash1 */
  gint ash_return;
  gdouble *f, *t, *w;            /* w = weights */
  gdouble kopt[] = { 2.0, 2.0 }; /* S function default values for kernel options */
  gdouble binwidth;

  /* for generating the interpolated values to plot */
  gdouble ti;

  bins = (gint *) g_malloc (nbins * sizeof (gint));

  min = max = vals[0];
  for (i = 1; i < nvals; i++) {
    min = MIN (min, vals[i]);
    max = MAX (max, vals[i]);
  }

  del = ((max - min) * beta) / 2.0;
  ab[0] = min - del;
  ab[1] = max + del;

  icheck = bin1 (vals, nvals, ab, nbins, bins);

  w = (gdouble *) g_malloc (n_ashes * sizeof (gdouble));  /* weights */
  t = (gdouble *) g_malloc (nbins * sizeof (gdouble));
  f = (gdouble *) g_malloc (nbins * sizeof (gdouble));
  ash_return = ash1 (n_ashes, bins, nbins, ab, kopt, t, f, w);

  binwidth = (ab[1] - ab[0]) / (gdouble) nbins;

  *lims_min = INT_MAX;
  *lims_max = -1 * INT_MAX;
  sum = 0;
  for (i = 0; i < nvals; i++) {
    ti = (vals[i] - ab[0]) / binwidth - .5;
    k = (gint) ti;
    ashed_vals[i] =
      f[k + 1] * (ti - (gdouble) k) + f[k] * ((gdouble) k + 1 - ti);

    /* without interpolation */
    /* ashed_vals[i] = f[(int) ffloor((vals[i] - ab[0]) / binwidth)]; */

    *lims_min = MIN (ashed_vals[i], *lims_min);
    *lims_max = MAX (ashed_vals[i], *lims_max);
    sum += ashed_vals[i];
  }
  *mean = sum / (gdouble) nvals;

                                                  /* Scale onto [0,100] *//*-- ggobi will handle the scaling --*/
/*
  for (i=0; i<nvals; i++) {
    ashed_vals[i] = (ashed_vals[i] - min) * 100. / max;
  }
*/

  g_free ((gpointer) bins);
  g_free ((gpointer) w);
  g_free ((gpointer) t);
  g_free ((gpointer) f);

  return ash_return;
}


/*----------------------------------------------------------------------*/

/*
c       April 8, 1986
c       Find bin counts of data array "x(n)" for ASH estimator
c       "nbin" bins are formed over the interval [a,b)
c       bin counts returned in array "nc"  -  # pts outside [a,b) = "nskip"
c
c  ##### Copyright 1986 David W. Scott
*/

gint
bin1 (gdouble * x, gint n, gdouble * ab, gint nbin, gint * nc)
{

/*
  x[n]
  ab[2]
  nc[nbin]
*/
  gint i, k, nskip;
  gdouble a, b, d;

  nskip = 0;
  a = ab[0];
  b = ab[1];

  for (i = 0; i < nbin; i++)
    nc[i] = 0;

  d = (b - a) / (gdouble) nbin;

  for (i = 0; i < n; i++) {
    k = (gint) ((x[i] - a) / d) + 1;
    if (k >= 1 && k <= nbin)
      nc[k] += 1;
    else
      nskip += 1;
  }

  return nskip;
}


/*
c   April 8, 1986
c
c   Computer ASH density estimate;  Quartic (biweight) kernel
c   Average of "m" shifted histograms
c
c   Bin counts in array "nc(nbin)"  -  from routine "bin1"
c   "nbin" bins are formed over the interval [a,b)
c
c   ASH estimates returned in array "f(nbin)"
c
c   FP-ASH plotted at  a+d/2 ... b-d/2   where d = (b-a)/nbin
c
c   Note:  If "nskip" was nonzero, ASH estimates incorrect near boundary
c   Note:  Should leave "m" empty bins on each end of array "nc" so f OK

c ##### Copyright 1986 David W. Scott
*/


gint
ash1 (gint m, gint * nc, gint nbin, gdouble * ab, gdouble * kopt, gdouble * t,
      gdouble * f, gdouble * w)
{

/*
  nc(nbin), ab(2), t(nbin), f(nbin), w(m), kopt(2)
*/
  gdouble a, b, delta, cons, c, h;
  gint i, k, n;

  gint ier = 0;
  a = ab[0];
  b = ab[1];
  n = 0;

/*
 * compute weights    cons * ( 1-abs((i/m))^kopt1)^kopt2
 *             --  should sum to "m"   5-8-91
 *                       w-array shifted by 1
*/


/*
 * cons = sum of weights from -(m-1) to (m-1) = 1 + 2 (sum from 1 to m-1)
*/

  w[0] = 1.0;
  cons = 1.0;

  for (i = 1; i < m; i++) {
    gdouble dtmp = pow ((gdouble) i / (gdouble) m, (gdouble) kopt[0]);
    w[i] = (gdouble) pow (1.0 - dtmp, (gdouble) kopt[1]);
    cons += 2 * w[i];
  }


  cons = (gdouble) m / cons;
  for (i = 0; i < m; i++) {
    w[i] *= cons;
  }

/*
 * check if estimate extends beyond mesh
*/

  for (i = 0; i < m; i++) {
    if (nc[i] + nc[nbin - 1 - i] > 0) {
      ier = 1;
    }
  }

/*
 * compute ash(m) estimate
*/

  delta = (b - a) / (gdouble) nbin;
  h = (gdouble) m *delta;


  for (i = 0; i < nbin; i++) {
    t[i] = a + ((gdouble) i + 0.5) * delta;
    f[i] = 0.0;
    n += nc[i];
  }

  for (i = 0; i < nbin; i++) {
    if (nc[i] == 0)
      continue;
    c = (gdouble) nc[i] / ((gdouble) n * h);
    for (k = MAX (0, i - (m - 1)); k < MIN (nbin - 1, i + m); k++) {
      f[k] += (c * w[iabs (k - i)]);
    }
  }

/*
 * This is indeed a density, it integrates to 1: that is,
 * summing the areas = 1
 *
 * sum_i ( binwidth * f[i] ) = 1
*/

  return ier;
}
