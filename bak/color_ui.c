#include <gtk/gtk.h>

#include "vars.h"

#define PSIZE 20

/* external functions */
extern void build_circle (icoords *, gint, arcd *, gint, gshort);
extern void build_rect (icoords *, gint, rectd *, gint, gshort);
extern void build_x (icoords *, gint, GdkSegment *, gint, gshort);
extern void build_plus (icoords *, gint, GdkSegment *, gint, gshort);
extern void gdk_draw_rectangles (GdkDrawable *, GdkGC *, gint, rectd *, gint);
extern void gdk_draw_arcs (GdkDrawable *, GdkGC *, gint, arcd *, gint);
extern gint sqdist (gint, gint, gint, gint);
extern void init_plot_GC ();
/*                    */

static GtkWidget *symbol_window = NULL;
static GtkWidget *display;

static GtkWidget *colorseldlg = NULL;
static GtkWidget *bg_da, *accent_da, *fg_da[NCOLORS], *current_da;

static gint margin = 10;  /* between glyphs in the display */
gint spacing;

/*
 * Redraw one of the foreground color swatches
*/
static void
redraw_fg (GtkWidget *w, gint k) {

  if (plot_GC == NULL)
    init_plot_GC (w->window);

  gdk_gc_set_foreground (plot_GC, &xg.default_color_table[k]);
  gdk_draw_rectangle (w->window, plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);

  /*
   * Draw a background border around the box containing the selected color
  */
  if (k == xg.color_id) {
    gdk_gc_set_foreground (plot_GC, &xg.bg_color);
    gdk_draw_rectangle (w->window, plot_GC,
      false, 0, 0, w->allocation.width-1, w->allocation.height-1);
    gdk_draw_rectangle (w->window, plot_GC,
      false, 1, 1, w->allocation.width-2, w->allocation.height-2);
  }
}

static void
find_selection_circle_pos (gint *sizes, icoords *pos) {
  gint i;
  glyphv g;

  if (xg.glyph_id.type == POINT_GLYPH) {
    pos->y = margin + (3*sizes[0])/2;
    pos->x = spacing/2;

  } else {

    pos->y = 0;
    for (i=0; i<NGLYPHSIZES; i++) {
      g.size = sizes[i];
      pos->y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
      pos->x = spacing + spacing/2;

      if (xg.glyph_id.type == PLUS_GLYPH && xg.glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (xg.glyph_id.type == X_GLYPH && xg.glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (xg.glyph_id.type == OPEN_RECTANGLE && xg.glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (xg.glyph_id.type == FILLED_RECTANGLE && xg.glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (xg.glyph_id.type == OPEN_CIRCLE && xg.glyph_id.size == g.size)
        break;

      pos->x += spacing;
      if (xg.glyph_id.type == FILLED_CIRCLE && xg.glyph_id.size == g.size)
        break;
    }
  }
}

static void
redraw_display (GtkWidget *w) {
  static gint sizes[] = {TINY, SMALL, MEDIUM, LARGE, JUMBO};

  gint i;
  glyphv g;
  icoords *pos = (icoords *) g_malloc (sizeof (icoords));
  GdkSegment *segv = (GdkSegment *) g_malloc (2 * sizeof (GdkSegment));
  rectd *r = (rectd *) g_malloc (sizeof (rectd));
  arcd *a = (arcd *) g_malloc (sizeof (arcd));

  spacing = w->allocation.width/NGLYPHTYPES;

  if (plot_GC == NULL)
    init_plot_GC (w->window);

  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (w->window, plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
  gdk_gc_set_foreground (plot_GC, &xg.default_color_table[xg.color_id]);

  /*
   * The factor of three is dictated by the sizing of circles
  */
  pos[0].y = margin + (3*sizes[0])/2;
  pos[0].x = spacing/2;
  gdk_draw_point (w->window, plot_GC, pos[0].x, pos[0].y);

  pos[0].y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    g.size = sizes[i];
    pos[0].y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
    pos[0].x = spacing + spacing/2;

    build_plus (pos, 0, segv, 0, g.size);
    gdk_draw_segments (w->window, plot_GC, segv, 2);

    pos[0].x += spacing;
    build_x (pos, 0, segv, 0, g.size);
    gdk_draw_segments (w->window, plot_GC, segv, 2);

    pos[0].x += spacing;
    build_circle (pos, 0, a, 0, g.size);
    gdk_draw_rectangles (w->window, plot_GC, OPEN, r, 1);
    pos[0].x += spacing;
    build_circle (pos, 0, a, 0, g.size);
    gdk_draw_rectangles (w->window, plot_GC, FILL, r, 1);

    pos[0].x += spacing;
    build_circle (pos, 0, a, 0, g.size);
    gdk_draw_arcs (w->window, plot_GC, OPEN, a, 1);
    pos[0].x += spacing;
    build_circle (pos, 0, a, 0, g.size);
    gdk_draw_arcs (w->window, plot_GC, OPEN, a, 1);
    gdk_draw_arcs (w->window, plot_GC, FILL, a, 1);
  }
  
  if (xg.point_painting_p) {
    icoords p;
    gint radius = (3*JUMBO)/2 + margin/2;
    find_selection_circle_pos (sizes, &p);

    gdk_gc_set_foreground (plot_GC, &xg.accent_color);
    gdk_gc_set_line_attributes (plot_GC,
      2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_draw_arc (w->window, plot_GC, false, p.x-radius, p.y-radius,
      2*radius, 2*radius, 0, (gshort) 23040);
    gdk_gc_set_line_attributes (plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }

  g_free ((gpointer) pos);
  g_free ((gpointer) segv);
  g_free ((gpointer) r);
  g_free ((gpointer) a);
}

static void
redraw_bg (GtkWidget *w) {

  if (plot_GC == NULL)
    init_plot_GC (w->window);

  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (w->window, plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
}

static void
redraw_accent (GtkWidget *w) {

  if (plot_GC == NULL)
    init_plot_GC (w->window);

  gdk_gc_set_foreground (plot_GC, &xg.accent_color);
  gdk_draw_rectangle (w->window, plot_GC,
    true, 0, 0, w->allocation.width, w->allocation.height);
}

void
color_changed_cb (GtkWidget *colorsel)
{
  gdouble color[3];
  GdkColor gdk_color;
  GdkColormap *cmap = gdk_colormap_get_system ();
  splotd *sp = current_splot;

  /* Get current color */
  gtk_color_selection_get_color (GTK_COLOR_SELECTION (colorsel), color);

  gdk_color.red = (guint16)(color[0]*65535.0);
  gdk_color.green = (guint16)(color[1]*65535.0);
  gdk_color.blue = (guint16)(color[2]*65535.0);

  /* Allocate color */
  gdk_colormap_alloc_color (cmap, &gdk_color, false, true);

  if (current_da == bg_da) {
    xg.bg_color = gdk_color;
    redraw_bg (bg_da);
  } else if (current_da == accent_da) {
    xg.accent_color = gdk_color;
    redraw_accent (accent_da);
  } else {
    xg.default_color_table[xg.color_id] = gdk_color;
    redraw_fg (fg_da[xg.color_id], xg.color_id);
  }

  redraw_display (display);

  if (sp->da != NULL) {
    gboolean rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (sp->da), "expose_event",
      (gpointer) sp, (gpointer) &rval);
  }
}

static gint
color_expose_fg (GtkWidget *w, GdkEventExpose *event)
{
  int k = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w), "index"));
  redraw_fg (w, k);
  return FALSE;
}

static void
dlg_close_cb (GtkWidget *ok_button ) {
  gtk_widget_hide (colorseldlg);
}

static gint
open_colorsel_dialog (GtkWidget *w) {
  gint handled = FALSE;
  GtkWidget *colorsel, *ok_button, *cancel_button, *help_button;
  gint i;
  gdouble color[3];

  /* Check if we've received a button pressed event */

  if (colorseldlg == NULL) {
    handled = TRUE;

    /* Create color selection dialog */
    colorseldlg = gtk_color_selection_dialog_new ("Select color");

    /* Get the ColorSelection widget */
    colorsel = GTK_COLOR_SELECTION_DIALOG (colorseldlg)->colorsel;

    /*
     * Connect to the "color_changed" signal, set the client-data
     * to the colorsel widget
    */
    gtk_signal_connect (GTK_OBJECT (colorsel), "color_changed",
      (GtkSignalFunc) color_changed_cb, NULL);


    /*
     * Connect up the buttons
    */
    ok_button = GTK_COLOR_SELECTION_DIALOG (colorseldlg)->ok_button;
    cancel_button = GTK_COLOR_SELECTION_DIALOG (colorseldlg)->cancel_button;
    help_button = GTK_COLOR_SELECTION_DIALOG (colorseldlg)->help_button;
    gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
                        (GtkSignalFunc) dlg_close_cb, NULL);
    gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                        (GtkSignalFunc) dlg_close_cb, NULL);

  } else {

    colorsel = GTK_COLOR_SELECTION_DIALOG (colorseldlg)->colorsel;

    if (w == bg_da) {
      color[0] = (gdouble) xg.bg_color.red / 65535.0;
      color[1] = (gdouble) xg.bg_color.green / 65535.0;
      color[2] = (gdouble) xg.bg_color.blue / 65535.0;

      gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);

    } else if (w == accent_da) {
      color[0] = (gdouble) xg.accent_color.red / 65535.0;
      color[1] = (gdouble) xg.accent_color.green / 65535.0;
      color[2] = (gdouble) xg.accent_color.blue / 65535.0;

      gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);
    }
    else {
      for (i=0; i<NCOLORS; i++) {
        if (w == fg_da[i]) {
          color[0] = (gdouble) xg.default_color_table[i].red / 65535.0;
          color[1] = (gdouble) xg.default_color_table[i].green / 65535.0;
          color[2] = (gdouble) xg.default_color_table[i].blue / 65535.0;
          gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), color);
        }
      }
    }
  }

  /* Show the dialog */
  gtk_widget_show (colorseldlg);

  return handled;
}

static void
set_one_color ( GtkWidget *w, GdkEventButton *event )
{
  if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS)
    open_colorsel_dialog (w);
}
static void
set_color_fg ( GtkWidget *w, GdkEventButton *event )
{
  gint prev = xg.color_id;
  gint k = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w), "index"));
  splotd *sp = current_splot;

  xg.color_id = k;

  if (event->type==GDK_2BUTTON_PRESS || event->type==GDK_3BUTTON_PRESS) {
    open_colorsel_dialog (w);
  } else {
    gint rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (display), "expose_event",
      (gpointer) sp, (gpointer) &rval);
  }

  redraw_fg (fg_da[prev], prev);
  redraw_fg (w, k);
}
static gint
set_color_id (GtkWidget *w, GdkEventButton *event)
{

  /*
   * So that the same routines can be used to handle both the foreground
   * and background color swatches, keep track of which drawing area
   * was most recently pressed.
  */
  current_da = w;

  if (w == bg_da || w == accent_da)
    set_one_color (w, event);
  else
    set_color_fg (w, event);

  return FALSE;
}
  


static gint
color_expose_bg (GtkWidget *w, GdkEventExpose *event )
{
  redraw_bg (w);
  return FALSE;
}
static gint
color_expose_accent (GtkWidget *w, GdkEventExpose *event )
{
  redraw_accent (w);
  return FALSE;
}


static void
choose_glyph_cb (GtkWidget *w, GdkEventButton *event) {
/*
 * Reset glyph_id to the nearest glyph.
*/
  static gint sizes[] = {TINY, SMALL, MEDIUM, LARGE, JUMBO};
  glyphv g;
  gint i, dsq, nearest_dsq, type, size, rval = false;
  icoords pos, ev;
  splotd *sp = current_splot;

  ev.x = (gint) event->x;
  ev.y = (gint) event->y;

  pos.y = margin + (3*sizes[0])/2;
  pos.x = spacing/2;
  g.type = POINT_GLYPH;
  g.size = 1;
  nearest_dsq = dsq = sqdist (pos.x, pos.y, ev.x, ev.y);
  type = g.type; size = g.size;

  pos.y = 0;
  for (i=0; i<NGLYPHSIZES; i++) {
    g.size = sizes[i];
    pos.y += (margin + ( (i==0) ? (3*g.size)/2 : 3*g.size ));
    pos.x = spacing + spacing/2;

    g.type = PLUS_GLYPH;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = X_GLYPH;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = OPEN_RECTANGLE;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = FILLED_RECTANGLE;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = OPEN_CIRCLE;
    if ( (dsq = sqdist (pos.x, pos.y, ev.x, ev.y)) < nearest_dsq ) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }

    pos.x += spacing;
    g.type = FILLED_CIRCLE;
    dsq = sqdist (pos.x, pos.y, ev.x, ev.y);
    if (dsq < nearest_dsq) {
      nearest_dsq = dsq; type = g.type; size = g.size;
    }
  }

  xg.glyph_id.type = type;
  xg.glyph_id.size = size;
  gtk_signal_emit_by_name (GTK_OBJECT (display), "expose_event",
    (gpointer) sp, (gpointer) &rval);
}



static gint
color_expose_show (GtkWidget *w, GdkEventExpose *event)
{
  redraw_display (w);

  return FALSE;
}

static void
hide_symbol_window () {

  gtk_widget_hide (symbol_window);

  if (colorseldlg != NULL && GTK_WIDGET_VISIBLE (colorseldlg))
    gtk_widget_hide (colorseldlg);
}

void
make_symbol_window () {

  GtkWidget *vbox, *fg_frame, *bg_frame, *accent_frame, *btn;
  GtkWidget *fg_table, *bg_table, *accent_table, *ebox;
  gint i, j, k;
  gint width, height;

  if (symbol_window == NULL) {
    symbol_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (symbol_window), "color/glyph chooser");

    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (symbol_window), vbox);

    /*
     * the current brush color and bg; the circle will be drawn
     * in the current fg (accent) color
    */
    display = gtk_drawing_area_new (); 

    /*
     * margin pixels between glyphs, and three at each border
    */
    width = NGLYPHTYPES*JUMBO + margin*(NGLYPHTYPES+1);

    height = 3 * (TINY + SMALL + MEDIUM + LARGE + JUMBO) + 6*margin;

    gtk_drawing_area_size (GTK_DRAWING_AREA (display), width, height);
    gtk_box_pack_start (GTK_BOX (vbox), display, false, false, 0);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
      display, "Click to select glyph", NULL);

    gtk_signal_connect (GTK_OBJECT (display), "expose_event",
      GTK_SIGNAL_FUNC (color_expose_show), NULL);
    gtk_signal_connect (GTK_OBJECT (display), "button_press_event",
      GTK_SIGNAL_FUNC (choose_glyph_cb), NULL);

    gtk_widget_set_events (display, GDK_EXPOSURE_MASK
          | GDK_ENTER_NOTIFY_MASK
          | GDK_LEAVE_NOTIFY_MASK
          | GDK_BUTTON_PRESS_MASK);

    fg_frame = gtk_frame_new ("Foreground colors");
    gtk_box_pack_start (GTK_BOX (vbox), fg_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (fg_frame), ebox);

    fg_table = gtk_table_new (2, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), fg_table);

    k = 0;
    for (i=0; i<5; i++) {
      for (j=0; j<2; j++) {
        fg_da[k] = gtk_drawing_area_new ();
        gtk_object_set_data (GTK_OBJECT (fg_da[k]),
                             "index",
                             GINT_TO_POINTER (k));
        gtk_drawing_area_size (GTK_DRAWING_AREA (fg_da[k]), PSIZE, PSIZE);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), fg_da[k],
          "Click to select brushing color, double click to reset",
          NULL);

        gtk_widget_set_events (fg_da[k],
                               GDK_EXPOSURE_MASK
                               | GDK_ENTER_NOTIFY_MASK
                               | GDK_LEAVE_NOTIFY_MASK
                               | GDK_BUTTON_PRESS_MASK);

        gtk_signal_connect (GTK_OBJECT (fg_da[k]), "button_press_event",
          GTK_SIGNAL_FUNC (set_color_id), NULL);
        gtk_signal_connect (GTK_OBJECT (fg_da[k]), "expose_event",
          GTK_SIGNAL_FUNC (color_expose_fg), NULL);
        gtk_table_attach (GTK_TABLE (fg_table), fg_da[k], i, i+1, j, j+1,
          GTK_FILL, GTK_FILL, 10, 10);

        k++;
      }
    }

/*
 * Background color
*/

    bg_frame = gtk_frame_new ("Background color");
    gtk_box_pack_start (GTK_BOX (vbox), bg_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (bg_frame), ebox);

    bg_table = gtk_table_new (1, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), bg_table);

    bg_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (bg_da), PSIZE, PSIZE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
      bg_da, "Double click to reset", NULL);
    gtk_widget_set_events (bg_da,
                           GDK_EXPOSURE_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect (GTK_OBJECT (bg_da), "expose_event",
      GTK_SIGNAL_FUNC (color_expose_bg), NULL);
    gtk_signal_connect (GTK_OBJECT (bg_da), "button_press_event",
      GTK_SIGNAL_FUNC (set_color_id), NULL);

    gtk_table_attach (GTK_TABLE (bg_table), bg_da, 0, 1, 0, 1,
      GTK_FILL, GTK_FILL, 10, 10);

/*
 * Accent color
*/
    accent_frame = gtk_frame_new ("Accent color");
    gtk_box_pack_start (GTK_BOX (vbox), accent_frame, false, false, 0);

    ebox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (accent_frame), ebox);

    accent_table = gtk_table_new (1, 5, true);
    gtk_container_add (GTK_CONTAINER (ebox), accent_table);

    accent_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (accent_da), PSIZE, PSIZE);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips),
      accent_da, "Double click to reset color for labels and axes", NULL);
    gtk_widget_set_events (accent_da,
                           GDK_EXPOSURE_MASK
                           | GDK_ENTER_NOTIFY_MASK
                           | GDK_LEAVE_NOTIFY_MASK
                           | GDK_BUTTON_PRESS_MASK);

    gtk_signal_connect (GTK_OBJECT (accent_da), "expose_event",
      GTK_SIGNAL_FUNC (color_expose_accent), NULL);
    gtk_signal_connect (GTK_OBJECT (accent_da), "button_press_event",
      GTK_SIGNAL_FUNC (set_color_id), NULL);

    gtk_table_attach (GTK_TABLE (accent_table), accent_da, 0, 1, 0, 1,
      GTK_FILL, GTK_FILL, 10, 10);

/*
 * Close button
*/

    btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 0);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (hide_symbol_window), NULL);
  }

  gtk_widget_show_all (symbol_window);
}
