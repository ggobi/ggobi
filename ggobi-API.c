#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "GGobiAPI.h"

#include "ggobi.h"
#include "types.h"
#include "vars.h"
#include "externs.h"
#include "display.h"

 extern const gchar *GlyphNames[];

void warning(const char *msg);

void GGOBI(displays_release)(ggobid *gg);
void GGOBI(display_release)(displayd *display, ggobid *gg);
void GGOBI(splot_release)(splotd *sp, displayd *display, ggobid *gg);
void GGOBI(data_release)(ggobid *gg);
void GGOBI(vardata_free)(ggobid *gg);
void GGOBI(vardatum_free)(vardatad *var, ggobid *gg);

const gchar *
GGOBI(getFileName) (ggobid *gg)
{
  return(gg->filename);
}


DataMode
GGOBI(getDataMode) (ggobid *gg)
{
  return(gg->data_mode);
}

const gchar * const 
GGOBI(getDataModeDescription)(DataMode mode)
{
 extern const gchar * const ModeNames[];
 return(ModeNames[mode]);
}

gchar **
GGOBI(getVariableNames)(int transformed, ggobid *gg)
{
  gchar **names;
  int nc = gg->ncols, i;
  vardatad *form;

  names = (gchar**) g_malloc(sizeof(gchar*)*nc);

    for(i = 0; i < nc; i++) {
      form = gg->vardata + i;
      names[i] = transformed ? form->collab_tform : form->collab;
    }

  return(names);
}


void
GGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed, ggobid *gg)
{
 gchar *old;

 if(transformed)
   old = gg->vardata[jvar].collab_tform;
 else
   old = gg->vardata[jvar].collab;

 if(old)
   g_free(old);

 if(transformed)
   gg->vardata[jvar].collab_tform = g_strdup(name);
 else
   gg->vardata[jvar].collab = g_strdup(name);
}


/*
  Closes the specified display
 */
void 
GGOBI(destroyCurrentDisplay)(ggobid *gg)
{
  display_free (gg->current_display, false, gg);
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
GGOBI(setData)(double *values, gchar **rownames, gchar **colnames, int nr, int nc, ggobid *gg)
{
 int i, j;

  GGOBI(displays_release)(gg);
  GGOBI(data_release)(gg);

  gg->ncols = nc;
  gg->nrows = nr;

  gg->nrows_in_plot = gg->nrows;  /*-- for now --*/
  gg->nrgroups = 0;              /*-- for now --*/

  gg->rows_in_plot = NULL;

  arrayf_alloc(&gg->raw, nr, nc);

  rowlabels_alloc(gg);
  /*  gg.rowlab = (gchar **) g_malloc(nr * sizeof(gchar*)); */

  vardata_alloc(gg);
  vardata_init(gg);

  for(j = 0; j < nc ; j++) {
   gg->vardata[j].collab = g_strdup(colnames[j]);
   gg->vardata[j].collab_tform = g_strdup(colnames[j]);
   for(i = 0; i < nr ; i++) {
     if(j == 0) {
       gg->rowlab[i] = g_strdup(rownames[i]);
     }

     gg->raw.data[i][j] = values[i + j*nr];
   }
  }

   /* Now recompute and display the top plot. */
  dataset_init(gg, false);

  /* Have to patch up the displays list since we removed
     every entry and that makes for meaningless entries.
   */
  gg->displays->next = NULL;
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
  display_free(display, true, gg);
 }
}

void
GGOBI(display_release)(displayd *display, ggobid *gg)
{
   display_free(display, true, gg);
}


void
GGOBI(splot_release)(splotd *sp, displayd *display, ggobid *gg)
{
 splot_free(sp, display, gg);
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
GGOBI(newScatterplot) (gint ix, gint iy, ggobid *gg)
{
 displayd *display = NULL;
 splotd* sp;

 display = display_alloc_init(scatterplot, false, gg);
 sp = splot_new (display, 400, 400, gg);

 sp->xyvars.x = ix;
 sp->xyvars.y = iy;

 display = scatterplot_new(false, sp, gg);
 display_add(display, gg);

return(display);
}

displayd *
GGOBI(newScatmat) (gint *rows, gint *columns, int nr, int nc, ggobid *gg)
{
 displayd *display = NULL;
 splotd **plots;
 int i, j, ctr;

  display = display_alloc_init(scatmat, false, gg);
  plots = (splotd**) g_malloc(sizeof(splotd*) * nr*nc);
  ctr = 0;
  for(i = 0 ; i < nr ; i++) {
    for(j = 0 ; j < nc ; j++, ctr++) {
      plots[ctr] = splot_new (display, 400, 400, gg);
      plots[ctr]->xyvars.x = rows[i];
      plots[ctr]->xyvars.y = columns[j];
      plots[ctr]->p1dvar = (rows[i] == columns[j]) ? rows[i] : -1;
    }
  }

  display = scatmat_new(false, plots, nr, nc, gg);
  display_add(display, gg);


  g_free(plots);

return(display);
}

displayd *
GGOBI(newParCoords)(gint *vars, gint numVars, ggobid *gg)
{
 displayd *display = NULL;
 splotd **plots;
 int i;

  display = display_alloc_init(parcoords, false, gg);
  plots = (splotd**) g_malloc(sizeof(splotd*) * numVars);
  for(i = 0 ; i < numVars; i++) {
    plots[i] = splot_new (display, 400, 400, gg);
    plots[i]->p1dvar = vars[i];
  }

  display = parcoords_new(false, plots, numVars, gg);
  display_add(display, gg);

  g_free(plots);

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
GGOBI(getRawData)(ggobid *gg)
{
 return((const gfloat**) gg->raw.data);
}

/*
  Pointer to the second transformation of the data managed by GGobi.
  Don't touch this.
 */
const gfloat** 
GGOBI(getTFormData)(ggobid *gg)
{
 return((const gfloat **)gg->tform2.data);
}


/*
  Returns a reference to the labels used to identify
  the observations.
  Do not change this as it is not a copy.
 */
const gchar **
GGOBI(getCaseNames)(ggobid *gg)
{
 return((const gchar **) gg->rowlab);
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
GGOBI(setCaseName)(gint index, const gchar *label, ggobid *gg)
{
  gchar *old;
  if(index < 0 || index >= gg->nrows) {
    warning("Index is out of range of observations in setCaseName");
    return; 
  }

  old = gg->rowlab[index];

  g_free(old);
  gg->rowlab[index] = (gchar *)label;
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
 static gint *glyphIds = NULL;
 *n = UNKNOWN_GLYPH-1; /* -1 since we start at 1 */

  if(glyphIds == NULL){
   int i;
   glyphIds = (gint*) g_malloc(*n* sizeof(gint));
   for(i = 0; i < *n ; i++) {
    glyphIds[i] = mapGlyphName(GlyphNames[i]);
   }
  }

 return(glyphIds);
}

const gchar **const
GGOBI(getGlyphTypeNames)(int *n)
{
  *n = UNKNOWN_GLYPH-1; /* -1 since we start at 1 */
  return(GlyphNames);
}


gchar const*
GGOBI(getGlyphTypeName)(int type)
{
 gchar const *ans;
  ans = GlyphNames[type-1];

 return(ans);
}


gint *
GGOBI(getCaseGlyphTypes)(gint *ids, gint n, ggobid *gg)
{
 int i;
 gint *ans = (gint *) g_malloc(n * sizeof(int));

 for(i = 0; i < n ; i++)
   ans[i] = GGOBI(getCaseGlyphType)(ids[i], gg);

 return(ids);
}

gint 
GGOBI(getCaseGlyphType)(gint id, ggobid *gg)
{
 int index = gg->rows_in_plot[id];
  return(gg->glyph_ids[index].type);
}

gint *
GGOBI(getCaseGlyphSizes)(gint *ids, gint n, ggobid *gg)
{
 int i;
 gint *ans = (gint *) g_malloc(n * sizeof(int));

 for(i = 0; i < n ; i++)
   ans[i] = GGOBI(getCaseGlyphSize)(ids[i], gg);

 return(ids);
}

gint 
GGOBI(getCaseGlyphSize)(gint id, ggobid *gg)
{
 int index = gg->rows_in_plot[id];

  return(gg->glyph_ids[index].size);
}


void 
GGOBI(setCaseGlyph)(gint index, gint type, gint size, ggobid *gg)
{
  gg->glyph_ids[index].size = size;
  gg->glyph_ids[index].type = type;
}

void 
GGOBI(setCaseGlyphs)(gint *ids, gint n, gint type, gint size, ggobid *gg)
{
 int i;
 for(i = 0; i < n ; i++)
   GGOBI(setCaseGlyph)(ids[i], type, size, gg);
}


void 
GGOBI(setCaseColor)(gint pt, gint colorIndex, ggobid *gg)
{
 gg->color_ids[pt] = gg->color_now[pt] = colorIndex;
}

void 
GGOBI(setCaseColors)(gint *pts, gint howMany, gint colorindx, ggobid *gg)
{
 int i;
 for(i = 0; i < howMany ; i++)
   GGOBI(setCaseColor)(pts[i], colorindx, gg);
}


gint 
GGOBI(getCaseColor) (gint pt, ggobid *gg)
{
  return(gg->color_ids[pt]);
}

gint *
GGOBI(getCaseColors)(gint *pts, gint howMany, ggobid *gg)
{
 int i;
 gint *ans = (gint*) g_malloc(howMany * sizeof(int));

 for(i = 0; i < howMany ; i++)
  ans[i] = GGOBI(getCaseColor)(pts[i], gg);

 return(ans);
}



void
GGOBI(setObservationSegment)(gint x, gint y, ggobid *gg)
{
  if(GGOBI(isConnectedSegment)(x, y, gg) == false) {
    segments_alloc(gg->nsegments+1, gg);
    gg->segment_endpoints[gg->nsegments].a = x;
    gg->segment_endpoints[gg->nsegments].b = y;
    gg->nsegments++;
  }
}


gboolean 
GGOBI(isConnectedSegment)(gint a, gint b, ggobid *gg)
{
  gint tmp, i;

  if(a > b) {
     tmp = a;
     a = b;
     b = tmp;
  }

  for(i = 0; i < gg->nsegments ; i++) {
    
    if(gg->segment_endpoints[i].a == a && gg->segment_endpoints[i].b == b)
       return(true);

    if(gg->segment_endpoints[i].a > a) {
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
GGOBI(getDisplay)(int which, ggobid *gg)
{
  displayd *display = NULL;

  if(which < g_list_length(gg->displays))
   display = (displayd *) g_list_nth_data(gg->displays, which);

 return(display);
}

DisplayOptions *
GGOBI(getDisplayOptions)(int displayNum, ggobid *gg)
{
 DisplayOptions *options = NULL;
  if(displayNum < 0)
   options =  GGOBI(getDefaultDisplayOptions)();
  else {
   displayd *display;
    display = GGOBI(getDisplay)(displayNum, gg);
    if(display)
      options = &(display->options);
  }

 return(options);
}


displayd *
GGOBI(getCurrentDisplay)(ggobid *gg)
{
 return(gg->current_display);
}

gint
GGOBI(getCurrentDisplayIndex)(ggobid *gg)
{
 return(g_list_index(gg->displays, gg->current_display));
}

displayd *
GGOBI(setCurrentDisplay)(int which, ggobid *gg)
{
 displayd *d;

 d = GGOBI(getDisplay)(which, gg);

 if(d != NULL)
   display_set_current(d, gg);

 return(d);
}


splotd *
GGOBI(getPlot)(displayd *display, int which)
{
  splotd *sp = (splotd *) g_list_nth_data(display->splots, which);
  return(sp);
}


int
GGOBI(getNumGGobis)()
{
 extern int num_ggobis;
 return(num_ggobis);
}
