/********************************************************************
             Arbitrary dimensional projection pursuit indices
*********************************************************************/

#include "defines.h"
#include "projection-indices.h"
#include <math.h>
#include <string.h>

// Compute group counts 
guint compute_groups(vector_i group, vector_i ngroup, vector_d groups) { 
  gint i, j;
  guint numgroups = 0;
  guint nrows = groups.nels;
  
  gint* groupval = g_new0(gint, nrows);

  for (i=0; i < nrows; i++) {
    for (j=0; j < numgroups; j++) { 
      if (groupval[j] == group.els[i]) { 
        ngroup.els[j]++;
        break;
      }
    }
    if (j == numgroups ) { 
      groupval[j]  = group.els[i];
      ngroup.els[j] = 1;
      numgroups++;
    }
  }

  for (i=0; i < nrows; i++) { 
    for (j=0; j < numgroups; j++) { 
      if (groupval[j] == group.els[i])
        group.els[i] = j;
    }
  }

  g_free(groupval);

  return numgroups;
}

/*****************************************************/
/*               Utility Routines                    */
/*                                                   */
/* Reference : An Introduction to Numerical Analysis */
/*             - Kendall E. Atkinson                 */
/*             (p 449 - 450)                         */
/*****************************************************/

gdouble ludcmp(gdouble *a, gint n, gint *Pivot) 
{ 
  gint i,j,k,ier;
  gdouble *s,det,temp,c = 0;
  det=1;

  s = (gdouble *) g_malloc(n*sizeof(gdouble));

  for (i=0;i<n; i++) {  
    s[i] = a[i*n+1];
    for(j=1; j<n; j++)
      if(s[i] < a[i*n+j]) s[i] = a[i*n+j];
  }
  for (k=0;k<n-1; k++) {  
    for(i=k; i<n; i++) {   
      temp = fabs(a[i*n+k]/s[i]);
      if(i==k) { 
        c=temp; Pivot[k]=i;
      }
      else if(c <temp) {
        c = temp; Pivot[k]=i;
      }
    }  
    /* If all elements of a row(or column) of A are zero, |A| = 0 */
    if(c==0) {   
      det=0;
      return(det);
    }
    if(Pivot[k]!=k) {   
      det*=-1; 
      for(j=k; j<n; j++) {   
        temp = a[k*n+j]; 
        a[k*n+j]=a[Pivot[k]*n+j]; 
        a[Pivot[k]*n+j]=temp;
      }
      temp = s[k];
      s[k] = s[Pivot[k]];   
      s[Pivot[k]]=temp;
    }
    for(i=k+1; i<n; i++) {   
      temp = a[i*n+k]/a[k*n+k];
      a[i*n+k] = temp;
      for(j=k+1; j<n; j++)
      a[i*n+j] -= temp*a[k*n+j];
    }
    det *= a[k*n+k];
  }
  k = n-1;
  det *= a[(n-1)*n+(n-1)];
  ier=0; 

  g_free(s);

  return(det);
}

gdouble
inverse_step(gdouble *a,gdouble *b,gint n,gint *Pivot) 
{
  gint i,j,k;
  gdouble temp;
  
  for(k=0; k<(n-1); k++)
  {  if(Pivot[k] != k)
     {  temp = b[Pivot[k]];
        b[Pivot[k]] = b[k];
        b[k] = temp;
     }
     for(i=(k+1);i<n; i++)
        b[i] -= a[i*n+k]*b[k];
  }
  b[n-1] /= a[n*n-1];
  for(i=(n-2); i>=0; i--)
  {  temp=0;
     for(j=(i+1); j<n; j++)
       temp += a[i*n+j]*b[j];
       b[i] = (b[i] -temp)/a[i*n+i];
  }
  return(0);
}

void inverse(gdouble *a, gint n)
{
  gdouble *b,*inv,d;
  gint *P,i,j;

  P = (gint *) g_malloc(n*sizeof(gint));
  inv = (gdouble *) g_malloc(n*n*sizeof(gdouble));
  d = ludcmp(a,n,P);
 
  b = (gdouble *) g_malloc(n*sizeof(gdouble));
  for(i=0; i<n; i++)
  {  
    for(j=0; j<n; j++)
    {  
      if(i == j) b[j] = 1.0; else b[j] = 0.0;
    }
    d=inverse_step(a,b,n,P);
     for(j=0; j<n; j++)
       inv[j*n+i] = b[j];
  }
  memcpy(a,inv,n*n*sizeof(gdouble));

  g_free(P);
  g_free(inv);
  g_free(b);
}    

void arrayd_inverse(array_d a) {
  guint p = a.ncols;

  if (p == 1) {
    a.vals[0][0] = 1. / MAX(a.vals[0][0], GGOBI_EPSILON);
    return;
  }
  
  gdouble* temp = (gdouble *) g_new0(gdouble, p * p);
  
  // Copy over
  for (guint i = 0; i<p; i++)
    for (guint j = 0; j<p; j++)
      temp[i * p + j] = a.vals[i][j];

  inverse(temp, p);

  // Copy back
  for (guint i = 0; i<p; i++)
    for (guint j = 0; j<p; j++)
      a.vals[i][j] = temp[i*p+j];
      
  g_free(temp);
}

gdouble arrayd_determinant(array_d a) {
  guint p = a.ncols;
  
  if (p == 1) return fabs(a.vals[0][0]);

  gint *pivot = g_new0(gint, p);
  gdouble* temp = g_new0(gdouble, p * p);

  for (guint i=0; i < p; i++)
    for (guint j=0; j < p; j++)
      temp[i*p+j] = a.vals[i][j];

  gdouble det = ludcmp(temp, p, pivot);
  
  g_free(pivot);
  g_free(temp);
  
  return det;
}

                              
void center (array_d data) { 
  gint i, j; 
  gdouble mean; 
  for (i=0; i < data.ncols; i++) 
  { mean = 0.0; 
    for (j=0; j < data.nrows; j++) 
      mean += data.vals[j][i]; 
    mean = mean/data.nrows; 
    for (j=0; j < data.nrows; j++) 
      data.vals[j][i] -= mean; 
  } 
} 

// PCA index
// Computes traces of covariance matrix  
gdouble ppi_pca (array_d data, vector_d groups) { 
  center (data); 
 
  gdouble val = 0.0; 
  for (guint i = 0; i < data.ncols; i++) {
    for (guint j = 0; j < data.nrows; j++) {
      val += data.vals[j][i] * data.vals[j][i]; 
    }
  } 
  return val / (data.nrows - 1); 
}

// Gaussian filter (?) used as basis for hole and central mass indices
gdouble gaussian_filter(array_d data) { 
  int i, p, n, k,j;
  gdouble tmp,x1,x2;

  p = data.ncols; 
  n = data.nrows;
  
  // Compute column means
  vector_d means;
  vectord_alloc_zero(&means, p);

  for(j = 0; j < p; j++) {
    for(i = 0; i < n; i++) means.els[j] += data.vals[i][j];
    means.els[j] /= n;
  }

  // Compute covariance matrix
  array_d cov;
  arrayd_alloc_zero(&cov, p, p);

  for (j=0; j<p; j++) { 
    for (k=0; k<=j; k++) { 
      cov.vals[k][j] = 0.0;
      for (i=0; i<n; i++) 
        cov.vals[k][j] += 
         (data.vals[i][j] - means.els[j]) *
         (data.vals[i][k] - means.els[k]);
      cov.vals[k][j] /= ((double)(n-1)); 
      if (j != k)
        cov.vals[j][k] = cov.vals[k][j];
      }
  }

  arrayd_inverse(cov);

  gdouble acoefs = 0.0;
  for (i=0; i<n; i++) { 
    tmp = 0.0;  
    for (j=0; j<p; j++) {   
      x1 = data.vals[i][j] - means.els[j];
      for (k=0; k<p; k++) {  
        x2 = data.vals[i][k] - means.els[k]; 
        tmp += x1 * x2 * cov.vals[j][k];
      }
    } 
    acoefs += exp(-tmp/2.0);
  }

  arrayd_free(&cov);
  vectord_free(&means);
  
  return acoefs;
}


// Holes index
// Looks for the projection with no data in center.
gdouble ppi_holes(array_d data, vector_d groups) { 
  guint p = data.ncols; 
  guint n = data.nrows;
  
  gdouble acoefs = gaussian_filter(data);
  return (1.0 - acoefs / (gdouble) n)/ (1.0 - exp(-p / 2.0));
}

// Central mass index
// Looks for the projection with lots of data in the center.
gdouble ppi_central_mass(array_d data, vector_d groups) { 
  guint p = data.ncols; 
  guint n = data.nrows;

  gdouble acoefs = gaussian_filter(data);
  return (acoefs / n - exp(-p / 2.0))/ (1.0 - exp(-p / 2.0));
}

/********************************************************************

Index          : Discriminant
Transformation : -
Purpose        : Looks for the best projection to discriminate
                 between groups.
*********************************************************************/


gdouble ppi_lda (array_d data, vector_d groups) { 
  gint i, j, k;

  guint n = data.nrows;
  guint p = data.ncols;

  vector_i group_lookup, ngroup;
  vectori_alloc_zero(&group_lookup, n);
  vectori_alloc_zero(&ngroup, n);
  guint numgroups = compute_groups(group_lookup, ngroup, groups);

  array_d group_means, covW, covWB;
  arrayd_alloc_zero(&group_means, p, numgroups);
  arrayd_alloc_zero(&covW, p, p);
  arrayd_alloc_zero(&covWB, p, p);

  vector_d means;
  vectord_alloc_zero(&means, p);

  /* Compute means */
  for (k=0; k < p; k++) {
    for (i=0; i < n; i++) { 
      group_means.vals[group_lookup.els[i]][k] += data.vals[i][k]; 
      means.els[k] += (gdouble) data.vals[i][k];
    }
  }

  for (k=0; k < p; k++) { 
    for (i=0; i < numgroups; i++) { 
      group_means.vals[i][k] /= ngroup.els[i];
    }
    means.els[k] /= n;
  }
  
  // Compute W
  for (i=0; i < n; i++) { 
    for (j=0; j < p; j++) { 
      for (k=0; k <= j; k++) { 
        covW.vals[k][j] += 
          (data.vals[i][j] - group_means.vals[group_lookup.els[i]][j])*
          (data.vals[i][k] - group_means.vals[group_lookup.els[i]][k]);
        covW.vals[j][k] = covW.vals[k][j];
      }
    }
  }

  // Compute W + B
  for (i=0; i < n; i++) { 
    for (j=0; j < p; j++) { 
      for (k=0; k <= j; k++) { 
        covWB.vals[k][j] += 
          (data.vals[i][j] - means.els[j])*
          (data.vals[i][k] - means.els[k]);
        covWB.vals[j][k] = covWB.vals[k][j];
      }
    }
  }

  return 1.0 - arrayd_determinant(covW) / arrayd_determinant(covWB);
}
