#define GGOBIINTERN

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

datad *
datad_new (ggobid *gg) {
  datad *d = (datad *) g_malloc (sizeof (datad));

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

  gg->d = g_slist_append (gg->d, d);
  return (d);
}

void
datad_free (datad *d, ggobid *gg) {
  arrayf_free (&d->raw, 0, 0);
  pipeline_arrays_free (d, gg);

  if (d->nmissing) {
    arrays_free (&d->missing, 0, 0);
    missing_world_free (d, gg);
  }

  g_free (d);
}

displayd *
datad_init (datad *d, ggobid *gg, gboolean cleanup)
{
  displayd *display = NULL;
  gint j;
  
  vgroups_sort (d, gg);
  for (j=0; j<d->ncols; j++)
    d->vardata[j].groupid = d->vardata[j].groupid_ori;

  /*
   * the varpanel has to know how many circles and labels to destroy
   * if new data is read in later
  */
  d->varpanel_ui.nvars = d->ncols;

  pipeline_init (d, gg);

  if (cleanup) {
    display_free_all (gg);  /*-- destroy any existing displays --*/

    /*-- initialize the first display --*/
    display = scatterplot_new (false, NULL, d, gg);
    /* Need to make certain this is the only one there.
       See
    */
    gg->displays = g_list_append (gg->displays, (gpointer) display);
    display_set_current (display, gg);
    gg->current_splot = (splotd *)
      g_list_nth_data (gg->current_display->splots, 0);
  }

  return (display);
}
