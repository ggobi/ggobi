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

void XGOBI(displays_release)(xgobid *xg);
void XGOBI(display_release)(displayd *display, xgobid *xg);
void XGOBI(splot_release)(splotd *sp, displayd *display, xgobid *xg);
void XGOBI(data_release)(xgobid *xg);
void XGOBI(vardata_free)(xgobid *xg);
void XGOBI(vardatum_free)(vardatad *var, xgobid *xg);

const gchar *
XGOBI(getFileName) ()
{
  return(xg.filename);
}


DataMode
XGOBI(getDataMode) ()
{
  return(xg.data_mode);
}

const gchar * const 
XGOBI(getDataModeDescription)(DataMode mode)
{
 extern const gchar * const ModeNames[];
 return(ModeNames[mode]);
}

gchar **
XGOBI(getVariableNames)(int transformed)
{
  gchar **names;
  int nc = xg.ncols, i;
  vardatad *form;

  names = (gchar**) g_malloc(sizeof(gchar*)*nc);

    for(i = 0; i < nc; i++) {
      form = xg.vardata + i;
      names[i] = transformed ? form->collab_tform : form->collab;
    }

  return(names);
}


void
XGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed)
{
 gchar *old;

 if(transformed)
   old = xg.vardata[jvar].collab_tform;
 else
   old = xg.vardata[jvar].collab;

 if(old)
   g_free(old);

 if(transformed)
   xg.vardata[jvar].collab_tform = g_strdup(name);
 else
   xg.vardata[jvar].collab = g_strdup(name);
}


/*
  Closes the specified display
 */
void 
XGOBI(destroyCurrentDisplay)()
{
  display_free (xg.current_display, false);
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
XGOBI(setData)(double *values, gchar **rownames, gchar **colnames, int nr, int nc)
{
extern void rowlabels_alloc(void);
 int i, j;

  XGOBI(displays_release)(&xg);
  XGOBI(data_release)(&xg);

  xg.ncols = nc;
  xg.nrows = nr;

  xg.nrows_in_plot = xg.nrows;  /*-- for now --*/
  xg.nlinkable = xg.nrows;      /*-- for now --*/
  xg.nrgroups = 0;              /*-- for now --*/

  xg.rows_in_plot = NULL;

  arrayf_alloc(&xg.raw, nr, nc);

  rowlabels_alloc();
  /*  xg.rowlab = (gchar **) g_malloc(nr * sizeof(gchar*)); */

  vardata_alloc();
  vardata_init();

  for(j = 0; j < nc ; j++) {
   xg.vardata[j].collab = g_strdup(colnames[j]);
   xg.vardata[j].collab_tform = g_strdup(colnames[j]);
   for(i = 0; i < nr ; i++) {
     if(j == 0) {
       xg.rowlab[i] = g_strdup(rownames[i]);
     }

     xg.raw.data[i][j] = values[i + j*nr];
   }
  }

   /* Now recompute and display the top plot. */
  dataset_init(&xg);

  /* Have to patch up the displays list since we removed
     every entry and that makes for meaningless entries.
   */
  xg.displays->next = NULL;
}


/* These are all for freeing the currently held data. */

void
XGOBI(displays_release)(xgobid *xg)
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
 int num = g_list_length(xg->displays);

 for(dlist = xg->displays; dlist != NULL; dlist = dlist->next, num--) {
  if(num == 0)
   break;
  display = (displayd *) dlist->data;
  /*  display_release(display, xg); */
  display_free(display, true);
 }
}

void
XGOBI(display_release)(displayd *display, xgobid *xg)
{
   display_free(display, true);
}


void
XGOBI(splot_release)(splotd *sp, displayd *display, xgobid *xg)
{
 splot_free(sp, display);
}

/* Not in the API for the moment. A "protected" routine. */
void
XGOBI(data_release)(xgobid *xg)
{
 extern void rowlabels_free(void);
 if(xg->rowlab) {
    rowlabels_free();
    xg->rowlab = NULL;
 }

  XGOBI(vardata_free)(xg);
}

void
XGOBI(vardata_free)(xgobid *xg)
{
 int i;

  for(i = 0; i < xg->ncols ; i++) {
    XGOBI(vardatum_free)(xg->vardata+i, xg);
  }
  g_free(xg->vardata);
  xg->vardata = NULL;
}

void 
XGOBI(vardatum_free)(vardatad *var, xgobid *xg)
{
 if(var->collab)
  g_free(var->collab);
 if(var->collab_tform)
  g_free(var->collab_tform);
}


const gchar * const*
XGOBI(getViewTypes)(int *n)
{
 *n = NDISPLAYTYPES;
 return(ViewTypes);
}

const gint *
XGOBI(getViewTypeIndeces)(int *n)
{
 *n = NDISPLAYTYPES;
 return(ViewTypeIndeces);
}


displayd *
XGOBI(newScatterplot) (gint ix, gint iy)
{
 displayd *display = NULL;

 display = display_create(0, &xg);

return(display);
}

displayd *
XGOBI(newScatmat) (gint *rows, gint *columns)
{
 displayd *display = NULL;

 display = display_create(1, &xg);

return(display);
}

displayd *
XGOBI(newParCoords)(gint *vars)
{
 displayd *display = NULL;

 display = display_create(2, &xg);

return(display);
}

displayd* 
XGOBI(createPlot)(int type, char **varnames)
{
 displayd *display = NULL;
 /*

   display_new(type);

 */
 return(display);
}


const gchar* 
XGOBI(getCurrentDisplayType)(xgobid *xg)
{
 return(XGOBI(getViewTypeName)(xg->current_display->displaytype));
}

const gchar *
XGOBI(getViewTypeName)(enum displaytyped type)
{
 int n, i;
 const gint *types = XGOBI(getViewTypeIndeces)(&n);
 const gchar * const *names = XGOBI(getViewTypes)(&n);

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
XGOBI(getRawData)()
{
 return((const gfloat**) xg.raw.data);
}

/*
  Pointer to the second transformation of the data managed by GGobi.
  Don't touch this.
 */
const gfloat** 
XGOBI(getTFormData)()
{
 return((const gfloat **)xg.tform2.data);
}


/*
  Returns a reference to the labels used to identify
  the observations.
  Do not change this as it is not a copy.
 */
const gchar **
XGOBI(getCaseNames)()
{
 return((const gchar **) xg.rowlab);
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
XGOBI(setCaseName)(gint index, const gchar *label)
{
  gchar *old;
  if(index < 0 || index >= xg.nrows) {
    warning("Index is out of range of observations in setCaseName");
    return; 
  }

  old = xg.rowlab[index];

  g_free(old);
  xg.rowlab[index] = (gchar *)label;
}


void
warning(const char *msg)
{
 fprintf(stderr, "%s\n", msg);
 fflush(stderr);
}



gint *
XGOBI(getGlyphTypes)(int *n)
{


 return(NULL);
}


gchar const*
XGOBI(getGlyphTypeName)(int type)
{
 gchar const *ans;
  ans = "Foo";

 return(ans);
}


gint *
XGOBI(getCaseGlyphTypes)(gint *ids, gint n)
{
 int i;
 gint *ans = (gint *) g_malloc(n * sizeof(int));

 for(i = 0; i < n ; i++)
   ans[i] = XGOBI(getCaseGlyphType)(ids[i]);

 return(ids);
}

gint 
XGOBI(getCaseGlyphType)(gint id)
{
 int index = xg.rows_in_plot[id];
  return(xg.glyph_ids[index].type);
}

gint *
XGOBI(getCaseGlyphSizes)(gint *ids, gint n)
{
 int i;
 gint *ans = (gint *) g_malloc(n * sizeof(int));

 for(i = 0; i < n ; i++)
   ans[i] = XGOBI(getCaseGlyphSize)(ids[i]);

 return(ids);
}

gint 
XGOBI(getCaseGlyphSize)(gint id)
{
 int index = xg.rows_in_plot[id];

  return(xg.glyph_ids[index].size);
}


void 
XGOBI(setCaseGlyph)(gint index, gint type, gint size)
{
  xg.glyph_ids[index].size = size;
  xg.glyph_ids[index].type = type;
}

void 
XGOBI(setCaseGlyphs)(gint *ids, gint n, gint type, gint size)
{
 int i;
 for(i = 0; i < n ; i++)
   XGOBI(setCaseGlyph)(ids[i], type, size);
}


void 
XGOBI(setCaseColor)(gint pt, gint colorIndex)
{
 xg.color_ids[pt] = xg.color_now[pt] = colorIndex;
}

void 
XGOBI(setCaseColors)(gint *pts, gint howMany, gint colorindx)
{
 int i;
 for(i = 0; i < howMany ; i++)
   XGOBI(setCaseColor)(pts[i], colorindx);
}


gint 
XGOBI(getCaseColor) (gint pt)
{
  return(xg.color_ids[pt]);
}

gint *
XGOBI(getCaseColors)(gint *pts, gint howMany)
{
 int i;
 gint *ans = (gint*) g_malloc(howMany * sizeof(int));

 for(i = 0; i < howMany ; i++)
  ans[i] = XGOBI(getCaseColor)(pts[i]);

 return(ans);
}



void
XGOBI(setObservationSegment)(gint x, gint y)
{
  if(XGOBI(isConnectedSegment)(x, y) == false) {
    segments_alloc(xg.nsegments+1);
    xg.segment_endpoints[xg.nsegments].a = x;
    xg.segment_endpoints[xg.nsegments].b = y;
    xg.nsegments++;
  }
}


gboolean 
XGOBI(isConnectedSegment)(gint a, gint b)
{
  gint tmp, i;

  if(a > b) {
     tmp = a;
     a = b;
     b = tmp;
  }

  for(i = 0; i < xg.nsegments ; i++) {
    
    if(xg.segment_endpoints[i].a == a && xg.segment_endpoints[i].b == b)
       return(true);

    if(xg.segment_endpoints[i].a > a) {
      return(false);
    } 
  }

 return(false);
}


gboolean 
XGOBI(getShowLines)()
{
 return(XGOBI(getDefaultDisplayOptions)()->segments_directed_show_p);
}


gboolean XGOBI(setShowLines)(gboolean val)
{
 gboolean old = XGOBI(getShowLines)();
 XGOBI(getDefaultDisplayOptions)()->segments_directed_show_p = val;

 return(old);
}

DisplayOptions *
XGOBI(getDefaultDisplayOptions)()
{
 return(&DefaultDisplayOptions);
}


displayd *
XGOBI(getDisplay)(int which)
{
  displayd *display = NULL;

  if(which < g_list_length(xg.displays))
   display = (displayd *) g_list_nth_data(xg.displays, which);

 return(display);
}

DisplayOptions *
XGOBI(getDisplayOptions)(int displayNum)
{
 DisplayOptions *options = NULL;
  if(displayNum < 0)
   options =  XGOBI(getDefaultDisplayOptions)();
  else {
   displayd *display;
    display = XGOBI(getDisplay)(displayNum);
    if(display)
      options = &(display->options);
  }

 return(options);
}


displayd *
XGOBI(getCurrentDisplay)()
{
 return(xg.current_display);
}

gint
XGOBI(getCurrentDisplayIndex)()
{
 return(g_list_index(xg.displays, xg.current_display));
}

displayd *
XGOBI(setCurrentDisplay)(int which)
{
 displayd *d;

 d = XGOBI(getDisplay)(which);

 if(d != NULL)
   display_set_current(d);

 return(d);
}


splotd *
XGOBI(getPlot)(displayd *display, int which)
{
  splotd *sp = (splotd *) g_list_nth_data(display->splots, which);
  return(sp);
}
