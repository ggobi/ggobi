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
#include <math.h>

#include "noop-checkbutton.h"

#include "vars.h"
#include "externs.h"

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

void
checkbox_delete_nth (gint jvar, datad *d)
{
  GtkWidget *w;
  w = (GtkWidget *) g_slist_nth_data (d->varpanel_ui.checkbox, jvar);
  if (w != NULL) {
    d->varpanel_ui.checkbox = g_slist_remove (d->varpanel_ui.checkbox,
                                              (gpointer) w);
    gtk_widget_destroy (w);
  }
}


GtkWidget *
checkbox_get_nth (gint jvar, datad *d)
{
  GtkWidget *w;
  w = (GtkWidget *) g_slist_nth_data (d->varpanel_ui.checkbox, jvar);
  return w;
}

void
varlabel_set (gint jvar, datad *d, ggobid *gg)
{
  GtkWidget *w = checkbox_get_nth (jvar, d);
  gtk_label_set_text (GTK_LABEL (GTK_BIN (w)->child),
                      d->vartable[jvar].collab_tform);
}


/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varpanel_checkbutton_set_active (gint jvar, gboolean active, datad *d)
{
  gboolean active_prev;
  GtkWidget *w;

  if (jvar >= 0 && jvar < d->ncols) {
    w = checkbox_get_nth (jvar, d);

    if (GTK_WIDGET_REALIZED (w)) {

      active_prev = GTK_TOGGLE_BUTTON (w)->active;
      GTK_TOGGLE_BUTTON (w)->active = active;

      if (active != active_prev)
        gtk_widget_queue_draw (w);
    }
  }
}


void
varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint btn,
  gint alt_mod, gint ctrl_mod, gint shift_mod, datad *d, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw;
  gint jvar_prev = -1;
  extern void tour2d_varsel (ggobid *, gint, gint);

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

    case scatterplot:
      switch (cpanel->projection) {
        case P1PLOT:
          redraw = p1d_varsel (sp, jvar, &jvar_prev, btn);
        break;
        case XYPLOT:
          redraw = xyplot_varsel (sp, jvar, &jvar_prev, btn);
        break;
        case TOUR2D:
          tour2d_varsel (gg, jvar, btn);
        break;
        case TOUR1D:
          tour1d_varsel (gg, jvar, btn);
        break;
        default:
        break;
    }
    break;
  }

  /*-- overkill for scatmat: could redraw one row, one column --*/
  /*-- overkill for parcoords: need to redraw at most 3 plots --*/
/* this is redrawing before it has the new window sizes, so the
 * lines aren't right */
  if (redraw) {
    display_tailpipe (display, gg);
  }
}

/*-------------------------------------------------------------------------*/


/*-- here's where we'd reset what's selected according to the current mode --*/
void
varpanel_refresh (ggobid *gg) {
  gint j;
  displayd *display = gg->current_display;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;
  gint nd = g_slist_length (gg->d);
  gint k;

  if (nd > 0 && sp != NULL) {

    for (k=0; k<nd; k++) {
      datad *d = (datad*) g_slist_nth_data (gg->d, k);
      if (display->d != d)
        ;  /*-- we will only deal with the current datad --*/
      else {

        switch (display->displaytype) {

          case parcoords:
          {
            GList *l;
            for (j=0; j<d->ncols; j++)
              varpanel_checkbutton_set_active (j, false, d);

            l = display->splots;
            while (l) {
              j = ((splotd *) l->data)->p1dvar;
              varpanel_checkbutton_set_active (j, true, d);
              l = l->next;
            }
          }
          break;

          case scatmat:
          {
            GList *l;
            for (j=0; j<d->ncols; j++)
              varpanel_checkbutton_set_active (j, false, d);
            l = display->scatmat_cols;  /*-- assume rows = cols --*/
            while (l) {
              j = GPOINTER_TO_INT (l->data);
              varpanel_checkbutton_set_active (j, true, d);
              l = l->next;
            }
          }
          break;

          case scatterplot:
            switch (cpanel->projection) {
              case P1PLOT:
                for (j=0; j<d->ncols; j++)
                  varpanel_checkbutton_set_active (j, (j == sp->p1dvar), d);
              break;
              case XYPLOT:
                for (j=0; j<d->ncols; j++)
                  varpanel_checkbutton_set_active (j,
                    (j == sp->xyvars.x || j == sp->xyvars.y), d);
              break;
              default:
              break;
          }
          break;
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
  cpaneld *cpanel = &display->cpanel;
  splotd *sp = gg->current_splot;

  if (d != display->d)
    return true;

  if (event->type == GDK_BUTTON_PRESS) {
    GdkEventButton *bevent = (GdkEventButton *) event;
    gint button = bevent->button;
    gboolean alt_mod, shift_mod, ctrl_mod;
    gint j, jvar;

    /*-- respond only to button 1 and button 2 --*/
    if (button != 1 && button != 2)
      return false;

    jvar = -1;
    for (j=0; j<d->ncols; j++) {
      if (checkbox_get_nth (j, d) == w) {
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
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, d, gg);
    varpanel_refresh (gg);
    return true;
  }

  return false;
}

/*-------------------------------------------------------------------------*/
/*                  adding and deleting variables                          */
/*-------------------------------------------------------------------------*/

void
varpanel_checkbox_add (gint j, datad *d, ggobid *gg) 
{
  GtkWidget *w = gtk_noop_check_button_new_with_label (d->vartable[j].collab);
  GGobi_widget_set (w, gg, true);
  gtk_signal_connect (GTK_OBJECT (w),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);
  gtk_box_pack_start (GTK_BOX (d->varpanel_ui.vbox),
    w, false, false, 0);

  d->varpanel_ui.checkbox = g_slist_append (d->varpanel_ui.checkbox, w);
  gtk_widget_show (w);
}

/*-- delete nc checkboxes, starting at jcol --*/
void
varpanel_checkboxes_delete (gint nc, gint jcol, datad *d) {
  gint j;
  GtkWidget *w;

  if (nc > 0 && nc < d->ncols) {  /*-- forbid deleting every checkbox --*/
    for (j=jcol; j<jcol+nc; j++) {
      w = checkbox_get_nth (jcol, d);
      d->varpanel_ui.checkbox = g_slist_remove (d->varpanel_ui.checkbox, w);
      gtk_widget_destroy (w);  /*-- maybe not necessary? --*/
    }
  }
}

/*-------------------------------------------------------------------------*/
/*                  initialize and populate the var panel                  */
/*-------------------------------------------------------------------------*/

/*
 * build the notebook to contain one scrolled window and vbox for each d
*/
void
varpanel_make (GtkWidget *parent, ggobid *gg) {

  gg->varpanel_ui.layoutByRow = true;  /*-- for the circles --*/
  gg->selvarfg_GC = NULL;

/*
  gg->varpanel_ui.varpanel_accel_group = gtk_accel_group_new ();
*/
  gg->varpanel_ui.tips = gtk_tooltips_new ();
  
  gg->varpanel_ui.notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
    GTK_POS_TOP);
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
    for (k=0; k<npages; k++)
      gtk_notebook_remove_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook), k);
  }
}


/*-- should rename varpanel_checkboxes_populate --*/
/*-- for each datad, a scrolled window, vbox, and column of check buttons --*/
void varpanel_populate (datad *d, ggobid *gg)
{
  gint j;
  GtkWidget *labelw;

  /*-- we don't know the length of gg->d when the notebook is created --*/
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
    g_slist_length (gg->d) > 1);

  /*-- create a scrolled window --*/
  d->varpanel_ui.swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->varpanel_ui.swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  labelw = (g_slist_length (gg->d) > 1) ? gtk_label_new (d->name) : NULL;
  gtk_notebook_append_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                            d->varpanel_ui.swin, labelw);


  /*-- add an ebox to the scrolled window: needed for tooltips? --*/
  d->varpanel_ui.ebox = gtk_event_box_new ();
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (d->varpanel_ui.swin),
    d->varpanel_ui.ebox);
  
  /*-- add a vbox to the ebox --*/
  d->varpanel_ui.vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (d->varpanel_ui.ebox), d->varpanel_ui.vbox);

  gtk_widget_show_all (d->varpanel_ui.swin);
  gdk_flush ();

  d->varpanel_ui.checkbox = NULL;

  for (j=0; j<d->ncols; j++)
    varpanel_checkbox_add (j, d, gg);
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
varpanel_tooltips_set (ggobid *gg) 
{
  displayd *display = gg->current_display;
  gint projection = projection_get (gg);
  gint j, k;
  gint nd = g_slist_length (gg->d);
  datad *d;

  /*-- for each datad --*/
  for (k=0; k<nd; k++) {
    d = (datad*) g_slist_nth_data (gg->d, k);
    /*-- for each variable --*/
    for (j=0; j<d->ncols; j++) {
      if (checkbox_get_nth (j, d) == NULL)
        break;
      
      switch (display->displaytype) {

        case parcoords:
          gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
            checkbox_get_nth (j, d),
            "Click to replace/insert/append a variable, or to delete it",
            NULL);
        break;

        case scatmat:
          gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
            checkbox_get_nth (j, d),
            "Click to replace/insert/append a variable, or to delete it",
            NULL);
        break;

        case scatterplot:
          switch (projection) {
            case P1PLOT:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                checkbox_get_nth (j, d),
                "Click left to plot horizontally, middle to plot vertically",
                NULL);
            break;
            case XYPLOT:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                checkbox_get_nth (j, d),
                "Click left to select the horizontal variable, middle for vertical",
                NULL);
            break;
            case TOUR2D:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                checkbox_get_nth (j, d),
/*                d->varpanel_ui.checkbox.vals[j],*/
                "Click to select a variable to be available for touring",
                NULL);
            break;
            case TOUR1D:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                checkbox_get_nth (j, d),
/*                d->varpanel_ui.checkbox.vals[j],*/
                "Click to select a variable to be available for touring",
                NULL);
            break;
            default:
            break;
        }
        break;
      }
    }
  }
}
