/* parcoords_ui.c */

#include <gtk/gtk.h>
#include "vars.h"

#define WIDTH   150
#define HEIGHT  300

/* external functions */
extern void populate_option_menu (GtkWidget *, gchar **, gint,
                                  GtkSignalFunc);
extern void set_mode_cb (gpointer, guint, GtkWidget *);
extern void get_main_menu (GtkItemFactoryEntry[], gint, GtkAccelGroup *,
                           GtkWidget  *, GtkWidget **, gpointer);
extern GtkWidget *CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *,
                                  GtkWidget *, GtkAccelGroup *,
                                  GtkSignalFunc, gpointer);
extern void init_control_panel (displayd *, cpaneld *, gint);
extern splotd * splot_new (displayd *, gint, gint);
extern void display_reproject (displayd *);
extern void splot_free (splotd *);
extern void display_free (displayd *);
extern void splot_add_border (splotd *);
extern void varpanel_refresh ();
extern void splot_get_dimensions (splotd *, gint *, gint *);

void parcoords_reset_arrangement (displayd *, gint);
/* */

static GtkWidget *mbar;
static GtkAccelGroup *pc_accel_group;
static GtkWidget *arrangement_box;


static void
close_cb (displayd *display, guint action, GtkWidget *w) {
  display_free (display);
}
static void
delete_cb (GtkWidget *w, GdkEvent *event, displayd *display) {
  display_free (display);
}

static void
options_cb (gpointer data, guint action, GtkCheckMenuItem *w) {
  g_printerr ("action = %d\n", action);

  switch (action) {

    case 0:

    case 1:
    case 2:
  }
}

static void ash_smoothness_cb (GtkAdjustment *adj, gpointer cbd) {
  g_printerr ("%f\n", adj->value);
}

static gchar *arrangement_lbl[] = {"Row", "Column"};
static void arrangement_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", arrangement_lbl[indx]);

  if (indx != current_display->cpanel.parcoords_arrangement)
    parcoords_reset_arrangement (current_display, indx);

  current_display->cpanel.parcoords_arrangement = indx;
}

static gchar *type_lbl[] = {"Dotplot", "Texturing", "ASH"};
static void type_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  cpaneld *cpanel = &current_display->cpanel;

  g_printerr ("cbd: %s\n", type_lbl[indx]);

  switch (indx) {
    case 0:
      cpanel->p1d_type = DOTPLOT;
      break;
    case 1:
      cpanel->p1d_type = TEXTURE;
      break;
    case 2:
      cpanel->p1d_type = ASH;
      break;
  }

  display_reproject (current_display);
}

static gchar *selection_mode_lbl[] = {"Replace", "Insert", "Append"};
static void selection_mode_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  cpaneld *cpanel = &current_display->cpanel;

  g_printerr ("cbd: %s\n", selection_mode_lbl[indx]);

  switch (indx) {
    case 0:
      cpanel->parcoords_selection_mode = VAR_REPLACE;
      break;
    case 1:
      cpanel->parcoords_selection_mode = VAR_INSERT;
      break;
    case 2:
      cpanel->parcoords_selection_mode = VAR_APPEND;
      break;
  }
}

static gchar *showcases_lbl[] = {"All", "Labelled"};
static void showcases_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", showcases_lbl[indx]);
}

static gchar *varscale_lbl[] = {"Common", "Independent"};
static void varscale_cb (GtkWidget *w, gpointer cbd)
{
  gint indx = GPOINTER_TO_INT (cbd);
  g_printerr ("cbd: %s\n", varscale_lbl[indx]);
}

void
cpanel_parcoords_make () {
  GtkWidget *vbox, *vb, *lbl, *sbar, *opt;
  GtkObject *adj;
  
  control_panel[PCPLOT] = gtk_vbox_new (false, VBOX_SPACING);
  gtk_container_set_border_width (GTK_CONTAINER (control_panel[PCPLOT]), 5);

/*
 * arrangement of plots, row or column
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Plot arrangement:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "When opening a new parallel coordinates display, arrange the 1d plots in a row or a column",
    NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, arrangement_lbl,
                        sizeof (arrangement_lbl) / sizeof (gchar *),
                        arrangement_cb);
/*
 * option menu: selection mode
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vb, false, false, 0);

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

/*
 * option menu
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vb, false, false, 0);

  lbl = gtk_label_new ("Spreading method:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Display either textured dot plots or average shifted histograms", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, type_lbl,
                        sizeof (type_lbl) / sizeof (gchar *),
                        type_cb);
/*
 * ASH smoothness
*/
  vbox = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (control_panel[PCPLOT]), vbox,
    false, false, 0);

  lbl = gtk_label_new ("ASH smoothness:"),
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), lbl, false, false, 0);

  adj = gtk_adjustment_new (0.1, 0.0, 0.5, 0.01, .01, 0.0);
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
                      GTK_SIGNAL_FUNC (ash_smoothness_cb), NULL);

  sbar = gtk_hscale_new (GTK_ADJUSTMENT (adj));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sbar,
    "Adjust ASH smoothness", NULL);
  gtk_range_set_update_policy (GTK_RANGE (sbar), GTK_UPDATE_CONTINUOUS);
  gtk_scale_set_value_pos (GTK_SCALE (sbar), GTK_POS_BOTTOM);
  gtk_scale_set_digits (GTK_SCALE (sbar), 2);

  gtk_box_pack_start (GTK_BOX (vbox), sbar,
    false, false, 1);

/*
 * show cases: label and option menu
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Show cases:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Show all visible cases, or show only labelled cases", NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, showcases_lbl,
                        sizeof (showcases_lbl) / sizeof (gchar *),
                        showcases_cb);

/*
 * Variable scales
*/
  vb = gtk_vbox_new (false, 0);
  gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

  lbl = gtk_label_new ("Scales:");
  gtk_misc_set_alignment (GTK_MISC (lbl), 0, 0.5);
  gtk_box_pack_start (GTK_BOX (vb), lbl, false, false, 0);

  opt = gtk_option_menu_new ();
  gtk_container_set_border_width (GTK_CONTAINER (opt), 4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), opt,
    "Scale variables (and variable groups) on a common scale, or independently",
     NULL);
  gtk_box_pack_start (GTK_BOX (vb), opt, false, false, 0);
  populate_option_menu (opt, varscale_lbl,
                        sizeof (varscale_lbl) / sizeof (gchar *),
                        varscale_cb);

  gtk_widget_show_all (control_panel[PCPLOT]);
}

void
cpanel_parcoords_init (cpaneld *cpanel) {
  cpanel->parcoords_selection_mode = VAR_REPLACE;
  cpanel->parcoords_arrangement = ARRANGE_ROW;
}

static GtkItemFactoryEntry menu_items[] = {
  { "/_FFile",         NULL,         NULL, 0, "<Branch>" },
  { "/FFile/Print",  
         "",         NULL,        0, "<Item>" },
  { "/FFile/sep",   NULL,     NULL,          0, "<Separator>" },
  { "/FFile/Close",  
         "",         close_cb,        0, "<Item>" },

  { "/_Options",      NULL,         NULL, 0, "<Branch>" },
  { "/Options/Double buffer",
         "",          options_cb,           1, "<CheckItem>" },

  { "/Options/sep",   NULL,     NULL,          0, "<Separator>" },
  { "/Options/Show lines",  
         "",          options_cb,           2, "<CheckItem>" },
  { "/Options/Show points",  
         "" ,         options_cb,           3, "<CheckItem>" },
  { "/Options/Show missings",  
         "" ,         options_cb,           4, "<CheckItem>" },

  { "/Options/sep",   NULL,     NULL,          0, "<Separator>" },
  { "/Options/Use short variable label",  
         "" ,         options_cb,           5, "<CheckItem>" },
  { "/Options/Display vertical axes",  
         "" ,         options_cb,           6, "<CheckItem>" },
  { "/Options/Display horizontal gridlines",  
         "" ,         options_cb,           7, "<CheckItem>" },

  { "/_Link/",  "" ,   NULL, 1, "<Branch>" },
  { "/Link/Link parallel coordinates plot",  
         "",          NULL,        0, "<CheckItem>" },
};

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
parcoords_new () {
  GtkWidget *vbox, *frame;
  gint i;
  splotd *sp;
  gint nplots = MIN (xg.ncols, MAXNPCPLOTS);

  displayd *display = (displayd *) g_malloc (sizeof (displayd));
  display->displaytype = parcoords;

  /* create a row of vertical plots by default */
  display->p1d_orientation = VERTICAL;

  init_control_panel (display, &display->cpanel, PCPLOT);

  display->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (display->window),
                       "displayd",
                       (gpointer) display);
  gtk_window_set_policy (GTK_WINDOW (display->window), true, true, true);
  gtk_container_set_border_width (GTK_CONTAINER (display->window), 3);
  gtk_signal_connect (GTK_OBJECT (display->window), "delete_event",
                      GTK_SIGNAL_FUNC (delete_cb), (gpointer) display);


/*
 * Add the main menu bar
*/
  vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (vbox), 1);
  gtk_container_add (GTK_CONTAINER (display->window), vbox);

  pc_accel_group = gtk_accel_group_new ();
  get_main_menu (menu_items, sizeof (menu_items) / sizeof (menu_items[0]),
                 pc_accel_group, display->window, &mbar, (gpointer) display);
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

  display->splots = null;

  for (i=0; i<nplots; i++) {
    sp = splot_new (display, WIDTH, HEIGHT);
    /*
     * instead of maintaining a list, each plot knows its variable
    */
    sp->p1dvar = i; 
    display->splots = g_list_append (display->splots, (gpointer) sp);
    gtk_box_pack_start (GTK_BOX (arrangement_box), sp->da, true, true, 0);
    gtk_widget_show (sp->da);
  }

  gtk_widget_show_all (display->window);

  return display;
}

/* debugging */
/*
      g_printerr ("s->p1dvar: ");
      l = current_display->splots;
      while (l) {
        s = (splotd *) l->data;
        g_printerr ("%d ", s->p1dvar);
        l = l->next;
      }
      g_printerr ("\n");
*/
/* */

gboolean
parcoords_select_variable (cpaneld *cpanel, splotd *sp,
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
  splotd *jvar_sp = null;

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
    if (jvar_sp != null && nplots > 1) {
      /*
       * Delete the plot from the list, and destroy it.
      */
      current_display->splots = g_list_remove (current_display->splots,
                                               (gpointer) jvar_sp);

      /*
       * keep the window from shrinking by growing all plots
      */
      ratio = (gfloat) nplots / (gfloat) (nplots-1);
      if (cpanel->parcoords_arrangement == ARRANGE_ROW)
        width = (gint) (ratio * (gfloat) width);
      else
        height = (gint) (ratio * (gfloat) height);

      l = current_display->splots;
      while (l) {
        w = ((splotd *) l->data)->da;
        gtk_widget_ref (w);
        /* shrink each plot */
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
        new_indx = (jvar_indx == 0) ? 0 : MIN (nplots-1, jvar_indx);
        current_splot = (splotd *)
          g_list_nth_data (current_display->splots, new_indx);
        /* just for insurance, to handle the unforeseen */
        if (current_splot == null) 
          current_splot = (splotd *)
            g_list_nth_data (current_display->splots, 0);
      }

      splot_free (jvar_sp);

      nplots--;
      redraw = false;
    }

  } else {

    if (cpanel->parcoords_selection_mode == VAR_REPLACE) {

      *jvar_prev = sp->p1dvar;
      sp->p1dvar = jvar;
      redraw = true;

    } else {

      /*
       * keep the window from growing by shrinking all plots
      */
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
    true, true, true);

  return redraw;
}

/**********************************************************************/
/******************* Resetting the main menubar ***********************/
/**********************************************************************/

GtkWidget *parcoords_mode_menu;


void
make_parcoords_menus (GtkAccelGroup *accel_group, GtkSignalFunc func) {

/*
 * I/O menu
*/
  parcoords_mode_menu = gtk_menu_new ();

  CreateMenuItem (parcoords_mode_menu, "Parallel Coordinates",
    "^c", "", NULL, accel_group, func, GINT_TO_POINTER (PCPLOT));

  /* Add a separator */
  CreateMenuItem (parcoords_mode_menu, NULL,
    "", "", NULL, NULL, NULL, NULL);

  CreateMenuItem (parcoords_mode_menu, "Brush",
    "^b", "", NULL, accel_group, func, GINT_TO_POINTER (BRUSH));
  CreateMenuItem (parcoords_mode_menu, "Identify",
    "^i", "", NULL, accel_group, func, GINT_TO_POINTER (IDENT));
  CreateMenuItem (parcoords_mode_menu, "Move Points",
    "^m", "", NULL, accel_group, func, GINT_TO_POINTER (MOVEPTS));

  gtk_widget_show (parcoords_mode_menu);
}

/**********************************************************************/
/******************* End of main menubar section **********************/
/**********************************************************************/
