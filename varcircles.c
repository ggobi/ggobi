#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define VAR_CIRCLE_DIAM 40

static void varcircle_create (gint, datad *, ggobid *gg);
static void varcircle_attach (gint, gint, gint, datad *);
static void varcircle_draw (gint, datad *, ggobid *gg); 

static gboolean da_expose_cb (GtkWidget *, GdkEventExpose *, gpointer cbd);

/*
 * This is unfortunately needed for now because no widgets have
 * been realized yet when this is first called.
 * Now that ..._reset has an escape hatch, it might not be needed.
 *
 * I'm not sure which layout should be first; this does it row-wise.
*/
void
varcircles_layout_init (datad *d, ggobid *gg) {
  gint tncols, tnrows;
  gint NCOLS = 5;

  tncols = MIN (NCOLS, d->ncols);
  tnrows = d->ncols / tncols;
  if (tnrows * tncols < d->ncols) tnrows++;

  d->varpanel_ui.tncols = tncols;
  d->varpanel_ui.tnrows = tnrows;
}

void
varcircles_layout_reset (gint ncols, datad *d, ggobid *gg) {
  gint j;
  GtkWidget *vb;
  gint tnrows, tncols;
  gint left_attach, top_attach;
  GtkAdjustment *adj;

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->varpanel_ui.swin),
    (gg->varpanel_ui.layoutByRow) ? GTK_POLICY_NEVER : GTK_POLICY_ALWAYS,
    (gg->varpanel_ui.layoutByRow) ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER
    );
  gdk_flush();


  if (gg->varpanel_ui.layoutByRow) {
    gint vport_width, vb_width;

    adj = gtk_scrolled_window_get_hadjustment (
      GTK_SCROLLED_WINDOW (d->varpanel_ui.swin));
    vport_width = adj->page_size;

    vb_width = d->varpanel_ui.vb[0]->allocation.width;
    if (vb_width < 5) vb_width = VAR_CIRCLE_DIAM;

    tncols = MIN (vport_width / vb_width, ncols);
    tnrows = ncols / tncols;
    if (tnrows * tncols < ncols) tnrows++;

  } else {
    gint vport_height, vb_height;

    adj = gtk_scrolled_window_get_vadjustment (
      GTK_SCROLLED_WINDOW (d->varpanel_ui.swin));
    vport_height = adj->page_size;

    vb_height = d->varpanel_ui.vb[0]->allocation.height;
    if (vb_height < 5) vb_height = VAR_CIRCLE_DIAM;  /*-- a bit low ... --*/

    tnrows = MIN (vport_height / vb_height, ncols);
    tncols = ncols / tnrows;
    if (tnrows * tnrows < ncols) tncols++;
  }
  d->varpanel_ui.tncols = tncols;
  d->varpanel_ui.tnrows = tnrows;


  for (j=0; j<ncols; j++) {
    /*-- if they're in the container, ref and remove them --*/
    if (d->varpanel_ui.vb[j]->parent != NULL &&
        d->varpanel_ui.vb[j]->parent == d->varpanel_ui.table)
    {
      gtk_widget_ref (d->varpanel_ui.vb[j]);
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.table),
                            d->varpanel_ui.vb[j]);
    }
  }

  gtk_table_resize (GTK_TABLE (d->varpanel_ui.table),
                    d->varpanel_ui.tnrows, d->varpanel_ui.tncols);

  left_attach = top_attach = 0;
  for (j=0; j<ncols; j++) {
    vb = d->varpanel_ui.vb[j];

    /*-- attach the first one at 0,0 --*/
    gtk_table_attach (GTK_TABLE (d->varpanel_ui.table),
      d->varpanel_ui.vb[j],
      left_attach, left_attach+1, top_attach, top_attach+1,
      GTK_FILL, GTK_FILL, 0, 0);
    if (GTK_OBJECT (d->varpanel_ui.vb[j])->ref_count > 1)
      gtk_widget_unref (d->varpanel_ui.vb[j]);

    if (gg->varpanel_ui.layoutByRow) {
      left_attach++; 
      if (left_attach == d->varpanel_ui.tncols) {
        left_attach = 0;
        top_attach++;
      }
    } else {
      top_attach++; 
      if (top_attach == d->varpanel_ui.tnrows) {
        top_attach = 0;
        left_attach++;
      }
    }
  }
}

/*-- create a grid of buttons in the table --*/
void varcircles_populate (datad *d, ggobid *gg)
{
  gint i, j, k;

  varcircles_layout_init (d, gg);

  d->varpanel_ui.table = gtk_table_new (d->varpanel_ui.tnrows,
                                        d->varpanel_ui.tncols, true);

  /*-- da and label are freed in varcircle_clear --*/
  d->varpanel_ui.vb = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));
  d->varpanel_ui.da = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));
  d->varpanel_ui.label = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));

  d->varpanel_ui.da_pix = (GdkPixmap **)
    g_malloc (d->ncols * sizeof (GdkPixmap *));


  k = 0;
  for (i=0; i<d->varpanel_ui.tnrows; i++) {
    for (j=0; j<d->varpanel_ui.tncols; j++) {
      varcircle_create (k, d, gg);
      varcircle_attach (i, j, k, d);
      k++;
      if (k == d->ncols) break;
    }
  }

  gtk_widget_show_all (d->varpanel_ui.table);
}

void
varcircles_delete (gint nc, gint jcol, datad *d, ggobid *gg) {
  gint j;

  if (nc > 0 && nc < d->ncols) {  /*-- forbid deleting every circle --*/
    for (j=jcol; j<jcol+nc; j++) {
      /*-- without a ref, this will be destroyed --*/
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.table),
                            d->varpanel_ui.vb[j]);
      gdk_pixmap_unref (d->varpanel_ui.da_pix[j]);  /*-- or g_free? --*/
    }
  }

  /*-- this may not be enough; time will tell --*/
  varcircles_layout_reset (d->ncols, d, gg);
}

void
varcircles_clear (ggobid *gg) {
  gint j;
  GSList *l;
  datad *d;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    for (j=0; j<d->varpanel_ui.nvars; j++) {
      gtk_widget_destroy (d->varpanel_ui.da[j]);
      gtk_widget_destroy (d->varpanel_ui.label[j]);
      gtk_widget_destroy (d->varpanel_ui.vb[j]);
    }
    /*-- free when they've been alloc'ed before --*/
    g_free (d->varpanel_ui.da);
    g_free (d->varpanel_ui.label);
    g_free (d->varpanel_ui.vb);
  }
}

/*-- responds to a button_press_event --*/
static gint
varcircle_sel_cb (GtkWidget *w, GdkEvent *event, gint jvar)
{
  ggobid *gg = GGobiFromWidget (w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  splotd *sp = gg->current_splot;
  datad *d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);

  if (d != display->d)
    return true;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint button = bevent->button;
    gboolean alt_mod, shift_mod, ctrl_mod;

    /*-- respond only to button 1 and button 2 --*/
    if (button != 1 && button != 2)
      return false;

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, d, gg);
    varcircles_refresh (d, gg);
    return true;
  }

  return false;
}

static void
varcircle_create (gint k, datad *d, ggobid *gg)
{
  d->varpanel_ui.da_pix[k] = NULL;

  d->varpanel_ui.vb[k] = gtk_vbox_new (false, 0);
  gtk_container_border_width (GTK_CONTAINER (d->varpanel_ui.vb[k]), 1);

  d->varpanel_ui.label[k] = gtk_button_new_with_label (d->vartable[k].collab);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    d->varpanel_ui.label[k], "Click left to select", NULL);
  gtk_object_set_data (GTK_OBJECT (d->varpanel_ui.label[k]), "datad", d);
  GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.label[k]), gg, true);
  gtk_container_add (GTK_CONTAINER (d->varpanel_ui.vb[k]),
    d->varpanel_ui.label[k]);

/*
  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.label[k]),
    "button_press_event",
    GTK_SIGNAL_FUNC (popup_varmenu), GINT_TO_POINTER (k));
*/


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
    GTK_SIGNAL_FUNC (varcircle_sel_cb), GINT_TO_POINTER (k));
  gtk_object_set_data (GTK_OBJECT (d->varpanel_ui.da[k]), "datad", d);
  GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.da[k]), gg, true);
  gtk_container_add (GTK_CONTAINER (d->varpanel_ui.vb[k]),
    d->varpanel_ui.da[k]);

  gtk_widget_show_all (d->varpanel_ui.vb[k]);
}

void
varcircle_attach (gint i, gint j, gint k, datad *d)
{
  gtk_table_attach (GTK_TABLE (d->varpanel_ui.table),
    d->varpanel_ui.vb[k], j, j+1, i, i+1,
    GTK_FILL, GTK_FILL, 0, 0);
}

void
varcircles_refresh (datad *d, ggobid *gg) {
  gint j;

  for (j=0; j<d->ncols; j++) {
    if (GTK_WIDGET_REALIZED (d->varpanel_ui.da[j])) {
      varcircle_draw (j, d, gg);
    }
  }
}

void
varcircle_draw (gint jvar, datad *d, ggobid *gg)
{
  gint r = VAR_CIRCLE_DIAM/2;
  gint x,y;
  gboolean chosen = false;
  splotd *sp = gg->current_splot;
  displayd *display;
  cpaneld *cpanel;
  gint k;

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

    case  scatterplot:
      switch (cpanel->projection) {
        case TOUR1D:
          x = 0;
          y = (gint) (display->u[0][jvar]*(gfloat)r);
          gdk_draw_line (d->varpanel_ui.da_pix[jvar],
            gg->selvarfg_GC, r, r, r+x, r-y);

          for (k=0; k<display->ntour_vars; k++) {
            if (display->tour_vars[k] == jvar) {
              chosen = true;
              break;
            }
          }
          break;
        case TOUR2D:
          x = (gint) (display->u[0][jvar]*(gfloat)r);
          y = (gint) (display->u[1][jvar]*(gfloat)r);
          gdk_draw_line (d->varpanel_ui.da_pix[jvar],
            gg->selvarfg_GC, r, r, r+x, r-y);

          for (k=0; k<display->ntour_vars; k++) {
            if (display->tour_vars[k] == jvar) {
              chosen = true;
              break;
            }
          }
          break;
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

  if (d->varpanel_ui.da_pix[j] == NULL)
    varcircle_draw (j, d, gg); 
  else
    gdk_draw_pixmap (d->varpanel_ui.da[j]->window, gg->unselvarfg_GC,
      d->varpanel_ui.da_pix[j], 0, 0, 0, 0,
      VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  return true;
}

/*-- used in cloning and appending variables; see vartable.c --*/
void
varcircles_add (gint nc, datad *d, ggobid *gg) 
{
  gint j;
  
  d->varpanel_ui.vb = (GtkWidget **)
    g_realloc (d->varpanel_ui.vb, nc * sizeof (GtkWidget *));
  d->varpanel_ui.da = (GtkWidget **)
    g_realloc (d->varpanel_ui.da, nc * sizeof (GtkWidget *));
  d->varpanel_ui.da_pix = (GdkPixmap **)
    g_realloc (d->varpanel_ui.da_pix, nc * sizeof (GdkPixmap *));
  d->varpanel_ui.label = (GtkWidget **)
    g_realloc (d->varpanel_ui.label, nc * sizeof (GtkWidget *));

  /*-- create the variable circles --*/
  for (j=d->ncols; j<nc; j++)
    varcircle_create (j, d, gg);
  varcircles_layout_reset (nc, d, gg);

  gtk_widget_show_all (gg->varpanel_ui.notebook);
}

/*-------------------------------------------------------------------------*/
/*                     Variable menus                                      */
/*-------------------------------------------------------------------------*/

/*
static void
varsel_from_menu (GtkWidget *w, gpointer data)
{
  varseldatad *vdata = (varseldatad *) data;
  ggobid *gg = vdata->gg;
  displayd *display = gg->current_display;
  datad *d = display->d;
  cpaneld *cpanel = &display->cpanel;

  *-- I think the menu should be destroyed here. --*
  gtk_widget_destroy (w->parent);

  varsel (cpanel, gg->current_splot, vdata->jvar, vdata->btn,
    vdata->alt_mod, vdata->ctrl_mod, vdata->shift_mod, d, gg);
}
*/

/*
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
  gtk_object_set_data (GTK_OBJECT (menu), "top", d->varpanel_ui.checkbox[jvar]);

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
*/

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

/*
static gint
popup_varmenu (GtkWidget *w, GdkEvent *event, gpointer cbd) 
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel;
  gint jvar = GPOINTER_TO_INT (cbd);
  GtkWidget *tour_menu;
  *-- w  is the variable label --*
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (w), "datad");

  if (display == NULL)
    return false;
  if (display->d != d)  *-- only select for the current plot --*
    return false;

  cpanel = &display->cpanel;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button == 1) {
      gint projection = projection_get (gg);

      switch (display->displaytype) {

        case scatterplot:
          switch (projection) {
            case TOUR1D:
              tour_menu = tour1d_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL,
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
        case parcoords:
          break;

      }
      return true;
    }
  }
  return false;
}
*/

/*-- from ggobid --*/
/*----------------- variable selection menus -------------------------*/

 struct {
  varseldatad vdata0, vdata1, vdata2;
 } tour2d_menu;

/*--------------------------------------------------------------------*/

