/*-- utils_gdk.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
mousepos_get_pressed (GtkWidget * w, GdkEventButton * event,
                      gboolean * btn1_down_p, gboolean * btn2_down_p,
                      splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  gint grab_ok;
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);

  grab_ok = gdk_pointer_grab (sp->da->window,
                              false,
                              (GdkEventMask) (GDK_POINTER_MOTION_MASK |
                                              GDK_BUTTON_RELEASE_MASK),
                              (GdkWindow *) NULL, (GdkCursor *) NULL,
                              event->time);

  if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    *btn1_down_p = true;
  else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    *btn2_down_p = true;
  else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    *btn2_down_p = true;

  if (*btn1_down_p)
    gg->buttondown = 1;
  else if (*btn2_down_p)
    gg->buttondown = 2;
}

void
mousepos_get_motion (GtkWidget * w, GdkEventMotion * event,
                     gboolean * btn1_down_p, gboolean * btn2_down_p,
                     splotd * sp)
{
  ggobid *gg = GGobiFromSPlot (sp);
  GdkModifierType state;

  *btn1_down_p = false;
  *btn2_down_p = false;

  /*-- that is, if using motion hints --*/
/*
  if (event->is_hint) {
*/

  gdk_window_get_pointer (w->window, &sp->mousepos.x, &sp->mousepos.y,
                          &state);
  if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
    *btn1_down_p = true;
  else if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
    *btn2_down_p = true;
  else if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
    *btn2_down_p = true;

/*
  } else {

    sp->mousepos.x = (gint) event->x;
    sp->mousepos.y = (gint) event->y;
    if ((event->state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
      *btn1_down_p = true;
    else if ((event->state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
      *btn2_down_p = true;
    else if ((event->state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
      *btn2_down_p = true;
  }
*/

  if (*btn1_down_p)
    gg->buttondown = 1;
  else if (*btn2_down_p)
    gg->buttondown = 2;
}

gboolean
mouseinwindow (splotd * sp)
{
  return (0 < sp->mousepos.x && sp->mousepos.x < sp->max.x &&
          0 < sp->mousepos.y && sp->mousepos.y < sp->max.y);

}
