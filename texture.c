/* texture.c */
/*-- generates the data for a textured dotplot --*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <gtk/gtk.h>

extern gdouble randvalue (void);

/*
 * The code in this file is taken from code written by Paul Tukey and
 * John Tukey, as described in their paper entitled
 * "Strips Displaying Empirical Distributions:  I.  Textured Dot
 * Strips."   I've converted it to C from ratfor and modfied it so
 * that it returns the texture axis in the order of the original
 * data.
*/

gint myrnd (gint);
gfloat *gy;

gint
psort (const void *arg1, const void *arg2)
{
  gint val = 0;
  gint *x1 = (gint *) arg1;
  gint *x2 = (gint *) arg2;

  if (gy[*x1] < gy[*x2])
    val = -1;
  else if (gy[*x1] > gy[*x2])
    val = 1;

  return (val);
}

void
next5 (gint *xlast, gint *perm)
{
/*
 * Extend a 5-string by 5 more numbers:
 * Given a last perm of length 5, choose a next perm, subject to restrictions
 *
 * perms is the list of all 32 possible permutations of (0,1,2,3,4)
 *   with no runs up or down of length 3
 * Note: for every perm in the list, (4 - perm) is also in the list,
 *   symmetrically placed.
*/
  gint i, j, last[5];
  gint nperms = 32;
  static gint cumcnt[5] = {4,11,19,26,31};
  static gint perms[32][5] = {
    {0,2,1,4,3},  {0,3,1,4,2},  {0,3,2,4,1},  {0,4,1,3,2},
    {0,4,2,3,1},  {1,0,3,2,4},  {1,0,4,2,3},  {1,2,0,4,3},
    {1,3,0,4,2},  {1,3,2,4,0},  {1,4,0,3,2},  {1,4,2,3,0},
    {2,0,3,1,4},  {2,0,4,1,3},  {2,1,3,0,4},  {2,1,4,0,3},
    {2,3,0,4,1},  {2,3,1,4,0},  {2,4,0,3,1},  {2,4,1,3,0},
    {3,0,2,1,4},  {3,0,4,1,2},  {3,1,2,0,4},  {3,1,4,0,2},
    {3,2,4,0,1},  {3,4,0,2,1},  {3,4,1,2,0},  {4,0,2,1,3},
    {4,0,3,1,2},  {4,1,2,0,3},  {4,1,3,0,2},  {4,2,3,0,1}
  };

  for (i=0; i<5; i++)
    last[i] = xlast[i];

  if (last[0]==0 && last[1]==0)
  {
  /*
   * Initialize a new perm by choosing a perm at random
  */
    j = myrnd(nperms) - 1;
    for (i=0; i<5; i++)
      last[i] = perms[j][i] ;
  }
/*
 *  Randomly choose a permutation perm(1-5) from among those
 *  that do not start with the previous digit and that make a
 *  transition which reverses the previous direction.
*/

  if (last[3] < last[4])
  {
    j = myrnd (cumcnt[last[4]]) - 1;
    for (i=0; i<5; i++)
      perm[i] = perms[j][i];
  }
  else
  {
    j = myrnd (cumcnt[3-last[4]]) - 1;
    for (i=0; i<5; i++)
      perm[i] = 4 - perms[j][i];
  }
  return;
}


void
next25 (gint *tt, gint *bigt, gint *smallt)
{
/*
 *  Calculate the next 25 values of a 2nd-stage shift vector
 *   by interleaving five 5-strings
*/
  gint i, j, k;

  if (bigt[0]==0 && bigt[1]==0)
  {
  /*
   * Force initialization
  */
    bigt[20] = 0;
    bigt[21] = 0;
    for (i=0; i<25; i++)
      smallt[i] = 0;
  }

  /*
   * Get next 25 elements of bigt, 5 at a time
  */
  next5 (&bigt[20], bigt);
  for (j=5; j<21; j=j+5)
    next5 (&bigt[j-5], &bigt[j]);

  /*
   * Extend each of the smallt series by 5
  */
  for (j=0; j<21; j=j+5)
    next5 (&smallt[j], &smallt[j]);

  /*
   * Interleave the smallt series according to bigt
  */
  for (i=0; i<5; i++)
    for (j=0; j<5; j++)
    {
      k   = 5*i + j;
      tt[k] = smallt[i + 5*bigt[k]];
    }

  return;
}

void
textur (gfloat *yy, gfloat *shft, gint ny, gint option, gfloat del, gint stages)
{
/*
 * Calculate a texturing shft vector based on data yy
 * Note: data vector yy is returned sorted
 *
 * Return shft resorted into the original order of yy.
 * Use the default values for these arguments, as follow:
 * option=1, del=1.0, stages=3
 *
 * A bit of work is needed if we want to use option=2.
*/
  gfloat lohnge, hihnge, delta;
  gfloat srnge, slo, shi;
  gint nny, window, mid, g, h, gg, hh;
  gint tmp5x5[25];
  gint tlarge[25], tsmall[25];
  gint i, ii;
  gint *indx;
  gfloat *xx;
  extern gint fcompare (const void *, const void *);

/*
 * Force initialization on first calls to next5.
*/
  for (i=0; i<2; i++)
    tlarge[i] = tsmall[i] = 0;

  indx = (gint *) g_malloc (ny * sizeof (gint));
/*
 * gy is needed solely for the psort routine:  psort is used by
 * qsort to put an index vector in the order that yy will assume.
*/
  gy = (gfloat *) g_malloc (ny * sizeof (gfloat));
  xx = (gfloat *) g_malloc (ny * sizeof (gfloat));

  for (i=0; i<ny; i++)
  {
    indx[i] = i;
    gy[i] = yy[i];
  }

  qsort ((void *) indx, (size_t) ny, sizeof (gint), psort);
  qsort ((void *) yy, (size_t) ny, sizeof (gfloat), fcompare);

/*
 * Bug here:  this is screwy if ny < 4.
*/
  lohnge = yy[ny/4 - 1];
  hihnge = yy[ny - ny/4 - 1];
  delta  = del * .03 * (hihnge-lohnge);

/*
 *  Do the first two stages of shift, based on 5-strings
*/
  nny = ny;
/*
 *  if( option == 2 )
 *    nny = MIN(nny, 50);
*/

  for (i=0; i<nny; i++) {
    ii = (i % 25);
    if (ii==0)
      next25 (tsmall, tlarge, tmp5x5);

    if (stages >= 2)
      shft[i] = (gfloat) (20*tlarge[ii] + 4*tsmall[ii]) + 2;
    else
      shft[i] = (gfloat) (20*tlarge[ii]) + 2;

  /*
   *  Note: we use the same tlarge 5-string both for gross shift
   *  and to interleave the 2nd-stage 5-strings.
  */
  }

  if (stages<=1) {
    g_free ((gpointer) indx);
    g_free ((gpointer) gy);
    g_free ((gpointer) xx);
    return;
  }

/*
 * Optionally, add a tiny bit of uniform jitter on the smallest scale
*/
  if (option == 1) {
    for (i=0; i<ny; i++) {
      shft[i] = shft[i] + ((gfloat) randvalue()) * 4 - 2;
    }
  }

/*
 * Optionally, repeat first block of 50, shifting the first 25 by a bit
 *  else if (option == 2)
 *  {
 *    for (i=0; i<25 && i<ny; i++)
 *      shft[i] = shft[i] - 2;
 *    for(i=50; i<ny; i++)
 *      shft[i] = shft[i-50];
 *  }
*/

/*
 * Now look at y values, and stretch bunched-up sections out to the edges
*/
  window = 5;
  mid  = (window+1)/2;
  h    = mid;
  while (h-mid+window < ny)
  {
     /*for(g = h; g-mid+window < ny; g = g+window)*/ /* bug */
     for (g = h; g-mid+window < ny && g+window < ny; g = g+window)
       if (yy[g+window] > yy[h] + 10.*delta)
  	    break;
    if (g-mid+window >= ny)
      break;
    hh = h - mid;
    gg = g - h + window;
    slo = 5;
    shi = 0;
    for (i=hh; i<hh+gg; i++)
    {
      if (shft[i]<slo)
        slo = shft[i];
      if (shft[i]>shi)
        shi = shft[i];
    }
    srnge = shi - slo;
    for (i=hh; i<gg; i++)
      shft[i] = 100 * (shft[i] - slo) / srnge;
    h = g + window;
  }


/*
 * Again looking at y values, pull points back to center thread,
 * or to the 30% and 70% positions, in sparse regions
*/
  for (i=1; i<ny-1; i++) {
    if (( yy[i] - yy[i-1] > delta) && (yy[i+1] - yy[i] > delta)) {
  	  shft[i] = 50;
  	}
  }

  for (i=1; i<ny-2; i++) {
    if ((yy[i] - yy[i-1] > delta) &&
        (yy[i+2] - yy[i+1] > delta) &&
        (yy[i+1] - yy[i] < delta))
    {
  	  shft[i]   = 30;
  	  shft[i+1] = 70;
  	}
  }


  if ( yy[1] - yy[0] > delta )
    shft[0]  = 50;
  if ( yy[ny-1] - yy[ny-2] > delta )
    shft[ny-1] = 50;

  if ((yy[2] - yy[1] > delta) && (yy[1] - yy[0] < delta)) {
    shft[0] = 30;
    shft[1] = 70;
  }
  if ((yy[ny-1] - yy[ny-2] < delta) && (yy[ny-2] - yy[ny-3] > delta)) {
    shft[ny-2] = 30;
    shft[ny-1] = 70;
  }

  for (i=0; i<ny; i++)
    xx[indx[i]] = shft[i];

  for (i=0; i<ny; i++)
    shft[i] = xx[i];

  g_free ((gpointer) indx);
  g_free ((gpointer) gy);
  g_free ((gpointer) xx);

  return;
}

gint
myrnd (gint n)
{
/*
 * Select a random integer between 1 and n
*/
  gint nn, myrndval;
  gfloat rrand;
  nn = MAX(n,1);

  rrand = (gfloat) randvalue();
  myrndval = MIN (nn, (gint) (rrand * (gfloat) nn) + 1);
  return (myrndval);
}
