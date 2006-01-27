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

static const gchar *mode_ui_str =
"<ui>"
"	<menubar>"
"		<menu action='PMode'>"
"			<menuitem action='1D Plot'/>"
"			<menuitem action='XY Plot'/>"
"			<menuitem action='1D Tour'/>"
"			<menuitem action='Rotation'/>"
"			<menuitem action='2D Tour'/>"
"			<menuitem action='2x1D Tour'/>"
"		</menu>"
"		<menu action='IMode'>"
"			<menuitem action='DefaultIMode'/>"
"			<separator/>"
"			<menuitem action='Scale'/>"
"			<menuitem action='Brush'/>"
"			<menuitem action='Identify'/>"
"			<menuitem action='Edit edges'/>"
"			<menuitem action='Move points'/>"
"		</menu>"
"	</menubar>"
"</ui>";

const gchar *
scatterplot_mode_ui_get(displayd *display)
{
	return(mode_ui_str);
}
#if 0
GtkWidget *
scatterplot_pmode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
			    ggobid *gg, gboolean useIds)
{
  /* Projections */
  GtkWidget *pmode_menu, *item;
  gboolean radiop = sessionOptions->useRadioMenuItems;

  pmode_menu = gtk_menu_new ();

  item = CreateMenuItemWithCheck (pmode_menu, "1D Plot",
    "^d", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (P1PLOT) : gg, gg,
    gg->pmodeRadioGroup, radiop);
  if (radiop && gg->pmode == P1PLOT)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (pmode_menu, "XYPlot",
    "^x", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (XYPLOT) : gg, gg,
    gg->pmodeRadioGroup, radiop);
  if (radiop && gg->pmode == XYPLOT)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (pmode_menu, "1D Tour",
    "^t", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR1D) : gg, gg,
    gg->pmodeRadioGroup, radiop);
  if (radiop && gg->pmode == TOUR1D)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (pmode_menu, "Rotation",
    "^r", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR2D3) : gg, gg,
    gg->pmodeRadioGroup, radiop);
  if (radiop && gg->pmode == TOUR2D3)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (pmode_menu, "2D Tour",
    "^g", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (TOUR2D) : gg, gg,
    gg->pmodeRadioGroup, radiop);
  if (radiop && gg->pmode == TOUR2D)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (pmode_menu, "2x1D Tour",
    "^c", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER  (COTOUR) : gg, gg,
    gg->pmodeRadioGroup, radiop);
  if (radiop && gg->pmode == COTOUR)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

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
  gboolean radiop = sessionOptions->useRadioMenuItems;

  imode_menu = gtk_menu_new ();

  item = CreateMenuItemWithCheck (imode_menu,
    (gg->pmode != -1) ? (gchar *)GGOBI(getPModeName)(gg->pmode) : "(Default)",
    "^h", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (DEFAULT_IMODE) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == DEFAULT_IMODE)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  /* Add a separator? */
  CreateMenuItem (imode_menu, "",
    "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuItemWithCheck (imode_menu, "Scale",
    "^s", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (SCALE) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == SCALE)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (imode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == BRUSH)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (imode_menu, "Identify",
    "^i", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (IDENT) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == IDENT)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (imode_menu, "Edit edges",
    "^e", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (EDGEED) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == EDGEED)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  item = CreateMenuItemWithCheck (imode_menu, "Move points",
    "^m", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == MOVEPTS)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  gtk_widget_show (imode_menu);
  return (imode_menu);
}
#endif

/*--------------------------------------------------------------------*/
/*                   Setting the display menubar                      */
/*--------------------------------------------------------------------*/

static void
edge_options_cb(GtkWidget *w, gpointer opt)
{
	displayd *dsp = g_object_get_data(G_OBJECT(w), "display");
	set_display_option(GTK_CHECK_MENU_ITEM(w)->active, GPOINTER_TO_INT(opt), dsp);
}

/*
 * This handles the initialization of the edge menu item and menu,
 * and it should also be called whenever the number of edge sets
 * might have changed:  when an edge set is added or removed, and
 * when a datad is added or removed.
*/
void
scatterplot_display_edge_menu_update (displayd *display,
                                      GtkAccelGroup *accel_group, ggobid *gg)
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
        display->edge_item = gtk_menu_item_new_with_mnemonic("_Edges");
        gtk_menu_shell_insert(GTK_MENU_SHELL(display->menubar), display->edge_item, 1);
		//submenu_insert (display->edge_item, display->menubar, 1);
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
        G_CALLBACK (edgeset_add_cb), onlye, gg);
      g_object_set_data(G_OBJECT (item),
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
            G_CALLBACK (edgeset_add_cb), e, gg);
          g_object_set_data(G_OBJECT (item),
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
      G_CALLBACK(edge_options_cb), GINT_TO_POINTER (DOPT_EDGES_U),
      display->options.edges_undirected_show_p, gg);
    gtk_widget_set_name (item, "DISPLAYMENU:edges_u");
    g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

    item = CreateMenuCheck (display->edge_menu,
      "Show directed edges (edges and 'arrowheads')",
      G_CALLBACK(edge_options_cb), GINT_TO_POINTER (DOPT_EDGES_D),
      display->options.edges_directed_show_p, gg);
    gtk_widget_set_name (item, "DISPLAYMENU:edges_d");
    g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

    item = CreateMenuCheck (display->edge_menu,
      "Show 'arrowheads' only",
      G_CALLBACK(edge_options_cb), GINT_TO_POINTER (DOPT_EDGES_A),
      display->options.edges_arrowheads_show_p, gg);
    gtk_widget_set_name (item, "DISPLAYMENU:edges_a");
    g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

    gtk_menu_item_set_submenu (GTK_MENU_ITEM (display->edge_item),
      display->edge_menu);
	 
	gtk_widget_show_all(display->edge_item);
  }
}

