/* varcircles.c */
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

#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#ifdef ENABLE_CAIRO
#include <math.h>
#endif


static GtkWidget *varcircle_create (gint, GGobiData *, ggobid * gg);
static void varcircle_draw (gint, GGobiData *, ggobid * gg);
static gboolean da_expose_cb (GtkWidget *, GdkEventExpose *, gpointer cbd);

GtkWidget *varcircles_get_nth (gint which, gint jvar, GGobiData * d);
static void varcircle_pack (GtkWidget *, GGobiData *);

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

#define VB  0
#define LBL 1
#define DA  2

void
varcircles_visibility_set (displayd * display, ggobid * gg)
{
  ProjectionMode projection;
  gint j;
  GtkWidget *box;
  GGobiData *d;
  GList *children;
  gint n = 0;

  if (!display)
    return;

  projection = pmode_get (display, gg);
  d = display->d;
  children = gtk_container_get_children (GTK_CONTAINER (d->vcirc_ui.table));

  switch (projection) {

  case TOUR2D3:
    for (j = 0; j < d->ncols; j++) {
      box = varcircles_get_nth (VB, j, d);
      /* if in the subset but not among the children, pack and unref */
      if (display->t2d3.subset_vars_p.els[j]) {
        if (g_list_index (children, box) == -1) {
          gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                              false, false, 2);
          gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
          gtk_widget_show_all (box);
          if (G_OBJECT (box)->ref_count > 1)
            gtk_widget_unref (box);
        }
        n++;

        /* if not in the subset but among the children, ref and remove */
      }
      else {
        if (g_list_index (children, box) >= 0) {
          gtk_widget_ref (box);
          gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
        }
      }
    }
    break;

  case TOUR1D:
    for (j = 0; j < d->ncols; j++) {
      box = varcircles_get_nth (VB, j, d);
      if (display->t1d.subset_vars_p.els[j]) {  /* in the subset */
        if (g_list_index (children, box) == -1) { /* but not among children */
          gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                              false, false, 2);
          gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
          gtk_widget_show_all (box);
          if (G_OBJECT (box)->ref_count > 1)
            gtk_widget_unref (box);
        }
        n++;

      }
      else {                    /* not in the subset */
        if (g_list_index (children, box) >= 0) {  /* but among children */
          gtk_widget_ref (box);
          gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
        }
      }
    }
    break;

  case TOUR2D:
    for (j = 0; j < d->ncols; j++) {
      box = varcircles_get_nth (VB, j, d);
      /* if in the subset but not among the children, pack and unref */
      if (display->t2d.subset_vars_p.els[j]) {
        if (g_list_index (children, box) == -1) {
          gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                              false, false, 2);
          gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
          gtk_widget_show_all (box);
          if (G_OBJECT (box)->ref_count > 1)
            gtk_widget_unref (box);
        }
        n++;

        /* if not in the subset but among the children, ref and remove */
      }
      else {
        if (g_list_index (children, box) >= 0) {
          gtk_widget_ref (box);
          gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
        }
      }
    }
    break;

  case COTOUR:
    for (j = 0; j < d->ncols; j++) {
      box = varcircles_get_nth (VB, j, d);
      /* if in either subset but not among the children, pack and unref */
      if (display->tcorr1.subset_vars_p.els[j] ||
          display->tcorr2.subset_vars_p.els[j]) {
        if (g_list_index (children, box) == -1) {
          gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                              false, false, 2);
          gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
          gtk_widget_show_all (box);
          if (G_OBJECT (box)->ref_count > 1)
            gtk_widget_unref (box);
        }
        n++;

        /* if not in the subset but among the children, ref and remove */
      }
      else {
        if (g_list_index (children, box) >= 0) {  /* among children */
          gtk_widget_ref (box);
          gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
        }
      }
    }
    break;
  default:
    break;
  }
}

GtkWidget *
varcircles_get_nth (gint which, gint jvar, GGobiData * d)
{
  GtkWidget *w = NULL;

  switch (which) {
  case VB:
    w = (GtkWidget *) g_slist_nth_data (d->vcirc_ui.vb, jvar);
    break;
  case LBL:
    w = (GtkWidget *) g_slist_nth_data (d->vcirc_ui.label, jvar);
    break;
  case DA:
    w = (GtkWidget *) g_slist_nth_data (d->vcirc_ui.da, jvar);
    break;
  default:
    break;
  }

  return w;
}

void
varcircles_delete_nth (gint jvar, GGobiData * d)
{
  GtkWidget *w;
  GdkPixmap *pix;

  w = varcircles_get_nth (LBL, jvar, d);
  d->vcirc_ui.label = g_slist_remove (d->vcirc_ui.label, (gpointer) w);
  w = varcircles_get_nth (DA, jvar, d);
  d->vcirc_ui.da = g_slist_remove (d->vcirc_ui.da, (gpointer) w);

  pix = (GdkPixmap *) g_slist_nth_data (d->vcirc_ui.da_pix, jvar);
  d->vcirc_ui.da_pix = g_slist_remove (d->vcirc_ui.da_pix, (gpointer) w);


  w = (GtkWidget *) g_slist_nth_data (d->vcirc_ui.vb, jvar);
  if (w != NULL) {
    if (w->parent) {            // If it has been packed, unpack it.
      g_object_ref (G_OBJECT (w));  // so it isn't destroyed when removed
      gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), w);
    }
    d->vcirc_ui.vb = g_slist_remove (d->vcirc_ui.vb, (gpointer) w);
  }
}


void
varcircle_label_set (GGobiData * d, gint j)
{
  GtkWidget *w = varcircles_get_nth (LBL, j, d);
  if (w != NULL)
    gtk_label_set_text (GTK_LABEL (w), ggobi_data_get_transformed_col_name(d, j));
}


/*
 * Return to the default cursor
*/
void
varcircles_cursor_set_default (GGobiData * d)
{
  GdkWindow *window = GTK_WIDGET (d->varpanel_ui.hpane)->window;
  gdk_cursor_destroy (d->vcirc_ui.cursor);
  d->vcirc_ui.jcursor = (gint) NULL;
  gdk_window_set_cursor (window, NULL);
}

static gint
manip_select_cb (GtkWidget * w, GdkEvent * event, GGobiData * d)
{
  GdkWindow *window = GTK_WIDGET (d->varpanel_ui.hpane)->window;

  d->vcirc_ui.cursor = gdk_cursor_new (GDK_HAND2);
  gdk_window_set_cursor (window, d->vcirc_ui.cursor);
  d->vcirc_ui.jcursor = (gint) GDK_HAND2;

  return true;
}

#ifdef FREEZE_IMPLEMENTED
static gint
freeze_select_cb (GtkWidget * w, GdkEvent * event, GGobiData * d)
{
  g_printerr ("not yet implemented\n");
  return true;
}
#endif

static gint
da_manip_expose_cb (GtkWidget * w, GdkEvent * event, GGobiData * d)
{
  ggobid *gg = GGobiFromWidget (w, true);
#ifdef ENABLE_CAIRO
  cairo_t *c = gdk_cairo_create (w->window);
  gdk_cairo_set_source_color (c, &gg->vcirc_manip_color);
  cairo_rectangle (c, 0, 0, w->allocation.width, w->allocation.height);
  cairo_fill (c);
  cairo_destroy (c);
#else
  GdkGC *gc = gdk_gc_new (w->window);

  gdk_gc_set_foreground (gc, &gg->vcirc_manip_color);
  gdk_draw_rectangle (w->window, gc,
                      true, 0, 0, w->allocation.width, w->allocation.height);
  gdk_gc_destroy (gc);
#endif

  return true;
}

#ifdef FREEZE_IMPLEMENTED
static gint
da_freeze_expose_cb (GtkWidget * w, GdkEvent * event, GGobiData * d)
{
  ggobid *gg = GGobiFromWidget (w, true);
#ifdef ENABLE_CAIRO
  cairo_t *c = gdk_cairo_create (w->window);
  gdk_cairo_set_source_color (c, &gg->vcirc_freeze_color);
  cairo_rectangle (c, 0, 0, w->allocation.width, w->allocation.height);
  cairo_fill (c);
  cairo_destroy (c);
#else
  GdkGC *gc = gdk_gc_new (w->window);

  gdk_gc_set_foreground (gc, &gg->vcirc_freeze_color);
  gdk_draw_rectangle (w->window, gc,
                      true, 0, 0, w->allocation.width, w->allocation.height);

  gdk_gc_destroy (gc);
#endif

  return true;
}
#endif

/*-- hide the circles interface --*/
void
varcircles_show (gboolean show, GGobiData * d, displayd * display,
                 ggobid * gg)
{
  GtkWidget *basement = widget_find_by_name (gg->main_window, "BASEMENT");
  GtkWidget *parent = (d->vcirc_ui.ebox)->parent;

  if (show) {
    /*
     * Add the ebox for the table of variable circles/rectangles
     * to the paned widget
     */
    if (display)
      varcircles_visibility_set (display, gg);

    /* reparent the variable circles */
    if (parent == basement) {
      gtk_widget_ref (d->vcirc_ui.ebox);
      gtk_container_remove (GTK_CONTAINER (basement), d->vcirc_ui.ebox);
      gtk_paned_pack2 (GTK_PANED (d->varpanel_ui.hpane),
                       d->vcirc_ui.ebox, true, true);
      gtk_widget_show (d->vcirc_ui.ebox);
    }
    else if (parent == NULL) {
      gtk_paned_pack2 (GTK_PANED (d->varpanel_ui.hpane),
                       d->vcirc_ui.ebox, true, true);
    }
  }
  else {
    /*-- remove circles/rectangles --*/

    if (parent == d->varpanel_ui.hpane) {
      gtk_widget_hide (d->vcirc_ui.ebox);
      gtk_widget_ref (d->vcirc_ui.ebox);
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.hpane),
                            d->vcirc_ui.ebox);
      gtk_box_pack_start (GTK_BOX (basement), d->vcirc_ui.ebox, false, false,
                          0);
    }

  /*-- set the handle position all the way to the right --*/
    gtk_paned_set_position (GTK_PANED (d->varpanel_ui.hpane), -1);


    /*-- adjust the reference count --*/
    /*
       #if GTK_MAJOR_VERSION == 1
       if (GTK_OBJECT (d->vcbox_ui.ebox)->ref_count > 1)
       #else
       if (G_OBJECT (d->vcbox_ui.ebox)->ref_count > 1)
       #endif
       gtk_widget_unref (d->vcbox_ui.ebox);
     */
  }
}

/*-- create the variable circles interface --*/
void
varcircles_populate (GGobiData * d, ggobid * gg)
{
  gint j;
  GtkWidget *vb, *da;

  d->vcirc_ui.jcursor = (gint) NULL;  /*-- start with the default cursor --*/
  d->vcirc_ui.cursor = (gint) NULL;

  /*-- don't pack this in the hpane yet --*/
  d->vcirc_ui.ebox = gtk_event_box_new ();
  gtk_widget_show (d->vcirc_ui.ebox);

  d->vcirc_ui.vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (d->vcirc_ui.ebox), d->vcirc_ui.vbox);
  gtk_widget_show (d->vcirc_ui.vbox);

  /*-- the first child of the vbox: the scrolled window, with table --*/
  d->vcirc_ui.swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->vcirc_ui.swin),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (d->vcirc_ui.swin),
                                       GTK_SHADOW_NONE);

  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.vbox), d->vcirc_ui.swin,
                      true, true, 0);
  gtk_widget_show (d->vcirc_ui.swin);

  d->vcirc_ui.table = gtk_vbox_new (false, 0);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW
                                         (d->vcirc_ui.swin),
                                         d->vcirc_ui.table);

  {
    GList *children;
    GtkWidget *foo;
    children = gtk_container_get_children (GTK_CONTAINER (d->vcirc_ui.swin));
    foo = g_list_nth_data (children, 0);
    if (GTK_IS_VIEWPORT (foo)) {
      gtk_viewport_set_shadow_type (GTK_VIEWPORT (foo), GTK_SHADOW_NONE);
    }
  }

  gtk_widget_show (d->vcirc_ui.table);

  /*-- da and label are freed in varcircle_clear --*/
  d->vcirc_ui.vb = NULL;
  d->vcirc_ui.da = NULL;
  d->vcirc_ui.label = NULL;
  d->vcirc_ui.da_pix = NULL;

  for (j = 0; j < d->ncols; j++) {
    vb = varcircle_create (j, d, gg);
    varcircle_pack (vb, d);
  }

  /*-- the second child of the vbox: an hbox with buttons --*/
  d->vcirc_ui.hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.vbox), d->vcirc_ui.hbox,
                      false, false, 2);
  gtk_widget_show (d->vcirc_ui.hbox);

  /* -- a drawing area to place next to the manip button as a color key -- */
  da = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (da, false);
  gtk_widget_set_size_request (GTK_WIDGET (da), 8, 8);
  gtk_widget_set_events (da, GDK_EXPOSURE_MASK);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), da, false, false, 2);
  ggobi_widget_set (da, gg, true);
  g_signal_connect (G_OBJECT (da), "expose_event",
                    G_CALLBACK (da_manip_expose_cb), d);
  gtk_widget_show (da);

  d->vcirc_ui.manip_btn = gtk_button_new_with_label ("Manip");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), d->vcirc_ui.manip_btn,
                        "Click here, then click on the variable you wish to manipulate",
                        NULL);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), d->vcirc_ui.manip_btn,
                      true, true, 2);
  g_signal_connect (G_OBJECT (d->vcirc_ui.manip_btn),
                    "button_press_event", G_CALLBACK (manip_select_cb), d);
  gtk_widget_show (d->vcirc_ui.manip_btn);

#ifdef FREEZE_IMPLEMENTED
  /* -- a drawing area to place next to the freeze button as a color key -- */
  da = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (da, false);
  gtk_widget_set_size_request (GTK_WIDGET (da), 8, 8);
  gtk_widget_set_events (da, GDK_EXPOSURE_MASK);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), da, false, false, 2);
  ggobi_widget_set (da, gg, true);
  g_signal_connect (G_OBJECT (da), "expose_event",
                    G_CALLBACK (da_freeze_expose_cb), d);
  gtk_widget_show (da);

  d->vcirc_ui.freeze_btn = gtk_button_new_with_label ("Freeze");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), d->vcirc_ui.freeze_btn,
                        "Click here, then click on the variable you wish to freeze",
                        NULL);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), d->vcirc_ui.freeze_btn,
                      true, true, 2);
  g_signal_connect (G_OBJECT (d->vcirc_ui.freeze_btn),
                    "button_press_event", G_CALLBACK (freeze_select_cb), d);
  gtk_widget_show (d->vcirc_ui.freeze_btn);
#endif
}

void
varcircles_delete (gint nc, gint jvar, GGobiData * d, ggobid * gg)
{
  gint j;
  GtkWidget *w;
  GdkPixmap *pix;

  if (nc > 0 && nc < d->ncols) {  /*-- forbid deleting every circle --*/
    for (j = jvar; j < jvar + nc; j++) {
      w = varcircles_get_nth (LBL, j, d);
      d->vcirc_ui.label = g_slist_remove (d->vcirc_ui.label, w);

      w = varcircles_get_nth (DA, j, d);
      d->vcirc_ui.da = g_slist_remove (d->vcirc_ui.da, w);

      w = varcircles_get_nth (VB, j, d);
      d->vcirc_ui.vb = g_slist_remove (d->vcirc_ui.vb, w);
      /*-- without a ref, this will be destroyed --*/
      gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), w);

      pix = (GdkPixmap *) g_slist_nth_data (d->vcirc_ui.da, jvar);
      d->vcirc_ui.da_pix = g_slist_remove (d->vcirc_ui.da_pix, pix);
      gdk_pixmap_unref (pix);
    }
  }
}

/*-- this destroys them all -- it's not yet called anywhere --*/
void
varcircles_clear (ggobid * gg)
{
  GtkWidget *w;
  gint j;
  GSList *l;
  GGobiData *d;
  GdkPixmap *pix;

  for (l = gg->d; l; l = l->next) {
    d = (GGobiData *) l->data;
    for (j = 0; j < d->vcirc_ui.nvars; j++) {
                                           /*-- variable not initialized --*/
      w = varcircles_get_nth (LBL, j, d);
      d->vcirc_ui.label = g_slist_remove (d->vcirc_ui.label, w);

      w = varcircles_get_nth (DA, j, d);
      d->vcirc_ui.da = g_slist_remove (d->vcirc_ui.da, w);

      w = varcircles_get_nth (VB, j, d);
      d->vcirc_ui.vb = g_slist_remove (d->vcirc_ui.vb, w);
      gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), w);

      pix = (GdkPixmap *) g_slist_nth_data (d->vcirc_ui.da, j);
      d->vcirc_ui.da_pix = g_slist_remove (d->vcirc_ui.da_pix, pix);
      gdk_pixmap_unref (pix);
    }
  }
}


/*-- responds to a button_press_event --*/
static gint
varcircle_sel_cb (GtkWidget * w, GdkEvent * event, gint jvar)
{
  ggobid *gg = GGobiFromWidget (w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  splotd *sp = gg->current_splot;
  GGobiData *d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);

  if (d != display->d)
    return true;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint mouse = bevent->button;
    gboolean alt_mod, shift_mod, ctrl_mod;

    /*-- respond only to button 1 and button 2 --*/
    if (mouse != 1 && mouse != 2)
      return false;

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    /*-- general variable selection --*/
    varsel (w, cpanel, sp, jvar, -1 /* toggle */ , mouse,
            alt_mod, ctrl_mod, shift_mod, d, gg);
    varcircles_refresh (d, gg);
    return true;
  }

  return false;
}

static GtkWidget *
varcircle_create (gint j, GGobiData * d, ggobid * gg)
{
  GtkWidget *vb, *lbl, *da;

  vb = gtk_hbox_new (false, 0);
  d->vcirc_ui.vb = g_slist_append (d->vcirc_ui.vb, vb);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 1);


  /*-- a drawing area to contain the variable circle --*/
  da = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (da, false);
  d->vcirc_ui.da = g_slist_append (d->vcirc_ui.da, da);
  gtk_widget_set_size_request (GTK_WIDGET (da),
                               VAR_CIRCLE_DIAM + 2, VAR_CIRCLE_DIAM + 2);
  gtk_widget_set_events (da, GDK_EXPOSURE_MASK
                         | GDK_ENTER_NOTIFY_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), da,
                        "Click left to select or deselect", NULL);

  g_signal_connect (G_OBJECT (da), "expose_event",
                    G_CALLBACK (da_expose_cb), GINT_TO_POINTER (j));
  g_signal_connect (G_OBJECT (da), "button_press_event",
                    G_CALLBACK (varcircle_sel_cb), GINT_TO_POINTER (j));
  g_object_set_data (G_OBJECT (da), "datad", d);
  ggobi_widget_set (GTK_WIDGET (da), gg, true);
  /*gtk_container_add (GTK_CONTAINER (vb), da); */
  gtk_box_pack_start (GTK_BOX (vb), da, false, false, 0);

  /*-- label --*/
  lbl = gtk_label_new (ggobi_data_get_col_name(d, j));
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, .5);  /*- x: left, y: middle --*/
  d->vcirc_ui.label = g_slist_append (d->vcirc_ui.label, lbl);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
                        lbl, "Click left on the circle to select or deselect",
                        NULL);
  g_object_set_data (G_OBJECT (lbl), "datad", d);
  ggobi_widget_set (GTK_WIDGET (lbl), gg, true);
  /*gtk_container_add (GTK_CONTAINER (vb), lbl); */
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  gtk_widget_show_all (vb);
  return (vb);
}

static void
varcircle_pack (GtkWidget * vb, GGobiData * d)
{
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), vb, false, false, 2);
}

void
varcircles_refresh (GGobiData * d, ggobid * gg)
{
  gint j;
  GtkWidget *da;

  for (j = 0; j < d->ncols; j++) {
    da = varcircles_get_nth (DA, j, d);
    if (GTK_WIDGET_REALIZED (da) && GTK_WIDGET_VISIBLE (da))
      varcircle_draw (j, d, gg);
  }
}

void
varcircle_draw (gint jvar, GGobiData * d, ggobid * gg)
{
  gboolean chosen = false;
  splotd *sp = gg->current_splot;
  displayd *display;
  cpaneld *cpanel;
  gint k, len;
  GtkWidget *da = varcircles_get_nth (DA, jvar, d);
  GdkPixmap *da_pix;
#ifdef ENABLE_CAIRO
  cairo_t *c;
  double radius = VAR_CIRCLE_DIAM / 2.0;
#endif

  if (sp == NULL || jvar < 0 || jvar >= d->ncols)
    return;  /*-- return --*/

  display = (displayd *) sp->displayptr;

  if (display == NULL || display->d != d)
    return;  /*-- return --*/

  cpanel = &display->cpanel;

  if (gg->selvarfg_GC == NULL)
    init_var_GCs (da, gg);

  if ((len = g_slist_length (d->vcirc_ui.da_pix)) < d->ncols) {
    for (k = len; k < d->ncols; k++) {
      d->vcirc_ui.da_pix = g_slist_append (d->vcirc_ui.da_pix,
                                           gdk_pixmap_new (da->window,
                                                           VAR_CIRCLE_DIAM +
                                                           1,
                                                           VAR_CIRCLE_DIAM +
                                                           1, -1));
      /*
       * clear and initialize each pixmap here, because they may be
       * exposed out of sequence, and then noise is drawn to the
       * variable circle on the screen.
       */
      da_pix = g_slist_nth_data (d->vcirc_ui.da_pix, k);
      gdk_draw_rectangle (da_pix, gg->unselvarbg_GC, true,
                          0, 0, VAR_CIRCLE_DIAM + 1, VAR_CIRCLE_DIAM + 1);
#ifdef ENABLE_CAIRO
      c = gdk_cairo_create (da_pix);
      cairo_arc (c, radius, radius, radius, 0, 2 * M_PI);
      cairo_set_source_rgb (c, 0, 0, 0);
      cairo_fill_preserve (c);
      cairo_set_source_rgb (c, 1.0, 1.0, 1.0);
      cairo_stroke (c);
      cairo_destroy (c);
#else
      gdk_draw_arc (da_pix, gg->selvarbg_GC, true,
                    0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
      gdk_draw_arc (da_pix, gg->unselvarfg_GC, false,
                    0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
#endif
    }
  }

  da_pix = g_slist_nth_data (d->vcirc_ui.da_pix, jvar);

  /*-- clear the pixmap --*/
  gdk_draw_rectangle (da_pix, gg->unselvarbg_GC, true,
                      0, 0, VAR_CIRCLE_DIAM + 1, VAR_CIRCLE_DIAM + 1);

#ifdef ENABLE_CAIRO
  c = gdk_cairo_create (da_pix);
  cairo_arc (c, radius, radius, radius - 1, 0, 2 * M_PI);
  cairo_set_source_rgb (c, 1.0, 1.0, 1.0);
  cairo_fill_preserve (c);
  cairo_set_source_rgb (c, 0, 0, 0);
#else
  /*-- add a filled circle for the background --*/
  gdk_draw_arc (da_pix, gg->selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
#endif
  /*-- add the appropriate line --*/
  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    GGobiExtendedDisplayClass *klass;
    klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
    if (klass->varcircle_draw)
      chosen = klass->varcircle_draw (display, jvar, da_pix, gg);
  }

  /*
   * add an open circle for the outline
   */
  if (chosen) {
#ifdef ENABLE_CAIRO
    cairo_set_line_width (c, 2);
#else
    gdk_draw_arc (da_pix, gg->selvarfg_GC, false,
                  0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
#endif
  }
  else {
#ifdef ENABLE_CAIRO
    cairo_set_line_width (c, 1);
#else
    gdk_draw_arc (da_pix, gg->unselvarfg_GC, false,
                  0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
#endif
  }

#ifdef ENABLE_CAIRO
  cairo_stroke (c);
  cairo_destroy (c);
#endif

  /*
   * copy the pixmap to the window
   */
  gdk_draw_drawable (da->window, gg->unselvarfg_GC,
                     da_pix, 0, 0, 0, 0,
                     VAR_CIRCLE_DIAM + 1, VAR_CIRCLE_DIAM + 1);
}

void
tour_draw_circles (GGobiData * d, ggobid * gg)
{
  gint j;

  for (j = 0; j < d->ncols; j++) {
    varcircle_draw (j, d, gg);
  }
}

gboolean
da_expose_cb (GtkWidget * w, GdkEventExpose * event, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint j = GPOINTER_TO_INT (cbd);
  GGobiData *d = (GGobiData *) g_object_get_data (G_OBJECT (w), "datad");
  GtkWidget *da = varcircles_get_nth (DA, j, d);
  GdkPixmap *da_pix = g_slist_nth_data (d->vcirc_ui.da_pix, j);

  if (j >= d->ncols)
    return false;

  if (da_pix == NULL) {
    varcircle_draw (j, d, gg);
  }
  else {
    gdk_draw_pixmap (da->window, gg->unselvarfg_GC,
                     da_pix, 0, 0, 0, 0,
                     VAR_CIRCLE_DIAM + 1, VAR_CIRCLE_DIAM + 1);
  }

  return true;
}

/*-- used in cloning and appending variables; see vartable.c --*/
void
varcircles_add (gint nc, GGobiData * d, ggobid * gg)
{
  gint j;
  GtkWidget *vb;
  gint n = g_slist_length (d->vcirc_ui.vb);

  /*-- create the variable circles --*/
  for (j = n; j < nc; j++) {
    vb = varcircle_create (j, d, gg);
    /* varcircle_pack (vb, d); */
  }

  gtk_widget_show_all (gg->varpanel_ui.notebook);
}
