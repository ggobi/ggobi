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
mean_stddev (gdouble *x, gfloat *mean, gfloat *stddev, gint j)
/*
 * Find the minimum and maximum values of a column or variable
 * group scaling by mean and std_width standard deviations.
 * Use the function pointer to domain_adj.
*/
{
  gint i;
  gdouble sumxi = 0.0, sumxisq = 0.0;
  gdouble dmean, dvar, dstddev;
  gdouble dn = (gdouble) xg.nrows_in_plot;

  for (i=0; i<xg.nrows_in_plot; i++) {
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
median (gfloat **data, gint jcol)
{
/*
 * Find the minimum and maximum values of each column or variable
 * group scaling by median and largest distance
*/
  gint i, m, np = xg.nrows_in_plot;
  gfloat *x;
  gdouble dmedian = 0;
  extern gint fcompare (const void *, const void *);

  x = (gfloat *) g_malloc (xg.nrows_in_plot * sizeof (gfloat));
  for (i=0; i<np; i++) {
    m = xg.rows_in_plot[i];
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

static void
transform0_values_set (gint tform_type, gint jcol)
{
  tform0 = tform_type;

  switch (tform0) {

    case NO_TFORM0:  /*-- no domain adjustment --*/
      domain_incr = 0;
      domain_adj = no_change;
      inv_domain_adj = no_change;
      break;

    case RAISE_MIN_TO_0:
      domain_incr = fabs (xg.vardata[jcol].lim_raw_gp.min);
      domain_adj = raise_min_to_0;
      inv_domain_adj = inv_raise_min_to_0;
      break;

    case RAISE_MIN_TO_1:
      domain_incr = fabs (xg.vardata[jcol].lim_raw_gp.min);
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

  xg.vardata[jcol].tform0 = tform0;
  xg.vardata[jcol].domain_incr = domain_incr;
  xg.vardata[jcol].domain_adj = domain_adj;
  xg.vardata[jcol].inv_domain_adj = inv_domain_adj;
}

static void
transform1_values_set (gint tform_type, gfloat expt, gint jcol) {
  tform1 = tform_type;
  boxcoxparam = expt;
  
  xg.vardata[jcol].tform1 = tform_type;
  xg.vardata[jcol].param = expt;
}

gboolean 
transform1_apply (gint tform_type, gfloat expt, gint jcol)
{
  gint i, m;
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
    tform_ok = transform1_apply (tform1, boxcoxparam, jcol);
    return tform_ok;
  }

  switch (tform_type)
  {
    case NO_TFORM1:    /*-- Apply the stage0 transformation --*/
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        xg.tform1[m][jcol] = (*domain_adj)(xg.raw_data[m][jcol]);
      }
      break;

    case STANDARDIZE1:    /* (x-mean)/sigma */
      {
        gfloat mean, stddev;
        gdouble *x = (gdouble *) g_malloc (xg.nrows_in_plot * sizeof (gdouble));
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          x[i] = (*domain_adj) (xg.raw_data[m][jcol]);
        }

        mean_stddev (x, &mean, &stddev, jcol);

        if (stddev == 0) {
          quick_message (domain_error_message, false);
        } else {
          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            xg.tform1[m][jcol] = ((gfloat) x[i] - mean) / stddev;
          }
        }
        g_free ((gpointer) x);
      }
      break;

    case BOXCOX:  /* Box-Cox power transform family */

      if (fabs (expt-0) < .001) {       /* Natural log */
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          if ((*domain_adj)(xg.raw_data[m][jcol]) <= 0) {
            g_printerr ("%f %f\n",
              xg.raw_data[m][jcol], (*domain_adj)(xg.raw_data[m][jcol]));
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
        }
        if (tform_ok) {  /*-- if all values are in the domain of log --*/
          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            xg.tform1[m][jcol] = (gfloat)
              log ((gdouble) ((*domain_adj)(xg.raw_data[m][jcol])));
          }
        }
      }

      else {  /*-- if the exponent is outisde (-.001, .001) --*/

        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          dtmp = pow ((gdouble) (*domain_adj)(xg.raw_data[m][jcol]), expt);
          dtmp = (dtmp - 1.0) / expt;

          /* If dtmp no good, return */
#ifdef _WIN32
          if (!_finite (dtmp)) {
#else
          if (!finite (dtmp)) {
#endif
            g_printerr ("%f %f %f\n",
              xg.raw_data[m][jcol], (*domain_adj)(xg.raw_data[m][jcol]), dtmp);
            quick_message (domain_error_message, false);
            tform_ok = false;
            break;
          }
          xg.tform1[m][jcol] = (gfloat) dtmp;
        }
      }
      break;

    case ABSVALUE:
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        xg.tform1[m][jcol] = (xg.raw_data[m][jcol] + domain_incr < 0) ?
          fabs ((gdouble)(*domain_adj)(xg.raw_data[m][jcol])) :
          (*domain_adj)(xg.raw_data[m][jcol]);
      }
      break;

    case INVERSE:    /* 1/x */
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        if ((*domain_adj)(xg.raw_data[m][jcol]) == 0) {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }

      if (tform_ok) {
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][jcol] = (gfloat)
            pow ((gdouble) (*domain_adj)(xg.raw_data[m][jcol]),
              (gdouble) (-1.0));
        }
      }
      break;

    case LOG10:    /* Base 10 log */
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        if ((*domain_adj)(xg.raw_data[m][jcol]) <= 0) {
          quick_message (domain_error_message, false);
          tform_ok = false;
          break;
        }
      }
      if (tform_ok) {  /*-- if all values are in the domain of log10 --*/
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          xg.tform1[m][jcol] = (gfloat)
            log10 ((gdouble) (*domain_adj)(xg.raw_data[m][jcol]));
        }
      }
      break;

    case SCALE01:    /* Map onto [0,1] */
      /* First find min and max; they get updated after transformations */

      min = max = (*domain_adj)(xg.raw_data[0][jcol]);
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        ref = (*domain_adj)(xg.raw_data[m][jcol]);
        if (ref < min) min = ref;
        if (ref > max) max = ref;
      }

      limits_adjust (&min, &max);
      diff = max - min;

      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        xg.tform1[m][jcol] = ((*domain_adj)(xg.raw_data[m][jcol]) - min)/diff;
      }
      break;


    case DISCRETE2:    /* x>median */
      /* refuse to discretize if all values are the same */
      allequal = true;
      ref = xg.raw_data[0][jcol];
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        if (xg.raw_data[m][jcol] != ref) {
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

      fmedian = median (xg.raw_data, jcol);
      fmedian = (*domain_adj)(fmedian);

      /* Then find the true min and max */
      min = max = (*domain_adj)(xg.raw_data[0][jcol]);
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        ref = (*domain_adj)(xg.raw_data[m][jcol]);
        if (ref < min) min = ref;
        if (ref > max) max = ref;
      }

      /* This prevents the collapse of the data in a special case */
      if (max == fmedian)
        fmedian = (min + max)/2.0;

      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        xg.tform1[m][jcol] =
          ((*domain_adj)(xg.raw_data[m][jcol]) > fmedian) ? 1.0 : 0.0;
      }
      break;

    case RANK:
    case NORMSCORE:  /*-- normscore = qnorm applied to rank --*/
    {
      paird *pairs = (paird *)
        g_malloc (xg.nrows_in_plot * sizeof (paird));
    
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        pairs[m].f = xg.raw_data[m][jcol];
        pairs[m].indx = m;
      }
      qsort ((gchar *) pairs, xg.nrows_in_plot, sizeof (paird), pcompare);
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        xg.tform1[pairs[m].indx][jcol] =
          (tform_type == RANK) ?
            (gfloat) m :
            qnorm ((gfloat) (m+1) / (gfloat) (xg.nrows_in_plot+1));
      }

      g_free ((gpointer) pairs);
    }
      break;

    case ZSCORE:
    {
      gdouble *zscore_data;
      gdouble zmean=0, zvar=0;

      /* Allocate array for z scores */
      zscore_data = (gdouble *) g_malloc (xg.nrows_in_plot * sizeof (gdouble));

      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        dtmp = (gdouble) (*domain_adj)(xg.raw_data[m][jcol]);
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
        xg.tform1[m][jcol] = (gfloat) zscore_data[m]; 
      }
      g_free ((gpointer) zscore_data);
    }
    break;
  }

  for (i=0; i<xg.nrows_in_plot; i++) {
    m = xg.rows_in_plot[i];
    xg.tform2[m][jcol] = xg.tform1[m][jcol];
  }

  return (tform_ok);
}

void
transform2_values_set (gint tform_type, gint jcol)
{
  tform2 = tform_type;
  xg.vardata[jcol].tform2 = tform_type;
}

static gboolean 
transform2_apply (gint tform_type, gint jcol)
{
  gint i, m;
  gboolean tform_ok = true;

  /*
   * If tform_type == -1, then use the parameters that are global
   * within the file.  They should correspond to the settings on the
   * variable transformation panel.
  */
  if (tform_type == -1) {
    tform_ok = transform2_apply (tform2, jcol);
    return tform_ok;
  }

  switch (tform_type)
  {
    case NO_TFORM2:  /* Restore the values from transformation, stage 2 */

      tform1_to_tform2_copy ();
      break;

    case STANDARDIZE2:    /* (x-mean)/sigma */
      {
        gfloat mean, stddev;
        gdouble *x = (gdouble *) g_malloc (xg.nrows_in_plot * sizeof (gdouble));
        for (i=0; i<xg.nrows_in_plot; i++) {
          m = xg.rows_in_plot[i];
          x[i] = (gdouble) xg.tform2[i][jcol];
        }

        mean_stddev (x, &mean, &stddev, jcol);
        if (stddev == 0) {
          quick_message (domain_error_message, false);
        } else {

          for (i=0; i<xg.nrows_in_plot; i++) {
            m = xg.rows_in_plot[i];
            xg.tform2[m][jcol] = (x[i] - mean)/stddev;
          }
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
collab_tform_update (gint j)
{
  gchar *lbl0, *lbl1;

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
    case STANDARDIZE1:
      lbl1 = g_strdup_printf ("(%s-m)/s", lbl0);
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
    case STANDARDIZE2:
      xg.vardata[j].collab_tform = g_strdup_printf ("(%s-m)/s", lbl1);
      break;
    case PERMUTE:
      break;
    case SORT:
      break;
    case SPHERE:
      xg.vardata[j].collab_tform = g_strdup_printf ("PC%d", j+1);
      break;
  }

  g_free ((gpointer) lbl0);
  g_free ((gpointer) lbl1);
}

void tform_label_update (gint jcol)
{
  /*-- update the values of the variable labels --*/
  collab_tform_update (jcol);

  /*-- update the displayed variable circle labels --*/
  varlabel_set (jcol);

  /*-- update the variable statistics table --*/
  vartable_tform_set (jcol);

  /*-- adjust the settings on the transformation panel --*/
  transform_opt_menus_set_history (jcol);
}

/*---------------------------------------------------------------------*/

/*
 * stage = 0,1,2
 * tform_type depends on the stage
 * param is the box-cox exponent, only used in stage 1
*/
void
transform_variable (gint stage, gint tform_type, gfloat param, gint jcol)
{
  switch (stage) {
    case 0:
      transform0_values_set (tform_type, jcol);

      /*-- apply tform1 to the new domain, using pre-existing parameters --*/
      /*-- if it fails, reset tform1 to NULL --*/
      if (!transform1_apply (-1, 0., jcol)) {
        transform1_values_set (NO_TFORM1, 0.0, jcol);
        transform1_apply (NO_TFORM1, 0.0, jcol);
      }

      /*-- try to apply tform2 to the new values of tform1 --*/
      /*-- if it fails, reset tform2 to NULL --*/
      if (!transform2_apply (-1, jcol)) {
        transform2_values_set (NO_TFORM2, jcol);
        tform1_to_tform2_copy ();
      }
      break;

    case 1:
      if (transform1_apply (tform_type, param, jcol)) {
        transform1_values_set (tform_type, param, jcol);
      } else { 
        transform1_values_set (NO_TFORM1, 0., jcol);
        transform1_apply (NO_TFORM1, 0., jcol);
      }

      if (!transform2_apply (-1, jcol)) {
        transform2_values_set (NO_TFORM2, jcol);
        tform1_to_tform2_copy ();
      }
      break;

    case 2:
      if (transform2_apply (tform_type, jcol)) {
        transform2_values_set (tform_type, jcol);
      } else {
        transform2_values_set (NO_TFORM2, jcol);
        tform1_to_tform2_copy ();
      }
      break;
  }

  tform_label_update (jcol);
}

void
transform (gint stage, gint tform_type, gfloat param) 
{
  gint j, k, n;
  gint ncols, nselected_cols;
  gint *selected_cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  gint *cols = (gint *) g_malloc (xg.ncols * sizeof (gint));
  gboolean doit;
/*  displayd *display = (displayd *) current_splot->displayptr;*/

  nselected_cols = selected_cols_get (selected_cols, false);
  if (nselected_cols == 0)
    nselected_cols = plotted_cols_get (selected_cols, false);

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

    if (ncols && doit) {
      for (k=0; k<ncols; k++)
        transform_variable (stage, tform_type, param, cols[k]);
    }
  }

  g_free ((gpointer) cols);
  g_free ((gpointer) selected_cols);

  vardata_lim_update ();
  tform_to_world ();

  /*
   * there's no need to reproject if the variables just transformed
   * are not currently displayed, but we're doing it anyway.
  */
  /*-- do not redisplay the missing values displays --*/
  displays_tailpipe (REDISPLAY_PRESENT);
}
