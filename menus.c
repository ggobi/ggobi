/*-- menus.c: menus in the main menubar that change with the mode: --*/
/*--          Options, Reset, and I/O menus for all modes --*/

/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * ..._item is the widget you see in the main menubar
 * ..._menu is the submenu attached to ..._item
*/

/*--------------------------------------------------------------------*/
/*                   Plot1D: Options menu                             */
/*--------------------------------------------------------------------*/

void
p1dplot_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                     XYPlot: Options menu                           */
/*--------------------------------------------------------------------*/

void
xyplot_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                   Tour 1D: I/O and Options menus                   */
/*--------------------------------------------------------------------*/

void
tour1d_menus_make (ggobid *gg) {
  /*GtkWidget *item;*/
  extern void varcircles_layout_cb (GtkCheckMenuItem *w, guint action);
  extern void tour1d_fade_vars_cb (GtkCheckMenuItem *w, guint action);
  void tour1d_io_cb (GtkWidget *w, gpointer *cbd);

  /*-- I/O menu --*/
  /*  gg->menus.io_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Save coefficients");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour1d_io_cb),
                      (gpointer) "write_coeffs");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  item = gtk_menu_item_new_with_label ("Save history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour1d_io_cb),
                      (gpointer) "write_history");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  item = gtk_menu_item_new_with_label ("Read history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour1d_io_cb),
                      (gpointer) "read_history");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  gtk_widget_show_all (gg->menus.io_menu);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.io_item),
  gg->menus.io_menu); *//* di hasn't filled this in yet. */

  /*-- Options menu --*/ 
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (gg->menus.options_menu, "Lay out variable circles by row",
    GTK_SIGNAL_FUNC (varcircles_layout_cb), NULL,
    gg->varpanel_ui.layoutByRow, gg);

  CreateMenuCheck (gg->menus.options_menu, "Fade variables on de-selection",
    GTK_SIGNAL_FUNC (tour1d_fade_vars_cb), NULL,
    gg->tour1d.fade_vars, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                   Tour 2D: I/O and Options menus                   */
/*--------------------------------------------------------------------*/

void
tour2d_menus_make (ggobid *gg)
{
  /*GtkWidget *item;*/
  extern void varcircles_layout_cb (GtkCheckMenuItem *w, guint action);
  extern void tour2d_fade_vars_cb (GtkCheckMenuItem *w, guint action);
  void tour2d_io_cb (GtkWidget *w, gpointer *cbd);

  /*-- I/O menu --*/
  /*  gg->menus.io_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Save coefficients");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour2d_io_cb),
                      (gpointer) "write_coeffs");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  item = gtk_menu_item_new_with_label ("Save history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour2d_io_cb),
                      (gpointer) "write_history");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  item = gtk_menu_item_new_with_label ("Read history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tour2d_io_cb),
                      (gpointer) "read_history");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  gtk_widget_show_all (gg->menus.io_menu);*//* di hasn't filled in
//these routines yet */

  /*  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.io_item),
      gg->menus.io_menu); */

  /*-- Options menu --*/
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (gg->menus.options_menu, "Lay out variable circles by row",
    GTK_SIGNAL_FUNC (varcircles_layout_cb), NULL,
    gg->varpanel_ui.layoutByRow, gg);

  CreateMenuCheck (gg->menus.options_menu, "Fade variables on de-selection",
    GTK_SIGNAL_FUNC (tour2d_fade_vars_cb), NULL,
    gg->tour2d.fade_vars, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                 Corr Tour: I/O and Options menus                   */
/*--------------------------------------------------------------------*/

void
tourcorr_menus_make (ggobid *gg)
{
  /*GtkWidget *item;*/
  extern void varcircles_layout_cb (GtkCheckMenuItem *w, guint action);
  extern void tourcorr_fade_vars_cb (GtkCheckMenuItem *w, guint action);
  void tourcorr_io_cb (GtkWidget *w, gpointer *cbd);

  /*-- I/O menu --*/
  /*  gg->menus.io_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Save coefficients");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tourcorr_io_cb),
                      (gpointer) "write_coeffs");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  item = gtk_menu_item_new_with_label ("Save history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tourcorr_io_cb),
                      (gpointer) "write_history");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  item = gtk_menu_item_new_with_label ("Read history");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (tourcorr_io_cb),
                      (gpointer) "read_history");
  gtk_menu_append (GTK_MENU (gg->menus.io_menu), item);

  gtk_widget_show_all (gg->menus.io_menu);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.io_item),
  gg->menus.io_menu); *//* di hasn't filled this in yet */

  /*-- Options menu --*/
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (gg->menus.options_menu, "Lay out variable circles by row",
    GTK_SIGNAL_FUNC (varcircles_layout_cb), NULL,
    gg->varpanel_ui.layoutByRow, gg);

  CreateMenuCheck (gg->menus.options_menu, "Fade variables on de-selection",
    GTK_SIGNAL_FUNC (tourcorr_fade_vars_cb), NULL,
    gg->tourcorr.fade_vars, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                   Scaling: Reset and Options menus                 */
/*--------------------------------------------------------------------*/

void
scale_menus_make (ggobid *gg) {
  GtkWidget *item;
  void pan_reset_cb (GtkWidget *w, ggobid *gg);
  void zoom_reset_cb (GtkWidget *w, ggobid *gg);

  /*-- Reset menu --*/
  gg->menus.reset_menu = gtk_menu_new ();

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

  gtk_widget_show_all (gg->menus.reset_menu);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.reset_item),
    gg->menus.reset_menu); 

  /*-- Options menu --*/
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*               Brushing: Reset and Options menus                    */
/*--------------------------------------------------------------------*/

void
brush_menus_make (ggobid *gg)
{
  GtkWidget *item;
  void brush_reset_cb (GtkWidget *w, gpointer cbd);
  void brush_update_set_cb (GtkCheckMenuItem *w, guint action);

  /*-- Reset menu --*/
  gg->menus.reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Show all points");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (0));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

/*
  item = gtk_menu_item_new_with_label ("Reset point colors");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (1));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset glyphs");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (2));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);
*/

  item = gtk_menu_item_new_with_label ("Show all edges");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (3));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

/*
  item = gtk_menu_item_new_with_label ("Reset edgecolors");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER(4));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);
*/

  item = gtk_menu_item_new_with_label ("Reset brush size");
  GGobi_widget_set (item, gg, true);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (5));
  gtk_menu_append (GTK_MENU (gg->menus.reset_menu), item);

  gtk_widget_show_all (gg->menus.reset_menu);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.reset_item),
    gg->menus.reset_menu);

  /*-- Options menu --*/
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->menus.options_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, NULL);

  CreateMenuCheck (gg->menus.options_menu, "Update brushing continuously",
    GTK_SIGNAL_FUNC (brush_update_set_cb), NULL,
    gg->brush.updateAlways_p, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                   Identify: Options menu                           */
/*--------------------------------------------------------------------*/

void
identify_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                   Movepts: Options menu                            */
/*--------------------------------------------------------------------*/

void
movepts_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}


/*--------------------------------------------------------------------*/
/*                   Edge edit: Options menu                          */
/*--------------------------------------------------------------------*/

void
edgeedit_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);

}

/*--------------------------------------------------------------------*/
/*                   Scatmat: Options menu                            */
/*--------------------------------------------------------------------*/

void
scatmat_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*                   Parcoords: Options menu                          */
/*--------------------------------------------------------------------*/

void
pcplot_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}


/*--------------------------------------------------------------------*/
/*                   Time series: Options menu                        */
/*--------------------------------------------------------------------*/

void
tsplot_menus_make (ggobid *gg)
{
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->viewmode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
}

/*--------------------------------------------------------------------*/
/*               Routines to manage the mode menus                    */
/*--------------------------------------------------------------------*/

gboolean
mode_has_options_menu (gint mode)
{
  /*-- every mode has an options menu --*/
  return (mode == P1PLOT || mode == XYPLOT || mode == SCALE  ||
          mode == BRUSH  || mode == TOUR1D || mode == TOUR2D ||
          mode == COTOUR || mode == IDENT  ||
#ifdef EDIT_EDGES_IMPLEMENTED
          mode == EDGEED ||
#endif
          mode == MOVEPTS ||
          mode == SCATMAT || mode == PCPLOT || mode == TSPLOT);
}

gboolean
mode_has_reset_menu (gint mode)
{
  return (mode == SCALE  || mode == BRUSH);
}

gboolean
mode_has_io_menu (gint mode)
{
  return (mode == TOUR1D || mode == TOUR2D || mode == COTOUR);
}


/*-- make the menu items once, and then show/hide them as necessary --*/
void
viewmode_submenus_initialize (PipelineMode mode, ggobid *gg)
{
  gg->menus.options_item = NULL;
  gg->menus.reset_item = NULL;
  gg->menus.io_item = NULL;
}

void
viewmode_submenus_update (PipelineMode prev_mode, ggobid *gg)
{
  PipelineMode mode = viewmode_get (gg);

  /*-- remove any previous submenus --*/
  if (mode_has_options_menu (prev_mode)) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (gg->menus.options_item));
    /*-- destroy menu items if called for --*/
    if (!mode_has_options_menu (mode)) {
      if (gg->menus.options_item != NULL) {
        gtk_widget_destroy (gg->menus.options_item);
        gg->menus.options_item = NULL;
      }
    }
  } else {
    /*-- create and insert menu items if called for --*/
    if (mode_has_options_menu (mode)) {
      gg->menus.options_item = submenu_make ("_Options", 'O',
        gg->main_accel_group);
      submenu_insert (gg->menus.options_item,
        gg->main_menubar, 4);
    }
  }

  /*-- remove any previous submenus --*/
  /*  if (mode_has_io_menu (prev_mode)) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (gg->menus.io_item));
    *-- destroy menu items if called for --*
    if (!mode_has_io_menu (mode)) {
      if (gg->menus.io_item != NULL) {
        gtk_widget_destroy (gg->menus.io_item);
        gg->menus.io_item = NULL;
      }
      }
  } else {
    *-- create and insert menu items if called for --*
    if (mode_has_io_menu (mode)) {
      gg->menus.io_item = submenu_make ("_I/O", 'I',
        gg->main_accel_group);
      submenu_insert (gg->menus.io_item,
      gg->main_menubar, 5);
    }
    }*//* di hasn't filled these in yet*/

  if (mode_has_reset_menu (prev_mode)) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (gg->menus.reset_item));
    if (!mode_has_reset_menu (mode)) {
      if (gg->menus.reset_item != NULL) {
        gtk_widget_destroy (gg->menus.reset_item);
        gg->menus.reset_item = NULL;
      }
    }
  } else {
    if (mode_has_reset_menu (mode)) {
      gg->menus.reset_item = submenu_make ("_Reset", 'R',
        gg->main_accel_group);
      submenu_insert (gg->menus.reset_item,
        gg->main_menubar, 5);
    }
  }

  /*-- add any new submenus --*/
  switch (mode) {
    case PCPLOT:
      pcplot_menus_make (gg);
    break;
    case TSPLOT:
      tsplot_menus_make (gg);
    break;
    case SCATMAT:
      scatmat_menus_make (gg);
    break;

    case P1PLOT:
      p1dplot_menus_make (gg);
    break;
    case XYPLOT:
      xyplot_menus_make (gg);
    break;
#ifdef EDIT_EDGES_IMPLEMENTED
    case EDGEED:
      edgeedit_menus_make (gg);
    break;
#endif
    case MOVEPTS:
      movepts_menus_make (gg);
    break;

#ifdef ROTATION_IMPLEMENTED
    case ROTATE:
    break;
#endif

    case TOUR1D:
      tour1d_menus_make (gg);
    break;

    case TOUR2D:
      tour2d_menus_make (gg);
    break;

    case COTOUR:
      tourcorr_menus_make (gg);
    break;

    case SCALE :
      scale_menus_make (gg);
    break;

    case BRUSH :
      brush_menus_make (gg);
    break;

    case IDENT:
      identify_menus_make (gg);
    break;

    case ROTATE:
    case EDGEED:
    case NULLMODE:
    case NMODES:  /*-- why is this part of the enum? --*/
    break;
  }
}
