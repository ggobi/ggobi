#define GGOBIINTERN

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

void
datad_init (datad *d) {
  d->std_type = 0;

  /*-- initialize arrays to NULL --*/
  arrayf_init (&d->raw);
  arrayf_init (&d->tform1);
  arrayf_init (&d->tform2);
  arrayl_init (&d->world);
  arrayl_init (&d->jitdata);

  arrays_init (&d->missing);
  arrayl_init (&d->missing_world);
  arrayl_init (&d->missing_jitter);

  vectori_init (&d->clusterids);
}

datad *
datad_new (ggobid *gg) {
  datad dd = g_malloc (sizeof (datad));
  datad *d = &dd;

  datad_init (d);
  gg->d = g_slist_append (gg->d, d);
  return (d);
}

void
datad_finalize (datad *d, ggobid *gg)
{
  gint j;
  
  vgroups_sort (d, gg);
  for (j=0; j<d->ncols; j++)
    d->vardata[j].groupid = d->vardata[j].groupid_ori;
}
