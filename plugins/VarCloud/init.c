#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "varcloud.h"

void
vcl_init (vcld *vcl, ggobid *gg)
{

  /*-- initialize the datad pointers --*/
  vcl->dsrc = gg->d->data;
}
