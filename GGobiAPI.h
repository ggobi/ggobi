#ifndef GGOBI_API_H
#define GGOBI_API_H

/** @file 
    @nosubgrouping
*/  /*(for doxygen)*/

#include <gtk/gtk.h>
#include "ggobi.h"
#include "defines.h"
#include "display.h"

/**
 This is the publically accessible set of routines that allow a developer
 to access GGobi functionality from within their own application, 
 facilitating the embedding of GGobi in other software.
 The routines and data structures allow one to instantiate
 one or more independent GGobi objects (ggobid) and
 to query, set and modify the datasets and their variables and values.
 Within a ggobid object, one can create new plots of different types.
 One can programmatically control the content and appearance of the these plots, 
 including the color and glyph of individual points.
 One can programmatically brush regions of a plot.
 */

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

/**
 @defgroup Version Version Information
 @brief Meta information about the GGobi release.

 */
/**
 @ingroup Version
  Retrieve a string describing the ``release'' or distribution
  date of this version of ggobi. 
  This is useful for determining if certain features are available,
  bugs have been fixed, etc. 
  @see GGobi_getVersionString()
  @see GGobi_getVersionNumbers()
*/
extern  char * const GGOBI(getVersionDate)(void);

/**
 @ingroup Version
  Retrieve a string describing the ``release'' or distribution
  date of this version of ggobi. 
  This is useful for determining if certain features are available,
  bugs have been fixed, etc. 
*/
extern  char * const GGobi_getVersionString();

/**
 @ingroup Version
  Returns a triplet of integers giving the
  major, minor and patch level numbers for this particular
  distribution of GGobi. 
  These can be used to determine the characteristics
  of this version of the GGobi software.
*/
extern  int* const GGobi_getVersionNumbers();


/**
  @defgroup GGobi Meta-information for a GGobi Instance
  @brief Readable access for meta information about a GGobi instance.
 */

/**
 @ingroup GGobi
  Returns the name of the data source (i.e. file,
  URL, ...) associated with the (first) dataset in the 
  specified GGobi instance.
  Note that the returned value  is not a copy of the value 
  and is used by ggobi. If you alter it, it will be altered
  in the GGobi instance also.
 */
const gchar *GGobi_getFileName(ggobid *gg);

/**
 @ingroup GGobi

 Read the data from the specified file..
 To force a particular data mode (e.g. XML, ASCII, binary, etc.)
 rather than leaving it to the auto-detection,  use setDataMode()
 before calling this routine..
*/
const gchar *GGobi_setFileName(const gchar *fileName, DataMode data_mode, ggobid *gg);

/**
 @ingroup GGobi
  Returns whether the data was read from a binary
  or ASCII file.
 */
extern DataMode GGobi_getDataMode(ggobid *gg);
/**
 @ingroup GGobi
 */
extern const gchar * const GGobi_getDataModeDescription(DataMode mode);

/**
 @ingroup GGobi
 */
extern DataMode GGobi_setDataMode(DataMode newMode, ggobid *gg);

/**
 @ingroup GGobi
 * data, but also row and column labels, lines, colors, blahblah
 * how to do that from R?  Easier once we have xml i/o.

 * R and S would really like to provide a single vector/array
 * of doubles, arranged by column. Hence the double* for now.
 * A double array (double **) would be useful also.
*/
extern void GGobi_setData(gdouble *values, gchar **rownames, gchar **colnames,
  gint nr, gint nc, datad *d, gboolean initPlot, ggobid *gg,
  gchar **ids, gboolean duplicate, InputDescription *);


extern void GGobi_setDataName(const char * const name, datad *d);

/** 
 @ingroup GGobi
  Whether to get the transformed names or the regular ones. 

*/
extern gchar **GGobi_getVariableNames(gboolean transformed, datad *, ggobid *);



/**
 @ingroup GGobi
   Set the name of the jvar'th variable, either the regular
   name or that of the variable's tranformed counterpart.
 */
extern void GGobi_setVariableName(gint jvar, gchar *name, gboolean transformed, datad *, ggobid *gg);


/**

 @ingroup GGobi

 This provides a way to initialize and run GGobi as it would be in the 
 stand-alone application, but controlling whether the standard
 GGobi event loop is run or whether control is returned to the caller
 after the usual GGobi initialization is complete. 
 This is used to when embedding GGobi in other applications that want
 control when and how GGobi events are processed.
 */
extern gint GGobi_main(gint argc, gchar *argv[], gboolean processEvents);

/**
 @ingroup GGobi
   Maps the name of a view type to its symbolic constant.
 */
extern gint GGobi_getViewTypeIndex(gchar *viewType);

/**
 @ingroup GGobi
  Converts a symbolic constant to the name of a view type.
  See getCurrentDisplay.
 */
extern const gchar *GGobi_getViewTypeName(displayd *dpy);

/**
 @ingroup GGobi
  Returns a description of the type of the currently
  active display window.
 */
extern const gchar *GGobi_getCurrentDisplayType(ggobid *gg);


/**
 @ingroup GGobi
   Sets the value of the argument to the number of
   view types.
   See  ViewTypes[] and ViewTypeIndices[] in defines.h
 */
extern const gchar * const *GGobi_getViewTypes(int *n);

/**
  Get the symblic constants associated identifying
  each plot type.
  The value of the argument is set to the length of the
  array.
  See getViewTypes.
      ViewTypes[] and ViewTypeIndices[] in defines.h
 */
extern const gint  *GGobi_getViewTypeIndices(int *n);


/**
  Returns a pointer to the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
extern const gfloat** GGobi_getRawData(datad *, ggobid *);

/**
  Returns a pointer to the (second) transformation of the raw data.
  This is not a copy, but the actual data used by the
  internals of GGobi.
 */
extern const gfloat** GGobi_getTFormData(datad *, ggobid *);


/**
  Closes the currently active display,
  unless that is the only one available.
  We may provide access to the force 
  argument of display_free, but this needs
  a little more finessing.
 */
extern void GGobi_destroyCurrentDisplay();


/**
    Returns a reference to the names of the observations
    used by GGobi to identify the rows in the data.
  */
extern const gchar ** GGobi_getCaseNames();

/**
 */
extern void GGobi_setCaseName(gint pt, const gchar *lbl, datad *d, ggobid *gg);

/**
 @defgroup Displays Top-level Displays
 @brief  
 This set of routines relate to creating new top-level display windows
 within a GGobi instance, each display  containing one or more plots.
*/

/**
 @ingroup Displays
 Create a new display containing a single 
 scatterplot of (X, Y) of the variables identified by index 
 in the specified dataset in the given GGobi instance.
 @param ix the index of the variable in the dataset to use as the X variable.  
 @param iy the index of the variable in the dataset to use as the X variable.  
 @param data   reference to the dataset in which to find/resolve the variables.
 @param gg  the GGobi instance in which to find the dataset and to which the 
  display will belong.

 @return a new top-level display object.
 */
extern displayd *GGobi_newScatterplot(gint ix, gint iy, datad *data, ggobid *gg);

/**

 @ingroup Displays
 Create a top-level display containing a matrix of 
 scatterplots for each of the different pairs of 
 variables identified by the indices in rows and columns
 within the specified dataset within the GGobi instance.
 @param rows 
 @param columns
 @param nr 
 @param nc
 */
extern displayd *GGobi_newScatmat(gint *rows, gint *columns, gint nr, gint nc, datad *, ggobid *gg);

/**
 @ingroup Displays
 Create a new top-level display containing 
 parallel coordinate plots for the different 
 variables identified in the dataset within the GGobi
 instance by index (rather than name).
 The order of the plots is determined by the order
 in which the variables are given in the array.
 */
extern displayd *GGobi_newParCoords(gint *vars, gint num, datad *, ggobid *gg);

/**
 @ingroup Displays
 Create a top-level display containing a time series 
 plot for each of the variables identified by index
 via the argument vars in the specified
 dataset within the GGobi instance.
 */
extern displayd *GGobi_newTimeSeries(gint *yvars, gint numVars, datad *d, ggobid *gg);

/**
 @ingroup Displays
 */
extern displayd *createPlot(gint type, gchar **varnames);


/**
 @ingroup Displays
 Create a top-level display for a matrix of X-Y/scatter plots.

 See GGobi_newScatmat()
 */
gint  *createScatmatWindow(gint nrows, gint ncols, displayd *display, ggobid *gg, gboolean useWindow);



/**
  @defgroup DisplayAccess Accessors for Top-level Display Windows
  @brief Accessor routines for managing the top-level plot container
  windows.

*/

/* @{ */

/**
 @ingroup DisplayAccess
 */
extern displayd *GGobi_getDisplay(gint which, ggobid *gg);

/**
 @ingroup DisplayAccess
 */
extern DisplayOptions *GGobi_getDefaultDisplayOptions();
/**
 @ingroup DisplayAccess
 */
extern DisplayOptions *GGobi_getDisplayOptions(gint displayNum, ggobid *gg);
/**
 @ingroup DisplayAccess
 */
extern displayd *GGobi_setCurrentDisplay(gint which, ggobid *gg);

/**
 @ingroup DisplayAccess
 */
extern displayd *GGobi_getCurrentDisplay(ggobid *gg);

extern gint GGobi_getCurrentDisplayIndex(ggobid *gg);


/* @} */

/**
 */
extern void GGobi_setPlotRange(double *x, double *y, int plotNum, displayd *display, gboolean pixels, ggobid *gg);


/*-- point glyph types and sizes --*/
/**
 @defgroup Glyphs Glyphs
 @brief Accessors for each observations glyph, including
   type, color and size.
 */

/**
 @ingroup Glyphs
 Retrieve the list of the internal codes used for 
 representing the glyph types that GGobi understands.

 @return an array of integers giving the enumerated values for the different
  glyph types. The number of elements in the array
  is contained in the value of the argument (n).
 */
extern gint *GGobi_getGlyphTypes(gint *n);

/**
 Retrieve the list of the symbolic names associated
 with the glyph types that GGobi understands.
 The elements returned correspond to the elements returned
 by GGobi_getGlyphTypes().
 @return an array of strings. The number of elements in the array
  is contained in the value of the argument (n).
 @ingroup Glyphs
 */
extern const gchar **const GGobi_getGlyphTypeNames(gint *n);

/**
 Get the symbolic name of the glyph corresponding to the
 internal constant that represents the glyph type.

 @return a string giving the human readable name of the glyph type.

 @ingroup Glyphs
 */
extern gchar const* GGobi_getGlyphTypeName(gint n);

/**
 Retrieve the current glyph types for one or more observations.
 This returns the internal representation of the glyph type
 which can be converted to a symbolic, human-readable name
 using GGobi_getGlyphTypeName().

 @return an array of length `n' whose i-th entry 
  contains the current glyph type
 for the observation identified by the i-th value in the argument
 `which'
 @see GGobi_getGlyphTypeName()
 
 @ingroup Glyphs
 */
extern gint *GGobi_getCaseGlyphTypes(gint *which, gint n, datad *dataset, ggobid *gg);

/**
 Get the glyph type attribute for a single observation

 @param id the index of the observation of interest
 @param dataset the dataset within the GGobi instance in which to find the observation.
 @param gg the GGobi instance in which to find the dataset.
 @return the internal representation of a glyph, which is a symbolic constant.

 @see GGobi_getGlyphTypeName()
 @ingroup Glyphs
 */
extern gint GGobi_getCaseGlyphType(gint id, datad *dataset, ggobid *gg);

/**
 Get the glyph size attribute for a collection of observations.

 @param which an array of the indices of the observations of interest.
 @param n the length of the array `which', and the length of the array that is returned.
 @param dataset the dataset within the GGobi instance in which to find the observation.
 @param gg the GGobi instance in which to find the dataset.
 @return an array giving the size of the glyph for the specified observations.
   The i-th element corresponds to the observation identified by the i-th element
   of the array `which'.

 @see GGobi_getGlyphTypeName()
 @ingroup Glyphs
 */
extern gint *GGobi_getCaseGlyphSizes(gint *which, gint n, datad *dataset, ggobid *gg);

/**
 Get the glyph size attribute for a single observation

 @param id the index of the observation of interest
 @param dataset the dataset within the GGobi instance in which to find the observation.
 @param gg the GGobi instance in which to find the dataset.
 @return the size of the glyph for the given observation

 @see GGobi_getGlyphTypeName()
 @see GGobi_getGlyphTypeName()
 @ingroup Glyphs
 */
extern gint GGobi_getCaseGlyphSize(gint id, datad *dataset, ggobid *gg);
/**
 @ingroup Glyphs
 */
extern void GGobi_setCaseGlyph(gint pt, gint type, gint size, datad *d, ggobid *gg);
/**
 @ingroup Glyphs
 */
extern void GGobi_setCaseGlyphs(gint *pts, gint n, gint type, gint size, datad *d, ggobid *gg);

/* point colors */

/**
 Get the color attribute of a single observation.
 @param pt the index of the observation of interest.
 @param dataset the dataset  in which to resolve the observation
 @param gg the ggobi instance in which to find the dataset.
 @ingroup Glyphs
 */
extern gint GGobi_getCaseColor(gint pt, datad *dataset, ggobid *gg);
/**
 Query the color attribute of a collection of points.
 @param pts an array giving the indices of the different observations whose color
   attribute is to be queried.
   
 @param howMany the number of entries in the `pts' array.
 @param dataset the dataset  in which to resolve the observation
 @param gg the ggobi instance in which to find the dataset.
 @return an array of the indices into the colormap of the points. The length of this
  array is `howMany' and the i-th element corresponds to observation
  identified by the i-th element of `pts'.
 
 @see GGobi_getCaseColor()
 @see GGobi_setCaseColor()
 @see GGobi_setCaseColors()
 @ingroup Glyphs
 */
extern gint * GGobi_getCaseColors(gint *pts, gint howMany, datad *dataset, ggobid *gg);

/**
 Set the color (for each plot) for a single observation.
 @param pt the index of the record/observation whose color is to be set.
 @param colorIndex the index of the color in the current colormap to which
  the observations color is to be set.
 @param dataset the dataset  in which to resolve the observation
 @param gg the ggobi instance in which to find the dataset.

 @see GGobi_getCaseColor()
 @see GGobi_setCaseColors()
 @ingroup Glyphs
 */
extern void GGobi_setCaseColor(gint pt, gint colorIndex, datad *dataset, ggobid *gg);

/**
 Set the color (for each plot) for a collection of observations
 to the same value.
 @param pt an array of indices of the records/observations whose color attribute are to be set.
 @param n the number of elements in the `pts' array.
 @param colorIndex the index of the color in the current colormap to which
  the observations color is to be set.
 @param dataset the dataset  in which to resolve the observation
 @param gg the ggobi instance in which to find the dataset.

 @see GGobi_setCaseColor()
 @see GGobi_getCaseColor()
 @ingroup Glyphs
 */
extern void GGobi_setCaseColors(gint *pts, gint n, gint color, datad *dataset, ggobid *gg);


/* point hidden state */

/**
 */
extern gboolean GGobi_getCaseHidden(gint pt, datad *, ggobid *gg);

/**
 
 */
extern gboolean * GGobi_getCaseHiddens(gint *pts, gint howMany, datad *, ggobid *gg);

/**
  Specify whether an individual record within the dataset is to be considered 
  hidden or not.
 */
extern void GGobi_setCaseHidden(gint pt, gboolean hidden_p, datad *, ggobid *gg);

/**
 Control whether specific observations in a dataset are considered
 hidden or not for the purpose of displaying them.
 */
extern void GGobi_setCaseHiddens(gint *pts, gint howMany, gboolean hidden_p, datad *, ggobid *gg);



/**
 @defgroup Plots Plot Management

 @brief Manage individual plots within one or more top-level
        displays.

*/

/**
 @ingroup Plots

 Get the index of the currently active plot within the specified
 GGobi instance. This can be used in conjunction with
 @link GGobi_getCurrentDisplay() to get the currently active
 plot.

 @see GGobi_getCurrentDisplay()
 */
extern gint GGobi_getCurrentPlotIndex(ggobid *gg);

/**
 @ingroup Plots

 Get the `which'-th plot within the top-level display.
 @param display
 @param which the index of the plot of interest.
 */
extern splotd *GGobi_getPlot(displayd *display, gint which);

/**
 @ingroup Plots
 Retrieve the plot within a top-level display, identifying
 the plot of interest by number.
 */
extern splotd *GGobi_getSPlot(gint which, displayd *display);


/** 
  @defgroup Brush Brush Region Management
  @brief Programmatically query and set the location and size
    of the brushing region for a ggobi instance.
  @note 
   The dimenions perhaps should be specified in natural coordinates.
   But For now, these are specified as pixels!
 */

/**
 @ingroup Brush
  Relocate the top-left corner of the brushing region (rectangle)
  to the specified pixel.
 */
extern void GGobi_moveBrush(gint ulx, gint uly, ggobid *gg);

/**
 @ingroup Brush
  Resize the brushing region (rectangle)
  for the given plot to have the specified width and height given in pixels.

  @note Is the plot necessary? or does the setting apply to all plots?
 */
extern void GGobi_sizeBrush(gint width, gint height, splotd *, ggobid *gg);

/**
 @ingroup Brush
  Query the size and type of the glyph being used for the brushing
  area.
 */
extern void GGobi_getBrushGlyph(gint *type, gint *size, ggobid *gg);

/** 
 @ingroup Brush
 Set the glyph for the brushing region, providing the glyph type and size.
 */
extern gboolean GGobi_setBrushGlyph(gint type, gint size, ggobid *gg);

/**
 @ingroup Brush
  Query the current size (width and height) of the brushing region.
*/
extern void GGobi_getBrushSize(gint *w, gint *h, ggobid *gg);

/**
 @ingroup Brush
  Set the size (width and height) of the brushing region.
 */
extern void GGobi_setBrushSize(gint w, gint h, ggobid *gg);

/**
 @ingroup Brush
  Query the current location (horizontal and vertical coordinates) of the brushing region.
 */
extern void GGobi_getBrushLocation(gint *x, gint *y, ggobid *gg);


/**
  Within the plots of a ggobi instance, specify the location
  of the top-left corner of the brush region by giving pixel
  locations.

 @note This will be enhanced/changed to allow specification in natural coordinates.
 */
extern void GGobi_setBrushLocation(gint x, gint y, ggobid *gg);

/** 
 @ingroup Brush
 Get the current color (index in the colormap) of the brushing region.
 */
extern gint GGobi_getBrushColor(ggobid *gg);


/** 
 @ingroup Brush
 Set the current color (specifying the index in the colormap) of the brushing region.
 */
extern int GGobi_setBrushColor(gint cid, ggobid *gg);



/**
  @defgroup Color Color Management
  @brief  Interact with the colormap.
 */
/**
 @ingroup Color
  Specify a colormap of Red Green Blue triplets
 @param vals
 @param nr
 @param gg
 */
gboolean GGobi_setColorMap(gdouble *vals, gint nr, ggobid *gg);

/**
 @ingroup Color
 This forces the colormap values stored in the GGobi instance
 to be sent to the desktop server and explicitly allocated
 as entries in the colormap.

 @param gg the GGobi instance in which the potential colormap
  entries are to be found.
 */
gboolean GGobi_registerColorMap(ggobid *gg);

/**
 @ingroup Color
 Get the human readable name associated with a specific color in a GGobi
 instance's colormap.

 @param cid
 @param inDefault
 @param gg
 */
extern const gchar *const GGobi_getColorName(gint cid, ggobid *gg, gboolean inDefault);



/**
  Close down and destroy a GGobi instance, removing all its display windows
  and discarding its data.  If closeWindow is true, the control
  panel is also eliminated. Otherwise, the GGobi instance can be re-used
  and a new dataset(s) loaded.
 */
extern gboolean GGobi_close(ggobid *gg, gboolean closeWindow);



/**
  Return the names (and number) of the user operation modes.
  These include actions such as brushing, identify, etc.
 
  @param n a pointer to an integer which, on return, contains the 
   number of entries in the array that is returned.
  @return an (immutable) array of (immutable) strings  
    containing the descriptions of the mode names.
    The number of elements in the array is returned as the 
    value of the argument.
 */
extern const gchar * const* GGobi_getOpModeNames(int *n);

/**
  Get the symbolic constant associated with the 
  user-interaction operation mode given by name.
  The possible names are available from
  ggobi_getOpModeNames
 */
extern gint GGobi_getModeId(const gchar *name);

/**
 Set the operation mode (e.g. brush, identify, ...) for this GGobi instance.
 @param name one of the entries.
 */

extern gint GGobi_setMode(const gchar *name, ggobid *gg);
/**
 Get the human-readable name of the user interaction mode
 associated with the internal symbolic identifier.

 @see GGobi_getModeId()
 */
extern const gchar *GGobi_getModeName(gint which);

/**
 ? 
 */
extern int GGobi_full_viewmode_set(gint action, ggobid *gg);


/**
  Lower or raise a top-level display window associated with a ggobi
  instance, identifying the window of interest by number.
 */
extern gboolean GGobi_raiseWindow(gint which, gboolean raiseOrIcon, gboolean up, ggobid *gg);


/**
 Get the description string associated with the specific
 ggobid instance. This is usually related to the source of the
 data or how the ggobid was created by a host application.
 */
extern gchar *GGobi_getDescription(ggobid *gg);
extern void GGobi_splot_set_current_full(displayd *display, splotd *sp, ggobid *gg);

/**
  Find the index of the dataset in the particular ggobid instance
  by matching the name of the dataset to the given name.
 */
extern int GGobi_datasetIndex(const char *name,  const ggobid * const gg);

/**
 Get a particular ggobid instance from the global collection
 of ggobids in this session, identifying the ggobid of interest
 by index. 
 */
extern ggobid * GGobi_ggobi_get(gint);

/**
 Return the total number of ggobi instances that are currently
 in existence in this session.
 @see GGobi_ggobi_get()
 */
extern int GGobi_getNumGGobis();


/**
  @defgroup Data Dataset Accessors
  @brief Get dataset references and information about
    the contents of the dataset.
 */


/**
  @ingroup Data
 Get the dataset within the GGobi instance using the index to identify
 the dataset.

 @return a reference to the datad object that is accessed by the 
 ggobid instance. This is not a copy.
 @see GGobi_data_get_by_name()
 */
extern datad *GGobi_data_get(int which, const ggobid * const gg);

/**
  @ingroup Data
 Get the dataset within the GGobi instance using the name of the
 dataset to identify it.

 @return a reference to the datad object that is accessed by the 
 ggobid instance. This is not a copy.
 */
extern datad *GGobi_data_get_by_name(const gchar * const name, const ggobid * const gg);

/**
  @ingroup Data
 Determine the number of variables in the dataset.
 */
gint GGobi_ncols(datad *d);

/**
  @ingroup Data
 Determine the number of records in the dataset.
 */
gint GGobi_nrecords(datad *dg);

/**
  @ingroup Data
 */
extern int GGobi_getVariableIndex(const gchar *name, datad *, ggobid *gg);
/**
  @ingroup Data
 */
extern int GGobi_removeVariableByIndex(gint which, datad *, ggobid *gg);

/**
  @ingroup Data
 */
extern gboolean GGobi_setVariableValues(gint whichVar, gdouble *vals, gint num, gboolean update, datad *d, ggobid *gg);


    /* Need len just in case there is no data in the instance */
/**
  @ingroup Data
  Add a variable to the specified dataset, and optionally update the different
 displays, plots and control panel. 
  The ability to delay the update allows add several variables and wait until
  the last one to update, rather than updating after each addition.

  @param vals the values for the new variable
  @param len  the number of records or values for this variable.
  This should be the same as the number of records for the other
  variables in the dataset. This is a necessary argument in the case
  where the dataset is currently empty and this is the first variable to be
  added.
  @param name the name to use for the variable in the control panel, plots, etc.
   and  when referring to it in other calls to manipulate the GGobi instance.
  @param update  a logical value indicating whether to update the control panel
   and any other plots.
  @param d  the dataset into which the variable is to be added.
  @param gg  the GGobi instance in which the dataset to be augmented can be found.
 */
extern int GGobi_addVariable(gdouble *vals, gint len, gchar *name, gboolean update, datad *d, ggobid *gg);

extern int GGobi_addCategoricalVariable(gdouble *vals, gint len, gchar *name, 
                                        gchar **levels, gint *values, gint *counts, gint numLevels, 
                                        gboolean update, datad *d, ggobid *gg);

/**
  @ingroup Data
  Recompute the derived transformations for the specified dataset, updating plots
  if necessary. This is used to force changes in the raw data to be reflected
  in the different plots.
 */
extern void GGobi_update_data(datad *, ggobid *gg);


/* @} */

/**
 Get a list of the human readable names of the different 
 input modes for data  sources. These include
 ASCII, XML, database, URL, etc.
 */
const gchar *const * GGobi_getDataModeNames(int *n);


/** 
 @defgroup EventHandlers Event handlers 
 @brief Customizable Event Handlers
  for numbered key presses and record identify mode.
*/

/* @{ */

/** @ingroup EventHandlers
  register a handler routine which is to be called when one of the
  number keys is pressed.
*/
extern KeyEventHandler *GGobi_registerNumberedKeyEventHandler(KeyEventHandlerFunc routine, void *userData, char *description, ReleaseData *data, ggobid *gg, ProgrammingLanguage lang);

/**
  @ingroup EventHandlers
  Remove a number key handler routine.
 */
extern KeyEventHandler *GGobi_removeNumberedKeyEventHandler(ggobid *gg);

/**
  @ingroup EventHandlers
  Register a handler function which is to be invoked
 while GGobi is in "identify" mode and the pointer (mouse) 
 is close to a point.
 
  @param proc a C routine which is to be invoked when a GGobi identify
  event occurs.
  @param data arbitrary object/value  that is passed to the  
   routine `proc' when it is called. This allows one to parameterize
   the routine to behave differently for different calls.
  @param gg the ggobi instance in which the identify events are to be
 handled.
 */
extern void GGobi_setIdentifyHandler(IdentifyProc proc,  void *data, ggobid *gg);

/* @} */


/**
 @defgroup Edges Edge management.
 @brief Routines to set and query the connections or edges between
  records.

 */
#ifdef OBSOLETE_EDGE_CODE
/**
 @ingroup Edges
  Query whether the observations/records identified by number
  in the given dataset are connected by an edge.
 */
extern gboolean  GGobi_isConnectedEdge(gint a, gint b, datad *d, ggobid *gg);

/**
  @ingroup Edges
 */
extern void GGobi_setObservationEdge(gint x, gint y, datad *, ggobid *, gboolean update);
#endif

/**
  @ingroup Edges
  Determine whether we the default option is to show lines
  connecting points on a plot.

  @note This probably needs a ggobid or displayd argument.
 */
extern gboolean GGobi_getShowLines();

/**
  @ingroup Edges
 */
extern gboolean GGobi_setShowLines(displayd *dsp, gboolean val);

/** @} */



/**
 @group Plugins

 */

extern GtkWidget *GGobi_addDisplayMenuItem(const char *label, ggobid *gg);


/**
 @group Creation
 */
ggobid* ggobi_alloc(ggobid *tmp);

/**
 */
ggobid *create_ggobi(InputDescription *desc);

/**
 */
GGOBI_EXPORT datad* datad_create(int nr, int nc, ggobid *gg);


void GGobi_setSessionOptions(GGobiOptions *opts);



/**

 */

const gchar *GGobi_getLevelName(vartabled *vt, double value);



colorschemed *alloc_colorscheme();

/**
  Compute and update the bi-directional link information for the collection
  of edges. This simply fixes up the internal data structure in `edge' given
  the basic edge connections by pairs of records.
 */
void GGobi_cleanUpEdgeRelationships(struct _EdgeData *edge, int startPosition);

typedef int (*MissingValue_p)(double);
MissingValue_p GGobi_setMissingValueIdentifier(MissingValue_p f);
extern MissingValue_p GGobiMissingValue;

#ifdef __cplusplus
}
#endif




#endif /* End of conditional inclusion of entire file.*/

