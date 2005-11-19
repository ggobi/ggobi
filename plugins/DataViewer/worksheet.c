#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"

#include <stdlib.h>
#include <strings.h>

#include "errno.h"

#include "parcoordsClass.h"

#include "GGStructSizes.c"

void       add_ggobi_sheets(ggobid *gg, GtkWidget *notebook);
void       close_worksheet_window(GtkWidget *w, PluginInstance *inst);
GtkWidget* create_ggobi_sheet(datad *data, ggobid *gg);
void       add_ggobi_data(datad *data, GtkTreeModel *model);
GtkWidget *create_ggobi_worksheet_window(ggobid *gg, PluginInstance *inst);

void       show_data_edit_window(GtkAction *actions, PluginInstance *inst);

GtkWidget* create_ggobi_sheet(datad *data, ggobid *gg);
void update_cell(gint row, gint column, double value, datad *data);
void cell_changed(GtkCellRendererText *renderer, gchar *path_str, gchar *text, GtkTreeModel *model);

void brush_change(ggobid *gg, splotd *sp, GdkEventMotion *ev, datad *d, GtkWidget *sheet);
void move_point_value(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkWidget *sheet);
void monitor_new_plot(GtkWidget *w, splotd *sp, ggobid *gg, GtkWidget *sheet);
void identify_cell(ggobid *gg, splotd *sp, gint id, datad *d, GtkWidget *sheet);
void color_row(GtkWidget *sheet, gint row, gint ncols, GdkColor *col);

void connect_to_existing_displays(ggobid *gg, GtkWidget *sheet);

static GdkColor red = {-1, 65535, 0, 0};
static GdkColor black;

#if 0
/* dfs, working on adding a search facility */
static void
row_find_by_label (GtkWidget *w, GtkWidget *notebook)
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  GtkWidget *entry;
  gchar *vname;

  GtkWidget *swin, *sheet;

  entry = widget_find_by_name (GTK_DIALOG(dialog)->vbox, "find_entry");
  if (entry == NULL || !GTK_IS_ENTRY(entry)) {
    g_printerr ("found the wrong widget; bail out\n");
    return;
  }
  vname = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);

  g_printerr ("search for %s (search under construction)\n", vname);

  /*-- now get the current sheet --*/
  swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK(notebook), 
    gtk_notebook_get_current_page (GTK_NOTEBOOK(notebook)));
  if (swin) sheet = GTK_BIN(swin)->child;
  
  /*-- look for vname in the row labels --*/
  if (sheet) {
    gint i;
    gchar *str, *s;
    size_t l1 = (size_t) strlen(vname);
    gboolean gotit = false;

    for (i=0; i<GTK_SHEET(sheet)->maxrow; i++) {
      str = GTK_SHEET(sheet)->row[i].button.label;
      s = str;
      while (strlen(s) >= l1) {
        if (strncasecmp(s, vname, l1) == 0) {
          gtk_sheet_moveto(GTK_SHEET(sheet), i, 2, 0.5, 0.5);
          gtk_sheet_select_row(GTK_SHEET(sheet), i);
          gotit = true; break;
        }
        s++;
      }
      if (gotit) break;
    }
  }

  g_free(vname);
}
static void
dialog_close (GtkWidget *w) 
{
  GtkWidget *dialog = gtk_widget_get_toplevel (w);
  gtk_widget_destroy (dialog);
}
static void
open_find_dialog (GtkWidget *window)
{
  GtkWidget *dialog, *hb, *entry;
  GtkWidget *okay_btn, *cancel_btn;
  GtkWidget *notebook = (GtkWidget *)
    g_object_get_data(G_OBJECT (window), "notebook");

  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), "Find row by label");

  /*-- label and entry --*/
  hb = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Enter search string: "),
    true, true, 2);
  entry = gtk_entry_new();
  gtk_widget_set_name (entry, "find_entry");

  gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hb,
    false, false, 2);

  /*-- ok button --*/
  okay_btn = gtk_button_new_with_label ("Okay");
  g_signal_connect (G_OBJECT (okay_btn), "clicked",
    G_CALLBACK (row_find_by_label), notebook);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    okay_btn);

  /*-- cancel button --*/
  cancel_btn = gtk_button_new_with_label ("Close");
  g_signal_connect (G_OBJECT (cancel_btn), "clicked",
    G_CALLBACK (dialog_close), NULL);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    cancel_btn);

  gtk_widget_show_all (dialog);
}
static GtkItemFactoryEntry menubar_items[] = {
  { "/_Edit",            NULL,     NULL,             0, "<Branch>" },
  { "/Edit/Find ...",
       NULL,    
       (GtkItemFactoryCallback) open_find_dialog,  
       0 },
};
/* */
#endif


/**
 Called when the plugin instance is created for a new ggobi instance.
 This adds an entry to the Tools menu that the user can select to 
 bring up the datasheet window.
 */
gboolean
addToMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  GtkActionGroup *actions;
  extern GtkWidget *GGobi_addToolsMenuItem (gchar *label, ggobid *gg);

  inst->data = NULL;
  inst->info = plugin;

   /* These could be done in an onLoad routine rather than each time 
      we create a new plugin instance of this type. No big deal at all.
    */
  gdk_colormap_alloc_color(gdk_colormap_get_system(), &red, TRUE, TRUE);
  gdk_color_black(gdk_colormap_get_system(), &black);

  /*entry = GGobi_addToolsMenuItem ("Data grid ...", gg);
  g_signal_connect_object (G_OBJECT(entry), "activate",
                             G_CALLBACK (show_data_edit_window),
                             (gpointer) inst, 0);
*/
  static const gchar *ui = 
  "	<ui>"
  "		<menubar>"
  "			<menu action='Tools'>"
  "				<menuitem action='DataView'/>"
  "			</menu>"
  "		</menubar>"
  "	</ui>";
  
  static GtkActionEntry action_entries[] = {
	{ "DataView", NULL, "_Data Viewer", NULL, "View the data elements on a grid", 
		G_CALLBACK (show_data_edit_window) },
  };
  
  actions = gtk_action_group_new("DataViewer Actions");
  gtk_action_group_add_actions(actions, action_entries, G_N_ELEMENTS(action_entries), inst);
  gtk_ui_manager_insert_action_group(gg->main_menu_manager, actions, -1);
  gtk_ui_manager_add_ui_from_string(gg->main_menu_manager, ui, -1, NULL);
  
#if 0
    /* This is an attempt to use the more automated menu creation mechanism.
       However, it is not behaving itself quite yet, so we use the
       manual mechanism which is more verbose but more controllable. */
static GtkItemFactoryEntry menu_items[] = {
    {"/Data", NULL, NULL, "<LastBranch>"},
    {"/Data/View", NULL, (GtkItemFactoryCallback) show_data_edit_window, NULL}
 };

  gtk_item_factory_create_items(gg->main_menu_factory, sizeof(menu_items)/sizeof(menu_items[0]), menu_items, (gpointer) gg);
#endif
  return(true);
}


/**
  The callback for the menu item that brings up the datasheet  window
  for the GGobi instance associated with the menu.
 */
void
show_data_edit_window(GtkAction *action, PluginInstance *inst)
{
  if(g_slist_length(inst->gg->d) < 1) {
      fprintf(stderr, "No datasets to show\n");fflush(stderr);
      return;
  }

  if(inst->data == NULL) {
    GtkWidget *window;
    window = create_ggobi_worksheet_window(inst->gg, inst);
    inst->data = window;
  } else {
     gtk_widget_show_now((GtkWidget*) inst->data);
  }
}


/**
 Create the top-level window displaying the different 
 datasets in the ggobi instance.
 */
GtkWidget *
create_ggobi_worksheet_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggobi data viewer");
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

  g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (close_worksheet_window), inst);


  main_vbox=gtk_vbox_new(FALSE,1);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
  gtk_container_add(GTK_CONTAINER(window), main_vbox);

/* */
/*
  get_main_menu (menubar_items,
    sizeof (menubar_items) / sizeof (menubar_items[0]),
    gtk_accel_group_new(), window,
    &menubar, (gpointer) window);
  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, false, false, 0);
  */
/* */

  notebook=gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
  gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);
  gtk_widget_show(notebook);

  add_ggobi_sheets(gg, notebook);

  g_object_set_data(G_OBJECT (window), "notebook", notebook);
  gtk_widget_show_all(window);

  return(window);
}

/**
 Add an entry/page in the notebook for each of the datasets in the 
 ggobi instance.
 */
void
add_ggobi_sheets(ggobid *gg, GtkWidget *notebook)
{
  datad *data;
  GSList *el;

  el = gg->d;
  while(el) {
   GtkWidget *label;
   GtkWidget *sheet;
   data = (datad*) el->data;

   if (g_slist_length (data->vartable)) {
     label = gtk_label_new(data->name);
     sheet = create_ggobi_sheet(data, gg);
     gtk_notebook_append_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(sheet), label);
   }
   
   el = el->next;
  }
}


void
select_row_cb (GtkTreeSelection *tree_sel, datad *d)
{
  ggobid *gg = (ggobid *) d->gg;
  
  if (imode_get(gg) != IDENT)
    GGOBI(full_viewmode_set)(NULL_PMODE, IDENT, gg);
  
  d->nearest_point = tree_selection_get_selected_row(tree_sel);
  /*-- the label could be made sticky -- double click? keystroke? --*/
  /*sticky_id_toggle (d, gg);*/

  displays_tailpipe (QUICK, gg);
}

/**
 Create a scrolled window containing a worksheet in which the 
 contents of the data set are displayed. 
 And arrange to be informed if a cell is changed by the user
 or a new plot is created by ggobi. 
 */
GtkWidget*
create_ggobi_sheet(datad *data, ggobid *gg)
{
  GtkWidget *sheet, *scrolled_window;
  GtkListStore *model;
  GtkTreeModel *sorted_model;
  GtkTreeSelection *sel;
  GType *col_types = g_new(GType, data->ncols + 2);
  gchar **col_labels = g_new(gchar *, data->ncols + 1);
  gint i;
  
  // first column is for the row label
  col_types[0] = G_TYPE_STRING;
  col_labels[0] = "Row Label";
  col_types[data->ncols+1] = GDK_TYPE_COLOR; // last column for color (hidden)
  for(i = 0; i < data->ncols; i++) {
	  vartabled *vt = (vartabled*) g_slist_nth_data (data->vartable, i);
	  if (vt->vartype == integer || vt->vartype == counter)
		  col_types[i+1] = G_TYPE_INT;
	  else if (vt->vartype == categorical)
		  col_types[i+1] = G_TYPE_STRING;
	  else col_types[i+1] = G_TYPE_DOUBLE;
	  col_labels[i+1] = vt->collab;
  }
  
  model = gtk_list_store_newv(data->ncols+2, col_types);
  g_object_set_data(G_OBJECT(model), "data", data);
  sorted_model = gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(model));
  g_free(col_types);
  sheet = gtk_tree_view_new_with_model(sorted_model);
  // making editable cells is not that convenient
  for(i = 0; i < data->ncols+1; i++) { // note: row label not editable
	  GtkCellRenderer *renderer;
	  GtkTreeViewColumn *col;
	  GType type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(model), i);
	  if (i > 0 && type == G_TYPE_STRING) {// categorical (combo editing)
		  GtkListStore *level_model = gtk_list_store_new(1, G_TYPE_STRING);
		  GtkTreeIter iter;
		  vartabled *vt = g_slist_nth_data(data->vartable, i-1);
		  gint k;
		  for (k = 0; k < vt->nlevels; k++) {
			  gtk_list_store_append(level_model, &iter);
			  gtk_list_store_set(level_model, &iter, 0, vt->level_names[k], -1);
		  }
		  renderer = gtk_cell_renderer_combo_new();
		  g_object_set(G_OBJECT(renderer), "model", level_model, 
		  	"text-column", 0, NULL);
	  } else renderer = gtk_cell_renderer_text_new();
	  if (i > 0) {
		  g_object_set(G_OBJECT(renderer), "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);
		  g_object_set_data(G_OBJECT(renderer), "column", GINT_TO_POINTER(i-1));
		  g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_changed), model);
	  }
	  col = gtk_tree_view_column_new_with_attributes(col_labels[i], renderer, 
	  			"text", i, "foreground-gdk", data->ncols+1, NULL);
	  gtk_tree_view_column_set_sort_column_id(col, i);
	  gtk_tree_view_column_set_resizable(col, true);
	  gtk_tree_view_insert_column(GTK_TREE_VIEW(sheet), col, -1);
  }
  g_free(col_labels);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(sheet), true);
  gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(sheet), true);
  sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(sheet));
  g_signal_connect(G_OBJECT(sel), "changed", G_CALLBACK(select_row_cb), data);
  
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_window), sheet);

  add_ggobi_data(data, GTK_TREE_MODEL(model));

  gtk_widget_show_all(scrolled_window);

  g_signal_connect_object(G_OBJECT(gg), "splot_new",
    G_CALLBACK(monitor_new_plot), G_OBJECT(sheet), 0);

  connect_to_existing_displays(gg, sheet);

  return(scrolled_window);
}


CHECK_EVENT_SIGNATURE(brush_change, brush_motion_f)
CHECK_EVENT_SIGNATURE(move_point_value, move_point_f)
CHECK_EVENT_SIGNATURE(identify_cell, identify_point_f)

/** 
  Register signal handlers for GGobi events on the particular splot.
 */
void
connect_to_splot(splotd *sp, GtkWidget *sheet)
{
  ggobid *gg = sp->displayptr->ggobi;

  g_signal_connect_object (G_OBJECT(gg), "identify_point",
    G_CALLBACK(identify_cell), G_OBJECT(sheet), 0);
  g_signal_connect_object (G_OBJECT(gg), "move_point",
    G_CALLBACK(move_point_value), G_OBJECT(sheet), 0);
  g_signal_connect_object (G_OBJECT(gg), "brush_motion",
    G_CALLBACK(brush_change), G_OBJECT(sheet), 0);
}


/**
  Loops over the splotd's in the display and register signal handlers
  for this sheet. Perhaps this should do it based on the data in the 
  display being the same as the data in the sheet!
 */
void
connect_to_display(displayd *dpy, GtkWidget *sheet)
{
    GList *el;
    splotd *sp;

    el = dpy->splots;
    while(el) {
	sp = (splotd *) el->data;
	connect_to_splot(sp, sheet);
	el = el->next;
    }
}


/**
  Iterate over the current collection of displays in the ggobi instance
  and within each of those, iterate over the plots and register signal
  handlers for the different GGobi events.
 */
void
connect_to_existing_displays(ggobid *gg, GtkWidget *sheet)
{
    GList *el;
    displayd *dpy;
    el = gg->displays;
    while(el) {
	dpy = (displayd *) el->data;
	connect_to_display(dpy, sheet);
	el = el->next;
    }
}


/**
  Monitor any new plots that are created and register signal callbacks
  for those.
 */ 
void
monitor_new_plot(GtkWidget *w, splotd *sp, ggobid *gg, GtkWidget *sheet)
{
    connect_to_splot(sp, sheet);
}

/**
  Populate the worksheet (given by w) with the contents of the
  given data set (data).
 */
void 
add_ggobi_data(datad *data, GtkTreeModel *model)
{
  gint i, j, k, level;
  gboolean level_ok;
  const gfloat **raw;
  vartabled *vt;
  /*-- for working out the width of the longest row label --*/
  gchar *str;
  
  raw = GGOBI(getRawData)(data, data->gg);
  for(i = 0; i < data->nrows; i++) {
    GtkTreeIter iter;
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	
	str = (gchar *) g_array_index(data->rowlab, gchar*, i);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, str, -1);
	
    for(j = 0; j < data->ncols; j++) {
      vt = g_slist_nth_data(data->vartable, j);
	  /** FIXME: Handle missings */
      if(data->nmissing && data->missing.vals[i][j] && vt->vartype != categorical);
	  //	  gtk_list_store_set(GTK_LIST_STORE(model), &iter, j+1, NAN, -1);
	  else {
         if(vt->vartype == categorical)  {
		  gchar *level_str;
          level_ok = false;
          for (k=0; k<vt->nlevels; k++) {
            if ((gint)raw[i][j] == vt->level_values[k]) {
              level = k;
              level_ok = true;
              break;
            }
          }
          if (level_ok)
			  level_str = vt->level_names[level];
          else level_str = "<improper level>";
		  gtk_list_store_set(GTK_LIST_STORE(model), &iter, j+1, level_str, -1);
         } else {
          gtk_list_store_set(GTK_LIST_STORE(model), &iter, j+1, raw[i][j], -1);
         }
      }
    }
  }
}

/**
 
 */
void close_worksheet_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

/**

 */
void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    g_signal_handlers_disconnect_by_func(G_OBJECT(inst->data),
      G_CALLBACK (close_worksheet_window), inst);
    gtk_widget_destroy((GtkWidget*) inst->data);
  }
}

/**
  Assign a value, typically obtained from the spreadsheet,
  to the associated dataset in GGobi and then update all the
  affected GGobi plots.
 */
void
update_cell(gint row, gint column, double value, datad *data)
{
    data->raw.vals[row][column] = data->tform.vals[row][column] = value;
    tform_to_world (data, data->gg);
    displays_tailpipe (FULL, data->gg); 
}


/**
 Callback for when the contents of a cell have changed.
 This updates the corresponding entry in the ggobi dataset
 and redraws all the plots.
 */
void
cell_changed(GtkCellRendererText *renderer, gchar *path_str, gchar *text, GtkTreeModel *model)
{
	GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
	gint row = gtk_tree_path_get_indices(path)[0];
	GtkTreeIter iter;
	gint col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(renderer), "column"));
	datad *data = (datad *)g_object_get_data(G_OBJECT(model), "data");
	GType type = gtk_tree_model_get_column_type(model, col);
	gdouble value;
	
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	
	if (type == G_TYPE_STRING) { // categorical
		vartabled *vt = g_slist_nth_data(data->vartable, col);
		gchar *old_text;
		gint k;
		for (k = 0; k < vt->nlevels; k++) {
			if (!strcmp(vt->level_names[k], text))
				break;
		}
		value = vt->level_values[k];
		gtk_tree_model_get(model, &iter, col, &old_text, -1);
		g_free(old_text);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, col, text, -1);
	} else {
		value = atof(text);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, col, value, -1);
	}
      
    update_cell(row, col, value, data);
}


/**
  Callback for identifying points in a ggobi plot.
  This identifies the observation/record by scrolling to that value
  and selecting that row.
 */
void
identify_cell(ggobid *gg, splotd *sp, gint id, datad *d, GtkWidget *sheet)
{

	GtkTreePath *path, *child_path;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(sheet));
	
  if(id < 0)
    return;

  child_path = gtk_tree_path_new_from_indices(id, -1);
  path = gtk_tree_model_sort_convert_child_path_to_path(GTK_TREE_MODEL_SORT(model), child_path);
  gtk_tree_path_free(child_path);
  
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(sheet), path, NULL, true, 0.5, 0.5); 
  gtk_tree_selection_select_path(gtk_tree_view_get_selection(GTK_TREE_VIEW(sheet)), path);
  gtk_tree_path_free(path);
}


/**
 Called by ggobi when the user drags a point to change its value. This
 updates the value in the appropriate cell of the worksheet to reflect
 the new value.
 FIXME: This only works with parallel coordinate and xy plots
 */
void
move_point_value(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkWidget *sheet)
{ 
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	
  if(ev->id < 0)
    return;
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(sheet));
  model = gtk_tree_model_sort_get_model(GTK_TREE_MODEL_SORT(model));
  
  path = gtk_tree_path_new_from_indices(ev->id, -1);
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_path_free(path);
  
  if(GGOBI_IS_PAR_COORDS_SPLOT(sp)) {
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, sp->p1dvar+1, 
		sp->displayptr->d->raw.vals[ev->id][sp->p1dvar], -1);
  } else {
	  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
	  	sp->xyvars.x+1, sp->displayptr->d->raw.vals[ev->id][sp->xyvars.x],
		sp->xyvars.y+1, sp->displayptr->d->raw.vals[ev->id][sp->xyvars.y], -1);
  }
}


/**
 Changes the foreground color of the specified row.
 */
void
color_row(GtkWidget *sheet, gint row, gint ncols, GdkColor *col)
{
  GtkTreeIter iter;
  GtkTreePath *path;
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(sheet));
  model = gtk_tree_model_sort_get_model(GTK_TREE_MODEL_SORT(model));
  
  path = gtk_tree_path_new_from_indices(row, -1);
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_path_free(path);
  
  if(col == NULL)
    col = &red;
  
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, ncols+1, col, -1);
}

/**

 Really want the identity of the point that was added or discarded.
 */
void
brush_change(ggobid *gg, splotd *sp, GdkEventMotion *ev, datad *d, GtkWidget *sheet)
{
  /* datad *d = sp->displayptr->d; */
  int nr, i;
  nr = d->npts_under_brush;
  for (i = 0 ; i < d->nrows ; i++) {
    if(d->pts_under_brush.els[i])
      color_row(sheet, i, d->ncols, &red);
    else
      color_row(sheet, i, d->ncols, &black);
  }
}

