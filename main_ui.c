/* main_ui.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#include <stdlib.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "display_tree.h"

#include "write_state.h"

#ifdef SUPPORT_PLUGINS
#include "plugin.h"
#endif


#ifdef TEST_GGOBI_EVENTS
#include "testEvents.h"
#endif


const char *const GGOBI(OpModeNames)[] = {
  "1D Plot",
  "XYPlot",
  "Rotation",
  "1D Tour",
  "2D Tour",
  "Correlation Tour",
  "Scale",
  "Brush",
  "Identify",
  "Edit Edges",
  "Move Points",

  "Scatmat",
  "Parcoords",
#ifdef EXTENDED_MODES
  "TSplot",
  "Barchart"
#endif
};


static const char *const *viewmode_name = GGOBI(OpModeNames);

void addPreviousFilesMenu(GtkWidget *parent, GGobiInitInfo *info, ggobid *gg);

void store_session(ggobid *gg, gint action, GtkWidget *w);
void show_plugin_list(ggobid *gg, gint action, GtkWidget *w);
void create_new_ggobi(ggobid *gg, gint action, GtkWidget *w);

void
make_control_panels (ggobid *gg) 
{
  cpanel_p1dplot_make (gg);
  cpanel_xyplot_make (gg);
#ifdef ROTATION_IMPLEMENTED
  cpanel_rotation_make (gg);
#endif
  cpanel_tour1d_make (gg);
  cpanel_tour2d_make (gg);
  cpanel_ctour_make (gg);

  cpanel_brush_make (gg);
  cpanel_scale_make (gg);
  cpanel_identify_make (gg);
#ifdef EDIT_EDGES_IMPLEMENTED
  cpanel_edgeedit_make (gg);
#endif
  cpanel_movepts_make (gg);


  cpanel_scatmat_make (gg);

  /* Leave the extendeded display types to be done on demand. */
}

void
tooltips_show_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  if (w->active)
    gtk_tooltips_enable (gg->tips);
  else
    gtk_tooltips_disable (gg->tips);
}

void
cpanel_show_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);
  if (w->active)
    gtk_widget_show (gg->viewmode_frame);
  else
    gtk_widget_hide (gg->viewmode_frame);
}


PipelineMode
viewmode_get (ggobid* gg) 
{
  return gg->viewmode;
}
PipelineMode
projection_get (ggobid* gg) 
{
  return gg->projection;
}

/*
 * Use the mode to determine whether the variable selection
 * panel should display checkboxes or circles
*/
static gboolean
varpanel_highd (displayd *display)
{
  gboolean highd = false;

  if(display && GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     GtkGGobiExtendedDisplayClass *klass;
     klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
     if(klass->varpanel_highd)
       highd = klass->varpanel_highd(display);
  }

  return (highd);
}
gboolean
varpanel_permits_circles_or_checkboxes (gint mode)
{
  return (mode > COTOUR && mode < SCATMAT);
}
/*
 * Use the widget state to figure out which is currently displayed.
*/
static gboolean
varpanel_shows_circles (datad *d)
{
  return GTK_WIDGET_REALIZED (d->vcirc_ui.ebox);
}
static gboolean
varpanel_shows_checkboxes (datad *d)
{
  return GTK_WIDGET_REALIZED (d->vcbox_ui.ebox);
}

void
varpanel_reinit (ggobid *gg)
{
  datad *d;
  gboolean highd;
  displayd *display = gg->current_display;
  if (display == NULL) return;

  d = display->d;
  highd = varpanel_highd (display);

  if (highd && varpanel_shows_checkboxes (d)) {
    /*
     * Add the ebox for the table of variable circles/rectangles
     * to the paned widget
    */
    varcircles_visibility_set (display, gg);
    gtk_paned_pack2 (GTK_PANED (d->varpanel_ui.hpane),
      d->vcirc_ui.ebox, true, true);
    gtk_paned_set_handle_size (GTK_PANED(d->varpanel_ui.hpane), 10);
    gtk_paned_set_gutter_size (GTK_PANED(d->varpanel_ui.hpane), 15);

    /*-- update the reference count for the ebox --*/
#if GTK_MAJOR_VERSION == 1
    if (GTK_OBJECT (d->vcirc_ui.ebox)->ref_count > 1)
#else
    if (G_OBJECT (d->vcirc_ui.ebox)->ref_count > 1)
#endif
      gtk_widget_unref (d->vcirc_ui.ebox);

  } else if (!highd && varpanel_shows_circles (d)) {
    /*-- remove circles/rectangles --*/
    gtk_widget_ref (d->vcirc_ui.ebox);
    gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.hpane),
                                         d->vcirc_ui.ebox);
    gtk_paned_set_handle_size (GTK_PANED(d->varpanel_ui.hpane), 0);
    gtk_paned_set_gutter_size (GTK_PANED(d->varpanel_ui.hpane), 0);
    /*-- set the handle position all the way to the right --*/
    gtk_paned_set_position (GTK_PANED(d->varpanel_ui.hpane), -1);


    /*-- adjust the reference count --*/
#if GTK_MAJOR_VERSION == 1
    if (GTK_OBJECT (d->vcbox_ui.ebox)->ref_count > 1)
#else
    if (G_OBJECT (d->vcbox_ui.ebox)->ref_count > 1)
#endif
      gtk_widget_unref (d->vcbox_ui.ebox);
  }
}

void 
viewmode_set (PipelineMode m, ggobid *gg)
{
/*
 * This could be called ui_mode_set or main_window_mode_set,
 * because it just sets up the mode_frame and the variable
 * selection panel.
*/
  displayd *display = gg->current_display;

  gg->viewmode = m;

  if (gg->viewmode != gg->prev_viewmode) {

    if (gg->prev_viewmode != NULLMODE) {
      GtkWidget *modeBox = gg->current_control_panel;
      if(modeBox) {
        gtk_widget_ref (modeBox);
        gtk_container_remove (GTK_CONTAINER (gg->viewmode_frame), modeBox);
      }
    }

    if (gg->viewmode != NULLMODE) {
      gchar * modeName = NULL;
      GtkWidget *panel = NULL;

      if(gg->viewmode < EXTENDED_DISPLAY_MODE) {
        modeName = (gchar *) viewmode_name[gg->viewmode];
        panel = gg->control_panel[gg->viewmode];
      } else {
        GtkGGobiExtendedDisplayClass *klass;
        klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
        panel = klass->viewmode_control_box(display, gg->viewmode,
          &modeName, gg);
      }

      gtk_frame_set_label (GTK_FRAME (gg->viewmode_frame), modeName);
      gtk_container_add (GTK_CONTAINER (gg->viewmode_frame), panel);
      gg->current_control_panel = panel;

      /*-- avoid increasing the object's ref_count infinitely  --*/
#if GTK_MAJOR_VERSION == 1
      if (GTK_OBJECT (panel)->ref_count > 1)
#else
      if (G_OBJECT (panel)->ref_count > 1)
#endif
        gtk_widget_unref (panel);
    }
  }

  /*
   * The projection type is one of P1PLOT, XYPLOT, ROTATE,
   * TOUR1D, TOUR2D or COTOUR.  It only changes if another projection
   * type is selected.  (For parcoords and scatmat plots, the
   * value of projection is irrelevant.)
  */
   {
    GtkGGobiExtendedDisplayClass *klass;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
    if(klass->viewmode_set)
        klass->viewmode_set(display, gg);
   }

   if (gg->viewmode != gg->prev_viewmode) {
    /* 
     * If moving between modes whose variable selection interface
     * differs, swap in the correct display.
     */
    varpanel_reinit (gg);
  }

  gg->prev_viewmode = gg->viewmode;

  varpanel_tooltips_set (display, gg);
  varpanel_refresh (display, gg);
}

/*
 * Turn the tour procs on and off here
*/
static void
procs_activate (gboolean state, displayd *display, ggobid *gg)
{
  switch (gg->viewmode) {
    case TOUR2D:
      if (!display->cpanel.t2d.paused)
        tour2d_func (state, display, gg);
    break;
    case TOUR1D:
      if (!display->cpanel.t1d.paused)
        tour1d_func (state, display, gg);
    break;
    case COTOUR:
      if (!display->cpanel.tcorr1.paused)
        tourcorr_func (state, display, gg);
    break;
    default:
    break;
  }
}

RedrawStyle
viewmode_activate (splotd *sp, PipelineMode m, gboolean state, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  RedrawStyle redraw_style = NONE;

  if (state == off) {
    switch (m) {
      case XYPLOT:
      {
        xyplot_activate (state, display, gg);
      }
      break;
      case BRUSH:
        redraw_style = brush_activate (state, display, gg);
      break;
      case IDENT:
      {
        redraw_style = identify_activate (state, display, gg);
      }
      break;
      case TOUR2D:
      {
        if (cpanel->t2d.manip_mode != MANIP_OFF)
          splot_cursor_set ((gint) NULL, sp);
      }
      break;
      case COTOUR:
      {
        if (cpanel->tcorr.manip_mode != MANIP_OFF)
          splot_cursor_set ((gint) NULL, sp);
      }
      break;
      case SCALE:
        /*-- for insurance, because sometimes scaling doesn't quit --*/
        disconnect_motion_signal (sp);
        /*-- --*/
      break;
      default:
      break;
    }
  } else if (state == on) {
    switch (m) {
      case P1PLOT:
      {
        p1d_activate (state, display, gg);
      }
      break;
      case XYPLOT:
      {
        xyplot_activate (state, display, gg);
      }
      break;
      case BRUSH:
        redraw_style = brush_activate (state, display, gg);
      break;
      case SCALE:
      {
        scale_click_init (sp, gg);
      }
      break;
      case TOUR2D:
        if (cpanel->t2d.manip_mode != MANIP_OFF)
          splot_cursor_set (GDK_HAND2, sp);
      break;
      case COTOUR:
        if (cpanel->tcorr.manip_mode != MANIP_OFF)
          splot_cursor_set (GDK_HAND2, sp);
      break;
      default:
      break;
    }
  }
  return redraw_style;
}

void
viewmode_set_cb (GtkWidget *widget, gint action)
{
  ggobid *gg = GGobiFromWidget(widget,true);
  GGOBI(full_viewmode_set)((PipelineMode) action, gg);
}

/*
 * Verify that the number of variables is large enough before
 * allowing the projection to be reset.
*/
gboolean
projection_ok (gint m, displayd *display)
{
  gboolean ok = true;
  datad *d = display->d;

  /*-- if the mode is a projection-setting mode ... --*/
  if (m <= COTOUR) {
    switch (m) {
      case COTOUR:
        if (d->ncols < MIN_NVARS_FOR_COTOUR)
          ok = false;
      break;
      case TOUR2D:
        if (d->ncols < MIN_NVARS_FOR_TOUR2D)
          ok = false;
      break;
      case TOUR1D:
        if (d->ncols < MIN_NVARS_FOR_TOUR1D)
          ok = false;
      break;
      case XYPLOT:
        if (d->ncols < 2)
          ok = false;
      break;
      case P1PLOT:
        if (d->ncols < 1)
          ok = false;
      break;
      default:
      break;
    }
  }

  return ok;
}

gint
GGOBI(full_viewmode_set)(gint action, ggobid *gg)
{
/*
 * Some of the routines called here, like procs_activate
 * and reinit_transient brushing, are routines that we want
 * to have executed when a new viewmode is selected for the
 * current display, but not when the viewmode changes because
 * a new display becomes current.
 * Because of that, we don't put them in viewmode_activate.
*/
  PipelineMode prev_viewmode = gg->viewmode;
  gboolean reinit_transient_p = false;

  if (gg->current_display != NULL && gg->current_splot != NULL) {
    splotd *sp = gg->current_splot;
    displayd *display = gg->current_display;
    cpaneld *cpanel = &display->cpanel;
    RedrawStyle redraw_style = NONE;

    if (projection_ok (action, display)) {
      sp_event_handlers_toggle (sp, off);
      redraw_style = viewmode_activate (sp, gg->viewmode, off, gg);
      procs_activate (off, display, gg);

      display->cpanel.viewmode = (PipelineMode) action;
      viewmode_set (display->cpanel.viewmode, gg);

      sp_event_handlers_toggle (sp, on);
      viewmode_activate (sp, gg->viewmode, on, gg);

      procs_activate (on, display, gg);
      if (gg->viewmode != BRUSH && prev_viewmode == BRUSH) {
        if (cpanel->br_mode == BR_TRANSIENT) {
          reinit_transient_p = true;
          reinit_transient_brushing (display, gg);
        }
      }

      /*
       * work out which mode menus (Options, Reset, I/O) need
       * to be present, and add the needed callbacks.
      */
      viewmode_submenus_update (prev_viewmode, gg->current_display, gg);

      /*-- redraw this display --*/
      display_tailpipe (display, FULL, gg);

      /*-- redraw as needed for transient brushing and identify --*/
      if (redraw_style != NONE || reinit_transient_p) {
        displays_plot (sp, FULL, gg);
      }

/**/  return (action);
    }
  }

  return(-1);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_File",            NULL,     NULL,             0, "<Branch>" },
  { "/File/Open ...",
       NULL,    
       (GtkItemFactoryCallback) filename_get_r,  
       0 },
  { "/File/New",
       NULL,    
       (GtkItemFactoryCallback) create_new_ggobi,  
       0 },
  { "/File/Save ...",   
       NULL,    
       (GtkItemFactoryCallback) writeall_window_open,    
       2 },

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Store session",   
       NULL,   
       (GtkItemFactoryCallback) store_session, 
       0 },
#ifdef PRINTING_IMPLEMENTED
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Print",
       NULL,    
       (GtkItemFactoryCallback) display_write_svg,         
       0 },
#endif

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Close",   
       "<ctrl>C",   
       (GtkItemFactoryCallback) ggobi_close, 
       0 },
  { "/File/Quit",   
       "<ctrl>Q",   
       (GtkItemFactoryCallback) quit_ggobi, 
       0 },


  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Variable manipulation ...", 
       NULL,        
       (GtkItemFactoryCallback) vartable_open,   
       0,
       NULL },
  { "/Tools/Variable transformation ...", 
       NULL,        
       (GtkItemFactoryCallback) transform_window_open,
       0,
       NULL },
  { "/Tools/Sphering ...", 
       NULL,        
       (GtkItemFactoryCallback) sphere_panel_open,
       0,
       NULL },
#ifdef INFERENCE_IMPLEMENTED
  { "/Tools/Inference ...", 
       NULL,        
       (GtkItemFactoryCallback) NULL,  /*-- inference_window_open --*/
       0,
       NULL },
#endif
  { "/Tools/Variable jittering ...", 
       NULL,        
       (GtkItemFactoryCallback) jitter_window_open,   
       0,
       NULL },
  { "/Tools/Color schemes ...", 
       NULL,        
       (GtkItemFactoryCallback) wvis_window_open,   
       0,
       NULL },

  /*-- Tools that apply to cases --*/
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
  { "/Tools/Color & glyph groups ...", 
       NULL,        
       (GtkItemFactoryCallback) cluster_window_open,
       0,
       NULL },
  { "/Tools/Case subsetting and sampling ...", 
       NULL,        
       (GtkItemFactoryCallback) subset_window_open,   
       0,
       NULL },
  { "/Tools/sep",     NULL, NULL, 0, "<Separator>" },
#ifdef SMOOTH_IMPLEMENTED
  { "/Tools/Smooth ...", 
       NULL,        
       (GtkItemFactoryCallback) smooth_window_open,   
       0,
       NULL },
#endif

  { "/Tools/Missing values ...", 
       NULL,        
       (GtkItemFactoryCallback) impute_window_open,   
       0,
       NULL },

  {"/Dis_playTree", NULL, NULL, 0, "<Branch>"},
  { "/DisplayTree/Displays",    
       NULL, 
       (GtkItemFactoryCallback) show_display_tree,
       2},

  { "/_Help",                NULL, NULL, 0, "<LastBranch>" },
  { "/Help/About GGobi",
       NULL,
       (GtkItemFactoryCallback) splash_show,
       0 },
/*
  { "/Help/About help ...",  NULL, NULL, 0, NULL },
*/

#ifdef SUPPORT_PLUGINS
  { "/Help/About plugins ...",
       NULL,
       (GtkItemFactoryCallback) show_plugin_list,
       (gint) NULL },
#endif
};


/*
#ifndef AS_GGOBI_LIBRARY
*/
/*
  Wrapper for gtk_main_quit so that we can override this in
  other applications to avoid quitting when the user selects
  the Quit button.
 */
void
quit_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
#ifdef SUPPORT_PLUGINS
  extern void closePlugins(ggobid *gg);
  gint n, i;
  ggobid *el;
  n = GGobi_getNumGGobis();
  for(i = 0; i < n ; i++) {
    el = GGobi_ggobi_get(i);
    if(el != gg) 
      closePlugins(el);
  }
  closePlugins(gg);
#endif
  gtk_main_quit();
}


void 
make_ui (ggobid *gg) 
{
  GtkWidget *window;
  GtkWidget *hbox, *vbox;

  gg->tips = gtk_tooltips_new ();

  gg->main_window = window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  GGobi_widget_set (window, gg, true);

#ifdef TEST_GGOBI_EVENTS
  gtk_signal_connect (GTK_OBJECT(gg),
   "splot_new", test_new_plot_cb, (gpointer) "A new plot");
  gtk_signal_connect(GTK_OBJECT(gg),
   "datad_added", test_data_add_cb, NULL);
  gtk_signal_connect(GTK_OBJECT(gg),
   "sticky_point_added", test_sticky_points, NULL);
  gtk_signal_connect(GTK_OBJECT(gg),
   "sticky_point_removed", test_sticky_points, NULL);
#endif

/*
 * I used to set allow_shrink to true, but somehow it causes what we experience as
 * auto_shrink-ing.  That is, when changing viewmode or selecting variables, the size
 * of the ggobi console window can suddenly change.  This seems to fix that, with the
 * perhaps undesirable side effect that I can't reduce the size of the console below
 * its initial size.  -- dfs
*/
/*gtk_window_set_policy (GTK_WINDOW (window), allow_shrink, allow_grow, auto_shrink);*/
  gtk_window_set_policy (GTK_WINDOW (window), false,        true,       false);

  gtk_signal_connect(GTK_OBJECT (window), "delete_event",
                     GTK_SIGNAL_FUNC (ggobi_close), gg);
  gtk_signal_connect(GTK_OBJECT (window), "destroy_event",
                     GTK_SIGNAL_FUNC (ggobi_close), gg); 

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (false, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  gg->main_accel_group = gtk_accel_group_new ();
  gg->main_menu_factory = get_main_menu (menu_items,
    sizeof (menu_items) / sizeof (menu_items[0]),
    gg->main_accel_group, window,
    &gg->main_menubar, (gpointer) gg);

#ifdef SUPPORT_INIT_FILES
  if (sessionOptions->info && sessionOptions->info->numInputs > 0) {
    GtkWidget *w;
    w = gtk_item_factory_get_widget(gg->main_menu_factory, "/File");
    addPreviousFilesMenu(w, sessionOptions->info, gg);
  }
#endif

  display_menu_init (gg);

  gtk_box_pack_start (GTK_BOX (vbox), gg->main_menubar, false, false, 0);

  gtk_accel_group_lock (gg->main_accel_group);

  hbox = gtk_hbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, true, true, 0);

/*
 * Create a frame to hold the mode panels, set its label
 * and contents, using the default mode for the default display.
*/
  gg->viewmode_frame = gtk_frame_new ((gg->viewmode == NULLMODE) 
                                       ? "" : viewmode_name[gg->viewmode]);

  gtk_box_pack_start (GTK_BOX (hbox), gg->viewmode_frame, false, false, 3);
  gtk_container_set_border_width (GTK_CONTAINER (gg->viewmode_frame), 3);
  gtk_frame_set_shadow_type (GTK_FRAME (gg->viewmode_frame),
    GTK_SHADOW_IN);

  make_control_panels (gg);
  if (gg->viewmode != NULLMODE)
    gtk_container_add (GTK_CONTAINER (gg->viewmode_frame),
                       gg->control_panel[gg->viewmode]);

  /*-- Variable selection panel --*/
  varpanel_make (hbox, gg);

  gtk_widget_show_all (hbox);

  /*-- at this point, the mode could be NULLMODE, P1PLOT, or XYPLOT --*/
  /*mode_submenus_activate (NULL, gg->viewmode, on, gg);*/
  {
    void viewmode_submenus_initialize (PipelineMode mode, ggobid *gg);
    viewmode_submenus_initialize (gg->viewmode, gg);
  }

  if(sessionOptions->showControlPanel)
      gtk_widget_show_all (window);
}


const gchar * const* 
GGOBI(getOpModeNames)(int *n)
{
  /*  extern const gchar *const* GGOBI(ModeNames); */
  *n = sizeof(GGOBI(OpModeNames))/sizeof(GGOBI(OpModeNames)[0]);
  return (GGOBI(OpModeNames));
}




#ifdef SUPPORT_INIT_FILES

void load_previous_file(GtkWidget *w, gpointer cbd);
/*
  Add the previous input sources to the menu.
 */
void
addPreviousFilesMenu(GtkWidget *parent, GGobiInitInfo *info, ggobid *gg)
{
  gint i;
  GtkWidget *el;
  InputDescription *input;
  if(info) {
    for(i = 0 ; i < info->numInputs ; i++) {
      input = &(info->descriptions[i].input);
      if(input && input->fileName) {
        el = gtk_menu_item_new_with_label(input->fileName);
        gtk_signal_connect(GTK_OBJECT(el), "activate",
                           GTK_SIGNAL_FUNC(load_previous_file),
                           info->descriptions + i);
        GGobi_widget_set(el, gg, true);
        gtk_menu_insert(GTK_MENU(parent), el, 3 + i + 1);
      }
    }
  }
}


ggobid *create_ggobi(InputDescription *desc);

void
load_previous_file(GtkWidget *w, gpointer cbd)
{
  InputDescription *desc;
  GGobiDescription *gdesc;
  ggobid *gg;

  gg = GGobiFromWidget(w, false);
  gdesc = (GGobiDescription*) cbd;
  desc =  &(gdesc->input);

  if(g_slist_length(gg->d) > 0)
    create_ggobi(desc);
  else {
    read_input(desc, gg);
    /* Need to avoid the initial scatterplot. */
    start_ggobi(gg, true, gdesc->displays == NULL);
  }


  if (gdesc->displays) {
    gint i, n;
    GGobiDisplayDescription* dpy;
    n = g_list_length(gdesc->displays);
    for (i = 0; i < n ; i++) {    
      dpy = (GGobiDisplayDescription*) g_list_nth_data(gdesc->displays, i);
      createDisplayFromDescription(gg, dpy);
      /*
       * This line is added to counteract something done in
       * display_add:  if there's a previous splot, display_add
       * kindly arranges for it to get a QUICK redraw just to
       * eliminate the border.  The API, though, allows many
       * plots to be added before anything is drawn.  As a
       * result, if the first display is a parcoords plot, the
       * first splot is copied from pixmap0 to pixmap1 before it
       * has been drawn to pixmap1, resulting in garbage on the
       * screen.
      */
      gg->current_splot = NULL;
    }
  } 
}
#endif

/*
 This replicates code elsewhere and the two should be merged.
 */
ggobid *
create_ggobi(InputDescription *desc)
{
  gboolean init_data = true;
  ggobid *gg;

  gg = ggobi_alloc(NULL);

     /*-- some initializations --*/
  gg->displays = NULL;
  globals_init (gg);      /*-- variables that don't depend on the data --*/
  tour_manip_colors_init (gg); /*-- move to the tour code --*/
  make_ui (gg);
  gg->input = desc;

  read_input(desc, gg);

  if(sessionOptions->info != NULL) {
    extern gboolean registerPlugins(ggobid *gg, GList *plugins);
    registerPlugins(gg, sessionOptions->info->plugins);
  }

  start_ggobi(gg, init_data, sessionOptions->info->createInitialScatterPlot);

  return(gg);
}


#ifdef SUPPORT_PLUGINS
void
show_plugin_list(ggobid *gg, gint action, GtkWidget *w)
{
  if(sessionOptions->info && sessionOptions->info->plugins)
    showPluginInfo(sessionOptions->info->plugins,
      sessionOptions->info->inputPlugins, (ggobid*) gg);
}
#endif



void
store_session_in_file(GtkWidget *btn, GtkWidget *selector)
{
  gchar *fileName;
  ggobid *gg;

  fileName = gtk_file_selection_get_filename(GTK_FILE_SELECTION(selector));
  if(fileName && fileName[0]) {
    gg = gtk_object_get_data(GTK_OBJECT(selector), "ggobi");
    write_ggobi_as_xml(gg, fileName, NULL);
    gtk_widget_destroy(selector);
  } else {
    quick_message("Pick a file", true);
  }
}

void
store_session(ggobid *gg, gint action, GtkWidget *w)
{
  GtkWidget *dlg;
  if(!sessionOptions->info->sessionFile) {
    char buf[1000];
    sprintf(buf,"%s%c%s", getenv("HOME"), G_DIR_SEPARATOR, ".ggobi-session");
    dlg = gtk_file_selection_new("Save ggobi session");
    gtk_object_set_data(GTK_OBJECT(dlg), "ggobi", (gpointer) gg);
    gtk_file_selection_set_filename(GTK_FILE_SELECTION(dlg), buf);
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dlg)->ok_button),
      "clicked", GTK_SIGNAL_FUNC (store_session_in_file), dlg);

    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION(dlg)->cancel_button),
      "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
      (gpointer) dlg);

    gtk_widget_show(dlg);
  } else {
    ggobi_write_session(sessionOptions->info->sessionFile);
    /* write_ggobi_as_xml(gg, sessionOptions->info->sessionFile); */
  }
}



void
create_new_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
  create_ggobi(NULL);
}
