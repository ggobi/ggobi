/* barchart_ui.c */
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
 *
 * Contributing author of barchart and histogram code:  Heike Hofmann
*/

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "vars.h"
#include "externs.h"

static gchar *display_mode_lbl[] = { "Bars", "Spines" };

static gboolean barchart_scale (gboolean button1_p, gboolean button2_p,
                                splotd * sp);

void barchart_set_initials (splotd * sp, GGobiStage * d);
void barchart_allocate_structure (splotd * sp, GGobiStage * d);
extern void barchart_set_breakpoints (gfloat width, splotd * sp,
                                      GGobiStage * d);

static void
display_mode_cb (GtkWidget * w, ggobid * gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (gg->current_splot);
  displayd *display = gg->current_display;

  cpanel->barchart_display_mode =
    gtk_combo_box_get_active (GTK_COMBO_BOX (w));
  sp->bar->is_spine = !sp->bar->is_spine;
  scatterplot_show_vrule (display, !sp->bar->is_spine);
}

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

GtkWidget *
cpanel_barchart_make (ggobid * gg)
{
  GtkWidget *vb, *lbl, *opt;
  GtkWidget *panel;

  panel = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel), 5);

/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Display _mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  //opt = gtk_option_menu_new();
  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), opt);
  gtk_widget_set_name (opt, "BARCHART:display_mode_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                        "switch between height (bars) and width (spines) to represent count",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, display_mode_lbl, G_N_ELEMENTS (display_mode_lbl),
                      G_CALLBACK (display_mode_cb), gg);

  gtk_widget_show_all (panel);

  return (panel);
}

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

static const gchar *mode_ui_str = "<ui>" "	<menubar>"
/*"		<menu action='PMode'>"
"			<menuitem action='ExtendedDisplayPMode'/>"
"			<menuitem action='1D Tour'/>"
"		</menu>"*/
  "		<menu action='IMode'>"
  "			<menuitem action='DefaultIMode'/>" "			<separator/>"
#if BARCHART_SCALE
  "			<menuitem action='Scale'/>"
#endif
  "			<menuitem action='Brush'/>"
  "			<menuitem action='Identify'/>" "		</menu>" "	</menubar>" "</ui>";


const gchar *
barchart_mode_ui_get (displayd * display)
{
  return (mode_ui_str);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists barchart_cpanel_init --*/

void
cpanel_barchart_set (displayd * display, cpaneld * cpanel, GtkWidget * panel,
                     ggobid * gg)
{
  GtkWidget *w;

  w = widget_find_by_name (panel, "BARCHART:display_mode_option_menu");

  gtk_combo_box_set_active (GTK_COMBO_BOX (w), cpanel->barchart_display_mode);
}


/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean reallocate = FALSE;

  displayd *display = (displayd *) sp->displayptr;
  GGobiStage *d = display->d;

  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);

/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/
  switch (event->keyval) {
  case GDK_plus:
    if (GGOBI_STAGE_IS_COL_CATEGORICAL(d, sp->p1dvar)) {
      bsp->bar->new_nbins = bsp->bar->nbins + 1;
      reallocate = TRUE;
    }
    break;

  case GDK_minus:
    if (!GGOBI_STAGE_IS_COL_CATEGORICAL(d, sp->p1dvar)) {
      if (bsp->bar->nbins > 2) {
        bsp->bar->new_nbins = bsp->bar->nbins - 1;
        reallocate = TRUE;
      }
    }
    break;
  }

  if (reallocate) {
    displayd *display = (displayd *) sp->displayptr;
    GGobiStage *d = display->d;
    ggobid *gg = GGobiFromSPlot (sp);

    barchart_allocate_structure (sp, d);
    barchart_set_initials (sp, d);
    barchart_recalc_counts (GGOBI_BARCHART_SPLOT (sp), d, gg);
    sp->redraw_style = FULL;
    splot_redraw (sp, sp->redraw_style, gg);

    return true;
  }


  return false;
}

static gint
button_release_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;

  disconnect_motion_signal (sp);

  gg->buttondown = 0;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);

  gdk_pointer_ungrab (event->time);

  /*
   * When the mouse comes up, return to the default cursor.  If it's
   * actually still inside one of the triangular regions, it will
   * be restored to the special cursor as soon as the mouse moves.
   * If we want to be really precise, we can check the regions here, too.
   */
  splot_cursor_set ((gint) NULL, sp);
  sp->cursor = NULL;

  return retval;
}

static gint
mouse_motion_notify_cb (GtkWidget * w, GdkEventMotion * event, splotd * sp)
{
  gboolean button1_p, button2_p;
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x
      && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (bsp->bar->is_histogram) {
    GdkRegion *region;
    gboolean cursor_set = FALSE;

    /* This should be determined by when the button is pressed, and
     * not re-decided while the button is moving -- dfs */
    region = gdk_region_polygon (bsp->bar->anchor_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region, sp->mousepos.x, sp->mousepos.y)) {
      splot_cursor_set (GDK_HAND2, sp);
      cursor_set = TRUE;
    }
    gdk_region_destroy (region);

    region = gdk_region_polygon (bsp->bar->offset_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region, sp->mousepos.x, sp->mousepos.y)) {
      splot_cursor_set (GDK_BOTTOM_SIDE, sp);
      cursor_set = TRUE;
    }
    gdk_region_destroy (region);

    /* If all buttons are up and we're outside the triangular regions,
       restore the cursor to the default.
     */
    if (!button1_p && !button2_p && !cursor_set && sp->jcursor) {
      splot_cursor_set ((gint) NULL, sp);
      sp->cursor = NULL;
    }
  }

  if (button1_p || button2_p) {
    barchart_scale (button1_p, button2_p, sp);
  }

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return true;
}

/* Reset bin width or anchor point */
gboolean
barchart_scale (gboolean button1_p, gboolean button2_p, splotd * sp)
{
  displayd *display = sp->displayptr;
  ggobid *gg = GGobiFromSPlot (sp);
  cpaneld *cpanel = &display->cpanel;
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);
  GGobiStage *d = display->d;
  GGobiExtendedSPlotClass *klass;
  klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x
      && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (bsp->bar->is_histogram &&
      (bsp->bar->anchor_drag || bsp->bar->width_drag)) {
    gint idy = sp->mousepos.y - sp->mousepos_o.y;
    greal dy = (greal) idy;
    fcoords pts1, pts2;

    if (bsp->bar->anchor_drag) {
      gfloat scale_y;
      icoords scr;

      if (idy != 0) {
        gboolean set_anchor = TRUE;
        gfloat offset_old = bsp->bar->offset;
        gint pmid_old = sp->pmid.y;

        scr.x = scr.y = 0;
        scale_y = sp->scale.y;
        scale_y /= 2;
        sp->iscale.y = (gfloat) sp->max.y * scale_y;

        klass->screen_to_tform (cpanel, sp, &scr, &pts1, gg);
        sp->pmid.y -= (dy * (greal) PRECISION1 / sp->iscale.y);
        klass->screen_to_tform (cpanel, sp, &scr, &pts2, gg);
        bsp->bar->offset += (pts1.y - pts2.y);

/* moving the anchor more than one binwidth up or down doesn't make sense */
        if (bsp->bar->offset > offset_old) {
          /* not too high */
          set_anchor =
            (bsp->bar->offset < (bsp->bar->breaks[1] - bsp->bar->breaks[0]));
        }
        if (bsp->bar->offset < offset_old) {
          /* and not too low */
          set_anchor =
            (bsp->bar->offset > -(bsp->bar->breaks[1] - bsp->bar->breaks[0]));
        }


        if (set_anchor) {
          barchart_recalc_counts (GGOBI_BARCHART_SPLOT (sp), d, gg);
          splot_redraw (sp, FULL, gg);
        }
        else {
          sp->pmid.y = pmid_old;
          bsp->bar->offset = offset_old;
        }
      }
    }
    else {                      /* if (bsp->bar->width_drag) */

      if (idy != 0) {
        gfloat width, oldwidth;

        klass->screen_to_tform (cpanel, sp, &sp->mousepos_o, &pts1, gg);
        klass->screen_to_tform (cpanel, sp, &sp->mousepos, &pts2, gg);

        oldwidth = bsp->bar->breaks[1] - bsp->bar->breaks[0];
        width = oldwidth - (pts1.y - pts2.y);
        if (width > 0.) {
          gboolean set_breaks = TRUE;
          gint pix_width =
            bsp->bar->bins[0].rect.y - bsp->bar->bins[1].rect.y;
          if (width > oldwidth) {
            set_breaks = bsp->bar->bins[0].rect.y > (sp->max.y / 2 + 1);  /* not too big */
          }
          if (width < oldwidth) {
            set_breaks = pix_width > 6; /* not too small */
          }
          if (set_breaks) {
            barchart_set_breakpoints (width, sp, d);
            barchart_recalc_counts (GGOBI_BARCHART_SPLOT (sp), d, gg);
            splot_redraw (sp, FULL, gg);
          }
        }
      }
    }
  }
  else {     /*-- we're not dragging the bars, only scaling --*/

#if BARCHART_SCALE
    if (button1_p) {
      pan_by_drag (sp, gg);
    }
    else if (button2_p) {
      zoom_by_drag (sp, gg);
    }

        /*-- redisplay this plot --*/
    splot_plane_to_screen (display, &display->cpanel, sp, gg);
    ruler_ranges_set (false, gg->current_display, sp, gg);
    splot_redraw (sp, FULL, gg);
#endif

  }
  return true;
}

static gint
button_press_cb (GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  gboolean button1_p, button2_p;
  GdkRegion *region;
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);

  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;

  if (bsp->bar->is_histogram) {
    bsp->bar->anchor_drag = FALSE;
    bsp->bar->width_drag = FALSE;

    region = gdk_region_polygon (bsp->bar->anchor_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region, sp->mousepos.x, sp->mousepos.y)) {
      bsp->bar->anchor_drag = TRUE;
    }
    gdk_region_destroy (region);

    region = gdk_region_polygon (bsp->bar->offset_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region, sp->mousepos.x, sp->mousepos.y)) {
      bsp->bar->width_drag = TRUE;
    }
    gdk_region_destroy (region);
  }

  sp->motion_id = g_signal_connect (G_OBJECT (sp->da),
                                    "motion_notify_event",
                                    G_CALLBACK (mouse_motion_notify_cb),
                                    (gpointer) sp);

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return retval;
}

void
barchart_event_handlers_toggle (displayd * display, splotd * sp,
                                gboolean state, ProjectionMode pmode,
                                InteractionMode imode)
{
  if (!GGOBI_IS_WINDOW_DISPLAY (display))
    return;

  if (state == on) {
    GtkObject *winobj = GTK_OBJECT (GGOBI_WINDOW_DISPLAY (display)->window);
    /* Mode selection */
    sp->key_press_id =
      g_signal_connect (winobj, "key_press_event",
                        G_CALLBACK (key_press_cb), (gpointer) sp);

    /* Resetting bin width and anchor */
    sp->press_id = g_signal_connect (G_OBJECT (sp->da),
                                     "button_press_event",
                                     G_CALLBACK (button_press_cb),
                                     (gpointer) sp);
    sp->release_id = g_signal_connect (G_OBJECT (sp->da),
                                       "button_release_event",
                                       G_CALLBACK (button_release_cb),
                                       (gpointer) sp);
  }
  else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}
