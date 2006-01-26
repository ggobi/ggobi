/* sphere_ui.c */
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

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/

datad *
datad_get_from_window (GtkWidget *window)
{
  datad *d = NULL;
  GtkWidget *tree_view;

  if (window != NULL) {
    tree_view = get_tree_view_from_object (G_OBJECT(window));
    if (tree_view != NULL)
      d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");
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
#if 0
static gint
sphere_tree_view_size_alloc_cb (GtkWidget *w, GdkEvent *event, ggobid *gg)
{
  if (!widget_initialized (w)) {
    gint fheight;
    GtkTreeView *tree_view = GTK_TREE_VIEW (w);
    PangoContext *ctx = gtk_widget_get_pango_context(w);
	PangoFontMetrics *metrics = pango_context_get_metrics(ctx, 
		pango_context_get_font_description(ctx), NULL);
	gint ascent = PANGO_PIXELS(pango_font_metrics_get_ascent(metrics));
	gint descent = PANGO_PIXELS(pango_font_metrics_get_descent(metrics));
	/*gdk_text_extents (
      gtk_style_get_font (style),
      "arbitrary string", strlen ("arbitrary string"),
      &lbearing, &rbearing, &width, &ascent, &descent);
    */
    gtk_widget_set_usize (w, -1,
      4*(ascent+descent) + 3*1 +  /*-- 1 = CELL_SPACING --*/
      tree_view->column_title_area.height);

    widget_initialize (w, true);
	
	pango_font_metrics_unref(metrics);
  }

  return true;
}
#endif

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

    g_signal_emit_by_name (G_OBJECT (gg->sphere_ui.stdized_entry),
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
  if (d->sphere.eigenval.els == NULL) return;
  
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
  //GtkStyle *style = gtk_widget_get_style (gg->sphere_ui.scree_da);
  datad *d = datad_get_from_window (gg->sphere_ui.window);
  gint wid = w->allocation.width, hgt = w->allocation.height;
  gint *sphvars, nels;
  gfloat *evals;
  colorschemed *scheme = gg->activeColorScheme;
  PangoLayout *layout;

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
	  layout = gtk_widget_create_pango_layout(gg->sphere_ui.scree_da, tickmk);
	  gdk_draw_layout(gg->sphere_ui.scree_pixmap, gg->plot_GC, xpos, hgt-margin/2, layout);
      /*gdk_draw_string (gg->sphere_ui.scree_pixmap,
        gtk_style_get_font (style),
        gg->plot_GC, xpos, hgt-margin/2, tickmk);*/
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
    g_signal_emit_by_name (G_OBJECT (gg->sphere_ui.scree_da),
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
  /*-- for the tree_view of sphered variables --*/
  GtkWidget *scrolled_window;
  gchar *titles[1] = {"sphered variables"};
  GtkListStore *model;
  
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
    GtkWidget *tree_view = get_tree_view_from_object (G_OBJECT(gg->sphere_ui.window));
    d = (datad *) g_object_get_data(G_OBJECT (tree_view), "datad");
  }

  spherevars_set (gg); 

  if (gg->sphere_ui.window == NULL) {
    GtkWidget *btn;

    gg->sphere_ui.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (gg->sphere_ui.window),
      "sphere variables");
    g_signal_connect (G_OBJECT (gg->sphere_ui.window), "delete_event",
                        G_CALLBACK (close_wmgr_cb), (gpointer) gg);
    gtk_container_set_border_width (GTK_CONTAINER (gg->sphere_ui.window),
      10);

    /*-- partition the screen vertically: scree plot, choose nPCs, apply --*/
    vbox = gtk_vbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (gg->sphere_ui.window), vbox);

    /* Create a notebook, set the position of the tabs */
    notebook = create_variable_notebook (vbox,
      GTK_SELECTION_MULTIPLE, all_vartypes, all_datatypes,
      G_CALLBACK(NULL), NULL, gg);

    /*-- use correlation matrix? --*/
    btn = gtk_check_button_new_with_mnemonic ("Use _correlation matrix");
    gtk_widget_set_name (btn, "SPHERE:std_button");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "When this button is checked the correlation matrix is used to generate the PCs, otherwise the variance-covariance matrix is used",
      NULL);
    g_signal_connect (G_OBJECT (btn), "toggled",
                        G_CALLBACK(vars_stdized_cb), (gpointer) gg);
    gtk_box_pack_start (GTK_BOX (vbox), btn, false, false, 1);

    /*-- update scree plot when n selected vars changes --*/
    btn = gtk_button_new_with_mnemonic ("_Update scree plot");
    GGobi_widget_set (btn, gg, true);
    gtk_box_pack_start (GTK_BOX (vbox), btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Update scree plot when a new set of variables is selected, or when variables are transformed",
      NULL);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (scree_update_cb), gg);

    /*-- scree plot --*/
    frame = gtk_frame_new ("Scree plot");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 2);

    /*-- stick a box in here so we can control the border width --*/
    vb = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
    gtk_container_add (GTK_CONTAINER (frame), vb);
 
    gg->sphere_ui.scree_da = gtk_drawing_area_new ();
    gtk_widget_set_double_buffered(gg->sphere_ui.scree_da, false);
    gtk_widget_set_size_request(GTK_WIDGET (gg->sphere_ui.scree_da),
      SCREE_WIDTH, SCREE_HEIGHT);
    gtk_box_pack_start (GTK_BOX (vb), gg->sphere_ui.scree_da,
                        true, true, 1);

    g_signal_connect (G_OBJECT (gg->sphere_ui.scree_da),
                        "expose_event",
                        G_CALLBACK(scree_expose_cb),
                        (gpointer) gg);
    g_signal_connect (G_OBJECT (gg->sphere_ui.scree_da),
                        "configure_event",
                        G_CALLBACK(scree_configure_cb),
                        (gpointer) gg);

    /*-- element 3 of vbox: controls in a labelled frame --*/
    frame0 = gtk_frame_new ("Prepare to sphere");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame0), GTK_SHADOW_ETCHED_OUT);
    gtk_box_pack_start (GTK_BOX (vbox), frame0, false, false, 1);

    table = gtk_table_new (3, 2, false);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);
    gtk_container_add (GTK_CONTAINER (frame0), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 4);

    /*-- current variance --*/
    label = gtk_label_new_with_mnemonic ("Set number of _PCs");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label,
      0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

    /* Spinner: number of principal components */
    /*-- the parameters of the adjustment should be reset each time --*/
    gg->sphere_ui.npcs_adj = gtk_adjustment_new ((gfloat) d->sphere.vars.nels,
       1.0, (gfloat) d->sphere.vars.nels, 1.0, 5.0, 0.0);

    g_signal_connect (G_OBJECT (gg->sphere_ui.npcs_adj),
                        "value_changed",
                        G_CALLBACK (sphere_npcs_set_cb),
                        gg);

    spinner = gtk_spin_button_new (GTK_ADJUSTMENT (gg->sphere_ui.npcs_adj),
                                   0, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), spinner);
    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (spinner), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), spinner,
      "Specify the number of principal components",
      NULL);
    gtk_table_attach (GTK_TABLE (table), spinner,
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

    /*-- total variance --*/
    label = gtk_label_new_with_mnemonic ("_Variance");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label,
      0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

    gg->sphere_ui.variance_entry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), gg->sphere_ui.variance_entry);
    gtk_editable_set_editable (GTK_EDITABLE (gg->sphere_ui.variance_entry), false);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.variance_entry,
      "The percentage of variance accounted for by the first n principal components",
      NULL);
    gtk_widget_show (gg->sphere_ui.variance_entry);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.variance_entry), "-");

    gtk_table_attach (GTK_TABLE (table), gg->sphere_ui.variance_entry, 
      1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

    /*-- condition number --*/
    label = gtk_label_new_with_mnemonic ("Condition _number");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label,
      0, 1, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    gg->sphere_ui.condnum_entry = gtk_entry_new ();
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), gg->sphere_ui.condnum_entry);
    gtk_editable_set_editable (GTK_EDITABLE (gg->sphere_ui.condnum_entry), false);
    gtk_entry_set_text (GTK_ENTRY (gg->sphere_ui.condnum_entry), "-");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.condnum_entry,
      "The condition number for the specified number of principal components",
      NULL);
    gtk_table_attach (GTK_TABLE (table), gg->sphere_ui.condnum_entry, 
      1, 2, 2, 3, GTK_FILL, GTK_FILL, 0, 0);

    frame = gtk_frame_new ("Sphere");
    //gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    gtk_box_pack_start (GTK_BOX (vbox), frame, false, false, 2);

    vb = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 4);
    gtk_container_add (GTK_CONTAINER (frame), vb);

    /*-- last: after choosing nPCs, the apply button --*/
    gg->sphere_ui.apply_btn = gtk_button_new_with_mnemonic ("_Apply sphering");
    gtk_box_pack_start (GTK_BOX (vb), gg->sphere_ui.apply_btn,
      false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.apply_btn,
      "Apply principal components transformation to the selected variables",
      NULL);
    g_signal_connect (G_OBJECT (gg->sphere_ui.apply_btn), "clicked",
                        G_CALLBACK (sphere_apply_cb), gg);

    /*-- list to show the currently sphered variables --*/
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (vb), scrolled_window,
      true, true, 0);

	model = gtk_list_store_new(1, G_TYPE_STRING);
    gg->sphere_ui.tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
	populate_tree_view(gg->sphere_ui.tree_view, titles, G_N_ELEMENTS(titles), true, 
		GTK_SELECTION_SINGLE, NULL, NULL);
    /*g_signal_connect (G_OBJECT (gg->sphere_ui.tree_view),
                        "size_allocate",
                        G_CALLBACK(sphere_tree_view_size_alloc_cb),
                        (gpointer) gg);*/
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(gg->sphere_ui.tree_view), false);
    widget_initialize (gg->sphere_ui.tree_view, false);

    gtk_container_add (GTK_CONTAINER (scrolled_window),
      gg->sphere_ui.tree_view);
    /*-- --*/

/*
 * Di and I decided there's no good reason to have this button.
    gg->sphere_ui.restore_btn = gtk_button_new_with_label ("Restore scree plot");
    GGobi_widget_set (gg->sphere_ui.restore_btn, gg, true);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), gg->sphere_ui.restore_btn,
      "Restore the scree plot to reflect the current principal components",
      NULL);
    g_signal_connect (G_OBJECT (gg->sphere_ui.restore_btn), "clicked",
                        G_CALLBACK (scree_restore_cb), gg);
    gtk_box_pack_start (GTK_BOX (vb), gg->sphere_ui.restore_btn,
      false, false, 0);
*/

    /*-- close button --*/
    gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new(),
      false, true, 2);
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), hb, false, false, 1);

    btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_box_pack_start (GTK_BOX (hb), btn, true, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), btn,
      "Close the sphering window", NULL);
    g_signal_connect (G_OBJECT (btn), "clicked",
                        G_CALLBACK (close_btn_cb), gg);

    g_object_set_data(G_OBJECT (gg->sphere_ui.window),
      "notebook", notebook);
  }

  gtk_widget_show_all (vbox);
  gdk_flush ();

  gtk_widget_show_all (gg->sphere_ui.window);

/*-- play around with making this notebook larger --*/
  if (g_list_length (GTK_NOTEBOOK(notebook)->children) > 0) {
    gint page;
    GtkWidget *swin, *tree_view;
    GtkAdjustment *adj;
    page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    swin = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page);
    if (swin) {
      adj = gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (swin));
      tree_view = GTK_BIN (swin)->child;
      if (tree_view->allocation.height < adj->upper) {
        gint sz = MIN(tree_view->allocation.height*2, adj->upper);
        gtk_widget_set_size_request (tree_view, -1, sz);
      }
      /*
      g_printerr ("value %f lower %f upper %f size %f\n", 
        adj->value, adj->lower, adj->upper, adj->page_size);
      g_printerr ("swin height %d\n", swin->allocation.height);
      g_printerr ("tree_view height %d\n", tree_view->allocation.height);
      */
    }
  }
/*--  --*/

  scree_plot_make (gg);
}
