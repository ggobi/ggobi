/* timeplot.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* initial plot sizes */
#define WIDTH   150
#define HEIGHT  100

/*--------------------------------------------------------------------*/
/*                   Options section                                  */
/*--------------------------------------------------------------------*/

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,     NULL,     0,                    "<Branch>" },
  { "/FFile/Print",    "",       (GtkItemFactoryCallback) display_print_cb, 0, "<Item>" },
  { "/FFile/sep",      NULL,     NULL,     0, "<Separator>" },
  { "/FFile/Close",    "",       (GtkItemFactoryCallback) display_close_cb, 0, "<Item>" },
};
/* The rest of the menus will be appended once the menubar is created */

static void
tsplot_display_menus_make (displayd *display, 
  GtkAccelGroup *accel_group, GtkSignalFunc func, GtkWidget *mbar, ggobid *gg)
{
  GtkWidget *options_menu, *submenu, *item;

/*
 * Display options menu
*/
  submenu = submenu_make ("_Options", 'D', accel_group);
  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu, "Show line segments",
    func, GINT_TO_POINTER (DOPT_WHISKERS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
/*
  item = CreateMenuCheck (options_menu, "Show missings",
    func, GINT_TO_POINTER (DOPT_MISSINGS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
*/

  /* Add a separator */
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);

  item = CreateMenuCheck (options_menu, "Double buffer",
    func, GINT_TO_POINTER (DOPT_BUFFER), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), options_menu);
  submenu_append (submenu, mbar);

  gtk_widget_show (submenu);
}


void
tsplot_reset_arrangement (displayd *display, gint arrangement, ggobid *gg) {
  GList *l;
  GtkWidget *frame, *w;
  splotd *sp;
  gint height, width;

  if (display->cpanel.tsplot_arrangement == arrangement)
    return;

  for (l=display->splots; l; l=l->next) {
    w = ((splotd *) l->data)->da;
    gtk_widget_ref (w);
    gtk_container_remove (GTK_CONTAINER (gg->tsplot.arrangement_box), w);
  }

  frame = gg->tsplot.arrangement_box->parent;
  gtk_widget_destroy (gg->tsplot.arrangement_box);

/*    if (arrangement == ARRANGE_ROW) */
/*      gg->tsplot.arrangement_box = gtk_hbox_new (true, 0); */
/*    else */
    gg->tsplot.arrangement_box = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), gg->tsplot.arrangement_box);

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
    gtk_box_pack_start (GTK_BOX (gg->tsplot.arrangement_box),
                        sp->da, true, true, 0);
    gtk_widget_unref (sp->da);
 }

 /*-- position the display toward the lower left of the main window --*/
  display_set_position (display, gg);

  gtk_widget_show_all (gg->tsplot.arrangement_box);

  display_tailpipe (display, FULL, gg);

  varpanel_refresh (gg);
}


#define MAXNTSPLOTS 6
displayd *
tsplot_new (gboolean missing_p, gint nvars, gint *vars,
  datad *d, ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  GtkWidget *mbar;
  gint i;
  splotd *sp;
  gint nplots;
  displayd *display;

  display = display_alloc_init (tsplot, missing_p, d, gg);
  if (nvars == 0) {
    nplots = MIN ((d->ncols-1), MAXNTSPLOTS);
    for (i=1; i<nplots; i++)
      vars[i] = i;
  } else {
    nplots = nvars;
  }

  tsplot_cpanel_init (&display->cpanel, gg);

  display_window_init (display, 3, gg);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  gg->tsplot.accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
    gg->tsplot.accel_group, display->window, &mbar, (gpointer) display);

  /*
   * After creating the menubar, and populating the file menu,
   * add the Display Options and Link menus another way
  */
  tsplot_display_menus_make (display, gg->tsplot.accel_group,
                            (GtkSignalFunc) display_options_cb, mbar, gg);
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
  gg->tsplot.arrangement_box = gtk_vbox_new (true, 0);
  gtk_container_add (GTK_CONTAINER (frame), gg->tsplot.arrangement_box);

  display->splots = NULL;

  for (i=1; i<nplots; i++) {
    sp = splot_new (display, 2.5*WIDTH, HEIGHT, gg);
    sp->xyvars.y = vars[i];
    sp->xyvars.x = 0;

/*
    if (sub_plots == NULL) {
      sp = splot_new (display, WIDTH, HEIGHT, gg);
      sp->xyvars.y = i; 
    } else
       sp = sub_plots[i];
*/

    display->splots = g_list_append (display->splots, (gpointer) sp);
    gtk_box_pack_start (GTK_BOX (gg->tsplot.arrangement_box),
      sp->da, true, true, 0);
  }

  gtk_widget_show_all (display->window);

  return display;
}

static gboolean
tsplot_var_selected (gint jvar, displayd *display)
{
  gboolean selected = false;
  splotd *s;
  GList *l = display->splots;
  while (l) {
    s = (splotd *) l->data;
    if (s->xyvars.y == jvar || s->xyvars.x == jvar ) {
      selected = true;
      break;
    }
    l = l->next ;
  }
  return selected;
}

gboolean
tsplot_varsel (cpaneld *cpanel, splotd *sp, gint button,
  gint jvar, gint *jvar_prev, ggobid *gg)
{
  gboolean redraw = true;
  gint nplots = g_list_length (gg->current_display->splots);
  gint k, width, height;
  gint jvar_indx, new_indx;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *box, *w;
  gfloat ratio = 1.0;
  displayd *display=gg->current_display;

  /* The index of gg.current_splot */
  gint sp_indx = g_list_index (gg->current_display->splots, sp);

  gtk_window_set_policy (GTK_WINDOW (gg->current_display->window),
        false, false, false);

  splot_get_dimensions (sp, &width, &height);

  /*
   *  if left button click, the x variable no matter what
   *  selection_mode prevails.
  */
  if (button == 1) {
    l = display->splots;
    s = (splotd *) l->data;
    if (s->xyvars.x == jvar) redraw=false;  /*-- this is already the x var --*/
    else {
      while (l) {
        s = (splotd *) l->data;
        s->xyvars.x = jvar;
        l = l->next;
      }
    }

  } else if (button == 2 || button == 3) {

    if (tsplot_var_selected (jvar, display)) {

      /* If jvar is one of the plotted variables, its corresponding plot */
      splotd *jvar_sp = NULL;

      k = 0;
      l = display->splots;
      while (l) {
        s = (splotd *) l->data;
        if (s->xyvars.y == jvar) {
          jvar_sp = s;
          jvar_indx = k;
          break;
        }
        l = l->next;
        k++;
      }

      if (jvar_sp != NULL && nplots > 1 && 
         cpanel->tsplot_selection_mode == VAR_DELETE) {
        /*-- Delete the plot from the list, and destroy it. --*/
        display->splots = g_list_remove (display->splots, (gpointer) jvar_sp);

        /*-- keep the window from shrinking by growing all plots --*/
        ratio = (gfloat) nplots / (gfloat) (nplots-1);
        height = (gint) (ratio * (gfloat) height);

        l = display->splots;
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
            g_list_nth_data (display->splots, new_indx);
          /* just for insurance, to handle the unforeseen */
          if (gg->current_splot == NULL) 
            gg->current_splot = (splotd *) g_list_nth_data (display->splots, 0);
          display->current_splot = gg->current_splot;

          /*-- dfs, keeping event handling in sync --*/
          splot_set_current (gg->current_splot, on, gg);
        }

        splot_free (jvar_sp, display, gg);

        nplots--;
      }

    } else if(cpanel->tsplot_selection_mode != VAR_DELETE) {

      if (cpanel->tsplot_selection_mode == VAR_REPLACE) {
        *jvar_prev = sp->xyvars.y;
        sp->xyvars.y = jvar;
        redraw = true;

      } else {

        /*-- prepare to reset the current plot --*/
        sp_event_handlers_toggle (sp, off);

        /*-- keep the window from growing by shrinking all plots --*/
        ratio = (gfloat) nplots / (gfloat) (nplots+1);
        /*       if (cpanel->parcoords_arrangement == ARRANGE_ROW) */
        /*         width = (gint) (ratio * (gfloat) width); */
        /*       else */
        height = (gint) (ratio * (gfloat) height);
        /* */
        l = display->splots;
        sp_new = splot_new (display, width, height, gg);
        sp_new->xyvars.y = jvar;

        if (cpanel->tsplot_selection_mode == VAR_INSERT)
          display->splots = g_list_insert (display->splots,
                                           (gpointer) sp_new, sp_indx);
        else if (cpanel->tsplot_selection_mode == VAR_APPEND)
          display->splots = g_list_append (display->splots,
            (gpointer) sp_new);

        box = (sp->da)->parent;
        gtk_box_pack_end (GTK_BOX (box), sp_new->da, false, false, 0);
        gtk_widget_show (sp_new->da);

        while (l) {
          w = ((splotd *) l->data)->da;
          gtk_widget_ref (w);

          /* shrink each plot */
          gtk_widget_set_usize (w, -1, -1);
          gtk_widget_set_usize (w, width, height);
          /* */

          gtk_container_remove (GTK_CONTAINER (box), w);
          gtk_box_pack_start (GTK_BOX (box), w, true, true, 0);
          gtk_widget_unref (w);  /*-- decrease the ref_count by 1 --*/
          l = l->next ;
        }

        gg->current_splot = sp->displayptr->current_splot = sp_new;
        sp_event_handlers_toggle (sp_new, on);
        redraw = true;
      }
    }
  }
  gtk_window_set_policy (GTK_WINDOW (display->window),
    true, true, false);

  return redraw;
}

/*--------------------------------------------------------------------*/
/*               The whiskers for timeseries lines                    */  
/*--------------------------------------------------------------------*/

static void
tsplot_rewhisker (splotd *sp, ggobid *gg) {
  gint i, k, n;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gboolean draw_whisker;

  for (k=0; k<(d->nrows_in_plot-1); k++) {
    i = d->rows_in_plot[k];
    n = d->rows_in_plot[k+1];
    
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    if (!display->options.missings_show_p &&
          d->nmissing > 0 &&
          (d->missing.vals[i][sp->xyvars.x] || 
           d->missing.vals[i][sp->xyvars.y] ||
           d->missing.vals[n][sp->xyvars.x] || 
           d->missing.vals[n][sp->xyvars.y]) &&
           (sp->screen[i].x > sp->screen[n].x))/* to keep time going
                                                  forwards */
    {
      draw_whisker = false;
    }
    else
      draw_whisker = true;
    /* --- all whiskers --- */
    if (draw_whisker) {
      sp->whiskers[i].x1 = sp->screen[i].x;
      sp->whiskers[i].y1 = sp->screen[i].y;
      sp->whiskers[i].x2 = sp->screen[n].x;
      sp->whiskers[i].y2 = sp->screen[n].y;
    }      
  }
}


/*-- set the positions of the whiskers for sp and prev_sp --*/
void
tsplot_whiskers_make (splotd *sp, displayd *display, ggobid *gg) {
  GList *splist;
  splotd *splot;
  splotd *sp_next = (splotd *) NULL;

  for (splist = display->splots; splist; splist = splist->next) {
    splot = (splotd *) splist->data;
    if (splot == sp) {
      sp_next = (splist->next == NULL) ? NULL : (splotd *) splist->next->data;
    }
  }

  tsplot_rewhisker (sp, gg);
}
