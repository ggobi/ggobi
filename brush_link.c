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
linkvar_arrays_init (vartabled *vt, datad *d, ggobid *gg)
{
  gint i, k;
  gint link_index = g_slist_index (d->vartable, vt);
  GArray *arr;
  GSList *l;

  /*-- free any existing linkvar_arrays --*/
  if (d->linkvar_arrays != NULL) {
    for (l=d->linkvar_arrays; l; l=l->next) {
      g_array_free ((GArray *) l->data, false);
    }
    g_slist_free (d->linkvar_arrays);
    d->linkvar_arrays = NULL;
  }

  /* 
   * initialize nlevels GArrays to contain
   * the vector of indices for each level
  */
  for (k=0; k<vt->nlevels; k++) {
    arr = g_array_new (false, false, sizeof(gint));
    d->linkvar_arrays = g_slist_append (d->linkvar_arrays, arr);
  }

  /*-- populate the GArrays, making one pass through the data --*/
  for (i=0; i<d->nrows; i++) {
    for (k=0; k<vt->nlevels; k++) {
      if (d->raw.vals[i][link_index] == vt->level_values[k]) {
        g_array_append_val (g_slist_nth_data (d->linkvar_arrays, k), i);
        break;
      }
    }
  }
}


void
linking_method_set (displayd *display, datad *d, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  gg->linkby_cv = false;

  if (cpanel->br_linkby == BR_LINKBYVAR) {

    vartabled *vt;
    gint *vars = (gint *) g_malloc (d->ncols * sizeof(gint));
    gint nvars = selected_cols_get (vars, d, gg);

    if (nvars == 1) {
      vt = vartable_element_get (vars[0], d);
      if (vt->categorical_p) {
        gg->linkby_cv = true;
        if (d->linkvar_vt == NULL || d->linkvar_vt != vt) {
          d->linkvar_vt = vt;
          linkvar_arrays_init (vt, d, gg);
        }
      }
    }
  }
}

void
brush_link_by_var (gint jlinkby, gboolean *levels, cpaneld *cpanel,
  datad *d, ggobid *gg)
{
  gint m, i, level;

  /*
   * for this datad, loop once over all rows in plot 
  */
  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    level = (gint)d->raw.vals[i][jlinkby];

    if (levels[level]) {  /*-- if it's to acquire the new symbol --*/
      if (cpanel->br_mode == BR_PERSISTENT) {
        switch (cpanel->br_point_targets) {
          case BR_CANDG:  /*-- color and glyph, type and size --*/
            d->color.els[i] = d->color_now.els[i] = gg->color_id;
            d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
            d->glyph.els[i].type = d->glyph_now.els[i].type =
              gg->glyph_id.type;
          break;
          case BR_COLOR:  /*-- color only --*/
            d->color.els[i] = d->color_now.els[i] = gg->color_id;
          break;
          case BR_GLYPH:  /*-- glyph type and size --*/
            d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
            d->glyph.els[i].type = d->glyph_now.els[i].type =
              gg->glyph_id.type;
          break;
          case BR_GSIZE:  /*-- glyph size only --*/
            d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
          break;
          case BR_HIDE:  /*-- hidden --*/
            d->hidden.els[i] = d->hidden_now.els[i] = true;
          break;
          default:
          break;
        }

      } else if (cpanel->br_mode == BR_TRANSIENT) {
        switch (cpanel->br_point_targets) {
          case BR_CANDG:
            d->color_now.els[i] = gg->color_id;
            d->glyph_now.els[i].size = gg->glyph_id.size;
            d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
          case BR_COLOR:
            d->color_now.els[i] = gg->color_id;
          break;
          case BR_GLYPH:  /*-- glyph type and size --*/
            d->glyph_now.els[i].size = gg->glyph_id.size;
            d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
          case BR_GSIZE:  /*-- glyph size only --*/
            d->glyph_now.els[i].size = gg->glyph_id.size;
          break;
          case BR_HIDE:  /*-- hidden --*/
            d->hidden_now.els[i] = true;
          break;
          default:
          break;
        }

      }

    } else {  /*-- if it's to revert to the previous symbol --*/
      /*-- should only matter if transient, right? --*/
      switch (cpanel->br_point_targets) {
        case BR_CANDG:
          d->color_now.els[i] = d->color.els[i];
          d->glyph_now.els[i].size = d->glyph.els[i].size;
          d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
        case BR_COLOR:
          d->color_now.els[i] = d->color.els[i];
        break;
        case BR_GLYPH:  /*-- glyph type and size --*/
          d->glyph_now.els[i].size = d->glyph.els[i].size;
          d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
        case BR_GSIZE:  /*-- glyph size only --*/
          d->glyph_now.els[i].size = d->glyph.els[i].size;
        break;
        case BR_HIDE:  /*-- hidden --*/
          d->hidden_now.els[i] = d->hidden.els[i];
        break;
        default:
        break;
      }
    }
  }
}
