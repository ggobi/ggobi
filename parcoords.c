/* parcoords.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   150
#define HEIGHT  300

static GtkAccelGroup *pc_accel_group;
static GtkWidget *arrangement_box;

/*--------------------------------------------------------------------*/
/*                   Display section                                  */
/*--------------------------------------------------------------------*/

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,     NULL,     0,                    "<Branch>" },
  { "/FFile/Print",    "",       display_print_cb, 0, "<Item>" },
  { "/FFile/sep",      NULL,     NULL,     0, "<Separator>" },
  { "/FFile/Close",    "",       display_close_cb, 0, "<Item>" },
};
/* The rest of the menus will be appended once the menubar is created */


static void
parcoords_display_menus_make (displayd *display, 
  GtkAccelGroup *accel_group, GtkSignalFunc func, GtkWidget *mbar)
{
  GtkWidget *options_menu, *link_menu;
  GtkWidget *submenu;

/*
 * Display options menu
*/
  submenu = submenu_make ("_Display", 'D', accel_group);
  options_menu = gtk_menu_new ();

  CreateMenuCheck (display, options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on);
  CreateMenuCheck (display, options_menu, "Show lines",
    func, GINT_TO_POINTER (DOPT_SEGS), on);
  CreateMenuCheck (display, options_menu, "Show missings",
    func, GINT_TO_POINTER (DOPT_MISSINGS), on);
  CreateMenuCheck (display, options_menu, "Show gridlines",
    func, GINT_TO_POINTER (DOPT_GRIDLINES), off);
  CreateMenuCheck (display, options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), on);

  /* Add a separator */
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL);

  CreateMenuCheck (display, options_menu, "Double buffer",
    func, GINT_TO_POINTER (DOPT_BUFFER), on);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), options_menu);
  submenu_append (submenu, mbar);

  gtk_widget_show (submenu);

/*
 * Link menu
*/
  submenu = submenu_make ("_Link", 'L', accel_group);
  link_menu = gtk_menu_new ();

  CreateMenuCheck (display, link_menu, "Link to other plots",
    func, GINT_TO_POINTER (DOPT_LINK), on);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), link_menu);
  submenu_append (submenu, mbar);
  gtk_widget_show (submenu);
}


void
parcoords_reset_arrangement (displayd *display, gint arrangement) {
  GList *l;
  GtkWidget *frame, *w;
  splotd *sp;
  gint height, width;

  if (display->cpanel.parcoords_arrangement == arrangement)
    return;

  l = display->splots;
  while (l) {
    w = ((splotd *) l->data)->da;
    gtk_widget_ref (w);
    gtk_container_remove (GTK_CONTAINER (arrangement_box), w);
    l = l->next ;
  }

  frame = arrangement_box->parent;
  gtk_widget_destroy (arrangement_box);

  if (arrangement == ARRANGE_ROW)
    arrangement_box = gtk_hbox_new (true, 0);
  else
    arrangement_box = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), arrangement_box);

  display->p1d_orientation = (arrangement == ARRANGE_ROW) ? VERTICAL :
                                                            HORIZONTAL;

  height = (arrangement == ARRANGE_ROW) ? HEIGHT : WIDTH;
  width = (arrangement == ARRANGE_ROW) ? WIDTH : HEIGHT;
  l = display->splots;
  while (l) {
    sp = (splotd *) l->data;
    gtk_widget_set_usize (sp->da, width, height);
    gtk_box_pack_start (GTK_BOX (arrangement_box), sp->da, true, true, 0);
    l = l->next ;
  }

  gtk_widget_show_all (arrangement_box);

  varpanel_refresh ();
}


#define MAXNPCPLOTS 5
displayd *
parcoords_new (gboolean missing_p) {
  GtkWidget *vbox, *frame;
  GtkWidget *mbar;
  gint i;
  splotd *sp;
  gint nplots = MIN (xg.ncols, MAXNPCPLOTS);

  displayd *display = (displayd *) g_malloc (sizeof (displayd));

  display->displaytype = parcoords;
  display->missing_p = missing_p;
  /* create a row of vertical plots by default */
  display->p1d_orientation = VERTICAL;

  display->points_show_p = true;
  display->segments_show_p = true;
  display->missings_show_p = true;
  display->gridlines_show_p = false;
  display->axes_show_p = true;
  display->double_buffer_p = true;
  display->link_p = true;

  parcoords_cpanel_init (&display->cpanel);

  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (display->window),
                       "displayd",
                       (gpointer) display);
  gtk_window_set_policy (GTK_WINDOW (display->window), true, true, false);
  gtk_container_set_border_width (GTK_CONTAINER (display->window), 3);
  gtk_signal_connect (GTK_OBJECT (display->window), "delete_event",
                      GTK_SIGNAL_FUNC (display_delete_cb), (gpointer) display);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  pc_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 pc_accel_group, display->window, &mbar, (gpointer) display);

  /*
   * After creating the menubar, and populating the file menu,
   * add the Display Options and Link menus another way
  */
  parcoords_display_menus_make (display, pc_accel_group,
    display_options_cb, mbar);
  gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);


/*
 * splots in a box in a frame -- either a vbox or an hbox.
*/
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 1);

/*
 * this is the box that would have to change from horizontal to vertical
 * when the plot arrangement changes
*/
  arrangement_box = gtk_hbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), arrangement_box);

  display->splots = NULL;

  for (i=0; i<nplots; i++) {
    sp = splot_new (display, WIDTH, HEIGHT);
    /*
     * instead of maintaining a list, each plot knows its variable
    */
    sp->p1dvar = i; 
    display->splots = g_list_append (display->splots, (gpointer) sp);
    gtk_box_pack_start (GTK_BOX (arrangement_box), sp->da, true, true, 0);
  }

  gtk_widget_show_all (display->window);

  return display;
}

gboolean
parcoords_varsel (cpaneld *cpanel, splotd *sp,
  gint jvar, gint *jvar_prev, gboolean alt_mod)
{
  gboolean redraw = true;
  gint nplots = g_list_length (current_display->splots);
  gint k, width, height;
  gint jvar_indx, new_indx;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *box, *w;
  gfloat ratio = 1.0;

  /* The index of current_splot */
  gint sp_indx = g_list_index (current_display->splots, sp);

  /* If jvar is one of the plotted variables, its corresponding plot */
  splotd *jvar_sp = NULL;

  k = 0;
  l = current_display->splots;
  while (l) {
    s = (splotd *) l->data;
    if (s->p1dvar == jvar) {
      jvar_sp = s;
      jvar_indx = k;
      break;
    }
    l = l->next;
    k++;
  }

  gtk_window_set_policy (GTK_WINDOW (current_display->window),
        false, false, false);

  splot_get_dimensions (sp, &width, &height);


/*
 * If the alt key is pressed and jvar is plotted, delete it.
 * We don't care what the current splot, and we don't use the
 * argument sp.
*/
  if (alt_mod == true) {
    if (jvar_sp != NULL && nplots > 1) {
      /*-- Delete the plot from the list, and destroy it. --*/
      current_display->splots = g_list_remove (current_display->splots,
                                               (gpointer) jvar_sp);

      /*-- keep the window from shrinking by growing all plots --*/
      ratio = (gfloat) nplots / (gfloat) (nplots-1);
      if (cpanel->parcoords_arrangement == ARRANGE_ROW)
        width = (gint) (ratio * (gfloat) width);
      else
        height = (gint) (ratio * (gfloat) height);

      l = current_display->splots;
      while (l) {
        w = ((splotd *) l->data)->da;
        gtk_widget_ref (w);
        /*-- shrink each plot --*/
        gtk_widget_set_usize (w, -1, -1);
        gtk_widget_set_usize (w, width, height);
        /* */
        l = l->next ;
      }
      /* */

      /*
       * If the plot being removed is the current plot, reset
       * current_splot.
      */
      if (jvar_sp == current_splot) {
        sp_event_handlers_toggle (sp, off);

        new_indx = (jvar_indx == 0) ? 0 : MIN (nplots-1, jvar_indx);
        current_splot = (splotd *)
          g_list_nth_data (current_display->splots, new_indx);
        /* just for insurance, to handle the unforeseen */
        if (current_splot == NULL) 
          current_splot = (splotd *)
            g_list_nth_data (current_display->splots, 0);
      }

      splot_free (jvar_sp, current_display);

      nplots--;
/*      redraw = false;*/
    }

  } else {

    if (cpanel->parcoords_selection_mode == VAR_REPLACE) {

      *jvar_prev = sp->p1dvar;
      sp->p1dvar = jvar;
      redraw = true;

    } else {

      /*-- keep the window from growing by shrinking all plots --*/
      ratio = (gfloat) nplots / (gfloat) (nplots+1);
      if (cpanel->parcoords_arrangement == ARRANGE_ROW)
        width = (gint) (ratio * (gfloat) width);
      else
        height = (gint) (ratio * (gfloat) height);
      /* */

      sp_new = splot_new (current_display, width, height);
      sp_new->p1dvar = jvar; 

      if (cpanel->parcoords_selection_mode == VAR_INSERT)
        current_display->splots = g_list_insert (current_display->splots,
          (gpointer) sp_new, sp_indx);
      else if (cpanel->parcoords_selection_mode == VAR_APPEND)
        current_display->splots = g_list_insert (current_display->splots,
          (gpointer) sp_new, MIN (sp_indx+1, nplots));

      box = (sp->da)->parent;
      gtk_box_pack_end (GTK_BOX (box), sp_new->da, false, false, 0);
      gtk_widget_show (sp_new->da);

      l = current_display->splots;
      while (l) {
        w = ((splotd *) l->data)->da;
        gtk_widget_ref (w);

        /* shrink each plot */
        gtk_widget_set_usize (w, -1, -1);
        gtk_widget_set_usize (w, width, height);
        /* */

        gtk_container_remove (GTK_CONTAINER (box), w);
        gtk_box_pack_start (GTK_BOX (box), w, true, true, 0);
        l = l->next ;
      }

      current_splot = sp_new;
      redraw = true;
    }
  }

  gtk_window_set_policy (GTK_WINDOW (current_display->window),
    true, true, false);

  return redraw;
}

/*--------------------------------------------------------------------*/
/*               The whiskers, ie the parallel coordinate lines       */  
/*--------------------------------------------------------------------*/

static void
sp_rewhisker (splotd *sp_prev, splotd *sp, splotd *sp_next) {
  gint i, k, m;
  displayd *display = (displayd *) sp->displayptr;
  gboolean draw_whisker;

  for (k=0; k<xg.nrows_in_plot; k++) {
    i = xg.rows_in_plot[k];
    m = 2*i;

    /*-- if it's the leftmost plot, don'r draw the left whisker --*/
    if (sp_prev == NULL)
      draw_whisker = false;
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    else if (!display->missings_show_p &&
          xg.nmissing > 0 &&
          (xg.missing.data[i][sp->p1dvar] ||
           xg.missing.data[i][sp_prev->p1dvar]))
    {
      draw_whisker = false;
    }
    else
      draw_whisker = true;

    /* --- left whisker --- */
    if (draw_whisker) {
      sp->whiskers[m].x1 = 0;
      sp->whiskers[m].y1 = (sp_prev->screen[i].y + sp->screen[i].y) / 2;
      sp->whiskers[m].x2 = sp->screen[i].x;
      sp->whiskers[m].y2 = sp->screen[i].y;
    } else {
      sp->whiskers[m].x1 = sp->screen[i].x;
      sp->whiskers[m].y1 = sp->screen[i].y;
      sp->whiskers[m].x2 = sp->screen[i].x;
      sp->whiskers[m].y2 = sp->screen[i].y;
    }

    m++;

    /*-- if it's the rightmost plot, don'r draw the right whisker --*/
    if (sp_next == NULL)
      draw_whisker = false;
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    else if (!display->missings_show_p && xg.nmissing > 0 &&
            (MISSING_P(i,sp->p1dvar) || MISSING_P(i,sp_next->p1dvar)))
    {
      draw_whisker = false;
    }
    else
      draw_whisker = true;

    /* --- right whisker --- */
    if (draw_whisker) {
      sp->whiskers[m].x1 = sp->screen[i].x;
      sp->whiskers[m].y1 = sp->screen[i].y;
      sp->whiskers[m].x2 = sp->max.x;
      sp->whiskers[m].y2 = (sp->screen[i].y + sp_next->screen[i].y) / 2;
    } else {
      sp->whiskers[m].x1 = sp->screen[i].x;
      sp->whiskers[m].y1 = sp->screen[i].y;
      sp->whiskers[m].x2 = sp->screen[i].x;
      sp->whiskers[m].y2 = sp->screen[i].y;
    }
  }
}

/*-- set the positions of the whiskers for sp and prev_sp --*/
void
sp_whiskers_make (splotd *sp, displayd *display) {
  GList *splist;
  splotd *splot;
  splotd *sp_prev = NULL, *sp_prev_prev = NULL, *sp_next = NULL;

  for (splist = display->splots; splist; splist = splist->next) {
    splot = (splotd *) splist->data;

    if (splot == sp) {
      sp_next = (splist->next == NULL) ? NULL : (splotd *) splist->next->data;
      sp_prev = (splist->prev == NULL) ? NULL : (splotd *) splist->prev->data;
      if (sp_prev != NULL)
        sp_prev_prev = (splist->prev->prev == NULL) ? NULL :
                        (splotd *) splist->prev->prev->data;
    }
  }

  if (sp_prev != NULL)
    sp_rewhisker (sp_prev_prev, sp_prev, sp);

  if (sp_next == NULL)
    sp_rewhisker (sp_prev, sp, NULL);
}
