/* varpanel_ui.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#include "vars.h"
#include "externs.h"

#define VAR_CIRCLE_DIAM 40


/* */
static void varcircle_add (gint, gint, gint, datad *, ggobid *gg);
static void varcircle_draw (gint, datad *, ggobid *gg); 
/* */

/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint btn,
  gint alt_mod, gint ctrl_mod, gint shift_mod, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw;
  gint jvar_prev = -1;
  extern void tour2d_varsel (ggobid *, gint, gint);

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
            tour2d_varsel (gg, jvar, btn);
            break;
          default:
            break;
      }
      break;
  }

  varcircle_draw (jvar, d, gg);
  if (jvar_prev != -1)
    varcircle_draw (jvar_prev, d, gg);

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
  displayd *display = gg->current_display;
  datad *d = display->d;
  cpaneld *cpanel = &display->cpanel;

  /*-- I think the menu should be destroyed here. --*/
  gtk_widget_destroy (w->parent);

  varsel (cpanel, gg->current_splot, vdata->jvar, vdata->btn,
    vdata->alt_mod, vdata->ctrl_mod, vdata->shift_mod, d, gg);
}

GtkWidget *
p1d_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->p1d_menu.vdata0.sp = gg->p1d_menu.vdata1.sp = gg->current_splot;
  gg->p1d_menu.vdata0.jvar = gg->p1d_menu.vdata1.jvar = jvar;
  gg->p1d_menu.vdata0.alt_mod = gg->p1d_menu.vdata1.alt_mod = false;

  gg->p1d_menu.vdata0.btn = 1;
  gg->p1d_menu.vdata1.btn = 2;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &(gg->p1d_menu.vdata0), gg);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &(gg->p1d_menu.vdata1), gg);

  return menu;
}

GtkWidget *
xyplot_menu_build (gint jvar, datad *d, ggobid *gg)
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
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->xyplot_menu.vdata0, gg);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, gg->varpanel_ui.varpanel, gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &gg->xyplot_menu.vdata1, gg);

  return menu;
}

GtkWidget *
rotation_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;
  
  gg->rotation_menu.vdata0.sp = gg->rotation_menu.vdata1.sp =
    gg->rotation_menu.vdata2.sp = gg->current_splot;
  gg->rotation_menu.vdata0.jvar = gg->rotation_menu.vdata1.jvar =
    gg->rotation_menu.vdata2.jvar = jvar;
  gg->rotation_menu.vdata0.alt_mod = gg->rotation_menu.vdata1.alt_mod =
    gg->rotation_menu.vdata2.alt_mod = false;

  gg->rotation_menu.vdata0.btn = 1;
  gg->rotation_menu.vdata1.btn = 2;
  gg->rotation_menu.vdata2.btn = 3;

  gg->rotation_menu.vdata2.gg = gg->rotation_menu.vdata1.gg =
    gg->rotation_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select X  L",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->rotation_menu.vdata0, gg);
  CreateMenuItem (menu, "Select Y  M",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->rotation_menu.vdata1, gg);
  CreateMenuItem (menu, "Select Z  R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->rotation_menu.vdata2, gg);

  return menu;
}

GtkWidget *
tour2d_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->tour2d_menu.vdata0.sp = gg->tour2d_menu.vdata1.sp =
    gg->tour2d_menu.vdata2.sp = gg->current_splot;
  gg->tour2d_menu.vdata0.jvar = gg->tour2d_menu.vdata1.jvar =
    gg->tour2d_menu.vdata2.jvar = jvar;
  gg->tour2d_menu.vdata0.alt_mod = gg->tour2d_menu.vdata1.alt_mod =
    gg->tour2d_menu.vdata2.alt_mod = false;
  gg->tour2d_menu.vdata0.shift_mod = gg->tour2d_menu.vdata2.shift_mod = false;
  gg->tour2d_menu.vdata1.shift_mod = true;
  gg->tour2d_menu.vdata0.ctrl_mod = gg->tour2d_menu.vdata1.ctrl_mod = false;
  gg->tour2d_menu.vdata2.ctrl_mod = true;

  gg->tour2d_menu.vdata2.gg = gg->tour2d_menu.vdata1.gg =  
    gg->tour2d_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

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
parcoords_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->parcoords_menu.vdata0.sp = gg->parcoords_menu.vdata1.sp =
    gg->current_splot;
  gg->parcoords_menu.vdata0.jvar = gg->parcoords_menu.vdata1.jvar = jvar;
  gg->parcoords_menu.vdata0.alt_mod = false;
  gg->parcoords_menu.vdata1.alt_mod = true;

  gg->parcoords_menu.vdata1.gg = gg->parcoords_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select Y      M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->parcoords_menu.vdata0, gg);
  CreateMenuItem (menu, "Delete Y <alt>M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->parcoords_menu.vdata1, gg);

  return menu;
}

GtkWidget *
scatmat_menu_build (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *menu;

  gg->scatmat_menu.vdata0.sp =
    gg->scatmat_menu.vdata1.sp =
    gg->scatmat_menu.vdata2.sp =
    gg->scatmat_menu.vdata3.sp =
    gg->current_splot;
  gg->scatmat_menu.vdata0.jvar =
    gg->scatmat_menu.vdata1.jvar =
    gg->scatmat_menu.vdata2.jvar =
    gg->scatmat_menu.vdata3.jvar = jvar;
  gg->scatmat_menu.vdata0.alt_mod = gg->scatmat_menu.vdata1.alt_mod = false;
  gg->scatmat_menu.vdata2.alt_mod = gg->scatmat_menu.vdata3.alt_mod = 3;

  gg->scatmat_menu.vdata0.btn = gg->scatmat_menu.vdata2.btn = 1;
  gg->scatmat_menu.vdata1.btn = gg->scatmat_menu.vdata3.btn = 2;

  gg->scatmat_menu.vdata3.gg =
    gg->scatmat_menu.vdata2.gg =
    gg->scatmat_menu.vdata1.gg =
    gg->scatmat_menu.vdata0.gg = gg;

  menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.label[jvar]);

  CreateMenuItem (menu, "Select row  L",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->scatmat_menu.vdata0, gg);
  CreateMenuItem (menu, "Select col  M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->scatmat_menu.vdata1, gg);
  CreateMenuItem (menu, "Delete row  <alt>L",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC
    (varsel_from_menu), (gpointer) &gg->scatmat_menu.vdata2, gg);
  CreateMenuItem (menu, "Delete col  <alt>M,R",
    NULL, NULL, gg->varpanel_ui.varpanel,
    gg->varpanel_ui.varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu),
    (gpointer) &gg->scatmat_menu.vdata3, gg);

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
  /*-- w  is the variable label --*/
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (w), "datad");

  if (display == NULL)
    return false;
  if (display->d != d)  /*-- only select for the current plot --*/
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
              p1d_menu = p1d_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (p1d_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
            case XYPLOT:
              xyplot_menu = xyplot_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (xyplot_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
            case ROTATE:
              rotation_menu = rotation_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (rotation_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
            case TOUR2D:
            case COTOUR:
              tour_menu = tour2d_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
              break;
          }
          break;

        case scatmat:
          scatmat_menu = scatmat_menu_build (jvar, d, gg);
          gtk_menu_popup (GTK_MENU (scatmat_menu), NULL, NULL,
                position_popup_menu, NULL,
            bevent->button, bevent->time);
          break;

        case parcoords:
          parcoords_menu = parcoords_menu_build (jvar, d, gg);
          gtk_menu_popup (GTK_MENU (parcoords_menu), NULL, NULL,
                position_popup_menu, NULL,
            bevent->button, bevent->time);
          break;
      }
      return true;
    }
  }
  return false;
}

/*-------------------------------------------------------------------------*/
/*                   variable cloning                                      */
/*-------------------------------------------------------------------------*/

void
variable_clone (gint jvar, const gchar *newName, gboolean update,
  datad *d, ggobid *gg) 
{
  gint nc = d->ncols + 1;
  gint i, j, k = 0;
  
  /*-- set a view of the data values before building the new circle --*/
  vartable_row_append (d->ncols-1, d, gg);
  vartable_realloc (nc, d, gg);
  d->vartable[nc-1].collab =
    g_strdup (newName && newName[0] ? newName : d->vartable[jvar].collab);
  d->vartable[nc-1].collab_tform =
    g_strdup (newName && newName[0] ? newName : d->vartable[jvar].collab);

  /*
   * Follow the algorithm by which the table has been populated
  */
  if (d->varpanel_ui.vnrows*d->varpanel_ui.vncols <= d->ncols) {
    d->varpanel_ui.vnrows++;
    gtk_table_resize (GTK_TABLE (d->varpanel_ui.table),
                      d->varpanel_ui.vnrows, d->varpanel_ui.vncols);
  }

  k = 0;
  for (i=0; i<d->varpanel_ui.vnrows; i++) {
    for (j=0; j<d->varpanel_ui.vncols; j++) {
      if (k < d->ncols)
        ;
      else {
        d->varpanel_ui.da = (GtkWidget **)
          g_realloc (d->varpanel_ui.da, nc * sizeof (GtkWidget *));

        d->varpanel_ui.da_pix = (GdkPixmap **)
          g_realloc (d->varpanel_ui.da_pix, nc * sizeof (GdkPixmap *));
        d->varpanel_ui.da_pix[nc-1] = NULL;

        d->varpanel_ui.label = (GtkWidget **)
          g_realloc (d->varpanel_ui.label, nc * sizeof (GtkWidget *));
        varcircle_add (i, j, k, d, gg);
      }
      k++;
      if (k == d->ncols+1) break;
    }
  }


  /*-- now the rest of the variables --*/
  d->vartable[nc-1].groupid = d->vartable[nc-1].groupid_ori =
    d->vartable[d->ncols-1].groupid + 1; 

  d->vartable[nc-1].jitter_factor = d->vartable[jvar].jitter_factor;

  d->vartable[nc-1].nmissing = d->vartable[jvar].nmissing;

  if(update) {
    updateAddedColumn (nc, jvar, d, gg);
  }

  gtk_widget_show_all (gg->varpanel_ui.varpanel);
}


gboolean
updateAddedColumn (gint nc, gint jvar, datad *d, ggobid *gg)
{
  if(jvar > -1) {
    d->vartable[nc-1].mean = d->vartable[jvar].mean;
    d->vartable[nc-1].median = d->vartable[jvar].median;
    d->vartable[nc-1].lim.min =
      d->vartable[nc-1].lim_raw.min = d->vartable[nc-1].lim_raw_gp.min =
      d->vartable[nc-1].lim_tform.min = d->vartable[nc-1].lim_tform_gp.min =
      d->vartable[jvar].lim_raw.min;
    d->vartable[nc-1].lim.max =
      d->vartable[nc-1].lim_raw.max = d->vartable[nc-1].lim_raw_gp.max =
      d->vartable[nc-1].lim_tform.max = d->vartable[nc-1].lim_tform_gp.max =
      d->vartable[jvar].lim_raw.max;
   } 

  transform_values_init (nc-1, d, gg);

  pipeline_arrays_add_column (jvar, d, gg);  /* reallocate and copy */
  missing_arrays_add_column (jvar, d, gg);

  d->ncols++;
  tform_to_world (d, gg); /*-- need this only for the new variable --*/

  return (true);
}

/*-------------------------------------------------------------------------*/

static gint
varsel_cb (GtkWidget *w, GdkEvent *event, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *display = gg->current_display;
  datad *d = display->d;
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
      variable_clone (jvar, NULL, true, d, gg);
      return (false);
    }
    
    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, d, gg);
    return true;
  }

  return false;
}


void
varcircle_draw (gint jvar, datad *d, ggobid *gg)
{
  /*--  a single pixmap is shared among all variable circles --*/
  gint r = VAR_CIRCLE_DIAM/2;
  gint x,y;
  gboolean chosen = false;
  GList *l;
  splotd *s;
  splotd *sp = gg->current_splot;
  displayd *display;
  cpaneld *cpanel;

  if (sp == NULL || jvar < 0 || jvar >= d->ncols)
    return;  /*-- return --*/

  display = (displayd *) sp->displayptr;

  if (display == NULL || display->d != d)
    return;  /*-- return --*/

  cpanel = &display->cpanel;

  if (gg->selvarfg_GC == NULL) 
    init_var_GCs (d->varpanel_ui.da[jvar], gg);

  if (d->varpanel_ui.da_pix[jvar] == NULL)
    d->varpanel_ui.da_pix[jvar] = gdk_pixmap_new (
      d->varpanel_ui.da[jvar]->window,
      VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1, -1);

  /*-- clear the pixmap --*/
  gdk_draw_rectangle (d->varpanel_ui.da_pix[jvar], gg->unselvarbg_GC, true,
                      0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  /*-- add a filled circle for the background --*/
  gdk_draw_arc (d->varpanel_ui.da_pix[jvar], gg->selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);

  /*-- add the appropriate line --*/
  switch (display->displaytype) {

    case  parcoords:  /* only one mode, a 1d plot */
      l = display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->p1dvar == jvar) {
          if (display->p1d_orientation == HORIZONTAL)
            gdk_draw_line (d->varpanel_ui.da_pix[jvar],
              gg->selvarfg_GC, r, r, r+r, r);
          else
            gdk_draw_line (d->varpanel_ui.da_pix[jvar],
              gg->selvarfg_GC, r, r, r, 0);
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
            if (display->p1d_orientation == HORIZONTAL)
              gdk_draw_line (d->varpanel_ui.da_pix[jvar],
                gg->selvarfg_GC, r, r, r+r, r);
            else
              gdk_draw_line (d->varpanel_ui.da_pix[jvar],
                gg->selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
          break;
        case XYPLOT:
          if (jvar == sp->xyvars.x) {
            gdk_draw_line (d->varpanel_ui.da_pix[jvar],
              gg->selvarfg_GC, r, r, r+r, r);
            chosen = true;
          } else if (jvar == sp->xyvars.y) {
            gdk_draw_line (d->varpanel_ui.da_pix[jvar],
              gg->selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
          break;
        case TOUR2D:
          x = (gint) (display->u[0][jvar]*(gfloat)r);
          y = (gint) (display->u[1][jvar]*(gfloat)r);
          gdk_draw_line (d->varpanel_ui.da_pix[jvar],
            gg->selvarfg_GC, r, r, r+x, r-y);
          chosen = true;
          break;
      }
      break;

    case  scatmat:
      l = display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->p1dvar == -1) {
          if (s->xyvars.x == jvar) {
            gdk_draw_line (d->varpanel_ui.da_pix[jvar],
              gg->selvarfg_GC, r, r, r+r, r);
            chosen = true;
          } else if (s->xyvars.y == jvar) {
            gdk_draw_line (d->varpanel_ui.da_pix[jvar],
              gg->selvarfg_GC, r, r, r, 0);
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
    gdk_draw_arc (d->varpanel_ui.da_pix[jvar],
      gg->selvarfg_GC, false,
      0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
  } else {
    gdk_draw_arc (d->varpanel_ui.da_pix[jvar],
      gg->unselvarfg_GC, false,
      0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
  }


  /*
   * copy the pixmap to the window
  */
  gdk_draw_pixmap (d->varpanel_ui.da[jvar]->window, gg->unselvarfg_GC,
    d->varpanel_ui.da_pix[jvar], 0, 0, 0, 0,
    VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
}

void tour_draw_circles (datad *d, ggobid *gg)
{
  gint j;

  for (j=0; j<d->ncols; j++) {
    varcircle_draw (j, d, gg);
  }
}

gboolean
da_expose_cb (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint j = GPOINTER_TO_INT (cbd);
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (w), "datad");

  /* are there two gg assignments here? */
  /*gg = ggobi_get (0);*/

  if (d->varpanel_ui.da_pix[j] == NULL)
    varcircle_draw (j, d, gg); 
  else
    gdk_draw_pixmap (d->varpanel_ui.da[j]->window, gg->unselvarfg_GC,
      d->varpanel_ui.da_pix[j], 0, 0, 0, 0,
      VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  return true;
}


/*-------------------------------------------------------------------------*/
/*                  initialize and populate the var panel                  */
/*-------------------------------------------------------------------------*/

/*-- build the scrolled window and vbox; the d-specific parts follow --*/
void
varpanel_make (GtkWidget *parent, ggobid *gg) {

  gg->selvarfg_GC = NULL;

  gg->varpanel_ui.varpanel_accel_group = gtk_accel_group_new ();

  /*-- create a scrolled window --*/
  gg->varpanel_ui.scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent),
    gg->varpanel_ui.scrolled_window, true, true, 2);
  gtk_widget_show (gg->varpanel_ui.scrolled_window);

  /*-- create varpanel, a vbox, and add it to the scrolled window --*/
  gg->varpanel_ui.varpanel = gtk_vbox_new (false, 10);
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    gg->varpanel_ui.varpanel);

  gtk_widget_show_all (gg->varpanel_ui.scrolled_window);
}


static void
varcircle_add (gint i, gint j, gint k, datad *d, ggobid *gg)
{
  GtkWidget *vb;

  vb = gtk_vbox_new (false, 0);
  gtk_container_border_width (GTK_CONTAINER (vb), 1);
  gtk_widget_show (vb);

  d->varpanel_ui.label[k] =
    gtk_button_new_with_label (d->vartable[k].collab);

  gtk_widget_show (d->varpanel_ui.label[k]);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    d->varpanel_ui.label[k], "Click left to select", NULL);
  gtk_container_add (GTK_CONTAINER (vb), d->varpanel_ui.label[k]);

  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.label[k]),
    "button_press_event",
    GTK_SIGNAL_FUNC (popup_varmenu), GINT_TO_POINTER (k));

  gtk_object_set_data (GTK_OBJECT (d->varpanel_ui.label[k]), "datad", d);
  GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.label[k]), gg, true);

  /*
   * a drawing area to contain the variable circle
  */
  d->varpanel_ui.da[k] = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (d->varpanel_ui.da[k]),
                         VAR_CIRCLE_DIAM+2, VAR_CIRCLE_DIAM+2);
  gtk_widget_set_events (d->varpanel_ui.da[k], GDK_EXPOSURE_MASK
             | GDK_ENTER_NOTIFY_MASK
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_BUTTON_RELEASE_MASK);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    d->varpanel_ui.da[k], "Click to select; see menu", NULL);

  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.da[k]), "expose_event",
    GTK_SIGNAL_FUNC (da_expose_cb), GINT_TO_POINTER (k));
  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.da[k]), "button_press_event",
    GTK_SIGNAL_FUNC (varsel_cb), GINT_TO_POINTER (k));

  gtk_object_set_data (GTK_OBJECT (d->varpanel_ui.da[k]), "datad", d);
  GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.da[k]), gg, true);

  gtk_widget_show (d->varpanel_ui.da[k]);
  gtk_container_add (GTK_CONTAINER (vb), d->varpanel_ui.da[k]);
  gtk_table_attach (GTK_TABLE (d->varpanel_ui.table), vb, j, j+1, i, i+1,
    GTK_FILL, GTK_FILL, 0, 0);
}

void
vartable_clear (datad *d, ggobid *gg) {
  gint j;

  for (j=0; j<d->varpanel_ui.nvars; j++) {
    gtk_widget_destroy (d->varpanel_ui.da[j]);
    gtk_widget_destroy (d->varpanel_ui.label[j]);
  }

  /*-- free when they've been alloc'ed before --*/
  g_free (d->varpanel_ui.da);
  g_free (d->varpanel_ui.label);
}

void
varpanel_clear (ggobid *gg) {
  GSList *l;
  datad *d;
  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    vartable_clear (d, gg);
  }
}


/*-- create a grid of buttons in the table --*/
void vartable_populate (datad *d, ggobid *gg)
{
  gint i, j, k;
  GtkWidget *frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (gg->varpanel_ui.varpanel),
                      frame, true, true, 2);
  gtk_widget_show (frame);
  
  d->varpanel_ui.table = gtk_table_new (d->varpanel_ui.vnrows,
                                        d->varpanel_ui.vncols, true);
  gtk_container_add (GTK_CONTAINER (frame), d->varpanel_ui.table);

  /*-- da and label are freed in varpanel_clear --*/

  d->varpanel_ui.da = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));

  d->varpanel_ui.da_pix = (GdkPixmap **)
    g_malloc (d->ncols * sizeof (GdkPixmap *));
  for (j=0; j<d->ncols; j++) d->varpanel_ui.da_pix[j] = NULL;

  d->varpanel_ui.label = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));

  k = 0;
  for (i=0; i<d->varpanel_ui.vnrows; i++) {
    for (j=0; j<d->varpanel_ui.vncols; j++) {
      varcircle_add (i, j, k, d, gg);
      k++;
      if (k == d->ncols) break;
    }
  }

  gtk_widget_show_all (d->varpanel_ui.table);
}

/*
 * Make the variable panel as square as possible, and lay it out
 * row-wise.
*/
void
vartable_layout_init (datad *d, ggobid *gg) {

  d->varpanel_ui.vnrows = (gint) sqrt ((gdouble) d->ncols);
  d->varpanel_ui.vncols = d->varpanel_ui.vnrows;

  while (d->varpanel_ui.vnrows*d->varpanel_ui.vncols < d->ncols) {
    d->varpanel_ui.vnrows++;
    if (d->varpanel_ui.vnrows*d->varpanel_ui.vncols < d->ncols)
      d->varpanel_ui.vncols++;
  }
}


void
varpanel_size_init (gint cpanel_height, ggobid* gg)
{
/*
  gint i;
  GtkTable *t = GTK_TABLE (gg->varpanel_ui.varpanel);
  GtkTableRowCol c, r;
  gint width = 0, height = 0;
  GtkWidget *vport = GTK_WIDGET
    ((GTK_BIN (gg->varpanel_ui.scrolled_window))->child);
*/

  /*-- Find the width of the first few columns --*/
/*
  for (i=0; i<MIN (t->ncols, 3); i++) {
    c = t->cols[i];
    width += c.requisition + c.spacing;
  }
*/

  /*-- Find the height of the first few rows --*/
/*
  for (i=0; i<MIN (t->nrows, 4); i++) {
    r = t->rows[i];
    height += r.requisition + r.spacing;
  }

  gtk_widget_set_usize (vport, width, MAX (height, cpanel_height));
*/
}

void
vartable_refresh (datad *d, ggobid *gg) {
  gint j;

  for (j=0; j<d->ncols; j++) {
    if (GTK_WIDGET_REALIZED (d->varpanel_ui.da[j])) {
      varcircle_draw (j, d, gg);
/*    gtk_widget_queue_draw (d->varpanel_ui.da[j]);*/
    }
  }
}

void
varlabel_set (gint j, datad *d, ggobid *gg) {
  gtk_label_set_text (GTK_LABEL (GTK_BIN (d->varpanel_ui.label[j])->child),
    d->vartable[j].collab_tform);
}

/*-------------------------------------------------------------------------*/
/*                          API; not used                                  */
/*-------------------------------------------------------------------------*/

void
GGOBI(selectScatterplotX) (gint jvar, ggobid *gg) 
{
  displayd *display = gg->current_display;
  if (display->displaytype != scatterplot)
    return;
  else {

    datad *d = display->d;
    splotd *sp = (splotd *) display->splots->data;
    cpaneld *cpanel = &display->cpanel;

    varsel (cpanel, sp, jvar, 1, false, false, false, d, gg);
  }
}

