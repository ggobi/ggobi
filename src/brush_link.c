/* brush_link.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <string.h>

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
    gpointer ptr;
    if (sd->rowIds[k]) {
      ptr = g_hash_table_lookup(sd->idTable, sd->rowIds[k]);
      if (ptr)
        id = * ((guint *)ptr);
    } else {
      /* 
       * I've only seen one thing that causes this, and that was in
       * an xml file with two datad's, nodes and edges, but in which
       * the edge data mistakenly duplicated the record ids of the
       * node data.  I removed those record ids, and the file was
       * fine.  -- Debby
       */
      g_printerr ("rowIds[%d] is null\n", k);
    }
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

enum { LINKBYLIST_NAME, LINKBYLIST_VT, LINKBYLIST_NCOLS };

void linkby_notebook_subwindow_add (datad *d, GtkWidget *notebook, ggobid *);

void varlist_append(GtkListStore *list, vartabled *vt) {
	gchar *row;
	GtkTreeIter iter;
	
	if (vt && vt->vartype == categorical) {
		gtk_list_store_append(list, &iter);
		row = g_strdup_printf("%s", vt->collab);
		gtk_list_store_set(list, &iter, LINKBYLIST_NAME, row, LINKBYLIST_VT, vt, -1);
		g_free(row);
	}
}
void varlist_populate(GtkListStore *list, datad *d) {
  gint j;
  GtkTreeIter first;
  vartabled *vt;
  
  gtk_list_store_append(list, &first);
  gtk_list_store_set(list, &first, LINKBYLIST_NAME, "<i>Case ID</i>", -1);
  
  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get(j, d);
    varlist_append(list, vt);
  }
}

/* called from cpanel_brush_set */
void
linkby_current_page_set (displayd *display, GtkWidget *notebook, ggobid *gg)
{
  GtkWidget *swin;
  datad *d = display->d, *paged;
  gint page_num;

  if (notebook == NULL) {
    return;
  }

  /*
   * For each page of the notebook, get its child, the scrolled
   * window.  Get the datad that the scrolled window knows about,
   * and compare it with display->d
   */

  page_num = 0;
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK(notebook), page_num);
  while (swin) {
    paged = (datad *) g_object_get_data (G_OBJECT (swin), "datad");

    //g_printerr ("(current_page_set) paged %s d %s   ==? %d\n", paged->name, d->name, (paged == d));

    gtk_widget_set_sensitive (swin, (paged == d));
    if (paged == d) {
      gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook), page_num);
    }
    page_num += 1;
    swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK(notebook), page_num);
  }
}

void
linking_method_set_cb (GtkTreeSelection *treesel, ggobid *gg)
{
  datad *d = g_object_get_data (G_OBJECT(gtk_tree_selection_get_tree_view(treesel)), "datad");
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;  
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreePath *path;
  gint row = -1;

  if(gtk_tree_selection_get_selected(treesel, &model, &iter)) {
	  path = gtk_tree_model_get_path(model, &iter);
	  row = gtk_tree_path_get_indices(path)[0];
	  gtk_tree_path_free(path);
  }
  
  //notebook = (GtkWidget *) g_object_get_data(G_OBJECT(cl), "notebook");
  cpanel->br.linkby_row = row;

  if (row <= 0) {
    gg->linkby_cv = false;
    return;  /* link by case id; done */
  } else {
    //gpointer ptr = gtk_clist_get_row_data (GTK_CLIST(cl), row);
    //gint jvar = GPOINTER_TO_INT(ptr);
    vartabled *vt;
	gtk_tree_model_get(model, &iter, LINKBYLIST_VT, &vt, -1);
	gg->linkby_cv = true;
    d->linkvar_vt = vt;
	
    /* I need to get the text in the row and strip out "Link by ".
     * That will give me the variable name */
    /*
	 ok = gtk_clist_get_text (GTK_CLIST(cl), row, 0, &rowtext);
    if (ok) {
      varname = &rowtext[8];
      for (j=0; j<d->ncols; j++) {
        vt = vartable_element_get (j, d);
        if (vt && vt->vartype == categorical) {
          if (strcmp(vt->collab_tform, varname) == 0) {
            jvar = j;
            gg->linkby_cv = true;
            d->linkvar_vt = vt;
          }
        }
      }
    }*/

  }
}

GtkListStore *
list_from_data(ggobid *gg, datad *data, GtkNotebook *notebook) {
	GtkWidget *swin;
	GtkListStore *list = NULL;
	
	gint kd = g_slist_index(gg->d, data);
	swin = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), kd);
	
	if(swin)
		list = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(GTK_BIN(swin)->child)));
	
	return(list);
}

void 
linkby_notebook_varchanged_cb (ggobid *gg, datad *data, GtkNotebook *notebook) {
	GtkListStore *list = list_from_data(gg, data, notebook);
	gtk_list_store_clear(list);
	varlist_populate(list, data);
}

void 
linkby_notebook_varadded_cb (ggobid *gg, vartabled *vt, gint which,
  datad *data, GtkNotebook *notebook)
{
	GtkListStore *model = list_from_data(gg, data, notebook);
	if (model)
		varlist_append(model, vt);
	
	#if 0

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
  #endif
}

void 
linkby_notebook_list_changed_cb(ggobid *gg, datad *d, void *notebook)
{
  linkby_notebook_varchanged_cb(gg, d, notebook);
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
  GtkWidget *swin, *treeview;
  GtkListStore *list;

  GtkSelectionMode mode = GTK_SELECTION_SINGLE;

  if (d->ncols == 0)
    return;

  /* Create a scrolled window to pack the CList widget into */
  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  /* If this is not the first child of the notebook, initialize it
   * as insensitive.
   */

/*
  g_printerr ("(subwindow_add) d %s nchildren %d\n", d->name, g_list_length(gtk_container_get_children(GTK_CONTAINER(notebook))));
*/
  if (g_list_length(gtk_container_get_children(GTK_CONTAINER(notebook))) != 0) {
    gtk_widget_set_sensitive (swin, false);
  }
  g_object_set_data(G_OBJECT(swin), "datad", d);  /*setdata*/
/*
 * name or nickname?  Which one we'd prefer to use depends on the
 * size of the space we're working in -- maybe this will become an
 * argument.
*/
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), swin,
    (d->nickname != NULL) ?
      gtk_label_new (d->nickname) : gtk_label_new (d->name)); 

  
  /* add the treeview (list) */
  list = gtk_list_store_new(LINKBYLIST_NCOLS, G_TYPE_STRING, G_TYPE_POINTER);
  varlist_populate(list, d);

  treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list));
  populate_tree_view(treeview, NULL, 1, false, mode, G_CALLBACK(linking_method_set_cb), gg);

  g_object_set_data (G_OBJECT (treeview), "datad", d);
  //g_object_set_data (G_OBJECT (clist), "notebook", notebook);
  
  gtk_container_add (GTK_CONTAINER (swin), treeview);
  gtk_widget_show_all (swin);
  
  select_tree_view_row(treeview, 0);
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
  g_object_set_data (G_OBJECT(notebook), "SELECTION", (gpointer) mode);
  g_object_set_data (G_OBJECT(notebook), "vartype", (gpointer) vtype);
  g_object_set_data (G_OBJECT(notebook), "datatype", (gpointer) dtype);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (g_slist_length (d->vartable)) {
      linkby_notebook_subwindow_add (d, notebook, gg);
    }
  }

  /*-- listen for variable_added and _list_changed events on main_window --*/
  /*--   ... list_changed would be enough but it's only called on delete --*/
  g_signal_connect (G_OBJECT (gg),
		      "variable_added", 
		      G_CALLBACK (linkby_notebook_varadded_cb),
		      GTK_OBJECT (notebook));
  g_signal_connect (G_OBJECT (gg),
		      "variable_list_changed", 
		      G_CALLBACK (linkby_notebook_varchanged_cb),
		      GTK_OBJECT (notebook));

  /*-- listen for datad_added events on main_window --*/
  g_signal_connect (G_OBJECT (gg),
		      "datad_added", 
		      G_CALLBACK (linkby_notebook_adddata_cb),
		      GTK_OBJECT (notebook));

  return notebook;
}
