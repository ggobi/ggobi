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

void symbol_link_by_id(gint k, datad * sd, ggobid * gg)
{
/*-- sd = source_d --*/
  datad *d;
  GSList *l;
  gint i, id;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;

  /*-- k is the row number in source_d --*/

  if (sd->rowid.id.nels > 0) {
    id = sd->rowid.id.els[k];
    if (id < 0)      /*-- this would indicate a bug --*/
      return;

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (d == sd)
        continue;        /*-- skip the originating datad --*/

      /*-- if this id exists, it is in the range of d's ids ... --*/
      if (d->rowid.id.nels > 0 && d->rowid.idv.nels > id) {
        /*-- i is the row number, irrespective of rows_in_plot --*/
        i = d->rowid.idv.els[id];
        if (i < 0)      /*-- then no cases in d have this id --*/
          continue;

        /*-- if we get here, d has one case with the indicated id --*/
        if (d->sampled.els[i]) {
          switch (cpanel->br_mode) {

            /*-- if persistent, handle all target types --*/
          case BR_PERSISTENT:

            /*
             * make it link for everything, no matter
             * what kind of brushing is turned on, because
             * otherwise, connections between points and edges
             * gets messed up.
             */

            if (!d->hidden_now.els[i]) {
              d->color.els[i] = d->color_now.els[i] = sd->color.els[k];
              d->glyph.els[i].size = d->glyph_now.els[i].size =
                  sd->glyph.els[k].size;
              d->glyph.els[i].type = d->glyph_now.els[i].type =
                  sd->glyph.els[k].type;
            }
            d->hidden.els[i] = d->hidden_now.els[i] = sd->hidden.els[k];
/*
              switch (cpanel->br_point_targets) {
                case BR_CANDG:
                  if (!d->hidden_now.els[i]) {
                    d->color.els[i] = d->color_now.els[i] = sd->color.els[k];
                    d->glyph.els[i].size = d->glyph_now.els[i].size =
                      sd->glyph.els[k].size;
                    d->glyph.els[i].type = d->glyph_now.els[i].type =
                      sd->glyph.els[k].type;
                  }
                break;
                case BR_COLOR:
                  if (!d->hidden_now.els[i])
                    d->color.els[i] = d->color_now.els[i] = sd->color.els[k];
                break;
                case BR_GLYPH:
                  if (!d->hidden_now.els[i]) {
                    d->glyph.els[i].size = d->glyph_now.els[i].size =
                      sd->glyph.els[k].size;
                    d->glyph.els[i].type = d->glyph_now.els[i].type =
                      sd->glyph.els[k].type;
                  }
                break;
                case BR_GSIZE: 
                  if (!d->hidden_now.els[i]) {
                    d->glyph.els[i].size = d->glyph_now.els[i].size =
                      sd->glyph.els[k].size;
                  }
                break;
                case BR_HIDE:
                  d->hidden.els[i] = d->hidden_now.els[i] = sd->hidden.els[k];
                break;
                case BR_OFF:
                  ;
                break;
              }
*/
            break;

            /*-- if transient, handle all target types --*/
          case BR_TRANSIENT:
            if (!d->hidden_now.els[i]) {
              d->color_now.els[i] = sd->color_now.els[k];
              d->glyph_now.els[i].size = sd->glyph_now.els[k].size;
              d->glyph_now.els[i].type = sd->glyph_now.els[k].type;
            }
            d->hidden_now.els[i] = sd->hidden_now.els[k];
/*
              switch (cpanel->br_point_targets) {
                case BR_CANDG:
                  if (!d->hidden_now.els[i]) {
                    d->color_now.els[i] = sd->color_now.els[k];
                    d->glyph_now.els[i].size = sd->glyph_now.els[k].size;
                    d->glyph_now.els[i].type = sd->glyph_now.els[k].type;
                  }
                break;
                case BR_COLOR:
                  if (!d->hidden_now.els[i])
                    d->color_now.els[i] = sd->color_now.els[k];
                break;
                case BR_GLYPH:
                  if (!d->hidden_now.els[i]) {
                    d->glyph_now.els[i].size = sd->glyph_now.els[k].size;
                    d->glyph_now.els[i].type = sd->glyph_now.els[k].type;
                  }
                break;
                case BR_GSIZE: 
                  if (!d->hidden_now.els[i]) {
                    d->glyph_now.els[i].size = sd->glyph_now.els[k].size;
                  }
                break;
                case BR_HIDE:
                  d->hidden_now.els[i] = sd->hidden_now.els[k];
                break;
                case BR_OFF:
                  ;
                break;
              }
*/

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

void linking_method_set(displayd * display, datad * d, ggobid * gg)
{
  cpaneld *cpanel = &display->cpanel;
  gg->linkby_cv = false;

  if (cpanel->br_linkby == BR_LINKBYVAR) {

    vartabled *vt;
/*
    gint *vars = (gint *) g_malloc(d->ncols * sizeof(gint));
    gint nvars = selected_cols_get(vars, d, gg);
*/
/*
    GtkWidget *notebook = widget_find_by_name (gg->control_panel[BRUSH],
      "notebook");
*/
    gint jvar;
    GtkWidget *clist;
    clist = get_clist_from_object (GTK_OBJECT (gg->control_panel[BRUSH]));
    if (clist) {
      jvar = get_one_selection_from_clist (clist, d);
      if (jvar >= 0) {
        vt = vartable_element_get(jvar, d);
        if (vt->categorical_p) {
          gg->linkby_cv = true;
          if (d->linkvar_vt == NULL || d->linkvar_vt != vt) {
            d->linkvar_vt = vt;
          }
        }
      }
    }
    if (!gg->linkby_cv) {
      gchar *message =
        g_strdup_printf
          ("You have specified linking by categorical variable, but \n no categorical variable is selected for the current dataset.\n");
      quick_message(message, false);
      gdk_flush();
      g_free(message);
    }
  }
}

void
brush_link_by_var(gint jlinkby, vector_b * levelv,
                  cpaneld * cpanel, datad * d, ggobid * gg)
{
  gint m, i, level_value;

  /*
   * for this datad, loop once over all rows in plot 
   */
  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];

    level_value = (gint) d->raw.vals[i][jlinkby];

    if (levelv->els[level_value]) {  /*-- if it's to acquire the new symbol --*/
      if (cpanel->br_mode == BR_PERSISTENT) {
        switch (cpanel->br_point_targets) {
        case BR_CANDG:   /*-- color and glyph, type and size --*/
          d->color.els[i] = d->color_now.els[i] = gg->color_id;
          d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
          d->glyph.els[i].type = d->glyph_now.els[i].type =
              gg->glyph_id.type;
          break;
        case BR_COLOR:   /*-- color only --*/
          d->color.els[i] = d->color_now.els[i] = gg->color_id;
          break;
        case BR_GLYPH:   /*-- glyph type and size --*/
          d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
          d->glyph.els[i].type = d->glyph_now.els[i].type =
              gg->glyph_id.type;
          break;
        case BR_GSIZE:   /*-- glyph size only --*/
          d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
          break;
        case BR_HIDE:   /*-- hidden --*/
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
        case BR_GLYPH:   /*-- glyph type and size --*/
          d->glyph_now.els[i].size = gg->glyph_id.size;
          d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
        case BR_GSIZE:   /*-- glyph size only --*/
          d->glyph_now.els[i].size = gg->glyph_id.size;
          break;
        case BR_HIDE:   /*-- hidden --*/
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
      case BR_GLYPH:   /*-- glyph type and size --*/
        d->glyph_now.els[i].size = d->glyph.els[i].size;
        d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
      case BR_GSIZE:   /*-- glyph size only --*/
        d->glyph_now.els[i].size = d->glyph.els[i].size;
        break;
      case BR_HIDE:   /*-- hidden --*/
        d->hidden_now.els[i] = d->hidden.els[i];
        break;
      default:
        break;
      }
    }
  }
}

/*
 * We're working too hard here, looping whether there's any
 * change or not.  Maybe there's an easy way to set the value
 * of changed by keeping track of pts_under_brush_prev?
*/
gboolean
build_symbol_vectors_by_var(cpaneld * cpanel, datad * d, ggobid * gg)
{
  gint i, m, level_value, level_value_max;
  vector_b levelv;
  gint jlinkby;
  /*-- for other datad's --*/
  GSList *l;
  datad *dd;
  vartabled *vtt;
  gboolean changed = false;

  if (d->linkvar_vt == NULL)
    return false;

  jlinkby = g_slist_index(d->vartable, d->linkvar_vt);
/*
 * I may not want to allocate and free this guy every time the
 * brush moves.
*/
  level_value_max = d->linkvar_vt->nlevels;
  for (i = 0; i < d->linkvar_vt->nlevels; i++) {
    level_value = d->linkvar_vt->level_values[i];
    if (level_value > level_value_max)
      level_value_max = level_value;
  }

  vectorb_init_null(&levelv);
  vectorb_alloc(&levelv, level_value_max + 1);
  vectorb_zero(&levelv);

  /*-- find the levels which are among the points under the brush --*/
  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    if (d->pts_under_brush.els[i]) {
      level_value = (gint) d->raw.vals[i][jlinkby];
      levelv.els[level_value] = true;
    }
  }


  /*-- first do this d --*/
  brush_link_by_var(jlinkby, &levelv, cpanel, d, gg);

  /*-- now for the rest of them --*/
  for (l = gg->d; l; l = l->next) {
    dd = l->data;
    if (dd != d) {
      vtt = vartable_element_get_by_name(d->linkvar_vt->collab, dd);
      if (vtt != NULL) {
        jlinkby = g_slist_index(dd->vartable, vtt);
        brush_link_by_var(jlinkby, &levelv, cpanel, dd, gg);
      }
    }
  }

  vectorb_free(&levelv);

  changed = true;
  return (changed);
}
