/* sphere_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/
/*                   routines for manipulating the gui                     */
/*-------------------------------------------------------------------------*/

void sphere_enable (gboolean sens, ggobid* gg)
{
  if (gg->sphere_ui.apply_btn != NULL) {
    gtk_widget_set_sensitive (gg->sphere_ui.apply_btn, sens);
  }
}

void sphere_condnum_set (gfloat x, ggobid* gg)
{
  if (gg->sphere_ui.condnum_entry != NULL) {
    gchar *lbl = g_strdup_printf ("%5.1f", x);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.condnum_entry), lbl);
    g_free (lbl);
  }
}

void sphere_totvar_set (gfloat x, datad *d, ggobid* gg)
{
  if (gg->sphere_ui.totvar_entry != NULL) {
    gchar *lbl = g_strdup_printf ("%.2e", x);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.totvar_entry), lbl);
    g_free (lbl);
  }
}

/*-------------------------------------------------------------------------*/
/*                          callbacks                                      */
/*-------------------------------------------------------------------------*/

static void
deleteit (ggobid *gg) {
  gtk_widget_hide (gg->sphere_ui.window);

  gdk_pixmap_unref (gg->sphere_ui.scree_pixmap);
  gtk_widget_destroy (gg->sphere_ui.window);

  gg->sphere_ui.window = NULL;
  gg->sphere_ui.scree_da = NULL;
  gg->sphere_ui.scree_pixmap = NULL;
  gg->sphere_ui.condnum_entry = NULL;
  gg->sphere_ui.totvar_entry = NULL;
  gg->sphere_ui.apply_btn = NULL;
}

void
sphere_npcs_set_cb (GtkAdjustment *adj, ggobid *gg) 
{
  gint n = (gint) adj->value;
  datad *d = gg->current_display->d;

  sphere_npcs_set (n, d, gg);
}

static void
delete_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
  deleteit (gg);
}

static void
sphere_apply_cb (GtkWidget *w, ggobid *gg) { 
/*
 * finally, sphere the number of principal components selected;
 * executed when the apply button is pressed
*/
  gint j;
  datad *d = gg->current_display->d;
  gfloat firstpc = d->sphere.eigenval[0];
  gfloat lastpc = d->sphere.eigenval[d->sphere.npcs-1];

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened the tool window.
  */
  if (gg->sphere_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  if (d->sphere.npcs > 0 && d->sphere.npcs <= d->sphere.nvars)
  {
    if (lastpc == 0.0 || firstpc/lastpc > 10000.0) {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      spherize_data (d->sphere.npcs, d->sphere.nvars, d->sphere.vars, d, gg);
      sphere_varcovar_set (d, gg);
/*    pc_axes_sensitive_set (true);*/


      /*-- these three lines replicated from transform.c --*/
      vartable_lim_update (d, gg);
      tform_to_world (d, gg);
      displays_tailpipe (REDISPLAY_PRESENT, gg);
    }
  }
}

/*
 * reset the scree plot when the number of identify of the selected
 * variables has changed
*/
static void
sphere_reset_cb (GtkWidget *w, ggobid *gg) { 
  scree_plot_make (gg);
}

/*
 * close the window
*/
static void
sphere_close_cb (GtkWidget *w, ggobid *gg) { 
  deleteit (gg);
}

/*-------------------------------------------------------------------------*/
/*                         Scree plot                                      */
/*-------------------------------------------------------------------------*/

#define SCREE_WIDTH 200
#define SCREE_HEIGHT 100

gboolean scree_mapped_p (ggobid *gg) {
  return (gg->sphere_ui.scree_da != NULL);
}

static gint
scree_configure_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  if (gg->sphere_ui.scree_pixmap == NULL) {
    gg->sphere_ui.scree_pixmap = gdk_pixmap_new (w->window,
      w->allocation.width, w->allocation.height, -1);
  }

  return false;
}
static gint
scree_expose_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  gint j;
  gint xpos, ypos, xstrt, ystrt;
  gchar *tickmk;
  GtkStyle *style = gtk_widget_get_style (gg->sphere_ui.scree_da);
  datad *d = gg->current_display->d;

  gint *sphvars = (gint *) g_malloc (d->ncols * sizeof (gint));
  gfloat *evals = (gfloat *) g_malloc (d->ncols * sizeof (gfloat));

  CHECK_GG (gg);

  eigenvals_get (evals, d, gg);

for (j=0; j<d->sphere.nvars; j++)
g_printerr ("(expose) sphvar %d eval %f\n", d->sphere.vars[j], evals[j]);

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (gg->sphere_ui.scree_pixmap, gg->plot_GC,
                      true, 0, 0,
                      w->allocation.width,
                      w->allocation.height);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  gdk_draw_line (gg->sphere_ui.scree_pixmap, gg->plot_GC, 10, 90, 190, 90);
  gdk_draw_line (gg->sphere_ui.scree_pixmap, gg->plot_GC, 10, 90, 10, 10);

  for (j=0; j<d->sphere.nvars; j++) {
    xpos = (gint) (180./(gfloat)(d->sphere.nvars-1)*j+10);
    ypos = (gint) (90. - evals[j]/evals[0]*80.);

    tickmk = g_strdup_printf ("%d", j+1);
    gdk_draw_string (gg->sphere_ui.scree_pixmap,
      style->font, gg->plot_GC, xpos, 95, tickmk);
    g_free (tickmk);

    if (j>0) 
      gdk_draw_line (gg->sphere_ui.scree_pixmap,
        gg->plot_GC, xstrt, ystrt, xpos, ypos);

    xstrt = xpos;
    ystrt = ypos;
  }

  gdk_draw_pixmap (w->window, gg->plot_GC, gg->sphere_ui.scree_pixmap,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);

  g_free ((gpointer) sphvars);
  g_free ((gpointer) evals);

  return false;
}

/*
 * Calculate the svd and display results in a scree plot
 * Called when the sphere panel is opened, and when the reset
 * button is pressed.
*/
void scree_plot_make (ggobid *gg)
{
  datad *d = gg->current_display->d;

  if (pca_calc (d, gg)) {  /*-- spherevars_set is called here --*/
    gboolean rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (gg->sphere_ui.scree_da),
      "expose_event", (gpointer) gg, (gpointer) &rval);
    pca_diagnostics_set (d, gg);
  } else {
     quick_message ("Variance-covariance is identity already!\n", false);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Create and map the sphere panel                     */
/*-------------------------------------------------------------------------*/

void
sphere_panel_open (ggobid *gg)
{
  GtkWidget *frame0, *main_vbox, *vbox, *frame;
  GtkWidget *label;
  GtkWidget *spinner;
  datad *d = gg->current_display->d;

  spherevars_set (d, gg);
/*
  gint nvars;
  nvars = nspherevars_get (d, gg);
*/

  if (gg->sphere_ui.window == NULL) {
    gg->sphere_ui.d = d;

    gg->sphere_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->sphere_ui.window),
      "sphere variables");
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.window), "delete_event",
                        GTK_SIGNAL_FUNC (delete_cb), (gpointer) gg);
    gtk_container_set_border_width (GTK_CONTAINER (gg->sphere_ui.window), 10);

    /*-- partition the screen vertically: scree plot, choose nPCs, apply --*/
    main_vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->sphere_ui.window), main_vbox);

    /*-- element 1: reset scree plot when n selected vars changes --*/
    gg->sphere_ui.reset_btn = gtk_button_new_with_label ("Reset scree plot");
    gtk_box_pack_start (GTK_BOX (main_vbox), gg->sphere_ui.reset_btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.reset_btn,
      "Reset scree plot when the selected variables have been reset",
      NULL);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.reset_btn), "clicked",
                        GTK_SIGNAL_FUNC (sphere_reset_cb), gg);

    /*-- element 2 of main_vbox: scree plot --*/
    frame = gtk_frame_new ("Scree plot");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    gtk_box_pack_start (GTK_BOX (main_vbox), frame, true, true, 2);

    /*-- stick a box in here so we can control the border width --*/
    vbox = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
    gtk_container_add (GTK_CONTAINER (frame), vbox);
 
    gg->sphere_ui.scree_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->sphere_ui.scree_da),
      SCREE_WIDTH, SCREE_HEIGHT);
    gtk_box_pack_start (GTK_BOX (vbox), gg->sphere_ui.scree_da,
                        false, false, 1);

    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.scree_da),
                        "expose_event",
                        (GtkSignalFunc) scree_expose_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.scree_da),
                        "configure_event",
                        (GtkSignalFunc) scree_configure_cb,
                        (gpointer) gg);

    /*-- element 3 of main_vbox: controls in a labelled frame --*/
    frame0 = gtk_frame_new ("Specify number of PCs");
    gtk_frame_set_shadow_type (GTK_FRAME (frame0), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (main_vbox), frame0, true, false, 1);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
    gtk_container_add (GTK_CONTAINER (frame0), vbox);

    /* Spinner: number of principal components */
    /*-- the parameters of the adjustment should be reset each time --*/
    gg->sphere_ui.npcs_adj = gtk_adjustment_new ((gfloat) d->sphere.nvars,
       1.0, (gfloat) d->ncols, 1.0, 5.0, 0.0);

    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.npcs_adj),
                        "value_changed",
                        GTK_SIGNAL_FUNC (sphere_npcs_set_cb),
                        gg);

    spinner = gtk_spin_button_new (GTK_ADJUSTMENT (gg->sphere_ui.npcs_adj),
                                   0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Specify the number of variables to be sphered",
      NULL);
    gtk_box_pack_start (GTK_BOX (vbox), spinner, true, true, 0);

    /*-- total variance --*/
    label = gtk_label_new ("Total variance:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, false, false, 0);

    gg->sphere_ui.totvar_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (gg->sphere_ui.totvar_entry), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.totvar_entry,
      "The percentage of variance accounted for by the selected variables",
      NULL);
    gtk_box_pack_start (GTK_BOX (vbox), gg->sphere_ui.totvar_entry,
      true, true, 2);
    gtk_widget_show (gg->sphere_ui.totvar_entry);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.totvar_entry), "-");

    /*-- condition number --*/
    label = gtk_label_new ("Condition number:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vbox), label, false, false, 0);

    gg->sphere_ui.condnum_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (gg->sphere_ui.condnum_entry), false);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.condnum_entry), "-");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.condnum_entry,
      "The condition number for the selected variables",
      NULL);
    gtk_box_pack_start (GTK_BOX (vbox), gg->sphere_ui.condnum_entry,
      true, true, 2);
    gtk_widget_show (gg->sphere_ui.condnum_entry);


    /*-- last: after choosing nPCs, the apply button --*/
    gg->sphere_ui.apply_btn = gtk_button_new_with_label ("Apply sphering");
    gtk_box_pack_start (GTK_BOX (main_vbox), gg->sphere_ui.apply_btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.apply_btn,
      "Apply principal components transformation to the first n selected variables",
      NULL);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.apply_btn), "clicked",
                        GTK_SIGNAL_FUNC (sphere_apply_cb), gg);

    gg->sphere_ui.close_btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (main_vbox), gg->sphere_ui.close_btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.close_btn,
      "Close the sphering window", NULL);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.close_btn), "clicked",
                        GTK_SIGNAL_FUNC (sphere_close_cb), gg);
  }

  gtk_widget_show_all (gg->sphere_ui.window);
  gdk_flush ();

  scree_plot_make (gg);
}
