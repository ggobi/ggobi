#ifndef GGOBI_API_H
#define GGOBI_API_H


#include <gtk/gtk.h>
#include "ggobi.h"
#include "defines.h"
#include "display.h"


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
extern void GGOBI(setData)(double *values, gchar **rownames, gchar **colnames, int nr, int nc, ggobid *gg);


/* Whether to get the transformed names or the regular ones. */
extern gchar **GGOBI(getVariableNames)(gboolean transformed, ggobid *gg);



/*
   Set the name of the jvar'th variable, either the regular
   name or that of the variable's tranformed counterpart.
 */
extern void GGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed, ggobid *gg);


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
   See  ViewTypes[] and ViewTypeIndeces[] in defines.h
 */
extern const gchar * const *GGOBI(getViewTypes) (int *n);
/*
  Get the symblic constants associated identifying
  each plot type.
  The value of the argument is set to the length of the
  array.
  See getViewTypes.
      ViewTypes[] and ViewTypeIndeces[] in defines.h
 */
extern const gint  *GGOBI(getViewTypeIndeces)(int *n);


/*
  Returns a pointer to the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
extern const gfloat** GGOBI(getRawData)();

/*
  Returns a pointer to the (second) transformation of the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
extern const gfloat** GGOBI(getTFormData)();


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

extern void GGOBI(setCaseName)(gint pt, const gchar *lbl, ggobid *gg);


extern displayd *GGOBI(newScatterplot)(gint ix, gint iy, ggobid *gg);
extern displayd *GGOBI(newScatmat)(gint *rows, gint *columns, int nr, int nc, ggobid *gg);
extern displayd *GGOBI(newParCoords)(gint *vars, int num, ggobid *gg);
extern displayd *createPlot(int type, char **varnames);

/*-- point glyph types and sizes --*/
extern gint *GGOBI(getGlyphTypes)(int *n);
extern const gchar **const GGOBI(getGlyphTypeNames)(int *n);

extern gint *GGOBI(getCaseGlyphTypes)(gint *, gint n, ggobid *gg);
extern gint GGOBI(getCaseGlyphType)(gint id, ggobid *gg);

extern gint *GGOBI(getCaseGlyphSizes)(gint *, gint n, ggobid *gg);
extern gint GGOBI(getCaseGlyphSize)(gint id, ggobid *gg);

extern void GGOBI(setCaseGlyph) (gint pt, gint type, gint size, ggobid *gg);
extern void GGOBI(setCaseGlyphs) (gint *pts, gint n, gint type, gint size, ggobid *gg);

/*-- point colors --*/
extern gint GGOBI(getCaseColor) (gint pt, ggobid *gg);
extern gint * GGOBI(getCaseColors) (gint *pts, gint howMany, ggobid *gg);

extern void GGOBI(setCaseColor)(gint pt, gint colorIndex, ggobid *gg);
extern void GGOBI(setCaseColors)(gint *pts, gint howMany, gint colorindx, ggobid *gg);

/*-- point hidden state --*/
extern gboolean GGOBI(getCaseHidden) (gint pt, ggobid *gg);
extern gboolean * GGOBI(getCaseHiddens) (gint *pts, gint howMany, ggobid *gg);

extern void GGOBI(setCaseHidden)(gint pt, gboolean hidden_p, ggobid *gg);
extern void GGOBI(setCaseHiddens)(gint *pts, gint howMany, gboolean hidden_p, ggobid *gg);

extern gboolean  GGOBI(isConnectedSegment)(gint a, gint b, ggobid *gg);
extern void GGOBI(setObservationSegment)(gint x, gint y, ggobid *gg);

extern gboolean GGOBI(getShowLines)();
extern gboolean GGOBI(setShowLines)(gboolean val);

extern DisplayOptions *GGOBI(getDefaultDisplayOptions)();

extern displayd *GGOBI(getDisplay)(int which, ggobid *gg);

extern DisplayOptions *GGOBI(getDisplayOptions)(int displayNum, ggobid *gg);


extern displayd * GGOBI(getCurrentDisplay)();
extern gint GGOBI(getCurrentDisplayIndex)();
/* gint GGOBI(getCurrentPlot)(displayd *display); */

extern displayd *GGOBI(setCurrentDisplay)(int which, ggobid *gg);

extern splotd *GGOBI(getPlot)(displayd *display, int which);


extern void GGOBI(moveBrush) (gint ulx, gint uly, ggobid *gg);
extern void GGOBI(sizeBrush) (gint width, gint height, ggobid *gg);

extern int GGOBI(getNumGGobis)();

gboolean GGOBI(setColorMap)(double *vals, int nr, ggobid *gg);
gboolean GGOBI(registerColorMap)(ggobid *gg);

gboolean GGOBI(close)(ggobid *gg, gboolean closeWindow);

void GGOBI(setIdentifyHandler)(IdentifyProc proc,  void *data, ggobid *gg);

/* These perhaps should be specified in natural coordinates.
   For now, pixels
*/

void GGOBI(getBrushSize)(int *w, int *h, ggobid *gg);
void GGOBI(getBrushLocation)(int *x, int *y, ggobid *gg);

void GGOBI(setBrushSize)(int w, int h, ggobid *gg);
void GGOBI(setBrushLocation)(int x, int y, ggobid *gg);


#endif /* End of conditional inclusion of entire file.*/

