#ifndef READ_XML_H
#define READ_XML_H

#include <gtk/gtk.h>

#include "ggobi.h"

#include <libxml/parser.h>

enum xmlDataState { TOP = 0, DESCRIPTION, RECORD, RECORDS, VARIABLES, VARIABLE, DATA, CONNECTIONS, CONNECTION, UNKNOWN};


typedef struct {

  int color;
  int glyphType;
  int glyphSize;
  int lineColor;
  int lineWidth;

} DataOptions;

typedef struct _XMLUserData {
  enum xmlDataState state;
  int current_variable; /* Indexes the current variable. */
  int current_record;   /* Indexes the record we are currently working on. */
  int current_element;  /* Indexes the values within a record. */
  int current_segment;  /* Current segment being added. */


  /* A boolean indicating whether the transformation name of a variable
     was stored as an attribute.
   */
  gboolean variable_transform_name_as_attribute;

  ggobid *gg;

  gboolean terminateStrings;

  gchar *NA_identifier;

  DataOptions defaults;

  gchar **rowIds;

} XMLParserData;



enum xmlDataState tagType(const gchar *name, gboolean endTag);
gboolean newVariable(const CHAR **attrs, XMLParserData *data);
gboolean setDatasetInfo(const CHAR **attrs, XMLParserData *data);
gboolean allocVariables(const CHAR **attrs, XMLParserData *data);
gboolean  newRecord(const CHAR **attrs, XMLParserData *data);

gboolean setRecordValues(XMLParserData *data, const CHAR *line, int len);
gboolean setVariableName(XMLParserData *data, const CHAR *name, int len);

const char *skipWhiteSpace(const CHAR *ch, int *len);

const gchar *getAttribute(const CHAR **attrs, char *name);
int asInteger(const gchar *tmp);
double asNumber(const char *sval);

void xml_warning(const char *attribute, const char *value, const char *msg, XMLParserData *data);

void initParserData(XMLParserData *data, ggobid *gg);

gboolean setGlyph(const CHAR **attrs, XMLParserData *data, int i);
gboolean setColor(const CHAR **attrs, XMLParserData *data, int i);

gboolean allocSegments(const CHAR **attrs, XMLParserData *data);
gboolean addConnection(const CHAR **attrs, XMLParserData *data);
int rowId(const char *tmp, XMLParserData *data);

gboolean data_xml_read(const gchar *filename, ggobid *gg);

#endif
