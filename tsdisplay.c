#include "ggobi.h"
#include "tsdisplay.h"

#include "externs.h"

#include "write_state.h" /* for XML_addVariable */

#include <string.h>

displayd *
timeSeriesDisplayCreate(gboolean missing_p, splotd *sp, datad *d, ggobid *gg)
{
  gint *selected_vars, nselected_vars = 0;
  displayd *dpy;

  selected_vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  nselected_vars = selected_cols_get (selected_vars, d, gg);

  dpy = tsplot_new (false, nselected_vars, selected_vars, d, gg);

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
   in tsplot_set
   and avoid looking it up each time. Store it in the displayd object. */
  cpanel_tsplot_set (cpanel, w, gg);
  cpanel_brush_set (cpanel, gg);
  cpanel_identify_set (cpanel, gg);

  return(true);
}

void
tsplotDisplaySet(displayd *dpy, ggobid *gg)
{
  tsplot_mode_menu_make (gg->main_accel_group,
    (GtkSignalFunc) viewmode_set_cb, gg, true);
  gg->viewmode_item = submenu_make ("_ViewMode", 'V',
    gg->main_accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                             gg->tsplot.mode_menu); 
  submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
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
tsplotHandlesAction(displayd *dpy, PipelineMode mode)
{
  return(mode == BRUSH || mode == IDENT || mode == EXTENDED_DISPLAY_MODE);
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
  xmlSetProp(no, "type", "time");
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
tsplotMenusMake(displayd *dpy, PipelineMode viewMode, ggobid *gg)
{
  tsplot_menus_make (gg);
  return(NULL);
}


GtkWidget *
tsplotCPanelWidget(displayd *dpy, gint viewmode, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_tsplot_make(gg);
  }
  *modeName = "TSPlot";
  return(w);
}

gboolean 
tsplotEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, gint viewMode)
{
      if(viewMode != EXTENDED_DISPLAY_MODE)
        return(true);

      xyplot_event_handlers_toggle (sp, state);  /*-- ?? --*/
      return(true);
}


#include <gdk/gdkkeysyms.h>

gint 
tsplotSPlotKeyEventHandler(displayd *dpy, splotd *sp, gint keyval)
{
  gint action = -1;
  switch(keyval) {
    case GDK_v:
    case GDK_V:
       action = EXTENDED_DISPLAY_MODE;
    break;
    default:
  }
  return(action);
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
  cpanel->viewmode = EXTENDED_DISPLAY_MODE;
  cpanel->projection = XYPLOT;  /*-- does it need a projection? --*/

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
tsplot_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
     gg->menus.options_menu);
}
