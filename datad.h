/*-- datad.h --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#ifndef DATAD_H
#define DATAD_H

#include "defines.h"
#include "brushing.h"
#include "vartable.h"

#ifdef USE_XML
#include <libxml/parser.h>
struct _XMLUserData;
#endif

struct _ggobid;

#include "fileio.h"

typedef struct _datad datad;

#define GTK_TYPE_GGOBI_DATA	 (gtk_ggobi_data_get_type ())
#define GTK_GGOBI_DATA(obj)	 (GTK_CHECK_CAST ((obj), GTK_TYPE_GGOBI_DATA, datad))
#define GTK_GGOBI_DATA_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_GGOBI_DATA, GtkGGobiDataClass))
#define GTK_IS_GGOBI_DATA(obj)	 (GTK_CHECK_TYPE ((obj), GTK_TYPE_GGOBI_DATA))
#define GTK_IS_GGOBI_DATA_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_GGOBI_DATA))

GtkType gtk_ggobi_data_get_type(void);

datad *gtk_ggobi_data_new(ggobid *gg);
datad *gtk_ggobi_data_new_with_dimensions(int nr, int nc, ggobid *gg);

typedef struct _GtkGGobiDataClass
{
    GtkObjectClass parent_class;

} GtkGGobiDataClass;

struct _datad {

  GtkObject object;

  /* All the variables are left public since this the way they were in the
     C structure. Adding accessor routines and using those would be "good",
     but tedious.
   */

  /* Holds the name given to the dataset in an XML file and by which it
     can be indexed in the list of data elements within the ggobid structure.
   */
 const gchar *name;
 InputDescription *input;

 struct _ggobid *gg;  /*-- a pointer to the parent --*/

 guint nrows;
 GArray *rowlab;  /*-- allocates memory in chunks --*/

 /*-- row ids to support generalized linking --*/
 struct _RowID {
   vector_i id;
   vector_i idv;
 } rowid;
 /*-- --*/

 /*-- to support brushing by categorical variable --*/
 vartabled *linkvar_vt;   /*-- the linking variable --*/
 /*-- --*/

 gint ncols;
 GSList *vartable;
 GtkWidget *vartable_clist;
 gboolean single_column;  /*-- not handling single-column case now --*/

 array_f raw, tform;
 array_l world, jitdata;

 /*----------------------- missing values ---------------------------*/

 gint nmissing;
 array_s missing;  /*-- array of shorts --*/
 gboolean missings_show_p;  /*-- show/hide per datad, not per display --*/

 /*---------------- deleting the hidden points; subsetting ----------*/

 gint *rows_in_plot;
 gint nrows_in_plot;
 vector_b sampled;

 struct _Subset {
   gint random_n;
   /*-- adjustments from which to get values for blocksize, everyn --*/
   GtkAdjustment *bstart_adj, *bsize_adj;
   GtkAdjustment *estart_adj, *estep_adj;
 } subset;

 /*--------------- clusters: hiding, excluding ----------------------*/

 symbol_cell symbol_table[NGLYPHTYPES][NGLYPHSIZES][MAXNCOLORS];

 GtkWidget *cluster_table;  /*-- table of symbol groups from brushing --*/
 
 gint nclusters;
 clusterd *clusv;
 clusteruid *clusvui;
 vector_i clusterid;

 /*------------------------ jittering --------------------------------*/

 struct _Jitterd {
   gfloat factor;
   gboolean type;
   gboolean convex;
   gfloat *jitfacv;
 } jitter;

/*------------------------ brushing ----------------------------------*/

 /*-- it's odd to have these in datad; let me think about that --*/
 gint npts_under_brush;
 vector_b pts_under_brush;
 struct _BrushBins {
   gint nbins;
   bin_struct **binarray;
   icoords bin0, bin1;
 } brush;
 /*-- --*/

 vector_s color, color_now, color_prev;
 vector_b hidden, hidden_now, hidden_prev;
 vector_g glyph, glyph_now, glyph_prev;


/*---------------------- identification ------------------------------*/

 /*-- used in identification, line editing, and point motion --*/
 gint nearest_point, nearest_point_prev;
 GSList *sticky_ids;

/*-------------------- moving points ---------------------------------*/

 GSList *movepts_history;  /*-- a list of elements of type celld --*/

/*----------------- variable selection panel -------------------------*/

 struct _Varpanel_cboxd {
   GtkWidget *swin;
   GtkWidget *vbox;        /*-- child of swin --*/
   /*GSList *checkbox;*/   /*-- single column of checkboxes --*/
   /*-- switching from checkboxes to two toggle widgets and a label --*/
   GSList *box;   /*-- single column of hboxes --*/

 } vcbox_ui;
 struct _Varpanel_circd {
   GtkWidget *vbox;
   GtkWidget *swin, *hbox;  /*-- children of vbox --*/
   GtkWidget *table;        /*-- sole child of swin --*/
   GtkWidget *manip_btn, *freeze_btn;  /*-- children of hbox --*/

   GdkCursor *cursor;
   gint jcursor;

   /*-- components and properties of the table --*/
   gint tnrows, tncols;     /*-- table dimensions --*/
   GSList *vb, *da, *label;
   GSList *da_pix;          /*-- backing pixmaps --*/
   gint nvars;
 } vcirc_ui;

 struct _Varpaneld {
   GtkWidget *ebox;  /*-- child of the notebook --*/
 } varpanel_ui;

/*-------------------- transformation --------------------------------*/

 /* sphering transformation */
 struct _Sphere_d {
   vector_i vars;         /*-- vars available to be sphered --*/
   vector_i vars_sphered; /*-- vars that have been sphered --*/
   gint npcs;       /*-- the first npcs vars of vars will be sphered --*/
   vector_i pcvars; /*-- vars into which sphered data is written --*/

   vector_f eigenval;
   array_d eigenvec;
   array_f vc;
   vector_f tform_mean;
   vector_f tform_stddev;

   gboolean vars_stdized;
 } sphere;

/*----------------- segments in scatterplots -----------------------------*/

 /*-- edges --*/
 struct _EdgeData {
   gint n;
   endpointsd *endpoints;

   gint nxed_by_brush;
   vector_b xed_by_brush;
 } edge;

/*------------------------------------------------------------------------*/

      /* Instead of a method, use a function pointer which can be set
         for the different types.
       */
#ifdef USE_XML
   gboolean (*readXMLRecord)(const xmlChar **attrs, struct _XMLUserData *data);
#endif 
};


gint alloc_edgeIDs(datad *d);


extern datad *datad_new (datad *, struct _ggobid *);
void datad_instance_init(datad *d);
#endif
