/* time_ui.c */
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
static gchar *arrangement_lbl[] = { "Split", "Joint" };
static void
arrangement_cb (GtkWidget * w, ggobid * gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX (w));
  g_printerr ("cbd: %s\n", arrangement_lbl[indx]);

  if (indx != gg->current_display->cpanel.tsplot_arrangement)
    tsplot_reset_arrangement (gg->current_display, indx, gg);

  gg->current_display->cpanel.tsplot_arrangement = indx;
}
#endif

#ifdef TS_EXTENSIONS_IMPLEMENTED
/*
 * "Common" scales all series by 
 * argmax(argmax(var[i]) forall i)-argmin(argmin(var[i] forall i)), 
 * while "Independent" scales each series independently by its min 
 * and max to [0,1].
 */
static gchar *varscale_lbl[] = { "Common", "Independent" };
static void
varscale_cb (GtkWidget * w, ggobid * gg)
{
  gint indx = gtk_combo_box_get_active (GTK_COMBO_BOX (w));
  g_printerr ("cbd: %s\n", varscale_lbl[indx]);
}
#endif

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

GtkWidget *
cpanel_tsplot_make (ggobid * gg)
{
#ifdef TS_EXTENSIONS_IMPLEMENTED
  GtkWidget *vb, *lbl, *opt;
#endif
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
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), lbl);
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                        "Arrange the time series as single plot or several plots",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, arrangement_lbl, G_N_ELEMENTS (arrangement_lbl),
                      arrangement_cb, gg);
#endif

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
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), opt);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
                        "Scale variables (and variable groups) on a common scale, or independently",
                        NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, varscale_lbl, G_N_ELEMENTS (varscale_lbl),
                      varscale_cb, gg);
#endif


  gtk_widget_show_all (cpanel);

  return (cpanel);
}


/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

static const gchar *mode_ui_str =
  "<ui>"
  "	<menubar>"
  "		<menu action='IMode'>"
  "			<menuitem action='DefaultIMode'/>"
  "			<separator/>"
  "			<menuitem action='Brush'/>"
  "			<menuitem action='Identify'/>" "		</menu>" "	</menubar>" "</ui>";

const gchar *
tsplot_mode_ui_get (displayd * display)
{
  return (mode_ui_str);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists tsplot_cpanel_init --*/

void
cpanel_tsplot_set (displayd * display, cpaneld * cpanel,
                   GtkWidget * panelWidget, ggobid * gg)
{
}
