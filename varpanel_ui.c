/* varpanel_ui.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#include "vars.h"
#include "externs.h"

#define VAR_CIRCLE_DIAM 40


/* */
static void varcircle_add (gint, gint, gint, ggobid *gg);
static void varcircle_draw (gint, ggobid *gg); 
/* */

/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint btn,
  gint alt_mod, gint ctrl_mod, gint shift_mod, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw;
  gint jvar_prev = -1;

  if (display == NULL || !GTK_IS_WIDGET (display->window)) {
    g_printerr ("Bug?  I see no active display\n");
    return ;
  }
  
  switch (display->displaytype) {

    case parcoords:
      redraw = parcoords_varsel (cpanel, sp, jvar, &jvar_prev, alt_mod, gg);
      break;

    case scatmat:
      redraw = scatmat_varsel (cpanel, sp, jvar, &jvar_prev, btn, alt_mod, gg);
      break;

      case scatterplot:
        switch (cpanel->projection) {
          case P1PLOT:
            redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
            break;
          case XYPLOT:
            redraw = xyplot_varsel (sp, jvar, &jvar_prev, btn);
            break;
          case TOUR2D:
/*            tour2d_varsel (sp, jvar, &jvar_prev, btn);*/
          default:
            break;
      }
      break;
  }

  if (mode_get (gg) == BRUSH)
    assign_points_to_bins (gg);

  varcircle_draw (jvar, gg);
  if (jvar_prev != -1)
    varcircle_draw (jvar_prev, gg);

  /*-- overkill for scatmat: could redraw one row, one column --*/
  /*-- overkill for parcoords: need to redraw at most 3 plots --*/
/* this is redrawing before it has the new window sizes, so the
 * lines aren't right */
  if (redraw) {
    display_tailpipe (display, gg);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Variable menus                                      */
/*-------------------------------------------------------------------------*/


static void
varsel_from_menu (GtkWidget *w, gpointer data)
{
  varseldatad *vdata = (varseldatad *) data;
  ggobid *gg = vdata->gg;
  cpaneld *cpanel = &gg->current_display->cpanel;

  varsel (cpanel, gg->current_splot, vdata->jvar, vdata->btn,
    vdata->alt_mod, vdata->ctrl_mod, vdata->shift_mod, gg);
}

GtkWidget *
p1d_menu_build (gint jvar, ggobid *gg)
{
  GtkWidget *menu;

  gg->p1d_menu.vdata0.sp = gg->p1d_menu.vdata1.sp = gg->current_splot;
  gg->p1d_menu.vdata0.jvar = gg->p1d_menu.vdata1.jvar = jvar;
  gg->p1d_menu.vdata0.alt_mod = gg->p1d_menu.vdata1.alt_mod = false;

  gg->p1d_menu.vdata0.btn = 1;
  gg->p1d_menu.vdata1.btn = 2;

  menu = gtk_menu_new ();

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &(gg->p1d_menu.vdata0), gg);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &(gg->p1d_menu.vdata1), gg);

  return menu;
}

GtkWidget *
xyplot_menu_build (gint jvar, ggobid *gg)
{
  GtkWidget *menu;

  gg->xyplot_menu.vdata0.sp = gg->xyplot_menu.vdata1.sp = gg->current_splot;
  gg->xyplot_menu.vdata0.jvar = gg->xyplot_menu.vdata1.jvar = jvar;
  gg->xyplot_menu.vdata0.alt_mod = gg->xyplot_menu.vdata1.alt_mod = false;

  gg->xyplot_menu.vdata0.btn = 1;
  gg->xyplot_menu.vdata1.btn = 2;

  gg->xyplot_menu.vdata0.gg = gg;
  gg->xyplot_menu.vdata1.gg = gg;

  menu = gtk_menu_new ();

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->xyplot_menu.vdata0, gg);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->xyplot_menu.vdata1, gg);

  return menu;
}

GtkWidget *
rotation_menu_build (gint jvar, ggobid *gg)
{
  GtkWidget *menu;
  
  gg->rotation_menu.vdata0.sp = gg->rotation_menu.vdata1.sp = gg->rotation_menu.vdata2.sp = gg->current_splot;
  gg->rotation_menu.vdata0.jvar = gg->rotation_menu.vdata1.jvar = gg->rotation_menu.vdata2.jvar = jvar;
  gg->rotation_menu.vdata0.alt_mod = gg->rotation_menu.vdata1.alt_mod = gg->rotation_menu.vdata2.alt_mod = false;

  gg->rotation_menu.vdata0.btn = 1;
  gg->rotation_menu.vdata1.btn = 2;
  gg->rotation_menu.vdata2.btn = 3;

  gg->rotation_menu.vdata2.gg = gg->rotation_menu.vdata1.gg = gg->rotation_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Select X  L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->rotation_menu.vdata0, gg);
  CreateMenuItem (menu, "Select Y  M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->rotation_menu.vdata1, gg);
  CreateMenuItem (menu, "Select Z  R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->rotation_menu.vdata2, gg);

  return menu;
}

GtkWidget *
tour2d_menu_build (gint jvar, ggobid *gg)
{
  GtkWidget *menu;

  gg->tour2d_menu.vdata0.sp = gg->tour2d_menu.vdata1.sp = gg->tour2d_menu.vdata2.sp = gg->current_splot;
  gg->tour2d_menu.vdata0.jvar = gg->tour2d_menu.vdata1.jvar = gg->tour2d_menu.vdata2.jvar = jvar;
  gg->tour2d_menu.vdata0.alt_mod = gg->tour2d_menu.vdata1.alt_mod = gg->tour2d_menu.vdata2.alt_mod = false;
  gg->tour2d_menu.vdata0.shift_mod = gg->tour2d_menu.vdata2.shift_mod = false;
  gg->tour2d_menu.vdata1.shift_mod = true;
  gg->tour2d_menu.vdata0.ctrl_mod = gg->tour2d_menu.vdata1.ctrl_mod = false;
  gg->tour2d_menu.vdata2.ctrl_mod = true;

  gg->tour2d_menu.vdata2.gg =   gg->tour2d_menu.vdata1.gg =   gg->tour2d_menu.vdata0.gg = gg;


  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Tour   L,M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->tour2d_menu.vdata0, gg);
  CreateMenuItem (menu, "Manip  <Shift> L,M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->tour2d_menu.vdata1, gg);
  CreateMenuItem (menu, "Freeze <Ctrl> L,M",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->tour2d_menu.vdata2, gg);

  return menu;
}

/*
  corr_tour_menu = gtk_menu_new ();
  CreateMenuItem (corr_tour_menu, "Tour X    L",         NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Tour Y    M",         NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Manip X   <Shift> L", NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Manip Y   <Shift> M", NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Freeze X  <Ctrl> L",  NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Freeze Y  <Ctrl> M",  NULL, NULL,
    varpanel, varpanel_accel_group, NULL, NULL);
*/

GtkWidget *
parcoords_menu_build (gint jvar, ggobid *gg)
{
  GtkWidget *menu;

  gg->parcoords_menu.vdata0.sp = gg->parcoords_menu.vdata1.sp = gg->current_splot;
  gg->parcoords_menu.vdata0.jvar = gg->parcoords_menu.vdata1.jvar = jvar;
  gg->parcoords_menu.vdata0.alt_mod = false;
  gg->parcoords_menu.vdata1.alt_mod = true;

  gg->parcoords_menu.vdata1.gg = gg->parcoords_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Select Y      M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->parcoords_menu.vdata0, gg);
  CreateMenuItem (menu, "Delete Y <alt>M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->parcoords_menu.vdata1, gg);

  return menu;
}

GtkWidget *
scatmat_menu_build (gint jvar, ggobid *gg)
{
  GtkWidget *menu;

  gg->scatmat_menu.vdata0.sp = gg->scatmat_menu.vdata1.sp = gg->scatmat_menu.vdata2.sp = gg->scatmat_menu.vdata3.sp = gg->current_splot;
  gg->scatmat_menu.vdata0.jvar = gg->scatmat_menu.vdata1.jvar = gg->scatmat_menu.vdata2.jvar = gg->scatmat_menu.vdata3.jvar = jvar;
  gg->scatmat_menu.vdata0.alt_mod = gg->scatmat_menu.vdata1.alt_mod = false;
  gg->scatmat_menu.vdata2.alt_mod = gg->scatmat_menu.vdata3.alt_mod = 3;

  gg->scatmat_menu.vdata0.btn = gg->scatmat_menu.vdata2.btn = 1;
  gg->scatmat_menu.vdata1.btn = gg->scatmat_menu.vdata3.btn = 2;

  gg->scatmat_menu.vdata3.gg = gg->scatmat_menu.vdata2.gg = gg->scatmat_menu.vdata1.gg = gg->scatmat_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Select row  L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->scatmat_menu.vdata0, gg);
  CreateMenuItem (menu, "Select col  M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->scatmat_menu.vdata1, gg);
  CreateMenuItem (menu, "Delete row  <alt>L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->scatmat_menu.vdata2, gg);
  CreateMenuItem (menu, "Delete col  <alt>M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->scatmat_menu.vdata3, gg);

  return menu;
}

static gint
popup_varmenu (GtkWidget *w, GdkEvent *event, gpointer cbd) 
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel;
  gint jvar = GPOINTER_TO_INT (cbd);
  GtkWidget *p1d_menu, *xyplot_menu;
  GtkWidget *rotation_menu, *tour_menu;
  GtkWidget *parcoords_menu, *scatmat_menu;

  if (display == NULL)
    return false;

  cpanel = &display->cpanel;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button == 1) {
      gint projection = projection_get (gg);

      switch (display->displaytype) {

        case scatterplot:
          switch (projection) {
            case P1PLOT:
              p1d_menu = p1d_menu_build (jvar, gg);
              gtk_menu_popup (GTK_MENU (p1d_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
            case XYPLOT:
              xyplot_menu = xyplot_menu_build (jvar, gg);
              gtk_menu_popup (GTK_MENU (xyplot_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
            case ROTATE:
              rotation_menu = rotation_menu_build (jvar, gg);
              gtk_menu_popup (GTK_MENU (rotation_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
            case TOUR2D:
            case COTOUR:
              tour_menu = tour2d_menu_build (jvar, gg);
              gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
          }
          break;

        case scatmat:
          scatmat_menu = scatmat_menu_build (jvar, gg);
          gtk_menu_popup (GTK_MENU (scatmat_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;

        case parcoords:
          parcoords_menu = parcoords_menu_build (jvar, gg);
          gtk_menu_popup (GTK_MENU (parcoords_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;
      }
      return true;
    }
  }
  return false;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

void
variable_clone (gint jvar, const gchar *newName, gboolean update, ggobid *gg) 
{
  gint nc = gg->ncols + 1;
  gint i, j, k = 0;
  

  /*-- set a view of the data values before building the new circle --*/
  vartable_row_append (gg->ncols-1, gg);
  vardata_realloc (nc, gg);
  gg->vardata[nc-1].collab = g_strdup (newName && newName[0] ? newName : gg->vardata[jvar].collab);
  gg->vardata[nc-1].collab_tform = g_strdup (newName && newName[0] ? newName : gg->vardata[jvar].collab);

  /*
   * Follow the algorithm by which the table has been populated
  */
  if (gg->varpanel_ui.vnrows*gg->varpanel_ui.vncols <= gg->ncols) {
    gg->varpanel_ui.vnrows++;
    gtk_table_resize (GTK_TABLE (gg->varpanel_ui.varpanel),
                      gg->varpanel_ui.vnrows, gg->varpanel_ui.vncols);
  }

  k = 0;
  for (i=0; i<gg->varpanel_ui.vnrows; i++) {
    for (j=0; j<gg->varpanel_ui.vncols; j++) {
      if (k < gg->ncols)
        ;
      else {
        gg->varpanel_ui.da = (GtkWidget **)
          g_realloc (gg->varpanel_ui.da, nc * sizeof (GtkWidget *));
        gg->varpanel_ui.varlabel = (GtkWidget **)
          g_realloc (gg->varpanel_ui.varlabel, nc * sizeof (GtkWidget *));
        varcircle_add (i, j, k, gg);
      }
      k++;
      if (k == gg->ncols+1) break;
    }
  }


  /*-- now the rest of the variables --*/
  gg->vardata[nc-1].groupid = gg->vardata[nc-1].groupid_ori =
    gg->vardata[gg->ncols-1].groupid + 1; 

  gg->vardata[nc-1].jitter_factor = gg->vardata[jvar].jitter_factor;

  gg->vardata[nc-1].nmissing = gg->vardata[jvar].nmissing;

  if(update) {
    updateAddedColumn(nc, jvar, gg);
  }

  gtk_widget_show_all (gg->varpanel_ui.varpanel);
}


gboolean
updateAddedColumn(int nc, int jvar, ggobid *gg)
{
 if(jvar > -1) {
  gg->vardata[nc-1].mean = gg->vardata[jvar].mean;
  gg->vardata[nc-1].median = gg->vardata[jvar].median;
  gg->vardata[nc-1].lim.min =
    gg->vardata[nc-1].lim_raw.min = gg->vardata[nc-1].lim_raw_gp.min =
    gg->vardata[nc-1].lim_tform.min = gg->vardata[nc-1].lim_tform_gp.min =
    gg->vardata[jvar].lim_raw.min;
  gg->vardata[nc-1].lim.max =
    gg->vardata[nc-1].lim_raw.max = gg->vardata[nc-1].lim_raw_gp.max =
    gg->vardata[nc-1].lim_tform.max = gg->vardata[nc-1].lim_tform_gp.max =
    gg->vardata[jvar].lim_raw.max;
 } 

  transform_values_init (nc-1, gg);

  pipeline_arrays_add_column (jvar, gg);  /* reallocate and copy */
  missing_arrays_add_column (jvar, gg);

  gg->ncols++;
  tform_to_world (gg); /*-- need this only for the new variable --*/

  return(true);
}


static gint
varsel_cb (GtkWidget *w, GdkEvent *event, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;
  gint jvar = GPOINTER_TO_INT (cbd);

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint button = bevent->button;
    gboolean alt_mod, shift_mod, ctrl_mod;

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    if (ctrl_mod) {
      variable_clone (jvar, NULL, true, gg);
      return (false);
    }
    
    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, gg);
    return true;
  }

  return false;
}


void
varcircle_draw (gint jvar, ggobid *gg)
{
  /*--  a single pixmap is shared among all variable circles --*/
  static GdkPixmap *vpixmap = NULL;
  gint r = VAR_CIRCLE_DIAM/2;
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;
  gboolean chosen = false;
  GList *l;
  splotd *s;

  if (gg->current_splot == NULL || jvar < 0 || jvar >= gg->ncols)
    return;  /*-- return --*/

  if (gg->selvarfg_GC == NULL) 
    init_var_GCs (gg->varpanel_ui.da[jvar], gg);

  if (vpixmap == NULL) {
    vpixmap = gdk_pixmap_new (gg->varpanel_ui.da[jvar]->window,
              VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1, -1);
  }

  /*-- clear the pixmap --*/
  gdk_draw_rectangle (vpixmap, gg->unselvarbg_GC, true,
                      0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  /*-- add a filled circle for the background --*/
  gdk_draw_arc (vpixmap, gg->selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);

  /*-- add the appropriate line --*/
  switch (gg->current_display->displaytype) {

    case  parcoords:  /* only one mode, a 1d plot */
      l = gg->current_display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->p1dvar == jvar) {
          if (gg->current_display->p1d_orientation == HORIZONTAL)
            gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r+r, r);
          else
            gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r, 0);
          chosen = true;
          break;
        }
        l = l->next;
      }
      break;

    case  scatterplot:
      switch (cpanel->projection) {
        case P1PLOT:
          if (jvar == sp->p1dvar) {
            if (gg->current_display->p1d_orientation == HORIZONTAL)
              gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r+r, r);
            else
              gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
          break;
        case XYPLOT:
          if (jvar == sp->xyvars.x) {
            gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r+r, r);
            chosen = true;
          } else if (jvar == sp->xyvars.y) {
            gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
          break;
      }
      break;

    case  scatmat:
      l = gg->current_display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->p1dvar == -1) {
          if (s->xyvars.x == jvar) {
            gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r+r, r);
            chosen = true;
          } else if (s->xyvars.y == jvar) {
            gdk_draw_line (vpixmap, gg->selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
        }
        l = l->next;
      }
      break;

    default:
      ;
  }

  /*
   * add an open circle for the outline
  */
  if (chosen) {
    gdk_draw_arc (vpixmap, gg->selvarfg_GC, false,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  } else {
    gdk_draw_arc (vpixmap, gg->unselvarfg_GC, false,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  }


  /*
   * copy the pixmap to the window
  */
  gdk_draw_pixmap (gg->varpanel_ui.da[jvar]->window, gg->unselvarfg_GC, vpixmap, 0, 0, 0, 0,
    VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
}

gboolean
da_expose_cb (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  gint k = GPOINTER_TO_INT (cbd);

gg = ggobi_get(0);

  varcircle_draw (k, gg); 

  return true;
}


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*-------------------------------------------------------------------------*/

static void
varcircle_add (gint i, gint j, gint k, ggobid *gg)
{
  GtkWidget *vb;

  vb = gtk_vbox_new (false, 0);
  gtk_container_border_width (GTK_CONTAINER (vb), 1);
  gtk_widget_show (vb);

  gg->varpanel_ui.varlabel[k] =
    gtk_button_new_with_label (gg->vardata[k].collab);

  gtk_widget_show (gg->varpanel_ui.varlabel[k]);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    gg->varpanel_ui.varlabel[k], "Click left to select", NULL);
  gtk_container_add (GTK_CONTAINER (vb), gg->varpanel_ui.varlabel[k]);

  gtk_signal_connect (GTK_OBJECT (gg->varpanel_ui.varlabel[k]),
    "button_press_event",
    GTK_SIGNAL_FUNC (popup_varmenu), GINT_TO_POINTER (k));

  GGobi_widget_set(GTK_WIDGET(gg->varpanel_ui.varlabel[k]), gg, true);
  /*
   * a drawing area to contain the variable circle
  */

  gg->varpanel_ui.da[k] = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (gg->varpanel_ui.da[k]),
    VAR_CIRCLE_DIAM+2, VAR_CIRCLE_DIAM+2);
  gtk_widget_set_events (gg->varpanel_ui.da[k], GDK_EXPOSURE_MASK
             | GDK_ENTER_NOTIFY_MASK
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_BUTTON_RELEASE_MASK);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    gg->varpanel_ui.da[k], "Click to select; see menu", NULL);

  gtk_signal_connect (GTK_OBJECT (gg->varpanel_ui.da[k]), "expose_event",
    GTK_SIGNAL_FUNC (da_expose_cb), GINT_TO_POINTER (k));
  gtk_signal_connect (GTK_OBJECT (gg->varpanel_ui.da[k]), "button_press_event",
    GTK_SIGNAL_FUNC (varsel_cb), GINT_TO_POINTER (k));

  GGobi_widget_set(GTK_WIDGET(gg->varpanel_ui.da[k]), gg, true);

  gtk_widget_show (gg->varpanel_ui.da[k]);
  gtk_container_add (GTK_CONTAINER (vb), gg->varpanel_ui.da[k]);
  gtk_table_attach (GTK_TABLE (gg->varpanel_ui.varpanel), vb, j, j+1, i, i+1,
    GTK_FILL, GTK_FILL, 0, 0);
}

/*-- create a grid of buttons in the table --*/
void varpanel_populate (ggobid *gg)
{
  gint i, j, k;

  /*-- realloc in case they've been alloc'ed before --*/

  if (gg->varpanel_ui.da == NULL) {
    gg->varpanel_ui.da = (GtkWidget **) g_malloc (gg->ncols * sizeof (GtkWidget *));
    gg->varpanel_ui.varlabel = (GtkWidget **) g_malloc (gg->ncols * sizeof (GtkWidget *));
  } else {
    gg->varpanel_ui.da = (GtkWidget **) g_realloc (gg->varpanel_ui.da, gg->ncols * sizeof (GtkWidget *));
    gg->varpanel_ui.varlabel = (GtkWidget **) g_realloc (gg->varpanel_ui.varlabel,
                                         gg->ncols * sizeof (GtkWidget *));
  }

  k = 0;
  for (i=0; i<gg->varpanel_ui.vnrows; i++) {
    for (j=0; j<gg->varpanel_ui.vncols; j++) {
      varcircle_add (i, j, k, gg);
      k++;
      if (k == gg->ncols) break;
    }
  }
}

void
make_varpanel (GtkWidget *parent, ggobid *gg) {

  gg->selvarfg_GC = NULL;

  gg->varpanel_ui.varpanel_accel_group = gtk_accel_group_new ();

  /* create a new scrolled window. */
  gg->varpanel_ui.scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent),
    gg->varpanel_ui.scrolled_window, true, true, 2);
  gtk_widget_show (gg->varpanel_ui.scrolled_window);

  gg->varpanel_ui.varpanel = gtk_table_new (gg->varpanel_ui.vnrows,
    gg->varpanel_ui.vncols, true);

  /* pack the table into the scrolled window */
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    gg->varpanel_ui.varpanel);
  gtk_widget_show (gg->varpanel_ui.varpanel);

  varpanel_layout_init (gg);
  varpanel_populate (gg);

  gtk_widget_show_all (gg->varpanel_ui.scrolled_window);
}

/*
 * Make the variable panel as square as possible, and lay it out
 * row-wise.
*/
void
varpanel_layout_init (ggobid *gg) {

  gg->varpanel_ui.vnrows = (gint) sqrt ((gdouble) gg->ncols);
  gg->varpanel_ui.vncols = gg->varpanel_ui.vnrows;

  while (gg->varpanel_ui.vnrows*gg->varpanel_ui.vncols < gg->ncols) {
    gg->varpanel_ui.vnrows++;
    if (gg->varpanel_ui.vnrows*gg->varpanel_ui.vncols < gg->ncols)
      gg->varpanel_ui.vncols++;
  }
}


void
varpanel_size_init (gint cpanel_height, ggobid* gg)
{
  gint i;
  GtkTable *t = GTK_TABLE (gg->varpanel_ui.varpanel);
  GtkTableRowCol c, r;
  gint width = 0, height = 0;
  GtkWidget *vport = GTK_WIDGET
    ((GTK_BIN (gg->varpanel_ui.scrolled_window))->child);

  /*-- Find the width of the first few columns --*/
  for (i=0; i<MIN (t->ncols, 3); i++) {
    c = t->cols[i];
    width += c.requisition + c.spacing;
  }

  /*-- Find the height of the first few rows --*/
  for (i=0; i<MIN (t->nrows, 4); i++) {
    r = t->rows[i];
    height += r.requisition + r.spacing;
  }

  gtk_widget_set_usize (vport, width, MAX (height, cpanel_height));
}


void
varpanel_refresh (ggobid *gg) {
  gint j;

  for (j=0; j<gg->ncols; j++)
    if (GTK_WIDGET_REALIZED (gg->varpanel_ui.da[j]))
      gtk_widget_queue_draw (gg->varpanel_ui.da[j]);
}

void
varlabel_set (gint j, ggobid *gg) {
  gtk_label_set_text (GTK_LABEL (GTK_BIN (gg->varpanel_ui.varlabel[j])->child),
    gg->vardata[j].collab_tform);
}
