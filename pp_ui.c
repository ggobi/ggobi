/* pp_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   600
#define HEIGHT  300

static GtkWidget *window = NULL;
static GtkWidget *control_frame;
static GtkWidget *mbar;
static GtkAccelGroup *pp_accel_group;

/* terms in expansion, bandwidth */
static GtkWidget *param_vb, *param_lbl, *param_scale;
static GtkAdjustment *param_adj;

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (window);
}

static void
options_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {

    case 0:
      if (w->active)
        gtk_widget_hide (control_frame);
      else
        gtk_widget_show (control_frame);
      break;

    case 1:
    case 2:
  }
}
static void
line_options_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {
    case 0:
    case 1:
    case 2:
  }
}
static void
bitmap_size_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {
    case 0:
    case 1:
    case 2:
  }
}
static void
replot_freq_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
  }
}
static void
optimize_cb (GtkToggleButton  *w) {
  g_printerr ("optimize?  %d\n", w->active);
}

static gchar *func_lbl[] = {"Natural Hermite", "Hermite", "Central Mass",
                            "Holes", "Skewness", "Legendre",
                            "Friedman-Tukey", "Entropy",
                            "Binned Friedman-Tukey", "Binned Entropy"
                            };
static void func_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", func_lbl[indx]);

  /* value, lower, upper, step_increment, page_increment, page_size */
  /* Note that the page_size value only makes a difference for
   * scrollbar widgets, and the highest value you'll get is actually
   * (upper - page_size). */


  if (indx == CENTRAL_MASS || indx == HOLES || indx == SKEWNESS)
    gtk_widget_hide (param_vb);
  else {
    gtk_widget_show (param_vb);

    if (indx >= FTS) {
      gtk_label_set (GTK_LABEL (param_lbl), "Bandwidth:");
      gtk_range_set_adjustment (GTK_RANGE (param_scale),
        (GtkAdjustment *) gtk_adjustment_new (1.0, .1, 7.0, .1, 1.0, 0.0));
      gtk_scale_set_digits (GTK_SCALE (param_scale), 3);
    } else {
      gtk_label_set (GTK_LABEL (param_lbl), "Terms in expansion:");
      gtk_range_set_adjustment (GTK_RANGE (param_scale),
        (GtkAdjustment *) gtk_adjustment_new (1.0, 1, 30.0, 1, 1.0, 0.0));
      gtk_scale_set_digits (GTK_SCALE (param_scale), 0);
    }
  }
}

static void bitmap_cb (GtkButton *button)
{
  g_printerr ("drop a new bitmp\n");
}
static void
return_to_bitmap_cb (GtkToggleButton  *w) {
  g_printerr ("return to bitmap?  %d\n", w->active);
}
static void
record_bitmap_cb (GtkToggleButton  *w) {
  g_printerr ("record bitmap?  %d\n", w->active);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,         NULL, 0, "<Branch>" },
  { "/File/Close",  
         "",         hide_cb,        0, "<Item>" },
  { "/_Options",      NULL,         NULL, 0, "<Branch>" },
  { "/Options/Show controls",  
         "v",         options_cb, 0, "<CheckItem>" },
  { "/Options/Show lines",  
         "",         options_cb, 1, "<CheckItem>" },
  { "/Options/Show points",  
         "" ,        options_cb, 2, "<CheckItem>" },
  { "/Options/Line thickness",  
         "" ,        NULL,           0, "<Branch>" },
  { "/Options/Line thickness/Thin",  
         "" ,        line_options_cb,   0, "<RadioItem>" },
  { "/Options/Line thickness/Medium",  
         "" ,        line_options_cb,   1, "/Options/Line thickness/Thin" },
  { "/Options/Line thickness/Thick",  
         "" ,        line_options_cb,   2, "/Options/Line thickness/Thin" },
  { "/Options/Bitmap size",  
         "" ,        NULL,           0, "<Branch>" },
  { "/Options/Bitmap size/Small",  
         "" ,        bitmap_size_cb, 0, "<RadioItem>" },
  { "/Options/Bitmap size/Medium", 
         "" ,        bitmap_size_cb, 1, "/Options/Bitmap size/Small" },
  { "/Options/Bitmap size/Large",  
         "" ,        bitmap_size_cb, 2, "/Options/Bitmap size/Small" },
  { "/Options/Replot frequency",  
         "" ,        NULL,           0, "<Branch>" },
  { "/Options/Replot frequency/1",  
         "" ,        replot_freq_cb, 1, "<RadioItem>" },
  { "/Options/Replot frequency/2",  
         "" ,        replot_freq_cb, 2, "/Options/Replot frequency/1" },
  { "/Options/Replot frequency/4",  
         "" ,        replot_freq_cb, 4, "/Options/Replot frequency/1" },
  { "/Options/Replot frequency/8",  
         "" ,        replot_freq_cb, 8, "/Options/Replot frequency/1" },
  { "/Options/Replot frequency/16",  
         "" ,        replot_freq_cb, 16,"/Options/Replot frequency/1" },
};

void
open_tour2dpp_popup () {
  GtkWidget *hbox, *vbox, *vbc, *vb, *frame, *btn, *tgl, *entry;
  GtkWidget *da, *label, *hb, *opt;

  if (window == NULL) {

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                        GTK_SIGNAL_FUNC (hide_cb), (gpointer) NULL);
    gtk_window_set_title (GTK_WINDOW (window), "projection pursuit");
    gtk_window_set_policy (GTK_WINDOW (window), true, true, false);
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);

/*
 * Add the main menu bar
*/
    vbox = gtk_vbox_new (FALSE, 1);
    gtk_container_border_width (GTK_CONTAINER (vbox), 1);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    pp_accel_group = gtk_accel_group_new ();
    get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                   pp_accel_group, window, &mbar, (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);

/*
 * Divide the window:  controls on the left, plot on the right
*/
    hbox = gtk_hbox_new (false, 1);
    gtk_container_border_width (GTK_CONTAINER (hbox), 1);
    gtk_box_pack_start (GTK_BOX (vbox),
                        hbox, true, true, 1);

/*
 * Controls
*/
    control_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (control_frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (control_frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox),
                        control_frame, false, false, 1);

    vbc = gtk_vbox_new (false, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbc), 5);
    gtk_container_add (GTK_CONTAINER (control_frame), vbc);

/*
 * Optimize toggle
*/
    tgl = gtk_check_button_new_with_label ("Optimize");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Guide the tour using projection pursuit optimization or tour passively",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                        GTK_SIGNAL_FUNC (optimize_cb), (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vbc),
                      tgl, false, false, 1);

/*
 * Index value with label
*/
    hb = gtk_hbox_new (false, 3);
    gtk_box_pack_start (GTK_BOX (vbc), hb, false, false, 2);

    label = gtk_label_new ("PP index:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (hb), label, false, false, 0);

    entry = gtk_entry_new_with_max_length (32);
    gtk_entry_set_editable (GTK_ENTRY (entry), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), entry,
      "The value of the projection pursuit index for the current projection",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);

/*
 * pp index menu and scale inside frame
*/
    frame = gtk_frame_new ("PP index function");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbc), frame, false, false, 0);

    vb = gtk_vbox_new (false, 3);
    gtk_container_add (GTK_CONTAINER (frame), vb);


    opt = gtk_option_menu_new ();
    gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
      "Set the projection pursuit index", NULL);
    gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
    populate_option_menu (opt, func_lbl,
                          sizeof (func_lbl) / sizeof (gchar *),
                          func_cb);

    param_vb = gtk_vbox_new (false, 3);
    gtk_container_set_border_width (GTK_CONTAINER (param_vb), 4);
    gtk_box_pack_start (GTK_BOX (vb), param_vb, false, false, 2);

    param_lbl = gtk_label_new ("Terms in expansion:");
    gtk_misc_set_alignment (GTK_MISC (param_lbl), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (param_vb), param_lbl, false, false, 0);

    param_adj = (GtkAdjustment *) gtk_adjustment_new (1.0,
                                                      1.0, 30.0,
                                                      1.0, 1.0, 0.0);
    param_scale = gtk_hscale_new (GTK_ADJUSTMENT (param_adj));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), param_scale,
      "Set number of terms in the expansion for some indices; bandwidth for others", NULL);
    gtk_range_set_update_policy (GTK_RANGE (param_scale),
                                 GTK_UPDATE_CONTINUOUS);
    gtk_scale_set_digits (GTK_SCALE (param_scale), 0);
    gtk_scale_set_value_pos (GTK_SCALE (param_scale), GTK_POS_BOTTOM);

    gtk_box_pack_start (GTK_BOX (param_vb), param_scale, true, true, 0);

/*
 * Frame containing bitmap operations
*/

    frame = gtk_frame_new ("Bitmaps");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbc), frame, false, false, 0);

    vb = gtk_vbox_new (false, 3);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
    gtk_container_add (GTK_CONTAINER (frame), vb);

/*
 * New bitmap button
*/
    btn = gtk_button_new_with_label ("New bitmap");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
      "Add a bitmap below the plot containing the current scatterplot projection", NULL);
    gtk_box_pack_start (GTK_BOX (vb), btn, false, false, 1);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (bitmap_cb), NULL);

/*
 * Return to bitmap toggle
*/
    tgl = gtk_check_button_new_with_label ("Return to bitmap");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Click left on a bitmap to force the tour to return to this view and pause",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                        GTK_SIGNAL_FUNC (return_to_bitmap_cb),
                        (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vb), tgl, false, false, 1);

/*
 * Record bitmap toggle
*/
    tgl = gtk_check_button_new_with_label ("Record bitmap");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), tgl,
      "Click left on a bitmap to save it in a file, in tour history format",
      NULL);
    gtk_signal_connect (GTK_OBJECT (tgl), "toggled",
                        GTK_SIGNAL_FUNC (record_bitmap_cb),
                        (gpointer) NULL);
    gtk_box_pack_start (GTK_BOX (vb),
                        tgl, false, false, 1);
/*
 * Drawing area in a frame
*/
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox),
                        frame, true, true, 1);

    da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (da), WIDTH, HEIGHT);
    gtk_container_add (GTK_CONTAINER (frame), da);
  }

  gtk_widget_show_all (window);
}
