/* barchartClass.c */


#include "ggobi.h"
#include "barchartDisplay.h"

#include <string.h>

#include "externs.h"


 /* Making these available to ggobiClass.c */
static gboolean barchartVarSel(GtkWidget *w, displayd * display, splotd * sp,
                               gint jvar, gint toggle, gint mouse,
                               cpaneld * cpanel, ggobid * gg);
static gint barchartVarIsPlotted(displayd * dpy, gint * cols, gint ncols,
                                 datad * d);
static gboolean barchartCPanelSet(displayd * dpy, cpaneld * cpanel,
                                  ggobid * gg);
static void barchartDisplaySet(displayd * dpy, ggobid * gg);
static void barchartDestroy(GtkObject *);
static void barchartPlaneToScreen(splotd * sp, datad * d, ggobid * gg);
void barchart_clean_init(barchartSPlotd * sp);
void barchart_recalc_counts(barchartSPlotd * sp, datad * d, ggobid * gg);

static gboolean barchart_build_symbol_vectors(cpaneld *, datad *, ggobid *);
static void barchartVarpanelRefresh(displayd * display, splotd * sp,
                                    datad * d);
static gboolean barchartHandlesAction(displayd * dpy, PipelineMode mode);
static void barchartVarpanelTooltipsSet(displayd * dpy, ggobid * gg,
                                        GtkWidget * wx, GtkWidget * wy,
                                        GtkWidget *wz, GtkWidget * label);
static gint barchartPlottedColsGet(displayd * display, gint * cols,
                                   datad * d, ggobid * gg);
static GtkWidget *barchartCPanelWidget(displayd * dpy, gint viewmode,
                                       gchar ** modeName, ggobid * gg);
static GtkWidget *barchartMenusMake(displayd * dpy, PipelineMode viewMode,
                                    ggobid * gg);
static gboolean barchartEventHandlersToggle(displayd * dpy, splotd * sp,
                                            gboolean state, gint viewMode);
static gint barchartSPlotKeyEventHandler(displayd * dpy, splotd * sp,
                                         gint keyval);



static gint
barchart_is_variable_plotted(displayd * display, gint * cols, gint ncols,
                             datad * d)
{
  gint j;
  ggobid *gg = display->d->gg;
  splotd *sp = gg->current_splot;
  gint jplotted = -1;
  for (j = 0; j < ncols; j++) {
    if (sp->p1dvar == cols[j]) {
      jplotted = sp->p1dvar;
      return jplotted;
    }
  }

  return (-1);
}


/* barchart splot methods*/
static gchar *barchart_tree_label(splotd * sp, datad * d, ggobid * gg)
{
  vartabled *vt;
  int n;
  char *buf;

  vt = vartable_element_get(sp->p1dvar, d);
  n = strlen(vt->collab);
  buf = (gchar *) g_malloc(n * sizeof(gchar *));
  sprintf(buf, "%s", vt->collab);

  return (buf);
}


gboolean
barchartVarSel(GtkWidget *w, displayd * display, splotd * sp, gint jvar,
               gint toggle, gint mouse, cpaneld * cpanel, ggobid * gg)
{
  gint jvar_prev = -1;
  gboolean redraw = p1d_varsel(sp, jvar, &jvar_prev, toggle, mouse);
  if (redraw) {
    displayd *display = (displayd *) sp->displayptr;
    datad *d = display->d;

    barchart_clean_init(GTK_GGOBI_BARCHART_SPLOT(sp));
    barchart_recalc_counts(GTK_GGOBI_BARCHART_SPLOT(sp), d, d->gg);
  }

  return (true);
}

gint
barchartVarIsPlotted(displayd * dpy, gint * cols, gint ncols, datad * d)
{
  int j;
  splotd *sp = (splotd *) dpy->splots->data;
  for (j = 0; j < ncols; j++) {
    if (sp->p1dvar == cols[j]) {
      return (sp->p1dvar);
    }
  }

  return (-1);
}

gboolean barchartCPanelSet(displayd * dpy, cpaneld * cpanel, ggobid * gg)
{
  GtkWidget *w;
  w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if (!w) {
    GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =
        cpanel_barchart_make(gg);
  }
  cpanel_barchart_set(dpy, cpanel, w, gg);
  cpanel_brush_set(dpy, cpanel, gg);
  cpanel_identify_set(dpy, cpanel, gg);

  return (true);
}

void barchartDisplaySet(displayd * dpy, ggobid * gg)
{
  GtkWidget *menu;
  menu = barchart_mode_menu_make(gg->main_accel_group,
                                 (GtkSignalFunc) viewmode_set_cb, gg,
                                 true);
  gg->viewmode_item = submenu_make("_ViewMode", 'V', gg->main_accel_group);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(gg->viewmode_item), menu);
  submenu_insert(gg->viewmode_item, gg->main_menubar, 2);
}


/* This is for the barchart SPlot Class */
static void barchartDestroy(GtkObject *obj)
{
  barchartSPlotd * sp;
  GtkObjectClass *klass;

#ifdef GTK_2_0
  klass = g_type_class_peek_parent(GTK_GGOBI_EXTENDED_SPLOT_CLASS(obj));
#else
  klass = gtk_type_parent_class(GTK_TYPE_GGOBI_EXTENDED_SPLOT);
#endif

  sp = GTK_GGOBI_BARCHART_SPLOT(obj);

  if(sp && sp->bar) {
	  GtkObjectClass *klass;
                /* Goal here is to get the class object for the parent
                   of the GTK_TYPE_GGOBI_EXTENDED_SPLOT class so that we can call its
                   destroy method. */
#ifdef GTK_2_0
               /* Need to get the class of the barchart and then constrain it to the 
                  extended splot class. */
	  klass = g_type_class_peek_parent(GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(obj)));
#else
	  klass = gtk_type_parent_class(GTK_TYPE_GGOBI_EXTENDED_SPLOT);
#endif


	  barchart_free_structure(sp);
	  vectori_free (&sp->bar->index_to_rank);
	  g_free((gpointer) sp->bar);
	  sp->bar = NULL;

	  klass->destroy(GTK_OBJECT(sp)); 
  }

}


void barchartPlaneToScreen(splotd * sp, datad * d, ggobid * gg)
{
  barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);

  barchart_recalc_dimensions(sp, d, gg);
  barchart_recalc_group_dimensions(bsp, gg);
}

void barchartWorldToPlane (splotd *sp, datad *d, ggobid *gg)
{
  barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);

  barchart_clean_init(bsp);
  barchart_recalc_counts(bsp, d, gg);
}


/*----------------------------------------------------------------------*/
/*      local helper function for barcharts,                            */
/*      called by build_symbol_vectors                                  */
/*----------------------------------------------------------------------*/

gboolean barchart_build_symbol_vectors (cpaneld *cpanel, datad * d, ggobid * gg)
{
  gboolean changed = FALSE;
  gint j, m;
  gint nd = g_slist_length (gg->d);

  for (j = 0; j < d->nrows_in_plot; j++) {
    m = d->rows_in_plot.els[j];

    switch (cpanel->br.point_targets) {
      case br_candg:  /*-- color and glyph --*/
        changed = update_color_vectors(m, changed,
          d->pts_under_brush.els, d, gg);
        changed = update_glyph_vectors (m, changed,
          d->pts_under_brush.els, d, gg);
      break;
      case br_color:
        changed = update_color_vectors(m, changed,
          d->pts_under_brush.els, d, gg);
      break;
      case br_glyph:  /*-- glyph type and size --*/
        changed = update_glyph_vectors (m, changed,
          d->pts_under_brush.els, d, gg);
      break;
      case br_hide:
        changed = update_hidden_vectors (m, changed,
          d->pts_under_brush.els, d, gg);
      break;
      /* disabled
      case br_select:
        changed = update_selected_vectors (m, changed,
          d->pts_under_brush.els, d, gg);
      break;
      */
      case br_off:
        ;
      break;
    }

    /*-- link by id --*/
    if (!gg->linkby_cv && nd > 1) symbol_link_by_id (false, j, d, gg);
    /*-- --*/
  }

  return changed;
}

void barchartVarpanelRefresh(displayd * display, splotd * sp, datad * d)
{
  gint j;
  for (j = 0; j < d->ncols; j++) {
    varpanel_toggle_set_active(VARSEL_X, j, (j == sp->p1dvar), d);

    varpanel_toggle_set_active(VARSEL_Y, j, false, d);
    varpanel_widget_set_visible(VARSEL_Y, j, false, d);
    varpanel_toggle_set_active(VARSEL_Z, j, false, d);
    varpanel_widget_set_visible(VARSEL_Z, j, false, d);
  }
}

gboolean barchartHandlesAction(displayd * dpy, PipelineMode mode)
{
  return (mode == SCALE || mode == BRUSH || mode == IDENT
          || mode == EXTENDED_DISPLAY_MODE);
}



/*--------------------------------------------------------------------*/
/*                      Barchart: Options menu                        */
/*--------------------------------------------------------------------*/

static void barchart_menus_make(ggobid * gg)
{
  gg->menus.options_menu = gtk_menu_new();

  CreateMenuCheck(gg->menus.options_menu, "Show tooltips",
                  GTK_SIGNAL_FUNC(tooltips_show_cb), NULL,
                  GTK_TOOLTIPS(gg->tips)->enabled, gg);

  CreateMenuCheck(gg->menus.options_menu, "Show control panel",
                  GTK_SIGNAL_FUNC(cpanel_show_cb), NULL,
                  GTK_WIDGET_VISIBLE(gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(gg->menus.options_item),
                            gg->menus.options_menu);
}


void
barchartVarpanelTooltipsSet(displayd * dpy, ggobid * gg, GtkWidget * wx,
                            GtkWidget * wy, GtkWidget *wz, GtkWidget * label)
{
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), wx,
                       "Click to replace a variable", NULL);
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), label,
                       "Click to replace a variable", NULL);
}


gint
barchartPlottedColsGet(displayd * display, gint * cols, datad * d,
                       ggobid * gg)
{
  gint ncols = 0;
  cols[ncols++] = gg->current_splot->p1dvar;
  return (ncols);
}


GtkWidget *barchartMenusMake(displayd * dpy, PipelineMode viewMode,
                             ggobid * gg)
{
  barchart_menus_make(gg);
  return (NULL);
}


GtkWidget *barchartCPanelWidget(displayd * dpy, gint viewmode,
                                gchar ** modeName, ggobid * gg)
{
  GtkWidget *w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if (!w) {
    GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =
        cpanel_barchart_make(gg);
  }
  *modeName = "Bar Chart";
  return (w);
}


gboolean
barchartEventHandlersToggle(displayd * dpy, splotd * sp, gboolean state,
                            gint viewMode)
{
  if (viewMode == SCALE) {
    barchart_scale_event_handlers_toggle(sp, state);
    return (true);
  }

  if (viewMode != EXTENDED_DISPLAY_MODE && viewMode != SCALE)
    return (true);

  barchart_event_handlers_toggle(sp, state);
  return (true);
}

#include <gdk/gdkkeysyms.h>

gint barchartSPlotKeyEventHandler(displayd * dpy, splotd * sp, gint keyval)
{
  gint action = -1;
  switch (keyval) {
    case GDK_h:
    case GDK_H:
      action = EXTENDED_DISPLAY_MODE;
    break;
    default:
    break;
  }
  return (action);
}


void barchartDisplayClassInit(GtkGGobiBarChartDisplayClass * klass)
{
  klass->parent_class.treeLabel = klass->parent_class.titleLabel =
      "Barchart";
  klass->parent_class.create = barchart_new;
  klass->parent_class.createWithVars = barchart_new_with_vars;
  klass->parent_class.variable_select = barchartVarSel;
  klass->parent_class.variable_plotted_p = barchartVarIsPlotted;
  klass->parent_class.cpanel_set = barchartCPanelSet;
  klass->parent_class.display_unset = NULL;
  klass->parent_class.display_set = barchartDisplaySet;
  klass->parent_class.variable_plotted_p = barchart_is_variable_plotted;

  klass->parent_class.build_symbol_vectors = barchart_build_symbol_vectors;

  klass->parent_class.ruler_ranges_set = ruler_ranges_set;

  klass->parent_class.varpanel_refresh = barchartVarpanelRefresh;

  klass->parent_class.handles_action = barchartHandlesAction;

  klass->parent_class.xml_describe = NULL;

  klass->parent_class.varpanel_tooltips_set = barchartVarpanelTooltipsSet;

  klass->parent_class.plotted_vars_get = barchartPlottedColsGet;

  klass->parent_class.menus_make = barchartMenusMake;

  klass->parent_class.viewmode_control_box = barchartCPanelWidget;

  klass->parent_class.allow_reorientation = false;

  klass->parent_class.binning_ok = false;
  klass->parent_class.event_handlers_toggle = barchartEventHandlersToggle;
  klass->parent_class.splot_key_event_handler =
      barchartSPlotKeyEventHandler;
}

void barchartSPlotClassInit(GtkGGobiBarChartSPlotClass * klass)
{
  /* barcharts need more attention than redrawing the brush */
  klass->extendedSPlotClass.splot.redraw = FULL;
  klass->extendedSPlotClass.tree_label = barchart_tree_label;

  klass->extendedSPlotClass.identify_notify = barchart_identify_bars;
  klass->extendedSPlotClass.add_markup_cues = barchart_add_bar_cues;
  klass->extendedSPlotClass.add_scaling_cues =
      barchart_scaling_visual_cues_draw;
  klass->extendedSPlotClass.add_plot_labels =
      barchart_splot_add_plot_labels;
  klass->extendedSPlotClass.redraw = barchart_redraw;

/*
  klass->extendedSPlotClass.world_to_plane = barchart_recalc_dimensions;
*/
  klass->extendedSPlotClass.world_to_plane = barchartWorldToPlane;
  klass->extendedSPlotClass.plane_to_screen = barchartPlaneToScreen;

  klass->extendedSPlotClass.active_paint_points =
      barchart_active_paint_points;

  GTK_OBJECT_CLASS(klass)->destroy = barchartDestroy;

  klass->extendedSPlotClass.plotted_vars_get = splot1DVariablesGet;
}
