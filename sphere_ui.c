/* sphere_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *window = NULL;
static GtkAdjustment *npcs_adj;
static GtkWidget *totvar_entry, *condnum_entry;
static GtkWidget *sphere_apply_btn;

/*-- these will all be moved to externs.h --*/
extern gint sphere_npcs_get (void);
extern void sphere_npcs_set (gint);
extern void spherevars_set (void);
extern gint spherevars_get (gint *);
extern gint nspherevars_get (void);
extern void sphere_transform_set (void);
extern void pca_diagnostics_set ();
extern void eigenvals_get (gfloat *);
extern gboolean pca_calc (void);
extern void sphere_apply_cb (void);

/*-------------------------------------------------------------------------*/
/*                   routines for manipulating the gui                     */
/*-------------------------------------------------------------------------*/

void sphere_enable (gboolean sens)
{
  gtk_widget_set_sensitive (sphere_apply_btn, sens);
}

void sphere_condnum_set (gfloat x)
{
  gchar *lbl = g_strdup_printf ("%5.1f", x);
  gtk_entry_set_text (GTK_ENTRY (condnum_entry), lbl);
  g_free (lbl);
}

void sphere_totvar_set (gfloat x)
{
  gchar *lbl = g_strdup_printf ("%.2e", x);
  gtk_entry_set_text (GTK_ENTRY (totvar_entry), lbl);
  g_free (lbl);
}

/*-------------------------------------------------------------------------*/
/*                          callbacks                                      */
/*-------------------------------------------------------------------------*/

void
sphere_npcs_set_cb (GtkAdjustment *adj, GtkWidget *w) 
{
  gint n = (gint) adj->value;
  sphere_npcs_set (n);
}

/*-------------------------------------------------------------------------*/
/*                         Scree plot                                      */
/*-------------------------------------------------------------------------*/

#define SCREE_WIDTH 200
#define SCREE_HEIGHT 100

static GtkWidget *scree_da = NULL;
static GdkPixmap *scree_pixmap;

gboolean scree_mapped_p (void) {
  return (scree_da != NULL);
}

static gint
scree_configure_cb (GtkWidget *w, GdkEventConfigure *event, splotd *sp)
{
  if (scree_pixmap == NULL) {
    scree_pixmap = gdk_pixmap_new (w->window,
      w->allocation.width, w->allocation.height, -1);
  }

  return false;
}
static gint
scree_expose_cb (GtkWidget *w, GdkEventConfigure *event, splotd *sp)
{
  gint j;
  gint xpos, ypos, xstrt, ystrt;
  gchar *tickmk;
  GtkStyle *style = gtk_widget_get_style (scree_da);

  gint *sphvars = (gint *) g_malloc (xg.ncols * sizeof (gint));
  gfloat *evals = (gfloat *) g_malloc (xg.ncols * sizeof (gfloat));

  gint nsphvars = spherevars_get (sphvars);
  eigenvals_get (evals);

for (j=0; j<nsphvars; j++)
g_printerr ("(expose) sphvar %d eval %f\n", sphvars[j], evals[j]);

  /* clear the pixmap */
  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (scree_pixmap, plot_GC,
                      true, 0, 0,
                      w->allocation.width,
                      w->allocation.height);

  gdk_gc_set_foreground (plot_GC, &xg.accent_color);
  gdk_draw_line (scree_pixmap, plot_GC, 10, 90, 190, 90);
  gdk_draw_line (scree_pixmap, plot_GC, 10, 90, 10, 10);

  for (j=0; j<nsphvars; j++) {
    xpos = (gint) (180./(gfloat)(nsphvars-1)*j+10);
    ypos = (gint) (90. - evals[j]/evals[0]*80.);

    tickmk = g_strdup_printf ("%d", j+1);
    gdk_draw_string (scree_pixmap, style->font, plot_GC, xpos, 95, tickmk);
    g_free (tickmk);

    if (j>0) 
      gdk_draw_line (scree_pixmap, plot_GC, xstrt, ystrt, xpos, ypos);

    xstrt = xpos;
    ystrt = ypos;
  }

  gdk_draw_pixmap (w->window, plot_GC, scree_pixmap,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);

  g_free ((gpointer) sphvars);
  g_free ((gpointer) evals);

  return false;
}

/*
 * Calculate the svd and display results in a scree plot
*/
void scree_plot_make (void)  /*-- when sphere panel is opened --*/
{
g_printerr ("(scree_plot_make)\n");
  if (pca_calc ()) {
    gboolean rval = false;
    gtk_signal_emit_by_name (GTK_OBJECT (scree_da), "expose_event",
      (gpointer) NULL, (gpointer) &rval);
    pca_diagnostics_set ();
  } else {
     quick_message ("Variance-covariance is identity already!\n", false);
  }
}

/*-------------------------------------------------------------------------*/
/*                     Create and map the sphere panel                     */
/*-------------------------------------------------------------------------*/

void
sphere_panel_open (void)
{
  GtkWidget *hbox, *vbox, *frame, *frame_vb;
  GtkWidget *vb, *label;
  GtkWidget *hb, *npcs_spinner;
  gint nvars;

  spherevars_set ();
  nvars = nspherevars_get ();

  if (window == NULL) {
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "sphere variables");
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);

    hbox = gtk_hbox_new (false, 2);
    gtk_container_add (GTK_CONTAINER (window), hbox);

    /* Controls to the left */
    vbox = gtk_vbox_new (false, 5);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, false, false, 0);

    /* Spinner: number of principal components */
    vb = gtk_vbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vbox), vb, false, false, 0);

    label = gtk_label_new ("Number of PCs:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);


    /*-- hbox to contain the spinner and the 'apply' button --*/
    hb = gtk_hbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (vb), hb, true, true, 2);
    
    /*-- the parameters of the adjustment should be reset each time --*/
    npcs_adj = (GtkAdjustment *)
      gtk_adjustment_new ((gfloat) nvars, 1.0, (gfloat) xg.ncols,
                          1.0, 5.0, 0.0);
    gtk_signal_connect (GTK_OBJECT (npcs_adj), "value_changed",
		                GTK_SIGNAL_FUNC (sphere_npcs_set_cb),
		                NULL);
    npcs_spinner = gtk_spin_button_new (npcs_adj, 0, 0);

    gtk_spin_button_set_wrap (GTK_SPIN_BUTTON (npcs_spinner), false);
    gtk_spin_button_set_shadow_type (GTK_SPIN_BUTTON (npcs_spinner),
                                     GTK_SHADOW_OUT);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), npcs_spinner,
      "Specify the number of principal components",
      NULL);
    gtk_box_pack_start (GTK_BOX (hb), npcs_spinner, true, true, 0);

    sphere_apply_btn = gtk_button_new_with_label ("Apply");
    gtk_box_pack_start (GTK_BOX (hb), sphere_apply_btn, false, false, 0);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), sphere_apply_btn,
      "Perform principal components transformation for the first n variables",
      NULL);
    gtk_signal_connect (GTK_OBJECT (sphere_apply_btn), "clicked",
                        GTK_SIGNAL_FUNC (sphere_apply_cb), NULL);

    /*-- the labels, in a frame --*/
    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_box_pack_start (GTK_BOX (vbox), frame, true, true, 0);

    frame_vb = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (frame_vb), 5);
    gtk_container_add (GTK_CONTAINER (frame), frame_vb);

    /*-- total variance --*/
    vb = gtk_vbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (frame_vb), vb, false, false, 0);

    label = gtk_label_new ("Total variance:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    totvar_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (totvar_entry), false);
    gtk_entry_set_text (GTK_ENTRY (totvar_entry), "-");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), totvar_entry,
      "The percentage of variance accounted for by the selected variables",
      NULL);
    gtk_box_pack_start (GTK_BOX (vb), totvar_entry, true, true, 2);

    /*-- condition number --*/
    vb = gtk_vbox_new (false, 2);
    gtk_box_pack_start (GTK_BOX (frame_vb), vb, false, false, 0);

    label = gtk_label_new ("Condition number:");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_box_pack_start (GTK_BOX (vb), label, false, false, 0);

    condnum_entry = gtk_entry_new ();
    gtk_entry_set_editable (GTK_ENTRY (condnum_entry), false);
    gtk_entry_set_text (GTK_ENTRY (condnum_entry), "-");
    gtk_tooltips_set_tip (GTK_TOOLTIPS (xg.tips), condnum_entry,
      "The condition number for the selected variables",
      NULL);
    gtk_box_pack_start (GTK_BOX (vb), condnum_entry, true, true, 2);


    /*-- and on the right, the scree plot --*/
    frame = gtk_frame_new ("Scree plot");
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
    gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 1);

    scree_da = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (scree_da),
      SCREE_WIDTH, SCREE_HEIGHT);
    gtk_container_add (GTK_CONTAINER (frame), scree_da);

    gtk_signal_connect (GTK_OBJECT (scree_da),
                        "expose_event",
                        (GtkSignalFunc) scree_expose_cb,
                        (gpointer) NULL);
    gtk_signal_connect (GTK_OBJECT (scree_da),
                        "configure_event",
                        (GtkSignalFunc) scree_configure_cb,
                        (gpointer) NULL);
  }

  gtk_widget_show_all (window);

  scree_plot_make ();
}
