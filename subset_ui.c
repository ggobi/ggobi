/* subset_ui.c */

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define SS_RANDOM 0
#define SS_BLOCK  1
#define SS_RANGE  2
#define SS_EVERYN 3
#define SS_STICKY 4
#define SS_ROWLAB 5

/*-- called when closed from the close button --*/
static void close_btn_cb (GtkWidget *w, ggobid *gg) {
  gtk_widget_hide (gg->subset_ui.window);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
  gtk_widget_hide (gg->subset_ui.window);
}

static datad *
datad_get_from_widget (GtkWidget *w, ggobid *gg)
{
  datad *d = NULL;

  if (g_slist_length (gg->d) == 0)
    ;
  else if (g_slist_length (gg->d) == 1) 
    d = gg->d->data;
  else {
    GtkWidget *clist = (GtkWidget *)
      gtk_object_get_data (GTK_OBJECT (w), "datad_clist");
    if (clist) {
      GList *selection = GTK_CLIST (clist)->selection;
      if (selection) {
        gint kd = (gint) selection->data;
        /*-- Assume that all datad's are included --*/
        if (kd >= 0) d = (datad *) g_slist_nth_data (gg->d, kd);
      }
    }
  }

  return d;
}

static void
set_adjustment (GtkWidget *w, GtkAdjustment *adj_new)
{
  GtkAdjustment *adj_current;
  GtkSpinButton *btn;
  if (w) {
    btn = GTK_SPIN_BUTTON (w);
    adj_current = gtk_spin_button_get_adjustment (btn);
    if ((gint)adj_current != (gint)adj_new) {
      gtk_object_ref (GTK_OBJECT(adj_current));
      gtk_spin_button_set_adjustment (btn, adj_new);
    }
  }
}

static const gchar *const substr_lbl[] = {
 "Is identical to the string",
 "Includes the string",
 "Begins with the string",
 "Ends with the string",
 "Does not include the string",
};
static void subset_string_pos_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  datad *d = datad_get_from_widget (w, gg);
  GtkWidget *tgl = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:IGNORE_CASE");

  d->subset.string_pos = GPOINTER_TO_INT (cbd);

  /*
   * I'm not allowing the user to ignore the case of the string
   * when I'm using strstr to test whether the string is included
  */
  if (d->subset.string_pos == 1 || d->subset.string_pos == 4)
    gtk_widget_set_sensitive (tgl, false);
  else
    gtk_widget_set_sensitive (tgl, true);
}

static void
subset_display_update (datad *d, ggobid *gg)
{
  GtkWidget *spinbtn, *entry;
  /*
   * If this is a different d than was used the last time
   * the subset panel was opened, attach the right adjustments
   * to the spin_buttons.
  */
  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.bstart_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.bstart_adj);
  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.bsize_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.bsize_adj);

  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.estart_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.estart_adj);
  spinbtn = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(d->subset.estep_adj), "WIDGET");
  set_adjustment (spinbtn, d->subset.estep_adj);

  /*-- ... and set the values of the text entries, too --*/
  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:RANDOM_ENTRY");
  if (entry) {
    gchar *txt = g_strdup_printf ("%d", d->subset.random_n);
    gtk_entry_set_text (GTK_ENTRY (entry), txt);
    g_free (txt);
  }
  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:NROWS_ENTRY");
  if (entry) {
    gchar *txt = g_strdup_printf ("%d", d->nrows);
    gtk_entry_set_text (GTK_ENTRY (entry), txt);
    g_free (txt);
  }
  /*-- --*/
}


static void
subset_datad_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, ggobid *gg)
{
  /*-- Assume that all datad's are included --*/
  datad *d = g_slist_nth_data (gg->d, row);
  if (d)
    subset_display_update (d, gg);
}

static void
rescale_cb (GtkWidget *w, ggobid *gg)
{
  datad *d = datad_get_from_widget (w, gg);
  if (d) {
    limits_set (true, true, d, gg);
    vartable_limits_set (d);
    vartable_stats_set (d);

    tform_to_world (d, gg);
    displays_tailpipe (FULL, gg);
  }
}

static void
subset_cb (GtkWidget *w, ggobid *gg)
{
  gint subset_type;
  gchar *sample_str, *substr;
  gint bstart, bsize;
  gint estart, estep;
  gboolean redraw;
  datad *d = datad_get_from_widget (w, gg);
  GtkWidget *entry, *tgl;

  if (!d)
    return;

  subset_type = 
    gtk_notebook_get_current_page (GTK_NOTEBOOK (gg->subset_ui.notebook));

  switch (subset_type) {
    case SS_RANDOM:
      entry = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:RANDOM_ENTRY");
      sample_str = 
        gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
      d->subset.random_n = atoi (sample_str);
      redraw = subset_random (d->subset.random_n, d, gg);
    break;
    case SS_BLOCK:
      bstart = (gint) d->subset.bstart_adj->value;
      bsize = (gint) d->subset.bsize_adj->value;
      redraw = subset_block (bstart-1, bsize, d, gg);
    break;
    case SS_RANGE:
      redraw = subset_range (d, gg);
    break;
    case SS_EVERYN:
      estart = (gint) d->subset.estart_adj->value;
      estep = (gint) d->subset.estep_adj->value;
      redraw = subset_everyn (estart-1, estep, d, gg);
    break;
    case SS_STICKY:
      redraw = subset_sticky (d, gg);
    break;
    case SS_ROWLAB:
      /* use a toggle widget to specify whether to ignore case or not */
      entry = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window), "SS:ROWLAB");
      tgl = (GtkWidget *)
        gtk_object_get_data (GTK_OBJECT(gg->subset_ui.window),
          "SS:IGNORE_CASE");
      substr = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
      redraw = subset_rowlab (substr, d->subset.string_pos,
        gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(tgl)),
        d, gg);
    break;
  }

  if (redraw)
    subset_apply (d, gg);
}

static void
include_all_cb (GtkWidget *w, ggobid *gg) {
  datad *d = datad_get_from_widget (w, gg);

  if (d != NULL) {
    subset_include_all (d, gg);
    subset_apply (d, gg);
  }
}


static void 
subset_clist_datad_added_cb (ggobid *gg, datad *d, void *clist)
{
  gchar *row[1];
  GtkWidget *swin = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT (clist), "datad_swin");

  subset_init (d, gg);
  row[0] = g_strdup (d->name);
  gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
  g_free (row[0]);

  gtk_widget_show_all (swin);
}

CHECK_EVENT_SIGNATURE(subset_clist_datad_added_cb,datad_added_f)

/*------------------------------------------------------------------*/

void
subset_window_open (ggobid *gg, guint action, GtkWidget *w) {

  GtkWidget *button, *t;
  GtkWidget *vbox, *frame, *hb, *vb, *button_hbox, *close_hbox;
  GtkWidget *label, *btn, *spinbtn, *entry, *opt;
  datad *d;
  gchar *clist_titles[1] = {"datasets"};

  GtkWidget *swin, *clist;
  gchar *row[1];
  GSList *l;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
    return;

  else {

    d = gg->d->data;

    if (gg->subset_ui.window == NULL) {
    
      gg->subset_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title (GTK_WINDOW (gg->subset_ui.window),
        "subset data");
      gtk_signal_connect (GTK_OBJECT (gg->subset_ui.window),
        "delete_event", GTK_SIGNAL_FUNC (close_wmgr_cb), (gpointer) gg);
  
      gtk_container_set_border_width (GTK_CONTAINER (gg->subset_ui.window), 5);

      vbox = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (gg->subset_ui.window), vbox);


      /* Create a scrolled window to pack the CList widget into */
      swin = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
        GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

      clist = gtk_clist_new_with_titles (1, clist_titles);
      gtk_clist_set_selection_mode (GTK_CLIST (clist),
        GTK_SELECTION_SINGLE);
      gtk_object_set_data (GTK_OBJECT (clist), "datad_swin", swin);
      gtk_signal_connect (GTK_OBJECT (clist), "select_row",
        (GtkSignalFunc) subset_datad_set_cb, gg);
      gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
        (GtkSignalFunc) subset_clist_datad_added_cb, GTK_OBJECT (clist));
      /*-- --*/

      /*-- All datad's are included. This assumption is used in two places. */
      for (l = gg->d; l; l = l->next) {
        d = (datad *) l->data;
        subset_init (d, gg);
        row[0] = g_strdup (d->name);
        gtk_clist_append (GTK_CLIST (clist), row);
        g_free (row[0]);
      }
      gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
      gtk_container_add (GTK_CONTAINER (swin), clist);
      gtk_box_pack_start (GTK_BOX (vbox), swin, false, false, 2);

      d = gg->d->data;
    
      /* Create a new notebook, place the position of the tabs */
      gg->subset_ui.notebook = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->subset_ui.notebook),
        GTK_POS_TOP);
      gtk_box_pack_start (GTK_BOX (vbox), gg->subset_ui.notebook,
        false, false, 2);
    
      /*-- Random sample without replacement --*/
      frame = gtk_frame_new ("Random sample without replacement");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      hb = gtk_hbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), hb);

      gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("Sample size"),
        false, false, 2);
  
      /*-- entry: random sample size --*/
      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:RANDOM_ENTRY", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in the desired sample size", NULL);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

      gtk_box_pack_start (GTK_BOX (hb), gtk_label_new ("out of"),
        false, false, 2);

      /*-- entry: data size --*/
      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:NROWS_ENTRY", entry);
      gtk_entry_set_editable (GTK_ENTRY (entry), false);
      gtk_box_pack_start (GTK_BOX (hb), entry, true, true, 2);

      label = gtk_label_new ("Random");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);
      
      /*-----------------------*/
      /*-- Consecutive block --*/
      /*-----------------------*/
      frame = gtk_frame_new ("Consecutive block");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      t = gtk_table_new (2, 2, true);
      gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
      gtk_container_set_border_width (GTK_CONTAINER (t), 5);
      gtk_container_add (GTK_CONTAINER (frame), t);

      /*-- Block subsetting: First case (bstart) --*/
      vb = gtk_vbox_new (false, 3);
      label = gtk_label_new ("First case:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      spinbtn = gtk_spin_button_new (d->subset.bstart_adj, 0, 0);
      gtk_object_set_data (GTK_OBJECT(d->subset.bstart_adj), "WIDGET", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        spinbtn, "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- Block subsetting: blocksize (bsize) --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("Blocksize:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      spinbtn = gtk_spin_button_new (d->subset.bsize_adj, 0, 0);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
        spinbtn, "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb),
        spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);


      label = gtk_label_new ("Block");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*---------------------------*/
      /*-- Points within a range --*/
      /*---------------------------*/
      frame = gtk_frame_new ("Variable limits");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      vb = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), vb);

      gtk_box_pack_start (GTK_BOX (vb),
        gtk_label_new ("Exclude data outside the user limits\nin the variable manipulation table"),
        false, false, 0);

      label = gtk_label_new ("Limits");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*--------------------*/
      /*-- Every nth case --*/
      /*--------------------*/
      frame = gtk_frame_new ("Every nth case");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      t = gtk_table_new (1, 2, true);
      gtk_table_set_col_spacing (GTK_TABLE (t), 0, 20);
      gtk_container_set_border_width (GTK_CONTAINER (t), 5);
      gtk_container_add (GTK_CONTAINER (frame), t);

      /*-- everyn subsetting: start --*/
      vb = gtk_vbox_new (false, 3);
      label = gtk_label_new ("First case:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      spinbtn = gtk_spin_button_new (d->subset.estart_adj, 0, 0);
      gtk_object_set_data (GTK_OBJECT(d->subset.estart_adj), "WIDGET", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinbtn,
        "Specify the first row of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 0,1,0,1);

      /*-- everyn subsetting: stepsize --*/
      vb = gtk_vbox_new (false, 2);
      label = gtk_label_new ("N:");
      gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
      gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

      spinbtn = gtk_spin_button_new (d->subset.estep_adj, 0, 0);
      gtk_object_set_data (GTK_OBJECT(d->subset.estep_adj), "WIDGET", spinbtn);
      gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinbtn), false);
#if GTK_MAJOR_VERSION == 1
      gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinbtn),
                                       GTK_SHADOW_OUT);
#endif
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinbtn,
        "Specify the size of the block", NULL);
      gtk_box_pack_start (GTK_BOX (vb), spinbtn, false, false, 0);
      gtk_table_attach_defaults (GTK_TABLE (t), vb, 1,2,0,1);

      label = gtk_label_new ("Every n");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*-------------------------------------------------------*/
      /*-- Cases whose row label is one of the sticky labels --*/
      /*-------------------------------------------------------*/
      frame = gtk_frame_new ("Cases whose row label is sticky");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
      gtk_widget_set_usize (frame, 100, 75);

      vb = gtk_vbox_new (false, 2);
      gtk_container_add (GTK_CONTAINER (frame), vb);

      gtk_box_pack_start (GTK_BOX (vb),
        gtk_label_new ("Include only those cases with a sticky label"),
        false, false, 0);

      label = gtk_label_new ("Sticky");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*---------------------------------------------------------*/
      /*-- Cases whose row label includes the specified string --*/
      /*---------------------------------------------------------*/
      frame = gtk_frame_new ("Cases with specified row label");
      gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

      vb = gtk_vbox_new (false, 5);
      gtk_container_add (GTK_CONTAINER (frame), vb);

      hb = gtk_hbox_new (false, 5);
      gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 5);

      label = gtk_label_new ("Substring:");
      gtk_box_pack_start (GTK_BOX (hb), label, false, false, 2);

      entry = gtk_entry_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:ROWLAB", entry);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
        "Type in a string to specify the cases you want in the subset",
        NULL);
      gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);

      hb = gtk_hbox_new (false, 5);
      gtk_box_pack_start (GTK_BOX (vb), hb, false, false, 5);

      opt = gtk_option_menu_new ();
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:ROWLAB_POS", opt);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
        "Specify the position in the row labels to check for the substring",
        NULL);
      gtk_box_pack_start (GTK_BOX (hb), opt, false, false, 0);
      populate_option_menu (opt, (gchar**) substr_lbl,
        sizeof (substr_lbl) / sizeof (gchar *),
        (GtkSignalFunc) subset_string_pos_cb, "GGobi", gg);

      btn = gtk_check_button_new_with_label ("Ignore case");
      gtk_object_set_data (GTK_OBJECT(gg->subset_ui.window),
        "SS:IGNORE_CASE", btn);
      gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON(btn), true);
      gtk_box_pack_start (GTK_BOX (hb), btn, false, false, 0);

      label = gtk_label_new ("Row label");
      gtk_notebook_append_page (GTK_NOTEBOOK (gg->subset_ui.notebook),
        frame, label);

      /*-- hbox to hold a few buttons --*/
      button_hbox = gtk_hbox_new (true, 2);

      gtk_box_pack_start (GTK_BOX (vbox), button_hbox, false, false, 2);

      button = gtk_button_new_with_label ("Subset");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Draw a new subset and update all plots", NULL);
      gtk_object_set_data (GTK_OBJECT (button), "datad_clist", clist);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (subset_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);

      button = gtk_button_new_with_label ("Rescale");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Rescale the data after choosing a new subset", NULL);
      gtk_object_set_data (GTK_OBJECT (button), "datad_clist", clist);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (rescale_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);
    
      button = gtk_button_new_with_label ("Include all");
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), button,
        "Stop subsetting: include all cases and update all plots", NULL);
      gtk_object_set_data (GTK_OBJECT (button), "datad_clist", clist);
      gtk_signal_connect (GTK_OBJECT (button), "clicked",
                          GTK_SIGNAL_FUNC (include_all_cb), (gpointer) gg);
      gtk_box_pack_start (GTK_BOX (button_hbox), button, true, true, 2);

      /*-- Separator --*/
      gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
        false, true, 2);

      /*-- Close button --*/
      close_hbox = gtk_hbox_new (false, 2);
      gtk_box_pack_start (GTK_BOX (vbox), close_hbox, false, false, 1);

      btn = gtk_button_new_with_label ("Close");
      gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                          GTK_SIGNAL_FUNC (close_btn_cb), (ggobid *) gg);
      gtk_box_pack_start (GTK_BOX (close_hbox), btn, true, false, 0);

      /*-- initialize display --*/
      subset_display_update (gg->d->data, gg);

      if (g_slist_length (gg->d) > 1)
        gtk_widget_show_all (swin);
      gtk_widget_show (vbox);
      gtk_widget_show_all (button_hbox);
      gtk_widget_show_all (close_hbox);
      gtk_widget_show_all (gg->subset_ui.notebook);
    }  /*-- if window == NULL --*/

    gtk_widget_show (gg->subset_ui.window);
    gdk_window_raise (gg->subset_ui.window->window);
  }
}
