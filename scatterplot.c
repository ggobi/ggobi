/* scatterplot.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   370
#define HEIGHT  370

void
scatterplot_show_hrule (displayd *display, gboolean show) 
{
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

    case TOUR2D:
/*
 * Di, I'm commenting these out for now -- the error messages
 * are distracting -- dfs.
      scatterplot_show_vrule (display, true);
      scatterplot_show_hrule (display, true);
      break;
*/

    default:  /* in any other projection, no rulers */
      scatterplot_show_vrule (display, false);
      scatterplot_show_hrule (display, false);
      break;
  }
}

void
ruler_ranges_set (displayd *display, splotd *sp, ggobid *gg) {
  /*
   * Run 0 and sp->max through the reverse pipeline to find out
   * what their values should be in terms of the data.  Set the
   * ruler min and max to those values.
  */
  icoords scr;
  fcoords tfmin, tfmax;
  cpaneld *cpanel = &display->cpanel;

  if (display->hrule == NULL)
    return;

  scr.x = scr.y = 0;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmin, gg);

  scr.x = sp->max.x;
  scr.y = sp->max.y;
  splot_screen_to_tform (cpanel, sp, &scr, &tfmax, gg);

  /*
   * Reset only if necessary:  if the ruler is visible and the
   * ranges have changed
  */
  if (GTK_WIDGET_VISIBLE (display->hrule)) {
    if (((gfloat) GTK_EXT_RULER (display->hrule)->lower != tfmin.x) ||
        ((gfloat) GTK_EXT_RULER (display->hrule)->upper != tfmax.x))
    {
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->hrule),
                               (gdouble) tfmin.x, (gdouble) tfmax.x);
    }
  }

  if (GTK_WIDGET_VISIBLE (display->vrule)) {
    if (((gfloat) GTK_EXT_RULER (display->vrule)->upper != tfmin.y) ||
        ((gfloat) GTK_EXT_RULER (display->vrule)->lower != tfmax.y))
    {
      gtk_ext_ruler_set_range (GTK_EXT_RULER (display->vrule),
                               (gdouble) tfmax.y, (gdouble) tfmin.y);
    }
  }
}

/*----------------------------------------------------------------------*/
/*                          Options section                             */
/*----------------------------------------------------------------------*/

static GtkItemFactoryEntry menu_items[] = {
  {"/_FFile",        
    NULL,    
    NULL,            
    0,            
    "<Branch>" },
  {"/FFile/Print",   
    "",      
    (GtkItemFactoryCallback)display_print_cb,
    0,
    "<Item>" },
  {"/FFile/sep",     
    NULL,    
    NULL,            
    0,
    "<Separator>" },
  {"/FFile/Close",   
    "",      
    (GtkItemFactoryCallback) display_close_cb,
    0,
    "<Item>" },
};

static void
scatterplot_display_menus_make (displayd *display,
                                GtkAccelGroup *accel_group,
                                GtkSignalFunc func,
                                GtkWidget *mbar, ggobid *gg)
{
  GtkWidget *options_menu, *link_menu;
  GtkWidget *submenu;

/*
 * Options menu
*/
  submenu = submenu_make ("_Options", 'O', accel_group);
  options_menu = gtk_menu_new ();

  CreateMenuCheck (display, options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  CreateMenuCheck (display, options_menu, "Show lines (undirected)",
    func, GINT_TO_POINTER (DOPT_SEGS_U), off, gg);
  CreateMenuCheck (display, options_menu, "Show lines (directed)",
    func, GINT_TO_POINTER (DOPT_SEGS_D), off, gg);
/*
  CreateMenuCheck (display, options_menu, "Show missings",
    func, GINT_TO_POINTER (DOPT_MISSINGS), on, gg);
*/
  CreateMenuCheck (display, options_menu, "Show gridlines",
    func, GINT_TO_POINTER (DOPT_GRIDLINES), off, gg);

  /* Add a separator */
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (display, options_menu, "Show axes (3D+ modes)",
    func, GINT_TO_POINTER (DOPT_AXES), on, gg);
  CreateMenuCheck (display, options_menu, "Center axes (3D+ modes)",
    func, GINT_TO_POINTER (DOPT_AXES_C), on, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), options_menu);
  submenu_append (submenu, mbar);

  gtk_widget_show (submenu);

/*
 * Link menu
*/
  submenu = submenu_make ("_Link", 'L', accel_group);
  link_menu = gtk_menu_new ();

  CreateMenuCheck (display, link_menu, "Link to other plots",
    func, GINT_TO_POINTER (DOPT_LINK), on, gg);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), link_menu);
  submenu_append (submenu, mbar);
  gtk_widget_show (submenu);
}


displayd *
scatterplot_new (gboolean missing_p, splotd *sp, datad *d, ggobid *gg) {
  GtkWidget *table, *vbox;
  GtkWidget *mbar;
  displayd *display;

  if (d == NULL || d->ncols < 2)
    return (NULL);

  if (sp == NULL) {
    display = display_alloc_init (scatterplot, missing_p, d, gg);
  } else {
    display = (displayd*) sp->displayptr;
    display->d = d;
  }


  /* Want to make certain this is true, and perhaps it may be different
     for other plot types and so not be set appropriately in DefaultOptions.
    display->options.axes_center_p = true; 
   */

  scatterplot_cpanel_init (&display->cpanel, XYPLOT, gg);

  display_window_init (display, 3, gg);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  gg->app.sp_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 gg->app.sp_accel_group, display->window, &mbar,
                 (gpointer) display);
  /*
   * After creating the menubar, and populating the file menu,
   * add the Options and Link menus another way
  */
  scatterplot_display_menus_make (display, gg->app.sp_accel_group,
   (GtkSignalFunc) display_options_cb, mbar, gg);
  gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);


  /*
   * Initialize a single splot
  */
  if (sp == NULL) {
    sp = splot_new (display, WIDTH, HEIGHT, gg);
  }

  display->splots = NULL;
  display->splots = g_list_append (display->splots, (gpointer) sp);

  /* 
   * Initialize tour
   */
  if (display->displaytype == scatterplot && d->ncols > 2) 
    display_tour_init(display, gg);

  table = gtk_table_new (3, 2, false);  /* rows, columns, homogeneous */
  gtk_box_pack_start (GTK_BOX (vbox), table, true, true, 0);
  gtk_table_attach (GTK_TABLE (table),
                    sp->da, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    (GtkAttachOptions) (GTK_SHRINK|GTK_EXPAND|GTK_FILL),
                    0, 0 );


  /*
   * The horizontal ruler goes on top. As the mouse moves across the
   * drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->hrule = gtk_ext_hruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da), "motion_notify_event",
    (GtkSignalFunc) EVENT_METHOD (display->hrule, motion_notify_event),
    GTK_OBJECT (display->hrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->hrule, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL), 
                    (GtkAttachOptions) GTK_FILL,
                    0, 0);

  /*
   * The vertical ruler goes on the left. As the mouse moves across
   * the drawing area, a motion_notify_event is passed to the
   * appropriate event handler for the ruler.
  */
  display->vrule = gtk_ext_vruler_new ();
  gtk_signal_connect_object (GTK_OBJECT (sp->da),
                             "motion_notify_event",
                             (GtkSignalFunc) EVENT_METHOD (display->vrule,
                                                           motion_notify_event),
                             GTK_OBJECT (display->vrule));

  gtk_table_attach (GTK_TABLE (table),
                    display->vrule, 0, 1, 0, 1,
                    (GtkAttachOptions) GTK_FILL, 
                    (GtkAttachOptions) (GTK_EXPAND|GTK_SHRINK|GTK_FILL),
                    0, 0 );

  gtk_widget_show (sp->da);
  gtk_widget_show (display->hrule);
  gtk_widget_show (display->vrule);
  gtk_widget_show (table);

  /*-- position the display toward the lower left of the main window --*/
  display_set_position (display, gg);

  gtk_widget_show_all (display->window);
  
  ruler_ranges_set (display, sp, gg);

  return display;
}
