#ifndef DATAD_H
#define DATAD_H

#include "defines.h"

typedef struct /*-- datad --*/ {
 gint nrows;
 GArray *rowlab;

 gint ncols;
 vardatad *vardata;
 GtkWidget *vardata_clist;
 gboolean single_column;

 array_f raw, tform1, tform2;
 array_l world, jitdata;

 /*----------------------- missing values ---------------------------*/

 gint nmissing;
 array_s missing;  /*-- array of shorts --*/
 array_l missing_jitter, missing_world;
 gfloat missing_jitter_factor;
 lims missing_lim;  /*-- usually 0,1, but potentially wider --*/

 /*---------------- deleting the hidden points; subsetting ----------*/

 gint *rows_in_plot;
 gint nrows_in_plot;
 gboolean *sampled;

 struct _Subset {
   gint random_n;
   /*-- adjustments from which to get values for blocksize, everyn --*/
   GtkAdjustment *bstart_adj, *bsize_adj;
   GtkAdjustment *bstart_incr_adj, *bsize_incr_adj;
   GtkAdjustment *estart_adj, *estep_adj;
 } subset;

 /*--------------- clusters: hiding, excluding ----------------------*/

 GtkWidget *exclusion_table;
 gboolean *included;
 
 gint nclusters;
 clusterd *clusv;
 vector_i clusterids;

 /*----------------------- row grouping -----------------------------*/

 gint nrgroups, nrgroups_in_plot;
 gint *rgroup_ids;
 rgroupd *rgroups;

 /*------------------------ jittering --------------------------------*/

 struct _Jitterd {
   gfloat factor;
   gboolean type;
   gboolean vgroup;
   gboolean convex;
   gfloat *jitfacv;
 } jitter;

/*------------------------ brushing ----------------------------------*/

 
 gint npts_under_brush;
 gboolean *pts_under_brush;
 gshort *color_ids, *color_now, *color_prev;  /* 0:ncolors-1 */
 glyphv *glyph_ids, *glyph_now, *glyph_prev;
 gboolean *hidden, *hidden_now, *hidden_prev;
 struct _BrushBins {
   gint nbins;
   bin_struct **binarray;
   icoords bin0, bin1;
 } brush;

/*---------------------- identification ------------------------------*/

 gint nearest_point, nearest_point_prev;
 GSList *sticky_ids;

/*-------------------- moving points ---------------------------------*/

 GSList *movepts_history;  /*-- a list of elements of type celld --*/

/*----------------- variable selection panel -------------------------*/

 struct _Varpaneld {
   GtkWidget *table;
   gint vnrows, vncols, nvars;
   GtkWidget **da, **label;
 } varpanel_ui;

/*-------------------- transformation --------------------------------*/

 gint std_type;  /* Can be 0, 1 or 2 */

 /* sphering transformation */
 struct _Sphere_d {
   gint nvars, *vars;  /*-- vars available for sphering --*/
   gint npcs;          /*-- the first npcs vars of vars will be sphered --*/

   gfloat *eigenval;
   gfloat **eigenvec;
   gfloat **vc;
   gfloat *tform1_mean;
 } sphere;

} datad;

#endif
