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
    int numVars;            /* the number of variables in the plot. */
    gchar **varNames;       /* the variables in the plot. */
    int data;               /* with which datad is this associated */
} GGobiDisplayDescription;

typedef struct {
  InputDescription input;
  GList *displays;
} GGobiDescription;

typedef struct _GGobiInitInfo {
    int numInputs;               /* number of previously read input sources */
    GGobiDescription *descriptions;

#if 0
    InputDescription *inputs;    /* previously read input sources. */
#endif

    GList *plugins;  /* list of known available plugins */
    GList *inputPlugins;  /* list of the input reading plugins */

    char *filename; /* the name of the file from which this information was read. */

    char *colorSchemeFile; /* */
    GdkColor *bgColor;
    GdkColor *fgColor;
    glyphd    glyph;
} GGobiInitInfo;

GGobiInitInfo *read_init_file(const char *filename, GGobiInitInfo *info);
#ifdef USE_XML
xmlNode *getXMLDocElement(const xmlDocPtr doc, const char *tagName);
xmlNode *getXMLElement(const xmlNodePtr doc, const char *tagName);
#endif

#endif /* end of GGOBI_READ_INIT_H */
