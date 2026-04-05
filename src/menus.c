/*-- menus.c: menus in the display menubar that change with the mode;
*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*
 * These menus have migrated from the main menubar to the display
 * menubar.
 */

static gboolean
pmode_has_display_menu (ProjectionMode pmode)
{
  return (pmode == TOUR1D || pmode == TOUR2D || pmode == COTOUR);
}


gboolean
imode_has_display_menu (InteractionMode imode)
{
  return (imode == SCALE || imode == BRUSH);
}

static void
display_mode_menu_hide (displayd *display, const gchar *name)
{
  GtkWidget *item = widget_find_by_name (display->menubar, name);

  if (item == NULL)
    return;

  if (GTK_IS_MENU_ITEM (item))
    display_menu_clear (gtk_menu_item_get_submenu (GTK_MENU_ITEM (item)));
  gtk_widget_hide (item);
}

static GtkWidget *
display_mode_menu_prepare (displayd *display, const gchar *label,
                           const gchar *name)
{
  GtkWidget *item;
  GtkWidget *menu = display_menu_ensure (display, label, name);

  display_menu_clear (menu);
  item = widget_find_by_name (display->menubar, name);
  if (item != NULL)
    gtk_widget_show (item);

  return menu;
}

static void
display_menu_append_separator (GtkWidget *menu)
{
  GtkWidget *item = gtk_separator_menu_item_new ();

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  gtk_widget_show (item);
}

static void
menu_exclude_shadowed_points_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_EXCLUDE_SHADOW_POINTS);
}

static void
menu_include_shadowed_points_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_INCLUDE_SHADOW_POINTS);
}

static void
menu_unshadow_all_points_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_UNSHADOW_POINTS);
}

static void
menu_exclude_shadowed_edges_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_EXCLUDE_SHADOW_EDGES);
}

static void
menu_include_shadowed_edges_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_INCLUDE_SHADOW_EDGES);
}

static void
menu_unshadow_all_edges_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_UNSHADOW_EDGES);
}

static void
menu_reset_brush_cb (GtkWidget *item, displayd *display)
{
  brush_reset (display, RESET_INIT_BRUSH);
}

static void
menu_toggle_brush_update_cb (GtkCheckMenuItem *item, displayd *display)
{
  brush_update_set (gtk_check_menu_item_get_active (item), display,
                    display->ggobi);
}

static void
menu_toggle_brush_on_cb (GtkCheckMenuItem *item, displayd *display)
{
  brush_on_set (gtk_check_menu_item_get_active (item), display,
                display->ggobi);
}

static void
menu_reset_pan_cb (GtkWidget *item, displayd *display)
{
  scale_pan_reset (display);
}

static void
menu_reset_zoom_cb (GtkWidget *item, displayd *display)
{
  scale_zoom_reset (display);
}

static void
menu_toggle_scale_update_cb (GtkCheckMenuItem *item, displayd *display)
{
  scale_update_set (gtk_check_menu_item_get_active (item), display,
                    display->ggobi);
}

static void
menu_select_all_1d_cb (GtkWidget *item, displayd *display)
{
  tour1d_all_vars (display);
}

static void
menu_select_all_2d_cb (GtkWidget *item, displayd *display)
{
  tour2d_all_vars (display);
}

static void
menu_toggle_fade_vars_1d_cb (GtkCheckMenuItem *item, displayd *display)
{
  tour1d_fade_vars (gtk_check_menu_item_get_active (item), display->ggobi);
}

static void
menu_toggle_fade_vars_2d_cb (GtkCheckMenuItem *item, displayd *display)
{
  tour2d_fade_vars (gtk_check_menu_item_get_active (item), display->ggobi);
}

static void
menu_toggle_fade_vars_co_cb (GtkCheckMenuItem *item, displayd *display)
{
  tourcorr_fade_vars (gtk_check_menu_item_get_active (item), display->ggobi);
}

static void
display_brush_menu_build (displayd *display)
{
  GtkWidget *menu = display_mode_menu_prepare (display, "_Brush",
                                               "DISPLAY:brush_topmenu");
  GtkAccelGroup *accel_group = display_menu_accel_group_get (display);

  display_menu_append_item (menu, "E_xclude shadowed points", NULL,
                            "Exclude the points that are currently shadowed",
                            "<control>X",
                            G_CALLBACK (menu_exclude_shadowed_points_cb),
                            display, accel_group);
  display_menu_append_item (menu, "_Include shadowed points", NULL,
                            "Include the points that are currently shadowed",
                            "<control>I",
                            G_CALLBACK (menu_include_shadowed_points_cb),
                            display, accel_group);
  display_menu_append_item (menu, "_Unshadow all points", NULL,
                            "Make all points unshadowed",
                            "<control>U",
                            G_CALLBACK (menu_unshadow_all_points_cb),
                            display, accel_group);
  display_menu_append_separator (menu);
  display_menu_append_item (menu, "_Exclude shadowed edges", NULL,
                            "Exclude the edges that are shadowed",
                            "<control>E",
                            G_CALLBACK (menu_exclude_shadowed_edges_cb),
                            display, accel_group);
  display_menu_append_item (menu, "Include s_hadowed edges", NULL,
                            "Include the edges that are shadowed",
                            "<control>H",
                            G_CALLBACK (menu_include_shadowed_edges_cb),
                            display, accel_group);
  display_menu_append_item (menu, "U_nshadow all edges", NULL,
                            "Make all edges unshadowed",
                            "<control>N",
                            G_CALLBACK (menu_unshadow_all_edges_cb),
                            display, accel_group);
  display_menu_append_separator (menu);
  display_menu_append_item (menu, "_Reset brush", NULL,
                            "Reset the size of the brush",
                            "<control>R",
                            G_CALLBACK (menu_reset_brush_cb),
                            display, accel_group);
  display_menu_append_toggle_item (menu, "Update Brushing _Continuously",
                                   NULL,
                                   "Toggle whether the brush operates continuously",
                                   NULL,
                                   display->cpanel.br.updateAlways_p,
                                   G_CALLBACK (menu_toggle_brush_update_cb),
                                   display, accel_group);
  display_menu_append_toggle_item (menu, "Brush _On", NULL,
                                   "Toggle whether the brush is active",
                                   NULL,
                                   display->cpanel.br.brush_on_p,
                                   G_CALLBACK (menu_toggle_brush_on_cb),
                                   display, accel_group);
}

static void
display_scale_menu_build (displayd *display)
{
  GtkWidget *menu = display_mode_menu_prepare (display, "_Scale",
                                               "DISPLAY:scale_topmenu");
  GtkAccelGroup *accel_group = display_menu_accel_group_get (display);

  display_menu_append_item (menu, "Reset _pan", NULL,
                            "Return to initial position", "<control>P",
                            G_CALLBACK (menu_reset_pan_cb),
                            display, accel_group);
  display_menu_append_item (menu, "Reset _zoom", NULL,
                            "Return to initial zoom", "<control>Z",
                            G_CALLBACK (menu_reset_zoom_cb),
                            display, accel_group);
  display_menu_append_toggle_item (menu, "Update _Continuously", NULL,
                                   "Toggle whether panning and zooming operates continuously",
                                   NULL,
                                   display->cpanel.scale.updateAlways_p,
                                   G_CALLBACK (menu_toggle_scale_update_cb),
                                   display, accel_group);
}

static void
display_tour1d_menu_build (displayd *display)
{
  GtkWidget *menu = display_mode_menu_prepare (display, "_Tour1D",
                                               "DISPLAY:tour1d_topmenu");
  GtkAccelGroup *accel_group = display_menu_accel_group_get (display);

  display_menu_append_display_option_item (menu, DOPT_AXES, display);
  display_menu_append_toggle_item (menu, "_Fade Variables on Deselection",
                                   NULL,
                                   "Toggle whether variables fade when de-selected from the 1D tour",
                                   NULL, display->ggobi->tour1d.fade_vars,
                                   G_CALLBACK (menu_toggle_fade_vars_1d_cb),
                                   display, accel_group);
  display_menu_append_item (menu, "_Select all variables", NULL,
                            "Select all variables for this 1D tour",
                            "<control>S",
                            G_CALLBACK (menu_select_all_1d_cb),
                            display, accel_group);
}

static void
display_tour2d_menu_build (displayd *display)
{
  GtkWidget *menu = display_mode_menu_prepare (display, "_Tour2D",
                                               "DISPLAY:tour2d_topmenu");
  GtkAccelGroup *accel_group = display_menu_accel_group_get (display);

  display_menu_append_display_option_item (menu, DOPT_AXES, display);
  display_menu_append_display_option_item (menu, DOPT_AXESLAB, display);
  display_menu_append_display_option_item (menu, DOPT_AXESVALS, display);
  display_menu_append_separator (menu);
  display_menu_append_toggle_item (menu, "_Fade Variables on Deselection",
                                   NULL,
                                   "Toggle whether variables fade when de-selected from the 2D tour",
                                   NULL, display->ggobi->tour2d.fade_vars,
                                   G_CALLBACK (menu_toggle_fade_vars_2d_cb),
                                   display, accel_group);
  display_menu_append_item (menu, "_Select all variables", NULL,
                            "Select all variables for this 2D tour",
                            "<control>S",
                            G_CALLBACK (menu_select_all_2d_cb),
                            display, accel_group);
}

static void
display_cotour_menu_build (displayd *display)
{
  GtkWidget *menu = display_mode_menu_prepare (display, "_Correlation Tour",
                                               "DISPLAY:corrtour_topmenu");

  display_menu_append_display_option_item (menu, DOPT_AXES, display);
  display_menu_append_separator (menu);
  display_menu_append_toggle_item (menu, "_Fade Variables on Deselection",
                                   NULL,
                                   "Toggle whether variables fade when de-selected from the correlation tour",
                                   NULL, display->ggobi->tourcorr.fade_vars,
                                   G_CALLBACK (menu_toggle_fade_vars_co_cb),
                                   display,
                                   display_menu_accel_group_get (display));
}

void
display_mode_menus_update (ProjectionMode pmode_prev,
                           InteractionMode imode_prev, displayd * display,
                           ggobid * gg)
{
  ProjectionMode pmode = display->cpanel.pmode;
  InteractionMode imode = display->cpanel.imode;
  (void) pmode_prev;
  (void) imode_prev;
  (void) gg;

  if (display->menubar == NULL)
    return;

  display_mode_menu_hide (display, "DISPLAY:brush_topmenu");
  display_mode_menu_hide (display, "DISPLAY:scale_topmenu");
  display_mode_menu_hide (display, "DISPLAY:tour1d_topmenu");
  display_mode_menu_hide (display, "DISPLAY:tour2d_topmenu");
  display_mode_menu_hide (display, "DISPLAY:corrtour_topmenu");

  if (imode == BRUSH)
    display_brush_menu_build (display);
  else if (imode == SCALE)
    display_scale_menu_build (display);

  if (imode == DEFAULT_IMODE && pmode_has_display_menu (pmode)) {
    if (pmode == TOUR1D)
      display_tour1d_menu_build (display);
    else if (pmode == TOUR2D)
      display_tour2d_menu_build (display);
    else if (pmode == COTOUR)
      display_cotour_menu_build (display);
  }

}
