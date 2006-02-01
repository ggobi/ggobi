/* parcoordsClass.c */
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
*/

#include "parcoordsClass.h"

#include <string.h>
#include "write_state.h"
#include "externs.h"
#include <gdk/gdkkeysyms.h>

static gboolean parcoordsKeyEventHandled(GtkWidget *, displayd *, splotd *,
					GdkEventKey *, ggobid *);

gint
splot1DVariablesGet(splotd *sp, gint *cols, datad *d)
{
	cols[0] = sp->p1dvar;
	return(1);
}

static gboolean
parcoordsBinningPermitted(displayd *dpy)
{
  /*
  cpaneld *cpanel = &dpy->cpanel;
  if (cpanel->br_point_targets == br_select)
     return(false);
  */
   return(!dpy->options.whiskers_show_p);
}

static gboolean
cpanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;
  w = GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if (!w) {
    GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =
      cpanel_parcoords_make(gg);
  }

  cpanel_parcoords_set (dpy, cpanel, w, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);

  return(true);/* XX */
}

/**
 Instance initialization file.
*/
void
parcoordsDisplayInit(parcoordsDisplayd *display)
{
    GGOBI_DISPLAY(display)->p1d_orientation = VERTICAL;
}

static gboolean
handlesInteraction(displayd *dpy, InteractionMode v)
{
   return(v == BRUSH || v == IDENT || v == DEFAULT_IMODE);
}

void
start_parcoords_drag(GtkWidget *src, GdkDragContext *ctxt, GtkSelectionData *data, guint info, guint time, gpointer udata)
{
   gtk_selection_data_set(data, data->target, 8, (guchar *) src, sizeof(splotd *)); 
}

void
receive_parcoords_drag(GtkWidget *src, GdkDragContext *context, int x, int y, const GtkSelectionData *data,   unsigned int info, unsigned int event_time, gpointer *udata)
{
   splotd *to = GGOBI_SPLOT(src);
   splotd *from;
   displayd *display;
   guint tmp;
   display = to->displayptr;

#if 0
   from = (splotd *) data->data;/*XXX Want a proper, robust and portable way to do this. */
#endif

   from = GGOBI_SPLOT(gtk_drag_get_source_widget(context));

   if(from->displayptr != display) {
      gg_write_to_statusbar("the source and destination of the parallel coordinate plots are not from the same display.\n", display->ggobi);
      return;
   }

   tmp = to->p1dvar;
   to->p1dvar = from->p1dvar;
   from->p1dvar = tmp;

   display_tailpipe (display, FULL, display->ggobi);
   varpanel_refresh (display, display->ggobi);

}

void
parcoordsPlotDragAndDropEnable(splotd *sp, gboolean active) {
	static GtkTargetEntry target = {"text/plain", GTK_TARGET_SAME_APP, 1001};	
	if (active) {
		gtk_drag_source_set(GTK_WIDGET(sp), GDK_BUTTON1_MASK, &target, 1, GDK_ACTION_COPY);
		g_signal_connect(G_OBJECT(sp), "drag_data_get",  G_CALLBACK(start_parcoords_drag), NULL);
		gtk_drag_dest_set(GTK_WIDGET(sp), GTK_DEST_DEFAULT_ALL /* DROP */ , &target, 1, GDK_ACTION_COPY /*MOVE*/);
		g_signal_connect(G_OBJECT(sp), "drag_data_received",  G_CALLBACK(receive_parcoords_drag), NULL);
	} else {
		g_signal_handlers_disconnect_by_func(G_OBJECT(sp), G_CALLBACK(start_parcoords_drag), NULL);
		g_signal_handlers_disconnect_by_func(G_OBJECT(sp), G_CALLBACK(receive_parcoords_drag), NULL);
		gtk_drag_source_unset(GTK_WIDGET(sp));
		gtk_drag_dest_unset(GTK_WIDGET(sp));
	}
}

void
parcoordsDragAndDropEnable(displayd *dsp, gboolean active) {
	GList *l;
	for (l = dsp->splots; l; l = l->next) {
		splotd *sp = (splotd *)l->data;
		parcoordsPlotDragAndDropEnable(sp, active);
	}
}

gboolean
parcoordsEventHandlersToggle(displayd * dpy, splotd * sp, gboolean state,
                            ProjectionMode pmode, InteractionMode imode)
{
	/* it's necessary to disable/enable so that duplicate handlers are not registered */
	/* it's necessary to toggle all plots, because all plots need to be ready to receive */
	/* would be better if there was some callback when the imode changed */
  parcoordsDragAndDropEnable(dpy, false);
  
  switch (imode) {
  case DEFAULT_IMODE:
      p1d_event_handlers_toggle (sp, state);
	  parcoordsDragAndDropEnable(dpy, true);
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

  return (false);
}

static gboolean
parcoordsKeyEventHandled(GtkWidget *w, displayd *display, splotd * sp, GdkEventKey *event, ggobid *gg)
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
    //gint lbearing, rbearing, width, ascent, descent;
    //GtkStyle *style = gtk_widget_get_style (sp->da);
	PangoRectangle rect;
	PangoLayout *layout = gtk_widget_create_pango_layout(GTK_WIDGET(sp->da), NULL);
	
    cpaneld *cpanel = &display->cpanel;

    vt = vartable_element_get (sp->p1dvar, d);
    
	/*gdk_text_extents (
      gtk_style_get_font (style),
      vt->collab_tform, strlen (vt->collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    */
	layout_text(layout, vt->collab_tform, &rect);
    if (cpanel->parcoords_arrangement == ARRANGE_ROW)
      gdk_draw_layout(drawable, gg->plot_GC, 
		(rect.width <= sp->max.x) ?  sp->max.x/2 - rect.width/2 : 0, 
		sp->max.y - rect.height - 5,
		layout);
		
	  /*gdk_draw_string (drawable,
        gtk_style_get_font (style),
        gg->plot_GC,
        //-- if the label fits, center it; else, left justify
        (width <= sp->max.x) ?  sp->max.x/2 - width/2 : 0, 
        sp->max.y - 5,
        vt->collab_tform);*/
     else
      /*gdk_draw_string (drawable,
        gtk_style_get_font (style),
        gg->plot_GC, 5, 5+ascent+descent, vt->collab_tform);*/
		gdk_draw_layout(drawable, gg->plot_GC, 5, 5, layout);
	g_object_unref(G_OBJECT(layout));
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
  /*GtkWidget *imode_menu;
  imode_menu = parcoords_imode_menu_make (gg->imode_accel_group,
    G_CALLBACK(imode_set_cb), gg, true);
  gg->imode_item = submenu_make ("_Interaction", 'I',
    gg->main_accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->imode_item),
                             imode_menu); 
  submenu_insert (gg->imode_item, gg->main_menubar, 2);
  */
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
splot_add_whisker_cues (gboolean nearest_p, gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
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

  if (nearest_p) {
    /* Add the label for the nearest point at the top as well */
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    splot_add_point_label (true, k, true, sp, drawable, gg);
  }
}


static GtkWidget *
parcoordsCPanelWidget(displayd *dpy, gchar **modeName, ggobid *gg)
{
  GtkWidget *w = GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
  if(!w) {
   GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w = cpanel_parcoords_make(gg);
  }
  *modeName = "Parcoords";
  return(w);
}

static void
splotScreenToTform(cpaneld *cpanel, splotd *sp, icoords *scr,
		   fcoords *tfd, ggobid *gg)
{
  gcoords planar, world;
  greal precis = (greal) PRECISION1;
  greal ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gfloat scale_x, scale_y;
  vartabled *vt;

  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

/*
 * screen to plane 
*/
  planar.x = (scr->x - sp->max.x/2) * precis / sp->iscale.x ;
  planar.x += sp->pmid.x;
  planar.y = (scr->y - sp->max.y/2) * precis / sp->iscale.y ;
  planar.y += sp->pmid.y;

/*
 * plane to tform
*/

  if (sp->p1dvar != -1) {

      vt = vartable_element_get (sp->p1dvar, d);
      max = vt->lim.max;
      min = vt->lim.min;
      rdiff = max - min;

      if (display->p1d_orientation == HORIZONTAL) {
        /* x */
        world.x = planar.x;
        ftmp = world.x / precis;
        tfd->x = (ftmp + 1.0) * .5 * rdiff;
        tfd->x += min;
      } else {
        /* y */
        world.y = planar.y;
        ftmp = world.y / precis;
        tfd->y = (ftmp + 1.0) * .5 * rdiff;
        tfd->y += min;
      }
  }

}

void
parcoordsDisplayClassInit(GGobiParCoordsDisplayClass *klass)
{
  klass->parent_class.loop_over_points = true;
  klass->parent_class.binningPermitted = parcoordsBinningPermitted;

  klass->parent_class.allow_reorientation = true;
  klass->parent_class.options_menu_p = true;

  klass->parent_class.loop_over_points = true;
  klass->parent_class.titleLabel = "Parallel Coordinates Display";
  klass->parent_class.treeLabel = "Parallel Coordinates";

  /* No create method, just createWithVars. */
  klass->parent_class.createWithVars = parcoords_new_with_vars;

  klass->parent_class.variable_select = variableSelect;
  klass->parent_class.variable_plotted_p = plotted;

  /* no unset */
  klass->parent_class.display_set = displaySet;
  
  klass->parent_class.mode_ui_get = parcoords_mode_ui_get;

  /* no build_symbol_vectors */

  /* ruler ranges set. */

  klass->parent_class.varpanel_refresh = varpanelRefresh;
  
  klass->parent_class.handles_interaction = handlesInteraction;

  klass->parent_class.cpanel_set = cpanelSet;

  klass->parent_class.xml_describe = add_xml_parcoords_variables;

  klass->parent_class.varpanel_tooltips_set = varpanelTooltipsSet;

  klass->parent_class.plotted_vars_get = plottedVarsGet;

  klass->parent_class.imode_control_box = parcoordsCPanelWidget;

/* menus_make */

  klass->parent_class.event_handlers_toggle = parcoordsEventHandlersToggle;
  klass->parent_class.splot_key_event_handled = parcoordsKeyEventHandled;

  klass->parent_class.add_plot_labels = addPlotLabels;
}


void
parcoordsSPlotClassInit(GGobiParCoordsSPlotClass *klass)
{
   klass->parent_class.alloc_whiskers = allocWhiskers;
   klass->parent_class.add_identify_cues = splot_add_whisker_cues;

   klass->parent_class.tree_label = treeLabel;

   /* reverse pipeline */ 
   klass->parent_class.screen_to_tform = splotScreenToTform;
   klass->parent_class.world_to_plane = worldToPlane;
   
   klass->parent_class.sub_plane_to_screen = withinPlaneToScreen;

   klass->parent_class.draw_edge_p = drawEdge_p;
   klass->parent_class.draw_case_p = drawCase_p;

   klass->parent_class.within_draw_to_binned = withinDrawBinned;
   klass->parent_class.within_draw_to_unbinned = withinDrawUnbinned;

   klass->parent_class.plotted_vars_get = splot1DVariablesGet;
}


splotd *
ggobi_parcoords_splot_new(displayd *dpy, ggobid *gg)
{
   splotd *sp = g_object_new(GGOBI_TYPE_PAR_COORDS_SPLOT, NULL);
   splot_init(sp, dpy, gg);
   return(sp);
}
