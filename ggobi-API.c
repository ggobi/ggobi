#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "GGobiAPI.h"

#include "ggobi.h"
#include "types.h"
#include "vars.h"
#include "externs.h"
#include "display.h"

void warning(const char *msg);

void GGOBI(displays_release)(ggobid *gg);
void GGOBI(display_release)(displayd *display, ggobid *gg);
void GGOBI(splot_release)(splotd *sp, displayd *display, ggobid *gg);
void GGOBI(data_release)(ggobid *gg);
void GGOBI(vardata_free)(ggobid *gg);
void GGOBI(vardatum_free)(vardatad *var, ggobid *gg);

const gchar *
GGOBI(getFileName) ()
{
  return(gg.filename);
}


DataMode
GGOBI(getDataMode) ()
{
  return(gg.data_mode);
}

const gchar * const 
GGOBI(getDataModeDescription)(DataMode mode)
{
 extern const gchar * const ModeNames[];
 return(ModeNames[mode]);
}

gchar **
GGOBI(getVariableNames)(int transformed)
{
  gchar **names;
  int nc = gg.ncols, i;
  vardatad *form;

  names = (gchar**) g_malloc(sizeof(gchar*)*nc);

    for(i = 0; i < nc; i++) {
      form = gg.vardata + i;
      names[i] = transformed ? form->collab_tform : form->collab;
    }

  return(names);
}


void
GGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed)
{
 gchar *old;

 if(transformed)
   old = gg.vardata[jvar].collab_tform;
 else
   old = gg.vardata[jvar].collab;

 if(old)
   g_free(old);

 if(transformed)
   gg.vardata[jvar].collab_tform = g_strdup(name);
 else
   gg.vardata[jvar].collab = g_strdup(name);
}


/*
  Closes the specified display
 */
void 
GGOBI(destroyCurrentDisplay)()
{
  display_free (gg.current_display, false);
}

/*

  An initial attempt to allow new data to be introduced
  to the Ggobi session, replacing the existing contents.
 
  There are still a few details remaining regarding the scaling
  on the axes, etc. (See ruler_ranges_set in scatterplot.c)
  The reverse pipeline data has not been established correctly
  and the computation is incorrect.
  Specifically, the routine splot_screen_to_tform() is not
  

  When this works, we will take the calls for the different stages 
  and put them in separate routines.
 */
void
GGOBI(setData)(double *values, gchar **rownames, gchar **colnames, int nr, int nc)
{
extern void rowlabels_alloc(void);
 int i, j;

  GGOBI(displays_release)(&gg);
  GGOBI(data_release)(&gg);

  gg.ncols = nc;
  gg.nrows = nr;

  gg.nrows_in_plot = gg.nrows;  /*-- for now --*/
  gg.nlinkable = gg.nrows;      /*-- for now --*/
  gg.nrgroups = 0;              /*-- for now --*/

  gg.rows_in_plot = NULL;

  arrayf_alloc(&gg.raw, nr, nc);

  rowlabels_alloc();
  /*  gg.rowlab = (gchar **) g_malloc(nr * sizeof(gchar*)); */

  vardata_alloc();
  vardata_init();

  for(j = 0; j < nc ; j++) {
   gg.vardata[j].collab = g_strdup(colnames[j]);
   gg.vardata[j].collab_tform = g_strdup(colnames[j]);
   for(i = 0; i < nr ; i++) {
     if(j == 0) {
       gg.rowlab[i] = g_strdup(rownames[i]);
     }

     gg.raw.data[i][j] = values[i + j*nr];
   }
  }

   /* Now recompute and display the top plot. */
  dataset_init(&gg);

  /* Have to patch up the displays list since we removed
     every entry and that makes for meaningless entries.
   */
  gg.displays->next = NULL;
}


/* These are all for freeing the currently held data. */

void
GGOBI(displays_release)(ggobid *gg)
{
 GList *dlist;
 displayd *display;

 /* We have to be careful here as we are removing all the elements
    of the singly-linked list. When we remove the last one,
    the ->next value of the dlist becomes non-NULL. Hence
    we are getting garbage. Accordingly, we count down from the total
    number to remove using num and when this is 0, we exit.
    This should leave the slist allocated, but empty.

    We have to patch the list up afterwards.
  */
 int num = g_list_length(gg->displays);

 for(dlist = gg->displays; dlist != NULL; dlist = dlist->next, num--) {
  if(num == 0)
   break;
  display = (displayd *) dlist->data;
  /*  display_release(display, gg); */
  display_free(display, true);
 }
}

void
GGOBI(display_release)(displayd *display, ggobid *gg)
{
   display_free(display, true);
}


void
GGOBI(splot_release)(splotd *sp, displayd *display, ggobid *gg)
{
 splot_free(sp, display);
}

/* Not in the API for the moment. A "protected" routine. */
void
GGOBI(data_release)(ggobid *gg)
{
 extern void rowlabels_free(void);
 if(gg->rowlab) {
    rowlabels_free();
    gg->rowlab = NULL;
 }

  GGOBI(vardata_free)(gg);
}

void
GGOBI(vardata_free)(ggobid *gg)
{
 int i;

  for(i = 0; i < gg->ncols ; i++) {
    GGOBI(vardatum_free)(gg->vardata+i, gg);
  }
  g_free(gg->vardata);
  gg->vardata = NULL;
}

void 
GGOBI(vardatum_free)(vardatad *var, ggobid *gg)
{
 if(var->collab)
  g_free(var->collab);
 if(var->collab_tform)
  g_free(var->collab_tform);
}


const gchar * const*
GGOBI(getViewTypes)(int *n)
{
 *n = NDISPLAYTYPES;
 return(ViewTypes);
}

const gint *
GGOBI(getViewTypeIndeces)(int *n)
{
 *n = NDISPLAYTYPES;
 return(ViewTypeIndeces);
}


displayd *
GGOBI(newScatterplot) (gint ix, gint iy)
{
 displayd *display = NULL;

 display = display_create(0, &gg);

return(display);
}

displayd *
GGOBI(newScatmat) (gint *rows, gint *columns)
{
 displayd *display = NULL;

 display = display_create(1, &gg);

return(display);
}

displayd *
GGOBI(newParCoords)(gint *vars)
{
 displayd *display = NULL;

 display = display_create(2, &gg);

return(display);
}

displayd* 
GGOBI(createPlot)(int type, char **varnames)
{
 displayd *display = NULL;
 /*

   display_new(type);

 */
 return(display);
}


const gchar* 
GGOBI(getCurrentDisplayType)(ggobid *gg)
{
 return(GGOBI(getViewTypeName)(gg->current_display->displaytype));
}

const gchar *
GGOBI(getViewTypeName)(enum displaytyped type)
{
 int n, i;
 const gint *types = GGOBI(getViewTypeIndeces)(&n);
 const gchar * const *names = GGOBI(getViewTypes)(&n);

 for(i = 0; i < n; i++) {
   if(types[i] == type) {
    return(names[i]);
   }
 }

 return(NULL);
}


/*
  Pointer to the raw data managed by GGobi.
  Don't touch this.
 */
const gfloat** 
GGOBI(getRawData)()
{
 return((const gfloat**) gg.raw.data);
}

/*
  Pointer to the second transformation of the data managed by GGobi.
  Don't touch this.
 */
const gfloat** 
GGOBI(getTFormData)()
{
 return((const gfloat **)gg.tform2.data);
}


/*
  Returns a reference to the labels used to identify
  the observations.
  Do not change this as it is not a copy.
 */
const gchar **
GGOBI(getCaseNames)()
{
 return((const gchar **) gg.rowlab);
}

/*
 This does not copy the label, so it is assumed
 that the caller has already allocated the space
 using the appropriate GTK memory model.
 If it is apparent that this is being called from 
 contexts that use a very different memory model (e.g. S/R),
 we can change the behaviour to copy this.

 Similarly, this can be modified to return the previous
 value, but the caller will have to free that pointer.
 */
void
GGOBI(setCaseName)(gint index, const gchar *label)
{
  gchar *old;
  if(index < 0 || index >= gg.nrows) {
    warning("Index is out of range of observations in setCaseName");
    return; 
  }

  old = gg.rowlab[index];

  g_free(old);
  gg.rowlab[index] = (gchar *)label;
}


void
warning(const char *msg)
{
 fprintf(stderr, "%s\n", msg);
 fflush(stderr);
}



gint *
GGOBI(getGlyphTypes)(int *n)
{


 return(NULL);
}


gchar const*
GGOBI(getGlyphTypeName)(int type)
{
 gchar const *ans;
  ans = "Foo";

 return(ans);
}


gint *
GGOBI(getCaseGlyphTypes)(gint *ids, gint n)
{
 int i;
 gint *ans = (gint *) g_malloc(n * sizeof(int));

 for(i = 0; i < n ; i++)
   ans[i] = GGOBI(getCaseGlyphType)(ids[i]);

 return(ids);
}

gint 
GGOBI(getCaseGlyphType)(gint id)
{
 int index = gg.rows_in_plot[id];
  return(gg.glyph_ids[index].type);
}

gint *
GGOBI(getCaseGlyphSizes)(gint *ids, gint n)
{
 int i;
 gint *ans = (gint *) g_malloc(n * sizeof(int));

 for(i = 0; i < n ; i++)
   ans[i] = GGOBI(getCaseGlyphSize)(ids[i]);

 return(ids);
}

gint 
GGOBI(getCaseGlyphSize)(gint id)
{
 int index = gg.rows_in_plot[id];

  return(gg.glyph_ids[index].size);
}


void 
GGOBI(setCaseGlyph)(gint index, gint type, gint size)
{
  gg.glyph_ids[index].size = size;
  gg.glyph_ids[index].type = type;
}

void 
GGOBI(setCaseGlyphs)(gint *ids, gint n, gint type, gint size)
{
 int i;
 for(i = 0; i < n ; i++)
   GGOBI(setCaseGlyph)(ids[i], type, size);
}


void 
GGOBI(setCaseColor)(gint pt, gint colorIndex)
{
 gg.color_ids[pt] = gg.color_now[pt] = colorIndex;
}

void 
GGOBI(setCaseColors)(gint *pts, gint howMany, gint colorindx)
{
 int i;
 for(i = 0; i < howMany ; i++)
   GGOBI(setCaseColor)(pts[i], colorindx);
}


gint 
GGOBI(getCaseColor) (gint pt)
{
  return(gg.color_ids[pt]);
}

gint *
GGOBI(getCaseColors)(gint *pts, gint howMany)
{
 int i;
 gint *ans = (gint*) g_malloc(howMany * sizeof(int));

 for(i = 0; i < howMany ; i++)
  ans[i] = GGOBI(getCaseColor)(pts[i]);

 return(ans);
}



void
GGOBI(setObservationSegment)(gint x, gint y)
{
  if(GGOBI(isConnectedSegment)(x, y) == false) {
    segments_alloc(gg.nsegments+1);
    gg.segment_endpoints[gg.nsegments].a = x;
    gg.segment_endpoints[gg.nsegments].b = y;
    gg.nsegments++;
  }
}


gboolean 
GGOBI(isConnectedSegment)(gint a, gint b)
{
  gint tmp, i;

  if(a > b) {
     tmp = a;
     a = b;
     b = tmp;
  }

  for(i = 0; i < gg.nsegments ; i++) {
    
    if(gg.segment_endpoints[i].a == a && gg.segment_endpoints[i].b == b)
       return(true);

    if(gg.segment_endpoints[i].a > a) {
      return(false);
    } 
  }

 return(false);
}


gboolean 
GGOBI(getShowLines)()
{
 return(GGOBI(getDefaultDisplayOptions)()->segments_directed_show_p);
}


gboolean GGOBI(setShowLines)(gboolean val)
{
 gboolean old = GGOBI(getShowLines)();
 GGOBI(getDefaultDisplayOptions)()->segments_directed_show_p = val;

 return(old);
}

DisplayOptions *
GGOBI(getDefaultDisplayOptions)()
{
 return(&DefaultDisplayOptions);
}


displayd *
GGOBI(getDisplay)(int which)
{
  displayd *display = NULL;

  if(which < g_list_length(gg.displays))
   display = (displayd *) g_list_nth_data(gg.displays, which);

 return(display);
}

DisplayOptions *
GGOBI(getDisplayOptions)(int displayNum)
{
 DisplayOptions *options = NULL;
  if(displayNum < 0)
   options =  GGOBI(getDefaultDisplayOptions)();
  else {
   displayd *display;
    display = GGOBI(getDisplay)(displayNum);
    if(display)
      options = &(display->options);
  }

 return(options);
}


displayd *
GGOBI(getCurrentDisplay)()
{
 return(gg.current_display);
}

gint
GGOBI(getCurrentDisplayIndex)()
{
 return(g_list_index(gg.displays, gg.current_display));
}

displayd *
GGOBI(setCurrentDisplay)(int which)
{
 displayd *d;

 d = GGOBI(getDisplay)(which);

 if(d != NULL)
   display_set_current(d);

 return(d);
}


splotd *
GGOBI(getPlot)(displayd *display, int which)
{
  splotd *sp = (splotd *) g_list_nth_data(display->splots, which);
  return(sp);
}
