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

#ifdef USE_XML
#include <parser.h>
struct _XMLUserData;
#endif

struct _ggobid;

#include "fileio.h"

/*
  This is now changed so that datad is a class rather than a structure.
  The intention is that this allows us to create different derived classes
  which can read their data in different ways. For example, we may read node
  records in one way (i.e. what attributes they expect) and edge records
  in another. 

  See the methods at the bottom of the class definition.
 */
#ifndef USE_CLASSES
struct _datad {
#else
class datad {
#endif

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

 gint nrows;
 GArray *rowlab;

 gint ncols;
 vartabled *vartable;
 GtkWidget *vartable_clist;
 gboolean single_column;

 array_f raw, tform;
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
 vector_b sampled;

 struct _Subset {
   gint random_n;
   /*-- adjustments from which to get values for blocksize, everyn --*/
   GtkAdjustment *bstart_adj, *bsize_adj;
   GtkAdjustment *bstart_incr_adj, *bsize_incr_adj;
   GtkAdjustment *estart_adj, *estep_adj;
 } subset;

 /*--------------- clusters: hiding, excluding ----------------------*/

 GtkWidget *exclusion_table;
/* gboolean *included;*/
 vector_b included;
 
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
   gboolean convex;
   gfloat *jitfacv;
 } jitter;

/*------------------------ brushing ----------------------------------*/

 brush_coords brush_pos;  
 gint npts_under_brush;
 vector_b pts_under_brush;
 vector_s color_ids, color_now, color_prev;  /* 0:ncolors-1 */
 vector_b hidden, hidden_now, hidden_prev;
 glyphv *glyph_ids, *glyph_now, *glyph_prev;

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
   GtkWidget *swin;       /*-- child of the notebook --*/
   GtkWidget *ebox;       /*-- child of the scrolled_window --*/

   /*-- widgets for the simplest modes: checkboxes --*/
   GtkWidget *vbox;      /*-- child of the ebox --*/
   GSList *checkbox;     /*-- single column of checkboxes --*/

   /*-- widgets for the richest modes: variable circles --*/
   GtkWidget *table;    /*-- replaces the vbox for circles --*/
   gint tnrows, tncols; /*-- table dimensions --*/
   GSList *vb, *da, *label;
   GSList *da_pix;  /*-- backing pixmaps --*/

   /*-- don't know if I need this, but it may be useful --*/
   gint nvars;

 } varpanel_ui;

/*-------------------- transformation --------------------------------*/

 /* sphering transformation */
 struct _Sphere_d {
   vector_i vars;         /*-- vars available to be sphered --*/
   vector_i vars_sphered; /*-- vars that have been sphered --*/
   gint npcs;       /*-- the first npcs vars of vars will be sphered --*/
   vector_i pcvars; /*-- vars into which sphered data is written --*/

   vector_f eigenval;
   array_f eigenvec;
   array_f vc;
   vector_f tform_mean;
 } sphere;


#ifdef USE_CLASSES
 public:
   datad () {};
   datad (struct _ggobid *gg);


   /* The following are methods that one might want to override in order to 
      modify how records are handled.
      As more are needed, migrate the bodies of the C routines in
      read_xml.c, etc.  to here.
    */
#ifdef USE_XML
  virtual gboolean readXMLRecord(const CHAR **attrs, struct _XMLUserData *data);
#endif

#else

      /* Instead of a method, use a function pointer which can be set
         for the different types.
       */
#ifdef USE_XML
   gboolean (*readXMLRecord)(const CHAR **attrs, struct _XMLUserData *data);
#endif 

   gboolean edgeData;
   guint *sourceID;
   guint *destinationID;   
   struct _datad *nodeData; 

#endif /* end of USE_CLASSES */
};

#ifndef USE_CLASSES
typedef struct _datad datad;
gint alloc_edgeIDs(datad *d);
#endif





extern datad *datad_new (datad *, struct _ggobid *);

#ifdef USE_CLASSES
class EdgeDatad : public datad 
{
 public:
  EdgeDatad() : datad() { sourceID = NULL; 
                          destinationID = NULL;
                        }

  EdgeDatad(struct _ggobid *gg) : datad(gg) { 
  }


#if 1
#ifdef USE_XML
  gboolean readXMLRecord(const CHAR **attrs, XMLParserData *data);
#endif
#endif

 protected:
  int *sourceID;
  int *destinationID;
  datad *nodeData;
};
#endif



#endif
