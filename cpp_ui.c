/* pp_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   600
#define HEIGHT  300

static GtkWidget *window = NULL;
static GtkWidget *control_frame;
static GtkWidget *mbar;
static GtkAccelGroup *cpp_accel_group;

static void
hide_cb (GtkWidget *w ) {
  gtk_widget_hide (window);
}

static void
optimize_cb (GtkToggleButton  *w) {
  g_printerr ("optimize?  %d\n", w->active);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,         NULL, 0, "<Branch>" },
  { "/File/Close",  
         "",         (GtkItemFactoryCallback) hide_cb,        0, "<Item>" },
};

void
ctourpp_window_open (ggobid *gg) {
  GtkWidget *hbox, *vbox, *vbc, *frame, *tgl, *entry;
  GtkWidget *da, *label, *hb;

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

    cpp_accel_group = gtk_accel_group_new ();
    get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                   cpp_accel_group, window, &mbar, (gpointer) window);
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
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), tgl,
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
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), entry,
      "The value of the projection pursuit index for the current projection",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), entry, false, false, 2);

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
