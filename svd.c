/*
 * svdcomp - SVD decomposition routine.
 * Takes an mxn matrix a and decomposes it into udv, where u,v are
 * left and right orthogonal transformation matrices, and d is a
 * diagonal matrix of singular values.
 *
 * This routine is adapted from svdecomp.c in XLISP-STAT 2.1 which is
 * code from Numerical Recipes adapted by Luke Tierney and David Betz.
 *
 * Input to dsvd is as follows:
 *   a = mxn matrix to be decomposed, gets overwritten with u
 *   m = row dimension of a
 *   n = column dimension of a
 *   w = returns the vector of singular values of a
 *   v = returns the right orthogonal transformation matrix
*/

#include <math.h>
#include <gtk/gtk.h>

#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

static gdouble PYTHAG (gdouble a, gdouble b)
{
  gdouble at = fabs(a), bt = fabs(b), ct, result;

  if (at > bt)     { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
  else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
  else result = 0.0;
  return(result);
}


gint
dsvd (gfloat **a, gint m, gint n, gfloat *w, gfloat **v)
{
  gint flag, i, its, j, jj, k, l, nm;
  gdouble c, f, h, s, x, y, z;
  gdouble anorm = 0.0, g = 0.0, scale = 0.0;
  gdouble *rv1;

  if (m < n)
  {
    g_printerr ("#rows must be > #cols \n");
    return (0);
  }

  rv1 = (gdouble *) g_malloc (n*sizeof (gdouble));

/* Householder reduction to bidiagonal form */
  for (i = 0; i < n; i++)
  {
    /* left-hand reduction */
    l = i + 1;
    rv1[i] = scale * g;
    g = s = scale = 0.0;
    if (i < m)
    {
      for (k = i; k < m; k++)
        scale += fabs ((gdouble)a[k][i]);
      if (scale)
      {
        for (k = i; k < m; k++)
        {
          a[k][i] = (gfloat)((gdouble)a[k][i]/scale);
          s += ((gdouble)a[k][i] * (gdouble)a[k][i]);
        }
        f = (gdouble)a[i][i];
        g = -SIGN(sqrt (s), f);
        h = f * g - s;
        a[i][i] = (gfloat)(f - g);
        if (i != n - 1)
        {
          for (j = l; j < n; j++)
          {
            for (s = 0.0, k = i; k < m; k++)
              s += ((gdouble)a[k][i] * (gdouble)a[k][j]);
            f = s / h;
            for (k = i; k < m; k++)
              a[k][j] += (gfloat)(f * (gdouble)a[k][i]);
          }
        }
        for (k = i; k < m; k++)
          a[k][i] = (gfloat)((gdouble)a[k][i]*scale);
      }
    }
    w[i] = (gfloat)(scale * g);

    /* right-hand reduction */
    g = s = scale = 0.0;
    if (i < m && i != n - 1)
    {
      for (k = l; k < n; k++)
        scale += fabs((gdouble)a[i][k]);
      if (scale)
      {
        for (k = l; k < n; k++)
        {
          a[i][k] = (gfloat)((gdouble)a[i][k]/scale);
          s += ((gdouble)a[i][k] * (gdouble)a[i][k]);
        }
        f = (gdouble)a[i][l];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][l] = (gfloat)(f - g);
        for (k = l; k < n; k++)
          rv1[k] = (gdouble)a[i][k] / h;
        if (i != m - 1)
        {
          for (j = l; j < m; j++)
          {
            for (s = 0.0, k = l; k < n; k++)
              s += ((gdouble)a[j][k] * (gdouble)a[i][k]);
            for (k = l; k < n; k++)
              a[j][k] += (gfloat)(s * rv1[k]);
          }
        }
        for (k = l; k < n; k++)
          a[i][k] = (gfloat)((gdouble)a[i][k]*scale);
      }
    }
    anorm = MAX (anorm, (fabs ((gdouble)w[i]) + fabs(rv1[i])));
  }

  /* accumulate the right-hand transformation */
  for (i = n - 1; i >= 0; i--)
  {
    if (i < n - 1)
    {
      if (g)
      {
        for (j = l; j < n; j++)
          v[j][i] = (gfloat)(((gdouble)a[i][j] / (gdouble)a[i][l]) / g);
          /* double division to avoid underflow */
        for (j = l; j < n; j++)
        {
          for (s = 0.0, k = l; k < n; k++)
            s += ((gdouble)a[i][k] * (gdouble)v[k][j]);
          for (k = l; k < n; k++)
            v[k][j] += (gfloat)(s * (gdouble)v[k][i]);
        }
      }
      for (j = l; j < n; j++)
        v[i][j] = v[j][i] = 0.0;
    }
    v[i][i] = 1.0;
    g = rv1[i];
    l = i;
  }

  /* accumulate the left-hand transformation */
  for (i = n - 1; i >= 0; i--)
  {
    l = i + 1;
    g = (gdouble)w[i];
    if (i < n - 1)
      for (j = l; j < n; j++)
        a[i][j] = 0.0;
    if (g)
    {
      g = 1.0 / g;
      if (i != n - 1)
      {
        for (j = l; j < n; j++)
        {
          for (s = 0.0, k = l; k < m; k++)
            s += ((gdouble)a[k][i] * (gdouble)a[k][j]);
          f = (s / (gdouble)a[i][i]) * g;
          for (k = i; k < m; k++)
            a[k][j] += (gfloat)(f * (gdouble)a[k][i]);
        }
      }
      for (j = i; j < m; j++)
        a[j][i] = (gfloat)((gdouble)a[j][i]*g);
    }
    else
    {
      for (j = i; j < m; j++)
        a[j][i] = 0.0;
    }
    ++a[i][i];
  }

  /* diagonalize the bidiagonal form */
  for (k = n - 1; k >= 0; k--)
  {               /* loop over singular values */
    for (its = 0; its < 30; its++)
    {             /* loop over allowed iterations */
      flag = 1;
      for (l = k; l >= 0; l--)
      {           /* test for splitting */
        nm = l - 1;
        if (fabs (rv1[l]) + anorm == anorm)
        {
          flag = 0;
          break;
        }
        if (fabs ((gdouble)w[nm]) + anorm == anorm)
          break;
      }
      if (flag)
      {
        c = 0.0;
        s = 1.0;
        for (i = l; i <= k; i++)
        {
          f = s * rv1[i];
          if (fabs(f) + anorm != anorm)
          {
            g = (gdouble)w[i];
            h = PYTHAG(f, g);
            w[i] = (gfloat)h;
            h = 1.0 / h;
            c = g * h;
            s = (- f * h);
            for (j = 0; j < m; j++)
            {
              y = (gdouble)a[j][nm];
              z = (gdouble)a[j][i];
              a[j][nm] = (gfloat)(y * c + z * s);
              a[j][i] = (gfloat)(z * c - y * s);
            }
          }
        }
      }
      z = (gdouble)w[k];
      if (l == k)
      {          /* convergence */
        if (z < 0.0)
        {        /* make singular value nonnegative */
          w[k] = (gfloat)(-z);
          for (j = 0; j < n; j++)
            v[j][k] = (-v[j][k]);
        }
        break;
      }
      if (its >= 30) {
        g_free (rv1);
        g_printerr ("No convergence after 30,000! iterations \n");
        return(0);
      }

      /* shift from bottom 2 x 2 minor */
      x = (gdouble)w[l];
      nm = k - 1;
      y = (gdouble)w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = PYTHAG(f, 1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;

      /* next QR transformation */
      c = s = 1.0;
      for (j = l; j <= nm; j++)
      {
        i = j + 1;
        g = rv1[i];
        y = (gdouble)w[i];
        h = s * g;
        g = c * g;
        z = PYTHAG(f, h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = x * c + g * s;
        g = g * c - x * s;
        h = y * s;
        y = y * c;
        for (jj = 0; jj < n; jj++)
        {
          x = (gdouble)v[jj][j];
          z = (gdouble)v[jj][i];
          v[jj][j] = (gfloat)(x * c + z * s);
          v[jj][i] = (gfloat)(z * c - x * s);
        }
        z = PYTHAG (f, h);
        w[j] = (gfloat)z;
        if (z)
        {
          z = 1.0 / z;
          c = f * z;
          s = h * z;
        }
        f = (c * g) + (s * y);
        x = (c * y) - (s * g);
        for (jj = 0; jj < m; jj++)
        {
          y = (gdouble)a[jj][j];
          z = (gdouble)a[jj][i];
          a[jj][j] = (gfloat)(y * c + z * s);
          a[jj][i] = (gfloat)(z * c - y * s);
        }
      }
      rv1[l] = 0.0;
      rv1[k] = f;
      w[k] = (gfloat)x;
    }
  }
  g_free (rv1);
  return (1);
}
