/* scatmat.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define WIDTH 200
#define HEIGHT 200
#define MAXNVARS 4   /* only used to set up the initial matrix */

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,     NULL,     0,                    "<Branch>" },
  { "/FFile/Print",    "",       (GtkItemFactoryCallback) display_print_cb, 0, "<Item>" },
  { "/FFile/sep",      NULL,     NULL,     0, "<Separator>" },
  { "/FFile/Close",    "",       (GtkItemFactoryCallback) display_close_cb, 0, "<Item>" },
};

/*
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
*/

static void
scatmat_display_menus_make (displayd *display, GtkAccelGroup *accel_group,
  GtkSignalFunc func, GtkWidget *mbar, ggobid *gg)
{
  GtkWidget *options_menu, *submenu, *item;

  /*-- Options menu --*/
  submenu = submenu_make ("_Options", 'O', accel_group);
  options_menu = gtk_menu_new ();

  item = CreateMenuCheck (options_menu, "Show points",
    func, GINT_TO_POINTER (DOPT_POINTS), on, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
/*  -- once the scatterplot is working, consider this --
  item = CreateMenuCheck (options_menu, "Show edges (undirected)",
    func, GINT_TO_POINTER (DOPT_EDGES_U), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu, "Show 'arrowheads'",
    func, GINT_TO_POINTER (DOPT_EDGES_U), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  item = CreateMenuCheck (options_menu, "Show edges (directed)",
    func, GINT_TO_POINTER (DOPT_EDGES_D), off, gg);
  gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
*/

  if (!display->missing_p) {
    item = CreateMenuCheck (options_menu, "Show missings",
      func, GINT_TO_POINTER (DOPT_MISSINGS),
      display->options.missings_show_p, gg);
    gtk_object_set_data (GTK_OBJECT (item), "display", (gpointer) display);
  }
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (submenu), options_menu);
  submenu_append (submenu, mbar);
  gtk_widget_show (submenu);
}

displayd *
scatmat_new (gboolean missing_p,
  gint numRows, gint *rows,
  gint numCols, gint *cols,
  datad *d, ggobid *gg) 
{
  GtkWidget *vbox, *frame;
  GtkWidget *mbar;
  gint i, j, ctr;
  gint width, height;
  gint scr_width, scr_height;
  gint scatmat_nrows, scatmat_ncols;
  splotd *sp;
  displayd *display;
  GtkAccelGroup *scatmat_accel_group;

  display = display_alloc_init (scatmat, missing_p, d, gg);
  /* If the caller didn't specify the rows and columns, 
     use the default which is the number of variables
     in the dataset or the maximum number of columns
     within a scatterplot matrix.
     ! Need to check rows and cols are allocated. !
   */
  if (numRows == 0 || numCols == 0) {
    scatmat_nrows = scatmat_ncols = MIN (d->ncols, MAXNVARS);
    for (j=0; j<scatmat_nrows; j++)
      rows[j] = cols[j] = j;
  } else { 
    scatmat_nrows = numRows;
    scatmat_ncols = numCols;
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
   * add the Options and Link menus another way
  */
  scatmat_display_menus_make (display, scatmat_accel_group,
                              (GtkSignalFunc) display_options_cb, mbar, gg);
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
      sp = splot_new (display, width, height, gg);
      sp->xyvars.x = rows[i]; 
      sp->xyvars.y = cols[j]; 
      sp->p1dvar = (rows[i] == cols[j]) ? cols[j] : -1;

      display->splots = g_list_append (display->splots, (gpointer) sp);

      gtk_table_attach (GTK_TABLE (display->table), sp->da, i, i+1, j, j+1,
        (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
        (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
        1, 1);
      gtk_widget_show (sp->da);
    }
  }

  display->scatmat_cols = NULL;
  for (j=0; j<scatmat_ncols; j++)
    display->scatmat_cols = g_list_append (display->scatmat_cols,
                                           GINT_TO_POINTER (cols[j]));
  display->scatmat_rows = NULL;
  for (i=0; i<scatmat_nrows; i++)
    display->scatmat_rows = g_list_append (display->scatmat_rows,
                                           GINT_TO_POINTER (cols[i]));

  gtk_widget_show (display->table);

  /*-- position the display toward the lower left of the main window --*/
  display_set_position (display, gg);

  gtk_widget_show_all (display->window);

  return display;
}

/*
 * check symmetry assumption -- the plot could have been rendered
 * asymmetric by actions in the API.
*/
static gboolean
scatmat_symmetric (displayd *display) 
{
  GList *lcols, *lrows;
  gint scatmat_nvars, j;
  gboolean symmetric = true;

  if ((scatmat_nvars = g_list_length (display->scatmat_rows)) !=
      g_list_length (display->scatmat_cols))
  {
    symmetric = false;
  } else {
    lrows = display->scatmat_rows;
    lcols = display->scatmat_cols;
    for (j=0; j<scatmat_nvars; j++) {
      if (GPOINTER_TO_INT (lcols->data) != GPOINTER_TO_INT (lrows->data)) {
        symmetric = false;
        break;
      }
      lcols = lcols->next;
      lrows = lrows->next;
    }
  }
  return symmetric;
}

/*
 * Assuming symmetry, find out whether a variable is selected
*/
static gboolean
scatmat_var_selected (gint jvar, displayd *display)
{
  gboolean selected = false;
  gint j;

  GList *l = display->scatmat_cols;
  while (l) {
    j = GPOINTER_TO_INT (l->data);
    if (j == jvar) {
      selected = true;
      break;
    }
    l = l->next;
  }
  return selected;
}

static splotd *
scatmat_add_plot (gint xvar, gint yvar, gint col, gint row,
  gint width, gint height,
  displayd *display, ggobid *gg)
{
  splotd *sp_new;

  sp_new = splot_new (display, width, height, gg);
  sp_new->xyvars.x = xvar;
  sp_new->xyvars.y = yvar;
  sp_new->p1dvar = (sp_new->xyvars.x == sp_new->xyvars.y) ? xvar : -1;

  gtk_table_attach (GTK_TABLE (display->table),
    sp_new->da, col, col+1, row, row+1,
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND), 
    (GtkAttachOptions) (GTK_SHRINK|GTK_FILL|GTK_EXPAND),
    1, 1);
  gtk_widget_show (sp_new->da);

  /* We don't care where, I think */
  display->splots = g_list_append (display->splots,
    (gpointer) sp_new);

  return sp_new;
}

gboolean
scatmat_varsel_simple (cpaneld *cpanel, splotd *sp, gint jvar,
  gint *jvar_prev, ggobid *gg)
{
  gboolean redraw = true;
  gboolean Delete = false;
  gint k, width, height;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *da;
  gfloat ratio = 1.0;
  GtkTableChild *child;
  displayd *display = gg->current_display;

  /* the number of variables in the current layout -- assuming symmetry */
  gint scatmat_nvars = g_list_length (display->scatmat_cols);

  /* Check the assumption of layout symmetry */
  if (!scatmat_symmetric) {
/**/return  (false);
  }

  splot_get_dimensions (gg->current_splot, &width, &height);

/*
 * If jvar is selected, delete a row and a column.  Delete the
 * variable that's selected; we have no interest in the current splot.
*/

  /*-- VAR_DELETE --*/
  if (cpanel->scatmat_selection_mode == VAR_DELETE) {
    if (scatmat_var_selected (jvar, display)) {
      gboolean deleted_p = false;  /*-- have we deleted yet? --*/
      /* if jvar is one of the plotted variables, its row and column */
      gint jvar_rc;
      jvar_rc = g_list_index (display->scatmat_cols, GINT_TO_POINTER (jvar));

      ratio = (gfloat) scatmat_nvars / (gfloat) (scatmat_nvars-1);
      width = (gint) (ratio * (gfloat) width);

      l = (GTK_TABLE (display->table))->children;
      while (l) {
        Delete = false;
        child = (GtkTableChild *) l->data;
        l = l->next;
        da = child->widget;

        if (child->left_attach == jvar_rc)
          Delete = true;
        else if (child->left_attach > jvar_rc) {
          child->left_attach--;
          child->right_attach--;
        }
        if (child->top_attach == jvar_rc) {
          Delete = true;
        } else if (child->top_attach > jvar_rc) {
          child->top_attach--;
          child->bottom_attach--;
        }

        if (Delete && !deleted_p) {

          s = (splotd *) gtk_object_get_data (GTK_OBJECT (da), "splotd");
          display->splots = g_list_remove (display->splots,
                                           (gpointer) s);
          /*
           * add a reference to da here, because it's going to be
           * destroyed in splot_free, and we don't want it destroyed
           * as a result of gtk_container_remove.
          */
          gtk_widget_ref (da);
          gtk_container_remove (GTK_CONTAINER (display->table), da);

          if (s == gg->current_splot)
            sp_event_handlers_toggle (s, off);
          splot_free (s, display, gg);

          deleted_p = true;

        } else {
          gtk_widget_set_usize (da, -1, -1);
          gtk_widget_set_usize (da, width, height);
        }
  
        /*
         * Delete the list elements for the row&column that are being deleted
        */
        display->scatmat_cols = g_list_remove_nth (display->scatmat_cols,
                                                   jvar_rc);
        display->scatmat_rows = g_list_remove_nth (display->scatmat_rows,
                                                   jvar_rc);

        gtk_table_resize (GTK_TABLE (display->table),
                          g_list_length (display->scatmat_rows),
                          g_list_length (display->scatmat_cols));

        /*
         * I'm not sure this is necessary -- am I checking whether the
         * gg.current_splot was deleted?
        */
        gg->current_splot = (splotd *) g_list_nth_data (display->splots, 0);
        display->current_splot = gg->current_splot;

        redraw = false;
      }
    }
  }

/*
 * Otherwise, replace, insert or append a row <and> a column --
 * depending on the value of scatmat_selection_mode.
*/
  /*-- VAR_REPLACE or VAR_INSERT or VAR_APPEND  --*/
  else if (!scatmat_var_selected (jvar, display)) {

    /* the row and column of gg.current_splot */
    gint sprow = 1, spcol = -1;

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
    }

    if (sprow != spcol) {
      g_printerr ("Please select one of the plots on the diagonal\n");
/**/  return false;
    }

    if (cpanel->scatmat_selection_mode == VAR_REPLACE) {

      redraw = true;
      l = (GTK_TABLE (display->table))->children;
      while (l) {
        child = (GtkTableChild *) l->data;
        l = l->next;
        da = child->widget;
        s = (splotd *) gtk_object_get_data (GTK_OBJECT (da), "splotd");

        if (child->left_attach == spcol) {
          *jvar_prev = s->xyvars.x;
          s->xyvars.x = jvar;
          s->p1dvar = (s->xyvars.x == s->xyvars.y) ? jvar : -1;
        }

        if (child->top_attach == sprow) {
          *jvar_prev = s->xyvars.y;
          s->xyvars.y = jvar;
          s->p1dvar = (s->xyvars.x == s->xyvars.y) ? jvar : -1;
        }
      }

      display->scatmat_cols = g_list_replace_nth (display->scatmat_cols,
        GINT_TO_POINTER (jvar), spcol);
        /*scatmat_cols_print (display);*/

      display->scatmat_rows = g_list_replace_nth (display->scatmat_rows,
        GINT_TO_POINTER (jvar), sprow);
        /*scatmat_rows_print (display);*/

    } else {  /* VAR_INSERT or VAR_APPEND */
      gint newvar;
      gint row = -1, col = -1;

      /*-- prepare to reset the current plot --*/
      sp_event_handlers_toggle (sp, off);

      /*
       * First adjust the table, inserting or appending a row
       * and resetting the attachment values.
      */

      /*
       * Prepare to decrease the size of each plot: symmetry says
       * that some of these variables are redundant.
      */
      col = (cpanel->scatmat_selection_mode == VAR_INSERT) ? spcol :
                                                             scatmat_nvars;
      row = (cpanel->scatmat_selection_mode == VAR_INSERT) ? sprow :
                                                             scatmat_nvars;

      ratio = (gfloat) scatmat_nvars / (gfloat) (scatmat_nvars+1);
      width = (gint) (ratio * (gfloat) width);
      height = (gint) (ratio * (gfloat) height);

      /*
       * Fix up the attachments of the rows below and columns to the right
       * of the inserted/appended row/column.
      */
      for (l=(GTK_TABLE (display->table))->children; l; l=l->next) {
        child = (GtkTableChild *) l->data;
        da = child->widget;
        gtk_widget_set_usize (da, -1, -1);
        gtk_widget_set_usize (da, width, height);

        if (child->left_attach >= col) {
          child->left_attach++;
          child->right_attach++;
        } 
        if (child->top_attach >= row) {
          child->top_attach++;
          child->bottom_attach++;
        }
      }

      /*
       * Now create the new plots and fill in the new row/column.
       * Work out the correct p1dvar/xyvars values for each new plot.
      */

      scatmat_nvars++;
      display->scatmat_cols = g_list_insert (display->scatmat_cols,
                                             GINT_TO_POINTER (jvar),
                                             col);
      display->scatmat_rows = g_list_insert (display->scatmat_rows,
                                             GINT_TO_POINTER (jvar),
                                             row);

      /*-- note the strong assumption of symmetry here --*/
      for (k=0; k<scatmat_nvars; k++) {
        /* which variable is plotting in the k'th intersecting row/column? */
        newvar = GPOINTER_TO_INT (g_list_nth_data (display->scatmat_rows, k));

        sp_new = scatmat_add_plot (jvar, newvar, col, k,
          width, height, display, gg);

        if (k != row) {  /*-- except at the intersection, do it twice --*/
          sp_new = scatmat_add_plot (newvar, jvar, k, row,
            width, height, display, gg);
        }
      }

      gtk_table_resize (GTK_TABLE (gg->current_display->table),
                        scatmat_nvars, scatmat_nvars);

      gg->current_splot = sp->displayptr->current_splot = sp;
      sp_event_handlers_toggle (sp_new, on);
      redraw = true;
    }
  }

  return redraw;
}

/**
  This creates the scatter matrix window contents using static
  variables within this file.

  The corresponding code in scatmat_new should be deprecated and
  that routine should be made to call this one.
 */
gint *
createScatmatWindow(gint nrows, gint ncols, displayd *display, ggobid *gg, gboolean useWindow)
{
  GtkWidget *vbox, *frame;
  GtkWidget *mbar;
  gint width, height;
  gint scr_width, scr_height;
  gint scatmat_nrows=1, scatmat_ncols=1; /* Not used really! */
  GtkAccelGroup *scatmat_accel_group;
  gint *dims = NULL;


  display->p1d_orientation = HORIZONTAL;

  scatmat_cpanel_init (&display->cpanel, gg);

  if(useWindow) {
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
   * add the Options and Link menus another way
  */
  scatmat_display_menus_make (display, scatmat_accel_group,
                               (GtkSignalFunc) display_options_cb, mbar, gg);
  gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);

/*
 * splots in a table 
*/
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
  gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 1);

  gtk_widget_show (frame);
  }
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

  display->table = gtk_table_new (nrows, ncols, false);
  if(useWindow)
    gtk_container_add (GTK_CONTAINER (frame), display->table);


  dims = (gint *) g_malloc(sizeof(gint) * 2);
  dims[0] = width;
  dims[1] = height;

  return(dims);
}


#undef WIDTH
#undef HEIGHT
#undef MAXNVARS
