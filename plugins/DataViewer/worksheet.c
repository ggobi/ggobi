#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"

#include "gtkextra/gtksheet.h"

#include <stdlib.h>
#include "errno.h"

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
void identify_cell(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkSheet *sheet);
void color_row(GtkSheet *sheet, gint row, gint ncols, GdkColor *col);

void connect_to_existing_displays(ggobid *gg, GtkSheet *sheet);

static GdkColor red = {-1, 65535, 0, 0};
static GdkColor black;

gboolean
addToMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  extern GtkWidget *GGobi_addToolsMenuItem (const char *label, ggobid *gg);

  inst->data = NULL;
  inst->info = plugin;

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


GtkWidget *
create_ggobi_worksheet_window(ggobid *gg, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox, *notebook;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "ggobi data viewer");
  gtk_widget_set_usize(GTK_WIDGET(window), 600, 400);

  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (close_worksheet_window), inst);


  main_vbox=gtk_vbox_new(FALSE,1);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
  gtk_container_add(GTK_CONTAINER(window), main_vbox);


  notebook=gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
  gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);
  gtk_widget_show(notebook);

  add_ggobi_sheets(gg, notebook);

  gtk_widget_show(main_vbox);
  gtk_widget_show(window);

  return(window);
}

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
  gtk_signal_connect(GTK_OBJECT(gg->main_window), "splot_new", monitor_new_plot, sheet);

  connect_to_existing_displays(gg, GTK_SHEET(sheet));

  color_row(GTK_SHEET(sheet), 2, 3, &red);

  return(scrolled_window);
}

void
connect_to_splot(splotd *sp, GtkSheet *sheet)
{
  gtk_signal_connect(GTK_OBJECT(sp->da), "identify_point", identify_cell, sheet);
  gtk_signal_connect(GTK_OBJECT(sp->da), "move_point", move_point_value, sheet);
  gtk_signal_connect(GTK_OBJECT(sp->da), "brush_motion", brush_change, sheet);
}


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


void
monitor_new_plot(GtkWidget *w, splotd *sp, ggobid *gg, GtkSheet *sheet)
{
    connect_to_splot(sp, sheet);
}


void 
add_ggobi_data(datad *data, GtkWidget *w)
{
  gint i, j;
  GtkSheet *sheet;
  const gfloat **raw;
  vartabled *vt;
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
    gtk_sheet_row_button_add_label(sheet, i,
      (gchar *) g_array_index(data->rowlab, gchar*, i));

    for(j = 0; j < data->ncols; j++) {
      char buf[10];
      sprintf(buf, "%.3g", raw[i][j]);
      gtk_sheet_set_cell(sheet, i, j, GTK_JUSTIFY_RIGHT, buf);
    }
  }
}

void close_worksheet_window(GtkWidget *w, PluginInstance *inst)
{
  inst->data = NULL;
}

void closeWindow(ggobid *gg, PluginInstance *inst)
{
  if(inst->data) {
    gtk_signal_disconnect_by_func(GTK_OBJECT(inst->data),
      GTK_SIGNAL_FUNC (close_worksheet_window), inst);
    gtk_widget_destroy((GtkWidget*) inst->data);
  }
}

void
update_cell(gint row, gint column, double value, datad *data)
{
    data->raw.vals[row][column] = data->tform.vals[row][column] = value;
    tform_to_world (data, data->gg);
    displays_tailpipe (REDISPLAY_ALL, FULL, data->gg); 
}


void
cell_changed(GtkSheet *sheet, gint row, gint column, datad *data)
{
    char *val, *tmp;
    ggobid *gg;
    float value;
    val = gtk_sheet_cell_get_text(sheet, row, column);
    value = strtod(val, &tmp);
    if(val == tmp) {
	fprintf(stderr, "Error in strtod: %d\n", errno);fflush(stderr);
	return;
    }

#if 1
    update_cell(row, column, value, data);
#endif
}

void
identify_cell(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkSheet *sheet)
{
    if(ev->id < 0)
	return;

    gtk_sheet_moveto(sheet, ev->id, 2, 0.5, 0.5);
    gtk_sheet_select_row(sheet, ev->id);
}

void
move_point_value(GtkWidget *w, splotd *sp, GGobiPointMoveEvent *ev, ggobid *gg, GtkSheet *sheet)
{

    int cols[2];
    int n = 2, i;
    char buf[20];
    if(ev->id < 0)
	return;

    switch(sp->displayptr->displaytype) {
	case parcoords:
	    cols[0] = sp->p1dvar;
	    n = 1;
	    break;
	default:
	    cols[0] = sp->xyvars.x;
	    cols[1] = sp->xyvars.y;
    }

    for(i = 0; i < n ; i++) {
	sprintf(buf, "%f", sp->displayptr->d->raw.vals[ev->id][cols[i]]);
	gtk_sheet_set_cell(sheet, ev->id, cols[i], GTK_JUSTIFY_CENTER, buf);
    }
}


void
color_row(GtkSheet *sheet, gint row, gint ncols, GdkColor *col)
{
    GtkSheetRange range;
    range.row0 = row;
    range.col0 = 0;
    range.rowi = row+1;/* row or row+1*/
    range.coli = ncols-1;

    gtk_sheet_range_set_foreground(sheet, &range, col);
}

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

