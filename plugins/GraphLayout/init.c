#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "plugin.h"

#include "glayout.h"

void
glayout_init (glayoutd *gl) {

  arrayd_init_null (&gl->dist);
  arrayd_init_null (&gl->pos);

  gl->e = NULL;
  gl->dsrc = NULL;

  gl->centerNodeIndex = -1;
  gl->radialAutoUpdate = false;
  gl->radialNewData = true;
  gl->radial = NULL;

  gl->neato_dim = 2;
  gl->neato_model = neato_shortest_path;
  gl->neato_use_edge_length_p = false;
}
