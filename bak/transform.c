/* transform.c */

#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define SIGNUM(x) (((x)<0.0)?(-1.0):(((x)>0.0)?(1.0):(0.0)))

/* */
extern gint finite (gdouble);  /*-- defined in math.h, but being ignored --*/
extern gdouble erf (gdouble);  /*-- defined in math.h, but being ignored --*/
/* */

static gchar *domain_error_message = "Data outside the domain of function.";

/*
 * These variables should always match the displayed values in
 * the variable transformation panel -- domain_adj, too
*/
static gint    tform0 = NO_TFORM0;
static gfloat  domain_incr = 0.0;
static gint    tform1 = NO_TFORM1;
static gfloat  boxcoxparam = 1.0;
static gint    tform2 = NO_TFORM2;
/* */

gfloat no_change (gfloat x)      { return x; }
gfloat negate (gfloat x)         { return -x; }

gfloat raise_min_to_0 (gfloat x) { return (x + domain_incr); }
gfloat raise_min_to_1 (gfloat x) { return (x + domain_incr + 1.0); }
static gfloat (*domain_adj) (gfloat x) = no_change;

gfloat inv_raise_min_to_0 (gfloat x) { return (x - domain_incr); }
gfloat inv_raise_min_to_1 (gfloat x) { return (x - domain_incr - 1.0); }
static gfloat (*inv_domain_adj) (gfloat x) = no_change;

typedef struct {
  gfloat f;
  gint indx;
} paird;

static gint
pcompare (const void *val1, const void *val2)
{
  const paird *pair1 = (const paird *) val1;
  const paird *pair2 = (const paird *) val2;

  if (pair1->f < pair2->f)
    return (-1);
  else if (pair1->f == pair2->f)
    return (0);
  else
    return (1);
}


gint
cols_in_group (gint *cols, gint varno) {
/*
 * Figure out which columns are in the same vgroup as varno
*/
  gint j, ncols = 0;
  gint groupno = xg.vardata[varno].groupid;

  for (j=0; j<xg.ncols; j++) {
    if (xg.vardata[j].groupid == groupno)
      cols[ncols++] = j;
  }
  return (ncols);
}


static void
mean_stddev (gint *cols, gint ncols, gfloat (*stage1) (gfloat),
  gfloat *mean, gfloat *stddev)
/*
 * Find the minimum and maximum values of a column or variable
 * group scaling by mean and std_width standard deviations.
 * Use the function pointer to domain_adj.
*/
{
  gint i, j, m, n;
  gdouble sumxi = 0.0, sumxisq = 0.0;
  gdouble dx, dmean, dvar, dstddev;
  gdouble dn = (gdouble) (ncols * xg.nrows_in_plot);

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg.nrows_in_plot; i++) {
      m = xg.rows_in_plot[i];
      dx = (gdouble) (*stage1)(xg.raw_data[m][j]);
      sumxi = sumxi + dx;
      sumxisq = sumxisq + dx * dx;
    }
  }
  dmean = sumxi / dn;
  dvar = (sumxisq / dn) - (dmean * dmean);
  dstddev = sqrt (dvar);

  *mean = (gfloat) dmean;
  *stddev = (gfloat) dstddev;
}

gfloat
median (gfloat **data, gint *cols, gint ncols)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by median and largest distance
*/
  gint i, j, m, n, np;
  gfloat *x;
  gdouble dmedian = 0;
  extern gint fcompare (const void *, const void *);

  np = ncols * xg.nrows_in_plot;
  x = (gfloat *) g_malloc (np * sizeof (gfloat));
  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg.nrows_in_plot; i++) {
      m = xg.rows_in_plot[i];
      x[n*xg.nrows_in_plot + m] = data[m][j];
    }
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
  if(p > 0.5) p = 1.0 - pr;

  /*  depending on the size of pr this may error in log or sqrt */
  eta  = sqrt (-2.0 * log (p));
  term = ((f1*eta+f2) * eta + f3) / (((f4*eta+f5)*eta+f6) * eta + 1.0);
  if (pr <= .5)
    return (term - eta);
  else return (eta - term);
}


/*-- reset the transformation for all variables, not just selected ones --*/
void
reset_tform_all () {
  int j;

  for (j=0; j<xg.ncols; j++) {
    xg.vardata[j].tform1 = NO_TFORM1;
    xg.vardata[j].tform2 = NO_TFORM2;
    xg.vardata[j].domain_incr = 0.;
    xg.vardata[j].param = 0.;
    xg.vardata[j].domain_adj = no_change;
    xg.vardata[j].inv_domain_adj = no_change;
  }
}

void
transform0_values_set (gint tform_type, gint *cols, gint ncols)
{
  gint j;
  gfloat t1, tincr;

  tform0 = tform_type;

  switch (tform0) {

    case NO_TFORM0:  /*-- no domain adjustment --*/
      domain_incr = 0;
      domain_adj = no_change;
      inv_domain_adj = no_change;
      break;

    case RAISE_MIN_TO_0:
      tincr = fabs (xg.vardata[cols[0]].lim_raw_gp.min);
      for (j=1; j<ncols; j++)
        if ((t1 = fabs (xg.vardata[cols[j]].lim_raw_gp.min)) > tincr)
          tincr = t1;

      domain_incr = tincr;
      domain_adj = raise_min_to_0;
      inv_domain_adj = inv_raise_min_to_0;
      break;

    case RAISE_MIN_TO_1:
      tincr = fabs (xg.vardata[cols[0]].lim_raw_gp.min);
      for (j=1; j<ncols; j++)
        if ( (t1 = fabs (xg.vardata[cols[j]].lim_raw_gp.min)) > tincr)
          tincr = t1;
      domain_incr = tincr;

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

  for (j=0; j<ncols; j++) {
    xg.vardata[cols[j]].tform0 = tform0;
    xg.vardata[cols[j]].domain_incr = domain_incr;
    xg.vardata[cols[j]].domain_adj = domain_adj;
    xg.vardata[cols[j]].inv_domain_adj = inv_domain_adj;
  }
}

void
transform1_values_set (gint tform_type, gfloat expt, gint *cols, gint ncols) {
  gint j;

  tform1 = tform_type;
  boxcoxparam = expt;
  
  for (j=0; j<ncols; j++) {
    xg.vardata[cols[j]].tform1 = tform_type;
    xg.vardata[cols[j]].param = expt;
  }
}

gboolean 
transform1_apply (gint tform_type, gfloat expt, gint *cols, gint ncols)
{
  gint i, j, m, n;
  gfloat min, max, diff;
  gfloat fmedian, ref;
  gboolean allequal, tform_ok = true;
  gdouble dtmp;

  /*
   * If tform_type == -1, then use the parameters that are global
   * within the file.  They should correspond to the settings on the
   * variable transformation panel.
  */
  if (tform_type == -1) {
    tform_ok = transform1_apply (tform1, boxcoxparam, cols, ncols);
    return tform_ok;
  }

  switch (tform_type)
  {
    case NO_TFORM1:    /*-- Apply the stage0 transformation --*/
      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][j] = (*domain_adj)(xg.raw_data[m][j]);
        }
      }
      break;

    case BOXCOX:  /* Box-Cox power transform family */

      if (fabs (expt-0) < .001) {       /* Natural log */
        for (n=0; n<ncols; n++) {
          if (!tform_ok) break;
          j = cols[n];
          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            if ((*domain_adj)(xg.raw_data[m][j]) <= 0) {
              g_printerr ("%f %f\n",
                xg.raw_data[m][j], (*domain_adj)(xg.raw_data[m][j]));
              quick_message (domain_error_message, false);
              tform_ok = false;
              break;
            }
          }
        }
        for (n=0; n<ncols; n++) {
          j = cols[n];
          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            xg.tform1[m][j] = (gfloat)
              log ((gdouble) ((*domain_adj)(xg.raw_data[m][j])));
          }
        }
      }

      else {

        for (n=0; n<ncols; n++) {
          if (!tform_ok) break;
          j = cols[n];
          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            dtmp = pow ((gdouble) (*domain_adj)(xg.raw_data[m][j]), expt);
            dtmp = (dtmp - 1.0) / expt;

            /* If dtmp no good, return */
#ifdef _WIN32
            if (!_finite (dtmp)) {
#else
            if (!finite (dtmp)) {
#endif
              g_printerr ("%f %f %f\n",
                xg.raw_data[m][j], (*domain_adj)(xg.raw_data[m][j]), dtmp);
              quick_message (domain_error_message, false);
              tform_ok = false;
              break;
            }
            xg.tform1[m][j] = (gfloat) dtmp;
          }
        }
      }
      break;

    case ABSVALUE:
      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][j] = (xg.raw_data[m][j] + domain_incr < 0) ?
            fabs ((gdouble)(*domain_adj)(xg.raw_data[m][j])) :
            (*domain_adj)(xg.raw_data[m][j]);
        }
      }
      break;

    case INVERSE:    /* 1/x */
      for (n=0; n<ncols; n++) {
        if (!tform_ok) break;
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if ((*domain_adj)(xg.raw_data[m][j]) == 0) {
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
        }
      }

      if (tform_ok) {
        for (n=0; n<ncols; n++) {
          j = cols[n];
          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            xg.tform1[m][j] = (gfloat)
              pow ((gdouble) (*domain_adj)(xg.raw_data[m][j]),
                (gdouble) (-1.0));
          }
        }
      }
      break;

    case LOG10:    /* Base 10 log */
      for (n=0; n<ncols; n++) {
        if (!tform_ok) break;
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if ((*domain_adj)(xg.raw_data[m][j]) <= 0) {
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
        }
      }
      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][j] = (gfloat)
            log10 ((gdouble) (*domain_adj)(xg.raw_data[m][j]));
        }
      }
      break;

    case SCALE01:    /* Map onto [0,1] */
      /* First find min and max; they get updated after transformations */

      min = max = (*domain_adj)(xg.raw_data[0][cols[0]]);
      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if ((ref = (*domain_adj)(xg.raw_data[m][j])) < min)
            min = ref;
          else if (ref > max) max = ref;
        }
      }

      limits_adjust (&min, &max);
      diff = max - min;

      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++)
          m = xg.rows_in_plot[i];
          xg.tform1[m][j] = ((*domain_adj)(xg.raw_data[m][j]) - min)/diff;
      }
      break;


    case DISCRETE2:    /* x>median */
      /* refuse to discretize if all values are the same */
      allequal = true;
      ref = xg.raw_data[0][cols[0]];
      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if (xg.raw_data[m][j] != ref) {
            allequal = false;
            break;
          }
        }
      }

      if (allequal) {
        quick_message (domain_error_message, false);
        tform_ok = false;
        break;
      }

      /* First find median */

      fmedian = median (xg.raw_data, cols, ncols);
      fmedian = (*domain_adj)(fmedian);

      /* Then find the true min and max */
      min = max = (*domain_adj)(xg.raw_data[0][cols[0]]);
      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if ( (ref = (*domain_adj)(xg.raw_data[m][j])) < min)
            min = ref;
          else if (ref > max) max = ref;
        }
      }

      /* This prevents the collapse of the data in a special case */
      if (max == fmedian)
        fmedian = (min + max)/2.0;

      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][j] =
            ((*domain_adj)(xg.raw_data[m][j]) > fmedian) ? 1.0 : 0.0;
        }
      }
      break;

    case RANK:
    case NORMSCORE:  /*-- normscore = qnorm applied to rank --*/
    {
      paird *pairs = (paird *)
        g_malloc (xg.nrows_in_plot * sizeof (paird));
    
      for (n=0; n<ncols; n++) {
        j = cols[n];

        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          pairs[m].f = xg.raw_data[m][j];
          pairs[m].indx = m;
        }
        qsort ((gchar *) pairs, xg.nrows_in_plot, sizeof (paird), pcompare);
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[pairs[m].indx][j] =
            (tform_type == RANK) ?
              (gfloat) m :
              qnorm ((gfloat) (m+1) / (gfloat) (xg.nrows_in_plot+1));
        }
      }

      g_free ((gpointer) pairs);
    }
      break;

    case ZSCORE:
    {
      gdouble *zscore_data;

      /* Allocate array for z scores */
      zscore_data = (gdouble *) g_malloc (xg.nrows_in_plot * sizeof (gdouble));

      for (n=0; n<ncols; n++) {
        gdouble zmean=0, zvar=0;
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          dtmp = (gdouble) (*domain_adj)(xg.raw_data[m][j]);
          zscore_data[m] = dtmp;
          zmean += dtmp;
          zvar += (dtmp * dtmp);
        }
        zmean /= (gdouble) xg.nrows_in_plot;
        zvar = sqrt (zvar / (gdouble) xg.nrows_in_plot - zmean*zmean);
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          zscore_data[m] = (zscore_data[m] - zmean) / zvar;
        }

        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if (zscore_data[m] > 0)
            zscore_data[m] =
              erf (zscore_data[m]/sqrt (2.)) / 2.8284271+0.5;
          else if (zscore_data[m]<0)
            zscore_data[m] = 0.5 - erf (fabs
              (zscore_data[m])/sqrt (2.))/2.8284271;
          else 
            zscore_data[m] = 0.5;
        }
        
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][j] = (gfloat) zscore_data[m]; 
        }
      }
      g_free ((gpointer) zscore_data);
    }
    break;
  }

  for (n=0; n<ncols; n++) {
    j = cols[n];
    for (i=0; i<xg.nrows_in_plot; i++) {
      m = xg.rows_in_plot[i];
      xg.tform2[m][j] = xg.tform1[m][j];
    }
  }

  return (tform_ok);
}

gboolean 
transform2_values_set (gint tform_type, gint *cols, gint ncols) {
  gint j;

  tform2 = tform_type;

  for (j=0; j<ncols; j++)
    xg.vardata[cols[j]].tform2 = tform_type;

  return true;
}

gboolean 
transform2_apply (gint tform_type, gint *cols, gint ncols) {
  gint i, j, m, n;
  gfloat mean, stddev;
  gboolean tform_ok = true;

  /*
   * If tform_type == -1, then use the parameters that are global
   * within the file.  They should correspond to the settings on the
   * variable transformation panel.
  */
  if (tform_type == -1) {
    tform_ok = transform2_apply (tform2, cols, ncols);
    return tform_ok;
  }

  switch (tform_type)
  {
    case NO_TFORM2:  /* Restore the values from transformation, stage 2 */

      tform1_to_tform2_copy ();
      break;

    case STANDARDIZE:    /* (x-mean)/sigma */

      mean_stddev (cols, ncols, domain_adj, &mean, &stddev);
      /* DOMAIN_ERROR if stddev == 0 */

      for (n=0; n<ncols; n++) {
        j = cols[n];
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform2[m][j] = ((*domain_adj)(xg.raw_data[m][j]) - mean)/stddev;
        }
      }
      break;

    case PERMUTE:
    case SORT:
    case SPHERE:
  }
  return tform_ok;
}

/*
 * update the labels <after> transform has completed, so that
 * we're ready for any sort of success or failure
*/
void
collab_tform_update (gint *cols, gint ncols)
{
  gchar *lbl0, *lbl1;
  gint n, j;

  for (n=0; n<ncols; n++) {
    j = cols[n];

    g_free ((gpointer) xg.vardata[j].collab_tform);

    /*-- skip the stage0 changes except negation --*/
    switch (xg.vardata[j].tform0) {
      case NEGATE:
        lbl0 = g_strdup_printf ("-%s", xg.vardata[j].collab);
        break;
      default:
        lbl0 = g_strdup (xg.vardata[j].collab);
        break;
    }

    switch (xg.vardata[j].tform1) {
      case NO_TFORM1:
        lbl1 = g_strdup (lbl0);
        break;
      case BOXCOX:
        lbl1 = g_strdup_printf ("B-C(%s,%.2f)", lbl0, xg.vardata[j].param);
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

    switch (xg.vardata[j].tform2) {
      case NO_TFORM2:
        xg.vardata[j].collab_tform = g_strdup_printf (lbl1);
        break;
      case STANDARDIZE:
        xg.vardata[j].collab_tform = g_strdup_printf ("(%s-m)/s", lbl1);
        break;
      case PERMUTE:
        break;
      case SORT:
        break;
      case SPHERE:
        break;
    }

    g_free ((gpointer) lbl0);
    g_free ((gpointer) lbl1);
  }
}

/*---------------------------------------------------------------------*/

/*
 * stage = 0,1,2
 * tform_type depends on the stage
 * param is the box-cox exponent, only used in stage 1
*/
void
transform_variable (gint jcol, gint stage, gint tform_type, gfloat param)
{
  gint n;

  switch (stage) {
    case 0:
      transform0_values_set (tform_type, cols, ncols);

      /*-- apply tform1 to the new domain, using pre-existing parameters --*/
      /*-- if it fails, reset tform1 to null --*/
      if (!transform1_apply (-1, 0., cols, ncols)) {
        transform1_values_set (NO_TFORM1, 0.0, cols, ncols);
        transform1_apply (NO_TFORM1, 0.0, cols, ncols);
      }

      /*-- try to apply tform2 to the new values of tform1 --*/
      /*-- if it fails, reset tform2 to null --*/
      if (!transform2_apply (-1, cols, ncols)) {
        transform2_values_set (NO_TFORM2, cols, ncols);
        tform1_to_tform2_copy ();
      }
      break;

    case 1:
      if (transform1_apply (tform_type, param, cols, ncols)) {
        transform1_values_set (tform_type, param, cols, ncols);
      } else { 
        transform1_values_set (NO_TFORM1, 0., cols, ncols);
        transform1_apply (NO_TFORM1, 0., cols, ncols);
      }

      if (!transform2_apply (-1, cols, ncols)) {
        transform2_values_set (NO_TFORM2, cols, ncols);
        tform1_to_tform2_copy ();
      }
      break;

    case 2:
      if (transform2_apply (tform_type, cols, ncols)) {
        transform2_values_set (tform_type, cols, ncols);
      } else {
        transform2_values_set (NO_TFORM2, cols, ncols);
        tform1_to_tform2_copy ();
      }
      break;
  }

  /*-- update the values of the variable labels --*/
  collab_tform_update (cols, ncols);

  /*-- update the displayed variable circle labels --*/
  for (n=0; n<ncols; n++)
    varlabel_set (cols[n]);

  /*-- update the variable statistics table --*/
  for (n=0; n<ncols; n++)
    vartable_tform_set (cols[n]);

  /*-- adjust the settings on the transformation panel --*/
  for (n=0; n<ncols; n++)
    transform_opt_menus_set_history (cols[n]);
}

void
transform (gint stage, gint tform_type, gfloat param) 
{
  gint j, k, n;
  gint ncols, nselected_cols;
  gint *selected_cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  gint *cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  gboolean doit, clear_vartable;
  displayd *display = (displayd *) current_splot->displayptr;

  nselected_cols = get_selected_cols (selected_cols, false);

  /*-- if ncols == 0, choose all currently plotted columns --*/
  clear_vartable = false;
  if (nselected_cols == 0) {
    gint mode = mode_get ();
    clear_vartable = true;
    for (j=0; j<xg.ncols; j++) {
      /*-- if j is plotted in the current splot ... --*/
      switch (display->displaytype) {
        case scatterplot:
          switch (mode) {
            case P1PLOT:
              vartable_select_var (current_splot->p1dvar, true);
              break;
            case XYPLOT:
              vartable_select_var (current_splot->xyvars.x, true);
              vartable_select_var (current_splot->xyvars.y, true);
              break;
          }
          break;
        case scatmat:
          break;
        case parcoords:
          break;
      }
    }
    nselected_cols = get_selected_cols (selected_cols, false);
  }

  /*-- loop over selected columns, getting vgroup members --*/
  for (j=0; j<nselected_cols; j++) {
    n = selected_cols[j];
    doit = true;
    ncols = get_vgroup_cols (n, cols);
    for (k=0; k<ncols; k++) {
      if (cols[k] < n) {  /*-- depend on vgroups being sorted --*/
        doit = false;  /*-- this vgroup has already been transformed --*/
        break;
      }
    }

    if (ncols && doit) 
      transform_group (cols, ncols, stage, tform_type, param);
  }

  /*-- if we faked the selected variables, now deselect them --*/
  if (clear_vartable) 
    vartable_unselect_all ();

  g_free ((gpointer) cols);
  g_free ((gpointer) selected_cols);

  vardata_lim_update ();
  tform_to_world ();

  /*
   * there's no need to reproject if the variables just transformed
   * are not currently displayed, but we're doing it anyway.
  */
  /*-- do not redisplay the missing values displays --*/
  displays_reproject (REDISPLAY_PRESENT);
}

