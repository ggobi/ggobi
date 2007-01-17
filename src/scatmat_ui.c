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

/*------------------------------------------------------------------------*/
/*                         Control panel                                  */
/*------------------------------------------------------------------------*/

GtkWidget *
cpanel_scatmat_make (GGobiSession * gg)
{
  modepaneld *panel;

  panel = (modepaneld *) g_malloc (sizeof (modepaneld));
  gg->control_panels = g_list_append (gg->control_panels, (gpointer) panel);
  panel->name = g_strdup ("SCATMAT");
  panel->w = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (panel->w), 5);

  gtk_widget_show_all (panel->w);

  return panel->w;
}


/*------------------------------------------------------------------------*/
/*                       Resetting the main menubar                       */
/*------------------------------------------------------------------------*/

static const gchar *mode_ui_str =
  "<ui>"
  "	<menubar>"
  "		<menu action='IMode'>"
  "			<menuitem action='DefaultIMode'/>"
  "			<separator/>"
  "			<menuitem action='Scale'/>"
  "			<menuitem action='Brush'/>"
  "			<menuitem action='Identify'/>" "		</menu>" "	</menubar>" "</ui>";

const gchar *
scatmat_mode_ui_get (displayd * display)
{
  return (mode_ui_str);
}


/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists scatmat_cpanel_init --*/

void
cpanel_scatmat_set (displayd * display, cpaneld * cpanel, GGobiSession * gg)
{
}
