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

static void varpanel_checkbox_add (gint j, datad *d, ggobid *gg);

/*-------------------------------------------------------------------------*/
/*                     Variable selection                                  */
/*-------------------------------------------------------------------------*/

void
varpanel_checkbutton_set_active (gint jvar, gboolean active, datad *d)
{
  gboolean active_prev;

  if (jvar >= 0 && jvar < d->ncols) {
    GtkWidget *w = GTK_WIDGET (d->varpanel_ui.checkbox[jvar]);
    if (GTK_WIDGET_REALIZED (d->varpanel_ui.checkbox[jvar])) {

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
/*
          tour2d_varsel (gg, jvar, btn);
*/
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
/*                   variable cloning                                      */
/*-------------------------------------------------------------------------*/

void
variable_clone (gint jvar, const gchar *newName, gboolean update,
  datad *d, ggobid *gg) 
{
  gint nc = d->ncols + 1;
  
/*-- vartable_ui --*/
  /*-- set a view of the data values before adding the new label --*/
  vartable_row_append (d->ncols-1, d, gg);

/*-- vartable --*/
  vartable_realloc (nc, d, gg);
  d->vartable[nc-1].collab =
    g_strdup ((newName && newName[0]) ? newName : d->vartable[jvar].collab);
  d->vartable[nc-1].collab_tform =
    g_strdup ((newName && newName[0]) ? newName : d->vartable[jvar].collab);

/*-- varpanel_ui  --*/
  d->varpanel_ui.checkbox = (GtkWidget **)
    g_realloc (d->varpanel_ui.checkbox, nc * sizeof (GtkWidget *));
  varpanel_checkbox_add (nc-1, d, gg);

/*-- vartable --*/
  /*-- now the rest of the variables --*/
  d->vartable[nc-1].jitter_factor = d->vartable[jvar].jitter_factor;
  d->vartable[nc-1].nmissing = d->vartable[jvar].nmissing;

  if (update) {
    updateAddedColumn (nc, jvar, d, gg);
  }

  gtk_widget_show_all (gg->varpanel_ui.varpanel);
}

gboolean
updateAddedColumn (gint nc, gint jvar, datad *d, ggobid *gg)
{
/*-- vartable --*/
  if (jvar > -1) {
    d->vartable[nc-1].mean = d->vartable[jvar].mean;
    d->vartable[nc-1].median = d->vartable[jvar].median;
    d->vartable[nc-1].lim.min =
      d->vartable[nc-1].lim_raw.min = d->vartable[nc-1].lim_tform.min = 
      d->vartable[jvar].lim_raw.min;
    d->vartable[nc-1].lim.max =
      d->vartable[nc-1].lim_raw.max = d->vartable[nc-1].lim_tform.max = 
      d->vartable[jvar].lim_raw.max;
  } 

  transform_values_init (nc-1, d, gg);
/*-- --*/

/*-- pipeline --*/
  pipeline_arrays_add_column (jvar, d, gg);  /* reallocate and copy */
  missing_arrays_add_column (jvar, d, gg);

  d->ncols++;
  tform_to_world (d, gg); /*-- need this only for the new variable --*/

  return (true);
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
      if (d->varpanel_ui.checkbox[j] == w) {
        jvar = j;
        break;
      }
    }

/* looking for modifiers; don't know which ones we'll want */
    alt_mod = ((bevent->state & GDK_MOD1_MASK) == GDK_MOD1_MASK);
    shift_mod = ((bevent->state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK);
    ctrl_mod = ((bevent->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK);
/* */

    if (ctrl_mod) {
      variable_clone (jvar, NULL, true, d, gg);
      return (true);
    }
    
    /*-- general variable selection --*/
    varsel (cpanel, sp, jvar, button, alt_mod, ctrl_mod, shift_mod, d, gg);
    varpanel_refresh (gg);
    return true;
  }

  return false;
}

/*-------------------------------------------------------------------------*/
/*                  initialize and populate the var panel                  */
/*-------------------------------------------------------------------------*/

/*-- build the scrolled window and vbox; the d-specific parts follow --*/
void
varpanel_make (GtkWidget *parent, ggobid *gg) {

  gg->selvarfg_GC = NULL;

/*
  gg->varpanel_ui.varpanel_accel_group = gtk_accel_group_new ();
*/
  gg->varpanel_ui.tips = gtk_tooltips_new ();
  

  /*-- create a scrolled window --*/
  gg->varpanel_ui.scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
  gtk_box_pack_start (GTK_BOX (parent),
    gg->varpanel_ui.scrolled_window, true, true, 2);
  gtk_widget_show (gg->varpanel_ui.scrolled_window);

  /*-- create varpanel, a vbox, and add it to the scrolled window --*/
  gg->varpanel_ui.varpanel = gtk_vbox_new (false, 10);
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (gg->varpanel_ui.scrolled_window),
    gg->varpanel_ui.varpanel);

  gtk_widget_show_all (gg->varpanel_ui.scrolled_window);
}

static void
varpanel_checkbox_add (gint j, datad *d, ggobid *gg) 
{
  d->varpanel_ui.checkbox[j] =
    gtk_noop_check_button_new_with_label (d->vartable[j].collab);
  GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.checkbox[j]), gg, true);

  gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.checkbox[j]),
    "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);

  gtk_box_pack_start (GTK_BOX (d->varpanel_ui.vbox),
    d->varpanel_ui.checkbox[j], true, true, 0);
  gtk_widget_show (d->varpanel_ui.checkbox[j]);
}

/*-- create a column of check buttons? --*/
void varpanel_populate (datad *d, ggobid *gg)
{
  gint j;
  GtkWidget *ebox;
  GtkWidget *frame = gtk_frame_new (NULL);

  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
  gtk_box_pack_start (GTK_BOX (gg->varpanel_ui.varpanel),
    frame, false, false, 2);

  /*-- add an ebox to the frame --*/
  ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (frame), ebox);
  
  /*-- add a vbox to the ebox --*/
  d->varpanel_ui.vbox = gtk_vbox_new (false, 0);
  gtk_container_add (GTK_CONTAINER (ebox), d->varpanel_ui.vbox);

  gtk_widget_show_all (frame);
  gdk_flush ();

  d->varpanel_ui.checkbox = (GtkWidget **)
    g_malloc (d->ncols * sizeof (GtkWidget *));

  for (j=0; j<d->ncols; j++) {
    varpanel_checkbox_add (j, d, gg);
/*
    d->varpanel_ui.checkbox[j] =
      gtk_noop_check_button_new_with_label (d->vartable[j].collab);
    GGobi_widget_set (GTK_WIDGET (d->varpanel_ui.checkbox[j]), gg, true);

    gtk_signal_connect (GTK_OBJECT (d->varpanel_ui.checkbox[j]),
      "button_press_event", GTK_SIGNAL_FUNC (varsel_cb), d);

    gtk_box_pack_start (GTK_BOX (d->varpanel_ui.vbox),
      d->varpanel_ui.checkbox[j], true, true, 0);
    gtk_widget_show (d->varpanel_ui.checkbox[j]);
*/
  }
    
}


void
varlabel_set (gint j, datad *d, ggobid *gg) {
  gtk_label_set_text (GTK_LABEL (GTK_BIN (d->varpanel_ui.checkbox[j])->child),
    d->vartable[j].collab_tform);
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
      g_return_if_fail (d->varpanel_ui.checkbox != NULL);
      
      switch (display->displaytype) {

        case parcoords:
          gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
            d->varpanel_ui.checkbox[j],
            "Click to replace/insert/append a variable, or to delete it",
            NULL);
        break;

        case scatmat:
          gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
            d->varpanel_ui.checkbox[j],
            "Click to replace/insert/append a variable, or to delete it",
            NULL);
        break;

        case scatterplot:
          switch (projection) {
            case P1PLOT:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                d->varpanel_ui.checkbox[j],
                "Click left to plot horizontally, middle to plot vertically",
                NULL);
            break;
            case XYPLOT:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                d->varpanel_ui.checkbox[j],
                "Click left to select the horizontal variable, middle for vertical",
                NULL);
            break;
            case TOUR2D:
              gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->varpanel_ui.tips),
                d->varpanel_ui.checkbox[j],
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
