/* scatterplot_ui.c */
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

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

GtkWidget *
scatterplot_pmode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
			    ggobid *gg, gboolean useIds)
{
  GtkWidget *pmode_menu;
  /* Projections */

  pmode_menu = gtk_menu_new ();

  CreateMenuItem (pmode_menu, "1D Plot",
    "^d", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (P1PLOT) : gg, gg);
  CreateMenuItem (pmode_menu, "XYPlot",
    "^x", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (XYPLOT) : gg, gg);

  CreateMenuItem (pmode_menu, "1D Tour",
    "^t", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR1D) : gg, gg);
  CreateMenuItem (pmode_menu, "Rotation",
    "^r", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR2D3) : gg, gg);
  CreateMenuItem (pmode_menu, "2D Tour",
    "^g", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR2D) : gg, gg);
  CreateMenuItem (pmode_menu, "2x1D Tour",
    "^c", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (COTOUR) : gg, gg);

  gtk_widget_show (pmode_menu);
  return (pmode_menu);
}

GtkWidget *
scatterplot_imode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
			    ggobid *gg, gboolean useIds)
{
  /* Interaction modes for scatterplot.  The first one is "Default"
     which is generally going to mean we want to see the control panel
     corresponding to the projection. */

  GtkWidget *imode_menu, *item;

  imode_menu = gtk_menu_new ();

  CreateMenuItemWithCheck (imode_menu,
    (gg->pmode != -1) ? (gchar *)GGOBI(getPModeName)(gg->pmode) : "(Default)",
			   /*
    (gg->pmode != -1) ?
       g_strdup_printf("^%s", GGOBI(getPModeKey)(gg->pmode)) : "^x", 
			   */
			   "^h",
    "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (DEFAULT_IMODE) : gg, gg, 
    gg->imodeRadioGroup,
    sessionOptions->useRadioMenuItems);

  /* Add a separator? */
  CreateMenuItem (imode_menu, "",
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItemWithCheck (imode_menu, "Scale",
    "^s", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (SCALE) : gg, gg, 
    gg->imodeRadioGroup,
    sessionOptions->useRadioMenuItems);
  CreateMenuItemWithCheck (imode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg, 
    gg->imodeRadioGroup,
    sessionOptions->useRadioMenuItems);
  CreateMenuItemWithCheck (imode_menu, "Identify",
    "^i", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (IDENT) : gg, gg, 
    gg->imodeRadioGroup,
    sessionOptions->useRadioMenuItems);
  CreateMenuItemWithCheck (imode_menu, "Edit edges",
    "^e", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (EDGEED) : gg, gg, 
    gg->imodeRadioGroup,
    sessionOptions->useRadioMenuItems);
  CreateMenuItemWithCheck (imode_menu, "Move Points",
    "^m", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg, 
    gg->imodeRadioGroup,
    sessionOptions->useRadioMenuItems);

  /* A bit more thought is required to make this work with the new
     menus, and to update properly -- we don't need an "Off" button,
     but we do need a system to keep the selection current and these
     menus change a lot. dfs */

  if (sessionOptions->useRadioMenuItems) {
     item = CreateMenuItemWithCheck (imode_menu, "Off",
       "^o", "", NULL, accel_group, func,
       useIds ? GINT_TO_POINTER (DEFAULT_IMODE) : gg, gg,
       gg->imodeRadioGroup,
       sessionOptions->useRadioMenuItems);

     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);
  }


  gtk_widget_show (imode_menu);
  return (imode_menu);
}


/*--------------------------------------------------------------------*/
/*                   Setting the display menubar                      */
/*--------------------------------------------------------------------*/

/*
 * This handles the initialization of the edge menu item and menu,
 * and it should also be called whenever the number of edge sets
 * might have changed:  when an edge set is added or removed, and
 * when a datad is added or removed.
*/
void
scatterplot_display_edge_menu_update (displayd *display,
                                      GtkAccelGroup *accel_group,
                                      GtkSignalFunc func, ggobid *gg)
{
  datad *d = display->d;  /*-- this dataset --*/
  gint nd = g_slist_length (gg->d);
  datad *e, *onlye;
  gint k, ne = 0;
  GtkWidget *item;
  GtkWidget *submenu, *anchor;
  gchar *lbl;

  /*-- If this datad has ids, find the number of other datad's with
       edges --*/
/*
  If I wanted to verify that they were compatible, how would I
  do that?  Could I try to resolve them?  Yes, with
    resolveEdgePoints (e, d)
  If it returns NULL, there's no match.
  I just have to decide whether I want to expend the computing time.
*/
  if (d->rowIds) {
    endpointsd *endpoints;
    for (k=0; k<nd; k++) { 
      e = (datad*) g_slist_nth_data (gg->d, k);
      if (/* e != d && */ e->edge.n > 0) {
        endpoints = resolveEdgePoints(e, d);
        if (endpoints != NULL) {
          ne++;
          onlye = e;  /* meaningful if there's only one */
        }
        /* I don't know whether I need to unresolveEdgePoints afterwards */
      }
    }
  }

  /*-- remove any existing submenu --*/
  if (display->edge_item != NULL && display->edge_menu != NULL) {
    gtk_menu_item_remove_submenu (GTK_MENU_ITEM (display->edge_item));
    display->edge_menu = NULL;
    if (ne < 1) {
      /*-- destroy menu item if there are no edge sets --*/
      gtk_widget_destroy (display->edge_item);
      display->edge_item = NULL;
    }
  } else {
    /*-- create the edge menu item if there is at least one edge set --*/
    if (ne > 0) {
      if (display->edge_item == NULL) {
        display->edge_item = submenu_make ("_Edges", 'E',
          gg->main_accel_group);
        submenu_insert (display->edge_item, display->menubar, 1);
      }
    }
  }

  /*-- then build the new menu if appropriate --*/
  if (ne) {
    /*-- build the menu --*/
    display->edge_menu = gtk_menu_new ();

    /*
       When there's only one edge set, indicate that on the menu
       with a single menu item naming the edge set.  Let it behave
       like the other menu items, too, turning on undirected
       edges.  Selecting an edge set is required.
    */
    if (ne == 1) {
      lbl = g_strdup_printf ("Select edge set (%s)", onlye->name);
      item = CreateMenuItem (display->edge_menu, lbl,
        NULL, NULL, NULL, gg->main_accel_group,
        GTK_SIGNAL_FUNC (edgeset_add_cb), onlye, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "display", GINT_TO_POINTER (display));
      g_free (lbl);
    }

    /*-- if there's only one edge set, there's no need for this menu --*/
    else if (ne > 1) {  /*-- add cascading menu --*/

      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (display->edge_menu,
       "Select edge set",
        NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      for (k=0; k<nd; k++) { 
        e = (datad *) g_slist_nth_data (gg->d, k);
        /* if (e == d) continue; */
        if (e->edge.n > 0) {
          lbl = datasetName (e, gg);
          item = CreateMenuItem (submenu, lbl,
            NULL, NULL, NULL, gg->main_accel_group,
            GTK_SIGNAL_FUNC (edgeset_add_cb), e, gg);
          gtk_object_set_data (GTK_OBJECT (item),
            "display", GINT_TO_POINTER (display));
          g_free (lbl);
        }
      }

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);

      /* Add a separator */
      CreateMenuItem (display->edge_menu, NULL, "", "",
                      NULL, NULL, NULL, NULL, gg);
    } /*-- end of adding cascading menu --*/

    /*
     * The edge options are handled like other display options,
     * and their callback function is display_options_cb.
     * I may want to change that, but leave it for now.
    */
    item = CreateMenuCheck (display->edge_menu,
      "Show undirected edges",
      GTK_SIGNAL_FUNC(display_options_cb), GINT_TO_POINTER (DOPT_EDGES_U),
      display->options.edges_undirected_show_p, gg);
    gtk_widget_set_name (item, "DISPLAYMENU:edges_u");
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

    item = CreateMenuCheck (display->edge_menu,
      "Show directed edges (edges and 'arrowheads')",
      GTK_SIGNAL_FUNC(display_options_cb), GINT_TO_POINTER (DOPT_EDGES_D),
      display->options.edges_directed_show_p, gg);
    gtk_widget_set_name (item, "DISPLAYMENU:edges_d");
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

    item = CreateMenuCheck (display->edge_menu,
      "Show 'arrowheads' only",
      GTK_SIGNAL_FUNC(display_options_cb), GINT_TO_POINTER (DOPT_EDGES_A),
      display->options.edges_arrowheads_show_p, gg);
    gtk_widget_set_name (item, "DISPLAYMENU:edges_a");
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (display->edge_item),
      display->edge_menu);
  }
}

void
scatterplot_display_menus_make (displayd *display,
                                GtkAccelGroup *accel_group,
                                GtkSignalFunc func, ggobid *gg)
{
  GtkWidget *topmenu, *options_menu;
  GtkWidget *item;

  display->edge_item = NULL;
  display->edge_menu = NULL;
  scatterplot_display_edge_menu_update (display, accel_group, func, gg);

  /*-- Options menu --*/
  topmenu = submenu_make ("_Options", 'O', accel_group);
  gtk_widget_set_name (topmenu, "DISPLAY:options_topmenu");
  /*-- add a tooltip --*/
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), topmenu,
    "Options menu for this display", NULL);

  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  /*-- Add a separator --*/
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuCheck (options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), display->options.axes_show_p, gg);
  gtk_widget_set_name (item, "DISPLAY:show_axes");
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  item = CreateMenuCheck (options_menu, "Show 2D tour axes as text",
    func, GINT_TO_POINTER (DOPT_AXESLAB), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  item = CreateMenuCheck (options_menu, "Show 2D tour proj vals",
    func, GINT_TO_POINTER (DOPT_AXESVALS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (topmenu), options_menu);
  submenu_append (topmenu, display->menubar);
  gtk_widget_show (topmenu);
}

