/* varpanel_ui.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>

#include "vars.h"

#define VAR_CIRCLE_DIAM 40

static GtkWidget *varpanel, **da;
static GtkWidget *pc_menu, *p1_menu, *xyplot_menu, *rotation_menu, *tour_menu;

static gint vnrows, vncols;
static gboolean byrow;

/* external functions */
extern GtkWidget *CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *,
                                  GtkWidget *, GtkAccelGroup *,
                                  GtkSignalFunc, gpointer);
extern void init_var_GCs (GtkWidget *);
extern void splot_world_to_plane (cpaneld *, splotd *);
extern void splot_plane_to_screen (cpaneld *, splotd *);
extern void splot_plot (cpaneld *, splotd *);
extern void setRulerRanges (displayd *, splotd *);
extern gboolean parcoords_select_variable (cpaneld *, splotd *, gint, gint *,
                                           gboolean);
extern gboolean scatmat_select_variable (cpaneld *, splotd *, gint, gint *,
                                         gint, gboolean);
extern void splot_add_border (splotd *);
extern void scatterplot_show_rulers (displayd *, gint);

void varcircle_draw (gint); 
/* */


static gint
varsel_cb (GtkWidget *w, GdkEvent *event, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  splotd *sp = current_splot;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint jvar = GPOINTER_TO_INT (cbd);
    gint jvar_prev = -1;
    gint button = bevent->button;
    gboolean redraw = true;
    gboolean alt_modifier = false;
    gboolean shift_modifier = false;
    gboolean control_modifier = false;
    gboolean p1d_orientation_changed;

/* looking for modifiers; don't know which ones we'll want */
    if ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK)
      alt_modifier = true;
    else if ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK)
      shift_modifier = true;
    else if ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
      control_modifier = true;
/* */

    if (jvar >= 0 && jvar < xg.ncols) {

      switch (cpanel->projection) {
        case PCPLOT:
          redraw = parcoords_select_variable (cpanel, sp, jvar, &jvar_prev,
                                              alt_modifier);
          break;

        case SCATMAT:
          redraw = scatmat_select_variable (cpanel, sp, jvar, &jvar_prev,
                                            button, alt_modifier);
          redraw = false;
          break;

        case P1PLOT:
          p1d_orientation_changed =
            ((current_display->p1d_orientation == VERTICAL && button == 1) ||
             (current_display->p1d_orientation == HORIZONTAL && button != 1));
          current_display->p1d_orientation = (button == 1) ? HORIZONTAL :
                                                             VERTICAL;
          jvar_prev = sp->p1dvar;
          sp->p1dvar = jvar;

          if (p1d_orientation_changed)
            scatterplot_show_rulers (current_display, P1PLOT);
          break;

        case XYPLOT:
          if (button == 1) {
            if (jvar == sp->xyvars.x)
              redraw = false;
            else if (jvar == sp->xyvars.y) {
              sp->xyvars.y = jvar_prev = sp->xyvars.x;
            } else {
              jvar_prev = sp->xyvars.x;
            }
            sp->xyvars.x = jvar;
          } else if (button == 2 || button == 3) {
            if (jvar == sp->xyvars.y)
              redraw = false;
            else if (jvar == sp->xyvars.x) {
              sp->xyvars.x = jvar_prev = sp->xyvars.y;
            } else {
              jvar_prev = sp->xyvars.y;
            }
            sp->xyvars.y = jvar;
          }
          break;
        default:
          break;
      }

      varcircle_draw (jvar);
      if (jvar_prev != -1) {
        varcircle_draw (jvar_prev);
      }

      if (redraw) {
        splot_world_to_plane (cpanel, sp);
        splot_plane_to_screen (cpanel, sp);
        if (current_display->displaytype == scatterplot)
          setRulerRanges (current_display, sp);
        splot_plot (cpanel, sp);
      }

      splot_add_border (current_splot);

      return true;
    }
  }
  return false;
}


static gint
popup_varmenu (GtkWidget *w, GdkEvent *event) {

  if (current_display == null)
    return false;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button == 1) {

      switch (current_display->cpanel.projection) {
        case PCPLOT:
          gtk_menu_popup (GTK_MENU (pc_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;
        case P1PLOT:
          gtk_menu_popup (GTK_MENU (p1_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;
        case XYPLOT:
          gtk_menu_popup (GTK_MENU (xyplot_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;
        case ROTATE:
          gtk_menu_popup (GTK_MENU (rotation_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;
        case GRTOUR:
        case COTOUR:
          gtk_menu_popup (GTK_MENU (tour_menu), NULL, NULL, NULL, NULL,
            bevent->button, bevent->time);
          break;
      }
      return true;
    }
  }
  return false;
}

void
varcircle_draw (gint jvar)
{
  /*  a single pixmap is shared among all variable circles */
  static GdkPixmap *vpixmap = NULL;
  gint r = VAR_CIRCLE_DIAM/2;
  cpaneld *cpanel = &current_display->cpanel;
  splotd *sp = current_splot;
  gboolean chosen = false;
  GList *l;
  splotd *s;

  if (jvar <= -1 || jvar >= xg.ncols)
    return;

  if (selvarfg_GC == NULL) 
    init_var_GCs (da[jvar]);

  if (vpixmap == NULL) {
    vpixmap = gdk_pixmap_new (da[jvar]->window,
              VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1, -1);
  }

  /*
   * clear the pixmap 
  */
  gdk_draw_rectangle (vpixmap, unselvarbg_GC, true,
                      0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
  /*
   * add a filled circle for the background
  */
  gdk_draw_arc (vpixmap, selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);

  /*
   * add the appropriate line
  */
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
  return TRUE;
}

void
build_varmenus (GtkAccelGroup *accel) {

/*
CreateMenuItem (GtkWidget *menu, char *szName, char *szAccel, char *szTip,
  GtkWidget *wmain, GtkAccelGroup *accelgp, GtkSignalFunc func, gpointer data)
*/

  pc_menu = gtk_menu_new ();
  CreateMenuItem (pc_menu, "Select Y      M,R", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (pc_menu, "Delete Y <alt>M,R", NULL, NULL,
    varpanel, accel, NULL, NULL);


  p1_menu = gtk_menu_new ();
  CreateMenuItem (p1_menu, "Select X    L", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (p1_menu, "Select Y    M,R", NULL, NULL,
    varpanel, accel, NULL, NULL);

  xyplot_menu = gtk_menu_new ();
  CreateMenuItem (xyplot_menu, "Select X     L", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (xyplot_menu, "Select Y     M,R", NULL, NULL,
    varpanel, accel, NULL, NULL);

  rotation_menu = gtk_menu_new ();
  CreateMenuItem (rotation_menu, "Select X", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (rotation_menu, "Select Y", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (rotation_menu, "Select Z", NULL, NULL,
    varpanel, accel, NULL, NULL);

  tour_menu = gtk_menu_new ();
  CreateMenuItem (tour_menu, "Tour   L,M",         NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (tour_menu, "Manip  <Shift> L,M", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (tour_menu, "Freeze <Ctrl> L,M",  NULL, NULL,
    varpanel, accel, NULL, NULL);

/*
  corr_tour_menu = gtk_menu_new ();
  CreateMenuItem (corr_tour_menu, "Tour X    L",         NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Tour Y    M",         NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Manip X   <Shift> L", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Manip Y   <Shift> M", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Freeze X  <Ctrl> L",  NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (corr_tour_menu, "Freeze Y  <Ctrl> M",  NULL, NULL,
    varpanel, accel, NULL, NULL);
*/
}

static void
add_varcircle (gint i, gint j, gint k)
{
  GtkWidget *vb;
  GtkWidget *button;
  GtkWidget *lbl;

  vb = gtk_vbox_new (false, 0);
  gtk_container_border_width (GTK_CONTAINER (vb), 1);
  gtk_widget_show (vb);

  button = gtk_button_new_with_label (xg.collab[k]);
  gtk_widget_show (button);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
    button, "Click left to select", NULL);
  gtk_container_add (GTK_CONTAINER (vb), button);

  /*
   * This widget isn't shown; it's used to attach additional
   * data to the button.
  */
  lbl = gtk_label_new (g_strdup_printf ("%d", k));
  gtk_object_set_data (GTK_OBJECT (button), "index", GTK_OBJECT (lbl));

  gtk_signal_connect_object (GTK_OBJECT (button), "button_press_event",
    GTK_SIGNAL_FUNC (popup_varmenu), GTK_OBJECT (xyplot_menu));

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
  gtk_table_attach (GTK_TABLE (varpanel), vb, i, i+1, j, j+1,
    GTK_FILL, GTK_FILL, 2, 2);
}

void
make_varpanel (GtkWidget *parent) {
  GtkWidget *scrolled_window, *vport;
  gint i, j, k, _nc, _nr;
  GtkAccelGroup *accel_group;

  selvarfg_GC = NULL;

  da = (GtkWidget **) g_malloc (xg.ncols * sizeof (GtkWidget *));

  accel_group = gtk_accel_group_new ();

  /* create a new scrolled window. */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent), scrolled_window, true, true, 2);
  gtk_widget_show (scrolled_window);

  build_varmenus (accel_group);

  _nr = (byrow) ? vncols : vnrows;
  _nc = (byrow) ? vnrows : vncols;
  varpanel = gtk_table_new (_nr, _nc, true);

  /* pack the table into the scrolled window */
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (scrolled_window), varpanel);
  gtk_widget_show (varpanel);

  /*
   * create a grid of buttons in the table
  */
  k = 0;

  for (i=0; i<vncols; i++) {
    for (j=0; j<vnrows; j++) {
      if (byrow)
        add_varcircle (j, i, k);
      else
        add_varcircle (i, j, k);
      k++;
      if (k == xg.ncols) break;
    }
  }

/*
 * I'm on the right track here, but this needs more thought
*/
  gtk_widget_show_all (scrolled_window);
  g_printerr("vncols %d vnrows %d\n", vncols, vnrows);

  vport = GTK_WIDGET ((GTK_BIN (scrolled_window))->child);
  gtk_widget_set_usize (vport,
    MIN (vnrows, 4) * (VAR_CIRCLE_DIAM + 4) ,
    MIN (vncols, 6) * (VAR_CIRCLE_DIAM + 4)) ;

  gtk_widget_show_all (scrolled_window);
}

void
init_varpanel_geometry (gboolean layoutbyrow, gint dim1, gint dim2) {
  byrow = layoutbyrow;

/*
 * The sensible default number of rows is about 6; for columns, the
 * default should be 4
*/

  if (dim1 <= 0 || dim2 <= 0) {
    vnrows = 4;
  } else {
    vnrows = dim1;
    vncols = dim2;
  }

  /*
   * Adjust the user-supplied values to make sure they make sense.
  */
  vnrows = MIN (xg.ncols, vnrows); 
  vncols = (gint) (xg.ncols/vnrows) + (xg.ncols % vnrows > 0 ? 1 : 0);
}

void
varpanel_refresh () {
  gint j;
  gboolean rval = false;

  for (j=0; j<xg.ncols; j++)
    gtk_signal_emit_by_name (GTK_OBJECT (da[j]), "expose_event",
                            GINT_TO_POINTER (j), (gpointer) &rval);
}
