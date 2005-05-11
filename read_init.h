/* read_init.h */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef GGOBI_READ_INIT_H
#define GGOBI_READ_INIT_H

#include "ggobi.h"
#include "defines.h"
#include <gtk/gtk.h>
#include "fileio.h"


#ifdef XML_USE_CHILDS
# define XML_CHILDREN(node) (node)->childs
#else
# define XML_CHILDREN(node) (node)->children
#endif

typedef struct {
    gchar *typeName;     /* use for the extended type. */
    gint numVars;           /* the number of variables in the plot. */
    gchar **varNames;       /* the variables in the plot. */
    gchar *datasetName;     /* name of the dataset in which to find the variables. */
    gint data;              /* with which datad is this associated */
    gboolean canRecreate; /* see the unsupported tag in the output from write_state.*/
} GGobiDisplayDescription;

typedef struct {
  InputDescription input;
  GList *displays;
} GGobiDescription;

typedef struct _GGobiInitInfo {
    gint numInputs;               /* number of previously read input sources */
    GGobiDescription *descriptions;

#if 0
    InputDescription *inputs;    /* previously read input sources. */
#endif

    GList *plugins;  /* list of known available plugins */
    GList *inputPlugins;  /* list of the input reading plugins */

    gchar *filename; /* the name of the file from which this information was read. */

    gchar *colorSchemeFile; /* */
    GdkColor *bgColor;
    GdkColor *fgColor;
    glyphd    glyph;

    gboolean createInitialScatterPlot;
    gboolean allowCloseLastDisplay;
    gboolean quitWithNoGGobi;
    gint     numScatMatrixVars;
    gint     numParCoordsVars;
    gint     numTimePlotVars;

    gchar   *sessionFile;
    gint     compress;
} GGobiInitInfo;

GGobiInitInfo *read_init_file(const gchar *filename, GGobiInitInfo *info);
xmlNode *getXMLDocElement(const xmlDocPtr doc, const gchar *tagName);
xmlNode *getXMLElement(const xmlNodePtr doc, const gchar *tagName);

#ifdef __cplusplus
extern "C" {
#endif
gint getPreviousDisplays(xmlNodePtr node, GGobiDescription *desc);
#ifdef __cplusplus
}
#endif

#endif /* end of GGOBI_READ_INIT_H */
