/* brush_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
/*
 * Code pertaining to the control panel for brushing.
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H		    
#include <strings.h>
#endif

#include "vars.h"
#include "externs.h"

static void brush_on_cb (GtkToggleButton *button, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->brush_on_p = button->active;
}

static void brush_undo_cb (GtkToggleButton *button, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  datad *d = gg->current_display->d;

  if (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL)
    point_brush_undo (gg->current_splot, d, gg);
  if (cpanel->br_scope == BR_LINES || cpanel->br_scope == BR_PANDL)
    line_brush_undo (gg->current_splot, gg);
}

void
brush_scope_set (gint br_scope, datad *d, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;

  cpanel->br_scope = br_scope;
  splot_redraw (sp, QUICK, gg);  
}

static gchar *scope_lbl[] = {"Points", "Lines", "Points and lines"};
static void brush_scope_set_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  datad *d = gg->current_display->d;
  brush_scope_set (GPOINTER_TO_INT (cbd), d, gg);
}

static gchar *cg_lbl[] =
  {"Color and glyph", "Color only", "Glyph only", "Glyph size only", "Hide"};
static void brush_cg_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->br_target = GPOINTER_TO_INT (cbd);
}

static gchar *mode_lbl[] = {"Persistent", "Transient"};
static void brush_mode_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  cpanel->br_mode = GPOINTER_TO_INT (cbd);
}

static void open_symbol_window_cb (GtkWidget *w, ggobid *gg) 
{
  make_symbol_window (gg);
}

static void exclusion_window_cb (GtkToggleButton *button, ggobid *gg)
{
  exclusion_window_open (gg);
}


/*
 * Callbacks for menus in the main menubar
*/

/* Actions from the Reset menu in the main menubar */
static void
brush_reset_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint action = GPOINTER_TO_INT (cbd);
  gint m, i, k;
  datad *d = gg->current_display->d;

  switch (action) {
    case 0:  /*-- un-hide all points --*/
      for (m=0; m<d->nrows_in_plot; m++) {
        i = d->rows_in_plot[m];
        d->hidden.els[i] = d->hidden_now.els[i] = false;
      }
      displays_plot (NULL, FULL, gg);
      break;
    case 1:  /*-- reset point colors -- to what? --*/
      break;
    case 2:  /*-- reset point glyphs -- to what? --*/
      break;

    case 3:  /*-- un-hide all lines --*/
      for (k=0; k<gg->nedges; k++) {
        gg->line.hidden_now.els[k] = gg->line.hidden.els[k] = false;
      }
      displays_plot (NULL, FULL, gg);
      break;
    case 4:  /*-- reset line colors -- to what? --*/
      break;

    case 5:  /*-- reset brush size --*/
      brush_pos_init (d);
      splot_redraw (gg->current_splot, QUICK, gg);
      break;
  }
}

/* Actions from the Link menu in the main menubar */
static void
brush_link_cb (GtkCheckMenuItem *w, gpointer cbd)
{
  gchar *lbl = (gchar *) cbd;
  g_printerr ("state: %d, cbd: %s\n", w->active, lbl);
}

/* Options from the Options menu in the main menubar */
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
  ggobid *gg = GGobiFromWidget (w, true);
  splotd *sp = gg->current_splot;
  displayd *display = gg->current_display;
  datad *d = display->d;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  if (button1_p || button2_p)
    brush_motion (&sp->mousepos, button1_p, button2_p, cpanel, d, gg);

  return true;
}

/*-- response to the mouse click event --*/
static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  cpaneld *cpanel;
  gboolean retval = true;
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot(sp);
  datad *d;


  gg->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;
  cpanel = &gg->current_display->cpanel;
  d = gg->current_display->d;

  point_brush_prev_vectors_update (d, gg);
  line_brush_prev_vectors_update (gg);

  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                     "motion_notify_event",
                                     (GtkSignalFunc) motion_notify_cb,
                                     (gpointer) cpanel);

  GGobi_widget_set(sp->da, gg, true);

  brush_set_pos ((gint) event->x, (gint) event->y, d, gg);

  brush_motion (&sp->mousepos, button1_p, button2_p, cpanel, d, gg);

  return retval;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;

  sp->mousepos.x = (gint) event->x;
  sp->mousepos.y = (gint) event->y;

  gtk_signal_disconnect (GTK_OBJECT (sp->da), sp->motion_id);

  /*
   * We're redrawing everything on button release; we have no
   * way of knowing at this point whether things changed or
   * not, since that information is not accumulated.
  if (gg->brush_on) {
    if (gg->jump_brush || gg->is_line_painting) {
      plot_once(gg);
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


void
brush_menus_make (ggobid *gg) {
  GtkWidget *item;

/*
 * Reset menu
*/
  gg->brush.reset_menu = gtk_menu_new ();

  item = gtk_menu_item_new_with_label ("Show all points");
  gtk_object_set_data (GTK_OBJECT (item), "GGobi", (gpointer) gg);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (0));
  gtk_menu_append (GTK_MENU (gg->brush.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset point colors");
  gtk_object_set_data (GTK_OBJECT (item), "GGobi", (gpointer) gg);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (1));
  gtk_menu_append (GTK_MENU (gg->brush.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset glyphs");
  gtk_object_set_data (GTK_OBJECT (item), "GGobi", (gpointer) gg);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (2));
  gtk_menu_append (GTK_MENU (gg->brush.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Show all lines");
  gtk_object_set_data (GTK_OBJECT (item), "GGobi", (gpointer) gg);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (3));
  gtk_menu_append (GTK_MENU (gg->brush.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset linecolors");
  gtk_object_set_data (GTK_OBJECT (item), "GGobi", (gpointer) gg);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER(4));
  gtk_menu_append (GTK_MENU (gg->brush.reset_menu), item);

  item = gtk_menu_item_new_with_label ("Reset brush size");
  gtk_object_set_data (GTK_OBJECT (item), "GGobi", (gpointer) gg);
  gtk_signal_connect (GTK_OBJECT (item), "activate",
                      GTK_SIGNAL_FUNC (brush_reset_cb),
                      (gpointer) GINT_TO_POINTER (5));
  gtk_menu_append (GTK_MENU (gg->brush.reset_menu), item);

  gtk_widget_show_all (gg->brush.reset_menu);

/*
 * Link menu
*/
  gg->brush.link_menu = gtk_menu_new ();

  item = gtk_check_menu_item_new_with_label ("Link points <-> points");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "p2p");
  gtk_menu_append (GTK_MENU (gg->brush.link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link lines <-> lines");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "l2l");
  gtk_menu_append (GTK_MENU (gg->brush.link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link points <-> lines");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "p2l");
  gtk_menu_append (GTK_MENU (gg->brush.link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  item = gtk_check_menu_item_new_with_label ("Link color brushing");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "color");
  gtk_menu_append (GTK_MENU (gg->brush.link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);
  item = gtk_check_menu_item_new_with_label ("Link glyph brushing");
  gtk_signal_connect (GTK_OBJECT (item), "toggled",
                      GTK_SIGNAL_FUNC (brush_link_cb),
                      (gpointer) "glyph");
  gtk_menu_append (GTK_MENU (gg->brush.link_menu), item);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (item), true);

  gtk_widget_show_all (gg->brush.link_menu);
}

void
cpanel_brush_make (ggobid *gg) {
  GtkWidget *btn;
  
  gg->control_panel[BRUSH] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[BRUSH]), 5);

  gg->brush.brush_on_btn = gtk_check_button_new_with_label ("Brush on");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->brush.brush_on_btn,
    "Make the brush active or inactive", NULL);
  gtk_signal_connect (GTK_OBJECT (gg->brush.brush_on_btn), "toggled",
                     GTK_SIGNAL_FUNC (brush_on_cb), (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      gg->brush.brush_on_btn, false, false, 0);

/*
 * make an option menu for setting the brushing mode
*/
  gg->brush.scope_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->brush.scope_opt,
    "Brush points only, lines only, or both points and lines", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      gg->brush.scope_opt, false, false, 0);
  populate_option_menu (gg->brush.scope_opt, scope_lbl,
                        sizeof (scope_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) brush_scope_set_cb, gg);
  /*-- initial value: points only --*/
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->brush.scope_opt), 0); 
  
/*
 * option menu for specifying whether to brush with color/glyph/both
*/
  gg->brush.cg_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->brush.cg_opt,
    "Brush with color and glyph, color, glyph, glyph size; or hide", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      gg->brush.cg_opt, false, false, 0);
  populate_option_menu (gg->brush.cg_opt, cg_lbl,
                        sizeof (cg_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) brush_cg_cb, gg);
  /*-- initial value: both --*/
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->brush.cg_opt), 0);

/*
 * option menu for setting the brushing persistence
*/
  gg->brush.mode_opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->brush.mode_opt,
    "Persistent or transient brushing", NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      gg->brush.mode_opt, false, false, 0);
  populate_option_menu (gg->brush.mode_opt, mode_lbl,
                        sizeof (mode_lbl) / sizeof (gchar *),
                        (GtkSignalFunc) brush_mode_cb, gg);
  /* initialize transient */
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->brush.mode_opt), 1);


  btn = gtk_button_new_with_label ("Undo");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Undo the most recent brushing changes, from button down to button up",
    NULL);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      btn, false, false, 0);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (brush_undo_cb), gg);

/*
 * button for opening symbol panel
*/
  btn = gtk_button_new_with_label ("Choose symbol ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
    "Open panel for choosing color and glyph", NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (open_symbol_window_cb),
                      (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      btn, false, false, 1);

/*
 * button for opening hide/exclude panel
*/
  btn = gtk_button_new_with_label ("Hide or exclude ...");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
                        btn,
                        "Open panel for hiding or excluding brushed groups",
                        NULL);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (exclusion_window_cb),
                      (gpointer) gg);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BRUSH]),
                      btn, false, false, 1);

  gtk_widget_show_all (gg->control_panel[BRUSH]);
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_brush_init (cpaneld *cpanel, ggobid *gg) {
  cpanel->brush_on_p = true;

  cpanel->br_mode = BR_TRANSIENT;
  cpanel->br_target = BR_CANDG;  /* color and glyph */
  cpanel->br_scope = BR_POINTS;
}

void
cpanel_brush_set (cpaneld *cpanel, ggobid *gg) {
  GTK_TOGGLE_BUTTON (gg->brush.brush_on_btn)->active = cpanel->brush_on_p;

  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->brush.mode_opt),
                               cpanel->br_mode);
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->brush.scope_opt),
                               cpanel->br_scope);
  gtk_option_menu_set_history (GTK_OPTION_MENU (gg->brush.cg_opt),
                               cpanel->br_target);
}

