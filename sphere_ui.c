/* sphere_ui.c */

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static GtkWidget *window = NULL;
static GtkAdjustment *npcs_adj;
static GtkWidget *totvar_entry, *condnum_entry;
static GtkWidget *sphere_apply_btn;

extern gint xg_nspherevars;
extern gint *xg_spherevars;
extern gfloat *xg_eigenval;

gint sphere_npcs;

extern void vc_set (gint);
extern void vc_active_set (gint, gint*);
extern gint sphere_svd (gint, gint*);

/*-------------------------------------------------------------------------*/

void sphere_npcs_set (gint n)
{
  sphere_npcs = n;
}

void
sphere_enable (gboolean sens)
{
  gtk_widget_set_sensitive (sphere_apply_btn, sens);
}

void sphere_transform_set (void)
{
   gint j;

   for (j=0; j<xg_nspherevars; j++)
     xg.vardata[xg_spherevars[j]].tform2 = SPHERE;
}

void sphere_set_condnum (gfloat x)
{
  gchar *lbl = g_strdup_printf ("%5.1f", x);
  gtk_entry_set_text (GTK_ENTRY (condnum_entry), lbl);
  g_free (lbl);
}

void sphere_set_totvar (gfloat x)
{
  gchar *lbl = g_strdup_printf ("%.2e", x);
  gtk_entry_set_text (GTK_ENTRY (totvar_entry), lbl);
  g_free (lbl);
}

void select_npc_cb ()
{
  gint j;
  gfloat ftmp1=0.0, ftmp2=0.0;

  if (sphere_npcs<1) {
     quick_message ("Need to choose at least 1 PC.", false);
     sphere_enable (false);

  } else if (sphere_npcs > xg_nspherevars) {
     gchar *msg = g_strdup_printf ("Need to choose at most %d PCs.\n",
       xg_nspherevars);
     quick_message (msg, false);
     sphere_enable (false);
     g_free (msg);

  } else {
     
    for (j=0; j<sphere_npcs; j++)
      ftmp1 += xg_eigenval[j];
    for (j=0; j<xg_nspherevars; j++)
      ftmp2 += xg_eigenval[j];

    sphere_set_totvar (ftmp1/ftmp2);
    sphere_set_condnum (xg_eigenval[0]/xg_eigenval[sphere_npcs-1]);
    sphere_enable (true);
  }
}

/*-------------------------------------------------------------------------*/
/*                         Scree plot                                      */
/*-------------------------------------------------------------------------*/

#define SCREE_WIDTH 200
#define SCREE_HEIGHT 100

static GtkWidget *scree_da;
static GdkPixmap *scree_pixmap;

static gint
scree_configure_cb (GtkWidget *w, GdkEventConfigure *event, splotd *sp)
{
  if (scree_pixmap == NULL)
    scree_pixmap = gdk_pixmap_new (w->window,
      w->allocation.width, w->allocation.height, -1);

  return false;
}
static gint
scree_expose_cb (GtkWidget *w, GdkEventConfigure *event, splotd *sp)
{
  gint j;
  gint xpos, ypos, xstrt, ystrt;
  gchar *tickmk;
  GtkStyle *style = gtk_widget_get_style (scree_da);

  /* clear the pixmap */
  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (scree_pixmap, plot_GC,
                      true, 0, 0,
                      w->allocation.width,
                      w->allocation.height);

  gdk_draw_line (scree_pixmap, plot_GC, 10, 90, 190, 90);
  gdk_draw_line (scree_pixmap, plot_GC, 10, 90, 10, 10);

  for (j=0; j<xg_nspherevars; j++) {
    xpos = (gint) (180./(gfloat)(xg_nspherevars-1)*j+10);
    ypos = (gint) (90. - xg_eigenval[j]/xg_eigenval[0]*80.);

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
  return false;
}

/*
 * Calculate the svd and display results in a scree plot
*/
void scree_plot_make (void)  /*-- when sphere panel is opened --*/
{
  gint svd_ok;
  gint j;

  xg_nspherevars = selected_cols_get (xg_spherevars, false);
  if (xg_nspherevars == 0)
    xg_nspherevars = plotted_cols_get (xg_spherevars, false);

  sphere_transform_set ();
  for (j=0; j<xg_nspherevars; j++)
    vc_set (xg_spherevars[j]);
  
   /* If xg_nspherevars > 1 use svd routine, otherwise just standardize */
  if (xg_nspherevars > 1) {
    vc_active_set (xg_nspherevars, xg_spherevars);
    svd_ok = sphere_svd (xg_nspherevars, xg_spherevars);
    if (!svd_ok) {
       quick_message ("Variance-covariance is identity already!\n", false);
    } else {
      gboolean rval = false;
      gtk_signal_emit_by_name (GTK_OBJECT (scree_da), "expose_event",
        (gpointer) NULL, (gpointer) &rval);
    }
  } else {
    /*scale to variance=1*/
  }
}

/*-------------------------------------------------------------------------*/


void
sphere_npcs_set_cb (GtkAdjustment *adj, GtkWidget *w) 
{
  gint n = (gint) adj->value;
g_printerr ("numpcs=%d\n", n);

  sphere_npcs_set (n);
}



void
sphere_panel_open ()
{
  GtkWidget *hbox, *vbox, *frame, *frame_vb;
  GtkWidget *vb, *label;
  GtkWidget *hb, *npcs_spinner;

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
      gtk_adjustment_new (1.0, 1.0, xg.ncols-1, 1.0, 5.0, 0.0);
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
/*
    gtk_signal_connect (GTK_OBJECT (btn), "clicked",
                      GTK_SIGNAL_FUNC (reset_tform_cb), NULL);
*/

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

/* I'm not sure which of these should come first so that the
 * expose_cb works properly
*/

  scree_plot_make ();

  gtk_widget_show_all (window);

}
