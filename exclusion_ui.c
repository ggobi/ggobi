/*-- exclusion_ui.c  --*/
/*
 * This is no longer appropriately named: think of it as cluster_ui.c
*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gint exclusion_notebook_adddata_cb(ggobid *, datad *, void* notebook);

static void destroyit (gboolean kill, ggobid * gg)
{
  gint n, nrows;
  GSList *l;
  datad *d;

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    if (d->cluster_table) {
      nrows = GTK_TABLE(d->cluster_table)->nrows;
      for (n = 0; n < nrows - 1; n++)
        cluster_free(n, d, gg);
    }
  }

  if (kill) {
    gtk_widget_destroy(gg->cluster_ui.window);
    gg->cluster_ui.window = NULL;
  } else {
    /*-- the window should have just one child.  Find it and kill it --*/
    GList *gl =
        gtk_container_children(GTK_CONTAINER(gg->cluster_ui.window));
    GtkWidget *child = (GtkWidget *) gl->data;
    gtk_widget_destroy(child);
  }
}

/*-- called when closed from the close button --*/
static void close_btn_cb(GtkWidget * w, ggobid * gg)
{
  destroyit(true, gg);
}

/*-- called when closed from the window manager --*/
static void close_wmgr_cb(GtkWidget * w, GdkEvent * event, ggobid * gg)
{
  destroyit(true, gg);
}

static gint
cluster_symbol_show(GtkWidget * w, GdkEventExpose * event, gpointer cbd)
{
  gint k = GPOINTER_TO_INT(cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  icoords pos;
  glyphd g;
  datad *d = datad_get_from_notebook(gg->cluster_ui.notebook, gg);
  colorschemed *scheme = gg->activeColorScheme;

  /*-- fill in the background color --*/
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle(w->window, gg->plot_GC,
    true, 0, 0,
    w->allocation.width, w->allocation.height);

  /*-- draw the appropriate symbol in the appropriate color --*/
  gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb[d->clusv[k].color]);
  g.type = d->clusv[k].glyphtype;
  g.size = d->clusv[k].glyphsize;

  pos.x = w->allocation.width / 2;
  pos.y = w->allocation.height / 2;
  draw_glyph(w->window, &g, &pos, 0, gg);

  return FALSE;
}

void cluster_table_labels_update(datad * d, ggobid * gg)
{
  gint k;
  gchar *str;

  if (gg->cluster_ui.window == NULL)
    return;

  for (k = 0; k < d->nclusters; k++) {
    str = g_strdup_printf("%ld", d->clusv[k].nhidden);
    gtk_label_set_text(GTK_LABEL(d->clusvui[k].nh_lbl), str);
    g_free(str);

    str = g_strdup_printf("%ld", d->clusv[k].nshown);
    gtk_label_set_text(GTK_LABEL(d->clusvui[k].ns_lbl), str);
    g_free(str);

    str = g_strdup_printf("%ld", d->clusv[k].n);
    gtk_label_set_text(GTK_LABEL(d->clusvui[k].n_lbl), str);
    g_free(str);
  }
}

static void rescale_cb(GtkWidget * w, ggobid * gg)
{
  datad *d = datad_get_from_notebook(gg->cluster_ui.notebook, gg);
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;

  limits_set(true, true, d, gg);
  vartable_limits_set(d);
  vartable_stats_set(d);

  if (cpanel->projection == TOUR1D) {
    dsp->t1d.get_new_target = true;
  }
  if (cpanel->projection == TOUR2D3) 
    dsp->t2d3.get_new_target = true;
  if (cpanel->projection == TOUR2D) {
    dsp->t2d.get_new_target = true;
  }
  if (cpanel->projection == COTOUR) {
    dsp->tcorr1.get_new_target = true;
    dsp->tcorr2.get_new_target = true;
  }

  tform_to_world(d, gg);
  displays_tailpipe(FULL, gg);   /*-- points rebinned --*/
}


static gint hide_cluster_cb(GtkToggleButton * btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT(cbd);
  gint i;
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(btn), true);
  datad *d = datad_get_from_notebook(gg->cluster_ui.notebook, gg);

  /*-- operating on the current sample, whether hidden or shown --*/
  for (i = 0; i < d->nrows; i++) {
    if (d->sampled.els[i]) {
      if (d->clusterid.els[i] == k) {
        d->hidden.els[i] = d->hidden_now.els[i] = true;
      }
    }
  }

  rows_in_plot_set(d, gg);
  assign_points_to_bins(d, gg);
  clusters_set(d, gg);

  cluster_table_labels_update(d, gg);
  displays_plot(NULL, FULL, gg);

  return false;
}

static gint show_cluster_cb(GtkToggleButton * btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT(cbd);
  gint i;
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(btn), true);
  datad *d = datad_get_from_notebook(gg->cluster_ui.notebook, gg);

  /*-- operating on the current sample, whether hidden or shown --*/
  for (i = 0; i < d->nrows; i++) {
    if (d->sampled.els[i]) {
      if (d->clusterid.els[i] == k) {
        d->hidden.els[i] = d->hidden_now.els[i] = false;
      }
    }
  }

  clusters_set(d, gg);
  cluster_table_labels_update(d, gg);

  rows_in_plot_set(d, gg);

  /*
   * Don't re-set the limits, but re-run the pipeline in case
   * the data about to be shown isn't on the current scale
   */
  tform_to_world(d, gg);
  displays_tailpipe(FULL, gg);  /*-- points rebinned in here --*/

  return false;
}

/*-- show/hide complement --*/
static gint comp_cluster_cb(GtkToggleButton * btn, gpointer cbd)
{
  gint k = GPOINTER_TO_INT(cbd);
  gint i;
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(btn), true);
  datad *d = datad_get_from_notebook(gg->cluster_ui.notebook, gg);

  /*-- operating on the current sample, whether hidden or shown --*/
  for (i = 0; i < d->nrows; i++) {
    if (d->sampled.els[i]) {
      if (d->clusterid.els[i] == k) {
        d->hidden.els[i] = d->hidden_now.els[i] = !d->hidden.els[i];
      }
    }
  }
  rows_in_plot_set(d, gg);
  assign_points_to_bins(d, gg);
  clusters_set(d, gg);
  cluster_table_labels_update(d, gg);
  displays_plot(NULL, FULL, gg);

  return false;
}

static gint
cluster_symbol_cb(GtkWidget * w, GdkEventExpose * event, gpointer cbd)
{
  /*-- reset the glyph and color of this glyph to the current values --*/
  gint n = GPOINTER_TO_INT(cbd);
  ggobid *gg = GGobiFromWidget(w, true);
  datad *d = datad_get_from_notebook(gg->cluster_ui.notebook, gg);
  gint k, m, i;
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean rval = false;
  gint nclusters = symbol_table_populate(d);
  gboolean proceed = true;
  gint targets = cpanel->br_point_targets;
  gint nd = g_slist_length(gg->d);

/*
 * Almost surely the user is not trying to collapse groups, so
 * check whether there's another cluster with this color/glyph
 * combination.  If there is, don't go ahead.  (Though we may
 * want to add a dialog for this later.)
*/
  for (k = 0; k < nclusters; k++) {
    if (k != n) {
      switch (targets) {
        case br_candg:
          if (d->clusv[k].glyphtype == gg->glyph_id.type &&
              d->clusv[k].glyphsize == gg->glyph_id.size &&
              d->clusv[k].color == gg->color_id)
          {
            proceed = false;
            break;
          }
        break;
        case br_color:
        /*
         * This would produce an identical cluster if the glyph
         * types and sizes of the clusters are already equal, and
         * the new glyph sizes would complete the match.
         */
          if (d->clusv[k].glyphtype == d->clusv[n].glyphtype &&
              d->clusv[k].glyphsize == d->clusv[n].glyphsize &&
              d->clusv[k].color == gg->color_id)
          {
            proceed = false;
            break;
          }
        break;
        case br_glyph:
          if (d->clusv[k].color == d->clusv[n].color &&
              d->clusv[k].glyphtype == gg->glyph_id.type &&
              d->clusv[k].glyphsize == gg->glyph_id.size)
          {
            proceed = false;
            break;
          }
        break;
      }
    }
  }

/*
 * This is a bit paternalistic, no?  But it's probably ok for now.
*/
  if (!proceed) {
    quick_message
      ("You're about to reset the color and/or glyph for this cluster\nin such a way as to merge it with another cluster.  I bet\nthat's not what you intend, so I won't let you do it.\n",
       false);
    return true;
  }

  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    if (d->clusterid.els[i] == n) {
      if (targets == br_candg || targets == br_color) {
        d->color.els[i] = d->color_now.els[i] = gg->color_id;
        /*-- this will be done multiple times, but who cares? --*/
        d->clusv[n].color = gg->color_id;
      }
      if (targets == br_candg || targets == br_glyph) {
        d->glyph.els[i].type = d->glyph_now.els[i].type =
            gg->glyph_id.type;
        d->glyph.els[i].size = d->glyph_now.els[i].size =
            gg->glyph_id.size;
        /*-- this will be done multiple times, but who cares? --*/
        d->clusv[n].glyphtype = gg->glyph_id.type;
        d->clusv[n].glyphsize = gg->glyph_id.size;
      }

      /*-- link so that displays of linked datad's will be brushed as well --*/
      if (nd > 1 && !gg->linkby_cv)
        symbol_link_by_id (true, i, d, gg);
    }
  }

  gtk_signal_emit_by_name(GTK_OBJECT(w), "expose_event",
    (gpointer) gg, (gpointer) & rval);

  /* clusters_set reorders clusv, so it's bad news here */
  /*clusters_set (d, gg); */

  displays_plot(NULL, FULL, gg);

  return false;
}

void cluster_add(gint k, datad * d, ggobid * gg)
{
  gchar *str;
  gint dawidth = 2 * NGLYPHSIZES + 1 + 10;

  d->clusvui[k].da = gtk_drawing_area_new();
#if GTK_MAJOR_VERSION == 2
  gtk_widget_set_double_buffered(d->clusvui[j].da, false);
#endif
  gtk_drawing_area_size(GTK_DRAWING_AREA(d->clusvui[k].da),
    dawidth, dawidth);

  gtk_widget_set_events(d->clusvui[k].da,
    GDK_EXPOSURE_MASK | GDK_ENTER_NOTIFY_MASK
    | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

  gtk_signal_connect(GTK_OBJECT(d->clusvui[k].da), "expose_event",
    GTK_SIGNAL_FUNC(cluster_symbol_show),
    GINT_TO_POINTER(k));
  gtk_signal_connect(GTK_OBJECT(d->clusvui[k].da), "button_press_event",
    GTK_SIGNAL_FUNC(cluster_symbol_cb),
    GINT_TO_POINTER(k));
  GGobi_widget_set(d->clusvui[k].da, gg, true);
  gtk_table_attach(GTK_TABLE(d->cluster_table), d->clusvui[k].da,
    0, 1, k + 1, k + 2,
    (GtkAttachOptions) 0, (GtkAttachOptions) 0, 5, 2);


  d->clusvui[k].h_btn = gtk_button_new_with_label("H");
  gtk_signal_connect(GTK_OBJECT(d->clusvui[k].h_btn), "clicked",
    GTK_SIGNAL_FUNC(hide_cluster_cb), GINT_TO_POINTER(k));
  GGobi_widget_set(d->clusvui[k].h_btn, gg, true);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].h_btn,
    1, 2, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);

  d->clusvui[k].s_btn = gtk_button_new_with_label("S");
  gtk_signal_connect(GTK_OBJECT(d->clusvui[k].s_btn), "clicked",
                     GTK_SIGNAL_FUNC(show_cluster_cb), GINT_TO_POINTER(k));
  GGobi_widget_set(d->clusvui[k].s_btn, gg, true);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].s_btn,
    2, 3, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);

  d->clusvui[k].c_btn = gtk_button_new_with_label("C");
  gtk_signal_connect(GTK_OBJECT(d->clusvui[k].c_btn), "clicked",
                     GTK_SIGNAL_FUNC(comp_cluster_cb), GINT_TO_POINTER(k));
  GGobi_widget_set(d->clusvui[k].c_btn, gg, true);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].c_btn,
    3, 4, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);

  str = g_strdup_printf("%ld", d->clusv[k].nhidden);
  d->clusvui[k].nh_lbl = gtk_label_new(str);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].nh_lbl,
    4, 5, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
  g_free(str);

  str = g_strdup_printf("%ld", d->clusv[k].nshown);
  d->clusvui[k].ns_lbl = gtk_label_new(str);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].ns_lbl,
    5, 6, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
  g_free(str);

  str = g_strdup_printf("%ld", d->clusv[k].n);
  d->clusvui[k].n_lbl = gtk_label_new(str);
  gtk_table_attach(GTK_TABLE(d->cluster_table),
    d->clusvui[k].n_lbl,
    6, 7, k + 1, k + 2, GTK_FILL, GTK_FILL, 5, 2);
  g_free(str);
}

void cluster_free(gint k, datad * d, ggobid * gg)
{
  if (d->clusvui[k].da) {
    gtk_widget_destroy(d->clusvui[k].da);
    gtk_widget_destroy(d->clusvui[k].h_btn);
    gtk_widget_destroy(d->clusvui[k].s_btn);
    gtk_widget_destroy(d->clusvui[k].c_btn);
    gtk_widget_destroy(d->clusvui[k].nh_lbl);
    gtk_widget_destroy(d->clusvui[k].ns_lbl);
    gtk_widget_destroy(d->clusvui[k].n_lbl);
  }
}


static void update_cb(GtkWidget * w, ggobid * gg)
{
  cluster_window_open(gg);
}

static gboolean nclusters_changed(ggobid * gg)
{
  datad *d;
  gint k, nrows = 0;
  GtkWidget *page;
  gboolean changed = false;
  gint nd = g_slist_length(gg->d);

  for (k = 0; k < nd; k++) {
    nrows = 0;
    page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(gg->cluster_ui.notebook),
                                     k);
    if (page) {
      d = (datad *) gtk_object_get_data(GTK_OBJECT(page), "datad");
      nrows = GTK_TABLE(d->cluster_table)->nrows;

      if (nrows != d->nclusters + 1) {/*-- add one for the titles --*/
        changed = true;
      }
    } else {  /*-- if page is NULL, a new datad has been added --*/
      changed = true;
    }
    if (changed)
      break;
  }
  return changed;
}


void cluster_table_update (datad * d, ggobid * gg)
{
  if (gg->cluster_ui.window == NULL) {
    ;
  } else {
    if (nclusters_changed(gg)) {   /*-- for any of the datad's --*/
      cluster_window_open(gg);
    } else {
      cluster_table_labels_update(d, gg); /*-- d, or all d's? --*//* do all */
    }
  }
}

static gint
exclusion_notebook_adddata_cb (ggobid *gg, datad * d, void* notebook)
{
  /*cluster_table_update(d, gg);*/

  cluster_window_open (gg);
  return true;  /* risky -- will this prevent other guys from getting it?  --*/
}

CHECK_EVENT_SIGNATURE(exclusion_notebook_adddata_cb, datad_added_f)


void cluster_window_open(ggobid * gg)
{
  GtkWidget *scrolled_window = NULL;
  GtkWidget *vbox, *tebox, *btn, *hbox, *lbl;
  GtkWidget *ebox;
  gint k;
  GSList *l;
  datad *d;
  gboolean new = false;

  /*-- if used before we have data, bail out --*/
  if (gg->d == NULL || g_slist_length(gg->d) == 0)
    /**/ return;

  /*-- if it isn't NULL, then destroy it and start afresh --*/
  if (gg->cluster_ui.window != NULL) {
    destroyit(false, gg);   /*-- don't kill the whole thing --*/
  }

  if (gg->cluster_ui.window == NULL ||
      !GTK_WIDGET_REALIZED(gg->cluster_ui.window))
  {
    gg->cluster_ui.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_signal_connect(GTK_OBJECT(gg->cluster_ui.window), "delete_event",
      GTK_SIGNAL_FUNC(close_wmgr_cb), (gpointer) gg);
    gtk_window_set_title(GTK_WINDOW(gg->cluster_ui.window),
      "color & glyph groups");
    new = true;
  }

  vbox = gtk_vbox_new(false, 5);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
  gtk_container_add(GTK_CONTAINER(gg->cluster_ui.window), vbox);

  tebox = gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(vbox), tebox, true, true, 2);

  /* Create a notebook, set the position of the tabs */
  gg->cluster_ui.notebook = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(gg->cluster_ui.notebook),
    GTK_POS_TOP);
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK(gg->cluster_ui.notebook),
    g_slist_length(gg->d) > 1);
  gtk_container_add(GTK_CONTAINER(tebox), gg->cluster_ui.notebook);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;

    /*-- skip datasets without variables --*/
    if (!datad_has_variables (d))
      continue;

    /* Create a scrolled window to hold the table */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

    gtk_object_set_data(GTK_OBJECT(scrolled_window), "datad", d);  /*setdata*/
    gtk_notebook_append_page(GTK_NOTEBOOK(gg->cluster_ui.notebook),
      scrolled_window, gtk_label_new(d->name));
    gtk_widget_show(scrolled_window);

    d->cluster_table = gtk_table_new(d->nclusters + 1, 7, false);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW
      (scrolled_window), d->cluster_table);

    /*-- add the row of titles --*/

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Click to change the color/glyph of all members of the selected cluster to the current brushing color/glyph",
      NULL);
    lbl = gtk_label_new("Symbol");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox, 0, 1, 0, 1,
      /*-- left, right, top, bottom --*/
      GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Hide all cases with the corresponding symbol",
      NULL);
    lbl = gtk_label_new("Hide");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Show all cases with the corresponding symbol",
      NULL);
    lbl = gtk_label_new("Show");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      2, 3, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "Complement: Show/hide all cases with the corresponding symbol that are hidden/shown",
      NULL);
    lbl = gtk_label_new("Comp");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      3, 4, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "The number of hidden cases out of N with the corresponding symbol.",
      NULL);
    lbl = gtk_label_new("Hidden");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      4, 5, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "The number of visible cases out of N with the corresponding symbol.",
      NULL);
    lbl = gtk_label_new("Shown");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      5, 6, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    ebox = gtk_event_box_new();
    gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), ebox,
      "The number of cases with the corresponding symbol.  If sampling, the number of cases in the current subsample",
      NULL);
    lbl = gtk_label_new("N");
    gtk_container_add(GTK_CONTAINER(ebox), lbl);
    gtk_table_attach(GTK_TABLE(d->cluster_table), ebox,
      6, 7, 0, 1, GTK_FILL, GTK_FILL, 5, 2);

    d->clusvui = (clusteruid *)
      g_realloc(d->clusvui, d->nclusters * sizeof(clusteruid));
    /*-- add the cluster rows, one by one --*/
    for (k = 0; k < d->nclusters; k++)
      cluster_add(k, d, gg);
  }

  /*-- listen for datad_added events on main_window --*/
  /*-- Be careful to add this signal handler only once! --*/
  if (new) {
    gtk_signal_connect(GTK_OBJECT(gg),
      "datad_added",
      GTK_SIGNAL_FUNC(exclusion_notebook_adddata_cb),
      NULL);
  }

  /*-- give the window an initial height --*/
  gtk_widget_set_usize(GTK_WIDGET(scrolled_window), -1, 150);

  /*-- horizontal box to hold a few buttons --*/
  hbox = gtk_hbox_new(false, 2);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 0);

  /*-- Update button --*/
  btn = gtk_button_new_with_label("Update");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "This table should stay up to date by itself, but this will reset it in case it doesn't do that.",
    NULL);
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(update_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(hbox), btn, true, true, 0);

  /*-- Rescale button --*/
  btn = gtk_button_new_with_label("Rescale");
  gtk_tooltips_set_tip(GTK_TOOLTIPS(gg->tips), btn,
    "Reset the plotting limits; use to rescale in response to hiding (or showing) points.",
    NULL);
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(rescale_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(hbox), btn, true, true, 0);

  /*-- Close button --*/
  btn = gtk_button_new_with_label("Close");
  gtk_signal_connect(GTK_OBJECT(btn), "clicked",
    GTK_SIGNAL_FUNC(close_btn_cb), (gpointer) gg);
  gtk_box_pack_start(GTK_BOX(hbox), btn, true, true, 0);

  gtk_widget_show_all(gg->cluster_ui.window);

  for (l = gg->d; l; l = l->next) {
    d = (datad *) l->data;
    /*-- this doesn't track cluster counts, just cluster identities --*/
    gtk_signal_emit(GTK_OBJECT(gg), GGobiSignals[CLUSTERS_CHANGED_SIGNAL], d);
  }

  gdk_window_raise(gg->cluster_ui.window->window);
}
