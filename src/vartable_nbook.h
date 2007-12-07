#ifndef VARTABLE_NBOOK_H
#define VARTABLE_NBOOK_H


#include <gtk/gtk.h>
#include "session.h"
#include "ggobi-stage.h"
#include "ggobi-variable.h"

typedef enum {no_edgesets, edgesets_only, all_datatypes} datatyped;

GtkWidget* create_variable_notebook (GtkWidget *box, GtkSelectionMode mode, GGobiVariableType vartype, datatyped dtype, GtkSignalFunc func, gpointer func_data, GGobiSession *);

GtkWidget*  vartable_tree_view_get (GGobiSession *gg);
void       vartable_alloc (GGobiStage *);
gint       vartable_index_get_by_name(gchar *name, GGobiStage *d);
gboolean   vartable_iter_from_varno(gint var, GGobiStage *d, GtkTreeModel **model, GtkTreeIter *iter);
gint	   vartable_varno_from_path(GtkTreeModel *model, GtkTreePath *path);
void       vartable_cells_set_by_var (gint j, GGobiStage *d);
void       vartable_collab_set_by_var (GGobiStage *, guint);
void       vartable_collab_tform_set_by_var (GGobiStage *s, guint j);
GGobiVariable *vartable_copy_var (GGobiVariable *var, GGobiVariable *var_to);
GGobiVariable* vartable_element_new (GGobiStage *d);
void       vartable_element_remove (gint, GGobiStage *);
void       vartable_init (GGobiStage *d);
void       vartable_limits_set (GGobiStage *);
void       vartable_limits_set_by_var (GGobiStage *s, guint j);
void       vartable_open (GGobiSession *);
void       vartable_row_append (gint j, GGobiStage *);
void       vartable_show_page (GGobiStage*, GGobiSession*);
void       vartable_stats_set (GGobiStage *);
void       vartable_stats_set_by_var (GGobiStage *, guint j);

#endif
