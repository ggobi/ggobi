/* varpanel_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "noop-toggle.h"

#include "vars.h"
#include "externs.h"


static gchar *varpanel_names[] = {"xtoggle", "ytoggle", "ztoggle", "label"};

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/


/*-- return the hbox --*/
GtkWidget *
varpanel_container_get_nth (gint jvar, datad *d)
{
  GtkWidget *w;
  w = (GtkWidget *) g_slist_nth_data (d->vcbox_ui.box, jvar);
  return w;
}
GtkWidget *
varpanel_widget_get_nth (gint jbutton, gint jvar, datad *d)
{
  GtkWidget *box, *child;
  box = (GtkWidget *) varpanel_container_get_nth (jvar, d);
  child = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (box),
    varpanel_names[jbutton]);
  return child;
}
void
varpanel_label_set (gint j, datad *d)
{
  GtkWidget *label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);
  vartabled *vt = vartable_element_get (j, d);

  gtk_label_set_text (GTK_LABEL (GTK_BIN (label)->child), vt->collab_tform);
}

GtkWidget *
varpanel_widget_set_visible (gint jbutton, gint jvar, gboolean show, datad *d)
{
  GtkWidget *box, *child;
  gboolean visible;

  box = (GtkWidget *) varpanel_container_get_nth (jvar, d);
  child = (GtkWidget *) gtk_object_get_data (GTK_OBJECT (box),
    varpanel_names[jbutton]);

  visible = GTK_WIDGET_VISIBLE (child);
  if (visible != show) {
    if (show) gtk_widget_show (child);
    else gtk_widget_hide (child);
  }
  
  return child;
}

void
varpanel_delete_nth (gint jvar, datad *d)
{
  GtkWidget *box = varpanel_container_get_nth (jvar, d);
  if (box != NULL) {
    d->vcbox_ui.box = g_slist_remove (d->vcbox_ui.box, (gpointer) box);
    gtk_widget_destroy (box);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varpanel_toggle_set_active (gint jbutton, gint jvar, gboolean active, datad *d)
{
  gboolean active_prev;
  GtkWidget *w;


  if (jvar >= 0 && jvar < d->ncols) {
    w = varpanel_widget_get_nth (jbutton, jvar, d);

    if (w && GTK_WIDGET_REALIZED (w)) {

      active_prev = GTK_TOGGLE_BUTTON (w)->active;

      if (active != active_prev) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), active);
      }
    }
  }
}

void
varsel (GtkWidget *w, cpaneld *cpanel, splotd *sp, gint jvar,
  gint toggle, gint mousebtn,
  gint alt_mod, gint ctrl_mod, gint shift_mod, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw = false;

  if (display == NULL || !GTK_IS_GGOBI_WINDOW_DISPLAY(display) || 
            !GTK_IS_WIDGET (GTK_GGOBI_WINDOW_DISPLAY(display)->window)) 
  {
    g_printerr ("Bug?  I see no active display\n");
    return ;
  }

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     redraw = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->variable_select(w, display, sp, jvar, toggle, mousebtn, cpanel, gg);
  }

  gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[VARIABLE_SELECTION_SIGNAL], 
    display->d, jvar, sp);

  /*-- overkill for scatmat: could redraw one row, one column --*/
  /*-- overkill for parcoords: need to redraw at most 3 plots --*/
  if (redraw) {
    display_tailpipe (display, FULL, gg);

    if (viewmode_get (gg) == BRUSH) {
      display_tailpipe (display, NONE, gg);
      brush_once_and_redraw (true, sp, display, gg); /* binning ok */
    }
  }
}

/*-------------------------------------------------------------------------*/

void
varpanel_show_page (displayd *display, ggobid *gg)
{
  GtkNotebook *nb;
  gint page, page_new;
  datad *d = display->d;
  GList *l, *children;
  GtkWidget *child, *tab_label;

  if (gg->varpanel_ui.notebook == NULL)
    return;

  nb = GTK_NOTEBOOK (gg->varpanel_ui.notebook);
  page = gtk_notebook_get_current_page (nb);

  if (page < 0)
    return;

  page_new = 0;
  children = gtk_container_children (GTK_CONTAINER (gg->varpanel_ui.notebook));
  for (l = children; l; l = l->next) {
    child = l->data;
    tab_label = (GtkWidget *) gtk_notebook_get_tab_label (nb, child);
    if (tab_label && GTK_IS_LABEL (tab_label)) {
      if (strcmp (GTK_LABEL (tab_label)->label, d->name) == 0) {
        if (page != page_new) {
          gtk_notebook_set_page (nb, page_new);
          break;
        }
      }
    }
    page_new++;
  }
}

void
varpanel_switch_page_cb (GtkNotebook *notebook, GtkNotebookPage *page,
  gint page_num, ggobid *gg)
{
  varpanel_reinit (gg);
}


/*-- here's where we'd reset what's selected according to the current mode --*/
void
varpanel_refresh (displayd *display, ggobid *gg) 
{
  splotd *sp = gg->current_splot;
  datad *d;

  if (display) {
    d = display->d;

    if (sp != NULL && d != NULL) {
      if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
        GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->varpanel_refresh(display, sp, d);
      }
    }
  } else {
    /*
     * if there's no display, but I can get hold of a datad,
     * use it to turn off all the buttons.
    */
    if (g_slist_length (gg->d) > 0) {
      d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
      if (d) {
        gint j;
        for (j=0; j<d->ncols; j++) {
          /*varpanel_widget_set_visible (VARSEL_X, j, false, d);*/
          varpanel_toggle_set_active (VARSEL_X, j, false, d);
          varpanel_widget_set_visible (VARSEL_Y, j, false, d);
          varpanel_toggle_set_active (VARSEL_Y, j, false, d);
          varpanel_toggle_set_active (VARSEL_Z, j, false, d);
          varpanel_widget_set_visible (VARSEL_Z, j, false, d);
        }
      }
    }
  }
}

/*-- responds to a button_press_event --*/
static gint
varsel_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  displayd *display = gg->current_display;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel;

  if (display == NULL)
    return 0;

  cpanel = &display->cpanel;

  if (d != display->d)
    return true;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint mousebutton = -1;
    gint togglebutton = -1;
    gboolean alt_mod, shift_mod, ctrl_mod;
    gint j, jvar;

    jvar = -1;
    for (j=0; j<d->ncols; j++) {
      if (varpanel_widget_get_nth (VARSEL_X, j, d) == w) {
        togglebutton = VARSEL_X;
        jvar = j;
        break;
      } else if (varpanel_widget_get_nth (VARSEL_Y, j, d) == w) {
        togglebutton = VARSEL_Y;
        jvar = j;
        break;
      } else if (varpanel_widget_get_nth (VARSEL_Z, j, d) == w) {
        togglebutton = VARSEL_Z;
        jvar = j;
        break;
      } else if (varpanel_widget_get_nth (VARSEL_LABEL, j, d) == w) {
        togglebutton = -1;
        mousebutton = bevent->button;
        jvar = j;
        break;
      }
    }

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    /*-- general variable selection --*/
    varsel (w, cpanel, sp, jvar, togglebutton, mousebutton,
      alt_mod, ctrl_mod, shift_mod, d, gg);
    varpanel_refresh (display, gg);
    return true;
  }

  return false;
}

/*-------------------------------------------------------------------------*/
/*                  adding and deleting variables                          */
/*-------------------------------------------------------------------------*/

static void
varpanel_add_row (gint j, datad *d, ggobid *gg) 
{
  vartabled *vt = vartable_element_get (j, d);
  GtkWidget *box, *xw, *yw, *zw, *label;

  box = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (d->vcbox_ui.vbox),
    box, false, false, 0);

  xw = gtk_noop_toggle_button_new_with_label (" X ");
  gtk_box_pack_start (GTK_BOX (box), xw, false, false, 2);
  GGobi_widget_set (xw, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_X], xw);
  gtk_signal_connect (GTK_OBJECT (xw),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);
  gtk_widget_show (xw);

  yw = gtk_noop_toggle_button_new_with_label (" Y ");
  gtk_box_pack_start (GTK_BOX (box), yw, false, false, 2);
  GGobi_widget_set (yw, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_Y], yw);
  gtk_signal_connect (GTK_OBJECT (yw),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);
  gtk_widget_show (yw);

  zw = gtk_noop_toggle_button_new_with_label (" Z ");
  gtk_box_pack_start (GTK_BOX (box), zw, false, false, 2);
  GGobi_widget_set (zw, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_Z], zw);
  gtk_signal_connect (GTK_OBJECT (zw),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);
  /*-- hide this widget by default --*/

  /*-- the label is actually a button, with the old behavior --*/
  label = gtk_button_new_with_label (vt->collab_tform);
  gtk_button_set_relief (GTK_BUTTON (label), GTK_RELIEF_NONE);
  GGobi_widget_set (label, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_LABEL], label);
  gtk_signal_connect (GTK_OBJECT (label),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);
  gtk_box_pack_start (GTK_BOX (box), label, false, false, 2);
  gtk_widget_show (label);

  d->vcbox_ui.box = g_slist_append (d->vcbox_ui.box, box);
  gtk_widget_show (box);
}

void
varpanel_widgets_add (gint nc, datad *d, ggobid *gg) 
{
  gint j;
  gint n = g_slist_length (d->vcbox_ui.box);
  
  /*-- create the variable widgets --*/
  for (j=n; j<nc; j++)
    varpanel_add_row (j, d, gg);
}

/*-------------------------------------------------------------------------*/
/*                  initialize and populate the var panel                  */
/*-------------------------------------------------------------------------*/

/*
 * build the notebook to contain a paned widget which will contain
 * checkboxes on the left and circles/rectangles on the right
*/
void
varpanel_make (GtkWidget *parent, ggobid *gg) {

  gg->varpanel_ui.layoutByRow = true;  /*-- for the circles --*/
  gg->selvarfg_GC = NULL;

  gg->varpanel_ui.notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
    GTK_POS_TOP);
  gtk_signal_connect (GTK_OBJECT (gg->varpanel_ui.notebook), "switch-page",
    GTK_SIGNAL_FUNC (varpanel_switch_page_cb), gg);

  gtk_box_pack_start (GTK_BOX (parent), gg->varpanel_ui.notebook,
    true, true, 2);

  gtk_widget_show (gg->varpanel_ui.notebook);
}

void
varpanel_clear (datad *d, ggobid *gg)
{
  GList *pages;
  gint npages;
  gint k;

  if (gg->varpanel_ui.notebook != NULL &&
      GTK_WIDGET_REALIZED (gg->varpanel_ui.notebook))
  {
    pages = gtk_container_children (GTK_CONTAINER (gg->varpanel_ui.notebook));
    npages = g_list_length (pages);
    for (k=0; k< npages; k++)
      gtk_notebook_remove_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook), 0);
  }
}


/*-- for each datad, a scrolled window, vbox, hbox, togglebuttons and label --*/
void varpanel_populate (datad *d, ggobid *gg)
{
  gint j, nd;

  nd = ndatad_with_vars_get (gg); 

  /*-- we don't know the length of gg->d when the notebook is created --*/
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
    nd > 1);

  /*-- only add a tab if there are variables --*/
  if (g_slist_length (d->vartable) > 0) {
    /*-- create a paned widget --*/
    d->varpanel_ui.hpane = gtk_hpaned_new ();
    gtk_paned_set_handle_size (GTK_PANED(d->varpanel_ui.hpane), 0);
    gtk_paned_set_gutter_size (GTK_PANED(d->varpanel_ui.hpane), 0);
    /*-- set the handle position all the way to the right --*/
    gtk_paned_set_position (GTK_PANED(d->varpanel_ui.hpane), -1);

    gtk_notebook_append_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                              d->varpanel_ui.hpane,
                              gtk_label_new (d->name));

    /*-- create an ebox, and put it in the hpane --*/
    d->vcbox_ui.ebox = gtk_event_box_new ();
    gtk_paned_pack1 (GTK_PANED(d->varpanel_ui.hpane),
      d->vcbox_ui.ebox, true, false);

    /*-- create a scrolled window, and put it in the ebox --*/
    d->vcbox_ui.swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->vcbox_ui.swin),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_container_add (GTK_CONTAINER (d->vcbox_ui.ebox), d->vcbox_ui.swin);

    /*-- add a vbox to the swin --*/
    d->vcbox_ui.vbox = gtk_vbox_new (false, 0);
    gtk_scrolled_window_add_with_viewport (
      GTK_SCROLLED_WINDOW (d->vcbox_ui.swin),
      d->vcbox_ui.vbox);
  
    gtk_widget_show_all (d->varpanel_ui.hpane);
    gdk_flush ();

    d->vcbox_ui.box = NULL;
    for (j=0; j<d->ncols; j++)
      varpanel_add_row (j, d, gg);
  }
}


/*-------------------------------------------------------------------------*/
/*                          API; not used                                  */
/*-------------------------------------------------------------------------*/

void
GGOBI(selectScatterplotX) (GtkWidget *w, gint jvar, ggobid *gg) 
{
  displayd *display = gg->current_display;
  GtkGGobiExtendedDisplayClass *klass;

  if(!GTK_IS_GGOBI_EXTENDED_DISPLAY(display))
    return;
  klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
  if(klass->select_X)
    klass->select_X(w, display, jvar, gg);
}

/*-------------------------------------------------------------------------*/
/*                    context-sensitive tooltips                           */
/*-------------------------------------------------------------------------*/

void
varpanel_tooltips_set (displayd *display, ggobid *gg) 
{
  gint j;
  datad *d;
  GtkWidget *wx, *wy, *wz, *label;

  if (display == NULL) {
    d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
    if (d) {
      for (j=0; j<d->ncols; j++) {
        if ((wx = varpanel_widget_get_nth (VARSEL_X, j, d)) == NULL)
          break;
        label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
          "Unable to plot without a display",
          NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
          "Unable to plot without a display",
          NULL);
      }
    }
  } else {
    d = display->d;

    /*-- for each variable, current datad only --*/
    for (j=0; j<d->ncols; j++) {
      if ((wx = varpanel_widget_get_nth (VARSEL_X, j, d)) == NULL)
        break;

      wy = varpanel_widget_get_nth (VARSEL_Y, j, d);
      wz = varpanel_widget_get_nth (VARSEL_Z, j, d);
      label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);
    
      if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
         GtkGGobiExtendedDisplayClass *klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
         if(klass->varpanel_tooltips_set)
           klass->varpanel_tooltips_set(display, gg, wx, wy, wz, label);
      }
    }
  }
}
