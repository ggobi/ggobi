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

static const gchar *tour1d_pmode_ui =
  "<ui>"
  "	<menubar>"
  "		<menu action='Tour1D'>"
  "			<menuitem action='ShowAxes'/>"
  "			<menuitem action='FadeVariables1D'/>"
  "			<menuitem action='SelectAllVariables1D'/>"
  "		</menu>" "	</menubar>" "</ui>";

static const gchar *tour2d_pmode_ui =
  "<ui>"
  "	<menubar>"
  "		<menu action='Tour2D'>"
  "			<menuitem action='ShowAxes'/>"
  "			<menuitem action='ShowAxesLabels'/>"
  "			<menuitem action='ShowAxesVals'/>"
  "			<separator/>"
  "			<menuitem action='FadeVariables2D'/>"
  "			<menuitem action='SelectAllVariables2D'/>"
  "		</menu>" "	</menubar>" "</ui>";


static const gchar *cotour_pmode_ui =
  "<ui>"
  "	<menubar>"
  "		<menu action='CorrTour'>"
  "			<menuitem action='ShowAxes'/>"
  "			<separator/>"
  "			<menuitem action='FadeVariablesCo'/>"
  "		</menu>" "	</menubar>" "</ui>";


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
  "			<menuitem action='BrushOn'/>" "		</menu>" "	</menubar>" "</ui>";

static const gchar *scale_imode_ui =
  "<ui>"
  "	<menubar>"
  "		<menu action='Scale'>"
  "			<menuitem action='ResetPan'/>"
  "			<menuitem action='ResetZoom'/>"
  "			<menuitem action='UpdateContinuously'/>"
  "		</menu>" "	</menubar>" "</ui>";


gboolean
imode_has_display_menu (InteractionMode imode)
{
  return (imode == SCALE || imode == BRUSH);
}

void
display_mode_menus_update (ProjectionMode pmode_prev,
                           InteractionMode imode_prev, displayd * display,
                           ggobid * gg)
{
  ProjectionMode pmode = display->cpanel.pmode;
  InteractionMode imode = display->cpanel.imode;
  const gchar *ui = NULL;
  GError *error = NULL;

  if (imode != imode_prev) {
    /* Remove any existing imode submenu */
    if (imode_has_display_menu (imode_prev)) {
      gtk_ui_manager_remove_ui (display->menu_manager,
                                display->imode_merge_id);
      /* I don't understand why all these tests are necessary ... dfs */
      /*if (GTK_IS_MENU_ITEM(display->imode_item))
         gtk_menu_item_remove_submenu (GTK_MENU_ITEM (display->imode_item));
         if (GTK_IS_WIDGET(display->imode_item))
         gtk_widget_destroy (display->imode_item);
         display->imode_item = NULL; */
    }

    /* If the new mode has an imode menu, build it */
    if (imode_has_display_menu (imode)) {
      if (imode == BRUSH) {
        ui = brush_imode_ui;
      }
      else if (imode == SCALE) {
        ui = scale_imode_ui;
      }

      if (ui)
        display->imode_merge_id =
          gtk_ui_manager_add_ui_from_string (display->menu_manager, ui, -1,
                                             &error);
      if (ui == NULL || error) {
        g_message ("Failed to load display imode ui!\n");
        g_error_free (error);
      }

    }
  }

  /* I'm not completely certain I have the correct test here */
  if (pmode != pmode_prev && imode == DEFAULT_IMODE) {

    /* Remove any existing pmode submenu */
    if (pmode_has_display_menu (pmode_prev) && display->pmode_merge_id) {
      gtk_ui_manager_remove_ui (display->menu_manager,
                                display->pmode_merge_id);
      /*gtk_menu_item_remove_submenu (GTK_MENU_ITEM (display->pmode_item));
         gtk_widget_destroy (display->pmode_item);
         display->pmode_item = NULL; */
    }

    /* Do we need to be in the default imode for this?  I don't see
       why */

    /* If the new mode has a pmode menu, build it */
    if (pmode_has_display_menu (pmode)) {
      if (pmode == TOUR1D) {
        ui = tour1d_pmode_ui;
      }
      else if (pmode == TOUR2D) {
        ui = tour2d_pmode_ui;
      }
      else if (pmode == COTOUR) {
        ui = cotour_pmode_ui;
      }

      display->pmode_merge_id =
        gtk_ui_manager_add_ui_from_string (display->menu_manager, ui, -1,
                                           &error);
      if (error) {
        g_message ("Failed to load display pmode ui!\n");
        g_error_free (error);
      }


    }


  }

}
