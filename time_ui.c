/* time_ui.c */
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
 * Contributing author of time series code:  Nicholas Lewin-Koh
*/


#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/


#ifdef TS_EXTENSIONS_IMPLEMENTED
/* 
 * Toggles between a single plot with all selected series and individual
 * plots of all selected series
 */
static gchar *arrangement_lbl[] = {"Split", "Joint"};
static void arrangement_cb (GtkWidget *w, ggobid *gg)
{
  gint indx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  g_printerr ("cbd: %s\n", arrangement_lbl[indx]);

  if (indx != gg->current_display->cpanel.tsplot_arrangement)
    tsplot_reset_arrangement (gg->current_display, indx, gg);

  gg->current_display->cpanel.tsplot_arrangement = indx;
}
#endif

/* 
 * Five selection modes, Replace, Insert and Append behave as in parcoords 
 * except that when in replace mode if a y variable is already in a plot 
 * nothing happens insted of deleting the variable. Delete will delete y
 * variables from a plot and if there is only 1 variable will remove the plot 
 * from the layout, unless there is only one plot with one variable in which 
 * case nothing happens. Overlay will add a variable to a plot unless the 
 * variable is already in that plot. 
 */ 

#ifdef TS_EXTENSIONS_IMPLEMENTED
static gchar *selection_mode_lbl[] = {"Replace","Insert","Append","Delete","Overlay"};
#else
static gchar *selection_mode_lbl[] = {"Replace","Insert","Append","Delete"};
#endif
static void selection_mode_cb (GtkWidget *w, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->tsplot_selection_mode = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
}

#ifdef TS_EXTENSIONS_IMPLEMENTED
/*
 * "Common" scales all series by 
 * argmax(argmax(var[i]) forall i)-argmin(argmin(var[i] forall i)), 
 * while "Independent" scales each series independently by its min 
 * and max to [0,1].
 */
static gchar *varscale_lbl[] = {"Common", "Independent"};
static void varscale_cb (GtkWidget *w, ggobid *gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX(w));
  g_printerr ("cbd: %s\n", varscale_lbl[indx]);
}
#endif

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

GtkWidget *
cpanel_tsplot_make (ggobid *gg) 
{
  GtkWidget *vb, *lbl, *opt;
  GtkWidget *cpanel;

  cpanel = gtk_vbox_new (false, VBOX_SPACING);

  gtk_container_set_border_width (GTK_CONTAINER (cpanel), 5);

#ifdef TS_EXTENSIONS_IMPLEMENTED
/*
 * arrangement of plots, row or column
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (cpanel), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("_Layout:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), lbl);
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
			"Arrange the time series as single plot or several plots",
			NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, arrangement_lbl, G_N_ELEMENTS(arrangement_lbl),
    arrangement_cb, gg);
#endif

/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (cpanel), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("_Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_widget_set_name (opt, "TSPLOT:sel_mode_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Selecting a variable either replaces the variable in the current plot (swapping if appropriate), inserts a new plot before the current plot, or appends a new plot after the last plot",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  /* dfs: I'm trying to build a menu with 4 menuitems, and the code
   * says it's working, but the resulting menu has only 3 items. */
  populate_combo_box (opt, selection_mode_lbl, G_N_ELEMENTS(selection_mode_lbl),
    G_CALLBACK(selection_mode_cb), gg);

/*
 * Variable scales
*/

#ifdef TS_EXTENSIONS_IMPLEMENTED
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (cpanel), vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("Sc_ales:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
   "Scale variables (and variable groups) on a common scale, or independently",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, varscale_lbl, G_N_ELEMENTS(varscale_lbl),
    varscale_cb, gg);
#endif


  gtk_widget_show_all (cpanel);

  return(cpanel);
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/


static const gchar* mode_ui_str =
"<ui>"
"	<menubar>"
"		<menu action='IMode'>"
"			<menuitem action='DefaultIMode'/>"
"			<separator/>"
"			<menuitem action='Brush'/>"
"			<menuitem action='Identify'/>"
"		</menu>"
"	</menubar>"
"</ui>";

const gchar*
tsplot_mode_ui_get(displayd *display)
{
	return(mode_ui_str);
}

#if 0
/*
  The useIds indicates whether the callback data should be integers
  identifying the menu item or the global gg.
  At present, this is always false.
  See scatmat_mode_menu_make and scatterplot_mode_menu_make.
 */
GtkWidget *
tsplot_imode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func, ggobid *gg, gboolean useIds) 
{
  GtkWidget *imode_menu, *item;
  gboolean radiop = sessionOptions->useRadioMenuItems;

  imode_menu = gtk_menu_new ();

  item = CreateMenuItemWithCheck (imode_menu, "Time Series",
    "^h", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (DEFAULT_IMODE) : gg, gg, 
    gg->imodeRadioGroup, radiop);
 if (radiop && gg->imode == DEFAULT_IMODE)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  /* Add a separator */
  CreateMenuItem (imode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

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

  gtk_widget_show (imode_menu);
  return (imode_menu);
}
#endif
/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */  
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists tsplot_cpanel_init --*/

void
cpanel_tsplot_set (displayd *display, cpaneld *cpanel, GtkWidget *panelWidget, ggobid *gg)
{
  GtkWidget *w;

  w = widget_find_by_name (panelWidget,
                           "TSPLOT:sel_mode_option_menu");

  gtk_combo_box_set_active (GTK_COMBO_BOX(w),
                               cpanel->tsplot_selection_mode);
}
