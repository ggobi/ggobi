/* sphere_ui.c */

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/

datad *
datad_get_from_window (GtkWidget *window)
{
  datad *d = NULL;
  GtkWidget *clist;

  if (window != NULL) {
    clist = get_clist_from_object (GTK_OBJECT(window));
    if (clist != NULL)
      d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  }

  return d;
}

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

/*-- reset the spinner max and value --*/
void sphere_npcs_range_set (gint n, ggobid *gg)
{
  if (gg->sphere_ui.npcs_adj != NULL) {
    GTK_ADJUSTMENT(gg->sphere_ui.npcs_adj)->upper = (gfloat) n;
    gtk_adjustment_set_value (GTK_ADJUSTMENT(gg->sphere_ui.npcs_adj),
      (gfloat) n);
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
    gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
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
  gg->sphere_ui.stdized_entry = NULL;
  gg->sphere_ui.apply_btn = NULL;
  gg->sphere_ui.npcs_adj = (GtkObject *) NULL;
}

void
sphere_npcs_set_cb (GtkAdjustment *adj, ggobid *gg) 
{
  gint n = (gint) adj->value;
  datad *d = datad_get_from_window (gg->sphere_ui.window);

  if (d != NULL)
    sphere_npcs_set (n, d, gg);
}

static void
vars_stdized_cb (GtkToggleButton *btn, ggobid *gg)
{
  datad *d = datad_get_from_window (gg->sphere_ui.window);

  d->sphere.vars_stdized = btn->active;
}

void
vars_stdized_send_event (datad *d, ggobid *gg)
{
  if (gg->sphere_ui.stdized_entry != NULL &&
      GTK_IS_WIDGET (gg->sphere_ui.stdized_entry) &&
      GTK_WIDGET_VISIBLE (gg->sphere_ui.stdized_entry))
  {
    gboolean rval = false;

    gtk_signal_emit_by_name (GTK_OBJECT (gg->sphere_ui.stdized_entry),
      "expose_event", (gpointer) d, (gpointer) &rval);
  }
}


static void
sphere_apply_cb (GtkWidget *w, ggobid *gg) { 
/*
 * finally, sphere the number of principal components selected;
 * executed when the apply button is pressed
*/
  gfloat firstpc, lastpc;
  datad *d = datad_get_from_window (gg->sphere_ui.window);

  if (d == NULL) return;
  
  firstpc = d->sphere.eigenval.els[0];
  lastpc = d->sphere.eigenval.els[d->sphere.npcs-1];

  if (d->sphere.npcs > 0 && d->sphere.npcs <= d->sphere.vars.nels) {
    if (lastpc == 0.0 || firstpc/lastpc > 10000.0) {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      /*-- set up the variables into which sphered data will be written --*/
      if (spherize_set_pcvars (d, gg)) {

        /*
         * sphere the variables in d->sphere.vars
         * into the variables in d->sphere.pcvars
        */
        spherize_data (&d->sphere.vars, &d->sphere.pcvars, d, gg);
        sphere_varcovar_set (d, gg);
/*      pc_axes_sensitive_set (true);*/

        /*-- these three lines replicated from transform.c --*/
        limits_set (false, true, d, gg);
        vartable_limits_set (d);
        vartable_stats_set (d);

        tform_to_world (d, gg);
        displays_tailpipe (FULL, gg);
      }
    }
  }
}

/*
static void
scree_restore_cb (GtkWidget *w, ggobid *gg)
{ 
  extern void sphere_malloc (gint, datad *, ggobid *);
  datad *d = datad_get_from_window (gg->sphere_ui.window);

  if (d != NULL && d->sphere.vars_sphered.nels > 0) {
    gint ncols = d->sphere.vars_sphered.nels;

    if (d->sphere.vars.els == NULL || d->sphere.vars.nels != ncols) {
      sphere_malloc (ncols, d, gg);
    }
  
    vectori_copy (&d->sphere.vars_sphered, &d->sphere.vars);
    vars_stdized_send_event (d, gg);
    scree_plot_make (gg);

  }  else {
    g_printerr ("sorry, there are no sphered variables to use\n");
  }
}
*/

/*
 * update the scree plot when the number or identity of the selected
 * variables has changed, or after the variables are transformed
*/
static void
scree_update_cb (GtkWidget *w, datad *d)
{ 
  ggobid *gg = GGobiFromWidget (w, true);
  spherevars_set (gg);
  scree_plot_make (gg);
}

/*-- called when closed from the close button --*/
static void
close_btn_cb (GtkWidget *w, ggobid *gg) { 
  deleteit (gg);
}
/*-- called when closed from the window manager --*/
static void
close_wmgr_cb (GtkWidget *w, GdkEvent *event, ggobid *gg) {
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
  datad *d = datad_get_from_window (gg->sphere_ui.window);
  gint wid = w->allocation.width, hgt = w->allocation.height;
  gint *sphvars, nels;
  gfloat *evals;
  colorschemed *scheme = gg->activeColorScheme;

  CHECK_GG (gg);

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (gg->sphere_ui.scree_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_line (gg->sphere_ui.scree_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (gg->sphere_ui.scree_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  if (d != NULL) {

    sphvars = (gint *) g_malloc (d->ncols * sizeof (gint));
    evals = (gfloat *) g_malloc (d->ncols * sizeof (gfloat));

    eigenvals_get (evals, d);

    nels = d->sphere.vars.nels;
    for (j=0; j<nels; j++) {
      xpos = (gint)(((gfloat) (wid - 2*margin))/(gfloat)(nels-1)*j+margin);
      ypos = (gint)(((gfloat) (hgt-margin)) - evals[j]/evals[0]*(hgt-2*margin));

      tickmk = g_strdup_printf ("%d", j+1);
      gdk_draw_string (gg->sphere_ui.scree_pixmap,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gg->plot_GC, xpos, hgt-margin/2, tickmk);
      g_free (tickmk);

      if (j>0) 
        gdk_draw_line (gg->sphere_ui.scree_pixmap,
          gg->plot_GC, xstrt, ystrt, xpos, ypos);

      xstrt = xpos;
      ystrt = ypos;
    }
    g_free ((gpointer) sphvars);
    g_free ((gpointer) evals);
  }

  gdk_draw_pixmap (w->window, gg->plot_GC, gg->sphere_ui.scree_pixmap,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
  return false;
}

/*
 * Calculate the svd and display results in a scree plot
 * Called when the sphere panel is opened, and when the update
 * button is pressed.
*/
void scree_plot_make (ggobid *gg)
{
  datad *d = datad_get_from_window (gg->sphere_ui.window);

  if (pca_calc (d, gg)) {  /*-- spherevars_set is called here --*/
    gboolean rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (gg->sphere_ui.scree_da),
      "expose_event", (gpointer) gg, (gpointer) &rval);
    pca_diagnostics_set (d, gg);
  } else {
     if (d->sphere.npcs > 0)
       quick_message ("Variance-covariance is identity already!\n", false);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Create and map the sphere panel                     */
/*-------------------------------------------------------------------------*/

void
sphere_panel_open (ggobid *gg)
{
  GtkWidget *frame0, *vbox, *vb, *hb, *table, *frame;
  GtkWidget *label;
  GtkWidget *spinner;
  datad *d;
  GtkWidget *notebook;
  /*-- for the clist of sphered variables --*/
  GtkWidget *scrolled_window;
  gchar *titles[1] = {"sphered variables"};
  /*-- --*/

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length (gg->d) == 0) 
/**/return;

/*
 * Maybe I'll change this, and leave all the following entries
 * and lists blank until variables are chosen in the new variable list.
*/
  if (gg->sphere_ui.window == NULL) {
    d = gg->current_display->d;
  } else {
    GtkWidget *clist = get_clist_from_object (GTK_OBJECT(gg->sphere_ui.window));
    d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  }

  spherevars_set (gg); 

  if (gg->sphere_ui.window == NULL) {
    GtkWidget *btn;

    gg->sphere_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->sphere_ui.window),
      "sphere variables");
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.window), "delete_event",
                        GTK_SIGNAL_FUNC (close_wmgr_cb), (gpointer) gg);
    gtk_container_set_border_width (GTK_CONTAINER (gg->sphere_ui.window),
      10);

    /*-- partition the screen vertically: scree plot, choose nPCs, apply --*/
    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->sphere_ui.window), vbox);

    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vbox, GTK_SELECTION_EXTENDED,
      (GtkSignalFunc) NULL, gg);

    /*-- use correlation matrix? --*/
    btn = gtk_check_button_new_with_label ("Use correlation matrix");
    gtk_widget_set_name (btn, "SPHERE:std_button");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "When this button is checked the correlation matrix is used to generate the PCs, otherwise the variance-covariance matrix is used",
      NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "toggled",
                        (GtkSignalFunc) vars_stdized_cb, (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (vbox), btn,
      true, true, 1);

    /*-- update scree plot when n selected vars changes --*/
    btn = gtk_button_new_with_label ("Update scree plot");
    GGobi_widget_set (btn, gg, true);
    gtk_box_pack_start (GTK_BOX (vbox), btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update scree plot when a new set of variables is selected, or when variables are transformed",
      NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (scree_update_cb), gg);

    /*-- scree plot --*/
    frame = gtk_frame_new ("Scree plot");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 2);

    /*-- stick a box in here so we can control the border width --*/
    vb = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
    gtk_container_add (GTK_CONTAINER (frame), vb);
 
    gg->sphere_ui.scree_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (gg->sphere_ui.scree_da),
      SCREE_WIDTH, SCREE_HEIGHT);
    gtk_box_pack_start (GTK_BOX (vb), gg->sphere_ui.scree_da,
                        true, true, 1);

    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.scree_da),
                        "expose_event",
                        (GtkSignalFunc) scree_expose_cb,
                        (gpointer) gg);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.scree_da),
                        "configure_event",
                        (GtkSignalFunc) scree_configure_cb,
                        (gpointer) gg);

    /*-- element 3 of vbox: controls in a labelled frame --*/
    frame0 = gtk_frame_new ("Prepare to sphere");
    gtk_frame_set_shadow_type (GTK_FRAME (frame0), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame0, false, false, 1);

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
       1.0, (gfloat) d->sphere.vars.nels, 1.0, 5.0, 0.0);

    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.npcs_adj),
                        "value_changed",
                        GTK_SIGNAL_FUNC (sphere_npcs_set_cb),
                        gg);

    spinner = gtk_spin_button_new (GTK_ADJUSTMENT (gg->sphere_ui.npcs_adj),
                                   0, 0);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
#if GTK_MAJOR_VERSION == 1
/*
 * The documentation suggests that this should still be present
 * in gtk2, but it isn't there.
*/
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (spinner),
                                     GTK_SHADOW_OUT);
#endif
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
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 2);

    vb = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
    gtk_container_add (GTK_CONTAINER (frame), vb);

    /*-- last: after choosing nPCs, the apply button --*/
    gg->sphere_ui.apply_btn = gtk_button_new_with_label ("Apply sphering");
    gtk_box_pack_start (GTK_BOX (vb), gg->sphere_ui.apply_btn,
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
    gtk_box_pack_start (GTK_BOX (vb), scrolled_window,
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

/*
 * Di and I decided there's no good reason to have this button.
    gg->sphere_ui.restore_btn = gtk_button_new_with_label ("Restore scree plot");
    GGobi_widget_set (gg->sphere_ui.restore_btn, gg, true);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.restore_btn,
      "Restore the scree plot to reflect the current principal components",
      NULL);
    gtk_signal_connect (GTK_OBJECT (gg->sphere_ui.restore_btn), "clicked",
                        GTK_SIGNAL_FUNC (scree_restore_cb), gg);
    gtk_box_pack_start (GTK_BOX (vb), gg->sphere_ui.restore_btn,
      false, false, 0);
*/

    /*-- close button --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
      false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_with_label ("Close");
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the sphering window", NULL);
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                        GTK_SIGNAL_FUNC (close_btn_cb), gg);

    gtk_object_set_data (GTK_OBJECT (gg->sphere_ui.window),
      "notebook", notebook);
  }

  gtk_widget_show_all (gg->sphere_ui.window);
  gdk_flush ();

  scree_plot_make (gg);
}
