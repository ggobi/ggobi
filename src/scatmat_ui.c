/* scatmat_ui.c */
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

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*                       Callbacks                                      */
/*----------------------------------------------------------------------*/

static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append", "Delete"};
static void selection_mode_cb (GtkWidget *w, ggobid *gg)
{
  gint indx = gtk_combo_box_get_active(GTK_COMBO_BOX(w));
  cpaneld *cpanel = &gg->current_display->cpanel;

  switch (indx) {
    case 0:
      cpanel->scatmat_selection_mode = VAR_REPLACE;
    break;
    case 1:
      cpanel->scatmat_selection_mode = VAR_INSERT;
    break;
    case 2:
      cpanel->scatmat_selection_mode = VAR_APPEND;
    break;
    case 3:
      cpanel->scatmat_selection_mode = VAR_DELETE;
    break;
  }
}

/*------------------------------------------------------------------------*/
/*                         Control panel                                  */
/*------------------------------------------------------------------------*/

GtkWidget *
cpanel_scatmat_make (ggobid *gg) {
  modepaneld *panel;
  GtkWidget *vb, *lbl, *opt;
  
  panel = (modepaneld *) g_malloc(sizeof(modepaneld));
  gg->control_panels = g_list_append(gg->control_panels, (gpointer) panel);
  panel->name = g_strdup("SCATMAT");
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w),
                                  5);

 /*-- option menu: selection mode --*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (panel->w),
                      vb, false, false, 0);

  lbl = gtk_label_new_with_mnemonic ("_Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
  gtk_widget_set_name (opt, "SCATMAT:sel_mode_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Selecting an unselected variable either replaces the variable in the current plot, inserts a new plot before the current plot, or appends a new plot after the last plot",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_combo_box (opt, selection_mode_lbl, G_N_ELEMENTS(selection_mode_lbl),
    G_CALLBACK(selection_mode_cb), gg);

  gtk_widget_show_all (panel->w);

  return panel->w;
}


/*------------------------------------------------------------------------*/
/*                       Resetting the main menubar                       */
/*------------------------------------------------------------------------*/

static const gchar* mode_ui_str =
"<ui>"
"	<menubar>"
"		<menu action='IMode'>"
"			<menuitem action='DefaultIMode'/>"
"			<separator/>"
"			<menuitem action='Scale'/>"
"			<menuitem action='Brush'/>"
"			<menuitem action='Identify'/>"
"		</menu>"
"	</menubar>"
"</ui>";

const gchar*
scatmat_mode_ui_get(displayd *display)
{
	return(mode_ui_str);
}


/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists scatmat_cpanel_init --*/

void
cpanel_scatmat_set (displayd *display, cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *pnl = mode_panel_get_by_name("SCATMAT", gg);
  GtkWidget *w;

  if (pnl) {
    w = widget_find_by_name (pnl, "SCATMAT:sel_mode_option_menu");
    gtk_combo_box_set_active (GTK_COMBO_BOX(w),
                                 cpanel->scatmat_selection_mode);
  }
}
