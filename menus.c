/*-- menus.c: menus in the main menubar that change with the mode: --*/
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

/*--          Options, Reset, and I/O menus for all modes --*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * ..._item is the widget you see in the main menubar
 * ..._menu is the submenu attached to ..._item
*/

/*--------------------------------------------------------------------*/
/*                   Tours: Options menus                             */
/*--------------------------------------------------------------------*/

void
tour1d_option_items_add (ggobid *gg) {
  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (gg->menus.options_menu, "Fade variables on de-selection",
    GTK_SIGNAL_FUNC (tour1d_fade_vars_cb), NULL,
    gg->tour1d.fade_vars, gg);

  CreateMenuCheck (gg->menus.options_menu, "Select all variables",
    GTK_SIGNAL_FUNC (tour1d_all_vars_cb), NULL,
    gg->tour1d.all_vars, gg);
}

void
tour2d_option_items_add (ggobid *gg) {

  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (gg->menus.options_menu, "Fade variables on de-selection",
    GTK_SIGNAL_FUNC (tour2d_fade_vars_cb), NULL,
    gg->tour2d.fade_vars, gg);

  CreateMenuCheck (gg->menus.options_menu, "Select all variables",
    GTK_SIGNAL_FUNC (tour2d_all_vars_cb), NULL,
    gg->tour2d.all_vars, gg);
}

void
cotour_option_items_add (ggobid *gg) {
  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (gg->menus.options_menu, "Fade variables on de-selection",
    GTK_SIGNAL_FUNC (tourcorr_fade_vars_cb), NULL,
    gg->tourcorr.fade_vars, gg);
}

/*--------------------------------------------------------------------*/
/*                   Scaling: Reset and Options menus                 */
/*--------------------------------------------------------------------*/

void
scale_reset_items_add(ggobid *gg) {
  void pan_reset_cb (GtkWidget *w, ggobid *gg);
  void zoom_reset_cb (GtkWidget *w, ggobid *gg);
  GtkWidget *item;

  item = gtk_menu_item_new_with_label ("Reset pan");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (pan_reset_cb),
                      (gpointer) gg);
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset zoom");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (zoom_reset_cb),
                      (gpointer) gg);
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);
}

/*--------------------------------------------------------------------*/
/*               Brushing: Reset and Options menus                    */
/*--------------------------------------------------------------------*/

void
brush_reset_items_add (ggobid *gg)
{
  GtkWidget *item;

/*
   Adding include/exclude for points; do I need to add menu items
   for the edges as well?  -- dfs
*/
  item = gtk_menu_item_new_with_label ("Exclude shadowed points in current display");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_EXCLUDE_SHADOW_POINTS));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Include shadowed points in current display");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_INCLUDE_SHADOW_POINTS));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Un-shadow all points in current display");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_UNSHADOW_POINTS));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Exclude shadowed edges in current display");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_EXCLUDE_SHADOW_EDGES));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Include shadowed edges in current display");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_INCLUDE_SHADOW_EDGES));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Un-shadow all edges in current display");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_UNSHADOW_EDGES));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset brush size");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_INIT_BRUSH));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);
}

void
brush_option_items_add(ggobid *gg)
{
  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, NULL);

  CreateMenuCheck (gg->menus.options_menu, "Update brushing continuously",
    GTK_SIGNAL_FUNC (brush_update_set_cb), NULL,
    gg->brush.updateAlways_p, gg);
}

/*--------------------------------------------------------------------*/
/*               Routines to manage the mode menus                    */
/*--------------------------------------------------------------------*/

gboolean
imode_has_reset_menu (InteractionMode imode)
{
  return (imode == SCALE  || imode == BRUSH);
}

/*-- make the menu items once, and then show/hide them as necessary --*/
void
main_miscmenus_initialize (ggobid *gg)
{
  gg->menus.options_item = NULL;
  gg->menus.reset_item = NULL;
}

void
main_options_menu_update (ProjectionMode pmode_prev, InteractionMode imode_prev, displayd *prev_display, ggobid *gg)
{
  ProjectionMode pmode = pmode_get (gg);
  InteractionMode imode = imode_get (gg);

  /* the options menu is always present */
  if (gg->menus.options_item) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (gg->menus.options_item));
  } else {
    /*-- create and insert menu new item --*/
      gg->menus.options_item = submenu_make ("_Options", 'O',
        gg->main_accel_group);
      submenu_insert (gg->menus.options_item,
        gg->main_menubar, 4);
  }

  gg->menus.options_menu = gtk_menu_new ();

  /* These three items are always present */

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->imode_frame), gg);

  CreateMenuCheck (gg->menus.options_menu, "Show status bar",
    GTK_SIGNAL_FUNC (statusbar_show_cb), NULL,
    gg->statusbar_p, gg);

  /* This should probably be class-specific */
  if (pmode == TOUR1D && imode == DEFAULT_IMODE) {
    tour1d_option_items_add (gg);
  } else if (pmode == TOUR2D && imode == DEFAULT_IMODE) {
    tour2d_option_items_add (gg);
  } else if (pmode == COTOUR && imode == DEFAULT_IMODE) {
    cotour_option_items_add (gg);
  } else if (imode == BRUSH) {
    brush_option_items_add(gg);
  }

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);

}
void
main_reset_menu_update (ProjectionMode pmode_prev, InteractionMode imode_prev, displayd *prev_display, ggobid *gg)
{
  InteractionMode imode = imode_get (gg);

  /* if the previous menu had a reset menu ... */
  if (imode_has_reset_menu (imode_prev) && gg->menus.reset_item) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (gg->menus.reset_item));
    if (!imode_has_reset_menu (imode)) {
      if (gg->menus.reset_item != NULL) {
        gtk_widget_destroy (gg->menus.reset_item);
        gg->menus.reset_item = NULL;
      }
    }
  } else { /* else if it didn't ... */
    if (imode_has_reset_menu (imode) && gg->menus.reset_item == NULL) {
      gg->menus.reset_item = submenu_make ("_Reset", 'R',
        gg->main_accel_group);
      submenu_insert (gg->menus.reset_item,
        gg->main_menubar, 5);
    } 
  }

  /* If the new mode has a reset menu, build it */
  if (imode_has_reset_menu(imode)) {
    gg->menus.reset_menu = gtk_menu_new ();

    if (imode == BRUSH) {
      brush_reset_items_add (gg);
    } else if (imode == SCALE) {
      scale_reset_items_add (gg);
    }

    gtk_widget_show_all (gg->menus.reset_menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.reset_item),
     gg->menus.reset_menu);
  }
}

void
main_miscmenus_update (ProjectionMode pmode_prev, InteractionMode imode_prev, displayd *prev_display,
  ggobid *gg)
{

  /*
Perhaps the thing to do is to destroy these menus right here, then
call class-specific menus to rebuild them.
  */

  main_options_menu_update(pmode_prev, imode_prev, prev_display, gg);
  main_reset_menu_update(pmode_prev, imode_prev, prev_display, gg);

  /*
  Should use instead klass->option_items_add() which does nothing,
  and maybe klass->reset_items_add() as well.
  if (pmode == EXTENDED_DISPLAY_MODE && imode == DEFAULT) {
      displayd *dpy = gg->current_display;
      if(GTK_IS_GGOBI_EXTENDED_DISPLAY(dpy)) {
        GtkGGobiExtendedDisplayClass *klass;
        klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(dpy));
        klass->menus_make(dpy, mode, gg);
      }
    }
  */

}

