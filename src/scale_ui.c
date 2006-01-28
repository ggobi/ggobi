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

static icoords mousedownpos;

void scale_update_set(gboolean update, displayd *dsp, ggobid *gg)
{
  dsp->cpanel.scale_updateAlways_p = update;
}

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
  ggobid *gg = dsp->ggobi;
  splotd *sp = gg->current_splot;
  
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
  GtkWidget *panel = mode_panel_get_by_name(GGOBI(getIModeName)(SCALE), gg);

  if (panel == (GtkWidget *) NULL)
    return;

  cpanel->scale_style = style;  /*-- DRAG or CLICK --*/
  click_p = (cpanel->scale_style == CLICK);

  splot_redraw (gg->current_splot, QUICK, gg);
}

// portions of this will be called in response to keyboard actions
void
interaction_style_cb (GtkToggleButton *w, ggobid *gg) 
{
  gint scale_style = (w->active) ? DRAG : CLICK;
  scale_interaction_style_set (scale_style, gg);
}

// These are just always going to be oblique -- no constraints.
// Call scale_click_init when click is turned on;
#if 0  // NOCLICK
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

void panoptions_set (gint pan_opt, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->scale_pan_opt = pan_opt;
}
static void panoptions_cb (GtkWidget *w, ggobid *gg)
{
  gint pan_opt = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  panoptions_set (pan_opt, gg);
}
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
#endif

/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

/*
If ctrl-alt-p is pressed, toggle on and off click-style panning.
If ctrl-alt-z is pressed, toggle on and off click-style zooming.
*/

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

      if (cpanel->scale_updateAlways_p) {
        /*-- redisplay this plot --*/
        splot_plane_to_screen (display, &display->cpanel, sp, gg);
        ruler_ranges_set (false, gg->current_display, sp, gg);
        splot_redraw (sp, FULL, gg);
      } else {
        splot_redraw (sp, QUICK, gg);
      }
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

  mousedownpos.x = sp->mousepos.x;
  mousedownpos.y = sp->mousepos.y;

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
  displayd *dsp = sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;

  gg->buttondown = 0;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y, &state);

  gdk_pointer_ungrab (event->time);
  disconnect_motion_signal (sp);

  if (!cpanel->scale_updateAlways_p) {
    displayd *display = sp->displayptr;
    /*-- redisplay this plot --*/
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    ruler_ranges_set (false, gg->current_display, sp, gg);
    splot_redraw (sp, FULL, gg);
  } else {
    splot_redraw (sp, QUICK, gg);
  }

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
  GtkWidget *f, *vbox, *hbox, *vb, *lbl;
  GtkWidget *tgl;

  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup(GGOBI(getIModeName)(SCALE));
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);


  vbox = gtk_vbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
  /*-- frame and vbox for drag-style controls --*/
  gtk_box_pack_start (GTK_BOX (panel->w),
                      vbox, false, false, 0);

  tgl = gtk_check_button_new_with_mnemonic ("Fixed _aspect");
  gtk_widget_set_name (tgl, "SCALE:drag_aspect_ratio_tgl");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
    "Fix the aspect ratio while zooming in the drag interaction style.",
    NULL);
  g_signal_connect (G_OBJECT (tgl), "toggled",
    G_CALLBACK (drag_aspect_ratio_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (vbox), tgl, false, false, 3);

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
      if (!cpanel->scale_updateAlways_p) {
	if (gg->buttondown)
          gdk_draw_line (drawable, gg->plot_GC,
            mousedownpos.x, mousedownpos.y,
            sp->mousepos.x, sp->mousepos.y);
      }
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

  cpanel->scale_updateAlways_p = true;
  cpanel->scale_style = DRAG;

  // Invisible options
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

  w = widget_find_by_name (panel,
    "SCALE:drag_aspect_ratio_tgl");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w),
    cpanel->scale_drag_aspect_p);
}

