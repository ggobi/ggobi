/* scale_ui.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include "vars.h"
#include "externs.h"
#include <math.h>

void
scale_pan_reset (displayd *display) {
	ggobid *gg = display->ggobi;
	splotd *sp = gg->current_splot;
  
  sp->pmid.x = sp->pmid.y = 0;

  splot_plane_to_screen (display, &display->cpanel, sp, gg);
  ruler_ranges_set (false, gg->current_display, sp, gg);
  splot_redraw (sp, FULL, gg);
}

void
scale_zoom_reset (displayd *dsp) {
  /*gint projection = projection_get (gg);*/
  ggobid *gg = dsp->ggobi;
  splotd *sp = gg->current_splot;
  
  /*  if (projection == TOUR2D)
    sp->tour_scale.x = sp->tour_scale.y = TOUR_SCALE_DEFAULT;
    else*/
    sp->scale.x = sp->scale.y = SCALE_DEFAULT;

  splot_plane_to_screen (dsp, &dsp->cpanel, sp, gg);
  ruler_ranges_set (false, dsp, sp, gg);
  splot_redraw (sp, FULL, gg);
}

/*--------------------------------------------------------------------*/
/*           Resetting various state variables                        */
/*--------------------------------------------------------------------*/

static void
drag_aspect_ratio_cb (GtkToggleButton *button, ggobid *gg)
{
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;

  cpanel->scale_drag_aspect_p = button->active;
}

void
scale_click_init (splotd *sp, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  gint pos = (gint)
    (.1 * sqrt ((gdouble) (sp->max.x*sp->max.x + sp->max.y*sp->max.y)));

  if (cpanel->scale_style != CLICK)
    return;

  switch (cpanel->scale_click_opt) {
    case PAN:
      sp->mousepos.x = sp->max.x/2 - pos;
      sp->mousepos.y = sp->max.y/2 - pos;
    break;
    case ZOOM:
      sp->mousepos.x = pos;
      sp->mousepos.y = pos;
    break;
    default:
    break;
  }
}

void
scale_interaction_style_set (gint style, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean click_p;
  GtkWidget *pan_radio, *zoom_radio, *pan_option_menu, *zoom_option_menu;
  GtkWidget *panel = mode_panel_get_by_name(GGOBI(getIModeName)(SCALE), gg);

  if (panel == (GtkWidget *) NULL)
    return;

  pan_radio = widget_find_by_name (panel, "SCALE:pan_radio_button");
  zoom_radio = widget_find_by_name (panel, "SCALE:zoom_radio_button");
  pan_option_menu = widget_find_by_name (panel,"SCALE:pan_option_menu");
  zoom_option_menu = widget_find_by_name (panel, "SCALE:zoom_option_menu");

  cpanel->scale_style = style;  /*-- DRAG or CLICK --*/
  click_p = (cpanel->scale_style == CLICK);

/*
 * If DRAG, disable all the click-style controls
*/
  gtk_widget_set_sensitive (pan_radio, click_p);
  gtk_widget_set_sensitive (zoom_radio, click_p);
  gtk_widget_set_sensitive (pan_option_menu, click_p);
  gtk_widget_set_sensitive (zoom_option_menu, click_p);

  if (click_p)
    scale_click_init (gg->current_splot, gg);

  splot_redraw (gg->current_splot, QUICK, gg);
}
void
interaction_style_cb (GtkToggleButton *w, ggobid *gg) 
{
  GtkWidget *radio, *opt;
  GtkWidget *panel = mode_panel_get_by_name(GGOBI(getIModeName)(SCALE), gg);

/*
 * This is connected to the Drag button
*/
  gint scale_style = (w->active) ? DRAG : CLICK;
  scale_interaction_style_set (scale_style, gg);

  if (panel == (GtkWidget *) NULL)
    return;

  radio = widget_find_by_name (panel,
    "SCALE:pan_radio_button");
  gtk_widget_set_sensitive (radio, scale_style == CLICK);
  radio = widget_find_by_name (panel,
    "SCALE:zoom_radio_button");
  gtk_widget_set_sensitive (radio, scale_style == CLICK);

  opt = widget_find_by_name (panel,
    "SCALE:pan_option_menu");
  gtk_widget_set_sensitive (opt, scale_style == CLICK);
  
  opt = widget_find_by_name (panel,
    "SCALE:zoom_option_menu");
  gtk_widget_set_sensitive (opt, scale_style == CLICK);

}

void scale_clickoptions_set (gint click_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;

  cpanel->scale_click_opt = click_opt;
  scale_click_init (gg->current_splot, gg);

  splot_redraw (gg->current_splot, QUICK, gg);
}
static void clickoptions_cb (GtkToggleButton *w, ggobid *gg)
{
  gint scale_click_opt = (w->active) ? PAN : ZOOM;
  scale_clickoptions_set (scale_click_opt, gg);
}

static gchar *panoptions_lbl[] = {"Horiz only", "Vert only", "Oblique"};
void panoptions_set (gint pan_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->scale_pan_opt = pan_opt;
}
static void panoptions_cb (GtkWidget *w, ggobid *gg)
{
  gint pan_opt = gtk_combo_box_get_active(GTK_COMBO_BOX(w));

  panoptions_set (pan_opt, gg);
}
static gchar *zoomoptions_lbl[] = {"Fixed aspect",
                                   "Horiz only",
                                   "Vert only",
                                   "Oblique"};
void zoomoptions_set (gint zoom_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->scale_zoom_opt = zoom_opt;

  splot_redraw (gg->current_splot, QUICK, gg);
}
static void zoomoptions_cb (GtkWidget *w, ggobid *gg)
{
  gint zoom_opt = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  zoomoptions_set (zoom_opt, gg);
}

/*--------------------------------------------------------------------*/
/*      Handling keyooooboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  gboolean redraw = false;
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  
/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

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
          } else if (event->keyval == GDK_Up || 
                     event->keyval == GDK_Down || 
                     event->keyval == GDK_Left ||
		     event->keyval == GDK_Right) 
          {
	    pan_step_key (sp, event->keyval, gg);
	    redraw = true;
          }
          break;
        case ZOOM:
          /* zoom in if > or . */
          if (event->keyval == GDK_greater || event->keyval == GDK_period) {
            zoom_step (sp, cpanel->scale_zoom_opt, ZOOM_IN,
              &gg->scale.click_rect, gg);
            redraw = true;
          /* zoom out if < or , */
          } else if (event->keyval == GDK_less || event->keyval == GDK_comma) {
            zoom_step (sp, cpanel->scale_zoom_opt, ZOOM_OUT,
              &gg->scale.click_rect, gg);
            redraw = true;
          }
          break;
      } /*-- end switch (scale_click_opt) --*/

      break;

    default:
      break;
  } /*-- end switch (scale_style) --*/

  /*-- redisplay this plot --*/
  if (redraw) {
    displayd *display = (displayd *) sp->displayptr;
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    splot_redraw (sp, FULL, gg);
	return true;
  }

  return false;
}

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  /*-- if neither button is pressed, we shouldn't have gotten the event --*/
  if (!button1_p && !button2_p)
    return false;

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x && sp->mousepos.y == sp->mousepos_o.y)
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
      ruler_ranges_set (false, gg->current_display, sp, gg);
      splot_redraw (sp, FULL, gg);
      break;

    case CLICK:
      splot_redraw (sp, QUICK, gg);
      break;

  }  /*-- end switch (scale_style) --*/

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return true;
}


static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);
  gboolean button1_p, button2_p;
  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                      "motion_notify_event",
                                      G_CALLBACK(motion_notify_cb),
                                      (gpointer) sp);
  return retval;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;

  gg->buttondown = 0;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y, &state);

  gdk_pointer_ungrab (event->time);
  disconnect_motion_signal (sp);

  return retval;
}

void
scale_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    if(GGOBI_IS_WINDOW_DISPLAY(display))
      sp->key_press_id = g_signal_connect (G_OBJECT (GGOBI_WINDOW_DISPLAY(display)->window),
        "key_press_event",
        G_CALLBACK(key_press_cb),
        (gpointer) sp);
      sp->press_id = g_signal_connect (G_OBJECT (sp->da),
        "button_press_event",
        G_CALLBACK(button_press_cb),
        (gpointer) sp);
      sp->release_id = g_signal_connect (G_OBJECT (sp->da),
        "button_release_event",
        G_CALLBACK(button_release_cb),
        (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}

void
cpanel_scale_make (ggobid *gg) {
  modepaneld *panel;
  GtkWidget *frame, *f, *vbox, *hbox, *vb, *lbl;
  GtkWidget *radio1, *radio2, *tgl;
  GSList *group;
  GtkWidget *pan_radio, *zoom_radio, *pan_option_menu, *zoom_option_menu;

  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getIModeName)(SCALE));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);


  frame = gtk_frame_new ("Interaction style");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      frame, false, false, 0);

  hbox = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), hbox);

  radio1 = gtk_radio_button_new_with_mnemonic (NULL, "_Drag");
  gtk_widget_set_name (radio1, "SCALE:drag_radio_button");
  GTK_TOGGLE_BUTTON (radio1)->active = TRUE;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio1,
    "Drag left to pan, drag middle or right to zoom (the most direct style).  Tip: when zooming, don't put the cursor too close to the center of the plot.\n(To reset, see Reset in main menubar)",
    NULL);
  g_signal_connect (G_OBJECT (radio1), "toggled",
    G_CALLBACK (interaction_style_cb), gg);
  gtk_box_pack_start (GTK_BOX (hbox), radio1, TRUE, TRUE, 0);

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio1));
  radio2 = gtk_radio_button_new_with_mnemonic (group, "_Click");
  gtk_widget_set_name (radio2, "SCALE:click_radio_button");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), radio2,
    "Use mouse clicks and key presses to pan and zoom (useful for large data).\n(To reset, see Reset in main menubar)",
    NULL);
  gtk_box_pack_start (GTK_BOX (hbox), radio2, TRUE, TRUE, 0);

  /*-- frame and vbox for drag-style controls --*/
  frame = gtk_frame_new ("Drag-style controls");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      frame, false, false, 0);

  vbox = gtk_vbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

  tgl = gtk_check_button_new_with_mnemonic ("Fixed _aspect");
  gtk_widget_set_name (tgl, "SCALE:drag_aspect_ratio_tgl");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
    "Fix the aspect ratio while zooming in the drag interaction style.",
    NULL);
  g_signal_connect (G_OBJECT (tgl), "toggled",
    G_CALLBACK (drag_aspect_ratio_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 3);

  /*-- frame and vbox for click-style controls --*/
  frame = gtk_frame_new ("Click-style controls");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      frame, false, false, 0);

  vbox = gtk_vbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
  gtk_container_add (GTK_CONTAINER (frame), vbox);

 /*-- pan or zoom radio buttons --*/
  f = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (f), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (vbox), f, false, false, 0);

  hbox = gtk_hbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
  gtk_container_add (GTK_CONTAINER (f), hbox);

  pan_radio = gtk_radio_button_new_with_mnemonic (NULL, "_Pan");
  gtk_widget_set_name (pan_radio, "SCALE:pan_radio_button");
  gtk_widget_set_sensitive (pan_radio, false);
  GTK_TOGGLE_BUTTON (pan_radio)->active = true;
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), pan_radio,
    "Pan: Hit the space bar to pan. A vector drawn on the plot dictates direction and distance. Drag the mouse to control the vector, and keep it small. The arrow keys work, too.  To reset, use `Reset pan' under the main menubar",
    NULL);
  g_signal_connect (G_OBJECT (pan_radio), "toggled",
                      G_CALLBACK (clickoptions_cb), gg);
  gtk_box_pack_start (GTK_BOX (hbox), pan_radio, TRUE, TRUE, 0);

  group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (pan_radio));
  zoom_radio = gtk_radio_button_new_with_mnemonic (group, "_Zoom");
  gtk_widget_set_name (zoom_radio, "SCALE:zoom_radio_button");
  gtk_widget_set_sensitive (zoom_radio, false);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), zoom_radio,
    "Zoom: hit > to zoom in and < to zoom out. A rectangle drawn on the plot dictates the degree of zoom. Drag the mouse to control the rectangle, and keep it large. To reset, use `Reset zoom' under the main menubar",
    NULL);
  gtk_box_pack_start (GTK_BOX (hbox), zoom_radio, TRUE, TRUE, 0);

 /*-- panning controls --*/

  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Pan _options:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  pan_option_menu = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), pan_option_menu);
  gtk_widget_set_name (pan_option_menu, "SCALE:pan_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), pan_option_menu,
    "Specify constraints on click-style panning.",
    NULL);
  gtk_box_pack_end (GTK_BOX (vb), pan_option_menu, false, false, 0);
  populate_combo_box (pan_option_menu, panoptions_lbl, G_N_ELEMENTS(panoptions_lbl),
    G_CALLBACK(panoptions_cb), gg);

 /*-- zooming controls --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Zoom o_ptions:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  zoom_option_menu = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), zoom_option_menu);
  gtk_widget_set_name (zoom_option_menu, "SCALE:zoom_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), zoom_option_menu,
    "Specify constraints on click-style zooming.",
    NULL);
  gtk_box_pack_end (GTK_BOX (vb), zoom_option_menu, false, false, 0);
  populate_combo_box (zoom_option_menu, zoomoptions_lbl, G_N_ELEMENTS(zoomoptions_lbl),
    G_CALLBACK(zoomoptions_cb), gg);

  /*-- start with dragging on by default --*/
  gtk_widget_set_sensitive (pan_radio, false);
  gtk_widget_set_sensitive (zoom_radio, false);
  gtk_widget_set_sensitive (pan_option_menu, false);
  gtk_widget_set_sensitive (zoom_option_menu, false);

  gtk_widget_show_all (panel->w);
}

void
scale_click_zoom_rect_calc (splotd *sp, gint sc_zoom_opt, ggobid *gg) {
  icoords mid;
  mid.x = sp->max.x / 2;
  mid.y = sp->max.y / 2;

  if (sp->mousepos.x <= mid.x && sp->mousepos.y <= mid.y) {
    /* upper left quadrant of plot, based on the value of mid */
    gg->scale.click_rect.x = sp->mousepos.x;
    gg->scale.click_rect.y = sp->mousepos.y;
  } else if (sp->mousepos.x <= mid.x && sp->mousepos.y > mid.y) {
    /* lower left quadrant of plot */
    gg->scale.click_rect.x = sp->mousepos.x;
    gg->scale.click_rect.y = mid.y - (sp->mousepos.y - mid.y);
  } else if (sp->mousepos.x > mid.x && sp->mousepos.y > mid.y) {
    /* lower right quadrant of plot */
    gg->scale.click_rect.x = mid.x - (sp->mousepos.x - mid.x);
    gg->scale.click_rect.y = mid.y - (sp->mousepos.y - mid.y);
  } else if (sp->mousepos.x > mid.x && sp->mousepos.y <= mid.y) {
    /* upper right quadrant of plot */
    gg->scale.click_rect.x = mid.x - (sp->mousepos.x - mid.x);
    gg->scale.click_rect.y = sp->mousepos.y;
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
scaling_visual_cues_draw (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  
  switch (cpanel->scale_style) {

    case DRAG:
      /*-- draw horizontal line --*/
      gdk_draw_line (drawable, gg->plot_GC,  
        0, sp->da->allocation.height/2,  
        sp->da->allocation.width, sp->da->allocation.height/2);
      /*-- draw vertical line --*/
      gdk_draw_line (drawable, gg->plot_GC,
        sp->da->allocation.width/2, 0,
        sp->da->allocation.width/2, sp->da->allocation.height);
      break;

    case CLICK:
      switch (cpanel->scale_click_opt) {
        case PAN:
          gdk_draw_line (drawable, gg->plot_GC,
            sp->max.x/2, sp->max.y/2,
            sp->mousepos.x, sp->mousepos.y);
          break;
        case ZOOM:
          scale_click_zoom_rect_calc (sp, cpanel->scale_zoom_opt, gg);
          gdk_draw_rectangle (drawable, gg->plot_GC, false,
            gg->scale.click_rect.x, gg->scale.click_rect.y,
            gg->scale.click_rect.width, gg->scale.click_rect.height);
          break;
      }  /*-- end switch (scale_click_opt) --*/
  }  /*-- end switch (scale_style) --*/
}

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

void
cpanel_scale_init (cpaneld *cpanel, ggobid *gg) {

  cpanel->scale_style = DRAG;
  cpanel->scale_click_opt = PAN;
  cpanel->scale_drag_aspect_p = false;
  cpanel->scale_pan_opt = P_OBLIQUE;
  cpanel->scale_zoom_opt = Z_OBLIQUE;
}

void
cpanel_scale_set (displayd *display, cpaneld *cpanel, ggobid *gg) {
  GtkWidget *w;
  GtkWidget *panel = mode_panel_get_by_name(GGOBI(getIModeName)(SCALE), gg);

  if (panel == (GtkWidget *) NULL)
    return;

  /*-- set the Drag or Click radio buttons --*/
  if (cpanel->scale_style == DRAG)
    w = widget_find_by_name (panel,
      "SCALE:drag_radio_button");
  else
    w = widget_find_by_name (panel,
      "SCALE:click_radio_button");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), true);

  w = widget_find_by_name (panel,
    "SCALE:drag_aspect_ratio_tgl");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w),
    cpanel->scale_drag_aspect_p);

  /*-- set the Pan or Zoom radio buttons --*/
  w = widget_find_by_name (panel,
    "SCALE:pan_radio_button");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w),
    cpanel->scale_click_opt == PAN);

  /*-- set the Pan options --*/
  w = widget_find_by_name (panel,
    "SCALE:pan_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX(w), cpanel->scale_pan_opt);

  /*-- set the Zoom options--*/
  w = widget_find_by_name (panel,
                           "SCALE:zoom_option_menu");
  gtk_combo_box_set_active (GTK_COMBO_BOX(w), cpanel->scale_zoom_opt);
}
