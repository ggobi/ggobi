#include "scatmatClass.h"
#include <gtk/gtk.h>

#include <math.h>
#include <string.h>

#include "externs.h"
#include "write_state.h"

static gboolean
cpanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{
  cpanel_scatmat_set (dpy, cpanel, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);
  return(true);
}

static void
movePointsMotionCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventMotion *event, ggobid *gg)
{
  if(sp->p1dvar == -1)
     scatterplotMovePointsMotionCb(display, sp, w, event, gg);
}

static void
movePointsButtonCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  if(sp->p1dvar == -1)
    scatterplotMovePointsButtonCb(display, sp, w, event, gg);
}

/* XXX duncan and dfs: you need to sort this out
static void
worldToRaw(displayd *display, splotd *sp, gint pt, datad *d, ggobid *gg)
{
 if (sp->p1dvar == -1) {
    world_to_raw_by_var (pt, sp->xyvars.x, display, d, gg);
    world_to_raw_by_var (pt, sp->xyvars.y, display, d, gg);
  }
}
*/


static gint 
variablePlottedP(displayd *display, gint *cols, gint ncols, datad *d)
{
	GList *l;
	gint j;
	splotd *sp;
        for (l = display->splots; l; l = l->next) {
          sp = (splotd *) l->data;

          for (j=0; j<ncols; j++) {
            if (sp->p1dvar == -1) {
              if (sp->xyvars.x == cols[j]) {
                return(sp->xyvars.x);
              }
              if (sp->xyvars.y == cols[j]) {
                return(sp->xyvars.y);
              }
            } else if (sp->p1dvar == cols[j]) {
              return(sp->p1dvar);
            }
          }
        }
	return(-1);
}

static gboolean
variableSelect(GtkWidget *w, displayd *dpy, splotd *sp, gint jvar, gint toggle, gint mouse, cpaneld *cpanel, ggobid *gg)
{
  gint jvar_prev;
  return(scatmat_varsel_simple (cpanel, sp, jvar, &jvar_prev, gg));
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
  l = display->scatmat_cols;  /*-- assume rows = cols --*/
  while (l) {
    j = GPOINTER_TO_INT (l->data);
    varpanel_toggle_set_active (VARSEL_X, j, true, d);
    l = l->next;
  }
}


static void
varpanelTooltipsSet(displayd *display, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label)
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
        if (s->p1dvar == -1) {
          if (!array_contains (cols, ncols, s->xyvars.x))
            cols[ncols++] = s->xyvars.x;
          if (!array_contains (cols, ncols, s->xyvars.y))
            cols[ncols++] = s->xyvars.y;
        } else {
          if (!array_contains (cols, ncols, s->p1dvar))
            cols[ncols++] = s->p1dvar;
        }
      }
      return(ncols);
}

displayd *
createWithVars(gboolean missing_p, gint nvars, gint *vars, datad *d, ggobid *gg)
{
   return(GGOBI(newScatmat)(vars, vars, nvars, nvars, d, gg));
}


void
add_xml_scatmat_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot = plots->data;
  int n, n1, i;

  n1 = g_list_length(plots);
  n = sqrt(n1);

  for(i = 0; i < n1 ; i+=n) {
      plot = (splotd *) g_list_nth_data(plots, i);
      XML_addVariable(node, plot->xyvars.x, dpy->d);
  }
}

static void
displaySet(displayd *display, ggobid *gg)
{
        scatmat_mode_menu_make (gg->main_accel_group,
          (GtkSignalFunc) viewmode_set_cb, gg, true);
        gg->viewmode_item = submenu_make ("_ViewMode", 'V',
          gg->main_accel_group);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
                                   gg->app.scatmat_mode_menu); 
        submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
}

static gboolean
handlesAction(displayd *display, PipelineMode v)
{
   return(v == SCALE || v == BRUSH || v == IDENT || v == MOVEPTS || v == SCATMAT);
}

void
scatmatDisplayClassInit(GtkGGobiScatmatDisplayClass *klass)
{
	klass->parent_class.show_edges_p = true;
	klass->parent_class.treeLabel = klass->parent_class.titleLabel = "Scatterplot Matrix";
	klass->parent_class.cpanel_set = cpanelSet;
	klass->parent_class.xml_describe = add_xml_scatmat_variables;
	klass->parent_class.move_points_motion_cb = movePointsMotionCb;
	klass->parent_class.move_points_button_cb = movePointsButtonCb;
/* XXX duncan and dfs: you need to sort this out
	klass->parent_class.world_to_raw = worldToRaw;
*/
	klass->parent_class.variable_plotted_p = variablePlottedP;
	klass->parent_class.variable_select = variableSelect;
	klass->parent_class.varpanel_refresh = varpanelRefresh;
	klass->parent_class.varpanel_tooltips_set = varpanelTooltipsSet;
	klass->parent_class.plotted_vars_get = plottedVarsGet;
	klass->parent_class.createWithVars = createWithVars;
	klass->parent_class.display_set = displaySet;
	klass->parent_class.handles_action = handlesAction;
}


/* */
static gchar *
treeLabel(splotd *splot, datad *d, ggobid *gg)
{
   gint n;
   vartabled *vtx, *vty;
   gchar *buf;
      vtx = vartable_element_get (splot->xyvars.x, d);
      vty = vartable_element_get (splot->xyvars.y, d);

      n = strlen (vtx->collab) + strlen (vty->collab) + 5;
      buf = (gchar*) g_malloc (n * sizeof (gchar*));
      sprintf (buf, "%s v %s", vtx->collab, vty->collab);
      return(buf);
}


static void
worldToPlane(splotd *sp, datad *d, ggobid *gg)
{
      if (sp->p1dvar == -1)
        xy_reproject (sp, d->world.vals, d, gg);
      else
        p1d_reproject (sp, d->world.vals, d, gg);
}

gboolean
drawEdgeP(splotd *sp, gint m, datad *d, datad *e, ggobid *gg)
{
	gboolean draw_edge = true;
        if (sp->p1dvar != -1) {
          if (e->missing.vals[m][sp->p1dvar])
            draw_edge = false;
        } else {
          if (e->missing.vals[m][sp->xyvars.x] ||
              e->missing.vals[m][sp->xyvars.y])
          {
            draw_edge = false;
          }
        }
	return(draw_edge);
}

gboolean
drawCaseP(splotd *sp, gint m, datad *d, ggobid *gg)
{
	gboolean draw_case = true;
        if (sp->p1dvar != -1) {
          if (d->missing.vals[m][sp->p1dvar])
            draw_case = false;
        } else {
          if (d->missing.vals[m][sp->xyvars.x] ||
              d->missing.vals[m][sp->xyvars.y])
          {
            draw_case = false;
          }
        }
	return(draw_case);
}

void
addPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
    if (sp->p1dvar == -1)
      scatterXYAddPlotLabels(sp, drawable, gg->plot_GC);
    else {
           /*-- 1dplot: center the label --*/
      scatter1DAddPlotLabels(sp, drawable, gg->plot_GC);
    }
}


static gint
splotVariablesGet(splotd *sp, gint *cols, datad *d)
{
	if(sp->p1dvar > -1) {
   	   cols[0] = sp->p1dvar;
	   return(1);
	} else {
    	   cols[0] = sp->xyvars.x;
     	   cols[1] = sp->xyvars.y;
   	   return(2);
	}
}


void
scatmatSPlotClassInit(GtkGGobiScatmatSPlotClass *klass) 
{
  klass->parent_class.tree_label = treeLabel;
  klass->parent_class.world_to_plane = worldToPlane;
  klass->parent_class.draw_case_p = drawCaseP;
  klass->parent_class.draw_edge_p = drawEdgeP;
  klass->parent_class.add_plot_labels = addPlotLabels;

  klass->parent_class.plotted_vars_get = splotVariablesGet;
}
