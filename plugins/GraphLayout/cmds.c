/*  Heike Hofmann */

#ifdef CMDS

#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define sqr(x) (x*x)
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define min(x,y) ((x < y) ? x : y)
#define max(x,y) ((x > y) ? x : y)


void printX (gdouble **X, gint n, gboolean upper);
/*
const long N = 10;
void forrest (double **a, int n);
*/

gdouble **matrix (gint r, gint c);
gdouble *vector (gint r);

void choldc(gdouble **a, gint n, gdouble p[]);
gdouble pythag(gdouble a, gdouble b);
void tred2(gdouble **a, gint n, gdouble *d, gdouble *e);
void tqli(gdouble d[], gdouble e[], gint n, gdouble **z);
void tqlialt(gdouble d[], gdouble e[], gint n, gdouble **z);

void eigensort (gdouble **a, gdouble ew[], gint n);
void recoverX (gdouble **x, gdouble d[], gint n);

gdouble tr(gdouble **a, gint n);

void nrerror(gchar *string) {
  printf ("\nError: %s\n",string);
  /*abort;*/
}


gdouble tr(gdouble **a, gint n) {
  gint i;
  gdouble res = 0;
  for (i = 0; i < n; i++) {
    res += a[i][i];
  }
  return res;
}

gdouble **matrix (gint r, gint c) {
  gint i;
  gdouble **result = (gdouble **)malloc (sizeof(gdouble)*r);
  result[0] = (gdouble *)malloc (sizeof(gdouble)*r*c);
  for (i = 1; i < r; i++)
    result[i] = result[0] + i*sizeof(gdouble)*r;
    
  return result;
}

gdouble *vector (gint r) {
  return (gdouble *)malloc (sizeof(gdouble)*r);
}

void choldc(gdouble **a, gint n, gdouble p[])
/*  Given a positive-definite symmetric matrix a[1..n][1..n], this routine
constructs its Cholesky A = L L^T . On input, only the upper triangle of a need be given;
it is not modified. The Cholesky factor L is returned in the lower
triangle of a, except for its diagonal elements which are returned in p[1..n]. */
{
  gint i,j,k;
  gdouble sum;

  for (i=0;i<n;i++) {
    for (j=0;j<n;j++) {
      sum=a[i][j];
      for (k=i-1;k>=0;k--) sum -= a[i][k]*a[j][k];
      
      if (i == j) {
        if (sum < 0.0) { /*  a, with rounding errors, is not positive definite. */
      /*  if (sum <= 0.0)  a, with rounding errors, is not positive definite. */
          printf("%.10lf < 0  (%lf)\n",sum, sum-0);
          nrerror("choldc failed");
        }
        p[i]=sqrt(sum);
      } else a[j][i]=sum/p[i];
    }
  }
}



gdouble pythag(gdouble a, gdouble b)
/*  Computes (a 2 + b 2 ) 1=2 without destructive underflow or overflow.  */
{
  gdouble absa,absb;
  absa=fabs(a);
  absb=fabs(b);
  if (absa > absb) return absa*sqrt(1.0+sqr(absb/absa));
  else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+sqr(absa/absb)));
}

void printX (gdouble **X, gint n, gboolean upper) {
  gint i,j;
  
  printf ("\n");
  if (upper) {
    for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++) {
        printf ("%.3f\t",X[i][j]);
      }
      printf ("\n");
    }
  } else {
    for (i = 0; i < n; i++) {
      for (j = 0; j < i; j++) {
        printf ("%.3f\t",X[i][j]);
      }
      for (j = i; j < n; j++) {
        printf ("%.3f\t",X[j][i]);
      }  
      printf ("\n");
    }
  }
}

/* 
QL algorithm with implicit shifts, to determine the eigenvalues and
eigenvectors of a real, symmetric, tridiagonal matrix, or of a real,
symmetric matrix previously reduced by tred2 x 11.2. On input,
d[1..n] contains the diagonal elements of the tridiagonal matrix.

On output, it returns the eigenvalues. The vector e[1..n] inputs the
subdiagonal elements of the tridiagonal matrix, with e[1] arbitrary.
On output e is destroyed. When finding only the eigenvalues, several
lines may be omitted, as noted in the comments. If the eigenvectors
of a tridiagonal matrix are desired, the matrix z[1..n][1..n] is
input as the identity matrix. If the eigenvectors of a matrix that
has been reduced by tred2 are required, then z is input as the matrix
output by tred2.

In either case, the kth column of z returns the normalized
eigenvector corresponding to d[k]. 
*/
void tqli(gdouble d[], gdouble e[], gint n, gdouble **z)
{
  gint m,l,iter,i,k;
  gdouble s,r,p,g,f,dd,c,b;

  /*  Convenient to renumber the elements of e.  */
  for (i=1;i<n;i++) e[i-1]=e[i]; 
  e[n-1]=0.0;

  for (l=0;l<n;l++) {
    iter=0;
    do {
      /*  Look for a single small subdiagonal element to split
          the matrix.  */
      for (m=l;m<n-1;m++) { 
        dd=fabs(d[m])+fabs(d[m+1]);
        if ((gdouble)(fabs(e[m])+dd) == dd) break;
      }
      if (m != l) {
        if (iter++ == 30) nrerror("Too many iterations in tqli");
        g=(d[l+1]-d[l])/(2.0*e[l]);  /*   Form shift.  */
        r=pythag(g,1.0);
        g=d[m]-d[l]+e[l]/(g+SIGN(r,g));  /*  This is d_m - k_s  */
        s=c=1.0;
        p=0.0;
        /*  A plane rotation as in the original QL, followed by Givens
            rotations to restore tridiagonal form.  */
        for (i=m-1;i>=l;i--) { 
          f=s*e[i];
          b=c*e[i];      
          e[i+1]=(r=pythag(f,g));
          if (r == 0.0) {  /*  Recover from underflow.  */
            d[i+1] -= p;
            e[m]=0.0;
            break;
          }
          s=f/r;
          c=g/r;
          g=d[i+1]-p;
          r=(d[i]-g)*s+2.0*c*b;
          p=s*r;
          d[i+1]=g+p;
          g=c*r-b;
          /* Next loop can be omitted if eigenvectors not wanted*/
          for (k=0;k<n;k++) {  /*  Form eigenvectors.  */
            f=z[k][i+1];
            z[k][i+1]=s*z[k][i]+c*f;
            z[k][i]=c*z[k][i]-s*f;
          }
        }
        if (r == 0.0 && i >= l) continue;
        d[l] -= p;
        e[l]=g;
        e[m]=0.0;
      }
    } while (m != l);
  }
}


void tqlialt(gdouble d[], gdouble e[], gint n, gdouble **z)
{
  gint i,k,m,l,iter;
  gdouble s,r,p,g,f,dd,c,b;
  
  for (i = 1; i < n; i++) e[i-1] = e[i];
  e[n-1] = 0;
  for (l = 0; l < n; l++) {
  
    iter = 0;
    do {
      for (m = l; m < n-1; m++) {
        dd = fabs(d[m])+fabs(d[m+1]);
        if (fabs(e[m])+dd == dd) break;
      }
      
      if (m != l) {
        if (iter++ == 30) {
          nrerror ("too many iterations in tqlialt\n");
          return;
        }
        g = (d[l+1]-d[l])/(2.0*e[l]);
        r = sqrt(g*g+1);
        g = d[m]- d[l]+e[l]/(g+SIGN(r,g));
        s = c = 1.0;
        p = 0.0;
        for (i = m-1; i >= l; i--) {
          f = s * e[i];
          b = c * e[i];
          if (fabs(f) >= fabs (g)) {
            c = g/f;
            r = sqrt (c*c+1);
            e [i+1] = f*r;
            s = 1.0/r;
            c *= s;
          } else {
            s = f/g;
            r = sqrt (s*s+1);
            e[i+1] = g*r;
            c = 1.0/r;
            s *= c;
          }
          g = d[i+1] - p;
          r = (d[i] - g)*s+2.0*c*b;
          p = s*r;
          d[i+1] = g+p;
          g = c * r-b;
          for (k = 0; k < n; k++) {
            f = z[k][i+1];
            z[k][i+1] = s*z[k][i]+c*f;
            z[k][i] = c * z[k][i]-s*f;
          }
        }    
        d[l] = d[l] -p;
        e [l] = g;
        e [m] = 0.0;  
      }
    } while (m != l);
  
  }
}


void tred2 (gdouble **a, gint n, gdouble *d, gdouble *e)
/* 
Householder reduction of a real, symmetric matrix a[1..n][1..n]. On
output, a is replaced by the orthogonal matrix q effecting the
transformation.  d[1..n] returns the diagonal elements of the
tridiagonal matrix, and e[1..n] the off-diagonal, with e[1]=0.
Several statements, as noted in comments, can be omitted if only
eigenvalues are to be found, in which case a contains no useful
information on output. Otherwise they are to be included.
*/
{
  gint l,k,j,i;
  gdouble scale,hh,h,g,f;

  for (i=n-1;i>0;i--) {
    l=i-1;
    h=scale=0.0;
    if(l > 0) {
      for (k=0;k<=l;k++)
        scale += fabs(a[i][k]);
      if (scale == 0.0) /*  Skip transformation.  */
        e[i]=a[i][l];
      else {
        for (k=0;k<=l;k++) {
          a[i][k] /= scale;  /*  Use scaled a's for transformation.  */
          h += a[i][k]*a[i][k]; 
        }
        f=a[i][l];
        g=(f >= 0.0 ? -sqrt(h) : sqrt(h));
        e[i]=scale*g;
        h -= f*g; /*  Now h is equation (11.2.4).  */
        a[i][l]=f-g;  /*  Store u in the ith row of a.  */
        f=0.0;
        for (j=0;j<=l;j++) {
          /* Next statement can be omitted if eigenvectors not wanted */
          a[j][i]=a[i][j]/h; /*  Store u=H in ith column of a.  u in g. */
          g = 0.0;
        
          for (k=0;k<=j;k++)
            g += a[j][k]*a[i][k];
          
          for (k=j+1;k<=l;k++)
            g += a[k][j]*a[i][k];
          e[j]=g/h; /* Form element of p in temporarily unused element of e. */
          f += e[j]*a[i][j];
        }
        hh=f/(h+h); /*  Form K, equation (11.2.11). */
        for (j=0;j<=l;j++) { /* Form q and store in e overwriting p. */
          f=a[i][j];
          e[j]=g=e[j]-hh*f;
          for (k=0;k<=j;k++) /*  Reduce a, equation (11.2.13).  */
            a[j][k] -= (f*e[k]+g*a[i][k]);
        }
      }
    } else
      e[i]=a[i][l];
    d[i]=h;
g_printerr (".");
  }
g_printerr ("\n");

  /* Next statement can be omitted if eigenvectors not wanted */
  d[0]=0.0;
  e[0]=0.0;
  /*
   * Contents of this loop can be omitted if eigenvectors not
   * wanted except for statement d[i]=a[i][i];
  */
  for (i=0;i<n;i++) { /*  Begin accumulation of transformation matrices. */
    l=i-1;
    if (d[i]) { /*  This block skipped when i=1.  */
      for (j=0;j<=l;j++) {
        g=0.0;
        for (k=0; k<=l; k++)
          g += a[i][k]*a[k][j];
        for (k=0;k<=l;k++)
          a[k][j] -= g*a[k][i];
      }
    }
    d[i]=a[i][i];  /*  This statement remains.  */
    /*  Reset row and column of a to identity matrix for next iteration.  */
    a[i][i]=1.0; 
    
    for (j=0;j<=l;j++) a[j][i]=a[i][j]=0.0;
g_printerr (".");
  }
g_printerr ("\n");
}

void eigensort (gdouble **a, gdouble ew[], gint n)
{
  gint i,j,k;
  gdouble p;
  
  for (i = 0; i < n; i++) {
    k = i;
    p = ew[k];
    for (j = i+1; j < n; j++) if (ew[j] > p) p = ew[k=j];
    if (k != i) {
      ew[k]= ew[i];
      ew[i] = p;
      for (j = 0; j < n; j++) {
        p = a[j][i]; 
        a[j][i] = a[j][k];
        a[j][k] = p;
      }
    }
  }
}


gint cmds (array_d *D, array_d *X)
{
  gint N = D->nrows;
    /*  D_ij^2 = ||X_i  - x_j||^2  */ 
  gdouble C = 0;    /*  C = 2/N sum_ij (D_ij^2)    */
  array_d c;
  gint i, j, ncols;
  gdouble sum;
    /*  c_ij = <x_i, x_j>      */
  gdouble *d = g_malloc (N * sizeof (gdouble));
  gdouble *e = g_malloc (N * sizeof (gdouble));

  arrayd_init_null (&c);
  arrayd_alloc (&c, N, N);
  
  /* diagonal and off-diagonal elements of tridiagonalisation of c (e[0]=0) */

/* compute constant C */
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
      C += sqr(D->vals[i][j]);
  }
  C /= (gdouble) (N*N);

/*  recover scalar products from distances */
  /*  diagonal elements */
  for (i = 0; i < N; i++) {
    sum = 0;
    for (j = 0; j < N; j++) 
      sum += sqr(D->vals[i][j]);
    
    c.vals[i][i] = sum/N - .5*C;
  }

  /*  upper triangle */
  for (i = 0; i < N; i++) {
    for (j = i+1; j < N; j++) {
      c.vals[j][i] = c.vals[i][j] =
        -.5*(sqr(D->vals[i][j]) - c.vals[i][i] - c.vals[j][j]);
    }
  }

/*
  printf("matrix of scalar products:");
  printX(c,N,1);
  printf("\ntrace of c: %.2f\n",tr(c,N));
*/


/*  calculate eigenvectors and eigenvalues */

  /*  tridiagonalize c into d (diagonal) and e (off-diagonal) */
  tred2(c.vals,N,d,e);  
g_printerr ("through tred2\n");

/*
  printf("diagonal:\n"); 
  for (i = 0; i < N; i++) { printf("%.2f ",d[i]); }
  printf("\noff-diagonal:"); 
  for (i = 1; i < N; i++) { printf("%.2f ",e[i]); }
  printf("\n"); 
*/

  /* 
   * Tridiagonal QL Implicit: calculate eigenvectors and -values from
   * a tridiagonal matrix
  */
  tqli (d, e, N, c.vals);
g_printerr ("through tqli\n");
  eigensort (c.vals, d, N);
g_printerr ("through eigensort\n");

  sum = 0;
  for (i = 0; i < N; i++) {
    sum += d[i];
  }
g_printerr ("trace of c after decomposition: %.2f\n",sum); 

/*
  printf("\nX:\n"); 
  recoverX (c.vals, d, N);
  printX(c,N,1);
*/
  ncols = MIN (N, X->ncols);
  for (i = 0; i < N; i++) {
    for (j = 0; j < ncols; j++) {
      if (d[j] > 0) 
        X->vals[i][j] = c.vals[i][j] * sqrt(d[j]);
      else X->vals[i][j] = 0;
    }
  }

  arrayd_free (&c, 0, 0);
  g_free (d);
  g_free (e);

  return 0;
}

#endif
