/*-- datad.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#define GGOBIINTERN

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include <string.h> /* for memset() declaration */

#ifdef USE_CLASSES
datad::datad(ggobid *gg) 
{
  datad_new(this, gg);
}


datad *
datad_new (ggobid *gg) 
{
  datad *d = (datad *) g_malloc (sizeof (datad));
  return(datad_new(d, gg));
}
#endif

datad *
datad_new(datad *d, ggobid *gg)
{ 
  if(d == NULL)
    d = (datad *) g_malloc (sizeof (datad));

  memset(d, 0, sizeof(datad));

  d->std_type = 0;

  /*-- initialize arrays to NULL --*/
  arrayf_init (&d->raw);
  arrayf_init (&d->tform);
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

  varpanel_populate (d, gg);

  vgroups_sort (d, gg);
  for (j=0; j<d->ncols; j++)
    d->vartable[j].groupid = d->vartable[j].groupid_ori;

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

    varpanel_refresh (gg);
  }

  return (display);
}

/*
 * Several tables use notebook widgets to separate the controls
 * corresponding to different datad's.  This is a way to figure
 * out which datad we should be operating on in that case.
*/
datad *
datad_get_from_notebook (GtkWidget *notebook, ggobid *gg) {
  GtkNotebook *nb = GTK_NOTEBOOK (notebook);
  gint indx = gtk_notebook_get_current_page (nb);
  return ((datad *) g_slist_nth_data (gg->d, indx));
}
