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

#define VARSEL_X 0
#define VARSEL_Y 1
#define VARSEL_LABEL 2
static gchar *varpanel_names[] = {"xtoggle", "ytoggle", "label"};

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/


/*-- return the hbox --*/
static GtkWidget *
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
        /*GTK_TOGGLE_BUTTON (w)->active = active;*/
        /*gtk_widget_queue_draw (w);*/
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), active);
      }
    }
  }
}

void
varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint btn,
  gint alt_mod, gint ctrl_mod, gint shift_mod, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw = false;
  gint jvar_prev = -1;

  if (display == NULL || !GTK_IS_WIDGET (display->window)) {
    g_printerr ("Bug?  I see no active display\n");
    return ;
  }
  
  switch (display->displaytype) {

    case parcoords:
      redraw = parcoords_varsel (cpanel, sp, jvar, &jvar_prev, gg);
    break;

    case scatmat:
      redraw = scatmat_varsel_simple (cpanel, sp, jvar, &jvar_prev, gg);
    break;

    case tsplot:
      redraw = tsplot_varsel (cpanel, sp, btn, jvar, &jvar_prev, gg);
    break;

    case scatterplot:
      switch (cpanel->projection) {
        case P1PLOT:
          redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
          if (viewmode_get (gg) == BRUSH && cpanel->br_mode == BR_TRANSIENT)
            reinit_transient_brushing (display, gg);
        break;
        case XYPLOT:
          redraw = xyplot_varsel (sp, jvar, &jvar_prev, btn);
          if (viewmode_get (gg) == BRUSH && cpanel->br_mode == BR_TRANSIENT)
            reinit_transient_brushing (display, gg);
        break;
        case TOUR2D:
          tour2d_varsel (jvar, btn, d, gg);
        break;
        case TOUR1D:
          tour1d_varsel (jvar, btn, d, gg);
        break;
        case COTOUR:
          tourcorr_varsel (jvar, btn, d, gg);
        break;
        /*-- to pacify compiler if we change these to an enum --*/
        case NULLMODE:
        case ROTATE:
        case SCALE:
        case BRUSH:
        case IDENT:
        case EDGEED:
        case MOVEPTS:
        case SCATMAT:
        case PCPLOT:
        case TSPLOT:
#ifdef BARCHART_IMPLEMENTED
        case BARCHART:
#endif
        case NMODES:
        break;
    }
    break;

#ifdef BARCHART_IMPLEMENTED
    case barchart:
      redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
      if (redraw) {
        displayd *display = (displayd *) sp->displayptr;
        datad *d = display->d;

        barchart_clean_init (sp);
        barchart_recalc_counts (sp,d,gg);
      }
    break;
#endif

    case unknown_display_type:
    break;
  }

    /* Change the source object for this event to something more meaningful! */
  gtk_signal_emit(GTK_OBJECT(gg->main_window),
    GGobiSignals[VARIABLE_SELECTION_SIGNAL], jvar, display->d, sp, gg);

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
varpanel_refresh (displayd *display, ggobid *gg) {
  gint j;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;
  GList *l;
  datad *d = display->d;

  if (sp != NULL && d != NULL) {

    switch (display->displaytype) {

      case parcoords:
        for (j=0; j<d->ncols; j++) {
          varpanel_toggle_set_active (VARSEL_X, j, false, d);
          varpanel_toggle_set_active (VARSEL_Y, j, false, d);
          varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        }

        l = display->splots;
        while (l) {
          j = ((splotd *) l->data)->p1dvar;
          varpanel_toggle_set_active (VARSEL_X, j, true, d);
          l = l->next;
        }
      break;

      case scatmat:
        for (j=0; j<d->ncols; j++) {
          varpanel_toggle_set_active (VARSEL_X, j, false, d);
          varpanel_toggle_set_active (VARSEL_Y, j, false, d);
          varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        }
        l = display->scatmat_cols;  /*-- assume rows = cols --*/
        while (l) {
          j = GPOINTER_TO_INT (l->data);
          varpanel_toggle_set_active (VARSEL_X, j, true, d);
          l = l->next;
        }
      break;

      case tsplot:
        for (j=0; j<d->ncols; j++) {
          varpanel_toggle_set_active (VARSEL_X, j, false, d);
          varpanel_toggle_set_active (VARSEL_Y, j, false, d);
          varpanel_widget_set_visible (VARSEL_Y, j, true, d);
        }

        l = display->splots;
        while (l) {
          j = ((splotd *) l->data)->xyvars.y;
          varpanel_toggle_set_active (VARSEL_Y, j, true, d);
          j = ((splotd *) l->data)->xyvars.x;
          varpanel_toggle_set_active (VARSEL_X, j, true, d);
          l = l->next;
        }
      break;

      case scatterplot:
        switch (cpanel->projection) {
          case P1PLOT:
            for (j=0; j<d->ncols; j++) {
              varpanel_toggle_set_active (VARSEL_Y, j, false, d);
              varpanel_widget_set_visible (VARSEL_Y, j, false, d);

              varpanel_toggle_set_active (VARSEL_X, j, j == sp->p1dvar, d);
            }
          break;
          case XYPLOT:
            for (j=0; j<d->ncols; j++) {
              varpanel_toggle_set_active (VARSEL_X, j, 
                (j == sp->xyvars.x), d);
              varpanel_widget_set_visible (VARSEL_Y, j, true, d);
              varpanel_toggle_set_active (VARSEL_Y, j, 
                (j == sp->xyvars.y), d);
            }
          break;
          /*-- to pacify compiler --*/
          case NULLMODE:
          case TOUR2D:
          case TOUR1D:
          case COTOUR:
          case ROTATE:
          case SCALE:
          case BRUSH:
          case IDENT:
          case EDGEED:
          case MOVEPTS:
          case SCATMAT:
          case PCPLOT:
          case TSPLOT:
#ifdef BARCHART_IMPLEMENTED
          case BARCHART:
#endif
          case NMODES:
          break;
      }
      break;

#ifdef BARCHART_IMPLEMENTED
      case barchart:
        for (j=0; j<d->ncols; j++) {
          varpanel_toggle_set_active (VARSEL_X, j, (j == sp->p1dvar), d);
          varpanel_toggle_set_active (VARSEL_Y, j, false, d);
          varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        }
      break;
#endif

      case unknown_display_type:
      break;
    }
  }
/*
    }
  }
*/
}

/*-- responds to a button_press_event --*/
static gint
varsel_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  ggobid *gg = GGobiFromWidget (w, true);
  displayd *display = gg->current_display;
  cpaneld *cpanel = &display->cpanel;
  splotd *sp = gg->current_splot;
  gint xory;

  if (d != display->d)
    return true;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint button = bevent->button;
    gboolean alt_mod, shift_mod, ctrl_mod;
    gint j, jvar;

    jvar = -1;
    for (j=0; j<d->ncols; j++) {
      if (varpanel_widget_get_nth (VARSEL_X, j, d) == w) {
        xory = VARSEL_X;
        jvar = j;
        break;
      } else if (varpanel_widget_get_nth (VARSEL_Y, j, d) == w) {
        xory = VARSEL_Y;
        jvar = j;
        break;
      } else if (varpanel_widget_get_nth (VARSEL_LABEL, j, d) == w) {
        xory = -1;
        jvar = j;
        break;
      }
    }
   /*-- emulate the old behavior by translating xory into button --*/
   if (xory > -1)
     button = (xory == VARSEL_X) ? 1 : 2;

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, d, gg);
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
  GtkWidget *box, *xw, *yw, *label;

  box = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (d->vcbox_ui.vbox),
    box, false, false, 0);

  xw = gtk_noop_toggle_button_new_with_label (" X ");
  gtk_box_pack_start (GTK_BOX (box), xw, false, false, 2);
  GGobi_widget_set (xw, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_X], xw);
  gtk_signal_connect (GTK_OBJECT (xw),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);

  yw = gtk_noop_toggle_button_new_with_label (" Y ");
  gtk_box_pack_start (GTK_BOX (box), yw, false, false, 2);
  GGobi_widget_set (yw, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_Y], yw);
  gtk_signal_connect (GTK_OBJECT (yw),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);

  /*-- the label is actually a button, with the old behavior --*/
  label = gtk_button_new_with_label (vt->collab_tform);
  gtk_button_set_relief (GTK_BUTTON (label), GTK_RELIEF_NONE);
  GGobi_widget_set (label, gg, true);
  gtk_object_set_data (GTK_OBJECT(box), varpanel_names[VARSEL_LABEL], label);
  gtk_signal_connect (GTK_OBJECT (label),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);
  gtk_box_pack_start (GTK_BOX (box), label, false, false, 2);

  d->vcbox_ui.box = g_slist_append (d->vcbox_ui.box, box);
  gtk_widget_show_all (box);
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
 * build the notebook to contain an ebox which will be switched
 * between togglebuttons and circles
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
    /*-- create an ebox: needed for tooltips? --*/
    d->varpanel_ui.ebox = gtk_event_box_new ();
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                              d->varpanel_ui.ebox,
                              gtk_label_new (d->name));

    /*-- create a scrolled window, and put it in the ebox --*/
    d->vcbox_ui.swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->vcbox_ui.swin),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_container_add (GTK_CONTAINER (d->varpanel_ui.ebox), d->vcbox_ui.swin);

    /*-- add a vbox to the swin --*/
    d->vcbox_ui.vbox = gtk_vbox_new (false, 0);
    gtk_scrolled_window_add_with_viewport (
      GTK_SCROLLED_WINDOW (d->vcbox_ui.swin),
      d->vcbox_ui.vbox);
  
    gtk_widget_show_all (d->varpanel_ui.ebox);
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
GGOBI(selectScatterplotX) (gint jvar, ggobid *gg) 
{
  displayd *display = gg->current_display;
  if (display->displaytype != scatterplot)
    return;
  else {

    datad *d = display->d;
    splotd *sp = (splotd *) display->splots->data;
    cpaneld *cpanel = &display->cpanel;

    varsel (cpanel, sp, jvar, 1, false, false, false, d, gg);
  }
}

/*-------------------------------------------------------------------------*/
/*                    context-sensitive tooltips                           */
/*-------------------------------------------------------------------------*/

void
varpanel_tooltips_set (displayd *display, ggobid *gg) 
{
  gint projection = projection_get (gg);
  gint j;
  datad *d = display->d;
  GtkWidget *wx, *wy, *label;

  /*-- for each variable, current datad only --*/
  for (j=0; j<d->ncols; j++) {
    if ((wx = varpanel_widget_get_nth (VARSEL_X, j, d)) == NULL)
      break;
    wy = varpanel_widget_get_nth (VARSEL_Y, j, d);
    label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);
    
    switch (display->displaytype) {

      case parcoords:
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
          "Select to replace/insert/append a variable, or to delete it",
          NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
          "Click to replace/insert/append a variable, or to delete it",
          NULL);
      break;

      case scatmat:
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
          "Select to replace/insert/append a variable, or to delete it",
          NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
          "Click to replace/insert/append a variable, or to delete it",
          NULL);
      break;

      case tsplot:
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
          "Select to replace the horizontal (time) variable.",
          NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
          "Select to replace/insert/append/delete a Y variable.",
          NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
          "Click left to replace the horizontal (time) variable.  Click middle or right to replace/insert/append/delete a Y variable.",
          NULL);
      break;

      case scatterplot:
        switch (projection) {
          case P1PLOT:
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
              "Select to plot",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
              "Click left to plot horizontally, right or middle to plot vertically",
              NULL);
          break;
          case XYPLOT:
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
              "Press to select the horizontally plotted variable",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
              "Press to select the vertically plotted variable",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
              "Click left to select the horizontal variable, middle for vertical",
              NULL);

          break;
          case TOUR2D:
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
              "Click to select a variable to be available for touring",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
              "Click to select a variable to be available for touring",
              NULL);
          break;
          case TOUR1D:
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
              "Click to select a variable to be available for touring",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
              "Click to select a variable to be available for touring",
              NULL);
          break;
          case COTOUR:
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
              "Click to select a variable to be toured horizontally",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
              "Click to select a variable to be toured vertically",
              NULL);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
              "Click to select a variable to be available for touring",
              NULL);
          break;
          /*-- to pacify compiler if we change these to an enum --*/
          case ROTATE:
          case SCALE:
          case BRUSH:
          case IDENT:
          case EDGEED:
          case MOVEPTS:
          case SCATMAT:
          case PCPLOT:
          case TSPLOT:
#ifdef BARCHART_IMPLEMENTED
          case BARCHART:
#endif
          break;
      }
      break;

#ifdef BARCHART_IMPLEMENTED
      case barchart:
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
          "Click to replace a variable",
          NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
          "Click to replace a variable",
          NULL);
      break;
#endif

      case unknown_display_type:
      break;
    }
  }
}
