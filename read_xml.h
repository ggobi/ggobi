#ifndef READ_XML_H
#define READ_XML_H

#include "vars.h"

/*#include <libxml/parser.h>*/
#include <parser.h>

enum HiddenType {ROW, LINE};
enum xmlDataState { TOP = 0, DESCRIPTION,
  RECORD, RECORDS, VARIABLES, VARIABLE, DATA,
  CONNECTIONS, CONNECTION,  /* edges */
  EDGERECORD, EDGERECORDS, EDGEVARIABLES, EDGEVARIABLE,
  COLORMAP, COLOR, UNKNOWN};


typedef struct {

  int color;
  int glyphType;
  int glyphSize;
  int lineColor;
  int lineWidth;
  int lineHidden;
  int hidden;

} DataOptions;

typedef struct _XMLUserData {
  enum xmlDataState state;
  gint current_variable; /* Indexes the current variable. */
  gint current_record;   /* Indexes the record we are currently working on. */
  gint current_element;  /* Indexes the values within a record. */
/* this line will be deleted */
  gint current_edge;  /* Current edge being added. */
/* */

  gint current_edgevariable;
  gint current_edgerecord;
  gint current_edgeelement;


  gint current_color;    /* The index of the current element being processed in the colormap */
 
  /* Flag that says we are reading color entries from another file via a
   * sub-parser.  This allows us to reuse the same instance of user data
   * and the same handlers.
   */
  gboolean reading_colormap_file_p;

  /* A boolean indicating whether the transformation name of a variable
     was stored as an attribute.
   */
  gboolean variable_transform_name_as_attribute;

  /* The ggobi instance that is being initialized. */
  ggobid *gg;

  /* Flag indicating whether we should convert
     char arrays into null-terminated strings
     before passing them to the sub-handlers
     (e.g. setColorValue, setVariableName).
   */
  gboolean terminateStrings_p;

   /* The datasets global missing value identifier. */
  gchar *NA_identifier;
  /* The identifier for a missing value that is currently in effect.
     This might be specified per record and will be discarded
     at the end of that record.
     We could also do this columnwise.
   */
  gchar *current_NA_identifier;

    /* A set of values that apply to records when an attribute
       is not specified for that specific record but is set
       in the ggobidata tag.
     */
  DataOptions defaults;

  /* Local set of record identifiers that are used here
     for matching purposes when specifying edges.
     These are not set in the ggobid structure and 
     are different from the record's label attribute.
   */
  gchar **rowIds;

  /* Reference to the handlers being used as callbacks.
     Need this so that we can re-specify it when creating
     new sub-parsers.
   */
  xmlSAXHandlerPtr handlers;

} XMLParserData;


#ifdef __cplusplus
extern "C" {
#endif

enum xmlDataState tagType(const gchar *name, gboolean endTag);
gboolean newVariable(const CHAR **attrs, XMLParserData *data);
gboolean newEdgeVariable(const CHAR **attrs, XMLParserData *data);
gboolean setDatasetInfo(const CHAR **attrs, XMLParserData *data);
gboolean allocVariables(const CHAR **attrs, XMLParserData *data);
gboolean allocEdgeVariables(const CHAR **attrs, XMLParserData *data);
gboolean newRecord(const CHAR **attrs, XMLParserData *data);
gboolean newEdgeRecord(const CHAR **attrs, XMLParserData *data);

gboolean setRecordValues(XMLParserData *data, const CHAR *line, gint len);
gboolean setEdgeRecordValues(XMLParserData *data, const CHAR *line, gint len);
gboolean setVariableName(XMLParserData *data, const CHAR *name, gint len);
gboolean setEdgeVariableName(XMLParserData *data, const CHAR *name, gint len);

const char *skipWhiteSpace(const CHAR *ch, gint *len);

const gchar *getAttribute(const CHAR **attrs, gchar *name);


void xml_warning(const gchar *attribute, const gchar *value, const gchar *msg, XMLParserData *data);

void initParserData(XMLParserData *data, xmlSAXHandlerPtr handler, ggobid *gg);

gboolean setGlyph(const CHAR **attrs, XMLParserData *data, gint i);
gboolean setColor(const CHAR **attrs, XMLParserData *data, gint i);

gboolean allocEdges(const CHAR **attrs, XMLParserData *data);
gboolean addEdge(const CHAR **attrs, XMLParserData *data);
gint rowId(const gchar *tmp, XMLParserData *data);

gboolean data_xml_read (const gchar *filename, ggobid *gg);

gboolean setHidden(const CHAR **attrs, XMLParserData *data, gint i, enum HiddenType);

gboolean setColorValue(XMLParserData *data, const CHAR *name, gint len);
gboolean setColormapEntry(const CHAR **attrs, XMLParserData *data);
gboolean setColorMap(const CHAR **attrs, XMLParserData *data);
void setColorValues(GdkColor *color, double *values);

gboolean registerColorMap(ggobid *gg);

gboolean xmlParseColorMap(const gchar *fileName, int size, XMLParserData *data);
gboolean asciiParseColorMap(const gchar *fileName, int size, XMLParserData *data);

gchar *find_xml_file(const gchar *filename, const gchar *dir, ggobid *gg);
gchar *getFileDirectory(const gchar *filename);



int asInteger(const gchar *tmp);
double asNumber(const char *sval);
gboolean asLogical(const gchar *sval);

#ifdef __cplusplus
}
#endif

#endif
