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
    enum displaytyped type; /* the type of the plot. */
    gchar *typeName;     /* use for the extended type. */
    gint numVars;           /* the number of variables in the plot. */
    gchar **varNames;       /* the variables in the plot. */
    gint data;              /* with which datad is this associated */
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

#endif /* end of GGOBI_READ_INIT_H */
