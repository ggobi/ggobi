#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


static GtkWidget * varcircle_create (gint, datad *, ggobid *gg);
static void varcircle_draw (gint, datad *, ggobid *gg); 
static gboolean da_expose_cb (GtkWidget *, GdkEventExpose *, gpointer cbd);

GtkWidget *varcircles_get_nth (gint which, gint jvar, datad *d);
static void varcircle_pack (GtkWidget *, datad *);

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

#define VB  0
#define LBL 1
#define DA  2

void
varcircles_visibility_set (displayd *display, ggobid *gg)
{
  gint projection = (gint) projection_get (gg);
  gint j;
  GtkWidget *box;
  datad *d = display->d;
  GList *children = gtk_container_children (GTK_CONTAINER (d->vcirc_ui.table));
  gint n = 0;

/*
g_printerr ("0: nchildren = %d\n", g_list_length (children));
g_printerr ("0: nboxes = %d\n", g_slist_length (d->vcirc_ui.vb));
*/

  switch (projection) {

    case TOUR2D3:
      for (j=0; j<d->ncols; j++) {
        box = varcircles_get_nth (VB, j, d);
        /* if in the subset but not among the children, pack and unref */
        if (display->t2d3.subset_vars_p.els[j]) {
          if (g_list_index (children, box) == -1) {
            gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                                false, false, 2);
            gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
            gtk_widget_show_all (box);
#if GTK_MAJOR_VERSION == 1
            if (GTK_OBJECT (box)->ref_count > 1)
#else
            if (G_OBJECT (box)->ref_count > 1)
#endif
              gtk_widget_unref (box);
          }
          n++;

        /* if not in the subset but among the children, ref and remove */
        } else { 
          if (g_list_index (children, box) >= 0) {
            gtk_widget_ref (box);
            gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
          }
        }
      }
    break;

    case TOUR1D:
      for (j=0; j<d->ncols; j++) {
        box = varcircles_get_nth (VB, j, d);
        if (display->t1d.subset_vars_p.els[j]) {    /* in the subset */
          if (g_list_index (children, box) == -1) { /* but not among children */
            gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                                false, false, 2);
            gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
            gtk_widget_show_all (box);
#if GTK_MAJOR_VERSION == 1
/*g_printerr ("ref_count: %d\n", GTK_OBJECT (box)->ref_count); */
            if (GTK_OBJECT (box)->ref_count > 1)
#else
            if (G_OBJECT (box)->ref_count > 1)
#endif
              gtk_widget_unref (box);
          }
          n++;
 
        } else {  /* not in the subset */
          if (g_list_index (children, box) >= 0) {  /* but among children */
            gtk_widget_ref (box);
            gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
          }
        }
      }
    break;

    case TOUR2D:
      for (j=0; j<d->ncols; j++) {
        box = varcircles_get_nth (VB, j, d);
        /* if in the subset but not among the children, pack and unref */
        if (display->t2d.subset_vars_p.els[j]) {
          if (g_list_index (children, box) == -1) {
/*
g_printerr ("box %d in subset but not among children; pack and unref\n", j);
*/
            gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                                false, false, 2);
            gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
            gtk_widget_show_all (box);
#if GTK_MAJOR_VERSION == 1
            if (GTK_OBJECT (box)->ref_count > 1)
#else
            if (G_OBJECT (box)->ref_count > 1)
#endif
              gtk_widget_unref (box);
          } else {
/*
g_printerr ("box %d in subset AND among children; do nothing\n", j);
*/
          }
          n++;

        /* if not in the subset but among the children, ref and remove */
        } else { 
          if (g_list_index (children, box) >= 0) {
/*
g_printerr ("%d is not in subset but IS among children; ref & remove\n", j);
*/
            gtk_widget_ref (box);
            gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), box);
          } else {
/*
g_printerr ("%d is not in subset and is NOT among children; do nothing\n", j);
*/
          }
        }
      }
    break;

    case COTOUR:
      for (j=0; j<d->ncols; j++) {
        box = varcircles_get_nth (VB, j, d);
        /* if in either subset but not among the children, pack and unref */
        if (display->tcorr1.subset_vars_p.els[j] ||
            display->tcorr2.subset_vars_p.els[j]) {
          if (g_list_index (children, box) == -1) {
            gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), box,
                                false, false, 2);
            gtk_box_reorder_child (GTK_BOX (d->vcirc_ui.table), box, n);
            gtk_widget_show_all (box);
#if GTK_MAJOR_VERSION == 1
            if (GTK_OBJECT (box)->ref_count > 1)
#else
            if (G_OBJECT (box)->ref_count > 1)
#endif
              gtk_widget_unref (box);
          }
          n++;

        /* if not in the subset but among the children, ref and remove */
        } else {
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

/*
children = gtk_container_children (GTK_CONTAINER (d->vcirc_ui.table));
g_printerr ("1: nchildren = %d\n", g_list_length (children));
g_printerr ("1: nboxes = %d\n", g_slist_length (d->vcirc_ui.vb));
*/
}

GtkWidget *
varcircles_get_nth (gint which, gint jvar, datad *d) {
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
varcircles_delete_nth (gint jvar, datad *d)
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
    d->vcirc_ui.vb = g_slist_remove (d->vcirc_ui.vb, (gpointer) w);
    gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), w);
  }
}


/*
 * This is unfortunately needed for now because no widgets have
 * been realized yet when this is first called.
 *
 * I'm not sure which layout should be first; this does it row-wise.
*/
void
varcircles_layout_init (datad *d, ggobid *gg)
{
/*
  gint tncols = 0, tnrows = 0;
  gint NCOLS = 5;

  tncols = (d->ncols == 0) ? 0 : MIN (NCOLS, d->ncols);
  if (tncols) {
    tnrows = d->ncols / tncols;
    if (tnrows * tncols < d->ncols) tnrows++;
  }

  d->vcirc_ui.tncols = tncols;
  d->vcirc_ui.tnrows = tnrows;
*/
  d->vcirc_ui.tncols = 1;
  d->vcirc_ui.tnrows = d->ncols;
}

#ifdef CIRCLES_IN_TABLE
void
varcircles_layout_reset (gint ncols, datad *d, ggobid *gg)
{
  gint j;
  GtkWidget *vb;
  gint tnrows, tncols;
  gint left_attach, top_attach;
  GtkAdjustment *adj;

  /*-- let's see if this works ---*/
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->vcirc_ui.swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  /*-- we need any old vb; loop in case the first vars were deleted --*/
  vb = NULL;
  j = 0;
  while (vb == NULL && j < ncols)
    vb = varcircles_get_nth (VB, j++, d);

  if (gg->varpanel_ui.layoutByRow) {
    gint vport_width, vb_width;

    adj = gtk_scrolled_window_get_hadjustment (
      GTK_SCROLLED_WINDOW (d->vcirc_ui.swin));
    vport_width = adj->page_size;

    /*
     * if the varcircles have never been exposed, can't use page_size,
     * so just keep adding rows
    */
    if (vport_width == 0) {
      while (ncols > d->vcirc_ui.tncols * d->vcirc_ui.tnrows)
        d->vcirc_ui.tnrows++;
    } else {  /*-- fill the available space as best we can --*/
      vb_width = vb->allocation.width;
      if (vb_width < 5) vb_width = VAR_CIRCLE_DIAM;
      tncols = MIN (vport_width / vb_width, ncols);
      tnrows = ncols / tncols;
      if (tnrows * tncols < ncols) tnrows++;
      d->vcirc_ui.tncols = tncols;
      d->vcirc_ui.tnrows = tnrows;
    }

  } else {
    gint vport_height, vb_height;

    adj = gtk_scrolled_window_get_vadjustment (
      GTK_SCROLLED_WINDOW (d->vcirc_ui.swin));
    vport_height = adj->page_size;

    if (vport_height == 0) {
      while (ncols > d->vcirc_ui.tncols * d->vcirc_ui.tnrows)
        d->vcirc_ui.tncols++;
    } else {
      vb_height = vb->allocation.height;
      if (vb_height < 5) vb_height = VAR_CIRCLE_DIAM;  /*-- a bit low ... --*/

      tnrows = MIN (vport_height / vb_height, ncols);
      tncols = ncols / tnrows;
      if (tnrows * tnrows < ncols) tncols++;
      d->vcirc_ui.tncols = tncols;
      d->vcirc_ui.tnrows = tnrows;
    }
  }

  for (j=0; j<ncols; j++) {
    /*-- if they're in the container, ref and remove them --*/
    vb = varcircles_get_nth (VB, j, d);
    if (vb != NULL && vb->parent != NULL && vb->parent == d->vcirc_ui.table)
    {
      gtk_widget_ref (vb);
      gtk_container_remove (GTK_CONTAINER (d->vcirc_ui.table), vb);
    }
  }

  gtk_table_resize (GTK_TABLE (d->vcirc_ui.table),
                    d->vcirc_ui.tnrows, d->vcirc_ui.tncols);

  left_attach = top_attach = 0;
  for (j=0; j<ncols; j++) {
    vb = varcircles_get_nth (VB, j, d);

    /*-- attach the first one at 0,0 --*/
    gtk_table_attach (GTK_TABLE (d->vcirc_ui.table),
      vb,
      left_attach, left_attach+1, top_attach, top_attach+1,
      GTK_FILL, GTK_FILL, 0, 0);

#if GTK_MAJOR_VERSION == 1
      if (GTK_OBJECT (vb)->ref_count > 1)
        gtk_widget_unref (vb);
#else
      if (G_OBJECT (vb)->ref_count > 1)
        gtk_widget_unref (vb);
#endif

    if (gg->varpanel_ui.layoutByRow) {
      left_attach++; 
      if (left_attach == d->vcirc_ui.tncols) {
        left_attach = 0;
        top_attach++;
      }
    } else {
      top_attach++; 
      if (top_attach == d->vcirc_ui.tnrows) {
        top_attach = 0;
        left_attach++;
      }
    }
  }
}
#endif

void
varcircle_label_set (gint j, datad *d)
{
  GtkWidget *w = varcircles_get_nth (LBL, j, d);
  vartabled *vt = vartable_element_get (j, d);
  if (w != NULL)
    gtk_label_set_text (GTK_LABEL(w), vt->collab_tform);
}

/*-- called from the Options menu --*/
void
varcircles_layout_cb (GtkCheckMenuItem *w, guint action) 
{
/*
  GSList *l;
  datad *d;
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);

  gg->varpanel_ui.layoutByRow = !gg->varpanel_ui.layoutByRow;
  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    varcircles_layout_reset (d->ncols, d, gg);
  }
*/
}


/*
 * Return to the default cursor
*/
void
varcircles_cursor_set_default (datad *d)
{
  GdkWindow *window = GTK_WIDGET (d->varpanel_ui.hpane)->window;
  gdk_cursor_destroy (d->vcirc_ui.cursor);
  d->vcirc_ui.jcursor = (gint) NULL;
  gdk_window_set_cursor (window, NULL);
}

static gint
manip_select_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  GdkWindow *window = GTK_WIDGET (d->varpanel_ui.hpane)->window;

  d->vcirc_ui.cursor = gdk_cursor_new (GDK_HAND2);
  gdk_window_set_cursor (window, d->vcirc_ui.cursor);
  d->vcirc_ui.jcursor = (gint )GDK_HAND2;
  
  return true;
}

#ifdef FREEZE_IMPLEMENTED
static gint
freeze_select_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  g_printerr ("not yet implemented\n");
  return true;
}
#endif

static gint
da_manip_expose_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  GdkGC *gc = gdk_gc_new (w->window);

  gdk_gc_set_foreground (gc, &gg->vcirc_manip_color);
  gdk_draw_rectangle (w->window, gc,
                      true,
                      0, 0,
                      w->allocation.width, w->allocation.height);

  gdk_gc_destroy (gc);

  return true;
}

#ifdef FREEZE_IMPLEMENTED
static gint
da_freeze_expose_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  GdkGC *gc = gdk_gc_new (w->window);

  gdk_gc_set_foreground (gc, &gg->vcirc_freeze_color);
  gdk_draw_rectangle (w->window, gc,
                      true,
                      0, 0,
                      w->allocation.width, w->allocation.height);

  gdk_gc_destroy (gc);

  return true;
}
#endif


/*-- create the variable circles interface --*/
void
varcircles_populate (datad *d, ggobid *gg)
{
  gint j;
  GtkWidget *vb, *da;

#ifdef CIRCLES_IN_TABLE 
  varcircles_layout_init (d, gg);
#endif
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
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.vbox), d->vcirc_ui.swin,
    true, true, 2);
  gtk_widget_show (d->vcirc_ui.swin);

/*
  d->vcirc_ui.table = gtk_table_new (d->vcirc_ui.tnrows,
                                     d->vcirc_ui.tncols, true);
*/
  d->vcirc_ui.table = gtk_vbox_new (false, 0);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (d->vcirc_ui.swin),
    d->vcirc_ui.table);
  gtk_widget_show (d->vcirc_ui.table);

  /*-- da and label are freed in varcircle_clear --*/
  d->vcirc_ui.vb = NULL;
  d->vcirc_ui.da = NULL;
  d->vcirc_ui.label = NULL;
  d->vcirc_ui.da_pix = NULL;

/*
  k = 0;
  for (i=0; i<d->vcirc_ui.tnrows; i++) {
    for (j=0; j<d->vcirc_ui.tncols; j++) {
*/
  for (j=0; j<d->ncols; j++) {
    vb = varcircle_create (j, d, gg);
    varcircle_pack (vb, d);
  }
/*
      k++;
      if (k == d->ncols) break;
    }
  }
*/

  /*-- the second child of the vbox: an hbox with buttons --*/
  d->vcirc_ui.hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.vbox), d->vcirc_ui.hbox,
    false, false, 2);
  gtk_widget_show (d->vcirc_ui.hbox);

  /* -- a drawing area to place next to the manip button as a color key --*/
  da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (da), 8, 8);
  gtk_widget_set_events (da, GDK_EXPOSURE_MASK);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), da,
    false, false, 2);
  GGobi_widget_set (da, gg, true);
  gtk_signal_connect (GTK_OBJECT (da), "expose_event",
    GTK_SIGNAL_FUNC (da_manip_expose_cb), d);
  gtk_widget_show (da);

  d->vcirc_ui.manip_btn = gtk_button_new_with_label ("Manip");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), d->vcirc_ui.manip_btn,
    "Click here, then click on the variable you wish to manipulate",
    NULL);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), d->vcirc_ui.manip_btn,
    true, true, 2);
  gtk_signal_connect (GTK_OBJECT (d->vcirc_ui.manip_btn),
    "button_press_event", GTK_SIGNAL_FUNC (manip_select_cb), d);
  gtk_widget_show (d->vcirc_ui.manip_btn);

#ifdef FREEZE_IMPLEMENTED
  /* -- a drawing area to place next to the freeze button as a color key --*/
  da = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (da), 8, 8);
  gtk_widget_set_events (da, GDK_EXPOSURE_MASK);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), da,
    false, false, 2);
  GGobi_widget_set (da, gg, true);
  gtk_signal_connect (GTK_OBJECT (da), "expose_event",
    GTK_SIGNAL_FUNC (da_freeze_expose_cb), d);
  gtk_widget_show (da);

  d->vcirc_ui.freeze_btn = gtk_button_new_with_label ("Freeze");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), d->vcirc_ui.freeze_btn,
    "Click here, then click on the variable you wish to freeze",
    NULL);
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.hbox), d->vcirc_ui.freeze_btn,
    true, true, 2);
  gtk_signal_connect (GTK_OBJECT (d->vcirc_ui.freeze_btn),
    "button_press_event", GTK_SIGNAL_FUNC (freeze_select_cb), d);
  gtk_widget_show (d->vcirc_ui.freeze_btn);
#endif
}

void
varcircles_delete (gint nc, gint jvar, datad *d, ggobid *gg)
{
  gint j;
  GtkWidget *w;
  GdkPixmap *pix;

  if (nc > 0 && nc < d->ncols) {  /*-- forbid deleting every circle --*/
    for (j=jvar; j<jvar+nc; j++) {
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

#ifdef CIRCLES_IN_TABLE
  /*-- this may not be enough; time will tell --*/
  varcircles_layout_reset (d->ncols, d, gg);
#endif
}

/*-- this destroys them all -- it's not yet called anywhere --*/
void
varcircles_clear (ggobid *gg) {
  GtkWidget *w;
  gint j;
  GSList *l;
  datad *d;
  GdkPixmap *pix;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    for (j=0; j<d->vcirc_ui.nvars; j++) {  /*-- variable not initialized --*/
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
varcircle_sel_cb (GtkWidget *w, GdkEvent *event, gint jvar)
{
  ggobid *gg = GGobiFromWidget (w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  splotd *sp = gg->current_splot;
  datad *d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);

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
    varsel (w, cpanel, sp, jvar, -1 /* toggle */, mouse,
      alt_mod, ctrl_mod, shift_mod, d, gg);
    varcircles_refresh (d, gg);
    return true;
  }

  return false;
}

static GtkWidget *
varcircle_create (gint j, datad *d, ggobid *gg)
{
  GtkWidget *vb, *lbl, *da;
  vartabled *vt = vartable_element_get (j, d);

  vb = gtk_hbox_new (false, 0);
  d->vcirc_ui.vb = g_slist_append (d->vcirc_ui.vb, vb);
  gtk_container_border_width (GTK_CONTAINER (vb), 1);

  /*-- a drawing area to contain the variable circle --*/
  da = gtk_drawing_area_new ();
  d->vcirc_ui.da = g_slist_append (d->vcirc_ui.da, da);
  gtk_drawing_area_size (GTK_DRAWING_AREA (da),
                         VAR_CIRCLE_DIAM+2, VAR_CIRCLE_DIAM+2);
  gtk_widget_set_events (da, GDK_EXPOSURE_MASK
             | GDK_ENTER_NOTIFY_MASK
             | GDK_LEAVE_NOTIFY_MASK
             | GDK_BUTTON_PRESS_MASK
             | GDK_BUTTON_RELEASE_MASK);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), da,
    "Click left to select or deselect", NULL);

  gtk_signal_connect (GTK_OBJECT (da), "expose_event",
    GTK_SIGNAL_FUNC (da_expose_cb), GINT_TO_POINTER (j));
  gtk_signal_connect (GTK_OBJECT (da), "button_press_event",
    GTK_SIGNAL_FUNC (varcircle_sel_cb), GINT_TO_POINTER (j));
  gtk_object_set_data (GTK_OBJECT (da), "datad", d);
  GGobi_widget_set (GTK_WIDGET (da), gg, true);
  /*gtk_container_add (GTK_CONTAINER (vb), da);*/
  gtk_box_pack_start (GTK_BOX (vb), da, false, false, 0);

  /*-- label --*/
  lbl = gtk_label_new (vt->collab);
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, .5);  /*- x: left, y: middle --*/
  d->vcirc_ui.label = g_slist_append (d->vcirc_ui.label, lbl);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
    lbl, "Click left on the circle to select or deselect", NULL);
  gtk_object_set_data (GTK_OBJECT (lbl), "datad", d);
  GGobi_widget_set (GTK_WIDGET (lbl), gg, true);
  /*gtk_container_add (GTK_CONTAINER (vb), lbl);*/
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  gtk_widget_show_all (vb);
  return (vb);
}

static void
varcircle_pack (GtkWidget *vb, datad *d)
{
  gtk_box_pack_start (GTK_BOX (d->vcirc_ui.table), vb, false, false, 2);

#ifdef CIRCLES_IN_TABLE
  gtk_table_attach (GTK_TABLE (d->vcirc_ui.table),
    vb, j, j+1, i, i+1,
    GTK_FILL, GTK_FILL, 0, 0);
#endif
}

void
varcircles_refresh (datad *d, ggobid *gg) {
  gint j;
  GtkWidget *da;

  for (j=0; j<d->ncols; j++) {
    da = varcircles_get_nth (DA, j, d);
    if (GTK_WIDGET_REALIZED (da) && GTK_WIDGET_VISIBLE (da))
      varcircle_draw (j, d, gg);
  }
}

void
varcircle_draw (gint jvar, datad *d, ggobid *gg)
{
  gboolean chosen = false;
  splotd *sp = gg->current_splot;
  displayd *display;
  cpaneld *cpanel;
  gint k, len;
  GtkWidget *da = varcircles_get_nth (DA, jvar, d);
  GdkPixmap *da_pix;

  if (sp == NULL || jvar < 0 || jvar >= d->ncols)
    return;  /*-- return --*/

  display = (displayd *) sp->displayptr;

  if (display == NULL || display->d != d)
    return;  /*-- return --*/

  cpanel = &display->cpanel;

  if (gg->selvarfg_GC == NULL) 
    init_var_GCs (da, gg);

  if ((len=g_slist_length (d->vcirc_ui.da_pix)) < d->ncols) {
    for (k=len; k<d->ncols; k++) {
      d->vcirc_ui.da_pix = g_slist_append (d->vcirc_ui.da_pix,
        gdk_pixmap_new (da->window, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1, -1));
      /*
       * clear and initialize each pixmap here, because they may be
       * exposed out of sequence, and then noise is drawn to the
       * variable circle on the screen.
      */
      da_pix = g_slist_nth_data (d->vcirc_ui.da_pix, k);
      gdk_draw_rectangle (da_pix, gg->unselvarbg_GC, true,
        0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
      gdk_draw_arc (da_pix, gg->selvarbg_GC, true,
        0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
        0, 64 * 360);
      gdk_draw_arc (da_pix, gg->unselvarfg_GC, false,
        0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
    }
  }

  da_pix = g_slist_nth_data (d->vcirc_ui.da_pix, jvar);

  /*-- clear the pixmap --*/
  gdk_draw_rectangle (da_pix, gg->unselvarbg_GC, true,
                      0, 0, VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);

  /*-- add a filled circle for the background --*/
  gdk_draw_arc (da_pix, gg->selvarbg_GC, true,
                0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);

  /*-- add the appropriate line --*/
  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    GtkGGobiExtendedDisplayClass *klass;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
    if(klass->varcircle_draw)
      chosen = klass->varcircle_draw(display, jvar, da_pix, gg);
  }

  /*
   * add an open circle for the outline
  */
  if (chosen) {
    gdk_draw_arc (da_pix, gg->selvarfg_GC, false,
      0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
  } else {
    gdk_draw_arc (da_pix, gg->unselvarfg_GC, false,
      0, 0, VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM, 0, 64 * 360);
  }

  /*
   * copy the pixmap to the window
  */
  gdk_draw_pixmap (da->window, gg->unselvarfg_GC,
    da_pix, 0, 0, 0, 0,
    VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
}

void tour_draw_circles (datad *d, ggobid *gg)
{
  gint j;

  for (j=0; j<d->ncols; j++) {
    varcircle_draw (j, d, gg);
  }
}

gboolean
da_expose_cb (GtkWidget *w, GdkEventExpose *event, gpointer cbd)
{
  ggobid *gg = GGobiFromWidget (w, true);
  gint j = GPOINTER_TO_INT (cbd);
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (w), "datad");
  GtkWidget *da = varcircles_get_nth (DA, j, d);
  GdkPixmap *da_pix = g_slist_nth_data (d->vcirc_ui.da_pix, j);

  if (j >= d->ncols)  
    return false;

  if (da_pix == NULL) {
    varcircle_draw (j, d, gg); 
  } else {
    gdk_draw_pixmap (da->window, gg->unselvarfg_GC,
      da_pix, 0, 0, 0, 0,
      VAR_CIRCLE_DIAM+1, VAR_CIRCLE_DIAM+1);
  }

  return true;
}

/*-- used in cloning and appending variables; see vartable.c --*/
void
varcircles_add (gint nc, datad *d, ggobid *gg) 
{
  gint j;
  GtkWidget *vb;
  gint n = g_slist_length (d->vcirc_ui.vb);
  
  /*-- create the variable circles --*/
  for (j=n; j<nc; j++) {
    vb = varcircle_create (j, d, gg);
    /*varcircle_pack (vb, d);*/
  }

#ifdef CIRCLES_IN_TABLE
  varcircles_layout_reset (nc, d, gg);
#endif

  gtk_widget_show_all (gg->varpanel_ui.notebook);
}

