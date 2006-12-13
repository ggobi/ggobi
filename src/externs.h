/* externs.h */
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

#ifndef GGOBI_S_H
#define GGOBI_S_H

#include <stdio.h>
#ifndef DEFINES_H
#include "defines.h"
#endif

#include "barchartDisplay.h"

#ifdef __cplusplus
extern "C" {
#endif

GtkWidget * mode_panel_get_by_name(const gchar *, ggobid *);
InteractionMode imode_get(ggobid *);  /* this should probably also
					 change */
ProjectionMode pmode_get(displayd *, ggobid *);
void pmode_set_cb (GtkWidget *widget, gint action);
void imode_set_cb (GtkWidget *widget, gint action);
void main_miscmenus_update (ProjectionMode, InteractionMode, displayd *, ggobid *gg);
void viewmode_set(ProjectionMode, InteractionMode, ggobid *);
gint ggobi_full_viewmode_set(ProjectionMode, InteractionMode, ggobid*);

RedrawStyle brush_activate (gboolean, displayd *, splotd *);
RedrawStyle edgeedit_activate (gboolean state, displayd *display, ggobid *gg);
RedrawStyle identify_activate (gint, displayd *, ggobid *);
RedrawStyle p1d_activate (gint, displayd *, ggobid *);
RedrawStyle imode_activate (splotd *, ProjectionMode, InteractionMode, gboolean, ggobid *);
RedrawStyle xyplot_activate (gint, displayd *, ggobid *);


/* reverse pipeline */
void pt_screen_to_plane (icoords *screen, gint id, gboolean horiz, gboolean vert, gcoords *eps, gcoords *planar, splotd *sp);
void pt_plane_to_world (splotd *sp, gcoords *planar, gcoords *eps, greal *world);
void pt_world_to_raw_by_var (gint j, greal *world, greal *raw, GGobiStage *d);

/* sort -t":" -k1.12 */
gboolean   array_contains (gint* arr, gint n, gint el);
void       edgeedit_init (ggobid * gg);
GtkWidget* CreateMenuCheck (GtkWidget *, gchar *, GtkSignalFunc, gpointer, gboolean, ggobid *);
GtkWidget* CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *, GtkWidget *, GtkAccelGroup *, GtkSignalFunc, gpointer, ggobid *) ;
GtkWidget* CreateMenuItemWithCheck (GtkWidget *, gchar *, gchar *, gchar *, GtkWidget *, GtkAccelGroup *, GtkSignalFunc, gpointer, ggobid *, GSList *, gboolean check) ;
ggobid*    GGobiFromDisplay (displayd *display);
ggobid*    GGobiFromSPlot (splotd *sp);
ggobid*    GGobiFromWidget (GtkWidget *w, gboolean);
ggobid*    GGobiFromWindow (GdkWindow *w);
void ggobi_addToolAction (GtkActionEntry *entry, gpointer *data, ggobid *gg);
GtkWidget* ggobi_addToolsMenuItem (gchar *label, ggobid *gg);
gboolean   ggobi_addToolsMenuWidget(GtkWidget *entry, ggobid *gg);
void       ggobi_widget_set (GtkWidget *, ggobid *gg, gboolean isWindow);
GGobiStage*     ValidateDatadRef (GGobiStage *d, ggobid *gg, gboolean fatal);
displayd*  ValidateDisplayRef(displayd *d, ggobid *gg, gboolean fatal);
ggobid*    ValidateGGobiRef (ggobid *gg, gboolean fatal);
#ifdef STORE_SESSION_ENABLED
xmlNodePtr XML_addVariable(xmlNodePtr node, gint j, GGobiStage *d);
#endif
void       addvar_propagate (gint ncols_prev, gint ncols_added, GGobiStage *);
gint       alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols, gint ndim);
gint       alloc_pp (pp_param *pp, gint nrows, gint ncols, gint ndim);
void       arrayd_add_cols (array_d *, gint);
void       arrayd_add_rows (array_d *, gint);
void       arrayd_alloc (array_d *, gint, gint);
void       arrayd_alloc_zero (array_d *, gint, gint);
void       arrayd_copy (array_d *, array_d *);
void       arrayd_delete_cols (array_d *, GSList *);
void       arrayd_free (array_d *, gint, gint);
void       arrayd_init_null (array_d *);
void       arrayd_zero (array_d *);
void       arrayf_add_cols (array_f *, gint);
void       arrayf_add_rows (array_f *, gint);
void       arrayf_alloc (array_f *, gint, gint);
void       arrayf_alloc_zero (array_f *, gint, gint);
void       arrayf_copy (array_f *, array_f *);
void       arrayf_delete_cols (array_f *, GSList *);
void       arrayf_free (array_f *, gint, gint);
void       arrayf_init_null (array_f *);
void       arrayf_zero (array_f *);
void       arrayg_add_cols (array_g *, gint);
void       arrayg_add_rows (array_g *, gint);
void       arrayg_alloc (array_g *, gint, gint);
void       arrayg_alloc_zero (array_g *, gint, gint);
void       arrayg_delete_cols (array_g *, GSList *);
void       arrayg_free (array_g *, gint, gint);
void       arrayg_init_null (array_g *);
void       arrayg_zero (array_g *);
void       arrayl_add_cols (array_l *, gint);
void       arrayl_add_rows (array_l *, gint);
void       arrayl_alloc (array_l *, gint, gint);
void       arrayl_alloc_zero (array_l *, gint, gint);
void       arrayl_delete_cols (array_l *, GSList *);
void       arrayl_free (array_l *, gint, gint);
void       arrayl_init_null (array_l *);
void       arrayl_zero (array_l *);
void       arrays_add_cols (array_s *, gint);
void       arrays_add_rows (array_s *, gint);
void       arrays_alloc (array_s *, gint, gint);
void       arrays_alloc_zero (array_s *, gint, gint);
void       arrays_delete_cols (array_s *, GSList *);
void       arrays_free (array_s *, gint, gint);
void       arrays_init_null (array_s *);
void       arrays_zero (array_s *);
void       ash_baseline_set (icoords *, splotd *sp);
void       assign_points_to_bins (GGobiStage *, splotd *, ggobid *);
gboolean   br_edge_vectors_check_size (gint, GGobiStage *);
void       brush_draw_brush (splotd *, GdkDrawable *, GGobiStage *, ggobid *);
void       brush_draw_label (splotd *, GdkDrawable *, GGobiStage *, ggobid *);
void       brush_event_handlers_toggle (splotd *, gboolean);
gboolean   brush_motion (icoords *, gboolean, gboolean, cpaneld *, splotd *, ggobid *);
void       brush_on_set (gboolean, displayd *, ggobid *);
gboolean   brush_once (gboolean force, splotd *, ggobid *);
gboolean   brush_once_and_redraw (gboolean binningp, splotd *sp, displayd *display, ggobid *gg);
void       brush_pos_init (splotd *);
void       brush_reset(displayd *display, gint action);
void       brush_set_pos (gint, gint, splotd *);
void       brush_undo (GGobiStage *);
void       brush_update_set (gboolean, displayd *, ggobid *);
gboolean   brush_all_matching_cv (cpaneld *, GGobiStage *, ggobid *);
gdouble    calc_norm (gdouble *, gint);
gboolean   checkequiv(gdouble **u0, gdouble **u1, gint nc, gint nd);
void       clone_vars (gint *cols, gint ncols, GGobiStage *);
void       cluster_free (gint, GGobiStage *, ggobid *);
void       cluster_table_labels_update (GGobiStage *d, ggobid *gg);
void       cluster_table_update (GGobiStage *, ggobid *);
void       cluster_window_open (ggobid *);
void       clusters_set(GGobiStage *);
void       collab_tform_update (gint j, GGobiStage *d);
gboolean   colors_remap (colorschemed *scheme, gboolean force, ggobid *gg);
void       colorscheme_init (colorschemed *scheme);
gchar*     computeTitle (gboolean, displayd *, ggobid *);
void       copy_mat(gdouble **, gdouble **, gint, gint);
void       cpanel_brush_init (cpaneld *, ggobid *);
void       cpanel_brush_make (ggobid *);
void       cpanel_brush_set (displayd *, cpaneld *, ggobid *);
void       cpanel_ctour_make (ggobid *);
void       cpanel_edgeedit_init (cpaneld *, ggobid *);
void       cpanel_edgeedit_make (ggobid *);
void       cpanel_edgeedit_set (displayd *, cpaneld *, ggobid *);
void       cpanel_identify_init (cpaneld *, ggobid *);
void       cpanel_identify_make (ggobid *);
void       cpanel_identify_set (displayd *, cpaneld *, ggobid *);
void       cpanel_movepts_make (ggobid *);
void       cpanel_p1d_init (cpaneld *, ggobid *);
void       cpanel_p1d_set (displayd *, cpaneld *, ggobid *);
void       cpanel_p1dplot_make (ggobid *);
GtkWidget *cpanel_parcoords_make (ggobid *);
void       cpanel_parcoords_set (displayd *, cpaneld *, GtkWidget *panel, ggobid *);
void       cpanel_scale_init (cpaneld *, ggobid *);
void       cpanel_scale_make (ggobid *);
void       cpanel_scale_set (displayd *, cpaneld *, ggobid *);
GtkWidget *cpanel_scatmat_make (ggobid *);
void       cpanel_scatmat_set (displayd *, cpaneld *, ggobid *);
void       cpanel_set (displayd *, ggobid *);
void       cpanel_show (gboolean show, ggobid *gg);
void       cpanel_t1d_init (cpaneld *, ggobid *);
void       cpanel_t2d3_init (cpaneld *, ggobid *);
void       cpanel_t2d_init (cpaneld *, ggobid *);
void       cpanel_tcorr_init (cpaneld *, ggobid *);
void       cpanel_tour1d_make (ggobid *);
void       cpanel_tour1d_set (displayd *, cpaneld *, ggobid *);
void       cpanel_tour2d3_make (ggobid *);
void       cpanel_tour2d3_set (displayd *, cpaneld *, ggobid *);
void       cpanel_tour2d_make (ggobid *);
void       cpanel_tour2d_set (displayd *, cpaneld *, ggobid *);
void       cpanel_tourcorr_set (displayd *, cpaneld *, ggobid *);
void       cpanel_xyplot_init (cpaneld *, ggobid *);
void       cpanel_xyplot_make (ggobid *);
void       cpanel_xyplot_set (displayd *, cpaneld *, ggobid *);
displayd * createDisplayFromDescription (ggobid *, GGobiDisplayDescription *desc);
GtkWidget* create_variable_notebook (GtkWidget *box, GtkSelectionMode mode, GGobiVariableType vartype, datatyped dtype, GtkSignalFunc func, gpointer func_data, ggobid *);
typedef const gchar ** (*GGobiVariableNotebookPrefixFunc) (GtkWidget *notebook, GGobiStage *d, gint *sel_prefix, gint *n_prefices);
GtkWidget* create_prefixed_variable_notebook (GtkWidget *box, GtkSelectionMode mode, GGobiVariableType vartype, datatyped dtype, GtkSignalFunc func, gpointer func_data, ggobid *, GGobiVariableNotebookPrefixFunc prefix_func);
void       ctour_event_handlers_toggle (splotd *, gboolean);
void       ctourpp_window_open (ggobid *);
colorschemed* default_scheme_init ();
gushort    datad_colors_used_get (gint *ncolors_used, gushort *colors_used, GGobiStage *, ggobid *); 
GGobiStage*     datad_get_from_notebook(GtkWidget *notebook);
void       datad_record_id_add (gchar *ids);
gint       delete_vars (gint *, gint, GGobiStage *);
void       disconnect_button_press_signal (splotd *sp);
void       disconnect_button_release_signal (splotd *sp);
void       disconnect_key_press_signal (splotd *sp);
void       disconnect_scroll_signal (splotd *sp);
void       disconnect_motion_signal (splotd *sp);
gint       display_add(displayd *display, ggobid *);
displayd*  display_alloc_init (gboolean, GGobiStage *, ggobid *);
void       display_close (displayd *d);
gboolean   display_copy_edge_options (displayd *dsp, displayd *dspnew);
void       display_delete_cb (GtkWidget *, GdkEvent *, displayd *);
void       display_free (displayd *, gboolean force, ggobid *);
void       display_free_all (ggobid *);
void       display_mode_menus_update (ProjectionMode, InteractionMode, displayd *, ggobid *);
void       display_menu_build (ggobid *);
void       display_menu_init (ggobid *);
void       display_new (ggobid *, guint action, GtkWidget *widget);
void       display_plot (displayd *display, RedrawStyle type, ggobid *);
void       display_print (displayd *d);
void       display_set_current (displayd *, ggobid *);
void       display_set_position (windowDisplayd *d, ggobid *gg);
GtkUIManager *display_menu_manager_create(displayd *d);
void       show_display_control_panel (displayd *display);
void       display_tailpipe (displayd *, RedrawStyle, ggobid *);
void       display_tour1d_init (displayd *dsp, ggobid *gg);
void       display_tour1d_init_null (displayd *dsp, ggobid *gg);
void       display_tour2d3_init (displayd *, ggobid *);
void       display_tour2d3_init_null (displayd *, ggobid *);
void       display_tour2d_init (displayd *dsp, ggobid *gg);
void       display_tour2d_init_null (displayd *dsp, ggobid *gg);
void       display_tourcorr_init (displayd *dsp, ggobid *gg);
void       display_tourcorr_init_null (displayd *dsp, ggobid *gg);
gboolean   display_type_handles_projection (displayd *, ProjectionMode);
gboolean     display_type_handles_interaction (displayd *, InteractionMode);

void       display_window_init (windowDisplayd *, gint, gint, gint, ggobid *);
void       displays_plot (splotd *, RedrawStyle, ggobid *);
void       displays_tailpipe (RedrawStyle, ggobid *);
void       set_display_options(displayd *display, ggobid *gg);
void	     set_display_option(gboolean active, guint action, displayd *display);
void       set_time(ggobid *);
void       run_timing_tests(ggobid *);
gint       do_ash1d (gfloat *, gint, gint, gint, gfloat *, gfloat *, gfloat *, gfloat *);
void       do_last_increment (vector_f, vector_f, gfloat, gint);
void       draw_3drectangle (GtkWidget *w, GdkDrawable *drawable, gint x, gint y, gint width, gint height, ggobid *gg);
void       draw_glyph (GdkDrawable *, glyphd *, icoords *, gint, ggobid *);
gint       dsvd (gdouble **a, gint m, gint n, gfloat *w, gdouble **v);

/* pango utils */
void layout_text(PangoLayout *layout, const gchar *text, PangoRectangle *rect);
void underline_text(PangoLayout *layout);

ProjectionMode pmode_get(displayd *, ggobid *);
InteractionMode imode_get(ggobid *);

gboolean   edge_add (gint, gint, gchar *, gchar *, GGobiStage *, GGobiStage *, ggobid *);
gboolean   edge_endpoints_get (gint k, gint *a, gint *b, GGobiStage *d, endpointsd *endpoints, GGobiStage *e);
void       edgeedit_event_handlers_toggle (splotd *, gboolean);
void       edges_free (GGobiStage *, ggobid *);
void       edgeset_add_cb (GtkAction *action, GGobiStage *e);
gint       edgesets_count (ggobid *gg);
void       eigenvals_get (gfloat *, GGobiStage *);
gint       fcompare (const void *x1, const void *x2);
void       filename_get_r (ggobid *);
void       filename_get_w (GtkWidget *, ggobid *);
guint      *find_keepers (gint ncols_current, GSList *cols, guint *nkeepers);
gint       find_nearest_edge (splotd *sp, displayd *display, ggobid *gg);
gint       find_nearest_point (icoords *, splotd *, GGobiStage *, ggobid *);
gint       free_optimize0_p (optimize0_param *op);
gint       free_pp (pp_param *pp);
GList*     g_list_remove_nth (GList *, gint);
GList*     g_list_replace_nth (GList *, gpointer, gint);
gint       getPreviousDisplays(xmlNodePtr node, GGobiDescription *desc);
GtkWidget* get_tree_view_from_notebook (GtkWidget *);
GtkWidget* get_tree_view_from_object (GObject *);
void       get_extended_brush_corners (icoords *, icoords *, GGobiStage *, splotd *);
gint  	   get_one_selection_from_tree_view (GtkWidget *tree_view, GGobiStage *d);
gint*      get_selections_from_tree_view (GtkWidget *, gint *);
void	   select_tree_view_row(GtkWidget *tree_view, gint row);
gint	   tree_selection_get_selected_row(GtkTreeSelection *tree_sel);
void       gg_write_to_statusbar (gchar *message, ggobid *gg);
void    ggobi_alloc (ggobid *tmp);
gboolean   ggobi_file_set_create (gchar *rootname, GGobiStage *, ggobid *);
ggobid*    ggobi_get (gint);
gint       ggobi_getIndex(ggobid *gg);
void       ggobi_sleep(guint);
void       globals_init (ggobid *);
gboolean   gram_schmidt (gdouble *, gdouble*, gint);
void       gt_basis (array_d, gint, vector_i, gint, gint);
gboolean   hasEdgePoints(GGobiStage *e, GGobiStage *d);
void       identify_event_handlers_toggle (splotd *, gboolean);
gchar *    identify_label_fetch (gint k, cpaneld *cpanel, GGobiStage *d, ggobid *);
void       identify_link_by_id (gint k, GGobiStage *source_d, ggobid *gg);
gboolean   impute_fixed (ImputeType, gfloat val, int nvars, gint *vars, GGobiStage *);
gboolean   impute_mean_or_median (gint, gint, gint *, GGobiStage *);
void       impute_random (GGobiStage *, gint nvars, gint *vars);
void       impute_window_open (ggobid *);
gboolean   in_vector (gint k, gint *vec, gint nels);
gint       include_hiddens (gboolean include, GGobiStage *d, ggobid *gg);
void       increment_tour(vector_f, vector_f, gfloat, gfloat, gfloat *, gint);
void       init_plot_GC (GdkWindow *, ggobid *);
void       init_var_GCs (GtkWidget *, ggobid *);
gdouble    inner_prod (gdouble *, gdouble *, gint);
gboolean   isCrossed (gdouble, gdouble, gdouble, gdouble, gdouble, gdouble, gdouble, gdouble);
gfloat     jitter_randval (gint);
void       jitter_value_set (gfloat, GGobiStage *, ggobid *);
void       jitter_vars_init (GGobiStage *);
void       jitter_window_open (ggobid *);
void       limits_adjust (gfloat *, gfloat *);
void       limits_display_set_by_var (GGobiStage *d, gint j, gboolean);
void       limits_set (GGobiStage *, gboolean);  
void       limits_set_by_var (GGobiStage *, guint, gboolean);  
gint       lines_intersect (glong, glong, glong, glong, glong, glong, glong, glong);
gint       lwidth_from_gsize(gint size);
gint       ltype_from_gtype(gint type);
gint       set_lattribute_from_ltype(gint, ggobid *);
void       linkby_current_page_set (displayd *, GtkWidget *w, ggobid *);
void       linking_method_set (displayd *, GGobiStage *, ggobid *);
void       make_ggobi (GGobiOptions *, gboolean, ggobid *);
void       make_symbol_window (ggobid *);
void       make_ui (ggobid *);
GlyphType  mapGlyphName (const gchar *gtype);
void       missing_arrays_add_cols (GGobiStage *d);
void       missings_datad_cb (GtkWidget *w, ggobid *gg);
gboolean   mouseinwindow (splotd *);
void       mousepos_get_motion (GtkWidget *, GdkEventMotion *, gboolean *, gboolean *, splotd *);
void       mousepos_get_pressed (GtkWidget *, GdkEventButton *, gboolean *, gboolean *, splotd *);
void       move_pt (gint id, gint x, gint y, splotd *sp, GGobiStage *d, ggobid *);
void       movepts_event_handlers_toggle (splotd *, gboolean);
void       movepts_history_add (gint id, splotd *sp, GGobiStage *, ggobid *);
void       movepts_history_delete_last (GGobiStage *, ggobid *);
gint       ndatad_with_vars_get (ggobid *gg);
void       norm (gdouble *, gint);
GtkWidget* create_menu_bar (GtkUIManager *, const gchar *, GtkWidget *);
void       p1d_event_handlers_toggle (splotd *, gboolean);
void       p1d_reproject (splotd *, greal **, GGobiStage *, ggobid *);
gboolean   p1d_varsel (splotd *, gint, gint *, gint, gint);
gint       p1dcycle_func (ggobid *gg);
void       pan_by_drag (splotd *, ggobid *);
void       parcoords_cpanel_init (cpaneld*, ggobid *);
const gchar *parcoords_mode_ui_get(displayd *dsp);
displayd*  parcoords_new_with_vars (gboolean, gint, gint *, GGobiStage *, ggobid *);
displayd*  parcoords_new (displayd *dpy, gboolean, gint, gint *, GGobiStage *, ggobid *);
void       parcoords_reset_arrangement (displayd *, gint, ggobid *);
gboolean   parcoords_varsel (cpaneld *, splotd *, gint, gint *, ggobid *);
void       parcoordsDragAndDropEnable(displayd *dsp, gboolean active);
gboolean   pca_calc (GGobiStage *, ggobid *);
void       pca_diagnostics_set (GGobiStage *d, ggobid *);
gint       pcompare (const void *, const void *);
gint       rank_compare (gconstpointer, gconstpointer, gpointer);
void       pipeline_arrays_alloc (GGobiStage *);
void       pipeline_arrays_check_dimensions (GGobiStage *d);
void       pipeline_init (GGobiStage *);
gint       plotted_cols_get (gint *, GGobiStage *, ggobid *);
gboolean   point_in_which_bin (gint, gint, gint *, gint *, GGobiStage *, splotd *);
void       populate_combo_box (GtkWidget *, gchar **, gint, GCallback, gpointer);
void       populate_tree_view(GtkWidget *tree_view, gchar **lbl, gint nitems, gboolean headers, GtkSelectionMode mode, GCallback func, gpointer obj);
gboolean   processRestoreFile(const gchar * const fileName, ggobid *gg);
void       procs_activate(gboolean state, ProjectionMode pmode, displayd *display, ggobid *gg);
gboolean   projection_ok (ProjectionMode m, displayd *display);
void       quick_message (const gchar * const, gboolean);
void       quit_ggobi(ggobid *gg);
gdouble    randvalue (void);
void       range_unset (ggobid *gg);
gboolean   reached_target(gfloat, gfloat, gint, gfloat *, gfloat *);
gint       realloc_optimize0_p(optimize0_param *, gint, vector_i);
void       recenter_data (gint, GGobiStage *, ggobid *);
gboolean   record_add (eeMode, gint a, gint b, gchar *lbl, gchar *id, gchar **vals, GGobiStage * d, GGobiStage * e, ggobid *gg);
void       reinit_transient_brushing (displayd *, ggobid *);
void       rejitter (gint *, gint, GGobiStage *, ggobid *);
void       reset_pp(GGobiStage *, ggobid *);
void       rnorm2 (gdouble *, gdouble *);
void       rotation_event_handlers_toggle (splotd *, gboolean);
void       ruler_ranges_set (gboolean force, displayd *, splotd *, ggobid *);
void       scale_event_handlers_toggle (splotd *, gboolean);
void       scale_set_default_values (GtkScale *scale);
void	   scale_zoom_reset (displayd *dsp);
void	   scale_pan_reset (displayd *display);
void       scale_update_set (gboolean, displayd *, ggobid *);
void       scaling_visual_cues_draw (splotd *, GdkDrawable *, ggobid *);
void       scatmat_cpanel_init (cpaneld *, ggobid *);
displayd*  scatmat_new (displayd *, gboolean, gint, gint *, gint, gint *, GGobiStage *, ggobid *);
gboolean   scatmat_varsel (GtkWidget *, cpaneld *, splotd *, gint, gint, gint *, gint, gboolean, ggobid *);
gboolean   scatmat_varsel_simple (cpaneld *, splotd *, gint, gint *, ggobid *);
const gchar* scatmat_mode_ui_get(displayd *display);

void       scatterplot_cpanel_init (cpaneld *, ProjectionMode, InteractionMode, ggobid *); 

void       scatterplot_display_edge_menu_update (displayd *, GtkAccelGroup *accel_group, ggobid *gg);
const gchar * scatterplot_mode_ui_get(displayd *display);		
displayd*  scatterplot_new (gboolean, splotd *sp, GGobiStage *d, ggobid *);
void       scatterplot_show_hrule (displayd *, gboolean show);
void       scatterplot_show_rulers (displayd *, gint);
void       scatterplot_show_vrule (displayd *, gboolean show);
gboolean   scree_mapped_p (ggobid *);
void       scree_plot_make (ggobid *);
gint       selected_cols_get (gint **, GGobiStage *d, ggobid *);
void       smooth_window_open (ggobid *);
void       sp_event_handlers_toggle (splotd *, gboolean, ProjectionMode, InteractionMode);
void       sp_whiskers_make (splotd *, displayd *, ggobid *);
void       special_colors_init (ggobid *);
void       speed_set (gfloat, gfloat *, gfloat *);
void       sphere_condnum_set (gfloat x, ggobid *);
void       sphere_enable (gboolean sens, ggobid *);
void       sphere_free (GGobiStage *);
void       sphere_init (GGobiStage *);
void       sphere_npcs_range_set (gint n, ggobid *gg);
void       sphere_npcs_set (gint, GGobiStage *, ggobid *);
void       sphere_panel_open (ggobid *);
void       sphere_varcovar_set (GGobiStage *, ggobid *);
void       sphere_variance_set (gfloat x, GGobiStage *, ggobid*);
void       spherevars_set (ggobid *);
void       spherize_data (vector_i *svars, vector_i *pcvars, GGobiStage *, ggobid *);
gboolean   spherize_set_pcvars (GGobiStage *, ggobid *);
void       splash_show (ggobid *gg);
void       splot_add_diamond_cue (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg);
void       splot_add_edge_highlight_cue (splotd *, GdkDrawable *, gint k, gboolean nearest, ggobid *);
void       splot_add_edge_label (splotd *, GdkDrawable *, gint k, gboolean nearest, ggobid *);
void       splot_add_edgeedit_cues (splotd *, GdkDrawable *, gint k, gboolean nearest, ggobid *);
void       splot_add_identify_edge_cues (splotd *sp, GdkDrawable *drawable, gint k, gboolean nearest, ggobid *gg);
void       splot_add_point_label (gboolean, gint, gboolean, splotd *, GdkDrawable *, ggobid *);
void       splot_cursor_set (gint jcursor, splotd *sp);
void       splot_dimension_set(splotd* sp, gint width, gint height);
void       splot_draw_tour_axes(splotd *sp, GdkDrawable *, ggobid *);
void       splot_edges_draw (splotd *sp, gboolean hiddens_p, GdkDrawable *drawable, ggobid *gg);
void       splot_edges_realloc (gint, splotd *, GGobiStage *);
gboolean   splot_event_handled (GtkWidget *, GdkEventKey *, cpaneld *, splotd *, ggobid *);
void       splot_expose (splotd *);
void       splot_free (splotd *, displayd *, ggobid *);
void       splot_get_dimensions (splotd *, gint *, gint *);
splotd*    splot_new (displayd *, gint, gint, ggobid *);
void       splot_pixmap0_to_pixmap1 (splotd *, gboolean, ggobid *);
void       splot_plane_to_screen (displayd *, cpaneld *, splotd *, ggobid *);
gboolean   splot_plot_case (gint m, GGobiStage *, splotd *, displayd *, ggobid *);
void       splot_points_realloc (gint, splotd *, GGobiStage *);
void       splot_redraw (splotd *sp, RedrawStyle, ggobid *);
void       splot_set_current (splotd *, gboolean, ggobid *);
void       splot_world_to_plane (cpaneld *, splotd *, ggobid *);
void       splot_zoom (splotd *sp, gfloat xsc, gfloat ysc) ;
gint       sqdist (gint, gint, gint, gint);
void       statusbar_show (gboolean show, ggobid *gg);
void       sticky_id_link_by_id (gint, gint, GGobiStage *, ggobid *);
void       sticky_id_toggle (GGobiStage *, ggobid *);
void       submenu_destroy (GtkWidget *);
void       submenu_insert (GtkWidget *, GtkWidget *, gint);
GtkWidget* submenu_make (gchar *, guint, GtkAccelGroup *);
void       subset_window_open (ggobid *);
gboolean brush_all_matching_id (GGobiStage * sd, gint k, gboolean condition, BrushTargetType brush_mode, GGobiAttrSetMethod brush);
gint       symbol_table_populate (GGobiStage *d);
void       symbol_window_redraw (ggobid *);
void       t1d_clear_ppda (displayd *, ggobid *);
void       t1d_optimz (gint, gboolean *, gint *, displayd *);
void       t1d_pp_reinit(displayd *, ggobid *);
void       t1d_ppcool_set(gfloat, displayd *, ggobid *);
void       t1d_ppdraw(gfloat, displayd *, ggobid *);
void       t1d_pptemp_set(gfloat, displayd *, ggobid *);
void       t2d_clear_ppda (displayd *, ggobid *);
void       t2d_optimz (gint, gboolean *, gint *, displayd *);
void       t2d_pp_reinit(displayd *, ggobid *);
void       t2d_ppcool_set(gfloat, displayd *, ggobid *);
void       t2d_ppdraw (gfloat, displayd *, ggobid *);
void       t2d_pptemp_set(gfloat, displayd *, ggobid *);
void       textur (gfloat *, gfloat *, gint, gint, gfloat, gint, ggobid *);
void       tform_to_world (GGobiStage *);
void       tform_to_world_by_var (GGobiStage *, guint j);
void       tooltips_show (gboolean show, ggobid *gg);
void       tour1d_all_vars (displayd *);
void       tour1d_do_step (displayd *,ggobid *);
void       tour1d_event_handlers_toggle (splotd *, gboolean);
void       tour1d_fade_vars (gboolean, ggobid *);
void       tour1d_func (gboolean, displayd *, ggobid *);
void       tour1d_io_cb (GtkWidget *w, gpointer *cbd);
void       tour1d_manip (gint, gint, splotd *, ggobid *);
void       tour1d_manip_end (splotd *);
void       tour1d_manip_init (gint, gint, splotd *);
void       tour1d_pause (cpaneld *, gboolean, displayd *, ggobid *);
void       tour1d_projdata (splotd *, greal **, GGobiStage *, ggobid *);
void       tour1d_realloc_down (GSList *cols, GGobiStage *d, ggobid *gg);
void       tour1d_reinit (ggobid *);
void       tour1d_scramble(ggobid *);
void       tour1d_snap(ggobid *);
void       tour1d_video(ggobid *);
void       tour1d_speed_set (gfloat, ggobid *);
gboolean   tour1d_varsel (GtkWidget *, gint jvar, gint toggle, gint btn, GGobiStage *, ggobid *);
void       tour1d_vert (cpaneld *, gboolean);
void       tour1dpp_window_open (ggobid *);
void       tour2d3_event_handlers_toggle (splotd *, gboolean state);
void       tour2d3_func (gboolean state, displayd *, ggobid *);
void       tour2d3_manip (gint, gint, splotd *, ggobid *);
void       tour2d3_manip_end (splotd *);
void       tour2d3_manip_init (gint, gint, splotd *);
void       tour2d3_pause (cpaneld *, gint, ggobid *);
void       tour2d3_projdata (splotd *, greal **world_data, GGobiStage *, ggobid *);
void       tour2d3_reinit (ggobid *);
void       tour2d3_scramble (ggobid *);
void       tour2d3_speed_set (gfloat, ggobid *);
gboolean   tour2d3_varsel (GtkWidget *, gint jvar, gint toggle, gint btn, GGobiStage *, ggobid *);
void       tour2d_all_vars (displayd *);
void       tour2d_do_step (displayd *,ggobid *);
void       tour2d_event_handlers_toggle (splotd *, gboolean);
void       tour2d_fade_vars (gboolean, ggobid *);
void       tour2d_func (gboolean, displayd *, ggobid *);
void       tour2d_io_cb (GtkWidget *w, gpointer *cbd);
void       tour2d_manip (gint, gint, splotd *, ggobid *);
void       tour2d_manip_end (splotd *);
void       tour2d_manip_init (gint, gint, splotd *);
void       tour2d_pause (cpaneld *, gboolean, displayd *, ggobid *);
void       tour2d_projdata (splotd *, greal **, GGobiStage *, ggobid *);
void       tour2d_realloc_down (GSList *cols, GGobiStage *d, ggobid *gg);
void       tour2d_reinit (ggobid *);
void       tour2d_scramble (ggobid *);
void       tour2d_snap(ggobid *);
void       tour2d_video(ggobid *);
void       tour2d_speed_set (gfloat, ggobid *);
gboolean   tour2d_varsel (GtkWidget *, gint jvar, gint toggle, gint btn, GGobiStage *, ggobid *);
void       tour2dpp_window_open (ggobid *);
gint       tour_path (array_d, array_d, array_d, gint, gint, array_d, array_d, array_d, vector_f, array_d, array_d, array_d, vector_f, vector_f, gfloat *, gfloat *);
void tour_realloc_up (GGobiStage *d, gint nc);
void       tour_reproject (vector_f, array_d, array_d, array_d, array_d, array_d, gint, gint);
void       tourcorr_fade_vars (gboolean, ggobid *);
void       tourcorr_func (gboolean, displayd *, ggobid *);
void       tourcorr_io_cb (GtkWidget *w, gpointer *cbd);
void       tourcorr_manip (gint, gint, splotd *, ggobid *);
void       tourcorr_manip_end (splotd *);
void       tourcorr_manip_init (gint, gint, splotd *);
void       tourcorr_pause (cpaneld *, gboolean, ggobid *);
void       tourcorr_projdata (splotd *, greal **, GGobiStage *, ggobid *);
void       tourcorr_realloc_down (GSList *cols, GGobiStage *d, ggobid *gg);
void       tourcorr_reinit (ggobid *);
void       tourcorr_scramble (ggobid *);
void       tourcorr_snap(ggobid *);
void       tourcorr_video(ggobid *);
void       tourcorr_speed_set (gfloat, ggobid *);
gboolean   tourcorr_varsel (GtkWidget *, gint jvar, gint toggle, gint btn, GGobiStage *, ggobid *);
void       transform (gint, gint, gfloat, gint *, gint, GGobiStage *, ggobid *);
void       transform0_combo_box_set_value (gint, gboolean, GGobiStage *, ggobid *);
void       transform0_values_set (gint, gint, GGobiStage *, ggobid *);
gboolean   transform1_apply (gint, GGobiStage *, ggobid *);
void       transform1_combo_box_set_value (gint, gboolean, GGobiStage *, ggobid *);
void       transform1_values_set (gint, gfloat, gint, GGobiStage *, ggobid *);
gboolean   transform2_apply (gint, GGobiStage *, ggobid *);
void       transform2_combo_box_set_value (gint, gboolean, GGobiStage *, ggobid *);
void       transform2_values_set (gint, gint, GGobiStage *, ggobid *);
gboolean   transform_values_compare (gint, gint, GGobiStage *);
void       transform_values_copy (gint jfrom, gint jto, GGobiStage *d);
void       transform_values_init (GGobiVariable *var);
gboolean   transform_variable (gint, gint, gfloat, gint, GGobiStage *, ggobid *);
void       varcircle_label_set (GGobiStage *s, gint jvar);
void       varcircles_add (gint ncols, GGobiStage *, ggobid *);
void       varcircles_cursor_set_default (GGobiStage *d);
void       varcircles_delete_nth (GGobiStage *, guint j);
void       varcircles_populate (GGobiStage *, ggobid *);
void       varcircles_refresh (GGobiStage *, ggobid *);
void       varcircles_show (gboolean, GGobiStage *, displayd *, ggobid *);
void       varcircles_visibility_set (displayd *display, ggobid *gg);
void       variable_notebook_handlers_disconnect (GtkWidget *notebook, ggobid *gg);
void       variable_notebook_list_changed_cb(ggobid *gg, GGobiStage *d, void *notebook);
void       variable_notebook_subwindow_add (GGobiStage *d, GtkSignalFunc func, gpointer func_data, GtkWidget *notebook, GGobiVariableType, datatyped, const gchar*, ggobid *gg);
void       variable_notebook_varchange_cb (ggobid *gg, gint which, GGobiStage *, void *notebook);
void       varpanel_clear (GGobiStage *, ggobid *);
void       varpanel_delete_nth (GGobiStage *d, gint jvar);
void       varpanel_label_set (GGobiStage *, gint);
void       varpanel_make (GtkWidget *, ggobid *);
void       varpanel_populate (GGobiStage *, ggobid *);
void       varpanel_refresh (displayd *, ggobid *);
void       varpanel_reinit (ggobid *gg);
void       varpanel_set_sensitive (GGobiStage *d, gboolean sensitive_p, ggobid *);
void       varpanel_show_page (displayd*, ggobid*);
void       varpanel_tooltips_set (displayd *, ggobid *);
GtkWidget* varpanel_widget_get_nth (gint jbutton, gint jvar, GGobiStage *d);
void       varpanel_widgets_add (gint nc, GGobiStage *d, ggobid *gg);
void       vars_stdized_send_event (GGobiStage *d, ggobid *gg);
void       varsel (GtkWidget *w, cpaneld *, splotd *, gint jvar, gint toggle, gint btn, gint alt_mod, gint ctrl_mod, gint shift_mod, GGobiStage *, ggobid *);
void       vartable_alloc (GGobiStage *);
gint       vartable_index_get_by_name(gchar *name, GGobiStage *d);
gboolean   vartable_iter_from_varno(gint var, GGobiStage *d, GtkTreeModel **model, GtkTreeIter *iter);
gint	   vartable_varno_from_path(GtkTreeModel *model, GtkTreePath *path);
void       vartable_cells_set_by_var (gint j, GGobiStage *d);
GtkWidget*  vartable_tree_view_get (ggobid *gg);
void       vartable_collab_set_by_var (GGobiStage *, guint);
void       vartable_collab_tform_set_by_var (GGobiStage *s, guint j);
GGobiVariable *vartable_copy_var (GGobiVariable *var, GGobiVariable *var_to);
GGobiVariable* vartable_element_new (GGobiStage *d);
void       vartable_element_remove (gint, GGobiStage *);
void       vartable_init (GGobiStage *d);
void       vartable_limits_set (GGobiStage *);
void       vartable_limits_set_by_var (GGobiStage *s, guint j);
void       vartable_open (ggobid *);
void       vartable_row_append (gint j, GGobiStage *);
void       vartable_show_page (GGobiStage*, ggobid*);
void       vartable_stats_set (GGobiStage *);
void       vartable_stats_set_by_var (GGobiStage *, guint j);
void       vectorb_alloc (vector_b *, gint);
void       vectorb_alloc_zero (vector_b *, gint);
void       vectorb_copy (vector_b *, vector_b *);
void       vectorb_delete_els (vector_b *vecp, GSList *els);
void       vectorb_free (vector_b *);
void       vectorb_init_null (vector_b *);
void       vectorb_realloc (vector_b *, gint);
void       vectorb_realloc_zero (vector_b *, gint);
void       vectorb_zero (vector_b *vecp);
void       vectord_alloc (vector_d *, gint);
void       vectord_alloc_zero (vector_d *, gint);
void       vectord_delete_els (vector_d *vecp, GSList *els);
void       vectord_free (vector_d *);
void       vectord_init_null (vector_d *);
void       vectord_realloc (vector_d *, gint);
void       vectord_zero (vector_d *vecp);
void       vectorf_alloc (vector_f *, gint);
void       vectorf_alloc_zero (vector_f *, gint);
void       vectorf_delete_els (vector_f *vecp, GSList *els);
void       vectorf_free (vector_f *);
void       vectorf_init_null (vector_f *);
void       vectorf_realloc (vector_f *, gint);
void       vectorf_zero (vector_f *vecp);
void       vectorg_alloc (vector_g *, gint);
void       vectorg_copy (vector_g *, vector_g *);
void       vectorg_free (vector_g *);
void       vectorg_init_null (vector_g *);
void       vectorg_realloc (vector_g *, gint);
void       vectorg_realloc_zero (vector_g *, gint);
void       vectori_alloc (vector_i *, gint);
void       vectori_alloc_zero (vector_i *, gint);
void       vectori_copy (vector_i *, vector_i *);
void       vectori_delete_els (vector_i *vecp, GSList *els);
void       vectori_free (vector_i *);
void       vectori_init_null (vector_i *);
void       vectori_realloc (vector_i *, gint);
void       vectori_zero (vector_i *vecp);
void       vectors_copy (vector_s *, vector_s *);
void       vectors_free (vector_s *);
void       vectors_init_null (vector_s *);
void       vectors_realloc (vector_s *, gint);
void       vectors_realloc_zero (vector_s *, gint);
GtkWidget* widget_find_by_name (GtkWidget *, gchar *);
void       widget_initialize (GtkWidget *w, gboolean initd);
gboolean   widget_initialized (GtkWidget *w);
void       writeall_window_open (ggobid *);
void       svis_init (ggobid *gg);
void       svis_window_open (ggobid *gg);
gboolean   write_csv(const gchar *, ggobid *);
void       wvis_init (ggobid *gg);
void       wvis_window_open (ggobid *gg);
void       xy_reproject (splotd *, greal **, GGobiStage *, ggobid *);
gint       xycycle_func (ggobid *gg);
void       xyplot_cycle_activate (gboolean state, cpaneld *cpanel, ggobid *gg);
void       xyplot_event_handlers_toggle (splotd *, gboolean);
gboolean   xyplot_varsel (splotd *, gint, gint *, gint, gint);
void       zoom_by_drag (splotd *, ggobid *);

/*tsplot functions*/
GtkWidget* cpanel_tsplot_make (ggobid *);
void      cpanel_tsplot_set (displayd *, cpaneld *, GtkWidget *, ggobid *);
void      tsplot_cpanel_init (cpaneld*, ggobid *);

const gchar* tsplot_mode_ui_get(displayd *display);
displayd* tsplot_new (displayd *, gboolean, gint, gint *, GGobiStage *, ggobid *);
displayd *tsplot_new_with_vars (gboolean missing_p, gint nvars, gint *vars, GGobiStage *d, ggobid *gg) ;
void      tsplot_reset_arrangement (displayd *, gint, ggobid *);
gboolean  tsplot_varsel (GtkWidget *, displayd *display, splotd *sp, gint jvar, gint toggle, gint mouse, cpaneld *cpanel,  ggobid *gg);
void      tsplot_whiskers_make (splotd *, displayd *, ggobid *);
void      zero_tau(vector_f, gint);

/* The new way of handling window closures, so that we don't just exit. */
gboolean  ggobi_close (ggobid *gg);
gint      ggobi_remove_by_index (ggobid *gg, gint which);
gint      ggobi_remove (ggobid *);
void      subset_init (GGobiStage *d, ggobid *gg);


displayd *createBarchart(displayd *display, gboolean missing_p, splotd * sp, gint var, GGobiStage * d,  ggobid * gg);
void      barchart_scaling_visual_cues_draw (splotd *sp, GdkDrawable *drawable, ggobid *gg);
gboolean  barchart_active_paint_points (splotd *sp, GGobiStage *, ggobid *); 
void      barchart_add_bar_cues (splotd *sp, GdkDrawable *drawable, ggobid *gg);
void      barchart_clean_init (barchartSPlotd *sp);
void      barchart_cpanel_init (cpaneld *, ggobid *);
void      barchart_event_handlers_toggle (displayd *, splotd *, gboolean state, ProjectionMode, InteractionMode);
void      barchart_free_structure (barchartSPlotd *sp);
gboolean  barchart_identify_bars (icoords mousepos, splotd *sp, GGobiStage *d, ggobid *gg);
void      barchart_init_vectors(barchartSPlotd *sp);
const gchar *barchart_mode_ui_get(displayd *display);
displayd *barchart_new (gboolean missing_p, splotd *sp, GGobiStage *d, ggobid *gg);
void      barchart_recalc_counts (barchartSPlotd *sp, GGobiStage *d, ggobid *gg);
void      barchart_recalc_dimensions (splotd *sp, GGobiStage *d, ggobid *gg);
void      barchart_recalc_group_dimensions (barchartSPlotd *sp, ggobid *gg);
gboolean  barchart_redraw (splotd *sp, GGobiStage *d, ggobid *gg, gboolean binned);
void      barchart_splot_add_plot_labels (splotd *, GdkDrawable *, ggobid *);
GtkWidget* cpanel_barchart_make (ggobid *gg);
void      cpanel_barchart_set (displayd *, cpaneld *, GtkWidget *panel, ggobid *gg);

void      barchart_scale_event_handlers_toggle(splotd *sp, gboolean state);

displayd *barchart_new_with_vars(gboolean missing_p, gint nvars, gint *vars, GGobiStage * d, ggobid * gg);

#ifdef WIN32
void      win32_draw_to_pixmap_binned (icoords *, icoords *, gint, splotd *, gboolean draw_hidden, ggobid *gg);
void      win32_draw_to_pixmap_unbinned (gint, splotd *, gboolean draw_hidden, ggobid *gg);
void      win32_drawing_arrays_free (splotd *sp);
#endif

#ifdef __cplusplus
}
#endif

#include "GGobiAPI.h"

#define CHECK_GG(a) ValidateGGobiRef(a, true)

/* Made externs for access from display class methods. */
void       varpanel_toggle_set_active (gint jbutton, gint jvar, gboolean active, GGobiStage *d);
GtkWidget *varpanel_widget_set_visible (gint jbutton, gint jvar, gboolean show, GGobiStage *d);

void       display_plot (displayd *display, RedrawStyle type, ggobid *gg);
void       scatterXYAddPlotLabels(splotd *sp, GdkDrawable *drawable, GdkGC *gc);
void       scatter1DAddPlotLabels(splotd *sp, GdkDrawable *drawable, GdkGC *gc);
gboolean   processRestoreFile(const gchar * const fileName, ggobid *gg);
void       scatterplotMovePointsMotionCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventMotion *event, ggobid *gg);
void       scatterplotMovePointsButtonCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventButton *event, ggobid *gg);
displayd * scatterplot_new_with_vars(gboolean missing_p, gint numVars, gint *vars, GGobiStage *d, ggobid *gg);

GGobiOptions *GGOBI_getSessionOptions();

#ifdef WIN32
/* needed by transform.c */
extern double erf(double x);
extern double erfc(double x);
#endif

void resetDataMode();

gboolean parcoords_add_delete_splot(cpaneld *cpanel, splotd *sp, gint jvar, gint *jvar_prev, ggobid *gg, displayd *display);

#ifdef ENABLE_CAIRO
cairo_t*   create_cairo_glitz(GdkDrawable *drawable);
#endif

#endif

void
pt_world_to_raw_by_var (gint j, greal * world, greal * raw, GGobiStage * d);

void
pt_screen_to_raw (icoords * screen, gint id, gboolean horiz, gboolean vert,
                  greal * raw, gcoords * eps, GGobiStage * d, splotd * sp,
                  ggobid * gg);
void
pt_plane_to_world (splotd * sp, gcoords * planar, gcoords * eps,
                   greal * world);
void
pt_screen_to_plane (icoords * screen, gint id, gboolean horiz, gboolean vert,
                   gcoords * eps, gcoords * planar, splotd * sp);
endpointsd *
resolveEdgePoints (GGobiStage * e, GGobiStage * d);
void
unresolveAllEdgePoints (GGobiStage * e);
void
edges_alloc (gint nsegs, GGobiStage * d);
