#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>

guint      *find_keepers (gint ncols_current, GSList *cols, guint *nkeepers);
gint       sqdist (gint, gint, gint, gint);
gdouble    random_normal();
gboolean   is_numeric (const gchar * str);
gdouble    as_number (const char *sval);
gboolean   as_logical (const gchar * sval);
gint       fcompare (const void *x1, const void *x2);
gint       pcompare (const void *, const void *);
gint       rank_compare (gconstpointer, gconstpointer, gpointer);
GList*     g_list_remove_nth (GList *, gint);
GList*     g_list_replace_nth (GList *, gpointer, gint);
gboolean   widget_initialized (GtkWidget *w);
void       widget_initialize (GtkWidget *w, gboolean initd);
GtkWidget* widget_find_by_name (GtkWidget *, gchar *);
void       ggobi_sleep(guint);
gboolean   in_vector (gint k, gint *vec, gint nels);

#endif
