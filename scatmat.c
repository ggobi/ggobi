/* scatmat.c */

#include <strings.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH 200
#define HEIGHT 200
#define MAXNVARS 4   /* only used to set up the initial matrix */

static GtkAccelGroup *scatmat_accel_group;

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,     NULL,     0,                    "<Branch>" },
  { "/FFile/Print",    "",       display_print_cb, 0, "<Item>" },
  { "/FFile/sep",      NULL,     NULL,     0, "<Separator>" },
  { "/FFile/Close",    "",       display_close_cb, 0, "<Item>" },
};

static void
scatmat_rows_print (displayd *display) {
  GList *l;
  g_printerr ("rows: ");
  for (l=display->scatmat_rows; l != NULL; l=l->next)
    g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  g_printerr ("\n");
}
static void
scatmat_cols_print (displayd *display) {
  GList *l;
  g_printerr ("cols: ");
  for (l=display->scatmat_cols; l != NULL; l=l->next)
    g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  g_printerr ("\n");
}

static void
scatmat_display_menus_make (displayd *display, GtkAccelGroup *accel_group,
  GtkSignalFunc func, GtkWidget *mbar, ggobid *gg)
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
  CreateMenuCheck (display, options_menu, "Show lines (undirected)",
    func, GINT_TO_POINTER (DOPT_SEGS_U), off, gg);
  CreateMenuCheck (display, options_menu, "Show lines (directed)",
    func, GINT_TO_POINTER (DOPT_SEGS_D), off, gg);
  if (!display->missing_p)
    CreateMenuCheck (display, options_menu, "Show missings",
      func, GINT_TO_POINTER (DOPT_MISSINGS), on, gg);
  CreateMenuCheck (display, options_menu, "Show gridlines",
    func, GINT_TO_POINTER (DOPT_GRIDLINES), off, gg);
  CreateMenuCheck (display, options_menu, "Show axes",
    func, GINT_TO_POINTER (DOPT_AXES), on, gg);

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
scatmat_new (gboolean missing_p, splotd **sub_plots, gint numRows, gint numCols,
  ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  GtkWidget *mbar;
  gint i, j, ctr;
  gint width, height;
  gint scr_width, scr_height;
  gint scatmat_nrows, scatmat_ncols;
  splotd *sp;
  displayd *display;

  if (sub_plots == NULL) {
    display = display_alloc_init (scatmat, missing_p, gg);
    /* What about the p1d_orientation here? */
    scatmat_nrows = scatmat_ncols = MIN (gg->ncols, MAXNVARS);
  } else { 
    scatmat_nrows = numRows;
    scatmat_ncols = numCols;
    display = (displayd*) sub_plots[0]->displayptr;
  }

  display->p1d_orientation = HORIZONTAL;

  scatmat_cpanel_init (&display->cpanel, gg);

  display_window_init (display, 5, gg);

/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  scatmat_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 scatmat_accel_group, display->window, &mbar,
                 (gpointer) display);
  /*
   * After creating the menubar, and populating the file menu,
   * add the Display Options and Link menus another way
  */
  scatmat_display_menus_make (display, scatmat_accel_group,
                               display_options_cb, mbar, gg);
  gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);

/*
 * splots in a table 
*/
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 1);

  gtk_widget_show (frame);

  /*
   * make the matrix take up no more than some fraction
   * the screen by default, and make the plots square.
  */
  scr_width = gdk_screen_width () / 2;
  scr_height = gdk_screen_height () / 2;
  width = (WIDTH * scatmat_ncols > scr_width) ?
    (scr_width / scatmat_ncols) : WIDTH;
  height = (HEIGHT * scatmat_nrows > scr_height) ?
    (scr_height / scatmat_nrows) : HEIGHT;
  width = height = MIN (width, height);
  /* */

  display->table = gtk_table_new (scatmat_ncols, scatmat_nrows, false);
  gtk_container_add (GTK_CONTAINER (frame), display->table);
  display->splots = NULL;
  ctr = 0;
  for (i=0; i<scatmat_ncols; i++) {
    for (j=0; j<scatmat_nrows; j++, ctr++) {
      if(sub_plots == NULL) {
        sp = splot_new (display, width, height, gg);
        sp->xyvars.x = i; 
        sp->xyvars.y = j; 
        sp->p1dvar = (i == j) ? j : -1;
      } else {
        sp = sub_plots[ctr];
        splot_dimension_set(sp, width, height);
      }

      display->splots = g_list_append (display->splots, (gpointer) sp);

      gtk_table_attach (GTK_TABLE (display->table), sp->da, i, i+1, j, j+1,
        GTK_SHRINK|GTK_FILL|GTK_EXPAND, GTK_SHRINK|GTK_FILL|GTK_EXPAND,
        1, 1);
      gtk_widget_show (sp->da);
    }
  }

  display->scatmat_cols = NULL;
  for (j=0; j<scatmat_ncols; j++)
    display->scatmat_cols = g_list_append (display->scatmat_cols,
                                           GINT_TO_POINTER (j));
  display->scatmat_rows = NULL;
  for (i=0; i<scatmat_nrows; i++)
    display->scatmat_rows = g_list_append (display->scatmat_rows,
                                           GINT_TO_POINTER (i));

  gtk_widget_show (display->table);
  gtk_widget_show_all (display->window);

  return display;
}

/* variable selection */

gboolean
scatmat_varsel (cpaneld *cpanel, splotd *sp,
  gint jvar, gint *jvar_prev, gint button, gboolean alt_mod, ggobid *gg)
{
  gboolean redraw = true;
  gboolean delete = false;
  gint k, width, height;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *da;
  gfloat ratio = 1.0;
  GtkTableChild *child;
  displayd *display = gg->current_display;
  gint scatmat_ncols = g_list_length (display->scatmat_cols);
  gint scatmat_nrows = g_list_length (display->scatmat_rows);

  /* The row and column of gg.current_splot */
  gint sprow = -1, spcol = -1;

  /* If jvar is one of the plotted variables, its row and column */
  gint jvar_row = -1, jvar_col = -1;

  /*
   * Find the row and column index of the currently selected plot
  */
  for (l=(GTK_TABLE (display->table))->children; l; l=l->next) {
    child = (GtkTableChild *) l->data;
    da = child->widget;
    s = (splotd *) gtk_object_get_data (GTK_OBJECT (da), "splotd");
    if (s == gg->current_splot) {
      sprow = child->top_attach;
      spcol = child->left_attach;
    }

    /*
     * ... and the row/column indices of jvar, if present
    */
    if (s->p1dvar == -1) {
      if (button == 1 && s->xyvars.x == jvar)
        jvar_col = child->left_attach;
      else if (button != 1 && s->xyvars.y == jvar)
        jvar_row = child->top_attach;
    } else {
      if (s->p1dvar == jvar) {
        jvar_col = child->left_attach;
        jvar_row = child->top_attach;
      }
    }
  }

  splot_get_dimensions (gg->current_splot, &width, &height);

/*
 * If the alt key is pressed and jvar is plotted, delete a
 * row or a column -- depending on which button was pressed.
*/

  if (alt_mod == true) {
    if ((scatmat_ncols > 1 && button == 1) ||
        (scatmat_nrows > 1 && button != 1))
    {
      if (button == 1) {
        ratio = (gfloat) scatmat_ncols / (gfloat) (scatmat_ncols-1);
        width = (gint) (ratio * (gfloat) width);
      } else {
        ratio = (gfloat) scatmat_nrows / (gfloat) (scatmat_nrows-1);
        height = (gint) (ratio * (gfloat) height);
      }


      l = (GTK_TABLE (display->table))->children;
      while (l) {
        delete = false;
        child = (GtkTableChild *) l->data;
        l = l->next;
        da = child->widget;

        if (button == 1) {
          if (child->left_attach == jvar_col)
            delete = true;
          else if (child->left_attach > jvar_col) {  /* if to the right */
            child->left_attach--;
            child->right_attach--;
          }

        } else if (button == 2 || button == 3) {
          if (child->top_attach == jvar_row) {
            delete = true;
          } else if (child->top_attach > jvar_row) {  /* if below */
            child->top_attach--;
            child->bottom_attach--;
          }
        }

        if (delete) {
          s = (splotd *) gtk_object_get_data (GTK_OBJECT (da), "splotd");
          display->splots = g_list_remove (display->splots,
                                           (gpointer) s);
          gtk_container_remove (GTK_CONTAINER (display->table), da);

          if (s == gg->current_splot)
            sp_event_handlers_toggle (s, off);
          splot_free (s, display, gg);
        } else {
          gtk_widget_set_usize (da, -1, -1);
          gtk_widget_set_usize (da, width, height);
        }
      }


      /*
       * Delete the list element for the row/column that's being deleted
      */
      if (button == 1) {
        display->scatmat_cols = g_list_remove_nth (display->scatmat_cols,
                                                   spcol);
      } else {
        display->scatmat_rows = g_list_remove_nth (display->scatmat_rows,
                                                   sprow);
      }

      gtk_table_resize (GTK_TABLE (display->table),
                        g_list_length (display->scatmat_rows),
                        g_list_length (display->scatmat_cols));

/*
 * I'm not sure this is necessary -- am I checking whether the
 * gg.current_splot was deleted?
*/
      gg->current_splot = (splotd *) g_list_nth_data (display->splots, 0);
    }
    redraw = false;

  } else {

/*
 * Otherwise, replace, insert or append a row or a column --
 * depending on which button was pressed, and depending on the
 * value of scatmat_selection_mode.
*/

    if (cpanel->scatmat_selection_mode == VAR_REPLACE) {

      redraw = true;
      l = (GTK_TABLE (display->table))->children;
      while (l) {
        child = (GtkTableChild *) l->data;
        l = l->next;
        da = child->widget;
        s = (splotd *) gtk_object_get_data (GTK_OBJECT (da), "splotd");

        if (button == 1) {
          if (child->left_attach == spcol) {
            *jvar_prev = s->xyvars.x;
            s->xyvars.x = jvar;
            s->p1dvar = (s->xyvars.x == s->xyvars.y) ? jvar : -1;
          }

        } else if (button == 2 || button == 3) {
          if (child->top_attach == sprow) {
            *jvar_prev = s->xyvars.y;
            s->xyvars.y = jvar;
            s->p1dvar = (s->xyvars.x == s->xyvars.y) ? jvar : -1;
          }
        }
      }

      if (button == 1) {
        display->scatmat_cols = g_list_replace_nth (display->scatmat_cols,
          GINT_TO_POINTER (jvar), spcol);
        scatmat_cols_print (display);
      } else {
        display->scatmat_rows = g_list_replace_nth (display->scatmat_rows,
          GINT_TO_POINTER (jvar), sprow);
        scatmat_rows_print (display);
      }

    } else {  /* VAR_INSERT or VAR_APPEND */
      gint row = -1, col = -1;

      /*
       * First adjust the table, inserting or appending a row
       * and resetting the attachment values.
      */

      /*
       * Prepare to decrease the size of each plot.
      */
      if (button == 1) {  /* insert or append a column */
        col = (cpanel->scatmat_selection_mode == VAR_INSERT) ? spcol : spcol-1;
        ratio = (gfloat) scatmat_ncols / (gfloat) (scatmat_ncols+1);
        width = (gint) (ratio * (gfloat) width);
      } else {
        row = (cpanel->scatmat_selection_mode == VAR_INSERT) ? sprow : sprow-1;
        ratio = (gfloat) scatmat_nrows / (gfloat) (scatmat_nrows+1);
        height = (gint) (ratio * (gfloat) height);
      }

      /*
       * Fix up the attachments of the rows below and columns to the right
       * of the inserted/appended row/column.
      */
      for (l=(GTK_TABLE (display->table))->children; l; l=l->next) {
        child = (GtkTableChild *) l->data;
        da = child->widget;
        gtk_widget_set_usize (da, -1, -1);
        gtk_widget_set_usize (da, width, height);

        if (button == 1 && child->left_attach >= col) {
          child->left_attach++;
          child->right_attach++;

        } else if (button != 1 && child->top_attach >= row) {
          child->top_attach++;
          child->bottom_attach++;
        }
      }

      /*
       * Now create the new plots and fill in the new row/column.
       * Work out the correct p1dvar/xyvars values for each new plot.
      */

      if (button == 1) {
        gint rowvar;

        scatmat_ncols++;
        display->scatmat_cols = g_list_insert (display->scatmat_cols,
                                               GINT_TO_POINTER (jvar),
                                               col);
        scatmat_cols_print (display);
        for (k=0; k<scatmat_nrows; k++) {
          sp_new = splot_new (display, width, height, gg);

          /* which variable is plotting in the k'th intersecting row? */
          rowvar = GPOINTER_TO_INT (g_list_nth_data (display->scatmat_rows, k));
          sp_new->xyvars.x = jvar;
          sp_new->xyvars.y = rowvar;
          sp_new->p1dvar = (sp_new->xyvars.x == sp_new->xyvars.y) ? jvar : -1;

          gtk_table_attach (GTK_TABLE (display->table),
            sp_new->da, col, col+1, k, k+1,
            GTK_SHRINK|GTK_FILL|GTK_EXPAND, GTK_SHRINK|GTK_FILL|GTK_EXPAND,
            1, 1);
          gtk_widget_show (sp_new->da);

          /* We don't care where, I think */
          display->splots = g_list_append (display->splots,
            (gpointer) sp_new);
        }

      } else {
        gint colvar;

        scatmat_nrows++;
        display->scatmat_rows = g_list_insert (display->scatmat_rows,
                                               GINT_TO_POINTER (jvar),
                                               row);
        scatmat_rows_print (display);
        for (k=0; k<scatmat_nrows; k++) {
          sp_new = splot_new (display, width, height, gg);

          sp_new->p1dvar = jvar;  /* placeholder */

          /* which variable is plotting in the k'th intersecting column? */
          colvar = GPOINTER_TO_INT (g_list_nth_data (display->scatmat_cols, k));
          sp_new->xyvars.x = colvar;
          sp_new->xyvars.y = jvar;
          sp_new->p1dvar = (sp_new->xyvars.x == sp_new->xyvars.y) ? jvar : -1;

          gtk_table_attach (GTK_TABLE (display->table),
            sp_new->da, k, k+1, row, row+1,
            GTK_SHRINK|GTK_FILL|GTK_EXPAND, GTK_SHRINK|GTK_FILL|GTK_EXPAND,
            1, 1);
          gtk_widget_show (sp_new->da);

          /* We don't care where, I think */
          display->splots = g_list_append (gg->current_display->splots,
            (gpointer) sp_new);
        }
      }


      gtk_table_resize (GTK_TABLE (gg->current_display->table),
                        scatmat_nrows, scatmat_ncols);

      gg->current_splot = sp_new;
      redraw = true;
    }
  }

  return redraw;
}


#undef WIDTH
#undef HEIGHT
#undef MAXNVARS
