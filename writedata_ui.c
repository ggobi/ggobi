/* writedata_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "writedata.h"

static GtkWidget *window = NULL;

static gchar *format_lbl[] =
  {"XML", "File set (ascii)", "File set (binary)", "MySQL"};
void format_set (gint fmt, ggobid *gg) { gg->save.format = fmt; }
static void format_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  format_set (GPOINTER_TO_INT (cbd), gg);
}

static gchar *stage_lbl[] = {"Raw data", "Transformed data"};
void stage_set (gint stage, ggobid *gg) { gg->save.stage = stage; }
static void stage_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  stage_set (GPOINTER_TO_INT (cbd), gg);
}

static gchar *jitter_lbl[] = {"Don't add jitter", "Add jitter"};
void jitterp_set (gboolean jitterp, ggobid *gg) { gg->save.jitter_p = jitterp; }
static void jitterp_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  jitterp_set ((gboolean ) GPOINTER_TO_INT (cbd), gg);
}

static gchar *rowdata_lbl[] = {"All cases",
                               "Displayed cases",
                               "Labeled cases"};
void rowind_set (gint ind, ggobid *gg) { gg->save.row_ind = ind; }
static void rowind_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  rowind_set (GPOINTER_TO_INT (cbd), gg);
}

static gchar *columndata_lbl[] = {"All variables",
                                  "Selected variables"};
void columnind_set (gint ind, ggobid *gg) { gg->save.column_ind = ind; }
static void columnind_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  columnind_set (GPOINTER_TO_INT (cbd), gg);
}

static gchar *missing_lbl[] = {"Missings as 'na'",
                               "Missings as '.'",
                               "Imputed values"};
void missingind_set (gint ind, ggobid *gg) { gg->save.missing_ind = ind; }
static void missingind_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  missingind_set (GPOINTER_TO_INT (cbd), gg);
}

static gchar *lines_lbl[] = {"Don't save lines", "Save lines"};
void linesp_set (gboolean linesp, ggobid *gg) { gg->save.lines_p = linesp; }
static void linesp_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  linesp_set ((gboolean ) GPOINTER_TO_INT (cbd), gg);
}

/*-- called when closed from the button --*/
static void
close_cb (GtkWidget *w) {
  gtk_widget_destroy (w);
  window = NULL;
}
/*-- called when closed from the window manager --*/
static void delete_cb (GtkWidget *w, GdkEvent *event) {
  gtk_widget_destroy (window);
  window = NULL;
}

extern void filename_get_w (GtkWidget *, ggobid *);
void
writeall_window_open (ggobid *gg) {
  GtkWidget *vbox, *table, *opt, *btn;
  gint j;

  if (window == NULL) {

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                        GTK_SIGNAL_FUNC (delete_cb), (gpointer) gg);
    gtk_window_set_title (GTK_WINDOW (window), "create ggobi file set");
    
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    vbox = gtk_vbox_new (false, VBOX_SPACING);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    table = gtk_table_new (7, 2, false);
/*    gtk_container_add (GTK_CONTAINER (window), table);*/
    gtk_box_pack_start (GTK_BOX (vbox), table,
      false, false, 3);

    /*-- Format --*/
    j = 0;
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Save the data in XML or in a ggobi file set (data in ascii or binary",
      NULL);
    populate_option_menu (opt, format_lbl,
      sizeof (format_lbl) / sizeof (gchar *), format_set_cb, gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      XMLDATA);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Format:"),
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Stage --*/
    j++;
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Save raw or transformed data",
      NULL);
    populate_option_menu (opt, stage_lbl,
      sizeof (stage_lbl) / sizeof (gchar *), stage_set_cb, gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      TFORMDATA);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Stage:"),
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
      sizeof (jitter_lbl) / sizeof (gchar *), jitterp_set_cb, gg);
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
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Specify which rows should be written out",
      NULL);
    populate_option_menu (opt, rowdata_lbl,
      sizeof (rowdata_lbl) / sizeof (gchar *), rowind_set_cb, gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      ALLROWS);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Cases:"),
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Which columns --*/
    j++;
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Specify which variables should be written out",
      NULL);
    populate_option_menu (opt, columndata_lbl,
      sizeof (columndata_lbl) / sizeof (gchar *), columnind_set_cb, gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      ALLCOLS);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Variables:"),
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Format for missings --*/
    j++;
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Specify how to write out missing data",
      NULL);
    populate_option_menu (opt, missing_lbl,
      sizeof (missing_lbl) / sizeof (gchar *), missingind_set_cb, gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      MISSINGSNA);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Format for missings:"),
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

    /*-- Lines? --*/
    j++;
    opt = gtk_option_menu_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
      "Include line segments?",
      NULL);
    populate_option_menu (opt, lines_lbl,
      sizeof (lines_lbl) / sizeof (gchar *), linesp_set_cb, gg);
    gtk_option_menu_set_history (GTK_OPTION_MENU (opt),
      false);

    gtk_table_attach (GTK_TABLE (table),
      gtk_label_new ("Lines?:"),
      0, 1, j, j+1, GTK_FILL, GTK_FILL, 5, 0);
    gtk_table_attach (GTK_TABLE (table), opt,
      1, 2, j, j+1, GTK_FILL, GTK_FILL, 5, 0);

/*
Add a button to open a file selection box; see filename_get in main_ui.c
*/
    btn = gtk_button_new_with_label ("Save ...");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Open file selection widget", NULL);
    gtk_box_pack_start (GTK_BOX (vbox), btn,
                        false, false, 3);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (filename_get_w), gg);


    gtk_widget_show_all (window);
  }

  gdk_window_raise (window->window);
}

