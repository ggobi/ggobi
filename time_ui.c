
/* time_ui.c */

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
static void arrangement_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
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
static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append","Delete", "Overlay"};
#else
static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append","Delete"};
#endif
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->tsplot_selection_mode = GPOINTER_TO_INT (cbd);
}

#ifdef TS_EXTENSIONS_IMPLEMENTED
/*
 * "Common" scales all series by 
 * argmax(argmax(var[i]) forall i)-argmin(argmin(var[i] forall i)), 
 * while "Independent" scales each series independently by its min 
 * and max to [0,1].
 */
static gchar *varscale_lbl[] = {"Common", "Independent"};
static void varscale_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", varscale_lbl[indx]);
}
#endif

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

void
cpanel_tsplot_make (ggobid *gg) {
  GtkWidget *vb, *lbl, *opt;
  
  gg->control_panel[TSPLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[TSPLOT]), 5);

#ifdef TS_EXTENSIONS_IMPLEMENTED
/*
 * arrangement of plots, row or column
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TSPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Layout:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Arrange the time series as single plot or several plots",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, arrangement_lbl,
                        sizeof (arrangement_lbl) / sizeof (gchar *),
                        arrangement_cb, gg);
#endif

/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TSPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Selecting a variable either replaces the variable in the current plot (swapping if appropriate), inserts a new plot before the current plot, or appends a new plot after it",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
                        sizeof (selection_mode_lbl) / sizeof (gchar *),
                        selection_mode_cb, gg);

/*
 * Variable scales
*/

#ifdef TS_EXTENSIONS_IMPLEMENTED
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[TSPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Scales:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
   "Scale variables (and variable groups) on a common scale, or independently",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, varscale_lbl,
                        sizeof (varscale_lbl) / sizeof (gchar *),
                        varscale_cb, gg);
#endif


  gtk_widget_show_all (gg->control_panel[TSPLOT]);
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/



/*
  The useIds indicates whether the callback data should be integers
  identifying the menu item or the global gg.
  At present, this is always false.
  See scatmat_mode_menu_make and scatterplot_mode_menu_make.
 */
void
tsplot_mode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func, ggobid *gg, gboolean useIds) {

/*
 * I/O menu
*/
  gg->tsplot.mode_menu = gtk_menu_new ();

  CreateMenuItem (gg->tsplot.mode_menu, "Time Series",
    "^v", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (TSPLOT) : gg, gg);

  /* Add a separator */
  CreateMenuItem (gg->tsplot.mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->tsplot.mode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg);
  CreateMenuItem (gg->tsplot.mode_menu, "Identify",
    "^i", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (IDENT) : gg, gg);

  gtk_widget_show (gg->tsplot.mode_menu);
}

void
tsplot_menus_make (ggobid *gg) {
/*
 * Options menu
*/
  gg->menus.options_item = submenu_make ("_Options", 'O',
    gg->main_accel_group);
  gg->menus.options_menu = gtk_menu_new ();

  CreateMenuCheck (gg->menus.options_menu, "Show tooltips",
    GTK_SIGNAL_FUNC (tooltips_show_cb), NULL,
    GTK_TOOLTIPS (gg->tips)->enabled, gg);

  CreateMenuCheck (gg->menus.options_menu, "Show control panel",
    GTK_SIGNAL_FUNC (cpanel_show_cb), NULL,
    GTK_WIDGET_VISIBLE (gg->mode_frame), gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->menus.options_item),
    gg->menus.options_menu);
  submenu_insert (gg->menus.options_item, gg->main_menubar, OPTIONS_MENU_POS);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */  
/*--------------------------------------------------------------------*/
