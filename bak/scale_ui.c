/* scale_ui.c */

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"

/* external functions */
extern void splot_set_plot_center (splotd *);
extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                  GtkSignalFunc);
/*                    */

static icoords mousepos;

void
reset_scale_cb (GtkWidget *w, gpointer cbd) {
  gchar *lbl = (gchar *) cbd;
  g_printerr ("reset scale: %s\n", lbl);
}

void
toggle_keys_cb (GtkToggleButton *w) {
/*
 * This is connected to the pan button
*/
  xg.pan_or_zoom = (w->active) ? PAN : ZOOM;
  g_printerr ("in toggle_keys: %s\n",
    (xg.pan_or_zoom == PAN) ? "PAN" : "ZOOM");
}
static void preserve_aspect_cb (GtkToggleButton *w)
{
  g_printerr ("in preserve_aspect: %d\n", w->active);
}


static gchar *view_stdize_lbl[] = {"Min/Max",
                                   "Mean/Lgst dist",
                                   "Med/Lgst dist"};
static void view_stdize_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", view_stdize_lbl[indx]);
}

/**********************************************************************/
/****** Handling keyboard and mouse events in the plot window *********/
/**********************************************************************/

static gint
motion_notify_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  g_printerr ("(sc_motion_notify_cb)\n");

  return true;
}

static void pan_step (gint keyval)
{
/*
 * panning uses re-centering:  the plot is re-centered at mousepos.
 * The left/right arrows will make the plot shift only horizontally,
 *   and only one will work for each mousepos; ditto the up/down
*/
  splotd *sp = current_splot;

  gint shift_x = mousepos.x - sp->mid.x;
  gint shift_y = mousepos.y - sp->mid.y;

  switch (keyval) {
    case GDK_Up:
      if (shift_y < 0) g_printerr ("pan up %d pixels \n", shift_y);
      break;
    case GDK_Down:
      if (shift_y > 0) g_printerr ("pan down %d pixels \n", shift_y);
      break;

    case GDK_Left:
      if (shift_x < 0) g_printerr ("pan left %d pixels \n", shift_x);
      break;
    case GDK_Right:
      if (shift_x > 0) g_printerr ("pan right %d pixels \n", shift_x);
      break;

    case GDK_plus:
      g_printerr ("plus key pressed; ignore\n");
      break;
    case GDK_minus:
      g_printerr ("minus key pressed; ignore\n");
      break;
    case GDK_space:
      g_printerr ("space key pressed: pan %d horiz and %d vert\n",
        shift_x, shift_y);
      break;
    default:
      break;
  }
}

static void zoom_step (gint keyval)
{
/*
 * zoom uses re-bordering  (We could use rebordering for panning,
 * too, if this turns out to be confusing.)
*/

  /*
   * For the moment, assume that I can get hold of the
   * current scatterplot this way.
  */
  splotd *sp = current_splot;

  fcoords fs;
  gfloat scale_x;
  gfloat d1, d2;
  lcoords planar;
  gulong fac;

  switch (keyval) {
    case GDK_Up:
      break;
    case GDK_Down:
      break;

    case GDK_Left:
      break;
    case GDK_Right:

      splot_set_plot_center (sp);
      planar.x = (mousepos.x - sp->cntr.x) * PRECISION1 / sp->is.x ;

      d1 = (gfloat) (sp->max.x - sp->mid.x);
      d2 = (gfloat) (mousepos.x - sp->mid.x);

      fac = (gfloat) (planar.x + sp->shift_wrld.x) /
            (gfloat) (PRECISION1 + sp->shift_wrld.x);

      fs.x = d1/d2 * fac * (gfloat) sp->is.x;

      scale_x = 2 * fs.x / (gfloat) sp->max.x ;

      g_printerr ("is %ld, scale %f\n",     sp->is.x, sp->scale.x);
      g_printerr ("fs %f, new scale %f\n", fs.x,       scale_x);
      
      break;

    case GDK_plus:
      g_printerr ("plus key pressed; ignore\n");
      break;
    case GDK_minus:
      g_printerr ("minus key pressed; ignore\n");
      break;
    case GDK_space:
      g_printerr ("space key pressed\n");
      break;
    default:
      break;
  }
}

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  if (xg.pan_or_zoom == PAN)
    pan_step (event->keyval);
  else
    zoom_step (event->keyval);

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  current_splot = sp;
  current_display = (displayd *) sp->displayptr;

  mousepos.x = event->x;
  mousepos.y = event->y;

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
                                      (gpointer) sp);

  return retval;
}
static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  mousepos.x = event->x;
  mousepos.y = event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  return retval;
}

void
splot_toggle_scale_handlers (splotd *sp, gpointer stateptr) {
  displayd *display = (displayd *) sp->displayptr;
  gboolean state = GPOINTER_TO_INT (stateptr);

  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                       "button_press_event",
                                       (GtkSignalFunc) button_press_cb,
                                       (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                         "button_release_event",
                                         (GtkSignalFunc) button_release_cb,
                                         (gpointer) sp);
  } else {
    gtk_signal_disconnect (GTK_OBJECT (display->window), sp->key_press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

void
display_toggle_scale_handlers (gboolean state) {
  if (current_display != null && current_display->splots != null)
    g_list_foreach (current_display->splots,
                    (GFunc) splot_toggle_scale_handlers,
                    GINT_TO_POINTER (state));
}


/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/

GtkWidget *scale_reset_menu;


void
make_scale_menus () {
  GtkWidget *item;

/*
 * Reset menu
*/
  scale_reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Reset shift");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (reset_scale_cb),
                      (gpointer) "shift");
  gtk_menu_append (GTK_MENU (scale_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset scale");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (reset_scale_cb),
                      (gpointer) "scale");
  gtk_menu_append (GTK_MENU (scale_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset aspect");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (reset_scale_cb),
                      (gpointer) "aspect");
  gtk_menu_append (GTK_MENU (scale_reset_menu), item);

  gtk_widget_show_all (scale_reset_menu);
}

/**********************************************************************/
/******************* End of main menubar section **********************/
/**********************************************************************/

void
cpanel_scale_make () {
  GtkWidget *frame, *vb, *btn, *hb, *lbl;
  GtkWidget *radio1, *radio2;
  GtkWidget *opt;
  GSList *group;

  control_panel[SCALE] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[SCALE]), 5);

  
/*
 * frame for arrow keys
*/
  frame = gtk_frame_new ("Arrow keys");
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (control_panel[SCALE]), frame, false, false, 0);

  vb = gtk_vbox_new (true, 1);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 3);
  gtk_container_add (GTK_CONTAINER (frame), vb);

  radio1 = gtk_radio_button_new_with_label (NULL, "Pan");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio1), TRUE);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), radio1,
    "First click in the plot window where you want the new center to be, then use the arrow keys or spacebar to pan", NULL);
  gtk_signal_connect (GTK_OBJECT (radio1), "toggled",
                      GTK_SIGNAL_FUNC (toggle_keys_cb), NULL);

  gtk_box_pack_start (GTK_BOX (vb), radio1, TRUE, TRUE, 0);

  group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));

  radio2 = gtk_radio_button_new_with_label (group, "Zoom");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), radio2,
    "First click in the plot window, using your distance from the border to indicate how much you want to zoom, then use the arrow keys, +, or -", NULL);
  gtk_box_pack_start (GTK_BOX (vb), radio2, TRUE, TRUE, 0);

/*
 * Toggle button
*/
  btn = gtk_check_button_new_with_label ("Fix aspect");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Preserve aspect ratio", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                     GTK_SIGNAL_FUNC (preserve_aspect_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[SCALE]), btn,
    false, false, 3);

/*
 * view standardization option menu with label
*/

  hb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[SCALE]), hb, false, false, 0);

  lbl = gtk_label_new ("Standardize view:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 1);
  gtk_box_pack_start (GTK_BOX (hb), lbl, false, false, 0);

  /*
   * option menu
  */
  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "View standardization: Minimum/Maximum, Mean/Largest distance, or Median/Largest distance", NULL);
  gtk_box_pack_end (GTK_BOX (hb), opt, false, false, 0);
  populate_option_menu (opt, view_stdize_lbl,
                        sizeof (view_stdize_lbl) / sizeof (gchar *),
                        view_stdize_cb);

  gtk_widget_show_all (control_panel[SCALE]);
}

