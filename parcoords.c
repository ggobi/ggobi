/* parcoords.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH   150
#define HEIGHT  300


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
  GtkAccelGroup *accel_group, GtkSignalFunc func, GtkWidget *mbar, ggobid *gg)
{
  GtkWidget *options_menu, *link_menu;
  GtkWidget *submenu;

/*
 * Display options menu
*/
  submenu = submenu_make ("_Display", 'D', accel_group);
  options_menu = gtk_menu_new ();

  CreateMenuCheck (display, options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  CreateMenuCheck (display, options_menu, "Show lines",
    func, GINT_TO_POINTER (DOPT_SEGS), on, gg);
  CreateMenuCheck (display, options_menu, "Show missings",
    func, GINT_TO_POINTER (DOPT_MISSINGS), on, gg);
  CreateMenuCheck (display, options_menu, "Show gridlines",
    func, GINT_TO_POINTER (DOPT_GRIDLINES), off, gg);
  CreateMenuCheck (display, options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), on, gg);

  /* Add a separator */
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  CreateMenuCheck (display, options_menu, "Double buffer",
    func, GINT_TO_POINTER (DOPT_BUFFER), on, gg);

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


void
parcoords_reset_arrangement (displayd *display, gint arrangement, ggobid *gg) {
  GList *l;
  GtkWidget *frame, *w;
  splotd *sp;
  gint height, width;

  if (display->cpanel.parcoords_arrangement == arrangement)
    return;

  for (l=display->splots; l; l=l->next) {
    w = ((splotd *) l->data)->da;
    gtk_widget_ref (w);
    gtk_container_remove (GTK_CONTAINER (gg->parcoords.arrangement_box), w);
  }

  frame = gg->parcoords.arrangement_box->parent;
  gtk_widget_destroy (gg->parcoords.arrangement_box);

  if (arrangement == ARRANGE_ROW)
    gg->parcoords.arrangement_box = gtk_hbox_new (true, 0);
  else
    gg->parcoords.arrangement_box = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), gg->parcoords.arrangement_box);

  display->p1d_orientation = (arrangement == ARRANGE_ROW) ? VERTICAL :
                                                            HORIZONTAL;

  height = (arrangement == ARRANGE_ROW) ? HEIGHT : WIDTH;
  width = (arrangement == ARRANGE_ROW) ? WIDTH : HEIGHT;
/*
  l = display->splots;
  while (l) {
    sp = (splotd *) l->data;
    gtk_widget_set_usize (sp->da, width, height);
    gtk_box_pack_start (GTK_BOX (arrangement_box), sp->da, true, true, 0);
    l = l->next ;
  }
*/
  for (l=display->splots; l; l=l->next) {
    sp = (splotd *) l->data;
    gtk_widget_set_usize (sp->da, width, height);
    gtk_box_pack_start (GTK_BOX (gg->parcoords.arrangement_box),
                        sp->da, true, true, 0);
  }

  gtk_widget_show_all (gg->parcoords.arrangement_box);

  display_tailpipe (display, gg);

  vartable_refresh (display->d, gg);
}


#define MAXNPCPLOTS 5
displayd *
parcoords_new (gboolean missing_p, gint nvars, gint *vars,
  datad *d, ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  GtkWidget *mbar;
  gint i;
  splotd *sp;
  gint nplots;
  displayd *display;

  display = display_alloc_init (parcoords, missing_p, d, gg);
  if (nvars == 0) {
    nplots = MIN (d->ncols, MAXNPCPLOTS);
    for (i=0; i<nplots; i++)
      vars[i] = i;
  } else {
    nplots = nvars;
  }

  parcoords_cpanel_init (&display->cpanel, gg);

  display_window_init(display, 3, gg);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  gg->parcoords.pc_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
    gg->parcoords.pc_accel_group, display->window, &mbar, (gpointer) display);

  /*
   * After creating the menubar, and populating the file menu,
   * add the Display Options and Link menus another way
  */
  parcoords_display_menus_make (display, gg->parcoords.pc_accel_group,
                                 display_options_cb, mbar, gg);
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
  gg->parcoords.arrangement_box = gtk_hbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), gg->parcoords.arrangement_box);

  display->splots = NULL;

  for (i=0; i<nplots; i++) {
    sp = splot_new (display, WIDTH, HEIGHT, gg);
    sp->p1dvar = vars[i]; 

/*
    if (sub_plots == NULL) {
      sp = splot_new (display, WIDTH, HEIGHT, gg);
      sp->p1dvar = i; 
    } else
       sp = sub_plots[i];
*/

    display->splots = g_list_append (display->splots, (gpointer) sp);
    gtk_box_pack_start (GTK_BOX (gg->parcoords.arrangement_box),
      sp->da, true, true, 0);
  }

  gtk_widget_show_all (display->window);

  return display;
}

gboolean
parcoords_varsel (cpaneld *cpanel, splotd *sp,
  gint jvar, gint *jvar_prev, gboolean alt_mod, ggobid *gg)
{
  gboolean redraw = true;
  gint nplots = g_list_length (gg->current_display->splots);
  gint k, width, height;
  gint jvar_indx, new_indx;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *box, *w;
  gfloat ratio = 1.0;

  /* The index of gg.current_splot */
  gint sp_indx = g_list_index (gg->current_display->splots, sp);

  /* If jvar is one of the plotted variables, its corresponding plot */
  splotd *jvar_sp = NULL;

  k = 0;
  l = gg->current_display->splots;
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

  gtk_window_set_policy (GTK_WINDOW (gg->current_display->window),
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
      gg->current_display->splots = g_list_remove (gg->current_display->splots,
                                               (gpointer) jvar_sp);

      /*-- keep the window from shrinking by growing all plots --*/
      ratio = (gfloat) nplots / (gfloat) (nplots-1);
      if (cpanel->parcoords_arrangement == ARRANGE_ROW)
        width = (gint) (ratio * (gfloat) width);
      else
        height = (gint) (ratio * (gfloat) height);

      l = gg->current_display->splots;
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
       * gg->current_splot.
      */
      if (jvar_sp == gg->current_splot) {
        sp_event_handlers_toggle (sp, off);

        new_indx = (jvar_indx == 0) ? 0 : MIN (nplots-1, jvar_indx);
        gg->current_splot = (splotd *)
          g_list_nth_data (gg->current_display->splots, new_indx);
        /* just for insurance, to handle the unforeseen */
        if (gg->current_splot == NULL) 
          gg->current_splot = (splotd *)
            g_list_nth_data (gg->current_display->splots, 0);
      }

      splot_free (jvar_sp, gg->current_display, gg);

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

      sp_new = splot_new (gg->current_display, width, height, gg);
      sp_new->p1dvar = jvar; 

      if (cpanel->parcoords_selection_mode == VAR_INSERT)
        gg->current_display->splots = g_list_insert (gg->current_display->splots,
          (gpointer) sp_new, sp_indx);
      else if (cpanel->parcoords_selection_mode == VAR_APPEND)
        gg->current_display->splots = g_list_insert (gg->current_display->splots,
          (gpointer) sp_new, MIN (sp_indx+1, nplots));

      box = (sp->da)->parent;
      gtk_box_pack_end (GTK_BOX (box), sp_new->da, false, false, 0);
      gtk_widget_show (sp_new->da);

      l = gg->current_display->splots;
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

      gg->current_splot = sp_new;
      redraw = true;
    }
  }

  gtk_window_set_policy (GTK_WINDOW (gg->current_display->window),
    true, true, false);

  return redraw;
}

/*--------------------------------------------------------------------*/
/*               The whiskers, ie the parallel coordinate lines       */  
/*--------------------------------------------------------------------*/

static void
sp_rewhisker (splotd *sp_prev, splotd *sp, splotd *sp_next, ggobid *gg) {
  gint i, k, m;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = (cpaneld *) &display->cpanel;
  datad *d = display->d;
  gboolean draw_whisker;

  for (k=0; k<d->nrows_in_plot; k++) {
    i = d->rows_in_plot[k];
    m = 2*i;

    /*-- if it's the leftmost plot, don'r draw the left whisker --*/
    if (sp_prev == NULL)
      draw_whisker = false;
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    else if (!display->options.missings_show_p &&
          d->nmissing > 0 &&
          (d->missing.vals[i][sp->p1dvar] ||
           d->missing.vals[i][sp_prev->p1dvar]))
    {
      draw_whisker = false;
    }
    else
      draw_whisker = true;

    /* --- left (or top) whisker  --- */
    if (cpanel->parcoords_arrangement == ARRANGE_ROW) {

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

    } else {  /* ARRANGE_COLUMN */
      if (draw_whisker) {
        sp->whiskers[m].x1 = (sp_prev->screen[i].x + sp->screen[i].x) / 2;
        sp->whiskers[m].y1 = 0;
        sp->whiskers[m].x2 = sp->screen[i].x;
        sp->whiskers[m].y2 = sp->screen[i].y;
      } else {
        sp->whiskers[m].x1 = sp->screen[i].x;
        sp->whiskers[m].y1 = sp->screen[i].y;
        sp->whiskers[m].x2 = sp->screen[i].x;
        sp->whiskers[m].y2 = sp->screen[i].y;
      }
    }

    m++;

    /*-- if it's the rightmost plot, don'r draw the right whisker --*/
    if (sp_next == NULL)
      draw_whisker = false;
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    else if (!display->options.missings_show_p && d->nmissing > 0 &&
            (MISSING_P(i,sp->p1dvar) || MISSING_P(i,sp_next->p1dvar)))
    {
      draw_whisker = false;
    }
    else
      draw_whisker = true;

    /* --- right (or bottom) whisker --- */
    if (cpanel->parcoords_arrangement == ARRANGE_ROW) {

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
    } else {  /* ARRANGE_COLUMN */
      if (draw_whisker) {
        sp->whiskers[m].x1 = sp->screen[i].x;
        sp->whiskers[m].y1 = sp->screen[i].y;
        sp->whiskers[m].x2 = (sp->screen[i].x + sp_next->screen[i].x) / 2;
        sp->whiskers[m].y2 = sp->max.y;
      } else {
        sp->whiskers[m].x1 = sp->screen[i].x;
        sp->whiskers[m].y1 = sp->screen[i].y;
        sp->whiskers[m].x2 = sp->screen[i].x;
        sp->whiskers[m].y2 = sp->screen[i].y;
      }
    }
  }
}

/*-- set the positions of the whiskers for sp and prev_sp --*/
void
sp_whiskers_make (splotd *sp, displayd *display, ggobid *gg) {
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
    sp_rewhisker (sp_prev_prev, sp_prev, sp, gg);

  if (sp_next == NULL)
    sp_rewhisker (sp_prev, sp, NULL, gg);
}
