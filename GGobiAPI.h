#ifndef XGOBI_API_H
#define XGOBI_API_H


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
const gchar *getFileName ();
/*
  Returns whether the data was read from a binary
  or ASCII file.
 */
DataMode getDataMode();

/*
 * data, but also row and column labels, lines, colors, blahblah
 * how to do that from R?  Easier once we have xml i/o.

 * R and S would really like to provide a single vector/array
 * of doubles, arranged by column. Hence the double* for now.
 * A double array (double **) would be useful also.
*/
void setData (double *values, gchar *rownames, gchar *colnames);


/* Whether to get the transformed names or the regular ones. */
gchar **getVariableNames (gboolean transformed);



/*
   Set the name of the jvar'th variable, either the regular
   name or that of the variable's tranformed counterpart.
 */
void setVariableName (gint jvar, gchar *name, gboolean transformed);


extern gint XGOBI (main)(gint argc, gchar *argv[], gboolean processEvents);

/*
   Maps the name of a view type to its symbolic constant.
 */
gint getViewTypeIndex (gchar *viewType);

/*
  Converts a symbolic constant to the name of a view type.
  See getCurrentDisplay.
 */
const gchar *getViewTypeName(enum displaytyped type);

/*
  Returns a description of the type of the currently
  active display window.
 */
const gchar *getCurrentDisplayType ();


/*
   Sets the value of the argument to the number of
   view types.
   See  ViewTypes[] and ViewTypeIndeces[] in defines.h
 */
const gchar * const *getViewTypes (int *n);
/*
  Get the symblic constants associated identifying
  each plot type.
  The value of the argument is set to the length of the
  array.
  See getViewTypes.
      ViewTypes[] and ViewTypeIndeces[] in defines.h
 */
const gint  *getViewTypeIndeces(int *n);


/*
  Returns a pointer to the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
const gfloat** getRawData();

/*
  Returns a pointer to the (second) transformation of the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
const gfloat** getTFormData();


/*
  Closes the currently active display,
  unless that is the only one available.
  We may provide access to the force 
  argument of display_free, but this needs
  a little more finessing.
 */
void destroyCurrentDisplay();


 /*
    Returns a reference to the names of the observations
    used by GGobi to identify the rows in the data.
  */
const gchar **getCaseNames();

void setCaseName (gint pt, const gchar *lbl);


displayd *newScatterplot (gint ix, gint iy, gchar *viewType);
displayd *newScatmat (gint *rows, gint *columns);
displayd *newParCoords (gint *vars);
displayd *createPlot(int type, char **varnames);



gint *getGlyphTypes(int *n);

gint *getCaseGlyphTypes(gint *, gint n);
gint getCaseGlyphType(gint id);

gint *getCaseGlyphSizes(gint *, gint n);
gint getCaseGlyphSize(gint id);


void setCaseGlyph (gint pt, gint type, gint size);
void setCaseGlyphs (gint *pts, gint n, gint type, gint size);


gint getCaseColor (gint pt);
gint * getCaseColors (gint *pts, gint howMany);

void setCaseColor(gint pt, gint colorIndex);
void setCaseColors(gint *pts, gint howMany, gint colorindx);

gboolean  isConnectedSegment(gint a, gint b);
void setObservationSegment(gint x, gint y);
#endif
