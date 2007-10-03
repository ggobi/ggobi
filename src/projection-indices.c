#include "projection-indices.h"
#include <math.h>
#include <string.h>


/*****************************************************/
/*               Utility Routines                    */
/*                                                   */
/* Reference : An Introduction to Numerical Analysis */
/*             - Kendall E. Atkinson                 */
/*             (p 449 - 450)                         */
/*****************************************************/

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
    d=tour_pp_solve(a,b,n,P);
     for(j=0; j<n; j++)
       inv[j*n+i] = b[j];
  }
  memcpy(a,inv,n*n*sizeof(gdouble));

  g_free(P);
  g_free(inv);
  g_free(b);
}    

gdouble
tour_pp_solve(gdouble *a,gdouble *b,gint n,gint *Pivot) 
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
/******************************************************************** 

Index          : PCA-d 
Transformation : - 
Purpose        : computes the trace of the cov matrix of pdata 
Note           : Modifies pdata ! 
 
*********************************************************************/ 
 
void center (array_d *data) { 
  gint i, j; 
  gdouble mean; 
  for (i=0; i<data->ncols; i++) 
  { mean = 0.0; 
    for (j=0; j<data->nrows; j++) 
      mean += data->vals[j][i]; 
    mean = mean/data->nrows; 
    for (j=0; j<data->nrows; j++) 
      data->vals[j][i] -= mean; 
  } 
} 
 
gint pca (array_d *pdata, void *param, gdouble *val, gpointer userData) { 
  gint i, j; 
 
  center (pdata); 
 
  *val = 0.0; 
  for (i=0; i<pdata->ncols; i++) 
  { for (j=0; j<pdata->nrows; j++) 
      *val += pdata->vals[j][i]*pdata->vals[j][i]; 
  } 
  *val /= (pdata->nrows-1); 
 
  return (0); 
}
/********************************************************************
             Arbitrary dimensional pp indices
*********************************************************************/

gint alloc_pp(pp_param *pp, gint nrows, gint ncols, gint ndim)
{
  gint nr = nrows;/* number of cases */
  gint nc = MAX(ncols,2); /* ncols = number of active vars */
  gint nd = MAX(ndim,2); /* projection dimension */
  gint ncolors = 50; /* This is a guess at the max num of colors -
                        it really shouldn't be hard-coded */

  vectori_alloc_zero(&pp->group, nr);
  vectori_alloc_zero(&pp->ngroup, nr);

  arrayd_alloc_zero(&pp->cov, nd, nd);
  arrayd_alloc_zero(&pp->tcov, nd, nd); /* temporary usage in lda */
  arrayd_alloc_zero(&pp->mean, ncolors, nd); /* means for each group */
  vectord_alloc_zero(&pp->ovmean, nc); /* mean for each projection */

  vectori_alloc_zero(&pp->index, nr);/* used in gini/entropy */
  vectori_alloc_zero(&pp->nright, nr);/* used in gini/entropy */
  vectord_alloc_zero(&pp->x, nr); /* used in gini/entropy */

  return 0;
}

gint free_pp (pp_param *pp)
{
  vectori_free(&pp->group);
  vectori_free(&pp->ngroup);

  arrayd_free (&pp->cov); 
  arrayd_free (&pp->tcov); 
  arrayd_free (&pp->mean); 
  vectord_free(&pp->ovmean);

  vectori_free(&pp->index);
  vectori_free(&pp->nright);
  vectord_free(&pp->x);

  return 0;
}

/********************************************************************

Index          : Holes
Transformation : -
Purpose        : Looks for the projection with no data in center.
*********************************************************************/

gint holes_raw(array_d *pdata, void *param, gdouble *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  int i, p, n, k,j;
  gdouble tmp,x1,x2;
  gdouble *cov;
  gdouble acoefs;
  gdouble tol = 0.0001;

  p = pdata->ncols; 
  n = pdata->nrows;
  cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
  zero(cov,p*p);

  for(j=0; j<p; j++) {
    pp->ovmean.els[j] = 0.0;
    for(i=0; i<n; i++) 
      pp->ovmean.els[j] += pdata->vals[i][j];
    pp->ovmean.els[j] /= ((gdouble)n);
  }

  for (j=0; j<p; j++) { 
    for (k=0; k<=j; k++) { 
      pp->cov.vals[k][j] = 0.0;
      for (i=0; i<n; i++) 
        pp->cov.vals[k][j] += (((pdata->vals[i][j])-pp->ovmean.els[j])*
         ((pdata->vals[i][k])-(pp->ovmean.els[k])));
      pp->cov.vals[k][j] /= ((double)(n-1)); 
      if (j != k)
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
  }

  /*  g_printerr("cov %f %f %f %f\n",pp->cov.vals[0][0],
      pp->cov.vals[0][1],pp->cov.vals[1][0],pp->cov.vals[1][1]);*/
  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    inverse(cov, p);
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else {
    if (pp->cov.vals[0][0] > tol)
      pp->cov.vals[0][0] = 1./pp->cov.vals[0][0];
    else
      pp->cov.vals[0][0] = 10000.0;
  }

  acoefs = 0.0;
  for (i=0; i<n; i++) { 
    tmp = 0.0;  
    for (j=0; j<p; j++) {   
      x1 = pdata->vals[i][j]-pp->ovmean.els[j];
      for (k=0; k<p; k++) {  
        x2 = pdata->vals[i][k]-pp->ovmean.els[k]; 
        tmp += (x1*x2*pp->cov.vals[j][k]);
      }
    } 
    acoefs += exp(-tmp/2.0);
  }
  *val = (1.0-acoefs/(gdouble)n)/(gdouble) (1.0-exp(-p/2.0));

  g_free(cov);

  return(0);
}

/********************************************************************

Index          : Central Mass
Transformation : -
Purpose        : Looks for the projection with lots of data in center.
*********************************************************************/

gint central_mass_raw(array_d *pdata, void *param, gdouble *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  int i, p, n,k,j;
  gdouble tmp,x1,x2;
  gdouble *cov;
  gdouble acoefs;
  gdouble tol = 0.0001;

  p = pdata->ncols; 
  n = pdata->nrows;
  cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
  zero(cov,p*p);

  for(j=0; j<p; j++) {
    pp->ovmean.els[j] = 0.0;
    for(i=0; i<n; i++) 
      pp->ovmean.els[j] += pdata->vals[i][j];
    pp->ovmean.els[j] /= ((gdouble)n);
  }

  for (j=0; j<p; j++) { 
    for (k=0; k<=j; k++) { 
      pp->cov.vals[k][j] = 0.0;
      for (i=0; i<n; i++) 
        pp->cov.vals[k][j] += (((pdata->vals[i][j])-pp->ovmean.els[j])*
         ((pdata->vals[i][k])-(pp->ovmean.els[k])));
      pp->cov.vals[k][j] /= ((double)(n-1)); 
      if (j != k)
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    inverse(cov, p);
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else {
    if (pp->cov.vals[0][0] > tol)
      pp->cov.vals[0][0] = 1./pp->cov.vals[0][0];
    else
      pp->cov.vals[0][0] = 10000.0;
  }

  acoefs = 0.0;
  for (i=0; i<n; i++) { 
    tmp = 0.0;  
    for (j=0; j<p; j++) {   
      x1 = pdata->vals[i][j]-pp->ovmean.els[j];
      for (k=0; k<p; k++) {  
        x2 = pdata->vals[i][k]-pp->ovmean.els[k]; 
        tmp += (x1*x2*pp->cov.vals[j][k]);
      }
    } 
    acoefs += exp(-tmp/2.0);
  }
  *val = (acoefs/n-exp(-p/2.0))/((gdouble) (1-exp(-p/2.0)));

  g_free(cov);
  return(0);
}

/********************************************************************

Index          : Discriminant
Transformation : -
Purpose        : Looks for the best projection to discriminate
                 between groups.
*********************************************************************/

void zero (gdouble *ptr, gint length)
{ 
  gint i;

  for (i=0; i<length; i++)
    ptr[i] = 0.0;
}

void zero_int(gint *mem, int size)
{
  gint i;
  for(i=0; i<size; i++)
  mem[i] = 0;
}
  
gint compute_groups (vector_i group, vector_i ngroup, gint *numgroups, 
  gint nrows, gdouble *gdata)
{ 
  gint i, j, *groupval;

  /* initialize data */
  groupval = g_malloc (nrows*sizeof(gint));

  *numgroups = 0;
  for (i=0; i<nrows; i++)
  { for (j=0; j<*numgroups; j++)
    { if (groupval[j]==gdata[i])
      { ngroup.els[j]++;
        break;
      }
    }
    if (j==*numgroups )
    { groupval[j]  = gdata[i];
      ngroup.els[j] = 1;
      (*numgroups)++;
    }
  }

  for (i=0; i<nrows; i++)
  { for (j=0; j<*numgroups; j++)
    { if (groupval[j]==gdata[i])
        group.els[i] = j;
    }
  }

  g_free(groupval);

  return ((*numgroups==1) || (*numgroups==nrows));
}

gint discriminant (array_d *pdata, void *param, gdouble *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  gint i, j, k, l;
  gint n, p;
  gdouble det;
  gint *Pv; /* dummy structure for pivot in ludcmp - not used */
  gdouble *cov; /* need to get rid of this variable */

  n = pdata->nrows;
  p = pdata->ncols;

  Pv = (gint *) g_malloc(p*sizeof(gint));
  cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));

  /* Compute means */
  for (k=0; k<p; k++) {
    for (l=0; l<pp->numgroups; l++)
      pp->mean.vals[l][k] = 0.0;
    pp->ovmean.els[k] = 0.0;
  }
  for (k=0; k<p; k++) {
    for (i=0; i<n; i++)
    { 
      pp->mean.vals[pp->group.els[i]][k] += (gdouble) pdata->vals[i][k]; 
      pp->ovmean.els[k] += (gdouble) pdata->vals[i][k];
    }
  }

  for (k=0; k<p; k++)
  { 
    for (i=0; i<pp->numgroups; i++)
    { 
      pp->mean.vals[i][k] /= (gdouble) pp->ngroup.els[i];
    }
    pp->ovmean.els[k] /= (gdouble) n;
  }

  /* Compute W */
  for (j=0; j<p; j++)
    for (k=0; k<p; k++)
      pp->cov.vals[j][k] = 0.0;
  for (i=0; i<n; i++)
  { 
    for (j=0; j<p; j++)
    { 
      for (k=0; k<=j; k++)
      { 
        pp->cov.vals[k][j] += 
          ((gdouble) pdata->vals[i][j]-pp->mean.vals[pp->group.els[i]][j])*
          ((gdouble) pdata->vals[i][k]-pp->mean.vals[pp->group.els[i]][k]);
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
    }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    det = ludcmp(cov, p, Pv); 
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else
    det = fabs((gdouble) pp->cov.vals[0][0]);
    *val = det;

  /* Compute B */
  /*  for (j=0; j<p; j++)
    for (k=0; k<p; k++)
      pp->cov.vals[j][k] = 0.0;
  for (j=0; j<p; j++) 
  {	
    for(k=0; k<p; k++)
    {
      for (i=0; i< pp->numgroups; i++)	
        pp->cov.vals[j][k] += (pp->mean.vals[i][j]-pp->ovmean.els[j])*
          (pp->mean.vals[i][k]-pp->ovmean.els[k])*(gdouble)(pp->ngroup.els[i]);
    }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    det = ludcmp(cov, p, Pv); 
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else
    det = fabs((gdouble) pp->cov.vals[0][0]);
    *val = det;*/

  /* Compute W+B */
  for (j=0; j<p; j++)
    for (k=0; k<p; k++)
      pp->cov.vals[j][k] = 0.0;
  for (i=0; i<n; i++)
  { 
    for (j=0; j<p; j++)
    { 
      for (k=0; k<=j; k++)
      { 
        pp->cov.vals[k][j] += 
          ((gdouble) pdata->vals[i][j]-pp->ovmean.els[j])*
          ((gdouble) pdata->vals[i][k]-pp->ovmean.els[k]);
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
    }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    det = ludcmp(cov, p, Pv); 
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else
    det = fabs((gdouble) pp->cov.vals[0][0]);

  *val = 1.0-*val/det; /*1-W/(W+B)*/
/*  *val = *val/det; B/(W+B) */

  g_free(Pv);
  g_free(cov);

  return (0);
}

/********************************************************************

Index          : Gini, Entropy, Variance
Transformation : -
Purpose        : Looks for the best split in 1d-projected data.

*********************************************************************/

void swap_group(array_d *pdata, gint *group, int i, int j)
{
  int temp1,k; 
  double temp2;

  temp1 = group[i];
  group[i] = group[j];
  group[j] = temp1;
  for(k=0; k<pdata->ncols; k++)
  { temp2 = pdata->vals[i][k];   
        pdata->vals[i][k] = pdata->vals[j][k];
        pdata->vals[j][k] = temp2;
  }

}

void sort_group(array_d *pdata, gint *group, int left, int right)
{
  int i, last;
                                
  if(left >= right) return;
  swap_group(pdata, group, left, (left+right)/2);
  last = left;   
  for(i=left+1; i<=right; i++)
    if(group[i] < group[left])
      swap_group(pdata, group, ++last,i);
  swap_group(pdata, group, left, last);
  sort_group(pdata, group, left, last-1);
  sort_group(pdata, group, last+1,right);
}       

void swap_data(double *x, int *index,int i, int j)
{
  int temp1; double temp2;
  
  temp1 = index[i];
  index[i] = index[j];
  index[j] = temp1;
  temp2 = x[i];
  x[i]= x[j];
  x[j] = temp2;
}

void sort_data(double *x, int *index,int left, int right)
{
  int i, last;
                                
  if(left >= right) return;
  swap_data(x,index,left,(left+right)/2);
  last = left;   
  for(i=left+1; i<=right; i++)
    if(x[i] < x[left])
      swap_data(x,index,++last,i);
  swap_data(x,index, left, last);
  sort_data(x,index, left, last-1);
  sort_data(x,index,last+1,right);
}       

void countgroup(int *group, int *gps, int n)
{
  int temp,i;
  int groups = *gps;

  temp = group[0]; 
  groups=1; 

  for(i=1; i<n; i++) 
    if (group[i] != temp) 
      (groups)++; 
  temp = group[i];

  *gps = groups;
}

void countngroup(int *group, int *ngroup, int n)
{
  int temp,i,j;

  temp= group[0]; 
  ngroup[0] = 1; 
  j=0; 
  for(i=1; i<n; i++) 
  {	
    if (group[i] != temp) 
      temp = group[i]; j++;
    (ngroup[j]) ++; 
  } 

}

gint cartgini (array_d *pdata, void *param, gdouble *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  gint i, k, n, p, g = pp->numgroups, left, right, l;
  gdouble dev, prob, maxindex = 0, index;

  n = pdata->nrows;
  p = pdata->ncols;

/* Sort pdata by group */ 
  right = pdata->nrows-1;
  left = 0;
  zero_int(pp->index.els,n);
  for (i=0; i<n; i++)
    pp->index.els[i] = pp->group.els[i];
  sort_group(pdata,pp->index.els,left,right);

/* data relocation and make index */ 
  arrayd_zero(&pp->x);

/* Calculate Gini index in each coordinate 
             and find minimum              */

  for (l=0; l<p; l++)
  {
    for (i=0; i<n; i++) { 
      pp->x.els[i] = pdata->vals[i][l];
      pp->index.els[i] = pp->group.els[i];
    }

    left=0;
    right=n-1;
    sort_data(pp->x.els, pp->index.els, left, right) ;

 /* Calculate gini index */
    zero_int(pp->nright.els,g);
    index = 1;
    for (i=0; i<g; i++) { 
      pp->nright.els[i] = 0;
      index -= (((gdouble)pp->ngroup.els[i])/((gdouble)n))*
        (((gdouble)pp->ngroup.els[i])/((gdouble)n));
    }
    for (i=0; i<n-1; i++)  {
      (pp->nright.els[pp->index.els[i]])++;
      dev=1;
      for (k=0; k<g; k++) {
        prob = ((gdouble) pp->nright.els[k])/((gdouble)(i+1));
        dev -= prob*prob*((gdouble)(i+1)/(gdouble)n);
        prob = ((gdouble) (pp->ngroup.els[k]-pp->nright.els[k]))/
          ((gdouble)(n-i-1));
        dev -= prob*prob*((gdouble)(n-i-1)/(gdouble)n);
      }
      if (dev<index) index = dev;
    }
    if(l==0) maxindex = index; /* index is between 0 and 1 - need max */
    else {
      if(maxindex < index) maxindex = index;
    }
  }
  *val = 1-maxindex ;
  return(0);
}

gint cartentropy (array_d *pdata, void *param, gdouble *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  gint i, k, n, p, g = pp->numgroups, left, right,l;
  gdouble dev, prob, maxindex = 0, index;

  n = pdata->nrows;
  p = pdata->ncols;

/* Sort pdata by group */ 
  right = pdata->nrows-1;
  left = 0;
  zero_int(pp->index.els,n);
  for (i=0; i<n; i++)
    pp->index.els[i] = pp->group.els[i];
  sort_group(pdata,pp->index.els,left,right);

/* data relocation and make index */ 
  arrayd_zero(pp->x);

/* Calculate index in each coordinate and find minimum  */
  for(l=0; l<p; l++)
  {
    for (i=0; i<n; i++) { 
      pp->x.els[i] = pdata->vals[i][l];
      pp->index.els[i] = pp->group.els[i];
    }

    left=0;
    right=n-1;
    sort_data(pp->x.els, pp->index.els,left,right) ;

 /* Calculate index */
    zero_int(pp->nright.els,g);
    index = 0;
    for (i=0; i<g; i++) { 
      pp->nright.els[i] = 0;
      index -= (((gdouble)pp->ngroup.els[i])/((gdouble)n))*
        log(((gdouble)pp->ngroup.els[i])/((gdouble)n));
    }
    for (i=0; i<n-1; i++)  {
      (pp->nright.els[pp->index.els[i]])++;
      dev=0;
      for (k=0; k<g; k++) {
        prob = ((double) pp->nright.els[k])/((double)(i+1));
        if (prob > 0)
          dev -= prob*log(prob)*((gdouble)(i+1)/(gdouble)n);
        prob = ((double) (pp->ngroup.els[k]-pp->nright.els[k]))/
          ((double)(n-i-1));
        if (prob > 0)
          dev -= prob*log(prob)*((gdouble)(n-i-1)/(gdouble)n);
      }
      if (dev<index) index = dev;
    }
    if(l==0) maxindex=index;
    else {
      if(maxindex < index) maxindex = index;
    }
  } 
  *val = 1-maxindex/log(g) ;
  return(0);

}