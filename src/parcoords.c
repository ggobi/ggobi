/*-- parcoords.c --*/
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#include "parcoordsClass.h"

#define WIDTH   150
#define HEIGHT  300


/*--------------------------------------------------------------------*/
/*                   Options section                                  */
/*--------------------------------------------------------------------*/

static const gchar* parcoords_ui =
"<ui>"
"	<menubar>"
"		<menu action='Options'>"
"			<menuitem action='ShowPoints'/>"
"			<menuitem action='ShowLines'/>"
"		</menu>"
"	</menubar>"
"</ui>";

#if 0
void
parcoords_display_menus_make (displayd *display, 
  GtkAccelGroup *accel_group, GtkSignalFunc func, GtkWidget *mbar, ggobid *gg)
{
  GtkWidget *options_menu, *submenu, *item;

  /*-- Options menu --*/
  submenu = submenu_make ("_Options", 'O', accel_group);
  /*-- add a tooltip --*/
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), submenu,
    "Options menu for this display", NULL);

  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu, "Show line segments",
    func, GINT_TO_POINTER (DOPT_WHISKERS), on, gg);
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);

  /* Add a separator */
/*
  CreateMenuItem (options_menu, NULL, "", "", NULL, NULL, NULL, NULL, gg);
*/

/*
  item = CreateMenuCheck (options_menu, "Double buffer",
    func, GINT_TO_POINTER (DOPT_BUFFER), on, gg);
  g_object_set_data(G_OBJECT (item), "display", (gpointer) display);
*/

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), options_menu);
  submenu_append (submenu, mbar);

  gtk_widget_show (submenu);
}

#endif

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
    //gtk_widget_set_usize (sp->da, width, height);
    gtk_box_pack_start (GTK_BOX (gg->parcoords.arrangement_box),
                        sp->da, true, true, 0);
    gtk_widget_unref (sp->da);  /*-- keep the ref_count appropriate --*/
  }

  /*-- position the display toward the lower left of the main window --*/
  display_set_position (GGOBI_WINDOW_DISPLAY(display), gg);

  gtk_widget_show_all (gg->parcoords.arrangement_box);

  display_tailpipe (display, FULL, gg);

  varpanel_refresh (display, gg);
}

#define MAXNPCPLOTS 5


displayd *
parcoords_new_with_vars(gboolean missing_p, gint nvars, gint *vars,
	       datad *d, ggobid *gg) 
{
	return(parcoords_new(NULL, missing_p, nvars, vars, d, gg));
}

displayd *
parcoords_new (displayd *display, gboolean missing_p, gint nvars, gint *vars,
	       datad *d, ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  gint i;
  splotd *sp;
  gint nplots;
  gint arrangement = ARRANGE_ROW;  /*-- default initial orientation --*/
  gint width, screenwidth;
  gint height, screenheight;

  if(!display) 
    display = g_object_new(GGOBI_TYPE_PAR_COORDS_DISPLAY, NULL);

  display_set_values(display, d, gg);

  if (nvars == 0) {
    nplots = MIN (d->ncols, sessionOptions->info->numParCoordsVars);
    if(nplots < 0) {
      nplots = d->ncols;
    }

    /* Initialize using the plotted variables in the current display,
       if appropriate */
    if (gg->current_display != NULL && gg->current_display != display && 
        gg->current_display->d == d && 
        GGOBI_IS_EXTENDED_DISPLAY(gg->current_display))
    {
      gint j, k, nplotted_vars;
      gint *plotted_vars = (gint *) g_malloc(d->ncols * sizeof(gint));
      displayd *dsp = gg->current_display;

      nplotted_vars = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dsp)->plotted_vars_get(dsp, plotted_vars, d, gg);
     
      nplots = MAX (nplots, nplotted_vars);
      for (j=0; j<nplotted_vars; j++)
        vars[j] = plotted_vars[j];
      j = nplotted_vars;
      for (k=0; k<d->ncols; k++) {
        if (!in_vector(k, plotted_vars, nplotted_vars)) {
          vars[j] = k;
          j++;
          if (j == nplots)
            break;
        }
      }
      g_free (plotted_vars);

    } else {

      for (i=0; i<nplots; i++)
        vars[i] = i;
    }
  } else {
    nplots = nvars;  /* and we assume the argument vars is populated */
  }

  parcoords_cpanel_init (&display->cpanel, gg);
  
  /*-- make sure the initial plot doesn't spill outside the screen --*/
  /*-- this should know about the display borders, but it doesn't --*/
  /*-- at the moment, the arrangement is forced to be ARRANGE_ROW --*/
  width = WIDTH;
  height = HEIGHT;
  if (arrangement == ARRANGE_ROW) {
    screenwidth = gdk_screen_width();
    while (nplots * width > screenwidth) {
      width -= 10;
    }
  } else {
    screenheight = gdk_screen_height();
    while (nplots * height > screenheight) {
      height -= 10;
    }
  }
  
  if(GGOBI_IS_WINDOW_DISPLAY(display) && GGOBI_WINDOW_DISPLAY(display)->useWindow)
     display_window_init (GGOBI_WINDOW_DISPLAY(display), nplots*width, height, 3, gg);

/*
 * Add the main menu bar
*/
  vbox = GTK_WIDGET(display); 
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 1);

  if(GGOBI_IS_WINDOW_DISPLAY(display) && GGOBI_WINDOW_DISPLAY(display)->window) {
    gtk_container_add (GTK_CONTAINER (GGOBI_WINDOW_DISPLAY(display)->window), vbox);

	display->menu_manager = display_menu_manager_create(display);
    //gg->parcoords.accel_group = gtk_accel_group_new ();
    display->menubar = create_menu_bar(display->menu_manager, parcoords_ui,
			     GGOBI_WINDOW_DISPLAY(display)->window);

    /*-- add a tooltip to the file menu --*/
    /* - tooltips are generally not done for toplevel menus
	w = gtk_item_factory_get_widget (factory, "<main>/File");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips),
			  gtk_menu_get_attach_widget (GTK_MENU(w)),
			  "File menu for this display", NULL);
	*/
    /*
     * After creating the menubar, and populating the file menu,
     * add the Options and Link menus another way
     */
    /*parcoords_display_menus_make (display, gg->parcoords.accel_group,
                                G_CALLBACK(display_options_cb), 
				  display->menubar, gg);*/
    gtk_box_pack_start (GTK_BOX (vbox), display->menubar, false, true, 0);
  }


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

  /*-- --*/

  for (i = 0; i < nplots; i++) {
    sp = ggobi_parcoords_splot_new(display, gg);
    sp->p1dvar = vars[i]; 

/*
    if (sub_plots == NULL) {
      sp = splot_new (display, width, height, gg);
      sp->p1dvar = i; 
    } else
       sp = sub_plots[i];
*/

    display->splots = g_list_append (display->splots, (gpointer) sp);
    gtk_box_pack_start (GTK_BOX (gg->parcoords.arrangement_box),
      sp->da, true, true, 0);
  }
	
  if(GGOBI_WINDOW_DISPLAY(display)->window)
     gtk_widget_show_all (GGOBI_WINDOW_DISPLAY(display)->window);

  return display;
}



static gboolean
parcoords_var_selected (gint jvar, displayd *display)
{
  gboolean selected = false;
  splotd *s;
  GList *l = display->splots;
  while (l) {
    s = (splotd *) l->data;
    if (s->p1dvar == jvar) {
      selected = true;
      break;
    }
    l = l->next ;
  }
  return selected;
}


gboolean
parcoords_varsel (cpaneld *cpanel, splotd *sp, gint jvar, gint *jvar_prev,
  ggobid *gg)
{
   gboolean status = false;
   ParCoordsSelectionMode mode;
   gboolean isVarSelected;

   mode = cpanel->parcoords_selection_mode;
   isVarSelected = parcoords_var_selected (jvar, gg->current_display);

   if((mode == VAR_DELETE && isVarSelected) || !isVarSelected)
        status = parcoords_add_delete_splot(cpanel, sp, jvar, jvar_prev, gg, gg->current_display, mode);

   return(status);
}


gboolean
parcoords_add_delete_splot(cpaneld *cpanel, splotd *sp, gint jvar, gint *jvar_prev, ggobid *gg, displayd *display, ParCoordsSelectionMode selMode)
{

  gboolean redraw = true;
  gint nplots = g_list_length (display->splots);
  gint k, width, height;
  gint jvar_indx, new_indx;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *box;// *w;
  //gfloat ratio = 1.0;


  /* The index of gg.current_splot */
  gint sp_indx = g_list_index (display->splots, sp);

  /*
  if(GGOBI_IS_WINDOW_DISPLAY(display))
      gtk_window_set_policy (GTK_WINDOW (GGOBI_WINDOW_DISPLAY(display)->window),
        false, false, false);*/

  splot_get_dimensions (sp, &width, &height);

  /*-- VAR_DELETE  --*/
  if (selMode == VAR_DELETE)
  {
    /* If jvar is one of the plotted variables, its corresponding plot */
    splotd *jvar_sp = NULL;

    k = 0;
    l = display->splots;
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

    if (jvar_sp != NULL && nplots > 1) {

      /*-- Delete the plot from the list, and destroy it. --*/
      display->splots = g_list_remove (display->splots, (gpointer) jvar_sp);
#if 0
      /*-- keep the window from shrinking by growing all plots --*/
      ratio = (gfloat) nplots / (gfloat) (nplots-1);
      if (cpanel->parcoords_arrangement == ARRANGE_ROW)
        width = (gint) (ratio * (gfloat) width);
      else
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
#endif
      /*
       * If the plot being removed is the current plot, reset
       * gg->current_splot.
      */
      /* ?mode or ?mode_prev?  Don't know. */
      if (jvar_sp == gg->current_splot) {
        sp_event_handlers_toggle (sp, off, display->cpanel.pmode, display->cpanel.imode);

        new_indx = (jvar_indx == 0) ? 0 : MIN (nplots-1, jvar_indx);
        gg->current_splot = (splotd *)
          g_list_nth_data (display->splots, new_indx);
        /* just for insurance, to handle the unforeseen */
        if (gg->current_splot == NULL)
          gg->current_splot = (splotd *) g_list_nth_data (display->splots, 0);
        display->current_splot = gg->current_splot;

/*-- dfs, testing -- all this, or just event_handlers_toggle? --*/
        splot_set_current (gg->current_splot, on, gg);
      }

      splot_free (jvar_sp, display, gg);

      nplots--;
    }

  /*-- VAR_REPLACE or VAR_INSERT or VAR_APPEND  --*/
  } else /* */ {

    if (selMode == VAR_REPLACE) {

      *jvar_prev = sp->p1dvar;
      sp->p1dvar = jvar;
      redraw = true;

    } else {

      /*-- prepare to reset the current plot --*/
      sp_event_handlers_toggle (sp, off, display->cpanel.pmode, display->cpanel.imode);

      /*-- keep the window from growing by shrinking all plots --*/
      /*ratio = (gfloat) nplots / (gfloat) (nplots+1);
      if (cpanel->parcoords_arrangement == ARRANGE_ROW)
        width = (gint) (ratio * (gfloat) width);
      else
        height = (gint) (ratio * (gfloat) height);
      */

      sp_new = ggobi_parcoords_splot_new (display, gg);
      sp_new->p1dvar = jvar; 

	  box = (sp->da)->parent;
	  gtk_box_pack_start (GTK_BOX (box), sp_new->da, true, true, 0);
	  
      if (cpanel->parcoords_selection_mode == VAR_INSERT) {
        display->splots = g_list_insert (display->splots,
          (gpointer) sp_new, sp_indx);
		gtk_box_reorder_child(GTK_BOX(box), sp_new->da, sp_indx);
	  } else if (cpanel->parcoords_selection_mode == VAR_APPEND)
        display->splots = g_list_append (display->splots,
          (gpointer) sp_new);

	  gtk_widget_show (sp_new->da);

#if 0
      l = display->splots;
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
#endif
      gg->current_splot = sp->displayptr->current_splot = sp_new;
      sp_event_handlers_toggle (sp_new, on, display->cpanel.pmode, display->cpanel.imode);
      redraw = true;
    }
  }

  /*if(GGOBI_IS_WINDOW_DISPLAY(display))
      gtk_window_set_policy (GTK_WINDOW (GGOBI_WINDOW_DISPLAY(display)->window),
       true, true, false);*/

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
    i = d->rows_in_plot.els[k];
    m = 2*i;

    /*-- if it's the leftmost plot, don't draw the left whisker --*/
    if (sp_prev == NULL)
      draw_whisker = false;
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    else if (!d->missings_show_p && d->nmissing > 0 &&
            (MISSING_P(i,sp->p1dvar) || MISSING_P(i,sp_prev->p1dvar)))
/*
    else if (d->nmissing > 0 && !d->missings_show_p &&
      (d->missing.vals[i][sp->p1dvar] ||
       d->missing.vals[i][sp_prev->p1dvar]))
*/
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

    /*-- if it's the rightmost plot, don't draw the right whisker --*/
    if (sp_next == NULL)
      draw_whisker = false;
    /*-- .. also if we're not drawing missings, and an endpoint is missing --*/
    else if (!d->missings_show_p && d->nmissing > 0 &&
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

  if (sp_prev != NULL) {
    sp_rewhisker (sp_prev_prev, sp_prev, sp, gg);
  }

  if (sp_next == NULL) {
    sp_rewhisker (sp_prev, sp, NULL, gg);
  }
}
