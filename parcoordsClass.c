#include "parcoordsClass.h"

#include <string.h>
#include "write_state.h"
#include "externs.h"


gint
splot1DVariablesGet(splotd *sp, gint *cols, datad *d)
{
	cols[0] = sp->p1dvar;
	return(1);
}

static gboolean
parcoordsBinningPermitted(displayd *dpy)
{
  cpaneld *cpanel = &dpy->cpanel;

  if (cpanel->br_point_targets == br_select)
     return(false);

   return(!dpy->options.whiskers_show_p);
}

static gboolean
cpanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;
  w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if (!w) {
    GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =
      cpanel_parcoords_make(gg);
  }

  cpanel_parcoords_set (cpanel, w, gg);
  cpanel_brush_set (cpanel, gg);
  cpanel_identify_set (cpanel, gg);

  return(true);/* XX */
}

/**
 Instance initialization file.
*/
void
parcoordsDisplayInit(parcoordsDisplayd *display)
{
    GTK_GGOBI_DISPLAY(display)->p1d_orientation = VERTICAL;
}

static gboolean
handlesAction(displayd *dpy, PipelineMode v)
{
   return(v == BRUSH || v == IDENT || v == PCPLOT);
}


gchar *
treeLabel(splotd *splot, datad *d, ggobid *gg)
{
   vartabled *vt;
   int n;
   gchar *buf;
      vt = vartable_element_get (splot->p1dvar, d);
      n = strlen (vt->collab);
      buf = (gchar*) g_malloc(n * sizeof (gchar*));
      sprintf(buf, "%s", vt->collab);
      return(buf);
}

static GdkSegment *
allocWhiskers(GdkSegment *whiskers, splotd *sp, gint nr, datad *d)
{
  return((GdkSegment *) g_realloc (whiskers, 2 * nr * sizeof (GdkSegment)));
}

void
worldToPlane(splotd *sp, datad *d, ggobid *gg)
{
  p1d_reproject (sp, d->world.vals, d, gg);
}


void
withinPlaneToScreen(splotd *sp, displayd *display, datad *d, ggobid *gg)
{
  sp_whiskers_make (sp, display, gg);
}


gboolean
drawEdge_p(splotd *sp, gint m, datad *d, datad *e, ggobid *gg)
{
  return(! e->missing.vals[m][sp->p1dvar]);
}

gboolean
drawCase_p(splotd *sp, gint m, datad *d, ggobid *gg)
{
  return(!d->missing.vals[m][sp->p1dvar]);
}

void
withinDrawBinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
 displayd *display = sp->displayptr;

 if (display->options.whiskers_show_p) {
	 gint n;
	 n = 2*m;
	 gdk_draw_line (drawable, gc,
			sp->whiskers[n].x1, sp->whiskers[n].y1,
			sp->whiskers[n].x2, sp->whiskers[n].y2);
	 n++;
	 gdk_draw_line (drawable, gc,
			sp->whiskers[n].x1, sp->whiskers[n].y1,
			sp->whiskers[n].x2, sp->whiskers[n].y2);
 }
}


void
withinDrawUnbinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
 displayd *display = sp->displayptr;
 if (display->options.whiskers_show_p) {
	 gint n = 2*m;
	 gdk_draw_line (drawable, gc,
			sp->whiskers[n].x1, sp->whiskers[n].y1,
			sp->whiskers[n].x2, sp->whiskers[n].y2);
	 n++;
	 gdk_draw_line (drawable, gc,
			sp->whiskers[n].x1, sp->whiskers[n].y1,
			sp->whiskers[n].x2, sp->whiskers[n].y2);
 }
}


static void
addPlotLabels(displayd *display, splotd *sp, GdkDrawable *drawable, datad *d, ggobid *gg)
{
    vartabled *vt;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style = gtk_widget_get_style (sp->da);
    cpaneld *cpanel = &display->cpanel;

    vt = vartable_element_get (sp->p1dvar, d);
    gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      vt->collab_tform, strlen (vt->collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);

    if (cpanel->parcoords_arrangement == ARRANGE_ROW)
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gg->plot_GC,
        /*-- if the label fits, center it; else, left justify --*/
        (width <= sp->max.x) ?  sp->max.x/2 - width/2 : 0, 
        sp->max.y - 5,
        vt->collab_tform);
     else
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gg->plot_GC, 5, 5+ascent+descent, vt->collab_tform);

}


static gint
plotted(displayd *display, gint *cols, gint ncols, datad *d)
{
	GList *l;
	splotd *sp;
	gint j;
        for (l = display->splots; l; l = l->next) {
          sp = (splotd *) l->data;

          for (j=0; j<ncols; j++) {
            if (sp->xyvars.x == cols[j]) {
              return(sp->xyvars.x);
            }
          }
        }
	return(-1);
}

static gboolean
variableSelect(GtkWidget *w, displayd *dpy, splotd *sp, gint jvar, gint toggle, gint mouse, cpaneld *cpanel, ggobid *gg)
{
  gint jvar_prev = -1;
  return(parcoords_varsel (cpanel, sp, jvar, &jvar_prev, gg));
}


static void
varpanelRefresh(displayd *display, splotd *sp, datad *d)
{
	gint j;
	GList *l;
        for (j=0; j<d->ncols; j++) {
          varpanel_toggle_set_active (VARSEL_X, j, false, d);

          varpanel_toggle_set_active (VARSEL_Y, j, false, d);
          varpanel_widget_set_visible (VARSEL_Y, j, false, d);
          varpanel_toggle_set_active (VARSEL_Z, j, false, d);
          varpanel_widget_set_visible (VARSEL_Z, j, false, d);
        }

        l = display->splots;
        while (l) {
          j = ((splotd *) l->data)->p1dvar;
          varpanel_toggle_set_active (VARSEL_X, j, true, d);
          l = l->next;
        }
}

static void
varpanelTooltipsSet(displayd *dpy, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *xz, GtkWidget *label)
{
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
    "Select to replace/insert/append a variable, or to delete it",
    NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
    "Click to replace/insert/append a variable, or to delete it",
    NULL);
}


static gint
plottedVarsGet(displayd *display, gint *cols, datad *d, ggobid *gg)
{
      GList *l;
      splotd *s;
      gint ncols = 0;
      for (l=display->splots; l; l=l->next) {
        s = (splotd *) l->data;
        if (!array_contains (cols, ncols, s->p1dvar))
          cols[ncols++] = s->p1dvar;
      }
      return(ncols);
}


static void
displaySet(displayd *display, ggobid *gg)
{
        parcoords_mode_menu_make (gg->main_accel_group,
          (GtkSignalFunc) viewmode_set_cb, gg, true);
        gg->viewmode_item = submenu_make ("_ViewMode", 'V',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                                   gg->parcoords.mode_menu); 
        submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
}

/*
  Write out the variables in a parallel coordinates plot
  to the current node in the  XML tree.

Should be able to make this generic.
 */
static void
add_xml_parcoords_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot;
  while(plots) {
    plot = (splotd *) plots->data;
    XML_addVariable(node, plot->p1dvar, dpy->d);
    plots = plots->next;
  }
}


/*------------------------------------------------------------------------*/
/*               case highlighting for points (and edges?)                */
/*------------------------------------------------------------------------*/

/*-- add highlighting for parallel coordinates plot --*/
static void
splot_add_whisker_cues (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint n;
  displayd *display = sp->displayptr;
  datad *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;

  if (k < 0 || k >= d->nrows) return;

  if (display->options.whiskers_show_p) {
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[ d->color_now.els[k] ]);

    n = 2*k;
    gdk_draw_line (drawable, gg->plot_GC,
      sp->whiskers[n].x1, sp->whiskers[n].y1,
      sp->whiskers[n].x2, sp->whiskers[n].y2);
    n++;
    gdk_draw_line (drawable, gg->plot_GC,
      sp->whiskers[n].x1, sp->whiskers[n].y1,
      sp->whiskers[n].x2, sp->whiskers[n].y2);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}


static GtkWidget *
parcoordsCPanelWidget(displayd *dpy, gint viewmode, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w = cpanel_parcoords_make(gg);
  }
  *modeName = "Parcoords";
  return(w);
}

void
parcoordsDisplayClassInit(GtkGGobiParCoordsDisplayClass *klass)
{
  klass->parent_class.loop_over_points = true;
  klass->parent_class.binningPermitted = parcoordsBinningPermitted;

  klass->parent_class.allow_reorientation = true;
  klass->parent_class.options_menu_p = true;

  klass->parent_class.loop_over_points = true;

  klass->parent_class.titleLabel = "parallel coordinates display";
  klass->parent_class.treeLabel = "Parallel Coordinates";

  /* No create method, just createWIthVars. */
  klass->parent_class.createWithVars = parcoords_new_with_vars;

  klass->parent_class.variable_select = variableSelect;
  klass->parent_class.variable_plotted_p = plotted;

  /* no unset */
  klass->parent_class.display_set = displaySet;

  /* no build_symbol_vectors */

  /* ruler ranges set. */

  klass->parent_class.varpanel_refresh = varpanelRefresh;
  
  klass->parent_class.handles_action = handlesAction;

  klass->parent_class.cpanel_set = cpanelSet;

  klass->parent_class.xml_describe = add_xml_parcoords_variables;

  klass->parent_class.varpanel_tooltips_set = varpanelTooltipsSet;

  klass->parent_class.plotted_vars_get = plottedVarsGet;

  klass->parent_class.viewmode_control_box = parcoordsCPanelWidget;

/* menus_make */

/* event_handlers_toggle */

/* splot_key_event_handler */

  klass->parent_class.add_plot_labels = addPlotLabels;
}


void
parcoordsSPlotClassInit(GtkGGobiParCoordsSPlotClass *klass)
{
   klass->parent_class.alloc_whiskers = allocWhiskers;
   klass->parent_class.add_identify_cues = splot_add_whisker_cues;

   klass->parent_class.tree_label = treeLabel;

   klass->parent_class.world_to_plane = worldToPlane;
   klass->parent_class.sub_plane_to_screen = withinPlaneToScreen;

   klass->parent_class.draw_edge_p = drawEdge_p;
   klass->parent_class.draw_case_p = drawCase_p;

   klass->parent_class.within_draw_to_binned = withinDrawBinned;
   klass->parent_class.within_draw_to_unbinned = withinDrawUnbinned;

   klass->parent_class.plotted_vars_get = splot1DVariablesGet;
}


splotd *
gtk_parcoords_splot_new(displayd *dpy, gint width, gint height, ggobid *gg)
{
   splotd *sp = gtk_type_new(GTK_TYPE_GGOBI_PARCOORDS_SPLOT);
   splot_init(sp, dpy, width, height, gg);
   return(sp);
}
