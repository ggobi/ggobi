/*-- rowids.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define MAXIDSIZE 104

/*--------------------------------------------------------------------*/
/*                   Memory allocation, initialization                */
/*--------------------------------------------------------------------*/

void rowids_init_null (datad *d)
{
  vectori_init_null (&d->rowid.id);
  vectori_init_null (&d->rowid.idv);
}

void rowids_free (datad *d)
{
  vectori_free (&d->rowid.id);
}

void
rowids_alloc (datad *d) 
{
  vectori_alloc (&d->rowid.id, d->nrows);
}

void
rowidv_init (datad *d) {
  gint i, k;

  if (d->rowid.id.nels > 0) {

    /*
     * assume sorting, use the maximum value of rowid.id.els
     * to dimension rowid.idv
    */
    gint nels = 1 + d->rowid.id.els[ d->rowid.id.nels-1 ]; 

    vectori_alloc (&d->rowid.idv, nels);
    for (i=0; i<nels; i++)
      d->rowid.idv.els[i] = -1;

    /*
     *  example: 
     *   row.id.els = {1,3,5}
     *   row.idv.els = {-1,0,-1,1,-1,2}
    */  

    for (i=0; i<d->nrows; i++) {
      k = d->rowid.id.els[i];
      d->rowid.idv.els[k] = i;
    }
  }
}
