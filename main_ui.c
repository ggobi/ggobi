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
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"
#include "display_tree.h"

#include "write_state.h"

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
  "TSplot",
};

static const char *const *viewmode_name = GGOBI(OpModeNames);

void addPreviousFilesMenu(GtkWidget *parent, GGobiInitInfo *info, ggobid *gg);

void store_session(ggobid *gg, gint action, GtkWidget *w);
void show_plugin_list(void *gg, gint action, GtkWidget *w);
void create_new_ggobi(ggobid *gg, gint action, GtkWidget *w);

void
make_control_panels (ggobid *gg) {

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

  cpanel_parcoords_make (gg);
  cpanel_scatmat_make (gg);
  cpanel_tsplot_make (gg);
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
viewmode_get (ggobid* gg) {
  return gg->viewmode;
}
PipelineMode
projection_get (ggobid* gg) {
  return gg->projection;
}

/*
 * Use the mode to determine whether the variable selection
 * panel should display checkboxes or circles
*/
gboolean
varpanel_highd (gint mode)
{
  return (mode == TOUR1D || mode == TOUR2D || mode == COTOUR);
}
gboolean
varpanel_permits_circles_or_checkboxes (gint mode)
{
  return (mode > COTOUR && mode < SCATMAT);
}
/*
 * Use the widget state to figure out which is currently displayed.
*/
gboolean
varpanel_shows_circles (ggobid *gg)
{
  datad *d = gg->current_display->d;
  return GTK_WIDGET_MAPPED (d->vcirc_ui.vbox);
}
gboolean
varpanel_shows_checkboxes (ggobid *gg)
{
  datad *d = gg->current_display->d;
  return GTK_WIDGET_MAPPED (d->vcbox_ui.swin);
}

static void
varpanel_reinit (ggobid *gg)
{
  GSList *l;
  datad *d;

  if (varpanel_highd(gg->projection) && varpanel_shows_checkboxes (gg))
  {  /*-- remove checkboxes and add circles --*/
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      /*
       * add a reference to the checkboxes' scrolled window
       * (so it won't disappear), then remove it from the ebox.
      */
      gtk_widget_ref (d->vcbox_ui.swin);
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.ebox),
                                           d->vcbox_ui.swin);
      /*
       * Now add the parent vbox for the table of variable circles
       * to the ebox
      */
      gtk_container_add (GTK_CONTAINER (d->varpanel_ui.ebox),
                                        d->vcirc_ui.vbox);
      /*-- update the reference count for the vbox --*/
      if (GTK_OBJECT (d->vcirc_ui.vbox)->ref_count > 1)
        gtk_widget_unref (d->vcirc_ui.vbox);
    }
  } else if (!varpanel_highd(gg->projection) && varpanel_shows_circles (gg))
  {  /*-- remove circles and add checkboxes --*/
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      gtk_widget_ref (d->vcirc_ui.vbox);
      gtk_container_remove (GTK_CONTAINER (d->varpanel_ui.ebox),
                                           d->vcirc_ui.vbox);
      gtk_container_add (GTK_CONTAINER (d->varpanel_ui.ebox),
                                        d->vcbox_ui.swin);
      if (GTK_OBJECT (d->vcbox_ui.swin)->ref_count > 1)
        gtk_widget_unref (d->vcbox_ui.swin);
    }
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
      /* Add a reference to the widget so it isn't destroyed */
      gtk_widget_ref (gg->control_panel[gg->prev_viewmode]);
      gtk_container_remove (GTK_CONTAINER (gg->viewmode_frame),
                            gg->control_panel[gg->prev_viewmode]);
    }

    if (gg->viewmode != NULLMODE) {
      gtk_frame_set_label (GTK_FRAME (gg->viewmode_frame),
        viewmode_name[gg->viewmode]);
      gtk_container_add (GTK_CONTAINER (gg->viewmode_frame),
        gg->control_panel[gg->viewmode]);

      /*-- avoid increasing the object's ref_count infinitely  --*/
      if (GTK_OBJECT (gg->control_panel[gg->viewmode])->ref_count > 1)
        gtk_widget_unref (gg->control_panel[gg->viewmode]);
    }
  }

  /*
   * The projection type is one of P1PLOT, XYPLOT, ROTATE,
   * TOUR1D, TOUR2D or COTOUR.  It only changes if another projection
   * type is selected.  (For parcoords and scatmat plots, the
   * value of projection is irrelevant.)
  */
  if (display->displaytype == scatterplot) {

    if (gg->viewmode <= COTOUR)
      display->cpanel.projection = gg->viewmode;
    gg->projection = display->cpanel.projection;

    if (gg->projection != gg->prev_projection) {
      scatterplot_show_rulers (display, gg->projection);
      gg->prev_projection = gg->projection;
    }
  }

  if (gg->viewmode != gg->prev_viewmode) {
    /* 
     * If moving between modes whose variable selection interface
     * differs, swap in the correct display.
     */
    varpanel_reinit (gg);
  }

  gg->prev_viewmode = gg->viewmode;

  varpanel_tooltips_set (gg);
  varpanel_refresh (gg);
}

/*
 * Turn the tour procs on and off here
*/
static void
procs_activate (gboolean state, displayd *display, ggobid *gg)
{
  switch (gg->viewmode) {
    case TOUR2D:
      if (!display->cpanel.t2d_paused)
        tour2d_func (state, display, gg);
    break;
    case TOUR1D:
      if (!display->cpanel.t1d_paused)
        tour1d_func (state, display, gg);
    break;
    case COTOUR:
      if (!display->cpanel.tcorr1_paused)
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
  RedrawStyle redraw_style = NONE;

  if (state == off) {
    switch (m) {
      case XYPLOT:
      {
        extern RedrawStyle xyplot_activate (gint, displayd *, ggobid *);
        xyplot_activate (state, display, gg);
      }
      break;
      case BRUSH:
        redraw_style = brush_activate (state, display, gg);
      break;
      case IDENT:
      {
        extern RedrawStyle identify_activate (gint, displayd *, ggobid *);
        redraw_style = identify_activate (state, display, gg);
      }
      break;
      default:
      break;
    }
  } else if (state == on) {
    switch (m) {
      case P1PLOT:
      {
        extern RedrawStyle p1d_activate (gint, displayd *, ggobid *);
        p1d_activate (state, display, gg);
      }
      break;
      case XYPLOT:
      {
        extern RedrawStyle xyplot_activate (gint, displayd *, ggobid *);
        xyplot_activate (state, display, gg);
      }
      break;
      case BRUSH:
        redraw_style = brush_activate (state, display, gg);
      break;
      case SCALE:
      {
        extern void scale_click_init (splotd *sp, ggobid *gg);
        scale_click_init (sp, gg);
      }
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
        if (d->ncols < 4)
          ok = false;
      break;
      case TOUR2D:
        if (d->ncols < 3)
          ok = false;
      break;
      case TOUR1D:
        if (d->ncols < 3)
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
 * to have executed when a new viewmode
 * is selected for the current display, but not when the
 * viewmode changes because a new display becomes current.
 * Because of that, we don't put them in viewmode_activate.
*/
  PipelineMode prev_viewmode = gg->viewmode;

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
      if (gg->viewmode != BRUSH && prev_viewmode == BRUSH)
        if (cpanel->br_mode == BR_TRANSIENT)
          reinit_transient_brushing (display, gg);

      /*
       * work out which mode menus (Options, Reset, I/O) need
       * to be present, and add the needed callbacks.
      */
      viewmode_submenus_update (prev_viewmode, gg);

      /*-- redraw this display --*/
      display_tailpipe (display, FULL, gg);

      /*-- redraw as needed for transient brushing and identify --*/
      if (redraw_style != NONE) {
        displays_plot (sp, redraw_style, gg);
      }

/**/  return (action);
    }
  }

  return(-1);
}

extern void display_write_svg (ggobid *);
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

#ifdef USE_XML
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Store session",   
       NULL,   
       (GtkItemFactoryCallback) store_session, 
       0 },
#endif
#ifdef PRINTING_IMPLEMENTED
  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
  { "/File/Print",
       NULL,    
       (GtkItemFactoryCallback) display_write_svg,         
       0 },
#endif

  { "/File/sep",         NULL,     NULL,          0, "<Separator>" },
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
  { "/Tools/Brush by variable ...", 
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


#ifndef AS_GGOBI_LIBRARY
/*
  Wrapper for gtk_main_quit so that we can override this in
  other applications to avoid quitting when the user selects
  the Quit button.
 */
void
quit_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
  gtk_main_quit();
}

#endif

void 
make_ui (ggobid *gg) {
  GtkWidget *window;
  GtkWidget *hbox, *vbox;

  gg->tips = gtk_tooltips_new ();

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gg->main_window = window;
  GGobi_widget_set (window, gg, true);

  gtk_window_set_policy (GTK_WINDOW (window), true, true, false);

  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (ggobi_close), gg);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
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
  gg->viewmode_frame = gtk_frame_new (
    (gg->viewmode == NULLMODE) ? "" :
      viewmode_name[gg->viewmode]);

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
  int i;
  GtkWidget *el;
  InputDescription *input;
  if(info) {
    for(i = 0 ; i < info->numInputs ; i++) {
     input = &(info->descriptions[i].input);
     if(input->fileName) {
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

  gg = ggobi_alloc();

     /*-- some initializations --*/
  gg->displays = NULL;
  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);
  make_ui (gg);

  read_input(desc, gg);

  start_ggobi(gg, init_data, true);

  return(gg);
}


#ifdef SUPPORT_PLUGINS
void
show_plugin_list(void *garbage, gint action, GtkWidget *w)
{
  extern GtkWidget * showPluginInfo (GList *plugins);
  showPluginInfo(sessionOptions->info->plugins);
}
#endif


#ifdef USE_XML
void
store_session(ggobid *gg, gint action, GtkWidget *w)
{
  write_ggobi_as_xml(gg, "duncan");
}
#endif


void
create_new_ggobi(ggobid *gg, gint action, GtkWidget *w)
{
  create_ggobi(NULL);
}
