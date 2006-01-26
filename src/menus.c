/*-- menus.c: menus in the display menubar that change with the mode;
*/
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
/*               Routines to manage the mode menus                    */
/*--------------------------------------------------------------------*/

#if 0
/*-- make the menu items once, and then show/hide them as necessary --*/
void
main_miscmenus_initialize (ggobid *gg)
{
  gg->menus.options_item = NULL;
}


/* This menu no longer needs updating, but can become part of the
   ItemFactory in main_ui.c */
void
main_options_menu_update (ProjectionMode pmode_prev, InteractionMode imode_prev, displayd *prev_display, ggobid *gg)
{
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
    G_CALLBACK (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    G_CALLBACK (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->imode_frame), gg);

  CreateMenuCheck (gg->menus.options_menu, "Show status bar",
    G_CALLBACK (statusbar_show_cb), NULL,
    gg->statusbar_p, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
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

  /*
  Should use instead klass->option_items_add() which does nothing,
  if (pmode == EXTENDED_DISPLAY_MODE && imode == DEFAULT) {
      displayd *dpy = gg->current_display;
      if(GGOBI_IS_EXTENDED_DISPLAY(dpy)) {
        GGobiExtendedDisplayClass *klass;
        klass = GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(dpy));
        klass->menus_make(dpy, mode, gg);
      }
    }
  */

}

#endif


/*
 * These menus have migrated from the main menubar to the display
 * menubar.
 */

static gboolean
pmode_has_display_menu (ProjectionMode pmode)
{
  return (pmode == TOUR1D  || pmode == TOUR2D || pmode == COTOUR);
}

static const gchar *tour1d_pmode_ui =
"<ui>"
"	<menubar>"
"		<menu action='Tour1D'>"
"			<menuitem action='ShowAxes'/>"
"			<menuitem action='FadeVariables1D'/>"
"			<menuitem action='SelectAllVariables1D'/>"
"		</menu>"
"	</menubar>"
"</ui>";

#if 0
void
tour1d_display_pmode_items_add (GtkWidget *menu, ggobid *gg)
{
  displayd *display = gg->current_display;
  GtkWidget *item;
  GCallback func = G_CALLBACK(display_options_cb);

  item = CreateMenuCheck (menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), display->options.axes_show_p, gg);
  gtk_widget_set_name (item, "DISPLAY:show_axes");
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem (menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (menu, "Fade variables on de-selection",
    G_CALLBACK (tour1d_fade_vars_cb), NULL,
    gg->tour1d.fade_vars, gg);

  CreateMenuCheck (menu, "Select all variables",
    G_CALLBACK (tour1d_all_vars_cb), NULL,
    gg->tour1d.all_vars, gg);
}
#endif

static const gchar* tour2d_pmode_ui =
"<ui>"
"	<menubar>"
"		<menu action='Tour2D'>"
"			<menuitem action='ShowAxes'/>"
"			<menuitem action='ShowAxesLabels'/>"
"			<menuitem action='ShowAxesVals'/>"
"			<separator/>"
"			<menuitem action='FadeVariables2D'/>"
"			<menuitem action='SelectAllVariables2D'/>"
"		</menu>"
"	</menubar>"
"</ui>";

#if 0
void
tour2d_display_pmode_items_add (GtkWidget *menu, ggobid *gg) {
  displayd *display = gg->current_display;
  GtkWidget *item;
  GtkSignalFunc func = G_CALLBACK(display_options_cb);

  item = CreateMenuCheck (menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), display->options.axes_show_p, gg);
  gtk_widget_set_name (item, "DISPLAY:show_axes");
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

  item = CreateMenuCheck (menu, "Show 2D tour axes as text",
    func, GINT_TO_POINTER (DOPT_AXESLAB), off, gg);
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

  item = CreateMenuCheck (menu, "Show 2D tour proj vals",
    func, GINT_TO_POINTER (DOPT_AXESVALS), on, gg);
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem (menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (menu, "Fade variables on de-selection",
    G_CALLBACK (tour2d_fade_vars_cb), NULL,
    gg->tour2d.fade_vars, gg);

  CreateMenuCheck (menu, "Select all variables",
    G_CALLBACK (tour2d_all_vars_cb), NULL,
    gg->tour2d.all_vars, gg);
}
#endif

static const gchar *cotour_pmode_ui =
"<ui>"
"	<menubar>"
"		<menu action='CorrTour'>"
"			<menuitem action='ShowAxes'/>"
"			<separator/>"
"			<menuitem action='FadeVariablesCo'/>"
"		</menu>"
"	</menubar>"
"</ui>";

#if 0
void
cotour_display_pmode_items_add (GtkWidget *menu, ggobid *gg) {
  displayd *display = gg->current_display;
  GtkWidget *item;
  GtkSignalFunc func = G_CALLBACK(display_options_cb);

  item = CreateMenuCheck (menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), display->options.axes_show_p, gg);
  gtk_widget_set_name (item, "DISPLAY:show_axes");
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem (menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (menu, "Fade variables on de-selection",
    G_CALLBACK (tourcorr_fade_vars_cb), NULL,
    gg->tourcorr.fade_vars, gg);
}
#endif

static const gchar *brush_imode_ui =
"<ui>"
"	<menubar>"
"		<menu action='Brush'>"
"			<menuitem action='ExcludeShadowedPoints'/>"
"			<menuitem action='IncludeShadowedPoints'/>"
"			<menuitem action='UnshadowAllPoints'/>"
"			<separator/>"
"			<menuitem action='ExcludeShadowedEdges'/>"
"			<menuitem action='IncludeShadowedEdges'/>"
"			<menuitem action='UnshadowAllEdges'/>"
"			<separator/>"
"			<menuitem action='ResetBrushSize'/>"
"			<menuitem action='UpdateBrushContinuously'/>"
"			<menuitem action='BrushOn'/>"
"		</menu>"
"	</menubar>"
"</ui>";

#if 0
void
brush_display_imode_items_add (GtkWidget *menu, ggobid *gg)
{
  GtkWidget *item;

/*
   Adding include/exclude for points; do I need to add menu items
   for the edges as well?  -- dfs
*/
  item = gtk_menu_item_new_with_label ("Exclude shadowed points");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_EXCLUDE_SHADOW_POINTS));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Include shadowed points");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_INCLUDE_SHADOW_POINTS));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Un-shadow all points");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_UNSHADOW_POINTS));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Exclude shadowed edges");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_EXCLUDE_SHADOW_EDGES));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Include shadowed edges");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_INCLUDE_SHADOW_EDGES));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Un-shadow all edges");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_UNSHADOW_EDGES));
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Reset brush size");
  GGobi_widget_set (item, gg, true);
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (RESET_INIT_BRUSH));
  gtk_menu_append (GTK_MENU (menu), item);

  /* Add a separator before the option */
  CreateMenuItem (menu, NULL,
    "", "", NULL, NULL, NULL, NULL, NULL);

  CreateMenuCheck (menu, "Update brushing continuously",
    G_CALLBACK (brush_update_set_cb), NULL,
    gg->brush.updateAlways_p, gg);
  CreateMenuCheck (menu, "Brush on",
    G_CALLBACK (brush_on_set_cb), NULL,
    gg->brush.brushOn_p, gg);

}
#endif

static const gchar* scale_imode_ui =
"<ui>"
"	<menubar>"
"		<menu action='Scale'>"
"			<menuitem action='ResetPan'/>"
"			<menuitem action='ResetZoom'/>"
"		</menu>"
"	</menubar>"
"</ui>";

#if 0
void
scale_display_imode_items_add (GtkWidget *menu, ggobid *gg) {
  void pan_reset_cb (displayd *display);
  void zoom_reset_cb (displayd *display);
  GtkWidget *item;

  item = gtk_menu_item_new_with_label ("Reset pan");
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (pan_reset_cb),
                      (gpointer) gg);
  gtk_menu_append (GTK_MENU (menu), item);

  item = gtk_menu_item_new_with_label ("Reset zoom");
  g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (zoom_reset_cb),
                      (gpointer) gg);
  gtk_menu_append (GTK_MENU (menu), item);
}
#endif

gboolean
imode_has_display_menu (InteractionMode imode)
{
  return (imode == SCALE  || imode == BRUSH);
}

void
display_mode_menus_update (ProjectionMode pmode_prev, InteractionMode imode_prev, 
  displayd *display, ggobid *gg)
{
   ProjectionMode pmode = display->cpanel.pmode;
  InteractionMode imode = display->cpanel.imode; 
  /* or imode_get (gg) */
  const gchar *ui;
  GError *error = NULL;
  
  if (imode != imode_prev) {
    /* Remove any existing imode submenu */
	if (imode_has_display_menu(imode_prev)) {
      gtk_ui_manager_remove_ui(display->menu_manager, display->imode_merge_id);
	  /* I don't understand why all these tests are necessary ... dfs */
      /*if (GTK_IS_MENU_ITEM(display->imode_item))
        gtk_menu_item_remove_submenu (GTK_MENU_ITEM (display->imode_item));
      if (GTK_IS_WIDGET(display->imode_item))
        gtk_widget_destroy (display->imode_item);
      display->imode_item = NULL;*/
    }

    /* If the new mode has an imode menu, build it */
    if (imode_has_display_menu (imode)) {
	  if (imode == BRUSH) {
		 ui = brush_imode_ui;
      } else if (imode == SCALE) {
         ui = scale_imode_ui;
      }
	  
	  display->imode_merge_id = 
	  	gtk_ui_manager_add_ui_from_string(display->menu_manager, ui, -1, &error);
	  if (error) {
		  g_message("Failed to load display imode ui!\n");
		  g_error_free(error);
	  }

      #if 0
	  GtkWidget *imode_menu;
      /* GtkAccelGroup *accel_group = gtk_accel_group_new(); not yet ...*/
	  
      display->imode_item = gtk_menu_item_new_with_label ((gchar *)GGOBI(getIModeName)(imode));
      gtk_widget_show (display->imode_item);
      submenu_insert (display->imode_item, display->menubar, 2);

      imode_menu = gtk_menu_new ();

      if (imode == BRUSH) {
        brush_display_imode_items_add (imode_menu, gg);
      } else if (imode == SCALE) {
        scale_display_imode_items_add (imode_menu, gg);
      }

      gtk_widget_show_all (imode_menu);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (display->imode_item), imode_menu);
	  #endif
    }
  }

  /* I'm not completely certain I have the correct test here */
  if (pmode != pmode_prev && imode == DEFAULT_IMODE) {

    /* Remove any existing pmode submenu */
    if (pmode_has_display_menu(pmode_prev) && display->pmode_merge_id) {
      gtk_ui_manager_remove_ui(display->menu_manager, display->pmode_merge_id);
	  /*gtk_menu_item_remove_submenu (GTK_MENU_ITEM (display->pmode_item));
      gtk_widget_destroy (display->pmode_item);
      display->pmode_item = NULL;*/
    }

    /* Do we need to be in the default imode for this?  I don't see
       why */

    /* If the new mode has a pmode menu, build it */
    if (pmode_has_display_menu (pmode)) {
	  if (pmode == TOUR1D) {
		  ui = tour1d_pmode_ui;
      } else if (pmode == TOUR2D) {
		  ui = tour2d_pmode_ui;
      } else if (pmode == COTOUR) {
          ui = cotour_pmode_ui;
      } 
	  
	  display->pmode_merge_id = 
	  	gtk_ui_manager_add_ui_from_string(display->menu_manager, ui, -1, &error);
	  if (error) {
		  g_message("Failed to load display pmode ui!\n");
		  g_error_free(error);
	  }
	  
      #if 0
	  GtkWidget *pmode_menu;
      /* GtkAccelGroup *accel_group = gtk_accel_group_new(); not yet ...*/

      display->pmode_item = gtk_menu_item_new_with_label ((gchar *)GGOBI(getPModeName)(pmode));
      gtk_widget_show (display->pmode_item);
      submenu_insert (display->pmode_item, display->menubar, 3);

      pmode_menu = gtk_menu_new ();

      if (pmode == TOUR1D) {
        tour1d_display_pmode_items_add (pmode_menu, gg);
      } else if (pmode == TOUR2D) {
        tour2d_display_pmode_items_add (pmode_menu, gg);
      } else if (pmode == COTOUR) {
        cotour_display_pmode_items_add (pmode_menu, gg);
      } 

      gtk_widget_show_all (pmode_menu);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (display->pmode_item), pmode_menu);
	  #endif
    }


  }

}
