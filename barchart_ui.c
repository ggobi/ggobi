/* barchart_ui.c */
#ifdef BARCHART_IMPLEMENTED
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "vars.h"
#include "externs.h"

static gchar *display_mode_lbl[] = {"Barchart", "Spineplot"};


void barchart_set_initials (splotd *sp, datad *d);
void barchart_recalc_counts (splotd *sp, datad *d, ggobid *gg);
void barchart_allocate_structure (splotd *sp, datad *d);

static void display_mode_cb (GtkWidget *w, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget(w, true);
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;
  displayd *display = gg->current_display;

  cpanel->barchart_display_mode = GPOINTER_TO_INT (cbd);
  sp->bar->is_spine = !sp->bar->is_spine; 
  scatterplot_show_vrule (display, !sp->bar->is_spine);
}

/*--------------------------------------------------------------------*/
/*                   Control panel section                            */
/*--------------------------------------------------------------------*/

void
cpanel_barchart_make (ggobid *gg) {
  GtkWidget *vb, *lbl, *opt;
 
  gg->control_panel[BARCHART] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (gg->control_panel[BARCHART]), 5);


/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (gg->control_panel[BARCHART]), vb, false, false, 0);

  lbl = gtk_label_new ("Display mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_widget_set_name (opt, "BARCHART:displ_mode_option_menu");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), opt,
    "some sensible text about spine plots and barcharts",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, display_mode_lbl,
                        sizeof (display_mode_lbl) / sizeof (gchar *),
                        display_mode_cb, gg);



  gtk_widget_show_all (gg->control_panel[BARCHART]);
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
void
barchart_mode_menu_make (GtkAccelGroup *accel_group, 
			 GtkSignalFunc func, 
 			 ggobid *gg, 
			 gboolean useIds) {

  gg->barchart.mode_menu = gtk_menu_new ();

  CreateMenuItem (gg->barchart.mode_menu, "Barchart",
    "^h", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BARCHART) : gg, gg);

  /* Add a separator */
  CreateMenuItem (gg->barchart.mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuItem (gg->barchart.mode_menu, "Scale",
    "^s", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (SCALE) : gg, gg);
  CreateMenuItem (gg->barchart.mode_menu, "Brush",
    "^b", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (BRUSH) : gg, gg);
  CreateMenuItem (gg->barchart.mode_menu, "Identify",
    "^i", "", NULL, accel_group, func,
    useIds ? GINT_TO_POINTER (IDENT) : gg, gg);

  gtk_widget_show (gg->barchart.mode_menu);
}

/*--------------------------------------------------------------------*/
/*                   End of main menubar section                      */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*                      Control panel updating                        */
/*--------------------------------------------------------------------*/

/*-- there already exists barchart_cpanel_init --*/

void
cpanel_barchart_set (cpaneld *cpanel, ggobid *gg)
{
  GtkWidget *w;

  w = widget_find_by_name (gg->control_panel[BARCHART],
                           "BARCHART:displ_mode_option_menu");

  gtk_option_menu_set_history (GTK_OPTION_MENU(w),
                               cpanel->barchart_display_mode);
}


/*--------------------------------------------------------------------*/
/*      Handling keyboard and mouse events in the plot window         */
/*--------------------------------------------------------------------*/

static gint
key_press_cb (GtkWidget *w, GdkEventKey *event, splotd *sp)
{
  ggobid *gg = GGobiFromSPlot(sp);
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean reallocate = FALSE;

  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  vartabled *vtx;
  vtx = vartable_element_get (sp->p1dvar, d);

/*-- add a key_press_cb in each mode, and let it begin with these lines --*/
  if (splot_event_handled (w, event, cpanel, sp, gg))
    return true;

  /*-- insert mode-specific key presses (if any) here --*/
  switch (event->keyval) {
    case GDK_plus:
      if (!vtx->categorical_p) { 
        sp->bar->new_nbins = sp->bar->nbins + 1;
        reallocate = TRUE;
      }
    break;

    case GDK_minus:
      if (!vtx->categorical_p) {
        if (sp->bar->nbins > 2) {
          sp->bar->new_nbins = sp->bar->nbins -1;
          reallocate = TRUE;
        }
      }
    break;
  }
  
  if (reallocate) {
    displayd *display = (displayd *) sp->displayptr;
    datad *d = display->d;
    ggobid *gg = GGobiFromSPlot(sp);

    barchart_allocate_structure (sp, d);
    barchart_set_initials (sp,d);
    barchart_recalc_counts (sp,d,gg);
    sp->redraw_style = FULL;
    splot_redraw (sp, sp->redraw_style, gg); 
  }
    

  return true;
}

static gint
button_release_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;

  gg->buttondown = 0;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y, &state);

  gdk_pointer_ungrab (event->time);
  disconnect_motion_signal (sp);

  return retval;
}

static gint
mouse_motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gboolean button1_p, button2_p;


  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (sp->bar->is_histogram) {
    GdkRegion *region;
    gboolean cursor_set = FALSE;
    
    region = gdk_region_polygon (sp->bar->anchor_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region, sp->mousepos.x,sp->mousepos.y)) {
      splot_cursor_set (GDK_SPIDER, sp);
 
     cursor_set = TRUE;
    }
    gdk_region_destroy(region);

    region = gdk_region_polygon (sp->bar->offset_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region,sp->mousepos.x,sp->mousepos.y)) {
      splot_cursor_set (GDK_UMBRELLA, sp);

      cursor_set = TRUE;
    }
/*
    if ((!cursor_set) && (sp->jcursor)) {
      splot_cursor_set ((gint)NULL, sp);
      sp->cursor = NULL;
    }
*/
    gdk_region_destroy(region);
  }


  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return true;
}


static gint
motion_notify_cb (GtkWidget *w, GdkEventMotion *event, splotd *sp)
{
  gboolean button1_p, button2_p;
  ggobid *gg = GGobiFromSPlot(sp);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  /*-- get the mouse position and find out which buttons are pressed --*/
  mousepos_get_motion (w, event, &button1_p, &button2_p, sp);

  /*-- if neither button is pressed, we shouldn't have gotten the event --*/
  if (!button1_p && !button2_p)
    return false;

  /*-- I'm not sure this could ever happen --*/
  if (sp->mousepos.x == sp->mousepos_o.x && sp->mousepos.y == sp->mousepos_o.y)
    return false;

  if (sp->bar->is_histogram) {
    if (sp->bar->anchor_drag) {
      gint dy;
      gfloat scale_y;
      cpaneld *cpanel = &display->cpanel;
      icoords scr;
      fcoords pts1, pts2;
 
      dy = sp->mousepos.y - sp->mousepos_o.y;
      if (dy != 0) {
        displayd *display = (displayd *) sp->displayptr;
        datad *d = display->d;
        gboolean set_anchor = TRUE; 
        gfloat offset_old = sp->bar->offset;
        gint pmid_old = sp->pmid.y;

        scr.x = scr.y = 0;
        scale_y = sp->scale.y;
        scale_y /= 2;
        sp->iscale.y = (glong) ((gfloat) sp->max.y * scale_y);

        splot_screen_to_tform (cpanel, sp, &scr, &pts1, gg);
        sp->pmid.y -= ((dy * PRECISION1) / sp->iscale.y);
        splot_screen_to_tform (cpanel, sp, &scr, &pts2, gg);
        sp->bar->offset +=  (pts1.y - pts2.y);

/* moving the anchor more than one binwidth up or down doesn't make sense */
        if (sp->bar->offset > offset_old) {
          /* not too high */
          set_anchor = (sp->bar->offset < (sp->bar->breaks[1]-sp->bar->breaks[0]));
        }
        if (sp->bar->offset < offset_old) {
          /* and not too low */
          set_anchor = (sp->bar->offset > -(sp->bar->breaks[1]-sp->bar->breaks[0])); 
        }
 
     
        if (set_anchor) {
          barchart_recalc_counts (sp, d, gg);
          splot_redraw (sp, FULL, gg);
        } else {
          sp->pmid.y = pmid_old;
          sp->bar->offset = offset_old;
        }
      }

      sp->mousepos_o.x = sp->mousepos.x;
      sp->mousepos_o.y = sp->mousepos.y;

/**/  return true; 
    }
    if (sp->bar->width_drag) {
      gint dy;
      cpaneld *cpanel = &display->cpanel;
      fcoords pts1, pts2;
     
      dy = sp->mousepos.y - sp->mousepos_o.y;
      if (dy != 0) {
        displayd *display = (displayd *) sp->displayptr;
        datad *d = display->d;
        gfloat width, oldwidth;

        splot_screen_to_tform (cpanel, sp, &sp->mousepos_o, &pts1, gg);
        splot_screen_to_tform (cpanel, sp, &sp->mousepos, &pts2, gg);

        oldwidth = sp->bar->breaks[1]- sp->bar->breaks[0];
        width = oldwidth -(pts1.y - pts2.y);
        if (width > 0.) {
          extern void barchart_set_breakpoints (gfloat width, splotd *sp, datad *d );
          gboolean set_breaks = TRUE; 
          gint pix_width = sp->bar->bins[0].rect.y - sp->bar->bins[1].rect.y; 
          if (width > oldwidth)  {
            set_breaks = sp->bar->bins[0].rect.y > (sp->max.y/2+1); /* not too big */
          }
          if (width < oldwidth) {
            set_breaks = pix_width > 6;  /* not too small */
          }
          if (set_breaks) {       
            barchart_set_breakpoints (width,sp,d);
            barchart_recalc_counts (sp,d,gg);
            splot_redraw (sp, FULL, gg);
          }
        }
      }
      sp->mousepos_o.x = sp->mousepos.x;
      sp->mousepos_o.y = sp->mousepos.y;

/**/  return true;
    }

  } 


  switch (cpanel->scale_style) {

    case DRAG:
      if (button1_p) {
        pan_by_drag (sp, gg);
      } else if (button2_p) {
        zoom_by_drag (sp, gg);
      }

      /*-- redisplay this plot --*/
      splot_plane_to_screen (display, &display->cpanel, sp, gg);
      ruler_ranges_set (false, gg->current_display, sp, gg);
      splot_redraw (sp, FULL, gg);
      break;

    case CLICK:
      splot_redraw (sp, QUICK, gg);
      break;

  }  /*-- end switch (scale_style) --*/

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  return true;
}

static gint
button_press_cb (GtkWidget *w, GdkEventButton *event, splotd *sp)
{
  gboolean retval = true;
  ggobid *gg = GGobiFromSPlot(sp);
  gboolean button1_p, button2_p;
  GdkRegion *region;
  mousepos_get_pressed (w, event, &button1_p, &button2_p, sp);

  gg->current_splot = sp->displayptr->current_splot = sp;
  gg->current_display = (displayd *) sp->displayptr;


  if (sp->bar->is_histogram) {
    sp->bar->anchor_drag = FALSE;
    sp->bar->width_drag = FALSE;

    region = gdk_region_polygon (sp->bar->anchor_rgn, 3, GDK_WINDING_RULE); 
    if (gdk_region_point_in (region, sp->mousepos.x,sp->mousepos.y)) {
      sp->bar->anchor_drag = TRUE;

    }
    gdk_region_destroy(region);

    region = gdk_region_polygon (sp->bar->offset_rgn, 3, GDK_WINDING_RULE);
    if (gdk_region_point_in (region,
                           sp->mousepos.x,sp->mousepos.y)) {
      sp->bar->width_drag = TRUE;
    }
    gdk_region_destroy(region);
  }

  sp->mousepos_o.x = sp->mousepos.x;
  sp->mousepos_o.y = sp->mousepos.y;

  disconnect_motion_signal (sp);
  sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) motion_notify_cb,
                                      (gpointer) sp);
  return retval;
}

void
barchart_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

  if (state == on) {
    sp->key_press_id = gtk_signal_connect (GTK_OBJECT (display->window),
                                           "key_press_event",
                                           (GtkSignalFunc) key_press_cb,
                                           (gpointer) sp);

  } else {
    disconnect_key_press_signal (sp);

  }
}


void
barchart_scale_event_handlers_toggle (splotd *sp, gboolean state) {
  displayd *display = (displayd *) sp->displayptr;

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
    sp->motion_id = gtk_signal_connect (GTK_OBJECT (sp->da),
                                      "motion_notify_event",
                                      (GtkSignalFunc) mouse_motion_notify_cb,
                                      (gpointer) sp);
  } else {
    disconnect_key_press_signal (sp);
    disconnect_button_press_signal (sp);
    disconnect_button_release_signal (sp);
  }
}

#endif
