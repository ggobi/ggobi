/* sphere_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

extern void spherevars_set (datad *, ggobid *);
extern void sphere_varcovar_set (datad *, ggobid *);
extern void spherize_data (vector_i *svars, vector_i *pcvars, datad *, ggobid *);

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

void sphere_variance_set (gfloat x, datad *d, ggobid* gg)
{
  if (gg->sphere_ui.variance_entry != NULL) {
    gchar *lbl = g_strdup_printf ("%.2e", x);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.variance_entry), lbl);
    g_free (lbl);
  }
}

/*-------------------------------------------------------------------------*/
/*                          callbacks                                      */
/*-------------------------------------------------------------------------*/

static gint
sphere_clist_size_alloc_cb (GtkWidget *w, GdkEvent *event, ggobid *gg)
{
  if (!widget_initialized (w)) {
    gint fheight;
    gint lbearing, rbearing, width, ascent, descent;
    GtkStyle *style;
    GtkCList *clist = GTK_CLIST (w);
    style = gtk_widget_get_style (w);
    gdk_text_extents (style->font,
      "arbitrary string", strlen ("arbitrary string"),
      &lbearing, &rbearing, &width, &ascent, &descent);
    fheight = ascent + descent;

    gtk_widget_set_usize (w, -1,
      4*fheight + 3*1 +  /*-- 1 = CELL_SPACING --*/
      clist->column_title_area.height);

    widget_initialize (w, true);
  }

  return true;
}

static void
deleteit (ggobid *gg) {
  GSList *l;

  gtk_widget_hide (gg->sphere_ui.window);

  for (l=gg->d; l; l=l->next)
    sphere_free ((datad *) l->data);

  gdk_pixmap_unref (gg->sphere_ui.scree_pixmap);
  gtk_widget_destroy (gg->sphere_ui.window);

  gg->sphere_ui.window = NULL;
  gg->sphere_ui.scree_da = NULL;
  gg->sphere_ui.scree_pixmap = NULL;
  gg->sphere_ui.condnum_entry = NULL;
  gg->sphere_ui.variance_entry = NULL;
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
vars_stdized_cb (GtkWidget *w, GdkEvent *event, datad *d)
{
  gboolean stdized = true;
  gint el, k;

  for (k=0; k<d->sphere.vars.nels; k++) {
    el = d->sphere.vars.vals[k];
    if (d->vartable[el].tform2 != STANDARDIZE) {
      stdized = false;
      break;
    }
  }

  gtk_entry_set_text (GTK_ENTRY (w), (stdized) ? "yes" : "no");
}

void
vars_stdized_send_event (datad *d, ggobid *gg)
{
  if (gg->sphere_ui.stdized_entry != NULL &&
      GTK_WIDGET_VISIBLE (gg->sphere_ui.stdized_entry))
  {
    gboolean rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (gg->sphere_ui.stdized_entry),
      "expose_event", (gpointer) d, (gpointer) &rval);
  }
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
  datad *d = gg->current_display->d;
  gfloat firstpc = d->sphere.eigenval.vals[0];
  gfloat lastpc = d->sphere.eigenval.vals[d->sphere.npcs-1];

  /* 
   * if datad has changed, refuse to do anything until the
   * user has closed and reopened the tool window.
  */
  if (gg->sphere_ui.d != d) {
    g_printerr ("Close and reopen this window, please\n");
    return;
  }

  if (d->sphere.npcs > 0 && d->sphere.npcs <= d->sphere.vars.nels)
  {
    if (lastpc == 0.0 || firstpc/lastpc > 10000.0) {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      /*-- set up the variables into which sphered data will be written --*/
      extern void spherize_set_pcvars (datad *, ggobid *);
      spherize_set_pcvars (d, gg);

      /*
       * sphere the variables in d->sphere.vars
       * into the variables in d->sphere.pcvars
      */
      spherize_data (&d->sphere.vars, &d->sphere.pcvars, d, gg);
      sphere_varcovar_set (d, gg);
/*    pc_axes_sensitive_set (true);*/

      /*-- these three lines replicated from transform.c --*/
      limits_set (false, true, d);
      vartable_limits_set (d);
      vartable_stats_set (d);

      tform_to_world (d, gg);
      displays_tailpipe (REDISPLAY_PRESENT, gg);
    }
  }
}

static void
scree_restore_cb (GtkWidget *w, datad *d)
{ 
  extern void sphere_malloc (gint, datad *, ggobid *);

  if (d->sphere.vars_sphered.nels > 0) {
    ggobid *gg = GGobiFromWidget (w, true);
    gint ncols = d->sphere.vars_sphered.nels;

    if (d->sphere.vars.vals == NULL || d->sphere.vars.nels != ncols) {
      sphere_malloc (ncols, d, gg);
    }
  
    vectori_copy (&d->sphere.vars_sphered, &d->sphere.vars);  /* from, to */

    /*-- update the "vars stdized?" text entry --*/
    vars_stdized_send_event (d, gg);

    scree_plot_make (d, gg);

  }  else {
    g_printerr ("sorry, there are no sphered variables to use\n");
  }
}

/*
 * update the scree plot when the number or identify of the selected
 * variables has changed, or after the variables are transformed
*/
static void
scree_update_cb (GtkWidget *w, datad *d)
{ 
  ggobid *gg = GGobiFromWidget (w, true);
  spherevars_set (d, gg);
  scree_plot_make (d, gg);
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
  if (gg->sphere_ui.scree_pixmap != NULL)
    gdk_pixmap_unref (gg->sphere_ui.scree_pixmap);

  gg->sphere_ui.scree_pixmap = gdk_pixmap_new (w->window,
    w->allocation.width, w->allocation.height, -1);

  return false;
}
static gint
scree_expose_cb (GtkWidget *w, GdkEventConfigure *event, ggobid *gg)
{
  gint margin=10;
  gint j;
  gint xpos, ypos, xstrt, ystrt;
  gchar *tickmk;
  GtkStyle *style = gtk_widget_get_style (gg->sphere_ui.scree_da);
  datad *d = gg->current_display->d;
  gint wid = w->allocation.width, hgt = w->allocation.height;

  gint *sphvars = (gint *) g_malloc (d->ncols * sizeof (gint));
  gfloat *evals = (gfloat *) g_malloc (d->ncols * sizeof (gfloat));
  gint nels;

  CHECK_GG (gg);

  eigenvals_get (evals, d);

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (gg->sphere_ui.scree_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  gdk_draw_line (gg->sphere_ui.scree_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (gg->sphere_ui.scree_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  nels = d->sphere.vars.nels;
  for (j=0; j<nels; j++) {
    xpos = (gint) (((gfloat) (wid - 2*margin))/(gfloat)(nels-1)*j+margin);
    ypos = (gint) (((gfloat) (hgt-margin)) - evals[j]/evals[0]*(hgt-2*margin));

    tickmk = g_strdup_printf ("%d", j+1);
    gdk_draw_string (gg->sphere_ui.scree_pixmap,
      style->font, gg->plot_GC, xpos, hgt-margin/2, tickmk);
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
 * Called when the sphere panel is opened, and when the update
 * button is pressed.
*/
void scree_plot_make (datad *d, ggobid *gg)
{
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
  GtkWidget *frame0, *main_vbox, *vbox, *table, *frame, *hbox;
  GtkWidget *label;
  GtkWidget *spinner;
  datad *d = gg->current_display->d;
  /*-- for the clist of sphered variables --*/
  GtkWidget *scrolled_window;
  gchar *titles[1] = {"sphered variables"};
  /*-- --*/

  spherevars_set (d, gg); 

  if (gg->sphere_ui.window == NULL) {
    GtkWidget *btn;
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

    /*-- element 1: update scree plot when n selected vars changes --*/
    btn = gtk_button_new_with_label ("Update scree plot");
    GGobi_widget_set (btn, gg, true);
    gtk_box_pack_start (GTK_BOX (main_vbox), btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update scree plot when a new set of variables is selected, or when variables are transformed",
      NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scree_update_cb), d);

    hbox = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (main_vbox), hbox, false, false, 0);

    gtk_box_pack_start (GTK_BOX (hbox),
      gtk_label_new ("Variables standardized?"),
      false, false, 0);
    gg->sphere_ui.stdized_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (gg->sphere_ui.stdized_entry), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.stdized_entry,
      "Have all the selected variables been standardized?  (To standardize, use Variable transformation, Stage 2, then update the scree plot)",
      NULL);
    gtk_box_pack_start (GTK_BOX (hbox), gg->sphere_ui.stdized_entry,
      false, false, 0);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.stdized_entry), "-");
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.stdized_entry),
                        "expose_event",
                        (GtkSignalFunc) vars_stdized_cb,
                        (gpointer) d);

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
                        true, true, 1);

    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.scree_da),
                        "expose_event",
                        (GtkSignalFunc) scree_expose_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.scree_da),
                        "configure_event",
                        (GtkSignalFunc) scree_configure_cb,
                        (gpointer) gg);

    /*-- element 3 of main_vbox: controls in a labelled frame --*/
    frame0 = gtk_frame_new ("Prepare to sphere");
    gtk_frame_set_shadow_type (GTK_FRAME (frame0), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (main_vbox), frame0, false, false, 1);

    table = gtk_table_new (3, 2, false);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);
    gtk_container_add (GTK_CONTAINER (frame0), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 4);

    /*-- current variance --*/
    label = gtk_label_new ("Set number of PCs");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label,
      0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

    /* Spinner: number of principal components */
    /*-- the parameters of the adjustment should be reset each time --*/
    gg->sphere_ui.npcs_adj = gtk_adjustment_new ((gfloat) d->sphere.vars.nels,
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
      "Specify the number of principal components",
      NULL);
    gtk_table_attach (GTK_TABLE (table), spinner,
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

    /*-- total variance --*/
    label = gtk_label_new ("Variance");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label,
      0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

    gg->sphere_ui.variance_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (gg->sphere_ui.variance_entry), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.variance_entry,
      "The percentage of variance accounted for by the first n principal components",
      NULL);
    gtk_widget_show (gg->sphere_ui.variance_entry);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.variance_entry), "-");

    gtk_table_attach (GTK_TABLE (table), gg->sphere_ui.variance_entry, 
      1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

    /*-- condition number --*/
    label = gtk_label_new ("Condition number");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label,
      0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    gg->sphere_ui.condnum_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (gg->sphere_ui.condnum_entry), false);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.condnum_entry), "-");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.condnum_entry,
      "The condition number for the specified number of principal components",
      NULL);
    gtk_table_attach (GTK_TABLE (table), gg->sphere_ui.condnum_entry, 
      1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    frame = gtk_frame_new ("Sphere");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    gtk_box_pack_start (GTK_BOX (main_vbox), frame, false, false, 2);

    vbox = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
    gtk_container_add (GTK_CONTAINER (frame), vbox);

    /*-- last: after choosing nPCs, the apply button --*/
    gg->sphere_ui.apply_btn = gtk_button_new_with_label ("Apply sphering");
    gtk_box_pack_start (GTK_BOX (vbox), gg->sphere_ui.apply_btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.apply_btn,
      "Apply principal components transformation to the selected variables",
      NULL);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.apply_btn), "clicked",
                        GTK_SIGNAL_FUNC (sphere_apply_cb), gg);

    /*-- list to show the currently sphered variables --*/
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_box_pack_start (GTK_BOX (vbox), scrolled_window,
      true, true, 0);

    gg->sphere_ui.clist = gtk_clist_new_with_titles (1, titles);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.clist),
                        "size_allocate",
                        (GtkSignalFunc) sphere_clist_size_alloc_cb,
                        (gpointer) gg);
    gtk_clist_column_titles_passive (GTK_CLIST (gg->sphere_ui.clist));
    widget_initialize (gg->sphere_ui.clist, false);

    gtk_container_add (GTK_CONTAINER (scrolled_window),
      gg->sphere_ui.clist);
    /*-- --*/

    gg->sphere_ui.restore_btn = gtk_button_new_with_label ("Restore scree plot");
    GGobi_widget_set (gg->sphere_ui.restore_btn, gg, true);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.restore_btn,
      "Restore the scree plot to reflect the current principal components",
      NULL);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.restore_btn), "clicked",
                        GTK_SIGNAL_FUNC (scree_restore_cb), d);
    gtk_box_pack_start (GTK_BOX (vbox), gg->sphere_ui.restore_btn,
      false, false, 0);

    /*-- close button --*/
    btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (main_vbox), btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the sphering window", NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (sphere_close_cb), gg);
  }

  gtk_widget_show_all (gg->sphere_ui.window);
  gdk_flush ();

  scree_plot_make (d, gg);
}
