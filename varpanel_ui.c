/* varpanel_ui.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#include "vars.h"
#include "externs.h"

#define VAR_CIRCLE_DIAM 40

static GtkWidget *varpanel = NULL;
static GtkWidget *scrolled_window;
static GtkWidget **da, **varlabel;
static GtkAccelGroup *varpanel_accel_group;

static gint vnrows, vncols;

/* */
static void varcircle_add (gint, gint, gint);
static void varcircle_draw (gint); 
/* */

/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint btn,
  gint alt_mod, gint ctrl_mod, gint shift_mod)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw;
  gint jvar_prev = -1;

  switch (display->displaytype) {

    case parcoords:
      redraw = parcoords_varsel (cpanel, sp, jvar, &jvar_prev, alt_mod);
      break;

    case scatmat:
      redraw = scatmat_varsel (cpanel, sp, jvar, &jvar_prev, btn, alt_mod);
      break;

      case scatterplot:
        switch (cpanel->projection) {
          case P1PLOT:
            redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
            break;
          case XYPLOT:
            redraw = xyplot_varsel (sp, jvar, &jvar_prev, btn);
            break;
          default:
            break;
      }
      break;
  }

  varcircle_draw (jvar);
  if (jvar_prev != -1)
    varcircle_draw (jvar_prev);

  /*-- overkill for scatmat: could redraw one row, one column --*/
  /*-- overkill for parcoords: need to redraw at most 3 plots --*/
/* this is redrawing before it has the new window sizes, so the
 * lines aren't right */
  if (redraw) {
    display_reproject (display);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Variable menus                                      */
/*-------------------------------------------------------------------------*/

typedef struct {
  splotd *sp;
  gint jvar;
  gint btn;       /*-- emulate button press --*/
  gint alt_mod;   /*-- emulate the alt key --*/
  gint shift_mod; /*-- emulate the shift key --*/
  gint ctrl_mod;  /*-- emulate the control key --*/
} varseldatad;

static void
varsel_from_menu (GtkWidget *w, gpointer data)
{
  varseldatad *vdata = (varseldatad *) data;
  cpaneld *cpanel = &current_display->cpanel;

  varsel (cpanel, current_splot, vdata->jvar, vdata->btn,
    vdata->alt_mod, vdata->ctrl_mod, vdata->shift_mod);
}

GtkWidget *
p1d_menu_build (gint jvar)
{
  GtkWidget *menu;
  static varseldatad vdata0, vdata1;

  vdata0.sp = vdata1.sp = current_splot;
  vdata0.jvar = vdata1.jvar = jvar;
  vdata0.alt_mod = vdata1.alt_mod = false;

  vdata0.btn = 1;
  vdata1.btn = 2;

  menu = gtk_menu_new ();

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata0);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata1);

  return menu;
}

GtkWidget *
xyplot_menu_build (gint jvar)
{
  GtkWidget *menu;
  static varseldatad vdata0, vdata1;

  vdata0.sp = vdata1.sp = current_splot;
  vdata0.jvar = vdata1.jvar = jvar;
  vdata0.alt_mod = vdata1.alt_mod = false;

  vdata0.btn = 1;
  vdata1.btn = 2;

  menu = gtk_menu_new ();

  CreateMenuItem (menu, "Select X    L",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata0);

  CreateMenuItem (menu, "Select Y    M,R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata1);

  return menu;
}

GtkWidget *
rotation_menu_build (gint jvar)
{
  GtkWidget *menu;
  static varseldatad vdata0, vdata1, vdata2;

  vdata0.sp = vdata1.sp = vdata2.sp = current_splot;
  vdata0.jvar = vdata1.jvar = vdata2.jvar = jvar;
  vdata0.alt_mod = vdata1.alt_mod = vdata2.alt_mod = false;

  vdata0.btn = 1;
  vdata1.btn = 2;
  vdata2.btn = 3;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Select X  L",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata0);
  CreateMenuItem (menu, "Select Y  M",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata1);
  CreateMenuItem (menu, "Select Z  R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata2);

  return menu;
}

GtkWidget *
gtour_menu_build (gint jvar)
{
  GtkWidget *menu;
  static varseldatad vdata0, vdata1, vdata2;

  vdata0.sp = vdata1.sp = vdata2.sp = current_splot;
  vdata0.jvar = vdata1.jvar = vdata2.jvar = jvar;
  vdata0.alt_mod = vdata1.alt_mod = vdata2.alt_mod = false;
  vdata0.shift_mod = vdata2.shift_mod = false;
  vdata1.shift_mod = true;
  vdata0.ctrl_mod = vdata1.ctrl_mod = false;
  vdata2.ctrl_mod = true;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Tour   L,M",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata0);
  CreateMenuItem (menu, "Manip  <Shift> L,M",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata1);
  CreateMenuItem (menu, "Freeze <Ctrl> L,M",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata2);

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
parcoords_menu_build (gint jvar)
{
  GtkWidget *menu;
  static varseldatad vdata0, vdata1;

  vdata0.sp = vdata1.sp = current_splot;
  vdata0.jvar = vdata1.jvar = jvar;
  vdata0.alt_mod = false;
  vdata1.alt_mod = true;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Select Y      M,R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata0);
  CreateMenuItem (menu, "Delete Y <alt>M,R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata1);

  return menu;
}

GtkWidget *
scatmat_menu_build (gint jvar)
{
  GtkWidget *menu;
  static varseldatad vdata0, vdata1, vdata2, vdata3;

  vdata0.sp = vdata1.sp = vdata2.sp = vdata3.sp = current_splot;
  vdata0.jvar = vdata1.jvar = vdata2.jvar = vdata3.jvar = jvar;
  vdata0.alt_mod = vdata1.alt_mod = false;
  vdata2.alt_mod = vdata3.alt_mod = 3;

  vdata0.btn = vdata2.btn = 1;
  vdata1.btn = vdata3.btn = 2;

  menu = gtk_menu_new ();
  CreateMenuItem (menu, "Select row  L",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata0);
  CreateMenuItem (menu, "Select col  M,R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata1);
  CreateMenuItem (menu, "Delete row  <alt>L",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata2);
  CreateMenuItem (menu, "Delete col  <alt>M,R",
    NULL, NULL, varpanel, varpanel_accel_group,
    GTK_SIGNAL_FUNC (varsel_from_menu), (gpointer) &vdata3);

  return menu;
}

static gint
popup_varmenu (GtkWidget *w, GdkEvent *event, gpointer cbd) {
  displayd *display = current_display;
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
      gint projection = projection_get ();

      switch (display->displaytype) {

        case scatterplot:
          switch (projection) {
            case P1PLOT:
              p1d_menu = p1d_menu_build (jvar);
              gtk_menu_popup (GTK_MENU (p1d_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
            case XYPLOT:
              xyplot_menu = xyplot_menu_build (jvar);
              gtk_menu_popup (GTK_MENU (xyplot_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
            case ROTATE:
              rotation_menu = rotation_menu_build (jvar);
              gtk_menu_popup (GTK_MENU (rotation_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
            case GRTOUR:
            case COTOUR:
              tour_menu = gtour_menu_build (jvar);
              gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL, NULL, NULL,
                bevent->button, bevent->time);
              break;
          }
          break;

        case scatmat:
          scatmat_menu = scatmat_menu_build (jvar);
          gtk_menu_popup (GTK_MENU (scatmat_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;

        case parcoords:
          parcoords_menu = parcoords_menu_build (jvar);
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

static void
variable_clone (gint jvar) {
  gint nc = xg.ncols + 1;
  gint i, j, k = 0;

  pipeline_arrays_add_column (jvar);  /* reallocate and copy */
  missing_arrays_add_column (jvar);

  vardata_realloc (nc);

  xg.vardata[nc-1].collab = g_strdup (xg.vardata[jvar].collab);
  xg.vardata[nc-1].collab_tform = g_strdup (xg.vardata[jvar].collab_tform);

  xg.vardata[nc-1].groupid = xg.vardata[nc-1].groupid_ori =
    xg.vardata[xg.ncols].groupid + 1; 

  xg.vardata[nc-1].mean = xg.vardata[jvar].mean;
  xg.vardata[nc-1].median = xg.vardata[jvar].median;
  xg.vardata[nc-1].lim_raw.min = xg.vardata[jvar].lim_raw.min;
  xg.vardata[nc-1].lim_raw.max = xg.vardata[jvar].lim_raw.max;

  xg.vardata[nc-1].tform1 = xg.vardata[jvar].tform1;
  xg.vardata[nc-1].tform2 = xg.vardata[jvar].tform2;
  xg.vardata[nc-1].domain_incr = xg.vardata[jvar].domain_incr;
  xg.vardata[nc-1].param = xg.vardata[jvar].param;
  xg.vardata[nc-1].domain_adj = xg.vardata[jvar].domain_adj;
  xg.vardata[nc-1].inv_domain_adj = xg.vardata[jvar].inv_domain_adj;

  xg.vardata[nc-1].jitter_factor = xg.vardata[jvar].jitter_factor;


  /*
   * Follow the algorithm by which the table
   * has been populated
  */
  if (vnrows*vncols <= xg.ncols) {
    vnrows++;
    gtk_table_resize (GTK_TABLE (varpanel), vnrows, vncols);
  }

  k = 0;
  for (i=0; i<vnrows; i++) {
    for (j=0; j<vncols; j++) {
      if (k < xg.ncols)
        ;
      else {
        da = (GtkWidget **) g_realloc (da,
          (xg.ncols+1) * sizeof (GtkWidget *));
        varlabel = (GtkWidget **) g_realloc (varlabel,
          (xg.ncols+1) * sizeof (GtkWidget *));
        varcircle_add (i, j, k);
      }
      k++;
      if (k == xg.ncols+1) break;
    }
  }

  gtk_widget_show_all (varpanel);
  xg.ncols++;

  vartable_row_append (xg.ncols-1);
}


static gint
varsel_cb (GtkWidget *w, GdkEvent *event, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  splotd *sp = current_splot;
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
      variable_clone (jvar);
      return (false);
    }
    
    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod);
    return true;
  }

  return false;
}


void
varcircle_draw (gint jvar)
{
  /*--  a single pixmap is shared among all variable circles --*/
  static GdkPixmap *vpixmap = NULL;
  gint r = VAR_CIRCLE_DIAM/2;
  cpaneld *cpanel = &current_display->cpanel;
  splotd *sp = current_splot;
  gboolean chosen = false;
  GList *l;
  splotd *s;

  if (current_splot == NULL || jvar < 0 || jvar >= xg.ncols)
    return;  /*-- return --*/

  if (selvarfg_GC == NULL) 
    init_var_GCs (da[jvar]);

  if (vpixmap == NULL) {
    vpixmap = gdk_pixmap_new (da[jvar]->window,
              VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1, -1);
  }

  /*-- clear the pixmap --*/
  gdk_draw_rectangle (vpixmap, unselvarbg_GC, true,
                      0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  /*-- add a filled circle for the background --*/
  gdk_draw_arc (vpixmap, selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);

  /*-- add the appropriate line --*/
  switch (current_display->displaytype) {

    case  parcoords:  /* only one mode, a 1d plot */
      l = current_display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->p1dvar == jvar) {
          if (current_display->p1d_orientation == HORIZONTAL)
            gdk_draw_line (vpixmap, selvarfg_GC, r, r, r+r, r);
          else
            gdk_draw_line (vpixmap, selvarfg_GC, r, r, r, 0);
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
            if (current_display->p1d_orientation == HORIZONTAL)
              gdk_draw_line (vpixmap, selvarfg_GC, r, r, r+r, r);
            else
              gdk_draw_line (vpixmap, selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
          break;
        case XYPLOT:
          if (jvar == sp->xyvars.x) {
            gdk_draw_line (vpixmap, selvarfg_GC, r, r, r+r, r);
            chosen = true;
          } else if (jvar == sp->xyvars.y) {
            gdk_draw_line (vpixmap, selvarfg_GC, r, r, r, 0);
            chosen = true;
          }
          break;
      }
      break;

    case  scatmat:
      l = current_display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->p1dvar == -1) {
          if (s->xyvars.x == jvar) {
            gdk_draw_line (vpixmap, selvarfg_GC, r, r, r+r, r);
            chosen = true;
          } else if (s->xyvars.y == jvar) {
            gdk_draw_line (vpixmap, selvarfg_GC, r, r, r, 0);
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
    gdk_draw_arc (vpixmap, selvarfg_GC, false,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  } else {
    gdk_draw_arc (vpixmap, unselvarfg_GC, false,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  }


  /*
   * copy the pixmap to the window
  */
  gdk_draw_pixmap (da[jvar]->window, unselvarfg_GC, vpixmap, 0, 0, 0, 0,
    VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
}

gboolean
da_expose_cb (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  gint k = GPOINTER_TO_INT (cbd);
  varcircle_draw (k); 
  return true;
}


/*-------------------------------------------------------------------------*/
/*                                                                         */
/*-------------------------------------------------------------------------*/

static void
varcircle_add (gint i, gint j, gint k)
{
  GtkWidget *vb;

  vb = gtk_vbox_new (false, 0);
  gtk_container_border_width (GTK_CONTAINER (vb), 1);
  gtk_widget_show (vb);

  varlabel[k] = gtk_button_new_with_label (xg.vardata[k].collab);
/*
  gtk_object_set_data (GTK_OBJECT (varlabel[k]), "index", GINT_TO_POINTER (k));
*/
  gtk_widget_show (varlabel[k]);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
    varlabel[k], "Click left to select", NULL);
  gtk_container_add (GTK_CONTAINER (vb), varlabel[k]);

  gtk_signal_connect (GTK_OBJECT (varlabel[k]), "button_press_event",
    GTK_SIGNAL_FUNC (popup_varmenu), GINT_TO_POINTER (k));

  /*
   * a drawing area to contain the variable circle
  */

  da[k] = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (da[k]),
    VAR_CIRCLE_DIAM+2, VAR_CIRCLE_DIAM+2);
  gtk_widget_set_events (da[k], GDK_EXPOSURE_MASK
             | GDK_ENTER_NOTIFY_MASK
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_BUTTON_RELEASE_MASK);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
    da[k], "Click to select; see menu", NULL);

  gtk_signal_connect (GTK_OBJECT (da[k]), "expose_event",
    GTK_SIGNAL_FUNC (da_expose_cb), GINT_TO_POINTER (k));
  gtk_signal_connect (GTK_OBJECT (da[k]), "button_press_event",
    GTK_SIGNAL_FUNC (varsel_cb), GINT_TO_POINTER (k));

  gtk_widget_show (da[k]);
  gtk_container_add (GTK_CONTAINER (vb), da[k]);
  gtk_table_attach (GTK_TABLE (varpanel), vb, j, j+1, i, i+1,
    GTK_FILL, GTK_FILL, 0, 0);
}

void
make_varpanel (GtkWidget *parent) {
  gint i, j, k;

  selvarfg_GC = NULL;

  da = (GtkWidget **) g_malloc (xg.ncols * sizeof (GtkWidget *));
  varlabel = (GtkWidget **) g_malloc (xg.ncols * sizeof (GtkWidget *));

  varpanel_accel_group = gtk_accel_group_new ();

  /* create a new scrolled window. */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent), scrolled_window, true, true, 2);
  gtk_widget_show (scrolled_window);

  varpanel = gtk_table_new (vnrows, vncols, true);

  /* pack the table into the scrolled window */
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (scrolled_window), varpanel);
  gtk_widget_show (varpanel);

  /*
   * create a grid of buttons in the table
  */
  k = 0;
  for (i=0; i<vnrows; i++) {
    for (j=0; j<vncols; j++) {
      varcircle_add (i, j, k);
      k++;
      if (k == xg.ncols) break;
    }
  }

  gtk_widget_show_all (scrolled_window);
}

/*
 * Make the variable panel as square as possible, and lay it out
 * row-wise.
*/
void
init_varpanel_layout () {

  vnrows = (gint) sqrt ((gdouble) xg.ncols);
  vncols = vnrows;

  while (vnrows*vncols < xg.ncols) {
    vnrows++;
    if (vnrows*vncols < xg.ncols)
      vncols++;
  }
}

void
varpanel_size_init (gint cpanel_height)
{
  gint i;
  GtkTable *t = GTK_TABLE (varpanel);
  GtkTableRowCol c, r;
  gint width = 0, height = 0;
  GtkWidget *vport = GTK_WIDGET ((GTK_BIN (scrolled_window))->child);

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
varpanel_refresh () {
  gint j;

  for (j=0; j<xg.ncols; j++)
    if (GTK_WIDGET_REALIZED (da[j]))
      gtk_widget_queue_draw (da[j]);
}

void
varlabel_set (gint j) {
  gtk_label_set_text (GTK_LABEL (GTK_BIN (varlabel[j])->child),
    xg.vardata[j].collab_tform);
}
