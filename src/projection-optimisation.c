#include "projection-optimisation.h"
#include "utils.h"
#include "vector.h"
#include <math.h>


gint alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols, gint ndim)
{
  arrayd_init_null (&op->proj_best);
  /*  arrayd_alloc_zero (&op->proj_best, ncols, ndim); *nrows, ncols);*/
  arrayd_alloc_zero (&op->proj_best, ndim, ncols); /*nrows, ncols);*/
  arrayd_init_null (&op->data);
  arrayd_alloc_zero (&op->data, nrows, ncols); 
  arrayd_init_null (&op->pdata);
  arrayd_alloc_zero (&op->pdata, nrows, ndim); 

  return 0;
}

gint free_optimize0_p (optimize0_param *op)
{ 
  arrayd_free (&op->proj_best);
  arrayd_free (&op->data);
  arrayd_free(&op->pdata);

  return 0;
}

gint realloc_optimize0_p (optimize0_param *op, gint ncols, vector_i pcols)
{
/* pdata doesn't need to be reallocated, since it doesn't depend on ncols */
  if (op->proj_best.ncols < ncols) {
    arrayd_add_cols(&op->proj_best, ncols);
    arrayd_add_cols(&op->data, ncols);
  }
  else {
    GSList *cols = NULL;
    for (guint i=0; i < op->proj_best.ncols - ncols; i++)
      cols = g_slist_append(cols, GINT_TO_POINTER(ncols - 1 - i));
    arrayd_delete_cols(&op->proj_best, cols);
    arrayd_delete_cols(&op->data, cols);
    g_slist_free (cols);
  }

  return 0;
}

/********************************************************************

                         OPTIMIZATION

The index function has to be defined as

     gint index (array_d *pdata, void *param, gdouble *val)

with   

Input:  pdata   projected data
        param   additional parameters for the index 
                (will not be touched by the optimization routine)

Output: val     the index-value
        
The return value should be zero, otherwise the optimization routine
assumes an error has occurred during computation of the index. 

*********************************************************************/

gboolean iszero (array_d *data)
{ 
  gdouble sum = 0;
  gint i, j;

  for (i=0; i<data->nrows; i++)
  { for (j=0; j<data->ncols; j++)
      sum += fabs(data->vals[i][j]);
  }
  return (sum<1e-6);
}

void normal_fill (array_d *data, gdouble delta, array_d *base)
{ 
  int i, j;
  for (i=0; i<data->nrows; i++)
  { for (j=0; j<data->ncols; j++)
      data->vals[i][j] = base->vals[i][j]+delta*random_normal();
  }
}

void orthonormal (array_d *proj)
{ 
  gint i, j, k;
  gdouble *ip = g_malloc (proj->ncols*sizeof(gdouble));
  gdouble norm;

  /* First norm vector p_i */
  for (i=0; i<proj->nrows; i++)
  { 
    norm = 0.0;
    for (k=0; k<proj->ncols; k++)
      norm += (proj->vals[i][k]*proj->vals[i][k]);
    norm = sqrt(norm);
    for (k=0; k<proj->ncols; k++)
      proj->vals[i][k] /= norm;
  }

  for (i=0; i<proj->nrows; i++)
  { /* Compute inner product between p_i and all p_j */
    for (j=0; j<i; j++)
    { ip[j] = 0;
      for (k=0; k<proj->ncols; k++)
        ip[j] += proj->vals[j][k]*proj->vals[i][k];
    }
    /* Subtract now all vectors from p_i */
    for (j=0; j<i; j++)
    { for (k=0; k<proj->ncols; k++)
        proj->vals[i][k] -= ip[j]*proj->vals[j][k];
    }
    /* Finally norm vector p_i */
    norm = 0.0;
    for (k=0; k<proj->ncols; k++)
      norm += (proj->vals[i][k]*proj->vals[i][k]);
    norm = sqrt(norm);
    for (k=0; k<proj->ncols; k++)
      proj->vals[i][k] /= norm;
  }
  g_free(ip);
}




gint optimize0 (optimize0_param *op,
                PPIndex index,
                void *param)
{ 
  gdouble index_work = 0.0;
/*  array_d proj_work, pdata, *proj;*/
  array_d proj_work, *proj;
  int i,j, m, k;

  proj = &(op->proj_best);
  arrayd_init_null (&proj_work);
  arrayd_alloc_zero (&proj_work, proj->nrows, proj->ncols);
  /*  arrayd_init_null (&pdata);
      arrayd_alloc_zero (&pdata, op->data.nrows, proj->ncols);*/

  /*  op->temp_start     =  1; * make this an interactive parameter */
  op->temp_end       =  0.001;
  /*  op->cooling        =  0.99; * make this an interactive parameter */
  /* is equivalent to log(temp_end/temp_start)/log(cooling) projections */
  op->heating        =  1;
  op->restart        =  1;
  op->success        =  0;
  op->temp           =  op->temp_start;/*0.1; 1.0;*/
  op->maxproj        =  op->restart*(1+log(op->temp_end/op->temp_start)/
    log(op->cooling)); /* :) */

  /*g_printerr("NEW OPTIMIZATION\n");
    g_printerr ("index_work %f index_best %f \n",index_work, op->index_best);*/
  /* This adds random noise to existing projection and orthonormalize, 
     if the current projection is null */
  if (iszero(proj))
  { /* sprintf (msg, "zero projection matrix"); print(); */
    normal_fill (proj, 1.0, proj);
    orthonormal (proj);
  }

  /*g_printerr("nrows %d ncols %d\n",op->data.nrows, op->data.ncols);

g_printerr ("proj: ");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", proj->vals[0][i]);
g_printerr ("\n");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", proj->vals[1][i]);
g_printerr ("\n");*/
  /* calculate projected data */ /* this is generated outside this function */
  /*  for (i=0; i<op->data.nrows; i++)
  { 
    for (j=0; j<proj->nrows; j++)
    { 
      op->pdata.vals[i][j] = 0;
      for (m=0; m<op->data.ncols; m++)
        op->pdata.vals[i][j] += op->data.vals[i][m]*proj->vals[j][m];
    }
    }*/
  /* do index calculation, functions return -1 if a problem, which
     is then passed back through optimize0 to tour1d_run */
  op->index_best = index (&op->pdata, param);

  /*g_printerr ("index_work %f index_best %f \n",index_work, op->index_best);*/
/*  if (index (&op->pdata, param, &index_work)) return(-1);*/

  /* fill proj_work */
  arrayd_copy (proj, &proj_work);

  op->success = k = 0;
  while (op->restart > 0)
  { /* sprintf (msg, "Restart %i", op->restart); print(); */
    while (op->temp > op->temp_end)
    { /* sprintf (msg, "Iteration %i", k); print(); */
      /* add some randomness to current projection */
      normal_fill (&proj_work, op->temp, proj);
      orthonormal (&proj_work);                              
      op->temp *= op->cooling;

      /* calc projected data */
      for (i=0; i<op->data.nrows; i++)
      { 
        for (j=0; j<proj->nrows; j++)
        { 
          op->pdata.vals[i][j] = 0;
          for (m=0; m<op->data.ncols; m++)
            op->pdata.vals[i][j] += op->data.vals[i][m]*proj_work.vals[j][m];
        }
      }

      /* Calculate pp index for current projection */       
      index_work = index (&op->pdata, param);

      /*g_printerr ("index_work %f temp %f \n",index_work, op->temp);
g_printerr ("proj_work: ");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", proj_work.vals[0][i]);
g_printerr ("\n    ");
g_printerr ("index_best %f temp %f \n", op->index_best, op->temp);
g_printerr ("proj_best: ");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", op->proj_best.vals[0][i]);
g_printerr ("\n");
      */
      if (index_work > op->index_best)
      { /* sprintf (msg, "Success %f", index_work); print(); */
        op->success++;
        /*printf ("Success %f\n", index_work); */
        arrayd_copy (&proj_work, proj); /*Sigbert's code, I think 
               this should be saving it into the best proj rather than work*/
        arrayd_copy (&proj_work, &op->proj_best);
        op->index_best = index_work;
        op->temp *= op->heating;
      }
      k++; 
      if (k >= op->maxproj) 
      { 
    /*        printf("A #iter = %d \n",k);
        printf ("Best = %f\n", op->index_best);
        for (i=0; i<proj->nrows; i++)        
    { for (j=0; j<proj->ncols; j++)
          printf ("%+5.3f ", proj->vals[i][j]);
        printf ("\n");
        }
        printf ("\n");*/
        return(k);
      }
    }
    op->temp = op->temp_start;
    op->restart--;
  } 

  /*  printf("B #iter = %d \n",k);
  printf ("Best = %f\n", op->index_best);
  for (i=0; i<proj->nrows; i++)        
  { 
    for (j=0; j<proj->ncols; j++)
      printf ("%+5.3f ", proj->vals[i][j]);
     printf ("\n");
  }
  printf ("\n");
  */
  return (k);
}