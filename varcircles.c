#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define VAR_CIRCLE_DIAM 40

extern void varpanel_add_data (datad *, ggobid *);
extern void varpanel_clear (ggobid *);
extern void varpanel_size_init (gint, ggobid *);

static void varcircle_add (gint, gint, gint, datad *, ggobid *gg);
static void varcircle_draw (gint, datad *, ggobid *gg); 

static gboolean da_expose_cb (GtkWidget *, GdkEventExpose *, gpointer cbd);

/*
 * Just to get going, use a fixed value: vncols = 5
 *
 * lay it out row-wise.
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

/*-- create a grid of buttons in the table --*/
void varcircles_populate (datad *d, ggobid *gg)
{
  gint i, j, k;

  varcircles_layout_init (d, gg);

  d->varpanel_ui.table = gtk_table_new (d->varpanel_ui.tnrows,
                                        d->varpanel_ui.tncols, true);
/*
  gtk_container_add (GTK_CONTAINER (frame), d->varpanel_ui.table);
*/

  /*-- da and label are freed in varpanel_clear --*/
  d->varpanel_ui.da = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));

  d->varpanel_ui.da_pix = (GdkPixmap **)
    g_malloc (d->ncols * sizeof (GdkPixmap *));
  for (j=0; j<d->ncols; j++) d->varpanel_ui.da_pix[j] = NULL;

  d->varpanel_ui.label = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));

  k = 0;
  for (i=0; i<d->varpanel_ui.tnrows; i++) {
    for (j=0; j<d->varpanel_ui.tncols; j++) {
      varcircle_add (i, j, k, d, gg);
      k++;
      if (k == d->ncols) break;
    }
  }

  gtk_widget_show_all (d->varpanel_ui.table);
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
varcircles_clear (ggobid *gg) {
  gint j;
  GSList *l;
  datad *d;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    for (j=0; j<d->varpanel_ui.nvars; j++) {
      gtk_widget_destroy (d->varpanel_ui.da[j]);
      gtk_widget_destroy (d->varpanel_ui.label[j]);
    }
    /*-- free when they've been alloc'ed before --*/
    g_free (d->varpanel_ui.da);
    g_free (d->varpanel_ui.label);
  }
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

/*
  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.label[k]),
    "button_press_event",
    GTK_SIGNAL_FUNC (popup_varmenu), GINT_TO_POINTER (k));
*/

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
/*
  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.da[k]), "button_press_event",
    GTK_SIGNAL_FUNC (varsel_cb), GINT_TO_POINTER (k));
*/

  gtk_object_set_data (GTK_OBJECT (d->varpanel_ui.da[k]), "datad", d);
  GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.da[k]), gg, true);

  gtk_widget_show (d->varpanel_ui.da[k]);
  gtk_container_add (GTK_CONTAINER (vb), d->varpanel_ui.da[k]);
  gtk_table_attach (GTK_TABLE (d->varpanel_ui.table), vb, j, j+1, i, i+1,
    GTK_FILL, GTK_FILL, 0, 0);
}

void
vartable_refresh (datad *d, ggobid *gg) {
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
  /*--  a single pixmap is shared among all variable circles --*/
  gint r = VAR_CIRCLE_DIAM/2;
  gint x,y;
  gboolean chosen = false;
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

    case  scatterplot:
      switch (cpanel->projection) {
        case TOUR2D:
          x = (gint) (display->u[0][jvar]*(gfloat)r);
          y = (gint) (display->u[1][jvar]*(gfloat)r);
          gdk_draw_line (d->varpanel_ui.da_pix[jvar],
            gg->selvarfg_GC, r, r, r+x, r-y);
          chosen = true;
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

void
varcircle_clone (gint jvar, const gchar *newName, gboolean update,
  datad *d, ggobid *gg) 
{
  gint nc = d->ncols + 1;
  gint i, j, k = 0;
  
  /*
   * Follow the algorithm by which the table has been populated
  if (d->varpanel_ui.tnrows*d->varpanel_ui.vncols <= d->ncols) {
    d->varpanel_ui.tnrows++;
    gtk_table_resize (GTK_TABLE (d->varpanel_ui.table),
                      d->varpanel_ui.tnrows, d->varpanel_ui.vncols);
  }
  */

  k = 0;
  for (i=0; i<d->varpanel_ui.tnrows; i++) {
    for (j=0; j<d->varpanel_ui.tncols; j++) {
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
/*        varcircle_add (i, j, k, d, gg);*/
      }
      k++;
      if (k == d->ncols+1) break;
    }
  }

  gtk_widget_show_all (gg->varpanel_ui.notebook);
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
/*
  datad *d = display->d;
  cpaneld *cpanel = &display->cpanel;
*/

  /*-- I think the menu should be destroyed here. --*/
  gtk_widget_destroy (w->parent);

/*
  varsel (cpanel, gg->current_splot, vdata->jvar, vdata->btn,
    vdata->alt_mod, vdata->ctrl_mod, vdata->shift_mod, d, gg);
*/
}

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

static gint
popup_varmenu (GtkWidget *w, GdkEvent *event, gpointer cbd) 
{
  ggobid *gg = GGobiFromWidget(w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel;
  gint jvar = GPOINTER_TO_INT (cbd);
  GtkWidget *tour_menu;
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
            case TOUR2D:
            case COTOUR:
/*
              tour_menu = tour2d_menu_build (jvar, d, gg);
              gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL,
                position_popup_menu, NULL,
                bevent->button, bevent->time);
*/
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

/*-- from ggobid --*/
/*----------------- variable selection menus -------------------------*/

 struct {
  varseldatad vdata0, vdata1, vdata2;
 } tour2d_menu;

/*--------------------------------------------------------------------*/

