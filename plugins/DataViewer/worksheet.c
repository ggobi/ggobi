#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"

#include "gtkextra/gtksheet.h"

#include <stdlib.h>
#include "errno.h"

#include "parcoordsClass.h"

#include "GGStructSizes.c"

void       add_ggobi_sheets(ggobid *gg, GtkWidget *notebook);
void       close_worksheet_window(GtkWidget *w, PluginInstance *inst);
GtkWidget* create_ggobi_sheet(datad *data, ggobid *gg);
void       add_ggobi_data(datad *data, GtkWidget *sheet);
GtkWidget *create_ggobi_worksheet_window(ggobid *gg, PluginInstance *inst);

void       show_data_edit_window(PluginInstance *inst, GtkWidget *widget);

GtkWidget* create_ggobi_sheet(datad *data, ggobid *gg);
void update_cell(gint row, gint column, double value, datad *data);
void cell_changed(GtkSheet *sheet, gint row, gint column, datad *data);

void brush_change(GtkWidget *w, ggobid *gg, splotd *sp, GdkEventMotion *ev, GtkSheet *sheet);
void move_point_value(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkSheet *sheet);
void monitor_new_plot(GtkWidget *w, splotd *sp, ggobid *gg, GtkSheet *sheet);
void identify_cell(ggobid *gg, splotd *sp, gint id, datad *d, GtkSheet *sheet);
void color_row(GtkSheet *sheet, gint row, gint ncols, GdkColor *col);

void connect_to_existing_displays(ggobid *gg, GtkSheet *sheet);

static GdkColor red = {-1, 65535, 0, 0};
static GdkColor black;

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
    gint i, k;
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
    gtk_object_get_data (GTK_OBJECT (window), "notebook");

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
  gtk_signal_connect (GTK_OBJECT (okay_btn), "clicked",
    GTK_SIGNAL_FUNC (row_find_by_label), notebook);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area),
    okay_btn);

  /*-- cancel button --*/
  cancel_btn = gtk_button_new_with_label ("Close");
  gtk_signal_connect (GTK_OBJECT (cancel_btn), "clicked",
    GTK_SIGNAL_FUNC (dialog_close), NULL);
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

/**
 Called when the plugin instance is created for a new ggobi instance.
 This adds an entry to the Tools menu that the user can select to 
 bring up the datasheet window.
 */
gboolean
addToMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  extern GtkWidget *GGobi_addToolsMenuItem (gchar *label, ggobid *gg);

  inst->data = NULL;
  inst->info = plugin;

   /* These could be done in an onLoad routine rather than each time 
      we create a new plugin instance of this type. No big deal at all.
    */
  gdk_colormap_alloc_color(gdk_colormap_get_system(), &red, TRUE, TRUE);
  gdk_color_black(gdk_colormap_get_system(), &black);

  entry = GGobi_addToolsMenuItem ("Data grid ...", gg);
  gtk_signal_connect_object (GTK_OBJECT(entry), "activate",
                             GTK_SIGNAL_FUNC (show_data_edit_window),
                             (gpointer) inst);

#ifdef USE_FACTORY
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
show_data_edit_window(PluginInstance *inst, GtkWidget *widget)
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
  GtkWidget *menubar;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggobi data viewer");
  gtk_widget_set_usize(GTK_WIDGET(window), 600, 400);

  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_worksheet_window), inst);


  main_vbox=gtk_vbox_new(FALSE,1);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
  gtk_container_add(GTK_CONTAINER(window), main_vbox);

/* */
  get_main_menu (menubar_items,
    sizeof (menubar_items) / sizeof (menubar_items[0]),
    gtk_accel_group_new(), window,
    &menubar, (gpointer) window);
  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, false, false, 0);
/* */

  notebook=gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
  gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);
  gtk_widget_show(notebook);

  add_ggobi_sheets(gg, notebook);

  gtk_object_set_data (GTK_OBJECT (window), "notebook", notebook);
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
select_row_cb (GtkSheet *sheet, gint row, datad *d)
{
  ggobid *gg = (ggobid *) d->gg;

  if (viewmode_get(gg) != IDENT)
    viewmode_set (IDENT, gg);

  d->nearest_point = row;
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

  sheet = gtk_sheet_new(data->nrows, data->ncols, data->name);  
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);

  gtk_container_add(GTK_CONTAINER(scrolled_window), sheet);

  gtk_widget_show(sheet);

  add_ggobi_data(data, sheet);

  gtk_widget_show(scrolled_window);

  gtk_signal_connect(GTK_OBJECT(sheet), "changed", cell_changed, data);
  gtk_signal_connect(GTK_OBJECT(gg), "splot_new", monitor_new_plot, sheet);

  gtk_signal_connect (GTK_OBJECT(sheet), "select_row", select_row_cb, data);

  connect_to_existing_displays(gg, GTK_SHEET(sheet));

  return(scrolled_window);
}


CHECK_EVENT_SIGNATURE(brush_change, brush_motion_f)
CHECK_EVENT_SIGNATURE(move_point_value, move_point_f)
CHECK_EVENT_SIGNATURE(identify_cell, identify_point_f)

/** 
  Register signal handlers for GGobi events on the particular splot.
 */
void
connect_to_splot(splotd *sp, GtkSheet *sheet)
{
  ggobid *gg = sp->displayptr->ggobi;
  gtk_signal_connect(GTK_OBJECT(gg), "identify_point", identify_cell, sheet);
  gtk_signal_connect(GTK_OBJECT(gg), "move_point", move_point_value, sheet);
  gtk_signal_connect(GTK_OBJECT(gg), "brush_motion", brush_change, sheet);
}


/**
  Loops over the splotd's in the display and register signal handlers
  for this sheet. Perhaps this should do it based on the data in the 
  display being the same as the data in the sheet!
 */
void
connect_to_display(displayd *dpy, GtkSheet *sheet)
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
connect_to_existing_displays(ggobid *gg, GtkSheet *sheet)
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
monitor_new_plot(GtkWidget *w, splotd *sp, ggobid *gg, GtkSheet *sheet)
{
    connect_to_splot(sp, sheet);
}

/**
  Populate the worksheet (given by w) with the contents of the
  given data set (data).
 */
void 
add_ggobi_data(datad *data, GtkWidget *w)
{
  gint i, j;
  GtkSheet *sheet;
  const gfloat **raw;
  vartabled *vt;
  /*-- for working out the width of the longest row label --*/
  gchar *str;
  GtkStyle *style = gtk_widget_get_style (w);
  gint lbearing, rbearing, width, ascent, descent;
  gint maxwidth = 0;

  sheet = GTK_SHEET(w);
  for(i = 0; i < data->ncols; i++) {
    char *name;
    vt = (vartabled*) g_slist_nth_data (data->vartable, i);
    name = vt->collab;
    gtk_sheet_column_button_add_label(sheet, i, name);
    gtk_sheet_set_column_title(sheet, i, name);
  }

  raw = GGOBI(getRawData)(data, data->gg);
  for(i = 0; i < data->nrows; i++) {
    str = (gchar *) g_array_index(data->rowlab, gchar*, i);
    gtk_sheet_row_button_justify (sheet, i, GTK_JUSTIFY_LEFT);

    gtk_sheet_row_button_add_label(sheet, i, str);

    /* keep track of the width of the longest label */
    gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      str, strlen (str),
      &lbearing, &rbearing, &width, &ascent, &descent);
    maxwidth = MAX (width, maxwidth);
    /* */

    for(j = 0; j < data->ncols; j++) {
      char buf[10];
      sprintf(buf, "%.3g", raw[i][j]);
      gtk_sheet_set_cell(sheet, i, j, GTK_JUSTIFY_RIGHT, buf);
    }
  }

  /*-- Does this apply to the row or column titles?  Alas, no.  --*/
  GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_AUTORESIZE);

  gtk_sheet_set_row_titles_width (sheet, maxwidth);
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
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_worksheet_window), inst);
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
cell_changed(GtkSheet *sheet, gint row, gint column, datad *data)
{
    char *val, *tmp;
    ggobid *gg;
    float value;
    if(row < 0)
	return;
    val = gtk_sheet_cell_get_text(sheet, row, column);
    if (val) {
      value = strtod(val, &tmp);
      if(val == tmp) {
        fprintf(stderr, "Error in strtod: %d\n", errno);fflush(stderr);
        return;
      }

#if 1
      update_cell(row, column, value, data);
#endif
  }
}


/**
  Callback for identifying points in a ggobi plot.
  This identifies the observation/record by scrolling to that value
  and selecting that row.
 */
void
identify_cell(ggobid *gg, splotd *sp, gint id, datad *d, GtkSheet *sheet)
{
    if(id < 0)
	return;

    gtk_sheet_moveto(sheet, id, 2, 0.5, 0.5);
    gtk_sheet_select_row(sheet, id);
}


/**
 Called by ggobi when the user drags a point to change its value. This
 updates the value in the appropriate cell of the worksheet to reflect
 the new value.
 */
void
move_point_value(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkSheet *sheet)
{

    int cols[2];
    int n = 2, i;
    char buf[20];
    if(ev->id < 0)
	return;

    if(GTK_IS_GGOBI_PARCOORDS_SPLOT(sp)) {
	    cols[0] = sp->p1dvar;
	    n = 1;
    } else {
	    cols[0] = sp->xyvars.x;
	    cols[1] = sp->xyvars.y;
    }

    for(i = 0; i < n ; i++) {
	sprintf(buf, "%f", sp->displayptr->d->raw.vals[ev->id][cols[i]]);
	gtk_sheet_set_cell(sheet, ev->id, cols[i], GTK_JUSTIFY_CENTER, buf);
    }
}


/**
 Changes the foreground color of the specified row.
 */
void
color_row(GtkSheet *sheet, gint row, gint ncols, GdkColor *col)
{
    GtkSheetRange range;
    range.row0 = row;
    range.col0 = 0;
    range.rowi = row+1;/* row or row+1*/
    range.coli = ncols-1;

    if(col == NULL)
	col = &red;
    gtk_sheet_range_set_foreground(sheet, &range, col);
}

/**

 Really want the identity of the point that was added or discarded.
 */
void
brush_change(GtkWidget *w, ggobid *gg, splotd *sp, GdkEventMotion *ev, GtkSheet *sheet)
{
 datad *d = sp->displayptr->d;
 int nr, i;
      nr = d->npts_under_brush;
      for(i = 0 ; i < d->nrows ; i++) {
	  if(d->pts_under_brush.els[i]) {
	      color_row(sheet, i, d->ncols, &red);
	  } else
	      color_row(sheet, i, d->ncols, &black);

      }
}

