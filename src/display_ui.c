/*-- display_ui.c --*/
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

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "plugin.h"

void buildExtendedDisplayMenu (ggobid * gg, int nd, GGobiStage * d0);

void
display_set_position (windowDisplayd * display, ggobid * gg)
{
  gint x, y, width, height;
  gint posx, posy;

  /*-- get the size and position of the gg->main_window) --*/
  gdk_window_get_root_origin (gg->main_window->window, &x, &y);
  gdk_window_get_size (gg->main_window->window, &width, &height);

  gtk_widget_realize (display->window);
  if (x == 0 && y == 0) {
                       /*-- can't get any info for the first display --*/
    posx = gdk_screen_width () / 4;
    posy = gdk_screen_height () / 4;
  }
  else {
    posx = x + (3 * width) / 4;
    posy = y + (3 * height) / 4;
  }
  gtk_window_move (GTK_WINDOW (display->window), posx, posy);
}

void
display_menu_build (ggobid * gg)
{
  gint nd;
  GGobiStage *d0;
  GtkWidget *item;

  if (gg == NULL || gg->d == NULL)
    return;

  nd = ndatad_with_vars_get (gg);

  d0 = (GGobiStage *) gg->d->data;
  if (gg->display_menu != NULL)
    gtk_widget_destroy (gg->display_menu);

  if (nd > 0) {
    gg->display_menu = gtk_menu_new ();

    if (g_slist_length (ExtendedDisplayTypes)) {
      buildExtendedDisplayMenu (gg, nd, d0);
    }
  }

  item = gtk_tearoff_menu_item_new ();
  gtk_menu_shell_prepend (GTK_MENU_SHELL (gg->display_menu), item);

  /* Experiment: move the DisplayTree to the Display menu -- dfs */
  /* Add a separator before the mode-specific items */
  CreateMenuItem (gg->display_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = gtk_menu_item_new_with_label ("Show Display Tree");
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (show_display_tree_cb), (gpointer) gg);
  gtk_menu_shell_append (GTK_MENU_SHELL (gg->display_menu), item);

  /*-- these two lines replace gtk_menu_popup --*/
  if (nd) {
    gtk_widget_show_all (gg->display_menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM
                               (gtk_ui_manager_get_widget
                                (gg->main_menu_manager, "/menubar/Display")),
                               gg->display_menu);
  }
}

void
display_menu_init (ggobid * gg)
{                               /*
                                   gg->display_menu_item = submenu_make ("_Display", 'D',
                                   gg->main_accel_group);

                                   gtk_widget_show (gg->display_menu_item);

                                   submenu_insert (gg->display_menu_item, gg->main_menubar, 1);
                                 */
}


typedef struct
{
  GGobiExtendedDisplayClass *klass;
  GGobiStage *d;
} ExtendedDisplayCreateData;

static void
extended_display_open_cb (GtkWidget * w, ExtendedDisplayCreateData * data)
{
  ggobid *gg = data->d->gg;
  displayd *dpy;

  if (data->d->n_rows== 0)
    return;

  splot_set_current (gg->current_splot, off, gg);
  if (data->klass->create) {
    dpy = data->klass->create (false, NULL, data->d, gg);
  }
  else if (data->klass->createWithVars) {
    gint *selected_vars, nselected_vars = 0;
    nselected_vars = selected_cols_get (&selected_vars,data->d, gg);
    dpy =
      data->klass->createWithVars (false, nselected_vars, selected_vars,
                                   data->d, gg);
    g_free (selected_vars);
  }
  else {
    /* How to get the name of the class from the class! GTK_OBJECT_CLASS(gtk_type_name(data->klass)->type) 
       Close.. */
    g_printerr
      ("Real problems! An extended display (%s) without a create routine!\n",
       g_type_name (G_TYPE_FROM_CLASS (data->klass)));
    return;
  }

  if (!dpy) {
    g_printerr ("Failed to create display of type %s\n",
                data->klass->titleLabel);
    return;
  }


  display_add (dpy, gg);
  varpanel_refresh (dpy, gg);
}

void
buildExtendedDisplayMenu (ggobid * gg, gint nd, GGobiStage * d0)
{
  gchar label[200], *lbl;
  GGobiExtendedDisplayClass *klass;
  GSList *el = ExtendedDisplayTypes;
  const gchar *desc;
  GtkWidget *item, *submenu, *anchor;
  gint k;
  ExtendedDisplayCreateData *cbdata;
  while (el) {
    klass = GGOBI_EXTENDED_DISPLAY_CLASS (el->data);
    desc = klass->titleLabel;
    sprintf (label, "New %s", desc);

    if (nd == 1) {
      cbdata = (ExtendedDisplayCreateData *)
        g_malloc (sizeof (ExtendedDisplayCreateData));
      cbdata->d = d0;
      cbdata->klass = klass;

      item = CreateMenuItem (gg->display_menu, label,
                             NULL, NULL, gg->main_menubar,
                             gg->main_accel_group,
                             G_CALLBACK (extended_display_open_cb),
                             (gpointer) cbdata, gg);
      g_object_set_data (G_OBJECT (item), "missing_p", GINT_TO_POINTER (0));
    }
    else {
      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, label,
                               NULL, NULL, gg->main_menubar, NULL, NULL, NULL,
                               NULL);

      k = 0;
      for (k = 0; k < g_slist_length (gg->d); k++) {
        GGobiStage *d = (GGobiStage *) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (ggobi_stage_has_vars(d)) {
          lbl = d->name;
          cbdata = (ExtendedDisplayCreateData *)
            g_malloc (sizeof (ExtendedDisplayCreateData));
          cbdata->d = d;
          cbdata->klass = klass;
          item = CreateMenuItem (submenu, lbl,
                                 NULL, NULL, gg->display_menu,
                                 gg->main_accel_group,
                                 G_CALLBACK (extended_display_open_cb),
                                 cbdata, gg);

          g_object_set_data (G_OBJECT (item),
                             "displaytype", (gpointer) klass);
          g_object_set_data (G_OBJECT (item),
                             "missing_p", GINT_TO_POINTER (0));
          g_free (lbl);
        }
      }
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    }

    el = el->next;
  }
}

static void
action_close_cb (GtkAction * action, displayd * display)
{
  display_close (display);
}
static void
action_exclude_shadowed_points_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_EXCLUDE_SHADOW_POINTS);
}
static void
action_include_shadowed_points_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_INCLUDE_SHADOW_POINTS);
}
static void
action_unshadow_all_points_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_UNSHADOW_POINTS);
}
static void
action_exclude_shadowed_edges_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_EXCLUDE_SHADOW_EDGES);
}
static void
action_include_shadowed_edges_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_INCLUDE_SHADOW_EDGES);
}
static void
action_unshadow_all_edges_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_UNSHADOW_EDGES);
}
static void
action_reset_brush_cb (GtkAction * action, displayd * display)
{
  brush_reset (display, RESET_INIT_BRUSH);
}
static void
action_reset_pan_cb (GtkAction * action, displayd * display)
{
  scale_pan_reset (display);
}
static void
action_reset_zoom_cb (GtkAction * action, displayd * display)
{
  scale_zoom_reset (display);
}
static void
action_toggle_scale_update_cb (GtkToggleAction * action, displayd * display)
{
  scale_update_set (gtk_toggle_action_get_active (action), display,
                    display->ggobi);
}
static void
action_select_all_1d_cb (GtkAction * action, displayd * display)
{
  tour1d_all_vars (display);
}
static void
action_select_all_2d_cb (GtkAction * action, displayd * display)
{
  tour2d_all_vars (display);
}
static void
action_toggle_axes_cb (GtkToggleAction * action, displayd * display)
{
  set_display_option (gtk_toggle_action_get_active (action), DOPT_AXES,
                      display);
}
static void
action_toggle_axes_labels_cb (GtkToggleAction * action, displayd * display)
{
  set_display_option (gtk_toggle_action_get_active (action), DOPT_AXESLAB,
                      display);
}
static void
action_toggle_axes_vals_cb (GtkToggleAction * action, displayd * display)
{
  set_display_option (gtk_toggle_action_get_active (action), DOPT_AXESVALS,
                      display);
}
static void
action_toggle_lines_cb (GtkToggleAction * action, displayd * display)
{
  set_display_option (gtk_toggle_action_get_active (action), DOPT_WHISKERS,
                      display);
}
static void
action_toggle_points_cb (GtkToggleAction * action, displayd * display)
{
  set_display_option (gtk_toggle_action_get_active (action), DOPT_POINTS,
                      display);
}
static void
action_toggle_fade_vars_1d_cb (GtkToggleAction * action, displayd * display)
{
  tour1d_fade_vars (gtk_toggle_action_get_active (action), display->ggobi);
}
static void
action_toggle_fade_vars_2d_cb (GtkToggleAction * action, displayd * display)
{
  tour2d_fade_vars (gtk_toggle_action_get_active (action), display->ggobi);
}
static void
action_toggle_fade_vars_co_cb (GtkToggleAction * action, displayd * display)
{
  tourcorr_fade_vars (gtk_toggle_action_get_active (action), display->ggobi);
}
static void
action_toggle_brush_update_cb (GtkToggleAction * action, displayd * display)
{
  brush_update_set (gtk_toggle_action_get_active (action), display,
                    display->ggobi);
}
static void
action_toggle_brush_on_cb (GtkToggleAction * action, displayd * display)
{
  brush_on_set (gtk_toggle_action_get_active (action), display,
                display->ggobi);
}

static GtkActionEntry disp_action_entries[] = {
  {"File", NULL, "_File"},
  {"Close", GTK_STOCK_CLOSE, "_Close", "<control>C", "Close this display",
   G_CALLBACK (action_close_cb)},
  {"Options", NULL, "_Options", NULL, "Options for this display"},
  /* imode brush specific */
  {"Brush", NULL, "_Brush"},
  {"ExcludeShadowedPoints", NULL, "E_xclude shadowed points", "<control>X",
   "Exclude the points that are currently shadowed",
   G_CALLBACK (action_exclude_shadowed_points_cb)
   },
  {"IncludeShadowedPoints", NULL, "_Include shadowed points", "<control>I",
   "Include the points that are currently shadowed",
   G_CALLBACK (action_include_shadowed_points_cb)
   },
  {"UnshadowAllPoints", NULL, "_Unshadow all points", "<control>U",
   "Make all points unshadowed", G_CALLBACK (action_unshadow_all_points_cb)
   },
  {"ExcludeShadowedEdges", NULL, "_Exclude shadowed edges", "<control>E",
   "Exclude the edges that are shadowed",
   G_CALLBACK (action_exclude_shadowed_edges_cb)
   },
  {"IncludeShadowedEdges", NULL, "Include s_hadowed edges", "<control>H",
   "Include the edges that are shadowed",
   G_CALLBACK (action_include_shadowed_edges_cb)
   },
  {"UnshadowAllEdges", NULL, "U_nshadow all edges", "<control>N",
   "Make all edges unshadowed", G_CALLBACK (action_unshadow_all_edges_cb)
   },
  {"ResetBrushSize", NULL, "_Reset brush", "<control>R",
   "Reset the size of the brush", G_CALLBACK (action_reset_brush_cb)
   },
  /* i-mode scale specific */
  {"Scale", NULL, "_Scale"},
  {"ResetPan", NULL, "Reset _pan", "<control>P",
   "Return to initial position", G_CALLBACK (action_reset_pan_cb)
   },
  {"ResetZoom", NULL, "Reset _zoom", "<control>Z",
   "Return to initial zoom", G_CALLBACK (action_reset_zoom_cb)
   },

  /* p-mode specific stuff - should move elsewhere */
  {"Tour1D", NULL, "_Tour1D"},
  {"SelectAllVariables1D", NULL, "_Select all variables", "<control>S",
   "Select all variables for this 1D tour",
   G_CALLBACK (action_select_all_1d_cb)
   },
  {"Tour2D", NULL, "_Tour2D"},
  {"SelectAllVariables2D", NULL, "_Select all variables", "<control>S",
   "Select all variables for this 2D tour",
   G_CALLBACK (action_select_all_2d_cb)
   },
  {"CorrTour", NULL, "_Correlation Tour"}
};

GtkActionGroup *
display_default_actions_create (displayd * display)
{
  GtkToggleActionEntry disp_t_action_entries[] = {
    {"ShowAxes", NULL, "Show _Axes", "<control>A",
     "Toggle visibility of axes on this display",
     G_CALLBACK (action_toggle_axes_cb), display->options.axes_show_p},
    {"ShowLines", NULL, "Show _Lines", "<control>L",
     "Toggle visibility of lines on this display",
     G_CALLBACK (action_toggle_lines_cb), display->options.whiskers_show_p},
    {"ShowPoints", NULL, "Show P_oints", "<control>O",
     "Toggle visibility of points on this display",
     G_CALLBACK (action_toggle_points_cb), display->options.points_show_p},
    {"ShowAxesLabels", NULL, "Show Axes _Labels", "<control>L",
     "Toggle display of the axes labels",
     G_CALLBACK (action_toggle_axes_labels_cb),
     display->options.axes_label_p},
    {"ShowAxesVals", NULL, "Show Projection _Vals", "<control>V",
     "Toggle display of the projection values",
     G_CALLBACK (action_toggle_axes_vals_cb), display->options.axes_values_p},
    {"FadeVariables1D", NULL, "_Fade Variables on Deselection", NULL,
     "Toggle whether variables fade on when de-selected from the 1D tour",
     G_CALLBACK (action_toggle_fade_vars_1d_cb),
     display->ggobi->tour1d.fade_vars},
    {"FadeVariables2D", NULL, "_Fade Variables on Deselection", NULL,
     "Toggle whether variables fade on when de-selected from the 2D tour",
     G_CALLBACK (action_toggle_fade_vars_2d_cb),
     display->ggobi->tour2d.fade_vars},
    {"FadeVariablesCo", NULL, "_Fade Variables on Deselection", NULL,
     "Toggle whether variables fade on when de-selected from the corr tour",
     G_CALLBACK (action_toggle_fade_vars_co_cb), display->ggobi->tourcorr.fade_vars}, /* i-mode specific */
    /* I'm going to make these display-specific since
       they're on the display menu -- dfs */
    {"UpdateBrushContinuously", NULL, "Update Brushing _Continuously", NULL,
     "Toggle whether the brush operates continuously",
     G_CALLBACK (action_toggle_brush_update_cb), display->cpanel.br.updateAlways_p},  /* i-mode specific */
    {"BrushOn", NULL, "Brush _On", NULL,
     "Toggle whether the brush is active",
     G_CALLBACK (action_toggle_brush_on_cb), display->cpanel.br.brush_on_p},
    {"UpdateContinuously", NULL, "Update _Continuously", NULL,
     "Toggle whether panning and zooming operates continuously",
     G_CALLBACK (action_toggle_scale_update_cb),
     display->cpanel.scale.updateAlways_p}
  };

  GtkActionGroup *actions = gtk_action_group_new ("DisplayActions");
  gtk_action_group_add_actions (actions, disp_action_entries,
                                G_N_ELEMENTS (disp_action_entries), display);
  gtk_action_group_add_toggle_actions (actions, disp_t_action_entries,
                                       G_N_ELEMENTS (disp_t_action_entries),
                                       display);
  return (actions);
}

static const gchar *display_default_ui =
  "<ui>"
  "	<menubar>"
  "		<menu action='File'>"
  "			<menuitem action='Close'/>" "		</menu>" "	</menubar>" "</ui>";

GtkUIManager *
display_menu_manager_create (displayd * display)
{
  GError *error = NULL;
  GtkUIManager *manager = gtk_ui_manager_new ();
  GtkActionGroup *disp_actions = display_default_actions_create (display);
  gtk_ui_manager_insert_action_group (manager, disp_actions, 0);
  g_object_unref (G_OBJECT (disp_actions));
  gtk_ui_manager_add_ui_from_string (manager, display_default_ui, -1, &error);
  if (error) {
    g_message ("Could not add default display ui!");
    g_error_free (error);
  }
  return (manager);
}
