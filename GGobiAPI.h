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
const gchar *GGOBI(getFileName)();
/*
  Returns whether the data was read from a binary
  or ASCII file.
 */
DataMode GGOBI(getDataMode)();
const gchar * const GGOBI(getDataModeDescription)(DataMode mode);

/*
 * data, but also row and column labels, lines, colors, blahblah
 * how to do that from R?  Easier once we have xml i/o.

 * R and S would really like to provide a single vector/array
 * of doubles, arranged by column. Hence the double* for now.
 * A double array (double **) would be useful also.
*/
void GGOBI(setData)(double *values, gchar **rownames, gchar **colnames, int nr, int nc);


/* Whether to get the transformed names or the regular ones. */
gchar **GGOBI(getVariableNames)(gboolean transformed);



/*
   Set the name of the jvar'th variable, either the regular
   name or that of the variable's tranformed counterpart.
 */
void GGOBI(setVariableName)(gint jvar, gchar *name, gboolean transformed);


extern gint GGOBI (main)(gint argc, gchar *argv[], gboolean processEvents);

/*
   Maps the name of a view type to its symbolic constant.
 */
gint GGOBI(getViewTypeIndex) (gchar *viewType);

/*
  Converts a symbolic constant to the name of a view type.
  See getCurrentDisplay.
 */
const gchar *GGOBI(getViewTypeName)(enum displaytyped type);

/*
  Returns a description of the type of the currently
  active display window.
 */
const gchar *GGOBI(getCurrentDisplayType)();


/*
   Sets the value of the argument to the number of
   view types.
   See  ViewTypes[] and ViewTypeIndeces[] in defines.h
 */
const gchar * const *GGOBI(getViewTypes) (int *n);
/*
  Get the symblic constants associated identifying
  each plot type.
  The value of the argument is set to the length of the
  array.
  See getViewTypes.
      ViewTypes[] and ViewTypeIndeces[] in defines.h
 */
const gint  *GGOBI(getViewTypeIndeces)(int *n);


/*
  Returns a pointer to the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
const gfloat** GGOBI(getRawData)();

/*
  Returns a pointer to the (second) transformation of the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
const gfloat** GGOBI(getTFormData)();


/*
  Closes the currently active display,
  unless that is the only one available.
  We may provide access to the force 
  argument of display_free, but this needs
  a little more finessing.
 */
void GGOBI(destroyCurrentDisplay)();


 /*
    Returns a reference to the names of the observations
    used by GGobi to identify the rows in the data.
  */
const gchar ** GGOBI(getCaseNames)();

void GGOBI(setCaseName)(gint pt, const gchar *lbl);


displayd *newScatterplot (gint ix, gint iy, gchar *viewType);
displayd *newScatmat (gint *rows, gint *columns);
displayd *newParCoords (gint *vars);
displayd *createPlot(int type, char **varnames);


gint *GGOBI(getGlyphTypes)(int *n);

gint *GGOBI(getCaseGlyphTypes)(gint *, gint n);
gint GGOBI(getCaseGlyphType)(gint id);

gint *GGOBI(getCaseGlyphSizes)(gint *, gint n);
gint GGOBI(getCaseGlyphSize)(gint id);


void GGOBI(setCaseGlyph) (gint pt, gint type, gint size);
void GGOBI(setCaseGlyphs) (gint *pts, gint n, gint type, gint size);


gint GGOBI(getCaseColor) (gint pt);
gint * GGOBI(getCaseColors) (gint *pts, gint howMany);

void GGOBI(setCaseColor)(gint pt, gint colorIndex);
void GGOBI(setCaseColors)(gint *pts, gint howMany, gint colorindx);

gboolean  GGOBI(isConnectedSegment)(gint a, gint b);
void setObservationSegment(gint x, gint y);

gboolean GGOBI(getShowLines)();
gboolean GGOBI(setShowLines)(gboolean val);

DisplayOptions *GGOBI(getDefaultDisplayOptions)();

displayd *GGOBI(getDisplay)(int which);

DisplayOptions *GGOBI(getDisplayOptions)(int displayNum);


displayd * GGOBI(getCurrentDisplay)();
gint GGOBI(getCurrentDisplayIndex)();
/* gint GGOBI(getCurrentPlot)(displayd *display); */

displayd *GGOBI(setCurrentDisplay)(int which);

splotd *GGOBI(getPlot)(displayd *display, int which);


void GGOBI(moveBrush) (gint ulx, gint uly);
void GGOBI(sizeBrush) (gint width, gint height);

#endif /* End of conditional inclusion of entire file.*/

