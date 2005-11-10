#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>

#include "vars.h"

#ifndef BITMAP_H
#include "bitmap.h"
#endif

#define VAR_CIRCLE_DIAM 40

static gint *varindices;

static GtkWidget *varpanel, **da;
static GtkWidget *oned_menu, *xyplot_menu, *rotation_menu, *tour_menu;

static int vnrows, vncols;
static gboolean byrow;

/* external functions */
extern GtkWidget *CreateMenuItem (GtkWidget *, char *, char *, char *,
                                  GtkWidget *, GtkAccelGroup *,
                                  GtkSignalFunc, gpointer);
extern void init_var_GCs (GtkWidget *);
extern void sp_world_to_plane (viewd *, splotd *);
extern void sp_plane_to_screen (viewd *, splotd *);
extern void setRulerRanges (viewd *, splotd *);
/*                    */


/***************************** drag and drop ******************************/

gboolean have_drag;

/* We'll just pass the starting index as a string, right? */
static GtkTargetEntry target_table[] = {
  { "STRING",     0, 0 },
};

void
move_variable (gint from, gint to) {
  gint j, k, from_value;
  GList *vb_list;
  GtkWidget *vb, *btn;
  GtkWidget *d;
  gchar *name;

  if (from >= 0 && from < xg.ncols &&
      to != from && to >= 0 && to < xg.ncols)
  {
    from_value = varindices[from];

    if (to < from) {
      for (j=from; j>to; j--)
        varindices[j] = varindices[j-1];
      varindices[to] = from_value;

    } else {
      for (j=from; j<to; j++)
        varindices[j] = varindices[j+1];
      varindices[to] = from_value;
    }

/*
 * These are now vboxes -- we want to get the button inside
 * the vbox, and then get_data to grab the label attached to
 * the button.
*/
    vb_list = gtk_container_get_children (GTK_CONTAINER (varpanel));  /* vboxes */

    for (j=0; j<xg.ncols; j++) {
      k = -1;

      /* get the next vbox */
      vb = (GtkWidget *) vb_list->data;

      /* get the button, the first child of the vbox */
      btn = (gtk_container_get_children (GTK_CONTAINER (vb)))->data;

      /* get the label, the user data associated with the button */
      d = g_object_get_data(G_OBJECT (btn), "index");
      if (d != NULL) {
        /* get the label of the label */
        gtk_label_get (GTK_LABEL (d), &name);
        k = atoi (name);
      }
      if (k == -1) break;

      /*
       * Reset the label and the tooltip for this variable label.
       * The selection status and what's drawn in the circle will
       * also need to be updated.
      */
      gtk_widget_set (GTK_WIDGET (btn), "label",
        (gchar *) xg.collab[varindices[k]], NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
        btn, xg.collab[varindices[k]], NULL);

      vb_list = vb_list->next;

      if (vb_list == NULL) break;
    }

/*
    for (j=0; j<xg.ncols; j++)
      g_printerr ("%d ", varindices[j]);
    g_printerr ("\n");
*/
  }
}

void  
ebox_drag_data_received  (GtkWidget          *w,
                          GdkDragContext     *context,
                          gint                x,
                          gint                y,
                          GtkSelectionData   *sd,
                          guint               info,
                          guint               time)
{
  gint j, k = -1, p = -1;
  gchar *label;
  GtkWidget *d;

/*
 * Here we're trapping the drag event on the button, to which a label
 * widget is attached.  The label widget has the index of destination
 * variable circle.
*/
  d = g_object_get_data(G_OBJECT (w), "index");
  gtk_label_get (GTK_LABEL (GTK_LABEL (d)), &label);
  p = atoi (label);
/* */
  
  if ((sd->length >= 0) && (sd->format == 8)) {
    k = atoi ((gchar *) sd->data);

    g_print ("source_drag_data_get: collab[%d] = %s; collab[%d] = %s\n",
      k, xg.collab[k], p, xg.collab[p]);
    g_print ("... or lab[ind[%d]] = %s, lab[ind[%d]] = %s\n",
      k, xg.collab[varindices[k]], p, xg.collab[varindices[p]]);

    if (k >= 0 && k < xg.ncols && p >= 0 && p < xg.ncols) {
      gboolean rval = false;
      move_variable (k, p);
      for (j=0; j<xg.ncols; j++)
        g_signal_emit_by_name (G_OBJECT (da[j]), "expose_event",
          GINT_TO_POINTER (j), &rval);
      gtk_drag_finish ((GdkDragContext *) context, TRUE, FALSE, time);
      return;
    }
  }

  gtk_drag_finish (context, FALSE, FALSE, time);
}

void  
source_drag_data_get  (GtkWidget          *w,
                       GdkDragContext     *context,
                       GtkSelectionData   *selection_data,
                       guint               info,
                       guint               time,
                       gpointer            cbd)
{
  int k = GPOINTER_TO_INT (cbd);
  gchar *str;
  str = g_strdup_printf ("%d", k);

  gtk_selection_data_set (selection_data,
                          selection_data->target,
                          8, str, strlen (str));
  g_free (str);
}

void                 
source_drag_data_delete  (GtkWidget          *widget,
                          GdkDragContext     *context,
                          gpointer            data)
{                           
  g_print ("Delete the data!\n");
}

/************************end drag and drop ******************************/

static void
select_varcircle (gint select, gint variable, gint varcircle) {
  gint var = variable, varcirc = varcircle;
  gboolean rval = false;
  gint i;

  g_return_if_fail (var != -1 || varcirc != -1);
  g_return_if_fail (var == -1 || varcirc == -1);

  if (varcirc == -1) {
    xg.view.varchosen[variable] = select;

    for (i=0; i<xg.ncols; i++) {
      if (varindices[i] == var) {
        varcirc = i;
        break;
      }
    }
  } else if (var == -1) {
    var = varindices[varcirc];
    xg.view.varchosen[var] = select;
  }

  g_signal_emit_by_name (G_OBJECT (da[varcirc]), "expose_event",
    GINT_TO_POINTER (varcirc), &rval);
}

static gint
varsel_cb (GtkWidget *w, GdkEvent *event, gpointer cbd)
{
  extern scatterplotd scat;
  viewd *view = &scat.view;
  splotd *sp = &scat.splot;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    int varcircle = GPOINTER_TO_INT (cbd);
    int button = bevent->button;
    int variable;
    gboolean rval = false;
    gboolean redraw = true;

    if (varcircle >= 0 && varcircle < xg.ncols) {
      variable = varindices[varcircle];

      switch (xg.view.projection) {
        case ONEDPLOT:
          break;
        case XYPLOT:
          if (button == 1) {
            if (variable == sp->xyvars.x)
              redraw = false;
            else if (variable == sp->xyvars.y)
              sp->xyvars.y = sp->xyvars.x;
            else { /* the indices in varchosen correspond to the data */
              select_varcircle (false, sp->xyvars.x, -1);
              select_varcircle (true, -1, varcircle);
            }
            sp->xyvars.x = variable;
          } else if (button == 2 || button == 3) {
            if (variable == sp->xyvars.y)
              redraw = false;
            else if (variable == sp->xyvars.x)
              sp->xyvars.x = sp->xyvars.y;
            else { /* the indices in varchosen correspond to the data */
              select_varcircle (false, sp->xyvars.y, -1);
              select_varcircle (true, -1, varcircle);
            }
            sp->xyvars.y = variable;
          }
          break;
        default:
          break;
      }
      if (redraw) {
        sp_world_to_plane (view, sp);
        sp_plane_to_screen (view, sp);
        setRulerRanges (view, sp);
        g_signal_emit_by_name (G_OBJECT (sp->da), "expose_event",
          (gpointer) sp, (gpointer) &rval);
      }
      return true;
    }
  }
  return false;
}


static gint
popup_varmenu (GtkWidget *w, GdkEvent *event) {

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    if (bevent->button == 1) {

      switch (xg.view.projection) {
        case ONEDPLOT:
          gtk_menu_popup (GTK_MENU (oned_menu), NULL, NULL, NULL, NULL,
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

gboolean
da_expose (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  static GdkPixmap *vpixmap = NULL;
  int k = GPOINTER_TO_INT (cbd);

  if (selvarfg_GC == NULL) 
    init_var_GCs (w);

  if (vpixmap == NULL) {
    vpixmap = gdk_pixmap_new (w->window,
              VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1, -1);
  }

  /*
   * clear the pixmap
  */
  gdk_draw_rectangle (vpixmap, unselvarbg_GC, true,
              0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  if (xg.view.varchosen[varindices[k]]) {
    gdk_draw_arc (vpixmap, selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
    gdk_draw_arc (vpixmap, selvarfg_GC, false,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  } else {
    gdk_draw_arc (vpixmap, unselvarfg_GC, false,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  }

  /* does it matter what GC we use here? probably not */
  gdk_draw_pixmap (w->window, unselvarfg_GC, vpixmap, 0, 0, 0, 0,
    VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  return TRUE;
}

void
build_varmenus (GtkAccelGroup *accel) {

/*
CreateMenuItem (GtkWidget *menu, char *szName, char *szAccel, char *szTip,
  GtkWidget *wmain, GtkAccelGroup *accelgp, GtkSignalFunc func, gpointer data)
*/

  oned_menu = gtk_menu_new ();
  CreateMenuItem (oned_menu, "Select X    L", NULL, NULL,
    varpanel, accel, NULL, NULL);
  CreateMenuItem (oned_menu, "Select Y    M,R", NULL, NULL,
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
add_varcircle (gint i, gint j, gint k,
  GdkPixmap *drag_icon, GdkBitmap *drag_mask)
{
  GtkWidget *vb;
  GtkWidget *button;
  GtkWidget *lbl, *ebox;

  vb = gtk_vbox_new (false, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 1);
  gtk_widget_show (vb);

  button = gtk_button_new_with_label (xg.collab[k]);
  gtk_widget_show (button);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
    button, "Click left to select, drag right to reorder", NULL);
  gtk_container_add (GTK_CONTAINER (vb), button);

  /*
   * This widget isn't shown; it's used to attach additional
   * data to the button.
  */
  lbl = gtk_label_new (g_strdup_printf ("%d", k));
  g_object_set_data(G_OBJECT (button), "index", GTK_OBJECT (lbl));

  g_signal_connect_swapped (G_OBJECT (button), "button_press_event",
    G_CALLBACK (popup_varmenu), G_OBJECT (xyplot_menu));

/*
 * connect the drag and drop signals to the right mouse button
*/
  /* button qua destination */

  gtk_drag_dest_set (button,
                     GTK_DEST_DEFAULT_ALL,
                     target_table, 1,
                     GDK_ACTION_COPY | GDK_ACTION_MOVE);
  g_signal_connect (G_OBJECT (button), "drag_data_received",
                     G_CALLBACK (ebox_drag_data_received), NULL);

  /* button qua source */
  gtk_drag_source_set (button, GDK_BUTTON3_MASK,
                       target_table, 1,
                       GDK_ACTION_COPY | GDK_ACTION_MOVE);
  gtk_drag_source_set_icon (button,
                            gtk_widget_get_colormap (varpanel),
                            drag_icon, drag_mask);
  gdk_pixmap_unref (drag_icon);
  gdk_pixmap_unref (drag_mask);
  g_signal_connect (G_OBJECT (button), "drag_data_get",
                      G_CALLBACK (source_drag_data_get),
                      GINT_TO_POINTER (k));
  g_signal_connect (G_OBJECT (button), "drag_data_delete",
                      G_CALLBACK (source_drag_data_delete),
                      GINT_TO_POINTER (k));

/*
 * Put the drawing area inside an event box in order to be able
 * to catch mouse events
*/
  ebox = gtk_event_box_new ();
  gtk_widget_show (ebox);

  /*
   * a drawing area to contain the variable circle
  */

  da[k] = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (da[k]),
    VAR_CIRCLE_DIAM+2, VAR_CIRCLE_DIAM+2);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
    ebox, "Click to select; see menu", NULL);

  g_signal_connect (G_OBJECT (da[k]), "expose_event",
    G_CALLBACK (da_expose), GINT_TO_POINTER (k));

  gtk_widget_show (da[k]);
  g_signal_connect (G_OBJECT (ebox), "button_press_event",
    G_CALLBACK (varsel_cb), GINT_TO_POINTER (k));

  gtk_container_add (GTK_CONTAINER (ebox), da[k]);
  gtk_container_add (GTK_CONTAINER (vb), ebox);

  gtk_table_attach (GTK_TABLE (varpanel), vb, i, i+1, j, j+1,
    GTK_FILL, GTK_FILL, 4, 4);
}

void
make_varpanel (GtkWidget *parent) {
  GtkWidget *scrolled_window;
  gint i, j, k, _nc, _nr;
  GdkPixmap *drag_icon;
  GdkBitmap *drag_mask;
  GtkAccelGroup *accel_group;

  selvarfg_GC = NULL;

/*
 * indicator for whether a variable one of those selected for
 * plotting or not -- initially, 0 and 1 are set.
*/
  xg.view.varchosen = g_malloc0(xg.ncols * sizeof (gint));
  xg.view.varchosen[0] = xg.view.varchosen[1] = true;
/* */
  da = (GtkWidget **) g_malloc (xg.ncols * sizeof (GtkWidget *));

  accel_group = gtk_accel_group_new ();

  /* create a new scrolled window. */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent), scrolled_window, true, true, 3);
  gtk_widget_show (scrolled_window);

  build_varmenus (accel_group);

  varindices = g_malloc (xg.ncols * sizeof (gint));
  for (i=0; i<xg.ncols; i++) varindices[i] = i;

  _nr = (byrow) ? vncols : vnrows;
  _nc = (byrow) ? vnrows : vncols;
  varpanel = gtk_table_new (_nr, _nc, true);

  /* pack the table into the scrolled window */
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (scrolled_window), varpanel);
  gtk_widget_show (varpanel);

/*
 * drag_icon = gdk_pixmap_colormap_create_from_xpm (NULL,
 *   gtk_widget_get_colormap (varpanel),
 *   &drag_mask, NULL, "drag_cursor40.xpm");
*/
  drag_icon = gdk_pixmap_colormap_create_from_xpm_d (NULL,
    gtk_widget_get_colormap (varpanel),
    &drag_mask, NULL, (gchar **) drag_cursor40x);

  /*
   * create a grid of buttons in the table
  */
  k = 0;

  for (i=0; i<vncols; i++) {
    for (j=0; j<vnrows; j++) {
      if (byrow)
        add_varcircle (j, i, k, drag_icon, drag_mask) ;
      else
        add_varcircle (i, j, k, drag_icon, drag_mask) ;
      k++;
      if (k == xg.ncols) break;
    }
  }

/*
 * I'm on the right track here, but this needs more thought
*/
  g_printerr("vncols %d vnrows %d\n", vncols, vnrows);
  gtk_widget_set_usize (scrolled_window,
    MIN (vnrows, 4) * (40 + 4) ,
    MIN (vncols, 6) * (40 + 4)) ;
  gtk_widget_show (scrolled_window);

}

void
init_varpanel_geometry (gboolean layoutbyrow, gint dim1, gint dim2) {
  byrow = layoutbyrow;

/*
 * The sensible default number of rows is about 6; for columns, the
 * default should be 4
*/

  if (dim1 <= 0 || dim2 <= 0) {
    vnrows = 5;
  } else {
    vnrows = dim1;
    vncols = dim2;
  }

  /*
   * Adjust the user-supplied values to make sure they make sense.
  */
  vnrows = MIN (xg.ncols, vnrows); 
  vncols = (int) (xg.ncols/vnrows) + (xg.ncols % vnrows > 0 ? 1 : 0);
}
