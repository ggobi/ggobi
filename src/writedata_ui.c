/* writedata_ui.c */
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

#include "writedata.h"

static gchar *format_lbl[] =
  {"XML", "File set (ascii)",
#ifdef BINARY_IO_IMPLEMENTED
"File set (binary)"
#endif
};
void format_set (gint fmt, ggobid *gg) { gg->save.format = fmt; }
static void format_set_cb (GtkWidget *w, ggobid *gg)
{
  format_set (gtk_combo_box_get_active(GTK_COMBO_BOX(w)), gg);
}

static gchar *stage_lbl[] = {"Raw data", "Transformed data"};
void stage_set (gint stage, ggobid *gg) { gg->save.stage = stage; }
static void stage_set_cb (GtkWidget *w, ggobid *gg)
{
  stage_set (gtk_combo_box_get_active(GTK_COMBO_BOX(w)), gg);
}

/*
static gchar *jitter_lbl[] = {"Don't add jitter", "Add jitter"};
void jitterp_set (gboolean jitterp, ggobid *gg) { gg->save.jitter_p = jitterp; }
static void jitterp_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  jitterp_set ((gboolean ) GPOINTER_TO_INT (cbd), gg);
}
*/

static gchar *rowdata_lbl[] = {"All cases",
                               "Displayed cases",
                               /*"Labeled cases"*/};
void rowind_set (gint ind, ggobid *gg) { gg->save.row_ind = ind; }
static void rowind_set_cb (GtkWidget *w, ggobid *gg)
{
  rowind_set (gtk_combo_box_get_active(GTK_COMBO_BOX(w)), gg);
}

static gchar *columndata_lbl[] = {"All variables",
                                  "Selected variables"};
void columnind_set (gint ind, ggobid *gg) { gg->save.column_ind = ind; }
static void columnind_set_cb (GtkWidget *w, ggobid *gg)
{;
  columnind_set (gtk_combo_box_get_active(GTK_COMBO_BOX(w)), gg);
}

static gchar *missing_lbl[] = {"Missings as 'na'",
                               "Missings as '.'",
                               "Imputed values"};
void missingind_set (gint ind, ggobid *gg) { gg->save.missing_ind = ind; }
static void missingind_set_cb (GtkWidget *w, ggobid *gg)
{
  missingind_set (gtk_combo_box_get_active(GTK_COMBO_BOX(w)), gg);
}

static gchar *edges_lbl[] = {"Don't save edges", "Save edges"};
void edgesp_set (gboolean edgesp, ggobid *gg) { gg->save.edges_p = edgesp; }
static void edgesp_set_cb (GtkWidget *w, ggobid *gg)
{
  edgesp_set ((gboolean ) gtk_combo_box_get_active(GTK_COMBO_BOX(w)), gg);
}

/*-- called when closed from the button -- what button? --*/
/*
static void
close_cb (GtkWidget *w) {
  gtk_widget_destroy (w);
  window = NULL;
}
*/
/*-- called when closed from the window manager --*/
static void delete_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
  gtk_widget_destroy (gg->save.window);
  gg->save.window = NULL;
}

void
writeall_window_open (ggobid *gg) {
  GtkWidget *vbox, *table, *opt, *btn, *lbl;
  gint j;

  if (gg->save.window == NULL) {

    gg->save.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (G_OBJECT (gg->save.window), "delete_event",
                        G_CALLBACK (delete_cb), (gpointer) gg);
    gtk_window_set_title (GTK_WINDOW (gg->save.window), "create ggobi file set");
    
    gtk_container_set_border_width (GTK_CONTAINER (gg->save.window), 10);

    vbox = gtk_vbox_new (false, VBOX_SPACING);
    gtk_container_add (GTK_CONTAINER (gg->save.window), vbox);

    table = gtk_table_new (7, 2, false);
/*    gtk_container_add (GTK_CONTAINER (gg->save.window), table);*/
    gtk_box_pack_start (GTK_BOX (vbox), table,
      false, false, 3);

    /*-- Format --*/
    j = 0;
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Save the data in XML or in a ggobi file set (data in ascii or binary",
      NULL);
    populate_combo_box (opt, format_lbl, G_N_ELEMENTS(format_lbl),
      G_CALLBACK(format_set_cb), gg);
    gtk_combo_box_set_active (GTK_COMBO_BOX (opt),
      XMLDATA);
    /*-- initialize variable to correspond to option menu --*/
    gg->save.format = XMLDATA;

	lbl = gtk_label_new_with_mnemonic ("_Format:");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
    gtk_table_attach (GTK_TABLE (table), lbl,
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Stage --*/
    j++;
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Save raw or transformed data",
      NULL);
    populate_combo_box (opt, stage_lbl, G_N_ELEMENTS(stage_lbl),
      G_CALLBACK(stage_set_cb), gg);
    gtk_combo_box_set_active (GTK_COMBO_BOX (opt),
      TFORMDATA);
    /*-- initialize variable to correspond to option menu --*/
    gg->save.stage = TFORMDATA;

	lbl = gtk_label_new_with_mnemonic ("_Stage:");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
    gtk_table_attach (GTK_TABLE (table), lbl,
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Jitter? --*/
/*
    j++;
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Include any added jitter?",
      NULL);
    populate_option_menu (opt, jitter_lbl,
      sizeof (jitter_lbl) / sizeof (gchar *), jitterp_set_cb, "GGobi", gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      true);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Jitter?:"),
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
*/

    /*-- Which rows --*/
    j++;
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Specify which rows should be written out",
      NULL);
    populate_combo_box (opt, rowdata_lbl, G_N_ELEMENTS(rowdata_lbl),
      G_CALLBACK(rowind_set_cb), gg);
    gtk_combo_box_set_active (GTK_COMBO_BOX (opt),
      ALLROWS);
    /*-- initialize variable to correspond to option menu --*/
    gg->save.row_ind = ALLROWS;

	lbl = gtk_label_new_with_mnemonic ("_Cases:");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
    gtk_table_attach (GTK_TABLE (table), lbl,
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Which columns --*/
    j++;
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Specify which variables should be written out",
      NULL);
    populate_combo_box (opt, columndata_lbl, G_N_ELEMENTS(columndata_lbl),
      G_CALLBACK(columnind_set_cb), gg);
    gtk_combo_box_set_active (GTK_COMBO_BOX (opt),
      ALLCOLS);
    /*-- initialize variable to correspond to option menu --*/
    gg->save.column_ind = ALLCOLS;

	lbl = gtk_label_new_with_mnemonic ("_Variables:");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
    gtk_table_attach (GTK_TABLE (table), lbl,
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Format for missings --*/
    j++;
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Specify how to write out missing data",
      NULL);
    populate_combo_box (opt, missing_lbl, G_N_ELEMENTS(missing_lbl),
      G_CALLBACK(missingind_set_cb), gg);
    gtk_combo_box_set_active (GTK_COMBO_BOX (opt),
      MISSINGSNA);
    /*-- initialize variable to correspond to option menu --*/
    gg->save.missing_ind = MISSINGSNA;

	lbl = gtk_label_new_with_mnemonic ("Format for _missings:");
	gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
    gtk_table_attach (GTK_TABLE (table), lbl,
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- edges? --*/
    j++;
    opt = gtk_combo_box_new_text ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Include line segments?",
      NULL);
    populate_combo_box (opt, edges_lbl, G_N_ELEMENTS(edges_lbl),
      G_CALLBACK(edgesp_set_cb), gg);
    /*-- initialize variable corresponding to option menu --*/
    /*
     * This is pretty simple-minded:  if any edgesets are present,
     * let the default be to save edges.  Otherwise not.
     */
    gg->save.edges_p = (edgesets_count(gg) > 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX (opt),
      gg->save.edges_p);

	  lbl = gtk_label_new_with_mnemonic ("_Edges?:");
	  gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), opt);
    gtk_table_attach (GTK_TABLE (table), lbl,
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

/*
 * Add a button to open a file selection box; see filename_get_* in io.c
*/
    btn = gtk_button_new_from_stock (GTK_STOCK_SAVE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Open file selection widget", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn,
                        false, false, 3);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (filename_get_w), gg);


    gtk_widget_show_all (gg->save.window);
  }

  gdk_window_raise (gg->save.window->window);
}

