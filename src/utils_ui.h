#ifndef UTILS_UI_H
#define UTILS_UI_H

#include <gtk/gtk.h>
#include "vartable_nbook.h"

typedef const gchar ** (*GGobiVariableNotebookPrefixFunc) (GtkWidget *notebook, GGobiStage *d, gint *sel_prefix, gint *n_prefices);

void       quick_message (const gchar * const, gboolean);

GtkWidget* create_prefixed_variable_notebook (GtkWidget *box, GtkSelectionMode mode, GGobiVariableType vartype, datatyped dtype, GtkSignalFunc func, gpointer func_data, GGobiSession *, GGobiVariableNotebookPrefixFunc prefix_func);

GtkWidget* CreateMenuCheck (GtkWidget *, gchar *, GtkSignalFunc, gpointer, gboolean, GGobiSession *);
GtkWidget* CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *, GtkWidget *, GtkAccelGroup *, GtkSignalFunc, gpointer, GGobiSession *) ;
GtkWidget* CreateMenuItemWithCheck (GtkWidget *, gchar *, gchar *, gchar *, GtkWidget *, GtkAccelGroup *, GtkSignalFunc, gpointer, GGobiSession *, GSList *, gboolean check) ;
GtkWidget* create_menu_bar (GtkUIManager *, const gchar *, GtkWidget *);
void       populate_combo_box (GtkWidget *, gchar **, gint, GCallback, gpointer);
void       populate_tree_view(GtkWidget *tree_view, gchar **lbl, gint nitems, gboolean headers, GtkSelectionMode mode, GCallback func, gpointer obj);

void       variable_notebook_subwindow_add (GGobiStage *d, GtkSignalFunc func, gpointer func_data, GtkWidget *notebook, GGobiVariableType, datatyped, const gchar*, GGobiSession *gg);
void       variable_notebook_varchange_cb (GGobiSession *gg, gint which, GGobiStage *, void *notebook);
void       variable_notebook_handlers_disconnect (GtkWidget *notebook, GGobiSession *gg);
void       variable_notebook_list_changed_cb(GGobiSession *gg, GGobiStage *d, void *notebook);
GtkWidget* get_tree_view_from_notebook (GtkWidget *);
GtkWidget* get_tree_view_from_object (GObject *);
gint  	   get_one_selection_from_tree_view (GtkWidget *tree_view, GGobiStage *d);
gint*      get_selections_from_tree_view (GtkWidget *, gint *);
void	   select_tree_view_row(GtkWidget *tree_view, gint row);
gint	   tree_selection_get_selected_row(GtkTreeSelection *tree_sel);

void ggobi_addToolAction (GtkActionEntry *entry, gpointer *data, GGobiSession *gg);
GtkWidget* ggobi_addToolsMenuItem (gchar *label, GGobiSession *gg);
gboolean   ggobi_addToolsMenuWidget(GtkWidget *entry, GGobiSession *gg);
GGobiStage*     datad_get_from_notebook(GtkWidget *notebook);

#endif
