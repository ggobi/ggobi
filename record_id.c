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

void rowids_free (datad *d, ggobid *gg)
{
  vectori_free (&d->rowid.id);
}


void
rowids_alloc (datad *d, ggobid *gg) 
{
  vectori_alloc (&d->rowid.id, d->nrows);
}

