/* barchart_ui.c */
#ifdef BARCHART_IMPLEMENTED
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "vars.h"
#include "externs.h"

static gchar *display_mode_lbl[] = { "Barchart", "Spineplot" };

static gboolean barchart_scale(gboolean button1_p, gboolean button2_p,
                               splotd * sp);

void barchart_set_initials(splotd * sp, datad * d);
void barchart_allocate_structure(splotd * sp, datad * d);
extern void barchart_set_breakpoints(gfloat width, splotd * sp, datad * d);

static void display_mode_cb(GtkWidget * w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  barchartSPlotd *sp = GTK_GGOBI_BARCHART_SPLOT(gg->current_splot);
  displayd *display = gg->current_display;

  cpanel->barchart_display_mode = GPOINTER_TO_INT(cbd);
  sp->bar->is_spine = !sp->bar->is_spine;
  scatterplot_show_vrule(display, !sp->bar->is_spine);
}

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

GtkWidget *cpanel_barchart_make(ggobid * gg)
{
  GtkWidget *vb, *lbl, *opt;
  GtkWidget *panel;

  panel = gtk_vbox_new(false, VBOX_SPACING);
  gtk_container_set_border_width(GTK_CONTAINER(panel), 5);


/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new(false, 0);
  gtk_box_pack_start(GTK_BOX(panel), vb, false, false, 0);

  lbl = gtk_label_new("Display mode:");
  gtk_misc_set_alignment(GTK_MISC(lbl), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(vb), lbl, false, false, 0);

  opt = gtk_option_menu_new();
  gtk_widget_set_name(opt, "BARCHART:displ_mode_option_menu");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), opt,
                       "some sensible text about spine plots and barcharts",
                       NULL);
  gtk_box_pack_start(GTK_BOX(vb), opt, false, false, 0);
  populate_option_menu(opt, display_mode_lbl,
                       sizeof(display_mode_lbl) / sizeof(gchar *),
                       (GtkSignalFunc) display_mode_cb, gg);

  gtk_widget_show_all(panel);

  return (panel);
}

/*--------------------------------------------------------------------*/
/*                   Resetting the main menubar                       */
/*--------------------------------------------------------------------*/



/*
  The useIds indicates whether the callback data should be integers
  identifying the menu item or the global gg.
  At present, this is always false.
  See scatmat_mode_menu_make and scatterplot_mode_menu_make.
 */
GtkWidget *barchart_mode_menu_make(GtkAccelGroup * accel_group,
                                   GtkSignalFunc func,
                                   ggobid * gg, gboolean useIds)
{
  GtkWidget *menu;
  /* menu used to be in gg->barchart.mode_menu. But that was never used
     except for a way to return the value from here. */
  menu = gtk_menu_new();

  CreateMenuItem(menu, "Barchart",
                 "^h", "", NULL, accel_group, func,
                 useIds ? GINT_TO_POINTER(EXTENDED_DISPLAY_MODE) : gg, gg);

  /* Add a separator */
  CreateMenuItem(menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem(menu, "Scale",
                 "^s", "", NULL, accel_group, func,
                 useIds ? GINT_TO_POINTER(SCALE) : gg, gg);
  CreateMenuItem(menu, "Brush",
                 "^b", "", NULL, accel_group, func,
                 useIds ? GINT_TO_POINTER(BRUSH) : gg, gg);
  CreateMenuItem(menu, "Identify",
                 "^i", "", NULL, accel_group, func,
                 useIds ? GINT_TO_POINTER(IDENT) : gg, gg);

  gtk_widget_show(menu);

  return (menu);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists barchart_cpanel_init --*/

void cpanel_barchart_set(cpaneld * cpanel, GtkWidget * panel, ggobid * gg)
{
  GtkWidget *w;

  w = widget_find_by_name(panel, "BARCHART:displ_mode_option_menu");

  gtk_option_menu_set_history(GTK_OPTION_MENU(w),
                              cpanel->barchart_display_mode);
}


/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint key_press_cb(GtkWidget * w, GdkEventKey * event, splotd * sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean reallocate = FALSE;

  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  vartabled *vtx;

  barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);

  vtx = vartable_element_get(sp->p1dvar, d);

/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled(w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/
  switch (event->keyval) {
  case GDK_plus:
    if (!vtx->categorical_p) {
      bsp->bar->new_nbins = bsp->bar->nbins + 1;
      reallocate = TRUE;
    }
    break;

  case GDK_minus:
    if (!vtx->categorical_p) {
      if (bsp->bar->nbins > 2) {
        bsp->bar->new_nbins = bsp->bar->nbins - 1;
        reallocate = TRUE;
      }
    }
    break;
  }

  if (reallocate) {
    displayd *display = (displayd *) sp->displayptr;
    datad *d = display->d;
    ggobid *gg = GGobiFromSPlot(sp);

    barchart_allocate_structure(sp, d);
    barchart_set_initials(sp, d);
    barchart_recalc_counts(GTK_GGOBI_BARCHART_SPLOT(sp), d, gg);
    sp->redraw_style = FULL;
    splot_redraw(sp, sp->redraw_style, gg);
  }


  return true;
}

static gint
button_release_cb(GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);
  GdkModifierType state;

  gg->buttondown = 0;

  gdk_window_get_pointer(w->window, &sp->mousepos.x, &sp->mousepos.y,
                         &state);

  gdk_pointer_ungrab(event->time);

  /*
   * When the mouse comes up, return to the default cursor.  If it's
   * actually still inside one of the triangular regions, it will
   * be restored to the special cursor as soon as the mouse moves.
   * If we want to be really precise, we can check the regions here, too.
   */
  splot_cursor_set((gint) NULL, sp);
  sp->cursor = NULL;

  return retval;
}

static gint
mouse_motion_notify_cb(GtkWidget * w, GdkEventMotion * event, splotd * sp)
{
  gboolean button1_p, button2_p;
  barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion(w, event, &button1_p, &button2_p, sp);

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x
      && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (bsp->bar->is_histogram) {
    GdkRegion *region;
    gboolean cursor_set = FALSE;

    region = gdk_region_polygon(bsp->bar->anchor_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in(region, sp->mousepos.x, sp->mousepos.y)) {
      splot_cursor_set(GDK_SPIDER, sp);
      cursor_set = TRUE;
    }
    gdk_region_destroy(region);

    region = gdk_region_polygon(bsp->bar->offset_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in(region, sp->mousepos.x, sp->mousepos.y)) {
      splot_cursor_set(GDK_UMBRELLA, sp);
      cursor_set = TRUE;
    }
    gdk_region_destroy(region);

    /* If all buttons are up and we're outside the triangular regions,
       restore the cursor to the default.
     */
    if (!button1_p && !button2_p && !cursor_set && sp->jcursor) {
      splot_cursor_set((gint) NULL, sp);
      sp->cursor = NULL;
    }
  }

  if (button1_p || button2_p) {
    barchart_scale(button1_p, button2_p, sp);
  }

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return true;
}

gboolean
barchart_scale(gboolean button1_p, gboolean button2_p, splotd * sp)
{
  displayd *display = sp->displayptr;
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &display->cpanel;
  barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);
  datad *d = display->d;

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x
      && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (bsp->bar->is_histogram &&
      (bsp->bar->anchor_drag || bsp->bar->width_drag)) {
    gint idy = sp->mousepos.y - sp->mousepos_o.y;
    greal dy = (greal) idy;
    fcoords pts1, pts2;

    if (bsp->bar->anchor_drag) {
      gfloat scale_y;
      icoords scr;

      if (idy != 0) {
        gboolean set_anchor = TRUE;
        gfloat offset_old = bsp->bar->offset;
        gint pmid_old = sp->pmid.y;

        scr.x = scr.y = 0;
        scale_y = sp->scale.y;
        scale_y /= 2;
        sp->iscale.y = (gfloat) sp->max.y * scale_y;

        splot_screen_to_tform(cpanel, sp, &scr, &pts1, gg);
        sp->pmid.y -= (dy * (greal)PRECISION1 / sp->iscale.y);
        splot_screen_to_tform(cpanel, sp, &scr, &pts2, gg);
        bsp->bar->offset += (pts1.y - pts2.y);

/* moving the anchor more than one binwidth up or down doesn't make sense */
        if (bsp->bar->offset > offset_old) {
          /* not too high */
          set_anchor =
              (bsp->bar->offset <
               (bsp->bar->breaks[1] - bsp->bar->breaks[0]));
        }
        if (bsp->bar->offset < offset_old) {
          /* and not too low */
          set_anchor =
              (bsp->bar->offset >
               -(bsp->bar->breaks[1] - bsp->bar->breaks[0]));
        }


        if (set_anchor) {
          barchart_recalc_counts(GTK_GGOBI_BARCHART_SPLOT(sp), d, gg);
          splot_redraw(sp, FULL, gg);
        } else {
          sp->pmid.y = pmid_old;
          bsp->bar->offset = offset_old;
        }
      }
    } else {                    /* if (bsp->bar->width_drag) */

      if (idy != 0) {
        gfloat width, oldwidth;

        splot_screen_to_tform(cpanel, sp, &sp->mousepos_o, &pts1, gg);
        splot_screen_to_tform(cpanel, sp, &sp->mousepos, &pts2, gg);

        oldwidth = bsp->bar->breaks[1] - bsp->bar->breaks[0];
        width = oldwidth - (pts1.y - pts2.y);
        if (width > 0.) {
          gboolean set_breaks = TRUE;
          gint pix_width =
              bsp->bar->bins[0].rect.y - bsp->bar->bins[1].rect.y;
          if (width > oldwidth) {
            set_breaks = bsp->bar->bins[0].rect.y > (sp->max.y / 2 + 1);        /* not too big */
          }
          if (width < oldwidth) {
            set_breaks = pix_width > 6; /* not too small */
          }
          if (set_breaks) {
            barchart_set_breakpoints(width, sp, d);
            barchart_recalc_counts(GTK_GGOBI_BARCHART_SPLOT(sp), d, gg);
            splot_redraw(sp, FULL, gg);
          }
        }
      }
    }
  } else {   /*-- we're not dragging the bars, only scaling --*/

    switch (cpanel->scale_style) {

    case DRAG:
      if (button1_p) {
        pan_by_drag(sp, gg);
      } else if (button2_p) {
        zoom_by_drag(sp, gg);
      }

        /*-- redisplay this plot --*/
      splot_plane_to_screen(display, &display->cpanel, sp, gg);
      ruler_ranges_set(false, gg->current_display, sp, gg);
      splot_redraw(sp, FULL, gg);
      break;

    case CLICK:
      splot_redraw(sp, QUICK, gg);
      break;

    }  /*-- end switch (scale_style) --*/
  }
  return true;
}

static gint
button_press_cb(GtkWidget * w, GdkEventButton * event, splotd * sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);
  gboolean button1_p, button2_p;
  GdkRegion *region;
  barchartSPlotd *bsp = GTK_GGOBI_BARCHART_SPLOT(sp);

  mousepos_get_pressed(w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;


  if (bsp->bar->is_histogram) {
    bsp->bar->anchor_drag = FALSE;
    bsp->bar->width_drag = FALSE;

    region = gdk_region_polygon(bsp->bar->anchor_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in(region, sp->mousepos.x, sp->mousepos.y)) {
      bsp->bar->anchor_drag = TRUE;

    }
    gdk_region_destroy(region);

    region = gdk_region_polygon(bsp->bar->offset_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in(region, sp->mousepos.x, sp->mousepos.y)) {
      bsp->bar->width_drag = TRUE;
    }
    gdk_region_destroy(region);
  }

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return retval;
}

void barchart_event_handlers_toggle(splotd * sp, gboolean state)
{
  displayd *display = (displayd *) sp->displayptr;

  if (!GTK_IS_GGOBI_WINDOW_DISPLAY(display))
    return;

  if (state == on) {
    GtkObject *winobj =
        GTK_OBJECT(GTK_GGOBI_WINDOW_DISPLAY(display)->window);
    sp->key_press_id =
        gtk_signal_connect(winobj, "key_press_event",
                           (GtkSignalFunc) key_press_cb, (gpointer) sp);

  } else {
    disconnect_key_press_signal(sp);
  }
}


void barchart_scale_event_handlers_toggle(splotd * sp, gboolean state)
{
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    GtkObject *winobj =
        GTK_OBJECT(GTK_GGOBI_WINDOW_DISPLAY(display)->window);
    if (GTK_IS_GGOBI_WINDOW_DISPLAY(display))
      sp->key_press_id = gtk_signal_connect(winobj,
                                            "key_press_event",
                                            (GtkSignalFunc) key_press_cb,
                                            (gpointer) sp);
    sp->press_id = gtk_signal_connect(GTK_OBJECT(sp->da),
                                      "button_press_event",
                                      (GtkSignalFunc) button_press_cb,
                                      (gpointer) sp);
    sp->release_id = gtk_signal_connect(GTK_OBJECT(sp->da),
                                        "button_release_event",
                                        (GtkSignalFunc) button_release_cb,
                                        (gpointer) sp);
    sp->motion_id = gtk_signal_connect(GTK_OBJECT(sp->da),
                                       "motion_notify_event",
                                       (GtkSignalFunc)
                                       mouse_motion_notify_cb,
                                       (gpointer) sp);
  } else {
    disconnect_key_press_signal(sp);
    disconnect_button_press_signal(sp);
    disconnect_button_release_signal(sp);
  }
}

#endif
