/* scatterplot.c */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"

#define WIDTH   400
#define HEIGHT  400

/* external functions */
extern void init_control_panel (displayd *, cpaneld *, gint);
extern splotd * splot_new (displayd *, gint, gint);
extern void splot_screen_to_tform (cpaneld *, splotd *, icoords *, fcoords *);
extern void get_main_menu (GtkItemFactoryEntry[], gint, GtkAccelGroup *,
                           GtkWidget  *, GtkWidget **, gpointer);
extern void display_free (displayd *);
/* */

static GtkWidget *mbar;
static GtkAccelGroup *sp_accel_group;

static void
close_cb (displayd *display, guint action, GtkWidget *w) {
  display_free (display);
}
static void
delete_cb (GtkWidget *w, GdkEvent *event, displayd *display) {
  display_free (display);
}

static void
options_cb(gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {

    case 0:

    case 1:
    case 2:
  }
}

void
scatterplot_show_hrule (displayd *display, gboolean show) {
  if (show) {
    if (!GTK_WIDGET_VISIBLE (display->hrule))
      gtk_widget_show (display->hrule);
  } else {
    if (GTK_WIDGET_VISIBLE (display->hrule))
      gtk_widget_hide (display->hrule);
  }
}
void
scatterplot_show_vrule (displayd *display, gboolean show) {
  if (show) {
    if (!GTK_WIDGET_VISIBLE (display->vrule))
      gtk_widget_show (display->vrule);
  } else {
    if (GTK_WIDGET_VISIBLE (display->vrule))
      gtk_widget_hide (display->vrule);
  }
}
void
scatterplot_show_rulers (displayd *display, gint projection)
{
  switch (projection) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL) {
        scatterplot_show_vrule (display, true);
        scatterplot_show_hrule (display, false);
      } else {
        scatterplot_show_vrule (display, false);
        scatterplot_show_hrule (display, true);
      }
      break;

    case XYPLOT:
      scatterplot_show_vrule (display, true);
      scatterplot_show_hrule (display, true);
      break;

    default:  /* in any other projection, no rulers */
      scatterplot_show_vrule (display, false);
      scatterplot_show_hrule (display, false);
      break;
  }
}

void
setRulerRanges (displayd *display, splotd *sp) {
  /*
   * Run 0 and sp->max through the reverse pipeline to find out
   * what their values should be in terms of the data.  Set the
   * ruler min and max to those values.
  */
  icoords scr;
  fcoords tfmin, tfmax;
  cpaneld *cpanel = &display->cpanel;

  scr.x = scr.y = 0;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmin);

  scr.x = sp->da->allocation.width;
  scr.y = sp->da->allocation.height,
  splot_screen_to_tform (cpanel, sp, &scr, &tfmax);

  /* lower, upper, position, max_size */
  if (GTK_WIDGET_VISIBLE (display->hrule))
    gtk_ruler_set_range (GTK_RULER (display->hrule),
                         tfmin.x, tfmax.x, 0, tfmax.x);
  if (GTK_WIDGET_VISIBLE (display->vrule))
    gtk_ruler_set_range (GTK_RULER (display->vrule),
                         tfmin.y, tfmax.y, 0, tfmax.y);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,         NULL, 0, "<Branch>" },
  { "/FFile/Print",  
         "",         NULL,        0, "<Item>" },
  { "/FFile/sep",   NULL,     NULL,          0, "<Separator>" },
  { "/FFile/Close",  
         "",         close_cb,        0, "<Item>" },

  { "/_Options",      NULL,         NULL, 0, "<Branch>" },
  { "/Options/Add _axes to plot",  
                      "<ctrl>a",          options_cb, 1, "<CheckItem>" },
  { "/Options/Add gridli_nes to plot",  
                      "<ctrl>n",          options_cb, 2, "<CheckItem>" },
  { "/Options/Center axes in 3D+ modes",  
                      NULL,               options_cb, 3, "<ToggleItem>" },
  { "/Options/Show points",  
                      NULL,               options_cb, 4, "<ToggleItem>" },
  { "/Options/Show undirected lines",  
                      NULL,               options_cb, 5, "<ToggleItem>" },
  { "/Options/Show directed lines",  
                      NULL,               options_cb, 6, "<ToggleItem>" },
};

displayd *
scatterplot_new () {
  GtkWidget *table, *vbox;
  displayd *display = (displayd *) g_malloc (sizeof (displayd));
  splotd *sp;

  display->displaytype = scatterplot;
  display->p1d_orientation = VERTICAL;

  init_control_panel (display, &display->cpanel, XYPLOT);

  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (display->window),
                       "displayd",
                       (gpointer) display);
  gtk_window_set_title (GTK_WINDOW (display->window),
                        "*** scatterplot display ***");
  gtk_window_set_policy (GTK_WINDOW (display->window), true, true, true);
  gtk_container_set_border_width (GTK_CONTAINER (display->window), 3);
  gtk_signal_connect (GTK_OBJECT (display->window), "delete_event",
                      GTK_SIGNAL_FUNC (delete_cb), (gpointer) display);
/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  sp_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 sp_accel_group, display->window, &mbar, (gpointer) display);
  gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);


  /*
   * Initialize a single splot
  */
  sp = splot_new (display, WIDTH, HEIGHT);
  display->splots = null;
  display->splots = g_list_append (display->splots, (gpointer) sp);


  table = gtk_table_new (3, 2, FALSE );  /* rows, columns, homogeneous */
  gtk_box_pack_start (GTK_BOX (vbox), table, false, true, 0);
  gtk_table_attach (GTK_TABLE (table),
                    sp->da, 1, 2, 0, 1,
                    GTK_EXPAND|GTK_FILL, GTK_FILL,
                    0, 0 );


  /*
   * The horizontal ruler goes on top. As the mouse moves across the
   * drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->hrule = gtk_hruler_new ();
  gtk_ruler_set_metric (GTK_RULER (display->hrule), GTK_PIXELS);
  gtk_signal_connect_object (GTK_OBJECT (sp->da), "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD (display->hrule, motion_notify_event),
    GTK_OBJECT (display->hrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->hrule, 1, 2, 1, 2,
                    GTK_EXPAND|GTK_SHRINK|GTK_FILL, GTK_FILL,
                    0, 0);

  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->vrule = gtk_vruler_new ();
  gtk_ruler_set_metric (GTK_RULER (display->vrule), GTK_PIXELS);
  gtk_signal_connect_object (GTK_OBJECT (sp->da),
                             "motion_notify_event",
                             (GtkSignalFunc) EVENT_METHOD (display->vrule,
                                                           motion_notify_event),
                             GTK_OBJECT (display->vrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    GTK_FILL, GTK_EXPAND|GTK_SHRINK|GTK_FILL,
                    0, 0 );

  gtk_widget_show (sp->da);
  gtk_widget_show (display->hrule);
  gtk_widget_show (display->vrule);
  gtk_widget_show (table);
  gtk_widget_show_all (display->window);

  setRulerRanges (display, sp);

  return display;
}
