/*-- color_ui.c --*/
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

#include "vars.h"
#include "externs.h"

#define PSIZE 20

static gint open_colorsel_dialog (GtkWidget *w, ggobid *gg);
static void redraw_fg (GtkWidget *w, gint k, ggobid *gg);

/*------------------------------------------------------------------------*/
/*                    symbol display routines                             */
/*------------------------------------------------------------------------*/

static void
choose_glyph_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
/*-- Reset glyph_id to the nearest glyph.  --*/
  glyphd g;
  gint i, dsq, nearest_dsq, type, size, rval = false;
  icoords pos, ev;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gint spacing = gg->color_ui.spacing;
  gint margin = gg->color_ui.margin;

  vectorg_copy (&d->glyph, &d->glyph_prev);  /*-- from, to --*/

  ev.x = (gint) event->x;
  ev.y = (gint) event->y;

  pos.y = margin + 3/2;
  pos.x = spacing/2;
  g.type = DOT;
  g.size = 1;
  nearest_dsq = dsq = sqdist (pos.x, pos.y, ev.x, ev.y);
  type = g.type; size = g.size;

  pos.y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    g.size = i;
    pos.y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
    pos.x = spacing + spacing/2;

    g.type = PLUS;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = X;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = OC;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = OR;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = FC;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = FR;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }
  }

  gg->glyph_id.type = type;
  gg->glyph_id.size = size;
  gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.symbol_display),
    "expose_event",
    (gpointer) sp, (gpointer) &rval);
  gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.line_display),
    "expose_event",
    (gpointer) sp, (gpointer) &rval);
}

static void
find_symbol_selection_circle_pos (icoords *pos, ggobid *gg) {
  gint i;
  glyphd g;
  gint spacing = gg->color_ui.spacing;
  gint margin = gg->color_ui.margin;

  if (gg->glyph_id.type == DOT) {
    pos->y = margin + 3/2;
    pos->x = spacing/2;

  } else {

    pos->y = 0;
    for (i=0; i<NGLYPHSIZES; i++) {
      g.size = i;
      pos->y += (margin + ((i==0) ? (3*g.size)/2 : 3*g.size ));
      pos->x = spacing + spacing/2;

      if (gg->glyph_id.type == PLUS && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == X && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == OC && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == OR && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == FC && gg->glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (gg->glyph_id.type == FR && gg->glyph_id.size == g.size)
        break;
    }
  }
}

static void
redraw_symbol_display (GtkWidget *w, ggobid *gg) {
  gint i;
  glyphd g;
  icoords pos;
  gint margin, spacing;
  colorschemed *scheme = gg->activeColorScheme;

  gg->color_ui.spacing = w->allocation.width/NGLYPHTYPES;

  margin = gg->color_ui.margin;
  spacing = gg->color_ui.spacing;

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[gg->color_id]);

  /*
   * The factor of three is dictated by the sizing of circles
   *  ... this should no longer be true; it should be 2*width + 1
  */
  pos.y = margin + 3/2;
  pos.x = spacing/2;
  gdk_draw_point (w->window, gg->plot_GC, pos.x, pos.y);

  pos.y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    g.size = i;
    pos.y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
    pos.x = spacing + spacing/2;

    g.type = PLUS;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = X;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = OC;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = OR;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = FC;
    draw_glyph (w->window, &g, &pos, 0, gg);

    pos.x += spacing;
    g.type = FR;
    draw_glyph (w->window, &g, &pos, 0, gg);
  }
  
  if (!gg->mono_p) {
    icoords p;
    /*-- 2*(NGLYPHSIZES+1) is the size of the largest glyph; draw_glyph() --*/
    gint radius = (3*NGLYPHSIZES)/2 + gg->color_ui.margin/2;
    find_symbol_selection_circle_pos (&p, gg);

    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    gdk_gc_set_line_attributes (gg->plot_GC,
      2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_draw_arc (w->window, gg->plot_GC, false, p.x-radius, p.y-radius,
      2*radius, 2*radius, 0, (gshort) 23040);
    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}

static gint
symbol_display_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_symbol_display (w, gg);
  return FALSE;
}

/*------------------------------------------------------------------------*/
/*                      line display routines                             */
/*------------------------------------------------------------------------*/

/*-- use the current glyph type and size to determine the line type --*/
/*
 * point, +, x -> lightest dashed line
 * open circle and rectangle -> middle dashed line
 * filled circle and rectangle -> solid line
*/
static void
find_line_selection_pos (icoords *pos, ggobid *gg) {
  gint i;
  glyphd g;
  gint spacing = gg->color_ui.spacing;
  gint margin = gg->color_ui.margin;

  if (gg->glyph_id.type == DOT) {
    pos->x = spacing + spacing/2;
    pos->y = margin;
  } else {
    pos->y = 0;
    for (i=0; i<NGLYPHSIZES; i++) {
      g.size = i;
      pos->y += (margin + ((i==0) ? (3*g.size)/2 : 3*g.size ));
      pos->x = spacing + spacing/2;

      if (gg->glyph_id.type == PLUS && gg->glyph_id.size == g.size)
        break;
      if (gg->glyph_id.type == X && gg->glyph_id.size == g.size)
        break;

      pos->x += (2*spacing);
      if (gg->glyph_id.type == OC && gg->glyph_id.size == g.size)
        break;
      if (gg->glyph_id.type == OR && gg->glyph_id.size == g.size)
        break;

      pos->x += (2*spacing);
      if (gg->glyph_id.type == FC && gg->glyph_id.size == g.size)
        break;
      if (gg->glyph_id.type == FR && gg->glyph_id.size == g.size)
        break;
    }
  }
}

static void
redraw_line_display (GtkWidget *w, ggobid *gg)
{
  gint i, linewidth;
  icoords pos;
  gint margin, spacing;
  gchar dash_list[2];
  colorschemed *scheme = gg->activeColorScheme;

  margin = gg->color_ui.margin;
  spacing = gg->color_ui.spacing;

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[gg->color_id]);
 
  pos.y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    linewidth = (i<3) ? 0 : (i-2)*2;
    pos.y += (margin + ((i==0) ? (3*i)/2 : 3*i));

    pos.x = spacing;
    dash_list[0] = 4;
    dash_list[1] = 2;
    gdk_gc_set_dashes (gg->plot_GC, 0, dash_list, 2);
    gdk_gc_set_line_attributes (gg->plot_GC, linewidth,
      GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_ROUND);
    gdk_draw_line (w->window, gg->plot_GC,
      pos.x, pos.y, pos.x + spacing, pos.y);

    pos.x += (2*spacing);
    gdk_gc_set_line_attributes (gg->plot_GC, linewidth,
      GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_ROUND);
    dash_list[0] = 8;
    dash_list[1] = 2;
    gdk_gc_set_dashes (gg->plot_GC, 0, dash_list, 2);
    gdk_draw_line (w->window, gg->plot_GC,
      pos.x, pos.y, pos.x + spacing, pos.y);

    pos.x += (2*spacing);
    gdk_gc_set_line_attributes (gg->plot_GC, linewidth,
      GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_ROUND);
    gdk_draw_line (w->window, gg->plot_GC,
      pos.x, pos.y, pos.x + spacing, pos.y);
  }

  gdk_gc_set_line_attributes (gg->plot_GC, 0,
    GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);

  if (!gg->mono_p) {
    icoords p;
    find_line_selection_pos (&p, gg);

    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    gdk_draw_rectangle (w->window, gg->plot_GC, false,
      p.x - spacing/2 - margin/2, p.y - (NGLYPHSIZES+1)/2 - margin/2,
      spacing + margin, (NGLYPHSIZES+1) + margin);
  }
}

static gint
line_display_expose_cb (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_line_display (w, gg);
  return FALSE;
}


/*------------------------------------------------------------------------*/
/*          foreground, background, accent color widgets                  */
/*------------------------------------------------------------------------*/
static void
set_one_color ( GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS)
    open_colorsel_dialog (w, gg);
}
static void
set_color_fg ( GtkWidget *w, GdkEventButton *event , ggobid *gg)
{
  gint i;
  gint prev = gg->color_id;
  gint k = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w), "index"));
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;

  for (i=0; i<d->nrows; i++)
    d->color_prev.els[i] = d->color.els[i];
  gg->color_id = k;

  if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS) {
    open_colorsel_dialog (w, gg);
  } else {
    gint rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.symbol_display),
      "expose_event", (gpointer) gg, (gpointer) &rval);
    gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.line_display),
      "expose_event", (gpointer) gg, (gpointer) &rval);
  }

  redraw_fg (gg->color_ui.fg_da[prev], prev, gg);
  redraw_fg (w, k, gg);
}

static gint
set_color_id (GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  /*
   * So that the same routines can be used to handle both the foreground
   * and background color swatches, keep track of which drawing area
   * was most recently pressed.
  */
  gg->color_ui.current_da = w;

  if (w == gg->color_ui.bg_da || w == gg->color_ui.accent_da)
    set_one_color (w, event, gg);
  else
    set_color_fg (w, event, gg);

#ifdef BARCHART_IMPLEMENTED
  /* barcharts need more attention than redrawing the brush */
  if (gg->current_splot->bar) {
    splot_redraw (gg->current_splot, FULL, gg);
  } else
#endif
  splot_redraw (gg->current_splot, QUICK, gg);  /*-- redraw brush --*/

  return FALSE;
}

/*-- Redraw one of the foreground color swatches --*/
static void
redraw_fg (GtkWidget *w, gint k, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &gg->activeColorScheme->rgb[k]);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);

  /*
   * Draw a background border around the box containing the selected color
  */
  if (k == gg->color_id) {
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
    gdk_draw_rectangle (w->window, gg->plot_GC,
      false, 0, 0, w->allocation.width-1, w->allocation.height-1);
    gdk_draw_rectangle (w->window, gg->plot_GC,
      false, 1, 1, w->allocation.width-2, w->allocation.height-2);
  }
}

static gint
color_expose_fg (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  gint k = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w), "index"));

  if (k <= gg->activeColorScheme->n)
    redraw_fg (w, k, gg);

  return FALSE;
}


static void
redraw_bg (GtkWidget *w, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
}
static gint
color_expose_bg (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_bg (w, gg);
  return FALSE;
}

static void
redraw_accent (GtkWidget *w, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  if (gg->plot_GC == NULL)
    init_plot_GC (w->window, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_rectangle (w->window, gg->plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
}
static gint
color_expose_accent (GtkWidget *w, GdkEventExpose *event, ggobid *gg)
{
  redraw_accent (w, gg);
  return FALSE;
}
static void
reverse_video_cb (GtkWidget *ok_button, ggobid* gg) {
  gulong pixel;
  gushort r, g, b;
  gint rval = false;
  colorschemed *scheme = gg->activeColorScheme;

  r = scheme->rgb_accent.red;
  g = scheme->rgb_accent.green;
  b = scheme->rgb_accent.blue;
  pixel = scheme->rgb_accent.pixel;

  scheme->rgb_accent.red =   scheme->rgb_bg.red;
  scheme->rgb_accent.green = scheme->rgb_bg.green;
  scheme->rgb_accent.blue =  scheme->rgb_bg.blue;
  scheme->rgb_accent.pixel = scheme->rgb_bg.pixel;

  scheme->rgb_bg.red = r;
  scheme->rgb_bg.green = g;
  scheme->rgb_bg.blue = b;
  scheme->rgb_bg.pixel = pixel;

  gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.symbol_display),
    "expose_event", (gpointer) gg, (gpointer) &rval);
  gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.line_display),
    "expose_event", (gpointer) gg, (gpointer) &rval);
  redraw_bg (gg->color_ui.bg_da, gg);
  redraw_accent (gg->color_ui.accent_da, gg);

  displays_plot ((splotd *) NULL, FULL, gg);
}

/*------------------------------------------------------------------------*/
/*                    color selection dialog routines                     */
/*------------------------------------------------------------------------*/

void
color_changed_cb (GtkWidget *colorsel, ggobid *gg)
{
  gdouble color[3];
  GdkColor gdk_color;
  GdkColormap *cmap = gdk_colormap_get_system ();
  splotd *sp = gg->current_splot;
  GtkWidget *wheel = GTK_COLOR_SELECTION (colorsel)->wheel_area;
  colorschemed *scheme = gg->activeColorScheme;

  /* Get current color */
  gtk_color_selection_get_color (GTK_COLOR_SELECTION (colorsel), color);

  gdk_color.red = (guint16)(color[0]*65535.0);
  gdk_color.green = (guint16)(color[1]*65535.0);
  gdk_color.blue = (guint16)(color[2]*65535.0);

  /* Allocate color */
  if (gdk_color_alloc (cmap, &gdk_color)) {

    if (gg->color_ui.current_da == gg->color_ui.bg_da) {
      scheme->rgb_bg = gdk_color;
      redraw_bg (gg->color_ui.bg_da, gg);
    } else if (gg->color_ui.current_da == gg->color_ui.accent_da) {
      scheme->rgb_accent = gdk_color;
      redraw_accent (gg->color_ui.accent_da, gg);
    } else {
      gg->activeColorScheme->rgb[gg->color_id] = gdk_color;
      redraw_fg (gg->color_ui.fg_da[gg->color_id], gg->color_id, gg);
    }

    redraw_symbol_display (gg->color_ui.symbol_display, gg);

    if (sp->da != NULL) {
      gboolean rval = false;
      gtk_signal_emit_by_name (GTK_OBJECT (sp->da), "expose_event",
        (gpointer) sp, (gpointer) &rval);
    }

    /*
     * If wheel doesn't have the grab, it means that the button
     * has been released:  update all plots.
    */
    if (!GTK_WIDGET_HAS_GRAB (wheel)) {
      displays_plot ((splotd *) NULL, FULL, gg);
    }
  }
}

static void
dlg_close_cb (GtkWidget *ok_button, ggobid* gg) {
  gtk_widget_hide (gg->color_ui.colorseldlg);
}

static gint
open_colorsel_dialog (GtkWidget *w, ggobid *gg) {
  gint handled = FALSE;
  GtkWidget *colorsel, *ok_button, *cancel_button, *help_button;
  gint i;
  gdouble color[3];
  GtkColorSelectionDialog *colordlg;
  colorschemed *scheme = gg->activeColorScheme;

  /* Check if we've received a button pressed event */

  if (gg->color_ui.colorseldlg == NULL) {
    handled = true;

    /* Create color selection dialog */
    gg->color_ui.colorseldlg = gtk_color_selection_dialog_new ("Select color");

    /* Get the ColorSelection widget */
    colordlg = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg);
    colorsel = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->colorsel;

    /*
     * Connect to the "color_changed" signal, set the client-data
     * to the colorsel widget
    */
    gtk_signal_connect (GTK_OBJECT (colorsel), "color_changed",
      (GtkSignalFunc) color_changed_cb, gg);

    /*
     * Connect up the buttons
    */
    ok_button = colordlg->ok_button;
    cancel_button = colordlg->cancel_button;
    help_button = colordlg->help_button;
    gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
                        (GtkSignalFunc) dlg_close_cb, gg);
    gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                        (GtkSignalFunc) dlg_close_cb, gg);

  } else {

    colorsel = GTK_COLOR_SELECTION_DIALOG (gg->color_ui.colorseldlg)->colorsel;
  }

  if (w == gg->color_ui.bg_da) {
    color[0] = (gdouble) scheme->rgb_bg.red / 65535.0;
    color[1] = (gdouble) scheme->rgb_bg.green / 65535.0;
    color[2] = (gdouble) scheme->rgb_bg.blue / 65535.0;

    gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);

  } else if (w == gg->color_ui.accent_da) {
    color[0] = (gdouble) scheme->rgb_accent.red / 65535.0;
    color[1] = (gdouble) scheme->rgb_accent.green / 65535.0;
    color[2] = (gdouble) scheme->rgb_accent.blue / 65535.0;

    gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);
  }
  else {
      for (i=0; i<MAXNCOLORS; i++) {
      if (w == gg->color_ui.fg_da[i]) {
        color[0] = (gdouble) gg->activeColorScheme->rgb[i].red / 65535.0;
        color[1] = (gdouble) gg->activeColorScheme->rgb[i].green / 65535.0;
        color[2] = (gdouble) gg->activeColorScheme->rgb[i].blue / 65535.0;
          gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);
      }
    }
  }

  /* Show the dialog */
  gtk_widget_show (gg->color_ui.colorseldlg);

  return handled;
}

  
/*------------------------------------------------------------------------*/
/*                    show/hide/delete the window                         */
/*------------------------------------------------------------------------*/

static void
hide_symbol_window (ggobid* gg) {

  gtk_widget_hide (gg->color_ui.symbol_window);

  if (gg->color_ui.colorseldlg != NULL &&
      GTK_IS_WIDGET (gg->color_ui.colorseldlg) &&
      GTK_WIDGET_VISIBLE (gg->color_ui.colorseldlg))
  {
     gtk_widget_hide (gg->color_ui.colorseldlg);
  }
}
/*-- catch a click on the close button --*/
static void
hide_symbol_window_cb (GtkWidget *w, ggobid* gg) {
  hide_symbol_window (gg);
}
/*-- catch the delete (close) event from the window manager --*/
static void
delete_symbol_window_cb (GtkWidget *w, GdkEventButton *event, ggobid *gg) {
  hide_symbol_window (gg);
}

/*------------------------------------------------------------------------*/
/*                   redraw the window                                    */
/*------------------------------------------------------------------------*/

void
symbol_window_redraw (ggobid *gg)
{
/*
 * Send expose events where necessary; show the appropriate
 * number of fg_da widgets.
*/
  gint k;
  splotd *sp = gg->current_splot;
  gint rval = false;

  if (gg->color_ui.symbol_display) {

    gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.symbol_display),
      "expose_event",
      (gpointer) sp, (gpointer) &rval);
    gtk_signal_emit_by_name (GTK_OBJECT (gg->color_ui.line_display),
      "expose_event",
      (gpointer) sp, (gpointer) &rval);
    redraw_bg (gg->color_ui.bg_da, gg);
    redraw_accent (gg->color_ui.accent_da, gg);

    for (k=0; k<gg->activeColorScheme->n; k++) {
      gtk_widget_show (gg->color_ui.fg_da[k]);
      redraw_fg (gg->color_ui.fg_da[k], k, gg);
    }
    for (k=gg->activeColorScheme->n; k<MAXNCOLORS; k++) {
      gtk_widget_hide (gg->color_ui.fg_da[k]);
    }
  }
}

/*------------------------------------------------------------------------*/
/*                    build the window                                    */
/*------------------------------------------------------------------------*/

void
make_symbol_window (ggobid *gg) {

  GtkWidget *vbox, *fg_frame, *bg_frame, *accent_frame, *btn;
  GtkWidget *fg_table, *bg_table, *accent_table, *ebox, *hbox;
  gint i, j, k;
  gint width, height;

  /*
   * This seems to handle the case where a the window was
   * closed using the window manager -- even though I'm capturing
   * a delete_event.
  */
  if (!GTK_IS_WIDGET (gg->color_ui.symbol_window))
    gg->color_ui.symbol_window = NULL;

  if (gg->color_ui.symbol_window == NULL) {
    gg->color_ui.symbol_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->color_ui.symbol_window),
      "color & glyph chooser");

    /*
     * I thought this would be enough to prevent the window from
     * being destroyed, but it doesn't seem to be.
    */
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.symbol_window),
                        "delete_event",
                        GTK_SIGNAL_FUNC (delete_symbol_window_cb),
                        (gpointer) gg);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->color_ui.symbol_window), vbox);

    /*-- to contain the two display areas --*/
    hbox = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

/*
 * display of glyph types and sizes
*/
    gg->color_ui.symbol_display = gtk_drawing_area_new (); 

    /*-- after this, margin is only used in determining y position --*/
    /*-- 2*(NGLYPHSIZES+1) is the size of the largest glyph --*/
    width = NGLYPHTYPES*2*(NGLYPHSIZES+1) + gg->color_ui.margin*(NGLYPHTYPES+1);

    /*-- initialize the spacing that will be used in drawing --*/
    gg->color_ui.spacing = width/NGLYPHTYPES;

    height = gg->color_ui.margin;
    for (i=0; i<NGLYPHSIZES; i++)
      height += (gg->color_ui.margin + 2*(i+2));
    height += gg->color_ui.margin;

    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.symbol_display),
      width, height);
    gtk_box_pack_start (GTK_BOX (hbox), gg->color_ui.symbol_display,
      true, true, 0);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.symbol_display,
      "Click to select glyph type and size -- which also selects the line type",
      NULL);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.symbol_display),
      "expose_event",
      GTK_SIGNAL_FUNC (symbol_display_expose_cb), gg);
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.symbol_display),
      "button_press_event",
      GTK_SIGNAL_FUNC (choose_glyph_cb), gg);

    gtk_widget_set_events (gg->color_ui.symbol_display, GDK_EXPOSURE_MASK
          | GDK_ENTER_NOTIFY_MASK
          | GDK_LEAVE_NOTIFY_MASK
          | GDK_BUTTON_PRESS_MASK);
/*
 * the display of line types and widths
*/
    gg->color_ui.line_display = gtk_drawing_area_new (); 

    width = NEDGETYPES*gg->color_ui.spacing +     /*-- lines --*/
            (NEDGETYPES+1)*gg->color_ui.spacing;  /*-- space between --*/

    /*-- use the same height we used for the symbol display --*/
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.line_display),
      width, height);
    gtk_box_pack_start (GTK_BOX (hbox), gg->color_ui.line_display,
      true, true, 0);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.line_display, "Shows the line type corresponding to the current glyph selection", NULL);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.line_display),
      "expose_event",
      GTK_SIGNAL_FUNC (line_display_expose_cb), gg);
/*
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.line_display),
      "button_press_event",
      GTK_SIGNAL_FUNC (choose_linetype_cb), gg);
*/

    gtk_widget_set_events (gg->color_ui.line_display, GDK_EXPOSURE_MASK
          | GDK_ENTER_NOTIFY_MASK
          | GDK_LEAVE_NOTIFY_MASK
          | GDK_BUTTON_PRESS_MASK);
/*-- --*/

    fg_frame = gtk_frame_new ("Foreground colors");
    gtk_box_pack_start (GTK_BOX (vbox), fg_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (fg_frame), ebox);

    /*-- create MAXNCOLORS drawing areas, showing gg->activeColorScheme->n of them --*/
    fg_table = gtk_table_new (1, MAXNCOLORS, true);
    gtk_container_add (GTK_CONTAINER (ebox), fg_table);

    k = 0;
    for (i=0, j=0; i<MAXNCOLORS; i++) {
      gg->color_ui.fg_da[k] = gtk_drawing_area_new ();
      gtk_object_set_data (GTK_OBJECT (gg->color_ui.fg_da[k]),
                           "index",
                           GINT_TO_POINTER (k));
      gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.fg_da[k]),
        PSIZE, PSIZE);

      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->color_ui.fg_da[k],
        "Click to select brushing color, double click to reset",
        NULL);

      gtk_widget_set_events (gg->color_ui.fg_da[k],
                             GDK_EXPOSURE_MASK
                             | GDK_ENTER_NOTIFY_MASK
                             | GDK_LEAVE_NOTIFY_MASK
                             | GDK_BUTTON_PRESS_MASK);

      gtk_signal_connect (GTK_OBJECT (gg->color_ui.fg_da[k]),
       "button_press_event",
        GTK_SIGNAL_FUNC (set_color_id), gg);
      gtk_signal_connect (GTK_OBJECT (gg->color_ui.fg_da[k]),
       "expose_event",
        GTK_SIGNAL_FUNC (color_expose_fg), gg);
      gtk_table_attach (GTK_TABLE (fg_table),
        gg->color_ui.fg_da[k], i, i+1, j, j+1,
        GTK_FILL, GTK_FILL, 10, 10);

      k++;
    }

    /*-- hbox to contain bg and accent color frames --*/
    hbox = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

    /*-- Background color --*/
    bg_frame = gtk_frame_new ("Background color");
    gtk_box_pack_start (GTK_BOX (hbox), bg_frame, true, true, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (bg_frame), ebox);

    bg_table = gtk_table_new (1, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), bg_table);

    gg->color_ui.bg_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.bg_da),
      PSIZE, PSIZE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.bg_da,
      "Double click to reset background color (Note: your color selection will have no visible effect unless you also reset the 'Value')",
      NULL);
    gtk_widget_set_events (gg->color_ui.bg_da,
                           GDK_EXPOSURE_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.bg_da),
      "expose_event", GTK_SIGNAL_FUNC (color_expose_bg), gg);
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.bg_da),
      "button_press_event", GTK_SIGNAL_FUNC (set_color_id), gg);

    gtk_table_attach (GTK_TABLE (bg_table),
      gg->color_ui.bg_da, 0, 1, 0, 1,
      GTK_FILL, GTK_FILL, 10, 10);

    /*-- Accent color --*/
    accent_frame = gtk_frame_new ("Accent color");
    gtk_box_pack_start (GTK_BOX (hbox), accent_frame, true, true, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (accent_frame), ebox);

    accent_table = gtk_table_new (1, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), accent_table);

    gg->color_ui.accent_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->color_ui.accent_da),
      PSIZE, PSIZE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
      gg->color_ui.accent_da, "Double click to reset color for labels and axes",
      NULL);
    gtk_widget_set_events (gg->color_ui.accent_da,
                           GDK_EXPOSURE_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect (GTK_OBJECT (gg->color_ui.accent_da),
      "expose_event", GTK_SIGNAL_FUNC (color_expose_accent), gg);
    gtk_signal_connect (GTK_OBJECT (gg->color_ui.accent_da),
      "button_press_event", GTK_SIGNAL_FUNC (set_color_id), gg);

    gtk_table_attach (GTK_TABLE (accent_table),
      gg->color_ui.accent_da, 0, 1, 0, 1,
      GTK_FILL, GTK_FILL, 10, 10);

    /*-- Temporary, perhaps: reverse video button --*/
    btn = gtk_button_new_with_label ("Reverse video");
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn),
                        "clicked",
                        GTK_SIGNAL_FUNC (reverse_video_cb),
                        (gpointer) gg);

    /*-- Close button --*/
    btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn),
                        "clicked",
                        GTK_SIGNAL_FUNC (hide_symbol_window_cb),
                        (gpointer) gg);
  }

  gtk_widget_show_all (gg->color_ui.symbol_window);

  /*
   * In case the default colorscheme has fewer colors than the
   * default, hide the extra fg_da's. 
  */
  for (k=gg->activeColorScheme->n; k<MAXNCOLORS; k++)
    gtk_widget_hide (gg->color_ui.fg_da[k]);
}
