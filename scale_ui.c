/* scale_ui.c */

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <strings.h>
#include "vars.h"
#include "externs.h"

/*
 * These should all be moved to cpanel.h, to be consistent.
*/
/* scaling variables */
static gint scale_style;       /* DRAG or CLICK */
static gint scale_click_opt;   /* PAN or ZOOM */
static gint scale_pan_opt;     /* P_OBLIQUE, P_HORIZ, P_VERT */
static gint scale_zoom_opt;    /* Z_OBLIQUE, Z_ASPECT, Z_HORIZ, Z_VERT */

static rectd scale_click_rect;

/*-- widgets whose sensitivity needs to be turned on and off --*/
static GtkWidget *pan_opt, *zoom_opt, *pan_radio, *zoom_radio;

void
scale_init () {

  scale_style = DRAG;
  scale_click_opt = PAN;
  scale_pan_opt = P_OBLIQUE;
  scale_zoom_opt = Z_OBLIQUE;
}

/*
 * Activated from the Reset menu in the main menubar
*/
void
reset_pan_cb (GtkWidget *w, gpointer cbd) {
  splotd *sp = current_splot;
  displayd *display = (displayd *) sp->displayptr;

  sp->ishift.x = sp->max.x/2;
  sp->ishift.y = sp->max.y/2;

  splot_plane_to_screen (display, &display->cpanel, sp);
  ruler_ranges_set (current_display, sp);
  splot_redraw (sp, FULL);
}
void
reset_zoom_cb (GtkWidget *w, gpointer cbd) {
  gchar *lbl = (gchar *) cbd;
  g_printerr ("reset zoom: %s\n", lbl);
}

void
interaction_style_cb (GtkToggleButton *w) {
/*
 * This is connected to the Drag button
*/
  scale_style = (w->active) ? DRAG : CLICK;
  g_printerr ("in interaction_style_cb: %s\n",
    (scale_style == DRAG) ? "DRAG" : "CLICK");
/*
 * If DRAG, disable all the click-style controls
*/
  gtk_widget_set_sensitive (pan_radio, (scale_style == CLICK));
  gtk_widget_set_sensitive (zoom_radio, (scale_style == CLICK));
  gtk_widget_set_sensitive (pan_opt, (scale_style == CLICK));
  gtk_widget_set_sensitive (zoom_opt, (scale_style == CLICK));

  splot_redraw (current_splot, QUICK);
}

static void clickoptions_cb (GtkToggleButton *w)
{
  scale_click_opt = (w->active) ? PAN : ZOOM;
  g_printerr ("in interaction_style_cb: %s\n",
    (scale_click_opt == PAN) ? "PAN" : "ZOOM");

  splot_redraw (current_splot, QUICK);
}

static gchar *panoptions_lbl[] = {"Oblique",
                                  "Horiz only",
                                  "Vert only"};
static void panoptions_cb (GtkWidget *w, gpointer cbd)
{
  scale_pan_opt = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", panoptions_lbl[scale_pan_opt]);
}
static gchar *zoomoptions_lbl[] = {"Oblique",
                                   "Fixed aspect",
                                   "Horiz only",
                                   "Vert only"};
static void zoomoptions_cb (GtkWidget *w, gpointer cbd)
{
  scale_zoom_opt = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", zoomoptions_lbl[scale_zoom_opt]);
  splot_redraw (current_splot, QUICK);
}

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gboolean button1_p, button2_p;
  displayd *display = (displayd *) sp->displayptr;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p);

  /*-- I'm not sure this could ever happen --*/
  if (mousepos.x == mousepos_o.x && mousepos.y == mousepos_o.y)
    return false;

  switch (scale_style) {

    case DRAG:
      if (button1_p) {
        pan_by_drag (sp);
      } else if (button2_p) {
        zoom_by_drag (sp);
      }

      /*-- redisplay this plot --*/
      splot_plane_to_screen (display, &display->cpanel, sp);
      ruler_ranges_set (current_display, sp);
      splot_redraw (sp, FULL);
      break;

    case CLICK:
      splot_redraw (sp, QUICK);
      break;

  }  /*-- end switch (scale_style) --*/

  mousepos_o.x = mousepos.x;
  mousepos_o.y = mousepos.y;

  return true;
}

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  gboolean redraw = false;

  switch (scale_style) {
    case DRAG:
      /*-- do nothing --*/
      break;

    case CLICK:
      switch (scale_click_opt) {
        case PAN:
          if (event->keyval == GDK_space) {
            pan_step (sp, scale_pan_opt);
            redraw = true;
          }
          break;
        case ZOOM:
          if (event->keyval == GDK_i || event->keyval == GDK_I) {
            zoom_step (sp, scale_zoom_opt, ZOOM_IN, &scale_click_rect);
            redraw = true;
          } else if (event->keyval == GDK_o || event->keyval == GDK_O) {
            zoom_step (sp, scale_zoom_opt, ZOOM_OUT, &scale_click_rect);
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
    splot_plane_to_screen (display, &display->cpanel, sp);
    splot_redraw (sp, FULL);
  }

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  current_splot = sp;
  current_display = (displayd *) sp->displayptr;

  mousepos_o.x = mousepos.x = event->x;
  mousepos_o.y = mousepos.y = event->y;

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

  mousepos.x = event->x;
  mousepos.y = event->y;

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
scale_menus_make () {
  GtkWidget *item;

/*
 * Reset menu
*/
  scale_reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Reset pan");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (reset_pan_cb),
                      (gpointer) "shift");
  gtk_menu_append (GTK_MENU (scale_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset zoom");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (reset_zoom_cb),
                      (gpointer) "scale");
  gtk_menu_append (GTK_MENU (scale_reset_menu), item);

  gtk_widget_show_all (scale_reset_menu);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

void
cpanel_scale_make () {
  GtkWidget *frame, *f, *vbox, *hbox, *vb, *lbl;
  GtkWidget *radio1, *radio2;
  GSList *group;

  scale_init ();

  control_panel[SCALE] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[SCALE]), 5);

  
/*
 * frame for arrow keys
*/
  frame = gtk_frame_new ("Interaction style");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (control_panel[SCALE]), frame, false, false, 0);

  hbox = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  radio1 = gtk_radio_button_new_with_label (NULL, "Drag");
  GTK_TOGGLE_BUTTON (radio1)->active = TRUE;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), radio1,
    "Drag left to pan, drag middle or right to zoom (most direct style)",
    NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (interaction_style_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), radio1, TRUE, TRUE, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_label (group, "Click");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), radio2,
    "Use mouse clicks and key presses to pan and zoom (useful for large data)",
    NULL);
  gtk_box_pack_start (GTK_BOX (hbox), radio2, TRUE, TRUE, 0);

/*
 * frame and vbox for click-style controls
*/
  frame = gtk_frame_new ("Click-style controls");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (control_panel[SCALE]), frame, false, false, 0);

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

  pan_radio = gtk_radio_button_new_with_label (NULL, "Pan");
  GTK_TOGGLE_BUTTON (pan_radio)->active = true;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), pan_radio,
    "Activate panning for click style interaction",
    NULL);
  gtk_signal_connect (GTK_OBJECT (pan_radio), "toggled",
                      GTK_SIGNAL_FUNC (clickoptions_cb), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), pan_radio, TRUE, TRUE, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (pan_radio));
  zoom_radio = gtk_radio_button_new_with_label (group, "Zoom");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), zoom_radio,
    "Activate zooming for click style interactione", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), zoom_radio, TRUE, TRUE, 0);

/*
 * panning controls
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Pan options:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  pan_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), pan_opt,
    "Specify any constraints on the panning direction.  Drag the arrow to set the angle and distance, then hit the spacebar to pan.",
    NULL);
  gtk_box_pack_end (GTK_BOX (vb), pan_opt, false, false, 0);
  populate_option_menu (pan_opt, panoptions_lbl,
                        sizeof (panoptions_lbl) / sizeof (gchar *),
                        panoptions_cb);
/*
 * zooming controls
*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Zoom options:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  zoom_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), zoom_opt,
    "Specify any constraints on the zoom.  Drag the box to set the distance, then hit 'i' to zoom in, 'o' to zoom out.",
    NULL);
  gtk_box_pack_end (GTK_BOX (vb), zoom_opt, false, false, 0);
  populate_option_menu (zoom_opt, zoomoptions_lbl,
                        sizeof (zoomoptions_lbl) / sizeof (gchar *),
                        zoomoptions_cb);

  gtk_widget_set_sensitive (pan_radio, (scale_style == CLICK));
  gtk_widget_set_sensitive (zoom_radio, (scale_style == CLICK));
  gtk_widget_set_sensitive (pan_opt, (scale_style == CLICK));
  gtk_widget_set_sensitive (zoom_opt, (scale_style == CLICK));

  gtk_widget_show_all (control_panel[SCALE]);
}

void
scale_click_zoom_rect_calc (splotd *sp, gint sc_zoom_opt) {
  icoords mid;
  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;

  if (mousepos.x <= mid.x && mousepos.y <= mid.y) {
    /* upper left quadrant of plot, based on the value of mid */
    scale_click_rect.x = mousepos.x;
    scale_click_rect.y = mousepos.y;
  } else if (mousepos.x <= mid.x && mousepos.y > mid.y) {
    /* lower left quadrant of plot */
    scale_click_rect.x = mousepos.x;
    scale_click_rect.y = mid.y - (mousepos.y - mid.y);
  } else if (mousepos.x > mid.x && mousepos.y > mid.y) {
    /* lower right quadrant of plot */
    scale_click_rect.x = mid.x - (mousepos.x - mid.x);
    scale_click_rect.y = mid.y - (mousepos.y - mid.y);
  } else if (mousepos.x > mid.x && mousepos.y <= mid.y) {
    /* upper right quadrant of plot */
    scale_click_rect.x = mid.x - (mousepos.x - mid.x);
    scale_click_rect.y = mousepos.y;
  }
  scale_click_rect.x = (mid.x - scale_click_rect.x < 20) ?
                       (mid.x - 20) :
                       scale_click_rect.x;
  scale_click_rect.y = (mid.y - scale_click_rect.y < 20) ?
                       (mid.y - 20) :
                       scale_click_rect.y;
  scale_click_rect.width = 2 * (mid.x - scale_click_rect.x);
  scale_click_rect.height = 2 * (mid.y - scale_click_rect.y);

  switch (sc_zoom_opt) {
    case Z_OBLIQUE:
      /* -- use the values just calculated --*/
      break;

    case Z_ASPECT:
      /*-- force the rectangle to be square --*/
      scale_click_rect.x = scale_click_rect.y =
        MAX (scale_click_rect.x, scale_click_rect.y);
      scale_click_rect.width = 2 * (mid.x - scale_click_rect.x);
      scale_click_rect.height = 2 * (mid.y - scale_click_rect.y);
      break;

    case Z_HORIZ:
      /*-- override the vertical position and height --*/
      scale_click_rect.y = 0;
      scale_click_rect.height = sp->max.y;
      break;
    case Z_VERT:
      /*-- override the horizontal position and width --*/
      scale_click_rect.x = 0;
      scale_click_rect.width = sp->max.x;
      break;
  }
}

void
scaling_visual_cues_draw (splotd *sp) {
  
  switch (scale_style) {

    case DRAG:
      gdk_draw_line (sp->pixmap1, plot_GC,
        0, sp->ishift.y,
        sp->da->allocation.width, sp->ishift.y);
      gdk_draw_line (sp->pixmap1, plot_GC,
        sp->ishift.x, 0,
        sp->ishift.x, sp->da->allocation.height);
      break;

    case CLICK:
      switch (scale_click_opt) {
        case PAN:
          gdk_draw_line (sp->pixmap1, plot_GC,
            sp->max.x/2, sp->max.y/2,
            mousepos.x, mousepos.y);
          break;
        case ZOOM:
          scale_click_zoom_rect_calc (sp, scale_zoom_opt);
          gdk_draw_rectangle (sp->pixmap1, plot_GC, false,
            scale_click_rect.x, scale_click_rect.y,
            scale_click_rect.width, scale_click_rect.height);
          break;
      }  /*-- end switch (scale_click_opt) --*/
  }  /*-- end switch (scale_style) --*/
}

