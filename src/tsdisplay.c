/* tsdisplay.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site,
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
 *
 * Contributing author of time series code:  Nicholas Lewin-Koh
*/


#include "ggobi.h"
#include "tsdisplay.h"

#include "externs.h"

#include "write_state.h" /* for XML_addVariable */

#include <string.h>

displayd *
timeSeriesDisplayCreate(gboolean missing_p, splotd *sp, datad *d, ggobid *gg)
{
  gint *selected_vars, nselected_vars = 0;
  displayd *dpy = NULL;

  selected_vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_vars = selected_cols_get (selected_vars, d, gg);

  dpy = tsplot_new (dpy, false, nselected_vars, selected_vars, d, gg);

  g_free(selected_vars);

  return(dpy);
}


gint
tsplotIsVarPlotted(displayd *display, gint *cols, gint ncols, datad *d)
{
  GList *l;
  splotd *sp;
  int j;

  for (l = display->splots; l; l = l->next) {
    sp = (splotd *) l->data;

    for (j=0; j<ncols; j++) {
      if (sp->xyvars.x == cols[j]) {
        return(sp->xyvars.x);
      }
      if (sp->xyvars.y == cols[j]) {
        return(sp->xyvars.y);
      }
    }
  }

  return(-1);
}

gboolean
tsplotCPanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{ 
  GtkWidget *w;
  w = GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
    GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_tsplot_make(gg);
  }
/* Can actually be more efficient here by storing the option menu used
   in tsplot_set and avoid looking it up each time. Store it in the
   displayd object. */
  cpanel_tsplot_set (dpy, cpanel, w, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);

  return(true);
}

void
tsplotDisplaySet(displayd *dpy, ggobid *gg)
{
}

void
tsplotVarpanelRefresh(displayd *display, splotd *sp, datad *d)
{
  gint j;
  GList *l;

  for (j=0; j<d->ncols; j++) {
     varpanel_toggle_set_active (VARSEL_X, j, false, d);
     varpanel_toggle_set_active (VARSEL_Y, j, false, d);
     varpanel_widget_set_visible (VARSEL_Y, j, true, d);
     varpanel_toggle_set_active (VARSEL_Z, j, false, d);
     varpanel_widget_set_visible (VARSEL_Z, j, false, d);
   }

   l = display->splots;
   while (l) {
     j = ((splotd *) l->data)->xyvars.y;
     varpanel_toggle_set_active (VARSEL_Y, j, true, d);
     j = ((splotd *) l->data)->xyvars.x;
     varpanel_toggle_set_active (VARSEL_X, j, true, d);
     l = l->next;
   }
}

gboolean
tsplotHandlesInteraction(displayd *dpy, InteractionMode mode)
{
  return(mode == BRUSH || mode == IDENT || mode == DEFAULT_IMODE);
}

/*
  Write out the variables in a time series plot
  to the current node in the XML tree.
 */
void
add_xml_tsplot_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot;
  xmlNodePtr no;

  plot = (splotd *)plots->data;
  no = XML_addVariable(node, plot->xyvars.x, dpy->d);
  xmlSetProp(no, (xmlChar *) "type", (xmlChar *) "time");
  while(plots) {
    plot = (splotd *)plots->data;
    XML_addVariable(node, plot->xyvars.y, dpy->d);
    plots = plots->next;
  }
}


void
tsplotVarpanelTooltipsSet(displayd *dpy, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label)
{
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
    "Select to replace the horizontal (time) variable.",
    NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
    "Select to delete or append a Y variable; drag plots to reorder.",
    NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
    "Click left to replace the horizontal (time) variable.  Click middle or right to append or delete a Y variable; drag plots to reorder.",
    NULL);
}


gint
tsplotPlottedColsGet(displayd *display, gint *cols, datad *d, ggobid *gg)
{
  GList *l;
  splotd *s;
  gint ncols = 0;

  for (l=display->splots; l; l=l->next) {
    s = (splotd *) l->data;
    if (!array_contains (cols, ncols, s->xyvars.y))
      cols[ncols++] = s->xyvars.y;
  }
  return(ncols);
}

GtkWidget *
tsplotCPanelWidget(displayd *dpy, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_tsplot_make(gg);
  }
  *modeName = "TSPlot";
  return(w);
}

/*********************************************************************/
void
start_timeSeries_drag(GtkWidget *src, GdkDragContext *ctxt, GtkSelectionData *data, guint info, guint time, gpointer udata)
{
   gtk_selection_data_set(data, data->target, 8, (guchar *) src, sizeof(splotd *)); 
}

void
receive_timeSeries_drag(GtkWidget *src, GdkDragContext *context, int x, int y, const GtkSelectionData *data,   unsigned int info, unsigned int event_time, gpointer *udata)
{
  splotd *to = GGOBI_SPLOT(src), *from, *sp;
  displayd *display;
  guint tmp;
  display = to->displayptr;
  GList *l;
  gint k;
  GList *ivars = NULL;
  gint xvar;

  from = GGOBI_SPLOT(gtk_drag_get_source_widget(context));

  if(from->displayptr != display) {
      gg_write_to_statusbar("the source and destination of the parallel coordinate plots are not from the same display.\n", display->ggobi);
      return;
  }

  /* Get the x variable */
  sp = (splotd *) (display->splots)->data;
  xvar = sp->xyvars.x;

  /* Gather a list of indices of the vertically plotted variables */
  l = display->splots;
  while (l) {
    sp = (splotd *) l->data;
    ivars = g_list_append(ivars, GINT_TO_POINTER(sp->xyvars.y));
    l = l->next;
  }

  /* Find the index of the to element */
  k = g_list_index(ivars, GINT_TO_POINTER(to->xyvars.y));
  /* Remove the from element */
  ivars = g_list_remove(ivars, GINT_TO_POINTER(from->xyvars.y));
  /* Insert the from element in the position of the to element */
  ivars = g_list_insert(ivars, GINT_TO_POINTER(from->xyvars.y), k);


  /* Assign them to the existing plots */
  k = 0;
  l = display->splots;
  while (l) {
    sp = (splotd *) l->data;
    sp->xyvars.y = GPOINTER_TO_INT(g_list_nth_data(ivars, k));
    k++;
    l = l->next;
  }
  g_list_free(ivars);

  display_tailpipe (display, FULL, display->ggobi);
  varpanel_refresh (display, display->ggobi);
}

void
timeSeriesPlotDragAndDropEnable(splotd *sp, gboolean active) {
  static GtkTargetEntry target = {"text/plain", GTK_TARGET_SAME_APP, 1001};	
  if (active) {
    gtk_drag_source_set(GTK_WIDGET(sp), GDK_BUTTON1_MASK, &target, 1, 
      GDK_ACTION_COPY);
    g_signal_connect(G_OBJECT(sp), "drag_data_get",  
      G_CALLBACK(start_timeSeries_drag), NULL);
    gtk_drag_dest_set(GTK_WIDGET(sp), GTK_DEST_DEFAULT_ALL /* DROP */,
      &target, 1, GDK_ACTION_COPY /*MOVE*/);
    g_signal_connect(G_OBJECT(sp), "drag_data_received",
      G_CALLBACK(receive_timeSeries_drag), NULL);
  } else {
    g_signal_handlers_disconnect_by_func(G_OBJECT(sp),
      G_CALLBACK(start_timeSeries_drag), NULL);
    g_signal_handlers_disconnect_by_func(G_OBJECT(sp),
      G_CALLBACK(receive_timeSeries_drag), NULL);
    gtk_drag_source_unset(GTK_WIDGET(sp));
    gtk_drag_dest_unset(GTK_WIDGET(sp));
  }
}

void
timeSeriesDragAndDropEnable(displayd *dsp, gboolean active) {
  GList *l;
  for (l = dsp->splots; l; l = l->next) {
    splotd *sp = (splotd *)l->data;
    timeSeriesPlotDragAndDropEnable(sp, active);
  }
}

/*************************************************************************/

gboolean 
tsplotEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, ProjectionMode pmode, InteractionMode imode)
{
  timeSeriesDragAndDropEnable(dpy, false);

  switch (imode) {
  case DEFAULT_IMODE:
    timeSeriesDragAndDropEnable(dpy, true);
    xyplot_event_handlers_toggle (sp, state);  /*-- ?? --*/
  break;
  case BRUSH:
    brush_event_handlers_toggle (sp, state);
  break;
  case IDENT:
    identify_event_handlers_toggle (sp, state);
  break;
  default:
  break;
  }

#if 0
  xyplot_event_handlers_toggle (sp, state);  /*-- ?? --*/
    return(true);
#endif

  return false;
}


#include <gdk/gdkkeysyms.h>

gboolean
tsplotKeyEventHandled(GtkWidget *w, displayd *display, splotd *sp, GdkEventKey *event, ggobid *gg)
{
  gboolean ok = true;
  ProjectionMode pmode = NULL_PMODE;
  InteractionMode imode = DEFAULT_IMODE;

  if (event->state == 0 || event->state == GDK_CONTROL_MASK) {

  switch (event->keyval) {
    case GDK_h:
    case GDK_H:
      pmode = EXTENDED_DISPLAY_PMODE;
    break;

    case GDK_b:
    case GDK_B:
      imode = BRUSH;
    break;
    case GDK_i:
    case GDK_I:
      imode = IDENT;
    break;

    default:
      ok = false;
    break;
  }

  if (ok) {
    GGOBI(full_viewmode_set)(pmode, imode, gg);
  }
  } else { ok = false; }

  return ok;
}


gchar *
tsplot_tree_label(splotd *sp, datad *d, ggobid *gg)
{
    vartabled *vty;
    int n;
    char *buf;

      vty = vartable_element_get (sp->xyvars.y, d);
      n = strlen (vty->collab);
      buf = (gchar*) g_malloc(n* sizeof (gchar*));
      sprintf(buf, "%s", vty->collab);
 
    return(buf);
}

/*************************************************/

void
tsplot_cpanel_init (cpaneld* cpanel, ggobid *gg) 
{
  cpanel->pmode = EXTENDED_DISPLAY_PMODE;
  cpanel->imode = DEFAULT_IMODE;

  /*-- 1d plots --*/
  cpanel->p1d.type = DOTPLOT;
  cpanel_p1d_init (cpanel, gg);

  cpanel->tsplot_arrangement = ARRANGE_COL;

  /*-- available modes --*/
  cpanel_brush_init (cpanel, gg);
  cpanel_identify_init (cpanel, gg);
}


/*--------------------------------------------------------------------*/
/*                   Time series: Options menu                        */
/*--------------------------------------------------------------------*/
