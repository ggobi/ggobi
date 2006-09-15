#ifndef GGOBI_APP_H
#define GGOBI_APP_H


#include <gtk/gtk.h>

extern GType ggobi_app_get_type(void);

#define GGOBI_TYPE_APP	      			(ggobi_app_get_type ())
#define GGOBI_APP(obj)	      			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GGOBI_TYPE_APP, GGobiApp))
#define GGOBI_APP_CLASS(klass)    		(G_TYPE_CHECK_CLASS_CAST ((klass), GGOBI_TYPE_APP, GGobiAppClass))
#define GGOBI_IS_APP(obj)	      		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGOBI_TYPE_APP))
#define GGOBI_IS_APP_CLASS(klass) 		(G_TYPE_CHECK_CLASS_TYPE ((klass), GGOBI_TYPE_APP))
#define GGOBI_APP_GET_CLASS(obj)  		(G_TYPE_INSTANCE_GET_CLASS ((obj), GGOBI_TYPE_APP, GGobiAppClass))



/* Create the GGobiApp class that is derived from GObject. */
typedef struct {
   GObjectClass parent_class;
} GGobiAppClass;


typedef struct {
  GObject object;
  /* Session info options would be appropriate here. 
     Also, all the global variables such as all_ggobis, num_ggobis, totalNumGGobis.
   */
} GGobiApp;

#endif /* GGOBI_APP_H */
