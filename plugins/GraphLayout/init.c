#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"

#include "glayout.h"

void
glayout_init (glayoutd *gl) {

  arrayd_init_null (&gl->dist_orig);
  arrayd_init_null (&gl->dist);
  arrayd_init_null (&gl->pos_orig);
  arrayd_init_null (&gl->pos);

  gl->radial = NULL;
#ifdef GRAPHVIZ
  gl->graphviz = NULL;
#endif
}
