#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"

#include "gtkextra/gtksheet.h"
#if 0
#include "pixmaps.h"
#endif

void       add_ggobi_sheets(ggobid *gg, GtkWidget *notebook);
void       close_worksheet_window(GtkWidget *w, PluginInstance *inst);
GtkWidget* create_ggobi_sheet(datad *data, ggobid *gg);
void       add_ggobi_data(datad *data, GtkWidget *sheet);
GtkWidget *create_ggobi_worksheet_window(ggobid *gg, PluginInstance *inst);

void       show_data_edit_window(PluginInstance *inst, GtkWidget *widget);


gboolean
addToMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *menu, *entry, *data_item;

  inst->data = NULL;
  inst->info = plugin;

  menu = gtk_menu_new();
  entry = gtk_menu_item_new_with_label("View");

  gtk_menu_append (GTK_MENU (menu), entry);

  gtk_signal_connect_object (GTK_OBJECT(entry), "activate",
                             GTK_SIGNAL_FUNC (show_data_edit_window),
                             (gpointer) inst);
  gtk_widget_show(entry);

/*
  data_item = gtk_menu_item_new_with_label("Data");
  gtk_widget_show(data_item);
  gtk_widget_add_accelerator (data_item, "activate",
    gg->main_accel_group, (guint) 'A',
    GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
*/
  data_item = submenu_make ("D_ata", 'A', gg->main_accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (data_item), menu);


/*
  We can append to the menubar but this puts the Data entry after
  the Help, which is now what we want. Instead, we actually
  compute the number of elements in the menubar and insert it
  just before the last one.

  gtk_menu_bar_append (GTK_MENU_BAR (gg->main_menubar), data_item);
*/
  {
    gint n;  
    GList *children = gtk_container_children(GTK_CONTAINER(gg->main_menubar));
    n = g_list_length(children);
    gtk_menu_bar_insert(GTK_MENU_BAR (gg->main_menubar), data_item, n-1);
  }


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

   label = gtk_label_new(data->name);
   sheet = create_ggobi_sheet(data, gg);
   gtk_notebook_append_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(sheet), label);
   
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

  return(scrolled_window);
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
