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

gboolean
symbol_link_by_id (gboolean persistentp, gint k, datad * sd, ggobid * gg)
{
/*-- sd = source_d --*/
  datad *d;
  GSList *l;
  gint i, id = -1;
  /*-- this is the cpanel for the display being brushed --*/
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean changed = false;

  /*-- k is the row number in source_d --*/

  if (sd->rowIds) {
    gpointer ptr = g_hash_table_lookup(sd->idTable, sd->rowIds[k]);
    if (ptr)
      id = * ((guint *)ptr);
  }

  if (id < 0)      /*-- this would indicate a bug --*/
/**/return false;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d == sd)
      continue;        /*-- skip the originating datad --*/

    i = -1;
    if(sd->rowIds && d->idTable) {
       gpointer ptr = g_hash_table_lookup(d->idTable, sd->rowIds[id]);
       if(ptr) {
         i = * ((guint *)ptr);
       }        
    }

    if (i < 0)      /*-- then no cases in d have this id --*/
       continue;

    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;
    if (d->sampled.els[i] && !d->excluded.els[i]) {

      if (persistentp || cpanel->br.mode == BR_PERSISTENT) {

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

        /*-- should we handle this here?  --*/
        d->excluded.els[i] = sd->excluded.els[k];

      } else if (cpanel->br.mode == BR_TRANSIENT) {

        if (!d->hidden_now.els[i]) {
          d->color_now.els[i] = sd->color_now.els[k];
          d->glyph_now.els[i].size = sd->glyph_now.els[k].size;
          d->glyph_now.els[i].type = sd->glyph_now.els[k].type;
        }
        d->hidden_now.els[i] = sd->hidden_now.els[k];
      }
    }
  }
  return changed;
}

gboolean
exclude_link_by_id (gint k, datad * sd, ggobid * gg)
{
/*-- sd = source_d --*/
  datad *d;
  GSList *l;
  gint i, id = -1;
  gboolean changed = false;

  /*-- k is the row number in source_d --*/

  if (sd->rowIds) {
    gpointer ptr = g_hash_table_lookup(sd->idTable, sd->rowIds[k]);
    if (ptr)
      id = * ((guint *)ptr);
  }

  if (id < 0)      /*-- this would indicate a bug --*/
/**/return false;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d == sd)
      continue;        /*-- skip the originating datad --*/

    i = -1;
    if(sd->rowIds && d->idTable) {
       gpointer ptr = g_hash_table_lookup(d->idTable, sd->rowIds[id]);
       if(ptr) {
         i = * ((guint *)ptr);
       }        
    }

    if (i < 0)      /*-- then no cases in d have this id --*/
       continue;

    /*-- if we get here, d has one case with the indicated id --*/
    changed = true;
    if (d->sampled.els[i])
      d->excluded.els[i] = sd->excluded.els[k];
  }
  return changed;
}

/*----------------------------------------------------------------------*/
/*   Linking within and between datad's using a categorical variable    */
/*----------------------------------------------------------------------*/

void
brush_link_by_var(gint jlinkby, vector_b * levelv,
                  cpaneld * cpanel, datad * d, ggobid * gg)
{
  gint m, i, level_value;

  /*
   * for this datad, loop once over all rows in plot 
   */
  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];

    level_value = (gint) d->raw.vals[i][jlinkby];

    if (levelv->els[level_value]) {  /*-- if it's to acquire the new symbol --*/
      if (cpanel->br.mode == BR_PERSISTENT) {
        switch (cpanel->br.point_targets) {
          case br_candg:   /*-- color and glyph, type and size --*/
            d->color.els[i] = d->color_now.els[i] = gg->color_id;
            d->glyph.els[i].size = d->glyph_now.els[i].size =
                gg->glyph_id.size;
            d->glyph.els[i].type = d->glyph_now.els[i].type =
                gg->glyph_id.type;
          break;
          case br_color:   /*-- color only --*/
            d->color.els[i] = d->color_now.els[i] = gg->color_id;
          break;
          case br_glyph:   /*-- glyph type and size --*/
            d->glyph.els[i].size = d->glyph_now.els[i].size =
              gg->glyph_id.size;
            d->glyph.els[i].type = d->glyph_now.els[i].type =
              gg->glyph_id.type;
          break;
          case br_hide:   /*-- hidden --*/
            d->hidden.els[i] = d->hidden_now.els[i] = true;
          break;
	  /*
          case br_select:
            d->hidden.els[i] = d->hidden_now.els[i] = false;
          break;
	  */
          default:
          break;
        }

      } else if (cpanel->br.mode == BR_TRANSIENT) {
        switch (cpanel->br.point_targets) {
          case br_candg:
            d->color_now.els[i] = gg->color_id;
            d->glyph_now.els[i].size = gg->glyph_id.size;
            d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
          case br_color:
            d->color_now.els[i] = gg->color_id;
          break;
          case br_glyph:   /*-- glyph type and size --*/
            d->glyph_now.els[i].size = gg->glyph_id.size;
            d->glyph_now.els[i].type = gg->glyph_id.type;
          break;
          case br_hide:   /*-- hidden --*/
            d->hidden_now.els[i] = true;
          break;
	  /*
          case br_select:
            d->hidden_now.els[i] = false;
          break;
	  */
          default:
          break;
        }
      }

    } else {  /*-- if it's to revert to the previous symbol --*/
      /*-- should only matter if transient, right? --*/
      switch (cpanel->br.point_targets) {
        case br_candg:
          d->color_now.els[i] = d->color.els[i];
          d->glyph_now.els[i].size = d->glyph.els[i].size;
          d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
        case br_color:
          d->color_now.els[i] = d->color.els[i];
        break;
        case br_glyph:   /*-- glyph type and size --*/
          d->glyph_now.els[i].size = d->glyph.els[i].size;
          d->glyph_now.els[i].type = d->glyph.els[i].type;
        break;
        case br_hide:   /*-- hidden --*/
          d->hidden_now.els[i] = d->hidden.els[i];
        break;
	/* disabled
        case br_select:
          d->hidden_now.els[i] = d->hidden.els[i];
        break;
	*/
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
    i = d->rows_in_plot.els[m];
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


/*********************************************************************/
/*          Create a variable notebook for brush linking rule        */
/*********************************************************************/

void linkby_notebook_subwindow_add (datad *d, GtkWidget *notebook, ggobid *);

void
linking_method_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  datad *d = gtk_object_get_data (GTK_OBJECT(cl), "datad");
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;  
  /* cpanel has to come from display; is there another way to
     get the display? Or can I get the cpanel from the swin? */
  /* think about why this is specified in cpanel, gg, and d -- seems
     confusing */
  GtkWidget *notebook;

  notebook = (GtkWidget *) gtk_object_get_data(GTK_OBJECT(cl), "notebook");

  cpanel->br.linkby_row = row;
  cpanel->br.linkby_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));

  if (row <= 0) {
    /*cpanel->br_linkby = BR_LINKBYID;*/
    gg->linkby_cv = false;
    return;  /* link by case id; done */
  } else {
    gpointer ptr = gtk_clist_get_row_data (GTK_CLIST(cl), row);
    gint jvar = GPOINTER_TO_INT(ptr);
    vartabled *vt;

    /*cpanel->br_linkby = BR_LINKBYVAR;*/
    gg->linkby_cv = true;
    if (d->linkvar_vt == NULL || d->linkvar_vt != vt) {
      vt = vartable_element_get(jvar, d);
      d->linkvar_vt = vt;
    }
  }
}

void 
linkby_notebook_varchange_cb (ggobid *gg, vartabled *vt, gint which,
  datad *data, void *notebook)
{
  GtkWidget *swin, *clist;

  /*-- add one or more variables to this datad --*/
  datad *d = (datad *) datad_get_from_notebook (GTK_WIDGET(notebook), gg);
  gint kd = g_slist_index (gg->d, d);

  /*-- get the clist associated with this data; clear and rebuild --*/
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (GTK_WIDGET(notebook)), kd);
  if (swin) {
    gint j, k;
    gchar *row[1];
    vartabled *vt;
    clist = GTK_BIN (swin)->child;

    gtk_clist_freeze (GTK_CLIST(clist));
    gtk_clist_clear (GTK_CLIST (clist));

    /* Insert this string */
    row[0] = g_strdup_printf ("Link by case id");
    gtk_clist_append (GTK_CLIST (clist), row);

    k = 1;
    for (j=0; j<d->ncols; j++) {
      vt = vartable_element_get (j, d);
      if (vt && vt->vartype == categorical) {
        row[0] = g_strdup_printf ("Link by %s", vt->collab);
        gtk_clist_append (GTK_CLIST (clist), row);
        gtk_clist_set_row_data(GTK_CLIST(clist), k, GINT_TO_POINTER(j));
        g_free (row[0]);
        k++;
      }
    }
    gtk_clist_thaw (GTK_CLIST(clist));
  }
}

void 
linkby_notebook_list_changed_cb(ggobid *gg, datad *d, void *notebook)
{
  linkby_notebook_varchange_cb(gg, NULL, -1, d, notebook);
}

CHECK_EVENT_SIGNATURE(linkby_notebook_adddata_cb, datad_added_f)
CHECK_EVENT_SIGNATURE(linkby_notebook_varchange_cb, variable_added_f)
CHECK_EVENT_SIGNATURE(linkby_notebook_list_changed_cb, variable_list_changed_f)

static void
linkby_notebook_adddata_cb (ggobid *gg, datad *d, void *notebook, GtkSignalFunc func)
{
  if (g_slist_length (d->vartable)) {
    linkby_notebook_subwindow_add (d, notebook, gg);
  }

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (GTK_OBJECT(notebook)),
                              g_slist_length (gg->d) > 1);
}

void
linkby_notebook_subwindow_add (datad *d, GtkWidget *notebook, ggobid *gg)
{
  GtkWidget *swin, *clist;
  gint j, k;
  gchar *row[1];
  vartabled *vt;

  GtkSelectionMode mode = GTK_SELECTION_SINGLE;

  if (d->ncols == 0)
    return;

  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  gtk_object_set_data(GTK_OBJECT(swin), "datad", d);  /*setdata*/
/*
 * name or nickname?  Which one we'd prefer to use depends on the
 * size of the space we're working in -- maybe this will become an
 * argument.
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin,
    (d->nickname != NULL) ?
      gtk_label_new (d->nickname) : gtk_label_new (d->name)); 

  /* add the CList */
  clist = gtk_clist_new (1);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), mode);
  gtk_object_set_data (GTK_OBJECT (clist), "datad", d);
  gtk_object_set_data (GTK_OBJECT (clist), "notebook", notebook);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row", 
    GTK_SIGNAL_FUNC (linking_method_set_cb), gg);

  /* Insert this string */
  row[0] = g_strdup_printf ("Link by case id");
  gtk_clist_append (GTK_CLIST (clist), row);

  k = 1;
  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    if (vt && vt->vartype == categorical) {
      row[0] = g_strdup_printf ("Link by %s", vt->collab);
      gtk_clist_append (GTK_CLIST (clist), row);
      gtk_clist_set_row_data(GTK_CLIST(clist), k, GINT_TO_POINTER(j));
      g_free (row[0]);
    }
  }

  /*-- suggested by Gordon Deane; causes no change under linux --*/
  gtk_clist_set_column_width(GTK_CLIST(clist), 0,
    gtk_clist_optimal_column_width (GTK_CLIST(clist), 0));
  /*--                           --*/

  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_widget_show_all (swin);

  /* It appears that this has to follow 'show_all' to take effect */
  gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
}

GtkWidget *
create_linkby_notebook (GtkWidget *box, ggobid *gg)
{
  GtkWidget *notebook;
  gint nd = g_slist_length (gg->d);
  GSList *l;
  datad *d;

  GtkSelectionMode mode = GTK_SELECTION_SINGLE;
  vartyped vtype = categorical;
  datatyped dtype = all_datatypes;

  /* Create a notebook, set the position of the tabs */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), nd > 1);
  gtk_box_pack_start (GTK_BOX (box), notebook, true, true, 2);
  gtk_object_set_data (GTK_OBJECT(notebook), "SELECTION", (gpointer) mode);
  gtk_object_set_data (GTK_OBJECT(notebook), "vartype", (gpointer) vtype);
  gtk_object_set_data (GTK_OBJECT(notebook), "datatype", (gpointer) dtype);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (g_slist_length (d->vartable)) {
      linkby_notebook_subwindow_add (d, notebook, gg);
    }
  }

  /*-- listen for variable_added and _list_changed events on main_window --*/
  /*--   ... list_changed would be enough --*/
  gtk_signal_connect (GTK_OBJECT (gg),
		      "variable_added", 
		      GTK_SIGNAL_FUNC (linkby_notebook_varchange_cb),
		      GTK_OBJECT (notebook));
  gtk_signal_connect (GTK_OBJECT (gg),
		      "variable_list_changed", 
		      GTK_SIGNAL_FUNC (linkby_notebook_varchange_cb),
		      GTK_OBJECT (notebook));

  /*-- listen for datad_added events on main_window --*/
  gtk_signal_connect (GTK_OBJECT (gg),
		      "datad_added", 
		      GTK_SIGNAL_FUNC (linkby_notebook_adddata_cb),
		      GTK_OBJECT (notebook));

  return notebook;
}
