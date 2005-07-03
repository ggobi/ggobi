#ifndef GGOBI_APP_H
#define GGOBI_APP_H


#include <gtk/gtk.h>

extern GtkType gtk_ggobi_app_get_type(void);

#define GTK_TYPE_GGOBI_APP	      (gtk_ggobi_app_get_type ())
#define GTK_GGOBI_APP(obj)	      (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_APP, GtkGGobiApp))
#define GTK_GGOBI_APP_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_APP, GtkGGobiAppClass))
#define GTK_IS_GGOBI_APP(obj)	      (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_APP))
#define GTK_IS_GGOBI_APP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_APP))


/* Create the GtkGGobiApp class that is derived from GtkObject. */
typedef struct {

   GtkObjectClass parent_class;

} GtkGGobiAppClass;


typedef struct {
  GtkObject object;
  /* Session info options would be appropriate here. 
     Also, all the global variables such as all_ggobis, num_ggobis, totalNumGGobis.
   */
} GtkGGobiApp;


#endif /* GGOBI_APP_H */
