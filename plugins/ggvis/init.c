#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "ggvis.h"

void
ggvis_init (ggvisd *ggv) {
  arrayd_init_null (&ggv->dist_orig);
  arrayd_init_null (&ggv->dist);
  arrayd_init_null (&ggv->pos_orig);
  arrayd_init_null (&ggv->pos);

  ggv->stressplot_pix = NULL;
  ggv->histogram_pix = NULL;
}

