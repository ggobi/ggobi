/* scale_ui.c */

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <strings.h>
#include "vars.h"
#include "externs.h"

/*
 * These should all be moved to cpanel.h, to be consistent.
*/


void
cpanel_scale_init (cpaneld *cpanel, ggobid *gg) {

  cpanel->scale_style = DRAG;
  cpanel->scale_click_opt = PAN;
  cpanel->scale_pan_opt = P_OBLIQUE;
  cpanel->scale_zoom_opt = Z_OBLIQUE;
}

/*
 * Activated from the Reset menu in the main menubar
*/
void
scale_pan_reset (ggobid *gg) {
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;

  sp->ishift.x = sp->max.x/2;
  sp->ishift.y = sp->max.y/2;

  splot_plane_to_screen (display, &display->cpanel, sp, gg);
  ruler_ranges_set (gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}
void
pan_reset_cb (GtkWidget *w, ggobid *gg) {
  scale_pan_reset (gg);
}

void
scale_zoom_reset (ggobid *gg) {
  gint projection = projection_get (gg);
  splotd *sp = gg->current_splot;
  displayd *dsp = (displayd *) sp->displayptr;

  if (projection == TOUR2D)
    sp->tour_scale.x = sp->tour_scale.y = TOUR_SCALE_DEFAULT;
  else
    sp->scale.x = sp->scale.y = SCALE_DEFAULT;

  splot_plane_to_screen (dsp, &dsp->cpanel, sp, gg);
  ruler_ranges_set (dsp, sp, gg);
  splot_redraw (sp, FULL, gg);
}
void
zoom_reset_cb (GtkWidget *w, ggobid *gg) {
  scale_zoom_reset (gg);
}

/*--------------------------------------------------------------------*/
/*           Resetting various state variables                        */
/*--------------------------------------------------------------------*/

void
scale_interaction_style_set (gint style, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean click_p;

  cpanel->scale_style = style;  /*-- DRAG or CLICK --*/
  click_p = (cpanel->scale_style == CLICK);

/*
 * If DRAG, disable all the click-style controls
*/
  gtk_widget_set_sensitive (gg->scale.pan_radio, click_p);
  gtk_widget_set_sensitive (gg->scale.zoom_radio, click_p);
  gtk_widget_set_sensitive (gg->scale.pan_opt, click_p);
  gtk_widget_set_sensitive (gg->scale.zoom_opt, click_p);

  splot_redraw (gg->current_splot, QUICK, gg);
}
void
interaction_style_cb (GtkToggleButton *w, ggobid *gg) 
{
/*
 * This is connected to the Drag button
*/
  gint scale_style = (w->active) ? DRAG : CLICK;
  scale_interaction_style_set (scale_style, gg);
}

void scale_clickoptions_set (gint click_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;

  cpanel->scale_click_opt = click_opt;
  g_printerr ("in interaction_style_cb: %s\n",
    (cpanel->scale_click_opt == PAN) ? "PAN" : "ZOOM");

  splot_redraw (gg->current_splot, QUICK, gg);
}
static void clickoptions_cb (GtkToggleButton *w, ggobid *gg)
{
  gint scale_click_opt = (w->active) ? PAN : ZOOM;
  scale_clickoptions_set (scale_click_opt, gg);
}

static gchar *panoptions_lbl[] = {"Oblique", "Horiz only", "Vert only"};
void panoptions_set (gint pan_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->scale_pan_opt = pan_opt;
}
static void panoptions_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  gint pan_opt = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", panoptions_lbl[pan_opt]);

  panoptions_set (pan_opt, gg);
}
static gchar *zoomoptions_lbl[] = {"Oblique",
                                   "Fixed aspect",
                                   "Horiz only",
                                   "Vert only"};
void zoomoptions_set (gint zoom_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->scale_zoom_opt = zoom_opt;

  splot_redraw (gg->current_splot, QUICK, gg);
}
static void zoomoptions_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  gint zoom_opt = GPOINTER_TO_INT (cbd);
  zoomoptions_set (zoom_opt, gg);

  g_printerr ("cbd: %s\n", zoomoptions_lbl[zoom_opt]);
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, gg);

  /*-- I'm not sure this could ever happen --*/
  if (gg->mousepos.x == gg->mousepos_o.x && gg->mousepos.y == gg->mousepos_o.y)
    return false;

  switch (cpanel->scale_style) {

    case DRAG:
      if (button1_p) {
        pan_by_drag (sp, gg);
      } else if (button2_p) {
        zoom_by_drag (sp, gg);
      }

      /*-- redisplay this plot --*/
      splot_plane_to_screen (display, &display->cpanel, sp, gg);
      ruler_ranges_set (gg->current_display, sp, gg);
      splot_redraw (sp, FULL, gg);
      break;

    case CLICK:
      splot_redraw (sp, QUICK, gg);
      break;

  }  /*-- end switch (scale_style) --*/

  gg->mousepos_o.x = gg->mousepos.x;
  gg->mousepos_o.y = gg->mousepos.y;

  return true;
}

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  gboolean redraw = false;
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;

  switch (cpanel->scale_style) {
    case DRAG:
      /*-- do nothing --*/
      break;

    case CLICK:
      switch (cpanel->scale_click_opt) {
        case PAN:
          if (event->keyval == GDK_space) {
            pan_step (sp, cpanel->scale_pan_opt, gg);
            redraw = true;
          }
          break;
        case ZOOM:
          if (event->keyval == GDK_i || event->keyval == GDK_I) {
            zoom_step (sp, cpanel->scale_zoom_opt, ZOOM_IN,
              &gg->scale.click_rect, gg);
            redraw = true;
          } else if (event->keyval == GDK_o || event->keyval == GDK_O) {
            zoom_step (sp, cpanel->scale_zoom_opt, ZOOM_OUT,
              &gg->scale.click_rect, gg);
            redraw = true;
          }
          break;
      } /*-- end switch (scale_click_opt) --*/

      break;

    default:
  } /*-- end switch (scale_style) --*/

  /*-- redisplay this plot --*/
  if (redraw) {
    displayd *display = (displayd *) sp->displayptr;
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    splot_redraw (sp, FULL, gg);
  }

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);

  gg->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;

  gg->mousepos_o.x = gg->mousepos.x = event->x;
  gg->mousepos_o.y = gg->mousepos.y = event->y;

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
                                      (gpointer) sp);
  return retval;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);

  gg->mousepos.x = event->x;
  gg->mousepos.y = event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  return retval;
}

void
scale_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
  } else {
    gtk_signal_disconnect (GTK_OBJECT (display->window), sp->key_press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

void
scale_menus_make (ggobid *gg) {
  GtkWidget *item;

/*
 * Reset menu
*/
  gg->scale.scale_reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Reset pan");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (pan_reset_cb),
                      (gpointer) gg);
  gtk_menu_append (GTK_MENU (gg->scale.scale_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset zoom");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (zoom_reset_cb),
                      (gpointer) gg);
  gtk_menu_append (GTK_MENU (gg->scale.scale_reset_menu), item);

  gtk_widget_show_all (gg->scale.scale_reset_menu);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

void
cpanel_scale_make (ggobid *gg) {
  GtkWidget *frame, *f, *vbox, *hbox, *vb, *lbl;
  GtkWidget *radio1, *radio2;
  GSList *group;

  gg->control_panel[SCALE] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[SCALE]), 5);

/*
 * frame for arrow keys
*/
  frame = gtk_frame_new ("Interaction style");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[SCALE]),
                      frame, false, false, 0);

  hbox = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  radio1 = gtk_radio_button_new_with_label (NULL, "Drag");
  GTK_TOGGLE_BUTTON (radio1)->active = TRUE;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "Drag left to pan, drag middle or right to zoom (most direct style)",
    NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (interaction_style_cb), gg);
  gtk_box_pack_start (GTK_BOX (hbox), radio1, TRUE, TRUE, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_label (group, "Click");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "Use mouse clicks and key presses to pan and zoom (useful for large data)",
    NULL);
  gtk_box_pack_start (GTK_BOX (hbox), radio2, TRUE, TRUE, 0);

/*
 * frame and vbox for click-style controls
*/
  frame = gtk_frame_new ("Click-style controls");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[SCALE]),
                      frame, false, false, 0);

  vbox = gtk_vbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

/*
 * pan or zoom radio buttons
*/
  f = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), f, false, false, 0);

  hbox = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
  gtk_container_add (GTK_CONTAINER (f), hbox);

  gg->scale.pan_radio = gtk_radio_button_new_with_label (NULL, "Pan");
  GTK_TOGGLE_BUTTON (gg->scale.pan_radio)->active = true;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->scale.pan_radio,
    "Activate panning for click style interaction",
    NULL);
  gtk_signal_connect (GTK_OBJECT (gg->scale.pan_radio), "toggled",
                      GTK_SIGNAL_FUNC (clickoptions_cb), gg);
  gtk_box_pack_start (GTK_BOX (hbox), gg->scale.pan_radio, TRUE, TRUE, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (gg->scale.pan_radio));
  gg->scale.zoom_radio = gtk_radio_button_new_with_label (group, "Zoom");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->scale.zoom_radio,
    "Activate zooming for click style interactione", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), gg->scale.zoom_radio, TRUE, TRUE, 0);

/*
 * panning controls
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Pan options:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  gg->scale.pan_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->scale.pan_opt,
    "Specify any constraints on the panning direction.  Drag the arrow to set the angle and distance, then hit the spacebar to pan.",
    NULL);
  gtk_box_pack_end (GTK_BOX (vb), gg->scale.pan_opt, false, false, 0);
  populate_option_menu (gg->scale.pan_opt, panoptions_lbl,
                        sizeof (panoptions_lbl) / sizeof (gchar *),
                        panoptions_cb, gg);
/*
 * zooming controls
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Zoom options:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  gg->scale.zoom_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->scale.zoom_opt,
    "Specify any constraints on the zoom.  Drag the box to set the distance, then hit 'i' to zoom in, 'o' to zoom out.",
    NULL);
  gtk_box_pack_end (GTK_BOX (vb), gg->scale.zoom_opt, false, false, 0);
  populate_option_menu (gg->scale.zoom_opt, zoomoptions_lbl,
                        sizeof (zoomoptions_lbl) / sizeof (gchar *),
                        zoomoptions_cb, gg);

  /*-- start with dragging on by default --*/
/*
  gtk_widget_set_sensitive (gg->scale.pan_radio, true);
  gtk_widget_set_sensitive (gg->scale.zoom_radio, true);
  gtk_widget_set_sensitive (gg->scale.pan_opt, true);
  gtk_widget_set_sensitive (gg->scale.zoom_opt, true);
*/

  gtk_widget_show_all (gg->control_panel[SCALE]);
}

void
scale_click_zoom_rect_calc (splotd *sp, gint sc_zoom_opt, ggobid *gg) {
  icoords mid;
  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;

  if (gg->mousepos.x <= mid.x && gg->mousepos.y <= mid.y) {
    /* upper left quadrant of plot, based on the value of mid */
    gg->scale.click_rect.x = gg->mousepos.x;
    gg->scale.click_rect.y = gg->mousepos.y;
  } else if (gg->mousepos.x <= mid.x && gg->mousepos.y > mid.y) {
    /* lower left quadrant of plot */
    gg->scale.click_rect.x = gg->mousepos.x;
    gg->scale.click_rect.y = mid.y - (gg->mousepos.y - mid.y);
  } else if (gg->mousepos.x > mid.x && gg->mousepos.y > mid.y) {
    /* lower right quadrant of plot */
    gg->scale.click_rect.x = mid.x - (gg->mousepos.x - mid.x);
    gg->scale.click_rect.y = mid.y - (gg->mousepos.y - mid.y);
  } else if (gg->mousepos.x > mid.x && gg->mousepos.y <= mid.y) {
    /* upper right quadrant of plot */
    gg->scale.click_rect.x = mid.x - (gg->mousepos.x - mid.x);
    gg->scale.click_rect.y = gg->mousepos.y;
  }
  gg->scale.click_rect.x = (mid.x - gg->scale.click_rect.x < 20) ?
                       (mid.x - 20) :
                       gg->scale.click_rect.x;
  gg->scale.click_rect.y = (mid.y - gg->scale.click_rect.y < 20) ?
                       (mid.y - 20) :
                       gg->scale.click_rect.y;
  gg->scale.click_rect.width = 2 * (mid.x - gg->scale.click_rect.x);
  gg->scale.click_rect.height = 2 * (mid.y - gg->scale.click_rect.y);

  switch (sc_zoom_opt) {
    case Z_OBLIQUE:
      /* -- use the values just calculated --*/
      break;

    case Z_ASPECT:
      /*-- force the rectangle to be square --*/
      gg->scale.click_rect.x = gg->scale.click_rect.y =
        MAX (gg->scale.click_rect.x, gg->scale.click_rect.y);
      gg->scale.click_rect.width = 2 * (mid.x - gg->scale.click_rect.x);
      gg->scale.click_rect.height = 2 * (mid.y - gg->scale.click_rect.y);
      break;

    case Z_HORIZ:
      /*-- override the vertical position and height --*/
      gg->scale.click_rect.y = 0;
      gg->scale.click_rect.height = sp->max.y;
      break;
    case Z_VERT:
      /*-- override the horizontal position and width --*/
      gg->scale.click_rect.x = 0;
      gg->scale.click_rect.width = sp->max.x;
      break;
  }
}

void
scaling_visual_cues_draw (splotd *sp, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  
  switch (cpanel->scale_style) {

    case DRAG:
      gdk_draw_line (sp->pixmap1, gg->plot_GC,
        0, sp->ishift.y,
        sp->da->allocation.width, sp->ishift.y);
      gdk_draw_line (sp->pixmap1, gg->plot_GC,
        sp->ishift.x, 0,
        sp->ishift.x, sp->da->allocation.height);
      break;

    case CLICK:
      switch (cpanel->scale_click_opt) {
        case PAN:
          gdk_draw_line (sp->pixmap1, gg->plot_GC,
            sp->max.x/2, sp->max.y/2,
            gg->mousepos.x, gg->mousepos.y);
          break;
        case ZOOM:
          scale_click_zoom_rect_calc (sp, cpanel->scale_zoom_opt, gg);
          gdk_draw_rectangle (sp->pixmap1, gg->plot_GC, false,
            gg->scale.click_rect.x, gg->scale.click_rect.y,
            gg->scale.click_rect.width, gg->scale.click_rect.height);
          break;
      }  /*-- end switch (scale_click_opt) --*/
  }  /*-- end switch (scale_style) --*/
}

