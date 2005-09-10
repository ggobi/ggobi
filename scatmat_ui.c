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
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  ggobid *gg = GGobiFromWidget(w, true);
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

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "SCATMAT:sel_mode_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "Selecting an unselected variable either replaces the variable in the current plot, inserts a new plot before the current plot, or appends a new plot after the last plot",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
    sizeof (selection_mode_lbl) / sizeof (gchar *),
    (GtkSignalFunc) selection_mode_cb, "GGobi", gg);

  gtk_widget_show_all (panel->w);

  return panel->w;
}


/*------------------------------------------------------------------------*/
/*                       Resetting the main menubar                       */
/*------------------------------------------------------------------------*/

GtkWidget *
scatmat_imode_menu_make (GtkAccelGroup *accel_group, GtkSignalFunc func,
  ggobid *gg, gboolean useIds) 
{
  GtkWidget *imode_menu, *item;
  gboolean radiop = sessionOptions->useRadioMenuItems;

  imode_menu = gtk_menu_new ();

  item = CreateMenuItemWithCheck (imode_menu, "Scatterplot Matrix",
    "^h", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (DEFAULT_IMODE) : gg, gg, 
    gg->imodeRadioGroup, radiop);
  if (radiop && gg->imode == DEFAULT_IMODE)
     gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), true);

  /*-- Add a separator --*/
  CreateMenuItem (imode_menu, NULL,
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

  /* temporarily disabled -- this tried to use movept_screen_to_raw,
which calls code in lineedit.c, which fails because the pmode is
wrong.  It will be necessarily to implement the reverse pipeline
as class-specific code.  dfs
  CreateMenuItem (imode_menu, "Move Points",
    "^m", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (MOVEPTS) : gg, gg);
  */

  gtk_widget_show (imode_menu);
  return (imode_menu);
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
    gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                                 cpanel->scatmat_selection_mode);
  }
}
