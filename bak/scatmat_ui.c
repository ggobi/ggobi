/* scatmat_ui.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <stdlib.h>

#include "vars.h"

#define WIDTH 200
#define HEIGHT 200
#define MAXNVARS 3   /* only used to set up the initial matrix */

/* external functions */
extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                  GtkSignalFunc);
extern splotd * splot_new (displayd *, gint, gint);
extern void get_main_menu (GtkItemFactoryEntry[], gint, GtkAccelGroup *,
                           GtkWidget  *, GtkWidget **, gpointer);
extern GtkWidget *CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *,
                                  GtkWidget *, GtkAccelGroup *,
                                  GtkSignalFunc, gpointer);
extern void init_control_panel (displayd *, cpaneld *, gint);
extern void splot_get_dimensions (splotd *, gint *, gint *);
extern void splot_free (splotd *);
extern void display_free (displayd *);
extern GList * g_list_remove_nth (GList *, gint);
extern GList * g_list_replace_nth (GList *, gpointer, gint);
/* */

static GtkWidget *mbar;
static GtkAccelGroup *scatmat_accel_group;

/* utilities within this file */
static void
print_attachments () {
  GList *l;
  GtkTableChild *child;

  g_printerr ("attachments:\n");
  for (l=(GTK_TABLE (current_display->table))->children; l; l=l->next) {
    child = (GtkTableChild *) l->data;
    g_printerr (" %d %d, %d %d\n",
      child->left_attach, child->right_attach,
      child->top_attach, child->bottom_attach);
  }
}

static void
print_lists (displayd *display) {
  GList *l;

  g_printerr ("columns: ");
  for (l=display->scatmat_cols; l; l=l->next)
    g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  g_printerr ("\n");
  g_printerr ("rows: ");
  for (l=display->scatmat_rows; l; l=l->next)
    g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  g_printerr ("\n");
}

static void
print_dims (displayd *d) {
  g_printerr ("children: %d\n", 
    g_list_length (gtk_container_children (GTK_CONTAINER (d->table))));

  g_printerr ("scatmat_ncols= %d, scatmat_nrows= %d ; ",
    g_list_length (d->scatmat_cols), g_list_length (d->scatmat_rows));
  g_printerr ("rows= %d, cols= %d\n",
    GTK_TABLE (d->table)->nrows, GTK_TABLE (d->table)->ncols);
}
/* */


static void
close_cb (displayd *display, guint action, GtkWidget *w) {
  display_free (display);
}
static void
delete_cb (GtkWidget *w, GdkEvent *event, displayd *display) {
  display_free (display);
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,         NULL, 0, "<Branch>" },
  { "/FFile/Print",  
         "",         NULL,        0, "<Item>" },
  { "/FFile/sep",   NULL,     NULL,          0, "<Separator>" },
  { "/FFile/Close",  
         "",         close_cb,        0, "<Item>" },
};


static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append"};
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  cpaneld *cpanel = &current_display->cpanel;

  switch (indx) {
    case 0:
      cpanel->scatmat_selection_mode = VAR_REPLACE;
      break;
    case 1:
      cpanel->scatmat_selection_mode = VAR_INSERT;
      break;
    case 2:
      cpanel->scatmat_selection_mode = VAR_APPEND;
      break;
  }
}


void
cpanel_scatmat_make () {
  GtkWidget *vb, *lbl, *opt;
  
  control_panel[SCATMAT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[SCATMAT]), 5);

/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[SCATMAT]), vb, false, false, 0);

  lbl = gtk_label_new ("Selection mode:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Selecting a variable either replaces the variable in the current plot (swapping if appropriate), inserts a new plot before the current plot, or appends a new plot after it",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, selection_mode_lbl,
                        sizeof (selection_mode_lbl) / sizeof (gchar *),
                        selection_mode_cb);

  gtk_widget_show_all (control_panel[SCATMAT]);
}

void
init_scatmat_cpanel (cpaneld *cpanel) {
  cpanel->p1d_type = ASH;
}

displayd *
scatmat_new () {
  GtkWidget *vbox, *frame;
  gint i, j;
  gint width, height;
  gint scr_width, scr_height;
  gint scatmat_nrows, scatmat_ncols;
  splotd *sp;
  displayd *display = (displayd *) g_malloc (sizeof (displayd));
  display->displaytype = scatmat;

  scatmat_nrows = scatmat_ncols = MIN (xg.ncols, MAXNVARS);

  init_control_panel (display, &display->cpanel, SCATMAT);

  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (display->window),
                       "displayd",
                       (gpointer) display);
  gtk_window_set_policy (GTK_WINDOW (display->window), true, true, true);
  gtk_container_set_border_width (GTK_CONTAINER (display->window), 5);
  gtk_signal_connect (GTK_OBJECT (display->window), "delete_event",
                      GTK_SIGNAL_FUNC (delete_cb), (gpointer) display);

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
  gtk_box_pack_start (GTK_BOX (vbox), mbar, false, true, 0);

  gtk_widget_show (vbox);

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
  display->splots = null;
  for (i=0; i<scatmat_ncols; i++) {
    for (j=0; j<scatmat_nrows; j++) {

      sp = splot_new (display, width, height);

      sp->xyvars.x = i; 
      sp->xyvars.y = j; 
      sp->p1dvar = (i == j) ? j : -1;

      display->splots = g_list_append (display->splots, (gpointer) sp);

      gtk_table_attach (GTK_TABLE (display->table), sp->da, i, i+1, j, j+1,
        GTK_SHRINK|GTK_FILL|GTK_EXPAND, GTK_SHRINK|GTK_FILL|GTK_EXPAND,
        1, 1);
      gtk_widget_show (sp->da);
    }
  }

  display->scatmat_cols = null;
  for (j=0; j<scatmat_ncols; j++)
    display->scatmat_cols = g_list_append (display->scatmat_cols,
                                           GINT_TO_POINTER (j));
  display->scatmat_rows = null;
  for (i=0; i<scatmat_nrows; i++)
    display->scatmat_rows = g_list_append (display->scatmat_rows,
                                           GINT_TO_POINTER (i));

  gtk_widget_show (display->table);
  gtk_widget_show (display->window);

  return display;
}

/* variable selection */

gboolean
scatmat_select_variable (cpaneld *cpanel, splotd *sp,
                         gint jvar, gint *jvar_prev,
                         gint button, gboolean alt_mod)
{
  gboolean redraw = true;
  gboolean delete = false;
  gint k, width, height;
  GList *l;
  splotd *s, *sp_new;
  GtkWidget *da;
  gfloat ratio = 1.0;
  GtkTableChild *child;
  displayd *display = current_display;
  gint scatmat_ncols = g_list_length (display->scatmat_cols);
  gint scatmat_nrows = g_list_length (display->scatmat_rows);

  /* The row and column of current_splot */
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
    if (s == current_splot) {
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

  splot_get_dimensions (current_splot, &width, &height);

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
g_printerr ("top_attach %d jvar_row %d\n", child->top_attach, jvar_row);

        if (button == 1) {
          if (child->left_attach == jvar_col)
            delete = true;
          else if (child->left_attach > jvar_col) {  /* if to the right */
            child->left_attach--;
            child->right_attach--;
          }

        } else if (button == 2 || button == 3) {
          if (child->top_attach == jvar_row) {
g_printerr ("deleting\n");
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
          splot_free (s);
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

      current_splot = (splotd *) g_list_nth_data (display->splots, 0);
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
      } else {
        display->scatmat_rows = g_list_replace_nth (display->scatmat_rows,
          GINT_TO_POINTER (jvar), sprow);
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

        for (k=0; k<scatmat_nrows; k++) {
          sp_new = splot_new (display, width, height);

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

        for (k=0; k<scatmat_nrows; k++) {
          sp_new = splot_new (display, width, height);

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
          display->splots = g_list_append (current_display->splots,
            (gpointer) sp_new);
        }
      }


      gtk_table_resize (GTK_TABLE (current_display->table),
                        scatmat_nrows, scatmat_ncols);

      current_splot = sp_new;
      redraw = true;
    }
  }

/*
  gtk_window_set_policy (GTK_WINDOW (current_display->window),
    true, true, true);
*/

  return redraw;
}

/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/

GtkWidget *scatmat_mode_menu;


void
make_scatmat_menus (GtkAccelGroup *accel_group, GtkSignalFunc func) {

/*
 * I/O menu
*/
  scatmat_mode_menu = gtk_menu_new ();

  CreateMenuItem (scatmat_mode_menu, "Scatterplot Matrix",
    "^x", "", NULL, accel_group, func, GINT_TO_POINTER (SCATMAT));

  /* Add a separator */
  CreateMenuItem (scatmat_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (scatmat_mode_menu, "Scale",
    "^s", "", NULL, accel_group, func, GINT_TO_POINTER (SCALE));
  CreateMenuItem (scatmat_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, GINT_TO_POINTER (BRUSH));
  CreateMenuItem (scatmat_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, GINT_TO_POINTER (IDENT));
  CreateMenuItem (scatmat_mode_menu, "Edit Lines",
    "^l", "", NULL, accel_group, func, GINT_TO_POINTER (LINEED));
  CreateMenuItem (scatmat_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, GINT_TO_POINTER (MOVEPTS));

  gtk_widget_show (scatmat_mode_menu);
}

/**********************************************************************/
/******************* End of main menubar section **********************/
/**********************************************************************/

#undef WIDTH
#undef HEIGHT
