#ifndef GGOBI_EXTERNS_H
#define GGOBI_EXTERNS_H

#include <stdio.h>

#ifdef USE_CLASSES
extern datad *datad_new (ggobid *);
#endif

extern datad *datad_new(datad *d, ggobid *gg);

#ifdef __cplusplus
extern "C" {
#endif

/* sort +2 */
extern GtkWidget* CreateMenuCheck (displayd *, GtkWidget *, gchar *, GtkSignalFunc, gpointer, gboolean, ggobid *);
extern GtkWidget* CreateMenuItem (GtkWidget *, gchar *, gchar *, gchar *, GtkWidget *, GtkAccelGroup *, GtkSignalFunc, gpointer, ggobid *) ;
extern ggobid* GGobiFromDisplay (displayd *display);
extern ggobid* GGobiFromSPlot (splotd *sp);
extern ggobid* GGobiFromWidget (GtkWidget *w, gboolean);
extern ggobid* GGobiFromWindow (GdkWindow *w);
extern void GGobi_widget_set (GtkWidget *, ggobid *gg, gboolean isWindow);
extern ggobid* ValidateGGobiRef (ggobid *gg, gboolean fatal);
extern void array_read (datad *, ggobid *);
extern void arrayf_add_cols (array_f *, gint);
extern void arrayf_add_rows (array_f *, gint);
extern void arrayf_alloc (array_f *, gint, gint);
extern void arrayf_alloc_zero (array_f *, gint, gint);
extern void arrayf_copy (array_f *, array_f *);
extern void arrayf_delete_cols (array_f *, gint, gint *);
extern void arrayf_free (array_f *, gint, gint);
extern void arrayf_init (array_f *);
extern void arrayf_zero (array_f *);
extern void arrayl_add_cols (array_l *, gint);
extern void arrayl_add_rows (array_l *, gint);
extern void arrayl_alloc (array_l *, gint, gint);
extern void arrayl_alloc_zero (array_l *, gint, gint);
extern void arrayl_delete_cols (array_l *, gint, gint *);
extern void arrayl_free (array_l *, gint, gint);
extern void arrayl_init (array_l *);
extern void arrayl_zero (array_l *);
extern void arrays_add_cols (array_s *, gint);
extern void arrays_add_rows (array_s *, gint);
extern void arrays_alloc (array_s *, gint, gint);
extern void arrays_alloc_zero (array_s *, gint, gint);
extern void arrays_delete_cols (array_s *, gint, gint *);
extern void arrays_free (array_s *, gint, gint);
extern void arrays_init (array_s *);
extern void arrays_zero (array_s *);
extern void assign_points_to_bins (datad *, ggobid *);
extern void br_color_ids_alloc (datad *, ggobid *);
extern void br_color_ids_init (datad *, ggobid *);
extern void br_glyph_ids_alloc (datad *, ggobid *);
extern void br_glyph_ids_init (datad *, ggobid *);
extern void br_line_color_alloc (ggobid *);
extern void br_line_color_init (ggobid *);
extern void br_line_vectors_check_size (gint, ggobid *);
extern void brush_activate (gboolean, datad *d, ggobid *);
extern void brush_draw_brush (splotd *, datad *d, ggobid *);
extern void brush_draw_label (splotd *, datad *d, ggobid *);
extern void brush_event_handlers_toggle (splotd *, gboolean);
extern void brush_init (datad *, ggobid *);
extern void brush_menus_make (ggobid *);
extern void brush_motion (icoords *, gboolean, gboolean, cpaneld *, datad *, ggobid *);
extern gboolean brush_once (gboolean force, datad *d, ggobid *);
extern void brush_options_cb (gpointer, guint, GtkCheckMenuItem *);
extern void brush_pos_init (datad *);
extern void brush_set_pos (gint, gint, datad *, ggobid *);
extern void cluster_free (gint, datad *, ggobid *);
extern void clusters_set (datad *, ggobid *);
extern gboolean collabels_read (gchar *, gboolean, datad *, ggobid *);
extern void color_table_init (ggobid *);
extern gchar* computeTitle (gboolean, displayd *, ggobid *);
extern void cpanel_brush_init (cpaneld *, ggobid *);
extern void cpanel_brush_make (ggobid *);
extern void cpanel_brush_set (cpaneld *, ggobid *);
extern void cpanel_ctour_make (ggobid *);
extern void cpanel_identify_make (ggobid *);
extern void cpanel_lineedit_make (ggobid *);
extern void cpanel_movepts_make (ggobid *);
extern void cpanel_p1d_init (cpaneld *, ggobid *);
extern void cpanel_p1d_set (cpaneld *, ggobid *);
extern void cpanel_p1dplot_make (ggobid *);
extern void cpanel_parcoords_init (cpaneld *, ggobid *);
extern void cpanel_parcoords_make (ggobid *);
extern void cpanel_rotation_init (cpaneld *, ggobid *);
extern void cpanel_rotation_make (ggobid *);
extern void cpanel_rotation_set (cpaneld *, ggobid *);
extern void cpanel_scale_init (cpaneld *, ggobid *);
extern void cpanel_scale_make (ggobid *);
extern void cpanel_scatmat_init (cpaneld *, ggobid *);
extern void cpanel_scatmat_make (ggobid *);
extern void cpanel_set (displayd *, ggobid *);
extern void cpanel_tour1d_make (ggobid *);
extern void cpanel_tour2d_make (ggobid *);
extern void cpanel_tour_init (cpaneld *, ggobid *);
extern void cpanel_xyplot_make (ggobid *);
extern void ctour_event_handlers_toggle (splotd *, gboolean);
extern void ctourpp_window_open (ggobid *);
extern void datad_free (datad *, ggobid *);
extern datad* datad_get_from_notebook (GtkWidget *notebook, ggobid *);
extern displayd* datad_init (datad *, ggobid *, gboolean);
extern gint display_add(displayd *display, ggobid *);
extern void display_close_cb (displayd *d, guint, GtkWidget *);
extern displayd* display_create (gint displaytype, gboolean missing_p, datad *, ggobid *);
extern void display_delete_cb (GtkWidget *, GdkEvent *, displayd *);
extern void display_free (displayd *, gboolean force, ggobid *);
extern void display_free_all (ggobid *);
extern void display_menu_build (ggobid *);
extern void display_menu_init (ggobid *);
extern void display_new (ggobid *, guint action, GtkWidget *widget);
extern void display_options_cb (GtkCheckMenuItem *w, guint action);
extern void display_plot (displayd *display, guint type, ggobid *);
extern void display_print_cb (displayd *d, guint, GtkWidget *);
extern void display_set_current (displayd *, ggobid *);
extern void display_set_position (displayd *d, ggobid *gg);
extern void display_tailpipe (displayd *, ggobid *);
extern void display_tailpipe (displayd *, ggobid *);
extern void display_tour_init (displayd *, ggobid *);
extern void display_window_init (displayd *, gint, ggobid *);
extern void displays_plot (splotd *, gint, ggobid *);
extern void displays_tailpipe (gint, ggobid *);
extern gint do_ash1d (gfloat *, gint, gint, gint, gfloat *, gfloat *, gfloat *);
extern void draw_glyph (GdkDrawable *, glyphv *, icoords *, gint, ggobid *);
extern gint dsvd (gfloat **a, gint m, gint n, gfloat *w, gfloat **v);
extern void edges_alloc (gint, datad *, ggobid *);
extern void edges_create (datad *, ggobid *);
extern void edges_free (datad *, ggobid *);
extern gboolean edges_read (gchar *, gboolean, datad *, ggobid *);
extern void eigenvals_get (gfloat *, datad *, ggobid *);
extern void exclusion_window_open (ggobid *);
extern gint fcompare (const void *x1, const void *x2);
extern void filename_get_r (ggobid *, guint, GtkWidget *);
extern void filename_get_w (GtkWidget *, ggobid *);
extern gboolean fileset_read (gchar *, ggobid *);
extern gboolean fileset_read_init (gchar *ldata_in, ggobid *);
extern void find_glyph_type_and_size (gint, glyphv *);
extern gint find_keepers (gint ncols_current, gint nc, gint *cols, gint *keepers);
extern gint find_nearest_point (icoords *, splotd *, datad *, ggobid *);
extern GList* g_list_remove_nth (GList *, gint);
extern GList* g_list_replace_nth (GList *, gpointer, gint);
extern void get_extended_brush_corners (icoords *, icoords *, datad *, ggobid *);
extern void get_main_menu (GtkItemFactoryEntry[], gint, GtkAccelGroup *, GtkWidget  *, GtkWidget **, gpointer);
extern ggobid* ggobi_alloc (void);
extern gboolean ggobi_file_set_create (gchar *rootname, datad *, ggobid *);
extern void globals_init (ggobid *);
extern void hidden_alloc (datad *, ggobid *);
extern void hidden_init (datad *, ggobid *);
extern gboolean hidden_read (gchar *, gboolean, datad *, ggobid *);
extern void identify_event_handlers_toggle (splotd *, gboolean);
extern void identify_menus_make (ggobid *);
extern gboolean impute_fixed (gint, datad *, ggobid *);
extern void impute_random (datad *, ggobid *);
extern void impute_window_open (ggobid *);
extern void init_plot_GC (GdkWindow *, ggobid *);
extern void init_var_GCs (GtkWidget *, ggobid *);
extern gfloat jitter_randval (gint);
extern void jitter_value_set (gfloat, datad *, ggobid *);
extern void jitter_vars_init (datad *, ggobid *);
extern void jitter_window_open (ggobid *);
extern void limits_adjust (gfloat *, gfloat *);
extern void limits_set (gboolean do_raw, gboolean do_tform, datad *);  
extern void line_brush_prev_vectors_update (ggobid *);
extern void line_brush_undo (splotd *, ggobid *);
extern gboolean line_colors_read (gchar *, gboolean, datad *, ggobid *);
extern void lineedit_event_handlers_toggle (splotd *, gboolean);
extern void make_symbol_window (ggobid *);
extern void make_ui (ggobid *);
extern GlyphType mapGlyphName (const gchar *gtype);
extern void missing_alloc (gint, gint);
extern void missing_arrays_add_cols (gint, datad *d, ggobid *);
extern void missing_block_alloc (gint, gint);
extern void missing_jitter_value_set (gfloat, datad *, ggobid *);
extern void missing_lim_set (datad *, ggobid *);
extern void missing_rejitter (datad *, ggobid *);
extern void missing_to_world (datad *, ggobid *);
extern void missing_values_read (gchar *, gboolean, datad *, ggobid *);
extern void missing_world_alloc (datad *, ggobid *);
extern void missing_world_free (datad *d, ggobid *);
extern void mode_activate (splotd *, gint, gboolean, ggobid *);
extern gint mode_get (ggobid *);
extern void mode_set (gint, ggobid *);
extern void mode_set_cb (GtkWidget  *, gint);
extern void mode_submenus_activate (splotd *, gint, gboolean, ggobid *);
extern gboolean mouseinwindow (splotd *);
extern void mousepos_get_motion (GtkWidget *, GdkEventMotion *, gboolean *, gboolean *, splotd *);
extern void mousepos_get_pressed (GtkWidget *, GdkEventButton *, gboolean *, gboolean *, splotd *);
extern void move_pt (gint id, gint x, gint y, splotd *sp, datad *d, ggobid *);
extern void movepts_event_handlers_toggle (splotd *, gboolean);
extern void movepts_history_add (gint id, splotd *sp, datad *, ggobid *);
extern void movepts_history_delete_last (datad *, ggobid *);
extern gdouble myrint (gdouble x);
extern FILE* open_ggobi_file_r (gchar *, gint, gchar **, gboolean);
extern gint option_menu_index (GtkOptionMenu *);
extern void p1d_reproject (splotd *, glong **, datad *, ggobid *);
extern gboolean p1d_varsel (splotd *, gint, gint *, gint);
extern void pan_by_drag (splotd *, ggobid *);
extern void pan_step (splotd *, gint, ggobid *);
extern void parcoords_cpanel_init (cpaneld*, ggobid *);
extern void parcoords_main_menus_make (GtkAccelGroup *, GtkSignalFunc, ggobid *gg, gboolean);
extern displayd* parcoords_new (gboolean, gint, gint *, datad *, ggobid *);
extern void parcoords_reset_arrangement (displayd *, gint, ggobid *);
extern gboolean parcoords_varsel (cpaneld *, splotd *, gint, gint *, ggobid *);
extern gboolean pca_calc (datad *, ggobid *);
extern void pca_diagnostics_set (datad *d, ggobid *);
extern gint pcompare (const void *, const void *);
extern void pipeline_arrays_add_column (gint, datad *, ggobid *);
extern void pipeline_arrays_alloc (datad *, ggobid *);
extern void pipeline_arrays_free (datad *d, ggobid *);
extern void pipeline_init (datad *, ggobid *);
extern gint plotted_cols_get (gint *, datad *, ggobid *);
extern void point_brush_prev_vectors_update (datad *, ggobid *);
extern void point_brush_undo (splotd *, datad *, ggobid *);
extern gboolean point_colors_read (gchar *, gboolean, datad *, ggobid *);
extern gboolean point_glyphs_read (gchar *, gboolean, datad *, ggobid *);
extern gboolean point_in_which_bin (gint, gint, gint *, gint *, datad *, ggobid *);
extern void populate_option_menu (GtkWidget *, gchar **, gint, GtkSignalFunc, ggobid *);
extern void position_popup_menu (GtkMenu *menu, gint *px, gint *py, gpointer);
extern gint projection_get (ggobid *);
extern void quick_message (gchar *, gboolean);
extern void quit_ggobi(ggobid *gg, gint action, GtkWidget *w);
extern gdouble randvalue (void);
extern void raw_to_tform_copy (void);
extern void rejitter (datad *, ggobid *);
extern void rnorm2 (gdouble *, gdouble *);
extern void rotation_event_handlers_toggle (splotd *, gboolean);
extern void rotation_menus_make (ggobid *);
extern void rowlabels_alloc (datad *d, ggobid *) ;
extern void rowlabels_free (datad *d, ggobid *);
extern gboolean rowlabels_read (gchar *, gboolean, datad *, ggobid *);
extern void rows_in_plot_set (datad *d, ggobid *);
extern void ruler_ranges_set (displayd *, splotd *, ggobid *);
extern void scale_event_handlers_toggle (splotd *, gboolean);
extern void scale_menus_make (ggobid *);
extern void scaling_visual_cues_draw (splotd *, ggobid *);
extern void scatmat_cpanel_init (cpaneld *, ggobid *);
extern void scatmat_main_menus_make (GtkAccelGroup *, GtkSignalFunc, ggobid *gg, gboolean);
extern displayd* scatmat_new (gboolean, gint, gint *, gint, gint *, datad *, ggobid *);
extern gboolean scatmat_varsel (cpaneld *, splotd *, gint, gint *, gint, gboolean, ggobid *);
extern gboolean scatmat_varsel_simple (cpaneld *, splotd *, gint, gint *, ggobid *);
extern void scatterplot_cpanel_init (cpaneld *, gint, ggobid *);
extern void scatterplot_main_menus_make (GtkAccelGroup *, GtkSignalFunc, ggobid *gg, gboolean);
extern displayd* scatterplot_new (gboolean, splotd *sp, datad *d, ggobid *);
extern void scatterplot_show_rulers (displayd *, gint);
extern gboolean scree_mapped_p (ggobid *);
extern void scree_plot_make (ggobid *);
extern gint selected_cols_get (gint *, datad *d, ggobid *);
extern void smooth_window_open (ggobid *);
extern void sp_event_handlers_toggle (splotd *, gboolean);
extern void sp_whiskers_make (splotd *, displayd *, ggobid *);
extern void sphere_condnum_set (gfloat x, ggobid *);
extern void sphere_enable (gboolean sens, ggobid *);
extern gint sphere_npcs_get (datad *, ggobid *);
extern void sphere_npcs_set (gint, datad *, ggobid *);
extern void sphere_panel_open (ggobid *);
extern gboolean sphere_svd (datad *, ggobid *);
extern void sphere_totvar_set (gfloat x, datad *, ggobid*);
extern void sphere_transform_set (datad *, ggobid *);
extern void sphere_varcovar_set (datad *, ggobid *);
extern void spherevars_set (datad *, ggobid *);
extern void spherize_data (gint num_pcs, gint nsvars, gint *svars, datad *, ggobid *);
extern void splot_add_point_label (splotd *, gint, gboolean, ggobid *);
extern void splot_dimension_set(splotd* sp, gint width, gint height);
extern void splot_draw_border (splotd *, ggobid *);
extern void splot_expose (splotd *);
extern void splot_free (splotd *, displayd *, ggobid *);
extern void splot_get_dimensions (splotd *, gint *, gint *);
extern splotd* splot_new (displayd *, gint, gint, ggobid *);
extern void splot_pixmap0_to_pixmap1 (splotd *, gboolean, ggobid *);
extern void splot_pixmap1_to_window (splotd *, ggobid *);
extern void splot_plane_to_screen (displayd *, cpaneld *, splotd *, ggobid *);
extern void splot_plane_to_world (splotd *, gint, ggobid *);
extern gboolean splot_plot_case (gint m, datad *, splotd *, displayd *, ggobid *);
extern void splot_point_colors_used_get (splotd *, gint *ncolors_used, gushort *colors_used, gboolean binned, ggobid *); 
extern void splot_redraw (splotd *sp, gint, ggobid *);
extern void splot_reverse_pipeline (splotd *, gint, lcoords *, gboolean, gboolean, ggobid *);
extern void splot_screen_to_tform (cpaneld *, splotd *, icoords *, fcoords *, ggobid *);
extern void splot_set_current (splotd *, gboolean, ggobid *);
extern void splot_set_plot_center (splotd *);
extern void splot_world_to_plane (cpaneld *, splotd *, ggobid *);
extern void splot_zoom (splotd *sp, gfloat xsc, gfloat ysc, ggobid *) ;
extern gint sqdist (gint, gint, gint, gint);
extern void strip_suffixes (ggobid *);
extern void submenu_append (GtkWidget *, GtkWidget *);
extern void submenu_destroy (GtkWidget *);
extern void submenu_insert (GtkWidget *, GtkWidget *, gint);
extern GtkWidget* submenu_make (gchar *, gint, GtkAccelGroup *);
extern void subset_apply (gboolean, datad *, ggobid *);
extern gboolean subset_block (gint, gint, datad *, ggobid *);
extern gboolean subset_everyn (gint, gint, datad *, ggobid *);
extern void subset_include_all (datad *, ggobid *);
extern gboolean subset_random (gint, datad *, ggobid *);
extern gboolean subset_rowlab (gchar *, datad *, ggobid *);
extern gboolean subset_sticky (datad *, ggobid *);
extern void subset_window_open (ggobid *, guint, GtkWidget *);
extern void textur (gfloat *, gfloat *, gint, gint, gfloat, gint, ggobid *);
extern void tform1_to_tform2_copy (void);
extern void tform_label_update (gint, datad *, ggobid *);
extern void tform_to_world (datad *, ggobid *);
extern void tform_to_world_by_var (gint j, datad *, ggobid *);
extern void tour1d_event_handlers_toggle (splotd *, gboolean);
extern void tour1d_func (gboolean, ggobid *);
extern void tour1d_menus_make (ggobid *);
extern void tour1d_projdata (splotd *, glong **, datad *, ggobid *);
extern void tour1dpp_window_open (ggobid *);
extern void tour2d_event_handlers_toggle (splotd *, gboolean);
extern void tour2d_func (gboolean, ggobid *);
extern void tour2d_func (gboolean, ggobid *);
extern void tour2d_menus_make (ggobid *);
extern void tour2d_projdata (splotd *, glong **, datad *, ggobid *);
extern void tour2d_projdata (splotd *, glong **, datad *, ggobid *);
extern void tour2dpp_window_open (ggobid *);
extern void tour_do_step (displayd *, ggobid *);
extern void transform (gint, gint, gfloat, datad *, ggobid *);
extern void transform0_opt_menu_set_value (gint j, datad *d, ggobid *gg);
extern void transform0_values_set (gint, gint, datad *, ggobid *);
extern gboolean transform1_apply (gint, datad *, ggobid *);
extern void transform1_opt_menu_set_value (gint j, datad *d, ggobid *gg);
extern void transform1_values_set (gint, gfloat, gint, datad *, ggobid *);
extern gboolean transform2_apply (gint, datad *, ggobid *);
extern void transform2_opt_menu_set_value (gint j, datad *d, ggobid *gg);
extern void transform2_values_set (gint, gint, datad *, ggobid *);
extern void transform_values_init (gint j, datad *, ggobid *);
extern void transform_variable (gint, gint, gfloat, gint, datad *, ggobid *);
extern void transform_window_open (ggobid *);
extern void varcircles_add (gint ncols, datad *, ggobid *);
extern void varcircles_layout_reset (gint ncols, datad *, ggobid *);
extern void varcircles_populate (datad *, ggobid *);
extern void varcircles_refresh (datad *, ggobid *);
extern void vardialog_open (ggobid *, gchar *title);
extern void variable_clone (gint, const gchar *, gboolean, datad *, ggobid *);
extern void varlabel_set (gint, datad *, ggobid *);
extern void varpanel_checkbox_add (gint j, datad *d, ggobid *gg);
extern void varpanel_make (GtkWidget *, ggobid *);
extern void varpanel_populate (datad *, ggobid *);
extern void varpanel_refresh (ggobid *);
extern void varpanel_tooltips_set (ggobid *);
extern void varsel (cpaneld *, splotd *, gint jvar, gint btn, gint alt_mod, gint ctrl_mod, gint shift_mod, datad *, ggobid *);
extern void vartable_alloc (datad *);
extern void vartable_collab_tform_set_by_var (gint j, datad *d);
extern void vartable_init (datad *d);
extern void vartable_limits_set (datad *);
extern void vartable_limits_set_by_var (gint j, datad *d);
extern void vartable_open (ggobid *);
extern void vartable_realloc (gint, datad *);
extern void vartable_row_append (gint, datad *, ggobid *);
extern void vartable_select_var (gint, gboolean, datad *, ggobid *);
extern void vartable_stats_set (datad *);
extern void vartable_stats_set_by_var (gint j, datad *);
extern void vectorb_copy (vector_b *, vector_b *);
extern void vectorb_free (vector_b *);
extern void vectorb_init (vector_b *);
extern void vectorb_realloc (vector_b *, gint);
extern void vectorb_realloc_zero (vector_b *, gint);
extern void vectori_init (vector_i *);
extern void vectori_realloc (vector_i *, gint);
extern void vectori_realloc_zero (vector_i *, gint);
extern void vectors_copy (vector_s *, vector_s *);
extern void vectors_free (vector_s *);
extern void vectors_init (vector_s *);
extern void vectors_realloc (vector_s *, gint);
extern void vectors_realloc_zero (vector_s *, gint);
extern void world_to_raw (gint, splotd *, datad *, ggobid *);
extern void writeall_window_open (ggobid *);
extern void xy_reproject (splotd *, glong **, datad *, ggobid *);
extern gboolean xyplot_varsel (splotd *, gint, gint *, gint);
extern void zoom_by_drag (splotd *, ggobid *);
extern void zoom_step (splotd *, gint, gint, rectd *, ggobid*);


ggobid * ggobi_get (gint);
displayd * display_alloc_init (enum displaytyped, gboolean, datad *, ggobid *);

/* The new way of handling window closures, so that we don't just exit. */
void ggobi_close (GtkObject *w, ggobid *);
int ggobi_remove_by_index (ggobid *gg, gint which);
int ggobi_remove (ggobid *);

#ifdef __cplusplus
}
#endif

#include "GGobiAPI.h"

#define CHECK_GG(a) ValidateGGobiRef(a, true)

#endif

