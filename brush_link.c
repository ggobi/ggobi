/* brush_link.c */
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

/*----------------------------------------------------------------------*/
/*                Linking to other datad's by id                        */
/*----------------------------------------------------------------------*/

void
color_link_by_id (gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gint i, id;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- k is the row number in source_d --*/

  if (source_d->rowid.id.nels > 0) {
    id = source_d->rowid.id.els[k];
    if (id < 0)  /*-- this would indicate a bug --*/
      return;

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (d == source_d)
        continue;        /*-- skip the originating datad --*/

      /*-- if this id exists, it is in the range of d's ids ... --*/
      if (d->rowid.id.nels > 0 && d->rowid.idv.nels > id) {
        /*-- i is the row number, irrespective of rows_in_plot --*/
        i = d->rowid.idv.els[id];
        if (i < 0)  /*-- then no cases in d have this id --*/
          continue;

        /*-- if we get here, d has one case with the indicated id --*/
        if (!d->hidden_now.els[i] && d->sampled.els[i]) {
          switch (cpanel->br_mode) {
            case BR_PERSISTENT:
              d->color.els[i] = d->color_now.els[i] = source_d->color.els[k];
            break;
            case BR_TRANSIENT:
              d->color_now.els[i] = source_d->color_now.els[k];
            break;
          }
        }
      }
    }
  }
}

void
glyph_link_by_id (gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gint i, id;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- k is the row number in source_d --*/

  if (source_d->rowid.id.nels > 0) {
    id = source_d->rowid.id.els[k];
    if (id < 0)  /*-- this would indicate a bug --*/
      return;

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (d == source_d)
        continue;        /*-- skip the originating datad --*/
 
      /*-- if this id exists is in the range of d's ids ... --*/
      if (d->rowid.id.nels > 0 && d->rowid.idv.nels > id) {
        /*-- i is the row number, irrespective of rows_in_plot --*/
        i = d->rowid.idv.els[id];
        if (i < 0)  /*-- then no cases in d have this id --*/
          continue;

        /*-- if we get here, d has one case with the indicated id --*/
        if (!d->hidden_now.els[i] && d->sampled.els[i]) {
          switch (cpanel->br_mode) {
            case BR_PERSISTENT:
              d->glyph.els[i].size = d->glyph_now.els[i].size =
                source_d->glyph.els[k].size;
              d->glyph.els[i].type = d->glyph_now.els[i].type =
                source_d->glyph.els[k].type;
            break;
            case BR_TRANSIENT:
              d->glyph_now.els[i].size = source_d->glyph_now.els[k].size;
              d->glyph_now.els[i].type = source_d->glyph_now.els[k].type;
            break;
          }
        }
      }
    }
  }
}

void
hidden_link_by_id (gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gint i, id;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- k is the row number in source_d --*/

  if (source_d->rowid.id.nels > 0) {
    id = source_d->rowid.id.els[k];
    if (id < 0)  /*-- this would indicate a bug --*/
      return;

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (d == source_d)
        continue;        /*-- skip the originating datad --*/
 
      /*-- if this id exists is in the range of d's ids ... --*/
      if (d->rowid.id.nels > 0 && d->rowid.idv.nels > id) {
        /*-- i is the row number, irrespective of rows_in_plot --*/
        i = d->rowid.idv.els[id];
        if (i < 0)  /*-- then no cases in d have this id --*/
          continue;

        /*-- if we get here, d has one case with the indicated id --*/
        /*-- in this routine alone we include the hidden guys --*/
        if (d->sampled.els[i]) {
          switch (cpanel->br_mode) {
            case BR_PERSISTENT:
              d->hidden.els[i] = d->hidden_now.els[i] = source_d->hidden.els[k];
            break;
            case BR_TRANSIENT:
              d->hidden_now.els[i] = source_d->hidden_now.els[k];
            break;
          }
        }
      }
    }
  }
}


/*----------------------------------------------------------------------*/
/*   Linking within and between datad's using a categorical variable    */
/*----------------------------------------------------------------------*/

void
linkvar_arrays_init (gchar *collab, datad *d, ggobid *gg)
{
  gint i, j, k;
  vartabled *vt = vartable_element_get_by_name (collab, d);
  gint link_index = g_slist_index (d->vartable, vt);

  /*-- free any existing linkvar_arrays --*/
  if (gg->linkvar_arrays != NULL) {
    GSList *l;
    for (l=gg->linkvar_arrays; l; l=l->next) {
      g_array_free ((GArray *) l->data, false);
    }
    g_slist_free (gg->linkvar_arrays);
  }

  /* 
   * allocate the new linkvar_arrays and then
   * initialize nlevels GArrays to contain
   * the vector of indices for each level
  */
  gg->linkvar_arrays = g_slist_alloc ();
  for (k=0; k<vt->nlevels; k++)
    gg->linkvar_arrays = g_slist_append (gg->linkvar_arrays,
      g_array_new (false, false, sizeof(gint)));
   
  /*-- populate the GArrays, making one pass through the data --*/
  for (i=0; i<d->nrows; i++) {
    for (j=0; j<vt->nlevels; j++) {
      if (d->raw.vals[i][link_index] == vt->level_values[j]) {
        g_array_append_val ((GArray *) g_slist_nth_data (gg->linkvar_arrays, j),
          i);
        break;
      }
    }
  }
}

