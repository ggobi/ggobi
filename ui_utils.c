#include "ggobi.h"
#include "GGobiAPI.h"

GtkWidget *
GGobi_addDisplayMenuItem(ggobid *gg, const char *label)
{
  GtkWidget *entry;

  GtkItemFactory *factory;
  GtkWidget *dpy_menu = gg->display_menu;
  datad *data;

  entry = gtk_menu_item_new_with_label (label);
  data = GGobi_data_get(0, gg);
  gtk_object_set_data(GTK_OBJECT(entry), "data", (gpointer) data);

  gtk_widget_show (entry);

  /* Add a separator */
  CreateMenuItem (dpy_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  gtk_menu_append (GTK_MENU (dpy_menu), entry);

  return(entry);
}
