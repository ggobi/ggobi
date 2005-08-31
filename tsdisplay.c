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
  w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
    GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_tsplot_make(gg);
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
  GtkWidget *imode_menu;

  imode_menu = tsplot_imode_menu_make (gg->imode_accel_group,
    (GtkSignalFunc) imode_set_cb, gg, true);
  gg->imode_item = submenu_make ("_Interaction", 'I', gg->main_accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->imode_item),
    imode_menu); 
  submenu_insert (gg->imode_item, gg->main_menubar, 2);
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
    "Select to replace/insert/append/delete a Y variable.",
    NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
    "Click left to replace the horizontal (time) variable.  Click middle or right to replace/insert/append/delete a Y variable.",
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
tsplotMenusMake(displayd *dpy, ggobid *gg)
{
  tsplot_menus_make (dpy, gg);
  return(NULL);
}


GtkWidget *
tsplotCPanelWidget(displayd *dpy, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_tsplot_make(gg);
  }
  *modeName = "TSPlot";
  return(w);
}

gboolean 
tsplotEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, ProjectionMode pmode, InteractionMode imode)
{
  if (imode != DEFAULT_IMODE)
    return(true);

  xyplot_event_handlers_toggle (sp, state);  /*-- ?? --*/
    return(true);
}


#include <gdk/gdkkeysyms.h>

gboolean
tsplotKeyEventHandled(GtkWidget *w, displayd *display, splotd *sp, GdkEventKey *event, ggobid *gg)
{
  gboolean ok = true;
  ProjectionMode pmode = NULL_PMODE;
  InteractionMode imode = DEFAULT_IMODE;

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

  cpanel->tsplot_selection_mode = VAR_REPLACE;  /*-- only this is used --*/
  cpanel->tsplot_arrangement = ARRANGE_COL;

  /*-- available modes --*/
  cpanel_brush_init (cpanel, gg);
  cpanel_identify_init (cpanel, gg);
}




/*--------------------------------------------------------------------*/
/*                   Time series: Options menu                        */
/*--------------------------------------------------------------------*/

void
tsplot_menus_make (displayd *display, ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->imode_frame), gg);

  CreateMenuCheck (gg->menus.options_menu, "Show status bar",
    GTK_SIGNAL_FUNC (statusbar_show_cb), NULL,
    gg->statusbar_p, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
     gg->menus.options_menu);
}
