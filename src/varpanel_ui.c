/* varpanel_ui.c */
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

/*-------------------------------------------------------------------------*/
/*            Listen for display_selected events                           */
/*-------------------------------------------------------------------------*/


/* Update variable selection panel */
void
varpanel_show_page_cb (ggobid * gg, displayd * display, GGobiStage * d)
{
  varpanel_show_page (display, gg);
}

/* Update tooltips */
void
varpanel_tooltips_set_cb (ggobid * gg, displayd * display, GGobiStage * d)
{
  varpanel_tooltips_set (display, gg);
}

/*-------------------------------------------------------------------------*/
/*                         utilities                                       */
/*-------------------------------------------------------------------------*/

static gchar *varpanel_names[] = { "xtoggle", "ytoggle", "ztoggle", "label" };

/*-- return the hbox --*/
GtkWidget *
varpanel_container_get_nth (gint jvar, GGobiStage * d)
{
  GtkWidget *w;
  w = (GtkWidget *) g_slist_nth_data (d->vcbox_ui.box, jvar);
  return w;
}

GtkWidget *
varpanel_widget_get_nth (gint jbutton, gint jvar, GGobiStage * d)
{
  GtkWidget *box, *child;
  box = (GtkWidget *) varpanel_container_get_nth (jvar, d);
  if (!box)
    return (NULL);

  child = (GtkWidget *) g_object_get_data (G_OBJECT (box),
                                           varpanel_names[jbutton]);
  return child;
}

void
varpanel_label_set (GGobiStage * d, gint j)
{
  GtkWidget *label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);
  /*-- the label is actually a button; this is the label --*/
  GtkWidget *labelw;

  if (!label || !GTK_IS_BIN (label))
    return;

  labelw = GTK_BIN (label)->child;

  if (!labelw)
    return;
  /*-- make sure it stays left-aligned --*/
  gtk_misc_set_alignment (GTK_MISC (labelw), 0, .5);
  gtk_label_set_text (GTK_LABEL (labelw), ggobi_stage_get_col_name(d, j));
}

GtkWidget *
varpanel_widget_set_visible (gint jbutton, gint jvar, gboolean show,
                             GGobiStage * d)
{
  GtkWidget *box, *child;
  gboolean visible;

  box = (GtkWidget *) varpanel_container_get_nth (jvar, d);
  child = (GtkWidget *) g_object_get_data (G_OBJECT (box),
                                           varpanel_names[jbutton]);

  visible = GTK_WIDGET_VISIBLE (child);
  if (visible != show) {
    if (show)
      gtk_widget_show (child);
    else
      gtk_widget_hide (child);
  }

  return child;
}

void
varpanel_delete_nth_cb (GGobiStage * d, guint jvar, gpointer user_data) 
{
  varpanel_delete_nth (d, jvar);
}

void
varpanel_delete_nth (GGobiStage * d, gint jvar)
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
varpanel_toggle_set_active (gint jbutton, gint jvar, gboolean active,
                            GGobiStage * d)
{
  gboolean active_prev;
  GtkWidget *w;


  if (jvar >= 0 && jvar < d->n_cols) {
    w = varpanel_widget_get_nth (jbutton, jvar, d);

    if (w && GTK_WIDGET_REALIZED (w)) {

      active_prev = GTK_TOGGLE_BUTTON (w)->active;

      if (active != active_prev) {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), active);
      }
    }
  }
}

void
varsel (GtkWidget * w, cpaneld * cpanel, splotd * sp, gint jvar,
        gint toggle, gint mousebtn,
        gint alt_mod, gint ctrl_mod, gint shift_mod, GGobiStage * d,
        ggobid * gg)
{
  displayd *display = (displayd *) sp->displayptr;
  gboolean redraw = false;

  if (display == NULL || !GGOBI_IS_WINDOW_DISPLAY (display) ||
      !GTK_IS_WIDGET (GGOBI_WINDOW_DISPLAY (display)->window)) {
    g_printerr ("Bug?  I see no active display\n");
    return;
  }

  if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
    redraw =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->variable_select (w, display,
                                                                   sp, jvar,
                                                                   toggle,
                                                                   mousebtn,
                                                                   cpanel,
                                                                   gg);
  }

  /* 
   * I'm not sure what we use this for, but it causes a problem for
   * the composite displays when a plot is deleted.  For example, when
   * a plot is deleted in the parallel coordinates plot, error
   * messages are generated by this signal_emit, presumably because sp
   * is NULL for some subroutine called by this guy.  If sp has to be
   * the current splot, that case is suppressed.  ml
   * 
   * Maybe I just figured it out -- in some of the composite displays,
   * the splot that was current at the start of this routine has been
   * destroyed, so sp is now null, or pointing at junk.  -- dfs
   */
  sp = gg->current_splot;
  if (sp == gg->current_splot)
    g_signal_emit (G_OBJECT (gg), GGobiSignals[VARIABLE_SELECTION_SIGNAL], 0,
                   display->d, jvar, sp);

  /*-- overkill for scatmat: could redraw one row, one column --*/
  /*-- overkill for parcoords: need to redraw at most 3 plots --*/
  if (redraw) {
    display_tailpipe (display, FULL, gg);

    if (imode_get (gg) == BRUSH) {
      display_tailpipe (display, NONE, gg);
      brush_once_and_redraw (true, sp, display, gg);  /* binning ok */
    }
  }
}

/*------------------------------------------------------------------------*/

void
varpanel_show_page (displayd * display, ggobid * gg)
{
  GtkNotebook *nb;
  gint page, page_new;
  GGobiStage *d = display->d, *paged = NULL;
  GList *l, *children;
  GtkWidget *child, *tab_label;
  GtkWidget *pagechild;

  if (gg->varpanel_ui.notebook == NULL)
    return;

  nb = GTK_NOTEBOOK (gg->varpanel_ui.notebook);
  page = gtk_notebook_get_current_page (nb);

  if (page < 0)
    return;

  page_new = 0;
  children =
    gtk_container_get_children (GTK_CONTAINER (gg->varpanel_ui.notebook));
  for (l = children; l; l = l->next) {
    child = l->data;
    tab_label = (GtkWidget *) gtk_notebook_get_tab_label (nb, child);
    if (tab_label && GTK_IS_LABEL (tab_label)) {
      if (strcmp (GTK_LABEL (tab_label)->label, d->name) == 0) {
        if (page != page_new) {

          // Set the buttons on 'page' to be insensitive
          pagechild = gtk_notebook_get_nth_page (nb, page);
          if (pagechild)
            paged = g_object_get_data (G_OBJECT (pagechild), "datad");
          if (paged)
            varpanel_set_sensitive (paged, false, gg);

          // Set the current page, and make its buttons sensitive
          gtk_notebook_set_current_page (nb, page_new);
          varpanel_set_sensitive (d, true, gg);
          if (gg->status_message_func)
            gg->status_message_func ((gchar *) NULL, gg);
          break;
        }
      }
    }
    page_new++;
  }
}

void
varpanel_switch_page_cb (GtkNotebook * notebook, GtkNotebookPage * page,
                         gint page_num, ggobid * gg)
{
  varpanel_reinit (gg);
  gdk_flush ();

  /*-- describe the datad being selected in the console statusbar --*/
  if (gg->status_message_func) {
    GGobiStage *d = (GGobiStage *) g_slist_nth_data (gg->d, page_num);
    if (d) {
      /*gchar *msg = g_strdup_printf ("%s: %d x %d (%s)",
                                    d->name, d->n_rows, d->n_cols,
                                    gg->data_source->uri);*/
      gg->status_message_func (NULL, gg);
      /*g_free (msg);*/
    }
  }
}


/*-- here's where we'd reset what's selected according to the current mode --*/
void
varpanel_refresh (displayd * display, ggobid * gg)
{
  splotd *sp = gg->current_splot;
  GGobiStage *d;

  if (display) {
    d = display->d;

    if (sp != NULL && d != NULL) {
      if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
        GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->varpanel_refresh (display,
                                                                      sp, d);
      }
    }
  }
  else {
    /*
     * if there's no display, but I can get hold of a datad,
     * use it to turn off all the buttons.
     */
    if (g_slist_length (gg->d) > 0) {
      d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
      if (d) {
        gint j;
        for (j = 0; j < d->n_cols; j++) {
          /*varpanel_widget_set_visible (VARSEL_X, j, false, d); */
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
varsel_cb (GtkWidget * w, GdkEvent * event, GGobiStage * d)
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
    for (j = 0; j < d->n_cols; j++) {
      if (varpanel_widget_get_nth (VARSEL_X, j, d) == w) {
        togglebutton = VARSEL_X;
        jvar = j;
        break;
      }
      else if (varpanel_widget_get_nth (VARSEL_Y, j, d) == w) {
        togglebutton = VARSEL_Y;
        jvar = j;
        break;
      }
      else if (varpanel_widget_get_nth (VARSEL_Z, j, d) == w) {
        togglebutton = VARSEL_Z;
        jvar = j;
        break;
      }
      else if (varpanel_widget_get_nth (VARSEL_LABEL, j, d) == w) {
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
/*------------------------------------------------------------------*/

static void
varpanel_add_row (gint j, GGobiStage * d, ggobid * gg)
{
  GtkWidget *box, *xw, *yw, *zw, *label;
  gboolean sens = false;
  GList *displays;

  for (displays = gg->displays; !sens && displays;
       displays = g_list_next (displays))
    if (GGOBI_DISPLAY (displays->data)->d == d)
      sens = true;

  box = gtk_hbox_new (false, 2);
  gtk_box_pack_start (GTK_BOX (d->vcbox_ui.vbox), box, false, false, 1);

  xw = ggobi_noop_toggle_button_new_with_label (" X ");
  gtk_widget_set_sensitive (xw, sens);
  gtk_box_pack_start (GTK_BOX (box), xw, false, false, 2);
  ggobi_widget_set (xw, gg, true);
  g_object_set_data (G_OBJECT (box), varpanel_names[VARSEL_X], xw);
  g_signal_connect (G_OBJECT (xw),
                    "button_press_event", G_CALLBACK (varsel_cb), d);
  gtk_widget_show (xw);

  yw = ggobi_noop_toggle_button_new_with_label (" Y ");
  gtk_widget_set_sensitive (yw, sens);
  gtk_box_pack_start (GTK_BOX (box), yw, false, false, 2);
  ggobi_widget_set (yw, gg, true);
  g_object_set_data (G_OBJECT (box), varpanel_names[VARSEL_Y], yw);
  g_signal_connect (G_OBJECT (yw),
                    "button_press_event", G_CALLBACK (varsel_cb), d);
  gtk_widget_show (yw);

  zw = ggobi_noop_toggle_button_new_with_label (" Z ");
  gtk_widget_set_sensitive (zw, sens);
  gtk_box_pack_start (GTK_BOX (box), zw, false, false, 2);
  ggobi_widget_set (zw, gg, true);
  g_object_set_data (G_OBJECT (box), varpanel_names[VARSEL_Z], zw);
  g_signal_connect (G_OBJECT (zw),
                    "button_press_event", G_CALLBACK (varsel_cb), d);
  /*-- hide this widget by default --*/

  /*-- the label is actually a button, with the old behavior --*/
  label = gtk_button_new_with_label (ggobi_stage_get_col_name(d, j));
  gtk_widget_set_sensitive (label, sens);
  gtk_button_set_relief (GTK_BUTTON (label), GTK_RELIEF_NONE);
  ggobi_widget_set (label, gg, true);
  g_object_set_data (G_OBJECT (box), varpanel_names[VARSEL_LABEL], label);
  g_signal_connect (G_OBJECT (label),
                    "button_press_event", G_CALLBACK (varsel_cb), d);
  gtk_box_pack_start (GTK_BOX (box), label, false, false, 2);
  gtk_widget_show (label);

  d->vcbox_ui.box = g_slist_append (d->vcbox_ui.box, box);
  gtk_widget_show (box);
}

void
varpanel_widgets_add (gint nc, GGobiStage * d, ggobid * gg)
{
  gint j;
  gint nd = g_slist_length (gg->d);
  gint n = g_slist_length (d->vcbox_ui.box);

  /*-- create the variable widgets --*/
  for (j = n; j < nc; j++)
    varpanel_add_row (j, d, gg);

  /* 
   * If there were no variables before, a tab hasn't been added;
   * add one now.
   */
  if (n == 0) {
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                              d->varpanel_ui.hpane, gtk_label_new (d->name));
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                                nd > 1);
  }
}

/*-------------------------------------------------------------------------*/
/*                  handle the addition of new variables                   */
/*-------------------------------------------------------------------------*/

void
varpanel_addvar_cb (ggobid * gg, gint which,
                    GGobiStage * d, void *p)
{
  /*-- variable toggle buttons and circles --*/
  varpanel_widgets_add (d->n_cols, d, gg);
  varcircles_add (d->n_cols, d, gg);

  /*-- make sure the right toggle widgets and circles are showing --*/
/* this gives the wrong result when the variable being added is
not displayed in the current display.  And I don't know what the mode
is for the other datad.  This suggests that I have to run these two
routines whenever the tab changes in the variable selection panel.
But even then it may not correspond to the current display.  So it
really suggests that the variable selection panel has to keep track of
its own projection.  Add a variable to varpanel_ui in datad.h? -- dfs */
  varpanel_refresh (gg->current_display, gg);
  varcircles_visibility_set (gg->current_display, gg);
}

/*-------------------------------------------------------------------------*/
/*                  initialize and populate the var panel                  */
/*-------------------------------------------------------------------------*/

void
varpanel_set_sensitive (GGobiStage * d, gboolean sensitive_p, ggobid * gg)
{
  GtkWidget *vbox = d->vcbox_ui.vbox, *hb;
  GList *vblist, *hblist;

  /* The vbox has one child per row, an hbox. */
  vblist = gtk_container_get_children (GTK_CONTAINER (vbox));
  while (vblist) {
    hb = (GtkWidget *) vblist->data;
    hblist = gtk_container_get_children (GTK_CONTAINER (hb));
    while (hblist) {
      gtk_widget_set_sensitive ((GtkWidget *) hblist->data, sensitive_p);
      hblist = hblist->next;
    }
    vblist = vblist->next;
  }
}

/* Respond to display_new events */
void
varpanel_set_sensitive_cb (ggobid * gg, displayd * display)
{
  varpanel_set_sensitive (display->d, true, gg);
}


/*-------------------------------------------------------------------------*/

/*
 * build the notebook to contain a paned widget which will contain
 * checkboxes on the left and circles/rectangles on the right
*/
void
varpanel_make (GtkWidget * parent, ggobid * gg)
{

  gg->varpanel_ui.layoutByRow = true;  /*-- for the circles --*/
  gg->selvarfg_GC = NULL;

  gg->varpanel_ui.notebook = gtk_notebook_new ();
  gtk_notebook_set_show_border (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                                FALSE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (gg->varpanel_ui.notebook), TRUE);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                            GTK_POS_TOP);
  g_signal_connect (G_OBJECT (gg->varpanel_ui.notebook), "switch-page",
                    G_CALLBACK (varpanel_switch_page_cb), gg);

  gtk_box_pack_start (GTK_BOX (parent), gg->varpanel_ui.notebook,
                      true, true, 2);

  /*-- prepare to respond to variable_added events --*/
  g_signal_connect (G_OBJECT (gg), "variable_added",
                    G_CALLBACK (varpanel_addvar_cb), NULL);

  gtk_widget_show (gg->varpanel_ui.notebook);
}

void
varpanel_clear (GGobiStage * d, ggobid * gg)
{
  GList *pages;
  gint npages;
  gint k;

  if (gg->varpanel_ui.notebook != NULL &&
      GTK_WIDGET_REALIZED (gg->varpanel_ui.notebook)) {
    pages =
      gtk_container_get_children (GTK_CONTAINER (gg->varpanel_ui.notebook));
    npages = g_list_length (pages);
    for (k = 0; k < npages; k++)
      gtk_notebook_remove_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook), 0);
  }
}



/*-- for each datad:  hpane, ebox, scrolled window, vbox;
     in varpanel_add_row, an hbox, togglebuttons and label --*/
void
varpanel_populate (GGobiStage * d, ggobid * gg)
{
  gint j, nd;
  GList *children;
  GtkWidget *foo;

  nd = ndatad_with_vars_get (gg);

  /*-- we don't know the length of gg->d when the notebook is created --*/
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                              nd > 1);

  /*-- create a paned widget --*/
  d->varpanel_ui.hpane = gtk_hpaned_new ();
  /* not possible to set gutter size in GTK2 */
  //gtk_paned_set_gutter_size (GTK_PANED(d->varpanel_ui.hpane), 0);
  /*-- set the handle position all the way to the right --*/
  gtk_paned_set_position (GTK_PANED (d->varpanel_ui.hpane), -1);

  g_object_set_data (G_OBJECT (d->varpanel_ui.hpane), "datad", d);  /*setdata */
  /*-- only add a tab if there are variables --*/
  if (ggobi_stage_has_vars(d)) {
    gtk_notebook_append_page (GTK_NOTEBOOK (gg->varpanel_ui.notebook),
                              d->varpanel_ui.hpane, gtk_label_new (d->name));
  }

  /* Check if we have been here before and already created the box, etc.. */
  if (d->vcbox_ui.ebox && GTK_IS_WIDGET (d->vcbox_ui.ebox))
    return;

  /*-- create an ebox, and put it in the hpane --*/
  d->vcbox_ui.ebox = gtk_event_box_new ();
  gtk_paned_pack1 (GTK_PANED (d->varpanel_ui.hpane),
                   d->vcbox_ui.ebox, true, true);

  /*-- create a scrolled window, and put it in the ebox --*/
  d->vcbox_ui.swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (d->vcbox_ui.swin),
                                  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (d->vcbox_ui.ebox), d->vcbox_ui.swin);

  /*-- add a vbox to the swin --*/
  d->vcbox_ui.vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER (d->vcbox_ui.vbox), 2);
  g_signal_connect (G_OBJECT (gg), "display_new",
                    G_CALLBACK (varpanel_set_sensitive_cb), NULL);

  ggobi_stage_connect__col_deleted (d, varpanel_delete_nth_cb, NULL);
  
  /* Connecting to display_selected event */
  g_signal_connect (G_OBJECT (gg), "display_selected",
                    G_CALLBACK (varpanel_show_page_cb), d);
  g_signal_connect (G_OBJECT (gg), "display_selected",
                    G_CALLBACK (varpanel_tooltips_set_cb), d);
  /* */

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW
                                         (d->vcbox_ui.swin),
                                         d->vcbox_ui.vbox);

  /* Set shadow type for viewport */
  children = gtk_container_get_children (GTK_CONTAINER (d->vcbox_ui.swin));
  foo = g_list_nth_data (children, 0);
  if (GTK_IS_VIEWPORT (foo))
    gtk_viewport_set_shadow_type (GTK_VIEWPORT (foo), GTK_SHADOW_NONE);


  gtk_widget_show_all (d->varpanel_ui.hpane);
  gdk_flush ();

  d->vcbox_ui.box = NULL;
  for (j = 0; j < d->n_cols; j++)
    varpanel_add_row (j, d, gg);
}


/*-------------------------------------------------------------------------*/
/*                          API; not used                                  */
/*-------------------------------------------------------------------------*/

void ggobi_selectScatterplotX (GtkWidget * w, gint jvar, ggobid * gg)
{
  displayd *display = gg->current_display;
  GGobiExtendedDisplayClass *klass;

  if (!GGOBI_IS_EXTENDED_DISPLAY (display))
    return;
  klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
  if (klass->select_X)
    klass->select_X (w, display, jvar, gg);
}

/*-------------------------------------------------------------------------*/
/*                    context-sensitive tooltips                           */
/*-------------------------------------------------------------------------*/

void
varpanel_tooltips_set (displayd * display, ggobid * gg)
{
  gint j;
  GGobiStage *d;
  GtkWidget *wx, *wy, *wz, *label;

  if (display == NULL) {
    d = datad_get_from_notebook (gg->varpanel_ui.notebook, gg);
    if (d) {
      for (j = 0; j < d->n_cols; j++) {
        if ((wx = varpanel_widget_get_nth (VARSEL_X, j, d)) == NULL)
          break;
        label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
                              "Unable to plot without a display", NULL);
        gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
                              "Unable to plot without a display", NULL);
      }
    }
  }
  else {
    d = display->d;

    /*-- for each variable, current datad only --*/
    for (j = 0; j < d->n_cols; j++) {
      if ((wx = varpanel_widget_get_nth (VARSEL_X, j, d)) == NULL)
        break;

      wy = varpanel_widget_get_nth (VARSEL_Y, j, d);
      wz = varpanel_widget_get_nth (VARSEL_Z, j, d);
      label = varpanel_widget_get_nth (VARSEL_LABEL, j, d);

      if (GGOBI_IS_EXTENDED_DISPLAY (display)) {
        GGobiExtendedDisplayClass *klass =
          GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
        if (klass->varpanel_tooltips_set)
          klass->varpanel_tooltips_set (display, gg, wx, wy, wz, label);
      }
    }
  }
}
