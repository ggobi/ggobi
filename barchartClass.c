#ifdef BARCHART_IMPLEMENTED

#include "ggobi.h"
#include "barchartDisplay.h"

#include <string.h>

#include "externs.h"


gint
barchart_is_variable_plotted(gint *cols, gint ncols, displayd *display)
{
    int j;
    ggobid *gg = display->d->gg;
    splotd *sp = gg->current_splot;
    int jplotted = -1;
    for (j=0; j<ncols; j++) {
	if (sp->p1dvar == cols[j]) {
            jplotted = sp->p1dvar;
            return jplotted;
	}
    }

    return(-1);
}


/* barchart splot methods*/
gchar *
barchart_tree_label(splotd *sp, datad *d, ggobid *gg)
{
    vartabled *vt;
    int n;
    char *buf;

      vt = vartable_element_get (sp->p1dvar, d);
      n = strlen (vt->collab);
      buf = (gchar*) g_malloc(n* sizeof (gchar*));
      sprintf(buf, "%s", vt->collab);

    return(buf);
}


gboolean
barchartVarSel(displayd *display, splotd *sp, gint jvar, gint btn, cpaneld *cpanel, ggobid *gg)
{
   gint jvar_prev = -1;
   gboolean redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
   if (redraw) {
      displayd *display = (displayd *) sp->displayptr;
      datad *d = display->d;

      barchart_clean_init (GTK_GGOBI_BARCHART_SPLOT(sp));
      barchart_recalc_counts (GTK_GGOBI_BARCHART_SPLOT(sp), d, d->gg);
   }

   return(true);
}

gint
barchartVarIsPlotted(displayd *dpy, gint *cols, gint ncols, datad *d)
{
  int j;
  splotd *sp = (splotd *) dpy->splots->data;
  for (j=0; j<ncols; j++) {
    if (sp->p1dvar == cols[j]) {
        return(sp->p1dvar);
       }
  }

  return(-1);
}

gboolean
barchartCPanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{
      GtkWidget *w;
      w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
      if(!w) {
        GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_barchart_make(gg);
      }
      cpanel_barchart_set (cpanel, w, gg);
      cpanel_brush_set (cpanel, gg);
      cpanel_identify_set (cpanel, gg);

      return(true);
}

void
barchartDisplaySet(displayd *dpy, ggobid *gg)
{
   GtkWidget *menu;
   menu = barchart_mode_menu_make (gg->main_accel_group,
				   (GtkSignalFunc) viewmode_set_cb, gg, true);
   gg->viewmode_item = submenu_make ("_ViewMode", 'V', gg->main_accel_group);
   gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item), menu);
   submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
}


void
barchartDestroy(barchartSPlotd *sp)
{
      barchart_free_structure (sp);
      g_free ((gpointer) sp->bar->index_to_rank);
      g_free ((gpointer) sp->bar);

      {
        GtkObjectClass *klass;
        klass = gtk_type_parent_class(GTK_TYPE_GGOBI_BARCHART_SPLOT);
	klass->destroy(GTK_OBJECT(sp));
      }
/*XX And need to chain */
}

void
barchartPlaneToScreen(splotd *sp, datad *d, ggobid *gg)
{
    barchartSPlotd *bsp =  GTK_GGOBI_BARCHART_SPLOT(sp);
    barchart_recalc_dimensions (sp, d, gg);
    barchart_recalc_group_dimensions (bsp,gg);
}


/*----------------------------------------------------------------------*/
/*      local helper function for barcharts,                            */ 
/*      called by build_symbol_vectors                                  */
/*----------------------------------------------------------------------*/

gboolean 
barchart_build_symbol_vectors (datad *d, ggobid *gg) 
{
  gboolean changed = FALSE;
  gint j,m;

  for (j=0; j<d->nrows_in_plot; j++) {
    m = d->rows_in_plot[j];
    changed = update_color_vectors (m, changed,
              d->pts_under_brush.els, d, gg);
  }

  return changed;
}

void
barchartVarpanelRefresh(displayd *display, splotd *sp, datad *d)
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    varpanel_toggle_set_active (VARSEL_X, j, (j == sp->p1dvar), d);
    varpanel_toggle_set_active (VARSEL_Y, j, false, d);
    varpanel_widget_set_visible (VARSEL_Y, j, false, d);
  }
}

gboolean
barchartHandlesAction(displayd *dpy, PipelineMode mode)
{
  return(mode == SCALE || mode == BRUSH || mode == IDENT || mode == EXTENDED_DISPLAY_MODE);
}



/*--------------------------------------------------------------------*/
/*                      Barchart: Options menu                        */
/*--------------------------------------------------------------------*/

void
barchart_menus_make (ggobid *gg)
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


void
barchartVarpanelTooltipsSet(displayd *dpy, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *label)
{
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
			      "Click to replace a variable",
			      NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
			      "Click to replace a variable",
			      NULL);
}


gint
barchartPlottedColsGet(displayd *display, gint *cols, datad *d, ggobid *gg)
{
   gint ncols = 0;
   cols[ncols++] = gg->current_splot->p1dvar;
   return(ncols);
}


GtkWidget *
barchartMenusMake(displayd *dpy, PipelineMode viewMode, ggobid *gg)
{
      barchart_menus_make (gg);
      return(NULL);
}


GtkWidget *
barchartCPanelWidget(displayd *dpy, gint viewmode, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_barchart_make(gg);
  }
  *modeName = "Bar Chart";
  return(w);
}


gboolean 
barchartEventHandlersToggle(displayd *dpy, splotd *sp, gboolean state, gint viewMode)
{
      if(viewMode != EXTENDED_DISPLAY_MODE && viewMode != SCALE)
        return(true);

      barchart_event_handlers_toggle (sp, state);
      return(true);
}

#include <gdk/gdkkeysyms.h>

gint 
barchartSPlotKeyEventHandler(displayd *dpy, splotd *sp, gint keyval)
{
  gint action = -1;
  switch(keyval) {
    case GDK_h:
    case GDK_H:
      action = EXTENDED_DISPLAY_MODE;
    break;
    default:
  }
  return(action);
}


#endif /* BARCHART_IMPLEMENTED */
