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


datad *
datad_new(datad *d, ggobid *gg)
{ 
  if (d == NULL)
    d = (datad *) g_malloc (sizeof (datad));

  d->gg = gg;

  memset(d, 0, sizeof(datad));

  /*-- initialize arrays to NULL --*/
  arrayf_init_null (&d->raw);
  arrayf_init_null (&d->tform);
  arrayl_init_null (&d->world);
  arrayl_init_null (&d->jitdata);

  arrays_init_null (&d->missing);
  arrayl_init_null (&d->missing_world);
  arrayl_init_null (&d->missing_jitter);

  vectori_init_null (&d->clusterid);

  /*-- brushing and linking --*/
  rowids_init_null (d);
  vectorb_init_null (&d->edge.xed_by_brush);

  /*-- linking by categorical variable --*/
  d->linkvar_vt = NULL;

  sphere_init (d);

  jitter_vars_init (d, gg);

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

  if (cleanup) {
    void varpanel_clear (datad *, ggobid *);
    varpanel_clear (d, gg);
  }

  varpanel_checkboxes_populate (d, gg);    /*-- checkboxes --*/
  varcircles_populate (d, gg);  /*-- circles: build but don't show --*/

  rowidv_init (d);  /*-- initialize the rowid vector --*/

  pipeline_init (d, gg);
  clusters_set (d, gg);  /*-- find the clusters for data just read in --*/

  d->nearest_point = -1;

  if (cleanup) {
    display_free_all (gg);  /*-- destroy any existing displays --*/

    /*-- initialize the first display --*/
    display = scatterplot_new (false, NULL, d, gg);
    /* Need to make certain this is the only one there.
       See
    */

    if (display != NULL) {
      gg->displays = g_list_append (gg->displays, (gpointer) display);
      display_set_current (display, gg);
      gg->current_splot = (splotd *)
        g_list_nth_data (gg->current_display->splots, 0);
      display->current_splot = gg->current_splot;

      /*-- turn on event handling in the very first plot --*/
      /*-- ... but will it cause trouble for later plots?  ok so far --*/
      sp_event_handlers_toggle (gg->current_splot, on);
    }
  }

  varpanel_refresh (gg);

#ifdef DATAD_ADDED_SIGNAL_IMPLEMENTED
  /*g_printerr ("emitting datad_added signal\n");*/
  gtk_signal_emit (GTK_OBJECT (gg->main_window), gg->signal_datad_added, gg); 
  /*{
    gboolean rval = false;
    g_printerr ("emitting datad_added signal by name\n");
    gtk_signal_emit_by_name (GTK_OBJECT (gg->main_window), "datad_added", 
      gg, &rval);
  }*/
#endif

  return (display);
}

/*
 * Several tables use notebook widgets to separate the controls
 * corresponding to different datad's.  This is a way to figure
 * out which datad we should be operating on in that case.
*/
datad *
datad_get_from_notebook (GtkWidget *notebook, ggobid *gg) {
  if (g_slist_length (gg->d) == 1) {
    return (datad *) gg->d->data;
  } else {
    GtkNotebook *nb = GTK_NOTEBOOK (notebook);
    gint indx = gtk_notebook_get_current_page (nb);
    return ((datad *) g_slist_nth_data (gg->d, indx));
  }
}

gint
ndatad_with_vars_get (ggobid *gg)
{
  gint nd;
  GSList *l;
  datad *d;

  /*-- silly to do this every time, perhaps, but harmless, I think --*/
  if (g_slist_length (gg->d) > 1) {
    nd = 0;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (g_slist_length (d->vartable) > 0)
        nd++;
    }
  }  else nd = 1;

  return nd;
}

gchar *
datasetName (datad *d, ggobid *gg)
{
  gint which = g_slist_index (gg->d, d);
  gchar *lbl = (gchar *)NULL;

  if (d->name && d->name[0])
    lbl = g_strdup(d->name);
  else
    lbl = g_strdup_printf ("data matrix %d", which);

  return (lbl);
}

