/* identify.c */
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

gint
find_nearest_point (icoords *lcursor_pos, splotd *splot, datad *d, ggobid *gg)
{
/*
 * Returns index of nearest un-hidden point
*/
  gint i, k, sqdist, near, xdist, ydist, npoint;

  g_assert (d->hidden.nels == d->nrows);

  npoint = -1;
  near = 20*20;  /* If nothing is close, don't show any label */

  for (i=0; i<d->nrows_in_plot; i++) {
    if (!d->hidden_now.els[ k = d->rows_in_plot.els[i] ]) {
      xdist = splot->screen[k].x - lcursor_pos->x;
      ydist = splot->screen[k].y - lcursor_pos->y;
      sqdist = xdist*xdist + ydist*ydist;
      if (sqdist < near) {
        near = sqdist;
        npoint = k;
      }
    }
  }
  return (npoint);
}

/*-- still having trouble getting identify turned off properly --*/
RedrawStyle
identify_activate (gint state, displayd *display, ggobid *gg)
{
  RedrawStyle redraw_style = NONE;
  datad *d = display->d;

/* At the moment, do the same thing whether identify is turning on or off */
  if (state == on || state == off) {
    if (d->nearest_point != -1) redraw_style = QUICK;
    d->nearest_point = -1;
  }

  return redraw_style;
}

void
sticky_id_toggle (datad *d, ggobid *gg)
{
  gint i = 0;
  gboolean i_in_list = false;
  gpointer ptr = NULL;

  if (d->nearest_point != -1) {

    if (d->sticky_ids && g_slist_length (d->sticky_ids) > 0) {
      GSList *l;
      for (l = d->sticky_ids; l; l = l->next) {
        i = GPOINTER_TO_INT (l->data);
        if (i == d->nearest_point) {
          i_in_list = true;
          ptr = l->data;
          break;
        }
      }
    }

    if (i_in_list) {
      d->sticky_ids = g_slist_remove (d->sticky_ids, ptr);
      sticky_id_link_by_id (STICKY_REMOVE, d->nearest_point, d, gg);
       /* This will become an event on the datad when we move to
          Gtk objects (soon now!) */
      gtk_signal_emit(GTK_OBJECT(gg),
        GGobiSignals[STICKY_POINT_REMOVED_SIGNAL], d->nearest_point,
        (gint) UNSTICKY, d);
    } else {
      ptr = GINT_TO_POINTER (d->nearest_point);
      d->sticky_ids = g_slist_append (d->sticky_ids, ptr);
      sticky_id_link_by_id (STICKY_ADD, d->nearest_point, d, gg);
       /* This will become an event on the datad when we move to
          Gtk objects (soon now!) */
      gtk_signal_emit(GTK_OBJECT(gg),
        GGobiSignals[STICKY_POINT_ADDED_SIGNAL], d->nearest_point,
        (gint) STICKY, d);
    }
  }
}

/*----------------------------------------------------------------------*/
/*                Linking to other datad's by id                        */
/*----------------------------------------------------------------------*/

void
identify_link_by_id (gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gboolean inrange;

  /*-- k is the row number in source_d --*/

  if (k < 0) {  /*-- handle this case separately --*/
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (d != source_d)
        d->nearest_point_prev = d->nearest_point = -1;
    }
    return;
  }

  if (source_d->rowIds) {
           /* if there is no */
    if(!source_d->rowIds[k]) {
       return;
    }
    for (l = gg->d; l; l = l->next) {
      gpointer ptr;
      d = (datad *) l->data;
      inrange = false;

      if (d == source_d || d->idTable == NULL)
        continue;        /*-- skip the originating datad --*/

      ptr = g_hash_table_lookup(d->idTable, source_d->rowIds[k]);
      if(ptr) {
        inrange = true;
        d->nearest_point_prev = d->nearest_point;
        d->nearest_point = * ((guint *)ptr);
      }

      if (!inrange) {
        d->nearest_point_prev = d->nearest_point;
        d->nearest_point = -1;
      }
    }
    return;
  }
}

void
sticky_id_link_by_id (gint whattodo, gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gint i, n, id = -1;
  gboolean i_in_list = false;
  GSList *ll;
  gpointer ptr = NULL;


  /*-- k is the row number in source_d --*/

  if(source_d->rowIds && source_d->rowIds[k]) {
      ptr = g_hash_table_lookup(source_d->idTable, source_d->rowIds[k]);
      if(ptr) 
         id = *(guint *) ptr;
  }

  if (id < 0)  /*-- this would indicate a bug --*/
    return;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d == source_d)
      continue;        /*-- skip the originating datad --*/

    i = -1;

    /*-- if this id exists is in the range of d's ids ... --*/
    if(d->idTable) {
      gpointer ptr = g_hash_table_lookup(d->idTable, source_d->rowIds[k]);
      if(ptr) 
         i = *(guint *) ptr;        
    }

    if (i < 0)  /*-- then no cases in d have this id --*/
      continue;

    if (g_slist_length (d->sticky_ids) > 0) {
      for (ll = d->sticky_ids; ll; ll = ll->next) {
        n = GPOINTER_TO_INT (ll->data);
        if (n == i) {  /*-- the row number of the id --*/
          i_in_list = true;
          ptr = ll->data;
          break;
        }
      }
    }

    if (i_in_list && whattodo == STICKY_REMOVE) {
      d->sticky_ids = g_slist_remove (d->sticky_ids, ptr);
    } else if (!i_in_list && whattodo == STICKY_ADD) {
      ptr = GINT_TO_POINTER (i);
      d->sticky_ids = g_slist_append (d->sticky_ids, ptr);
    }
  }
}

/*----------------------------------------------------------------------*/
/*                Called from sp_plot.c                                 */
/*----------------------------------------------------------------------*/

gchar *
identify_label_fetch (gint k, cpaneld *cpanel, datad *d, ggobid *gg)
{
  gchar *lbl;
  gint id_display_type = cpanel->id_display_type;

/*
 * How can I tell if the current page of the notebook
 * corresponds to the data?
*/
  /*-- if categorical, use level name ... --*/
  if (id_display_type == ID_VAR_LABELS) {
    vartabled *vt;
    GtkWidget *clist =
      get_clist_from_object (GTK_OBJECT (gg->control_panel[IDENT]));
    datad *clistd = (datad *) gtk_object_get_data (GTK_OBJECT(clist), "datad");

    if (clistd != d) {
/*
g_printerr ("selected variables don't correspond to what is identified\n");
*/
      id_display_type = ID_RECORD_LABEL;
      /*-- this will be caught below --*/

    } else {
      gint *vars = (gint *) g_malloc (d->ncols * sizeof(gint));
      gint nvars = get_selections_from_clist (d->ncols, vars, clist, d);
      gint j, lval;

      for (j=0; j<nvars; j++) {
        vt = vartable_element_get (vars[j], d);
        if (vt == NULL) continue;

        /*  missing value  */
        if (d->nmissing && d->missing.vals[k][vars[j]]) {
          if (j == 0)
            lbl = g_strdup_printf ("%s=NA", vt->collab_tform);
          else
            lbl = g_strdup_printf ("%s, %s=NA", lbl, vt->collab_tform);
        } else {   /* not missing */

          if (vt->vartype == categorical) {
            /*
             * since the level values can be any arbitrary integers,
             * it's necessary to dig out the level name using the list
             * of level values.
            */
            gint n, ktmp;
            gint kval = (gint) d->tform.vals[k][vars[j]];
            lval = -1;
            for (n=0; n<vt->nlevels; n++) {
              ktmp = vt->level_values[n];
              if (ktmp == kval) {
                lval = n;
                break;
              }
            }
          }
          if (lval == -1) {
            g_printerr ("The levels for %s aren't specified correctly\n",
              vt->collab);
            return NULL;
          }
  
          if (j == 0) {
            lbl = (vt->vartype == categorical) ?
              g_strdup_printf ("%s=%s",
                vt->collab_tform, vt->level_names[lval]) :
              g_strdup_printf ("%s=%g",
                vt->collab_tform, d->tform.vals[k][vars[j]]);
          } else {
            lbl = (vt->vartype == categorical) ?
              g_strdup_printf ("%s, %s=%s",
                  lbl, vt->collab_tform, vt->level_names[lval]) :
              g_strdup_printf ("%s, %s=%g",
                lbl, vt->collab_tform, d->tform.vals[k][vars[j]]);
          }
        }
      }
      g_free (vars);
    }
  }

  
  if (id_display_type == ID_RECORD_LABEL)
    lbl = (gchar *) g_array_index (d->rowlab, gchar *, k);

  else if (id_display_type == ID_RECORD_NO) {
    lbl = g_strdup_printf ("%d", k);

  }  else if (id_display_type == ID_RECORD_ID) {
    if (d->rowIds && d->rowIds[k]) {
      lbl = g_strdup_printf ("%s", d->rowIds[k]);
    } else {
      lbl = g_strdup ("");
    }
  }

  return lbl;
}
