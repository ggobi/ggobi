#ifndef GGOBI_API_H
#define GGOBI_API_H

#include <gtk/gtk.h>
#include "ggobi.h"
#include "defines.h"
#include "display.h"

#ifdef __cplusplus 
extern "C" {
#endif
/*
  The routines that copy values into an array
  may not be suitable for large datasets.
  In these cases, we may want to allow the caller
  to pass in a pre-allocated space. This is true
  of R & S, and other applications that manage the
  memory in different ways than malloc/calloc.
 */

/*
  Returns 
  This is not a copy of the value used by ggobi.
 */
const gchar *GGOBI(getFileName)(ggobid *gg);
/*
  Returns whether the data was read from a binary
  or ASCII file.
 */
extern DataMode GGOBI(getDataMode)(ggobid *gg);
extern const gchar * const GGOBI(getDataModeDescription)(DataMode mode);

/*
 * data, but also row and column labels, lines, colors, blahblah
 * how to do that from R?  Easier once we have xml i/o.

 * R and S would really like to provide a single vector/array
 * of doubles, arranged by column. Hence the double* for now.
 * A double array (double **) would be useful also.
*/
extern void GGOBI(setData)(gdouble *values, gchar **rownames, gchar **colnames, gint nr, gint nc, datad *d, ggobid *gg);


/* Whether to get the transformed names or the regular ones. */
extern gchar **GGOBI(getVariableNames)(gboolean transformed, datad *, ggobid *);



/*
   Set the name of the jvar'th variable, either the regular
   name or that of the variable's tranformed counterpart.
 */
extern void GGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed, datad *, ggobid *gg);


extern gint GGOBI (main)(gint argc, gchar *argv[], gboolean processEvents);

/*
   Maps the name of a view type to its symbolic constant.
 */
extern gint GGOBI(getViewTypeIndex) (gchar *viewType);

/*
  Converts a symbolic constant to the name of a view type.
  See getCurrentDisplay.
 */
extern const gchar *GGOBI(getViewTypeName)(enum displaytyped type);

/*
  Returns a description of the type of the currently
  active display window.
 */
extern const gchar *GGOBI(getCurrentDisplayType)();


/*
   Sets the value of the argument to the number of
   view types.
   See  ViewTypes[] and ViewTypeIndices[] in defines.h
 */
extern const gchar * const *GGOBI(getViewTypes) (int *n);
/*
  Get the symblic constants associated identifying
  each plot type.
  The value of the argument is set to the length of the
  array.
  See getViewTypes.
      ViewTypes[] and ViewTypeIndices[] in defines.h
 */
extern const gint  *GGOBI(getViewTypeIndices)(int *n);


/*
  Returns a pointer to the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
extern const gfloat** GGOBI(getRawData)(datad *, ggobid *);

/*
  Returns a pointer to the (second) transformation of the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
extern const gfloat** GGOBI(getTFormData)(datad *, ggobid *);


/*
  Closes the currently active display,
  unless that is the only one available.
  We may provide access to the force 
  argument of display_free, but this needs
  a little more finessing.
 */
extern void GGOBI(destroyCurrentDisplay)();


 /*
    Returns a reference to the names of the observations
    used by GGobi to identify the rows in the data.
  */
extern const gchar ** GGOBI(getCaseNames)();

extern void GGOBI(setCaseName)(gint pt, const gchar *lbl, datad *d, ggobid *gg);


extern displayd *GGOBI(newScatterplot)(gint ix, gint iy, datad *, ggobid *gg);
extern displayd *GGOBI(newScatmat)(gint *rows, gint *columns, gint nr, gint nc, datad *, ggobid *gg);
extern displayd *GGOBI(newParCoords)(gint *vars, gint num, datad *, ggobid *gg);
extern displayd *createPlot(gint type, gchar **varnames);

/*-- point glyph types and sizes --*/
extern gint *GGOBI(getGlyphTypes)(gint *n);
extern const gchar **const GGOBI(getGlyphTypeNames)(gint *n);

extern gint *GGOBI(getCaseGlyphTypes)(gint *, gint n, datad *, ggobid *gg);
extern gint GGOBI(getCaseGlyphType)(gint id, datad *, ggobid *gg);

extern gint *GGOBI(getCaseGlyphSizes)(gint *, gint n, datad *, ggobid *gg);
extern gint GGOBI(getCaseGlyphSize)(gint id, datad *, ggobid *gg);

extern void GGOBI(setCaseGlyph) (gint pt, gint type, gint size, datad *d, ggobid *gg);
extern void GGOBI(setCaseGlyphs) (gint *pts, gint n, gint type, gint size, datad *d, ggobid *gg);

/*-- point colors --*/
extern gint GGOBI(getCaseColor) (gint pt, datad *, ggobid *gg);
extern gint * GGOBI(getCaseColors) (gint *pts, gint howMany, datad *, ggobid *gg);

extern void GGOBI(setCaseColor)(gint pt, gint colorIndex, datad *, ggobid *gg);
extern void GGOBI(setCaseColors)(gint *pts, gint howMany, gint colorindx, datad *, ggobid *gg);

/*-- point hidden state --*/
extern gboolean GGOBI(getCaseHidden) (gint pt, datad *, ggobid *gg);
extern gboolean * GGOBI(getCaseHiddens) (gint *pts, gint howMany, datad *, ggobid *gg);

extern void GGOBI(setCaseHidden)(gint pt, gboolean hidden_p, datad *, ggobid *gg);
extern void GGOBI(setCaseHiddens)(gint *pts, gint howMany, gboolean hidden_p, datad *, ggobid *gg);

extern gboolean  GGOBI(isConnectedEdge)(gint a, gint b, ggobid *gg);
extern void GGOBI(setObservationEdge)(gint x, gint y, datad *, ggobid *, gboolean update);

extern gboolean GGOBI(getShowLines)();
extern gboolean GGOBI(setShowLines)(gboolean val);

extern DisplayOptions *GGOBI(getDefaultDisplayOptions)();

extern displayd *GGOBI(getDisplay)(gint which, ggobid *gg);

extern DisplayOptions *GGOBI(getDisplayOptions)(gint displayNum, ggobid *gg);


extern displayd * GGOBI(getCurrentDisplay)(ggobid *gg);
extern gint GGOBI(getCurrentDisplayIndex)(ggobid *gg);
extern gint GGOBI(getCurrentPlotIndex)(ggobid *gg);

extern displayd *GGOBI(setCurrentDisplay)(gint which, ggobid *gg);

extern splotd *GGOBI(getPlot)(displayd *display, gint which);


extern void GGOBI(moveBrush) (gint ulx, gint uly, ggobid *gg);
extern void GGOBI(sizeBrush) (gint width, gint height, ggobid *gg);

extern int GGOBI(getNumGGobis)();

gboolean GGOBI(setColorMap)(gdouble *vals, gint nr, ggobid *gg);
gboolean GGOBI(registerColorMap)(ggobid *gg);

gboolean GGOBI(close)(ggobid *gg, gboolean closeWindow);

void GGOBI(setIdentifyHandler)(IdentifyProc proc,  void *data, ggobid *gg);

/* These perhaps should be specified in natural coordinates.
   For now, pixels
*/

extern void GGOBI(getBrushGlyph)(gint *type, gint *size, ggobid *gg);

void GGOBI(getBrushSize)(gint *w, gint *h, datad *, ggobid *gg);
void GGOBI(getBrushLocation)(gint *x, gint *y, datad *, ggobid *gg);

void GGOBI(setBrushSize)(gint w, gint h, datad *, ggobid *gg);
void GGOBI(setBrushLocation)(gint x, gint y, datad *, ggobid *gg);

extern splotd *GGOBI(getSPlot)(gint which, displayd *display);

extern const gchar * const* GGOBI(getModeNames)(int *);
extern gint GGOBI(getModeId)(const gchar *name);
extern gint GGOBI(setMode)(const gchar *name, ggobid *gg);
extern const gchar *GGOBI(getModeName)(gint which);
extern int GGOBI(full_mode_set)(gint action, ggobid *gg);

extern int GGOBI(setBrushColor)(gint cid, ggobid *gg);
extern gboolean GGOBI(setBrushGlyph)(gint type, gint size, ggobid *gg);
extern int GGOBI(getVariableIndex)(const gchar *name, datad *, ggobid *gg);
extern int GGOBI(removeVariableByIndex)(gint which, datad *, ggobid *gg);

extern gboolean GGOBI(setVariableValues)(gint whichVar, gdouble *vals, gint num, gboolean update, datad *d, ggobid *gg);

/* Need len just in case there is no data in the instance*/
extern int GGOBI(addVariable)(gdouble *vals, gint len, gchar *name, gboolean update, datad *d, ggobid *gg);

extern void GGOBI(update_data)(datad *, ggobid *gg);
extern gboolean GGOBI(raiseWindow)(gint which, gboolean raiseOrIcon, gboolean up, ggobid *gg);


extern gchar *GGOBI(getDescription)(ggobid *gg);
extern void GGOBI(splot_set_current_full)(displayd *display, splotd *sp, ggobid *gg);

#ifdef __cplusplus
}
#endif


gchar *datasetName(datad *d, int which);

#endif /* End of conditional inclusion of entire file.*/

