/* brush_ui.c */
/*
 * Code pertaining to the control panel for brushing.
*/

#include <gtk/gtk.h>
#include <strings.h>

#include "vars.h"
#include "externs.h"

static GtkWidget *mode_opt;
static GtkWidget *cg_opt;
static GtkWidget *scope_opt;
static GtkWidget *brush_on_btn;

extern brush_coords brush_pos;  /* from brush.c */

static void brush_on_cb (GtkToggleButton *button)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->brush_on_p = button->active;
}

static gchar *scope_lbl[] = {"Points", "Lines", "Points and lines"};
static void brush_scope_cb (GtkWidget *w, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->br_scope = GPOINTER_TO_INT (cbd);
}

static gchar *cg_lbl[] =
  {"Color and glyph", "Color only", "Glyph only"};
static void brush_cg_cb (GtkWidget *w, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->br_cg = GPOINTER_TO_INT (cbd);
}

static gchar *mode_lbl[] = {"Persistent", "Transient", "Undo"};
static void brush_mode_cb (GtkWidget *w, gpointer cbd)
{
  cpaneld *cpanel = &current_display->cpanel;
  cpanel->br_mode = GPOINTER_TO_INT (cbd);
}

static void open_symbol_window_cb (GtkWidget *w) {
  make_symbol_window ();
}

/*
 * Callbacks for menus in the main menubar
*/

/* Actions from the Reset menu in the main menubar */
static void
brush_reset_cb (GtkWidget *w, gpointer cbd)
{
  gchar *lbl = (gchar *) cbd;
  g_printerr ("cbd: %s\n", lbl);
}

/* Actions from the Link menu in the main menubar */
static void
brush_link_cb (GtkCheckMenuItem *w, gpointer cbd)
{
  gchar *lbl = (gchar *) cbd;
  g_printerr ("state: %d, cbd: %s\n", w->active, lbl);
}

/* Options from the Display options menu in the main menubar */
void
brush_options_cb (gpointer data, guint action, GtkCheckMenuItem *w)
{
/*
 * action 0 : Brush jumps to cursor
 * action 1 : Update linked brushing continuously
*/
  g_printerr ("action: %d, state: %d\n", action,  w->active);
}

/*--------------------------------------------------------------------*/
/*                         Mouse events                               */
/*--------------------------------------------------------------------*/

static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, cpaneld *cpanel)
{
  gboolean button1_p, button2_p;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get (w, event, &button1_p, &button2_p);

  if (button1_p || button2_p)
    brush_motion (&mousepos, button1_p, button2_p, cpanel);

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  cpaneld *cpanel;
  gboolean retval = true;

  current_splot = sp;
  current_display = (displayd *) sp->displayptr;
  cpanel = &current_display->cpanel;

  mousepos.x = event->x;
  mousepos.y = event->y;

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                     "motion_notify_event",
                                     (GtkSignalFunc) motion_notify_cb,
                                     (gpointer) cpanel);

  return retval;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  mousepos.x = event->x;
  mousepos.y = event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  /*
   * We're redrawing everything on button release; we have no
   * way of knowing at this point whether things changed or
   * not, since that information is not accumulated.
  if (xg.brush_on) {
    if (xg.jump_brush || xg.is_line_painting) {
      plot_once(xg);
    }
  }
  */

  return retval;
}

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/

void
brush_event_handlers_toggle (splotd *sp, gboolean state) {

  if (state == on) {
    sp->press_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "button_press_event",
                                      (GtkSignalFunc) button_press_cb,
                                      (gpointer) sp);
    sp->release_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                        "button_release_event",
                                        (GtkSignalFunc) button_release_cb,
                                        (gpointer) sp);
  } else {
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->press_id);
    gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->release_id);
  }
}

GtkWidget *brush_reset_menu, *brush_link_menu;
void
brush_menus_make () {
  GtkWidget *item;

/*
 * Reset menu
*/
  brush_reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Reset point colors");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "pointcolors");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset brush size");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "brushsize");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset linecolors");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "linecolors");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset glyphs");
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) "glyphs");
  gtk_menu_append (GTK_MENU (brush_reset_menu), item);

  gtk_widget_show_all (brush_reset_menu);

/*
 * Link menu
*/
  brush_link_menu = gtk_menu_new ();

  item = gtk_check_menu_item_new_with_label ("Link points <-> points");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "p2p");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link lines <-> lines");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "l2l");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link points <-> lines");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "p2l");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link color brushing");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "color");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);
  item = gtk_check_menu_item_new_with_label ("Link glyph brushing");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "glyph");
  gtk_menu_append (GTK_MENU (brush_link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  gtk_widget_show_all (brush_link_menu);
}

void
cpanel_brush_make () {
  GtkWidget *btn;
  
  control_panel[BRUSH] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[BRUSH]), 5);

  brush_on_btn = gtk_check_button_new_with_label ("Brush on");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), brush_on_btn,
    "Make the brush active or inactive", NULL);
  gtk_signal_connect (GTK_OBJECT (brush_on_btn), "toggled",
                     GTK_SIGNAL_FUNC (brush_on_cb), (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      brush_on_btn, false, false, 0);

/*
 * make an option menu for setting the brushing mode
*/
  scope_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), scope_opt,
    "Brush points only, lines only, or both points and lines", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      scope_opt, false, false, 0);
  populate_option_menu (scope_opt, scope_lbl,
                        sizeof (scope_lbl) / sizeof (gchar *),
                        brush_scope_cb);
  /* points only */
  gtk_option_menu_set_history (GTK_OPTION_MENU (scope_opt), 0); 
  
/*
 * option menu for specifying whether to brush with color/glyph/both
*/
  cg_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), cg_opt,
    "Brush with color and glyph, color only, or glyph only", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      cg_opt, false, false, 0);
  populate_option_menu (cg_opt, cg_lbl,
                        sizeof (cg_lbl) / sizeof (gchar *),
                        brush_cg_cb);
  gtk_option_menu_set_history (GTK_OPTION_MENU (cg_opt), 0);  /* both */

/*
 * option menu for setting the brushing persistence
*/
  mode_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), mode_opt,
    "Persistent, transient or undo brushing", NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      mode_opt, false, false, 0);
  populate_option_menu (mode_opt, mode_lbl,
                        sizeof (mode_lbl) / sizeof (gchar *),
                        brush_mode_cb);
  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_opt), 1);  /* transient */

/*
 * button for opening symbol panel
*/
  btn = gtk_button_new_with_label ("Choose symbol ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), btn,
    "Open panel for choosing color and glyph", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_symbol_window_cb),
                      (gpointer) NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      btn, false, false, 1);

/*
 * button for opening hide/exclude panel
*/
  btn = gtk_button_new_with_label ("Hide or exclude ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
                        btn,
                        "Open panel for hiding or excluding brushed groups",
                        NULL);
  gtk_box_pack_start (GTK_BOX (control_panel[BRUSH]),
                      btn, false, false, 1);

  gtk_widget_show_all (control_panel[BRUSH]);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_brush_init (cpaneld *cpanel) {
  cpanel->brush_on_p = true;

  cpanel->br_mode = BR_TRANSIENT;
  cpanel->br_cg = BR_CANDG;  /* color and glyph */
  cpanel->br_scope = BR_POINTS;
}

void
cpanel_brush_set (cpaneld *cpanel) {
  GTK_TOGGLE_BUTTON (brush_on_btn)->active = cpanel->brush_on_p;

  gtk_option_menu_set_history (GTK_OPTION_MENU (mode_opt),
                               cpanel->br_mode);
  gtk_option_menu_set_history (GTK_OPTION_MENU (scope_opt),
                               cpanel->br_scope);
  gtk_option_menu_set_history (GTK_OPTION_MENU (cg_opt),
                               cpanel->br_cg);
}

