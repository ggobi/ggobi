/*#include <libxml/parserInternals.h>*/
#include <parserInternals.h>

/*
   This is a SAX based parser for reading a single input file
   formatted in XML for the DTD specified in ggobi.dtd.

   SAX is an event based parser. As different elements of the document
   are encountered by the parser on its input stream, it invokes
   user-specified callbacks. For our purposes, we currently need only
   handle 3 event types:
      1) the start of a tag
      2) the end of a tag  
      3) regular text data within a tag

   As with all event driven approaches, we can specify a user-level
   data object that can is passed to each event handler/callback.
   This is used to parameterize the particular call. In our case,
   we pass an XMLParserData * object (defined in read_xml.h).
   This basically stores what state in the document we currently have
   and indicates how to interpret the future callbacks, and also
   the ggobid structure which we are attempting to fill in.
   Additionally, it maintains a list of default settings that are 
   to be applied to each row, but which can be set generically in the
   top-level element (ggobidata).
  

   When we encounter the start of tag, we are given the tag name, the
   list of attributes for the tag instance and a reference to the
   parser instance XMLParserData object we specified at the start of the
   parsing. This handler is startXMLElement.  Here, we dispatch
   a call to the appropriate method based on the tag name. Generally,
   this either allocates storage space in the ggobid structure being
   filled in, or stores partial information needed to do this.
   Additionally, for the tag ggobidata, the attributes are processed
   to store the default values for record attributes. These are
   attributes such as color, glyph (size and type). 

   As each tag is encountered, we convert its type to an enumerated
   type and store this as the pending state. This is used to determine
   how to interpret ASCII text within an element.

   When we encounter the end of a tag/element,  the routine
   endXMLElement is called. We use this to increment counters indicating
   which record, variable or segment is next in the stream. These are 
   quasi-global variables that are parser-instance specific and a
   necessary consquence of the event-driven style of parsing.

   Finally, when ASCII text within an XML element is discovered by the
   parser, the routine characters() is called. We call different
   routines (setRecordsValues() and setVariableName()) based on the
   state of the parsing. This was set at the start of each tag/element
   in startXMLElement. 
 
 */

#include <stdlib.h>
#include <string.h>

#include "read_xml.h"

#include "ggobi.h"
#include "externs.h"

#include "GGobiAPI.h"


void startXMLElement(void *user_data, const CHAR *name, const CHAR **attrs);
void endXMLElement(void *user_data, const CHAR *name);
void Characters(void *user_data, const CHAR *ch, int len);


const gchar * const xmlDataTagNames[] = {
                                          "ggobidata",
                                          "description",
                                          "record",
                                          "records",
                                          "variables",
                                          "variable",
                                          "data",
/*-- these two lines will be deleted --*/
                                          "segments",
                                          "segment",
/* */

                                          "edgevariables",
                                          "edgevariable",
                                          "edgerecord",
                                          "edgerecords",

                                          "colormap",
                                          "color",
                                          ""
                                         };



/*
  We also need a version that takes a FILE*
  and reads from it. This is so that we can
  handle reading from standard input.

  The DOM style parsing can be initiated very simply.

  xmlDocPtr doc;
    doc = xmlParseFile(name);
 */


gboolean
data_xml_read(const gchar *filename, ggobid *gg)
{
 xmlSAXHandlerPtr xmlParserHandler;
 xmlParserCtxtPtr ctx = (xmlParserCtxtPtr) g_malloc(sizeof(xmlParserCtxtPtr));
 XMLParserData data;
 gboolean ok = false;  
 gchar *name = find_xml_file(filename, NULL, gg);
 
  if(name == NULL)
    return(false);

  gg->filename = name;



  xmlParserHandler = (xmlSAXHandlerPtr) g_malloc(sizeof(xmlSAXHandler));
  /* Make certain this is initialized so that we don't have any references
     to unwanted routines!
   */
  memset(xmlParserHandler, '\0', sizeof(xmlSAXHandler));

  xmlParserHandler->startElement = startXMLElement;
  xmlParserHandler->endElement = endXMLElement;
  xmlParserHandler->characters = Characters;


  initParserData(&data, xmlParserHandler, gg);


  ctx = xmlCreateFileParserCtxt(name);
  if(ctx == NULL) {
   xml_warning("File error:", name, "Can't open file ", &data);
   return(false);
  }

  ctx->userData = &data;
  ctx->sax = xmlParserHandler;

  xmlParseDocument(ctx);

  ctx->sax = NULL;
  xmlFreeParserCtxt(ctx);

  g_free(xmlParserHandler);


  ok = (gg->ncols > 0 && gg->nrows >0);

  /* Now perform the necessary computations to bring the entire
     ggobid structure into synch with the newly read data.
     This should be moved somewhere more reasonable 
   */
  vgroups_sort(gg);
  { gint j;
    for (j=0; j<gg->ncols; j++)
      gg->vardata[j].groupid = gg->vardata[j].groupid_ori;
  }

/*-- these lines will be deleted --- come to think of it, why default edges? */
/*
 *if (gg->nsegments < 1)
 * segments_create(gg);
*/

  return (ok);
}

void
initParserData(XMLParserData *data, xmlSAXHandlerPtr handler, ggobid *gg)
{
  data->gg=gg;
  data->current_record = 0;
  data->current_variable = 0;
  data->current_element = 0;
/* this line will be deleted */
  data->current_segment = 0;
/* */

  data->current_edgevariable = 0;
  data->current_edgerecord = 0;
  data->current_edgeelement = 0;

  data->current_color = 0;
  data->reading_colormap_file_p = false;
  data->state = UNKNOWN;
  data->terminateStrings_p = true;
  data->NA_identifier = NULL;
  data->rowIds = NULL;
  data->handlers = handler;
  data->defaults.color = -1;
  data->defaults.glyphType = -1;
  data->defaults.glyphSize = -1;
  data->defaults.lineWidth = -1;
  data->defaults.lineColor = -1;
  data->defaults.lineHidden = false;
  data->defaults.hidden = false;
}

void 
startXMLElement(void *user_data, const CHAR *name, const CHAR **attrs)
{
 XMLParserData *data = (XMLParserData*)user_data;
 enum xmlDataState type = tagType(name, false);

 data->state = type;

 switch(type) {
   case VARIABLES:
     allocVariables (attrs, data);
     break;
   case VARIABLE:
     newVariable (attrs, data);
     break;
   case TOP:
     setDatasetInfo(attrs, data);
     break;
   case RECORD:
     newRecord(attrs, data);
     break;
/*-- these lines will be deleted --*/
   case CONNECTIONS:
     allocSegments(attrs, data);
     break;
   case CONNECTION:
     addConnection(attrs, data);
     break;
/* */

/* populate */
   case EDGERECORDS:
/*     setDatasetEdgeInfo (attrs, data);*/
     break;
   case EDGERECORD:
/*     newEdgeRecord (attrs, data);*/
     break;
   case EDGEVARIABLES:
     allocEdgeVariables (attrs, data);
     break;
   case EDGEVARIABLE:
     newEdgeVariable (attrs, data);
     break;

   case COLORMAP:
     setColorMap(attrs, data);
     break;
   case COLOR:
     setColormapEntry(attrs, data);
     break;
   default:
     break;
 }
}


void 
endXMLElement(void *user_data, const CHAR *name)
{
 XMLParserData *data = (XMLParserData*)user_data;
 enum xmlDataState type = tagType(name, true);

 switch(type) {
   case RECORD:
     data->current_record++;
     break;
   case VARIABLE:
     data->current_variable++;
     break;
/* these lines will be deleted */
   case CONNECTION:
     data->current_segment++;
     break;
/* */

/* populate */
   case EDGERECORD:
     break;
   case EDGEVARIABLE:
     break;

   case COLOR:
     data->current_color++;
     break;
   case COLORMAP:
       /* Only set this if we are reading from the main file 
          and not a secondary colormap file.
        */
     if(data->reading_colormap_file_p == false)
       GGOBI(registerColorMap)(data->gg);
   default:
     break;
 }
}


enum xmlDataState
tagType(const gchar *name, gboolean endTag)
{
 int n = sizeof(xmlDataTagNames)/sizeof(xmlDataTagNames)[0] - 1; 
 int i;
 const gchar *tmp = name;
 /*
  if(endTag) {
   tmp++;
  }
 */

  for(i = 0; i < n; i++) {
    if(strcmp(tmp, xmlDataTagNames[i]) == 0) {
     return(i);
    }
  }

 return(UNKNOWN);
}


/*
  Called when data within an element is found by the parser.
  Note that the parser does not have to give us all the data
  in one go (although it usually does). Instead,
  it can make several calls to this routine for the same
  element instances. E.g 
    <text>
      a few words
     and another sentence
    </text>
   may result in two calls - one for each sentence.

   This is important to handle as it means that we must be prepared
   to read values within a record across different calls and remember
   which variable/column we last completed for this record.
   This is the current_element field in XMLParserData. 
   Additionally, if a variable name is split across multiple calls
   we must append subsequent calls to the initial value.
  

   Additionally, new lines and leading and trailing white space 
   are not removed. Hence, we must do this ourselves. We use
   skipWhiteSpace to move to first non-whitespace character in the
   string.

   Also, the text is given to us as a a sequence of bytes rather than
   a non-terminated string. Thus, we are told the number of bytes
   rather than being able to use strlen to compute the length of the
   string.
   If the flag terminateStrings is set in the XMLParserData instance,
   this routine takes care of copying the data into a regularly
   NULL-terminated string so that the routines to which the data
   is passed can work with it more easily.
 */
void 
Characters(void *user_data, const CHAR *ch, int len)
{
 char *tmp;
 int dlen = len;
 const CHAR *c;
 XMLParserData *data = (XMLParserData*)user_data;

 c = skipWhiteSpace(ch, &dlen);
 if(dlen < 1 || c[0] == '\n')
  return;

 if(data->terminateStrings_p) {
  tmp = g_malloc(sizeof(char)*(dlen+1));

  memcpy(tmp, c, dlen);
  memset(tmp+dlen, '\0', 1);

  c = tmp;
 }


 switch(data->state) {
   case RECORD:
     setRecordValues (data, c, dlen);
   break;
   case VARIABLE:
     setVariableName (data, c, dlen);
   break;

/* populate */
   case EDGERECORD:
     setEdgeRecordValues (data, c, dlen);
   break;
   case EDGEVARIABLE:
     setEdgeVariableName (data, c, dlen);
   break;
/* */

   case COLOR:
     setColorValue (data, c, dlen);
   default:
     break;

 }

 if(data->terminateStrings_p) {
  g_free(tmp);
 }
}

const char *
skipWhiteSpace(const CHAR *ch, int *len)
{
 const CHAR *tmp = ch;
  while(*len >= 0) {
   if(*len == 0 || (tmp[0] != ' ' && tmp[0] != '\t' && tmp[0] != '\n'))
    break;
   tmp++;
   (*len)--;
  }

return(tmp);
}


gboolean
setDatasetInfo (const CHAR **attrs, XMLParserData *data)
{
  const char *tmp = getAttribute(attrs, "numRecords");

  if (tmp == NULL) {
    g_printerr ("No numRecords attribute\n");
    exit(101);
  }

  data->gg->nrows = asInteger(tmp);
  data->gg->nrows_in_plot = data->gg->nrows;  /*-- for now --*/
  data->gg->nrgroups = 0;              /*-- for now --*/

  rowlabels_alloc (data->gg);
  br_glyph_ids_alloc (data->gg);
  br_glyph_ids_init (data->gg);

  br_color_ids_alloc (data->gg);
  br_color_ids_init (data->gg);

  tmp = getAttribute(attrs, "missingValue");
  if(tmp != NULL) {
    data->NA_identifier = g_strdup(tmp);
  }

  setGlyph (attrs, data, -1);  
  setColor (attrs, data, -1);
  setHidden (attrs, data, -1, ROW);
  /*
  setLineColor (attrs, data, -1);
  setLineWidth (attrs, data, -1);  
  */

  return (true);
}

int
asInteger(const gchar *tmp)
{
 int value;

  value = atoi(tmp);

 return(value);
}


const gchar *
getAttribute(const CHAR **attrs, char *name)
{
 const CHAR **tmp = attrs;
 while(tmp && tmp[0]) {
  if(strcmp(name, tmp[0]) == 0)
      return(tmp[1]);
   tmp += 2;
 }

 return(NULL);
}

gboolean 
newRecord(const CHAR **attrs, XMLParserData *data)
{
 const char *tmp;
 int i = data->current_record;
  data->current_element = 0;

  tmp = getAttribute(attrs, "label");
  if (tmp) {
    gchar *stmp = g_strdup (tmp);
    g_array_insert_val (data->gg->rowlab, data->current_record, stmp);
/*    g_free (stmp);*/
/*  data->gg->rowlab[data->current_record] = g_strdup(tmp);*/
  }


  setColor(attrs, data, i);
  setGlyph(attrs, data, i);
  setHidden(attrs, data, i, LINE);
 
  tmp = getAttribute(attrs, "id");
  if(tmp) {
    if(data->rowIds == NULL) {
     data->rowIds = (gchar **) g_malloc(data->gg->nrows * sizeof(gchar *));
     memset(data->rowIds, '\0', data->gg->nrows);
    }

    data->rowIds[i] = g_strdup(tmp);
  }


 return(true);
}

gboolean
setHidden(const CHAR **attrs, XMLParserData *data, int i, enum HiddenType type)
{
 const char *tmp;
  tmp = getAttribute(attrs, "hidden");
  if(tmp) {
    gboolean hidden = asLogical(tmp);


    if(i < 0) {
     if(type == RECORD)
       data->defaults.hidden = hidden;
     else {
       data->defaults.lineHidden = hidden;
     }     
    } else
     if(type == RECORD)
       data->gg->hidden[i] =
         data->gg->hidden_now[i] =
         data->gg->hidden_prev[i] = hidden;
     else {
       data->gg->line.hidden.vals[i] =
         data->gg->line.hidden_now.vals[i] =
         data->gg->line.hidden_prev.vals[i] = hidden;
     }
  }

 return(tmp != NULL);
}

gboolean
asLogical(const gchar *sval)
{
 int i;
 gboolean val = false;
 const gchar *const trues[] = {"T","true", "True","1"};
  for(i = 0; i < sizeof(trues)/sizeof(trues[0]); i++) {
    if(strcmp(sval, trues[i]) == 0)
      return(true);
  }

 return(val);
}

gboolean
setColor(const CHAR **attrs, XMLParserData *data, int i)
{
 const char *tmp;
 int value = data->defaults.color;
  tmp = getAttribute(attrs, "color");
  if(tmp) {
    value = asInteger(tmp);
  }

  if(value < 0 || value > NCOLORS) {
    if(tmp)
      xml_warning("color", tmp, "Out of range", data);
  } else {
    if(i < 0)
     data->defaults.color = value;
    else 
     data->gg->color_ids[i] =
       data->gg->color_now[i] =
       data->gg->color_prev[i] = value;    
  }

 return(value != -1);
}

gboolean
setGlyph(const CHAR **attrs, XMLParserData *data, int i)
{
  const char *tmp;
  int value;

  value = data->defaults.glyphSize;
  tmp = getAttribute(attrs, "glyphSize");
  if (tmp) {
   value = asInteger(tmp);
  }

  if (value < 0 || value > NGLYPHSIZES) {
    if (tmp)
      xml_warning ("glyphSize", tmp, "Out of range", data);
  } else {
   if (i < 0)
     data->defaults.glyphSize = value;
   else
     data->gg->glyph_ids[i].size = data->gg->glyph_now[i].size 
             = data->gg->glyph_prev[i].size = value;
  }


  value = data->defaults.glyphType;
  tmp = getAttribute(attrs, "glyphType");
  if(tmp) {
   value = asInteger(tmp);
  }

  if(value < 0 || value > NGLYPHS) {
    if(tmp)
      xml_warning("glyphType", tmp, "Out of range", data);
  } else {
    if(i < 0)
      data->defaults.glyphSize = value;
    else
     data->gg->glyph_ids[i].type = data->gg->glyph_now[i].type = 
           data->gg->glyph_prev[i].type = value;
  }


  tmp = getAttribute(attrs, "glyph");
  if(tmp != NULL) {
    const char *next;
    int j;
    next = tmp;
    next = strtok((char *)tmp, " ");
    j = 0;
    while(next) {
      if(j == 0) {
         value = mapGlyphName(next);
          if(i < 0)
            data->defaults.glyphType = value;
          else
            data->gg->glyph_ids[i].type = data->gg->glyph_now[i].type = data->gg->glyph_prev[i].type = value;       
      } else {
        value = atoi(next);
        if(i < 0)
            data->defaults.glyphSize = value;
        else
            data->gg->glyph_ids[i].size = data->gg->glyph_now[i].size = data->gg->glyph_prev[i].size = value;     
      }
     j++;
     next = strtok(NULL, " ");
    }

  }

 return(value != -1);
}


void
xml_warning(const char *attribute, const char *value, const char *msg, XMLParserData *data)
{
  g_printerr ("Incorrect data (record %d)\n",  data->current_record);
  g_printerr ("\t%s %s: value = %s\n",  attribute, msg, value);
}


/*
  Read the values for this record from free-formatted text. The entries
  are white-space delimited. They should not have quotes or anything
  that needs to be escaped.
*/

gboolean
setRecordValues (XMLParserData *data, const CHAR *line, gint len)
{
  gdouble value;
  const gchar *tmp = strtok((gchar*) line, " \t\n");

  while (tmp) {
    value = asNumber (tmp);
    data->gg->raw.vals[data->current_record][data->current_element++] = value;
    tmp = strtok (NULL, " \t\n");
  }

  return (true);
}

/*
  Convert the specified string to a numeric value.
 */
double
asNumber(const char *sval)
{
  return(atof(sval));
}


/*
   Read the declaration of a variable, gathering its information
   from the specified attributes.
   This includes its name, transformation name, etc.

    Called in response to a <variable> tag.
 */

gboolean
newVariable(const CHAR **attrs, XMLParserData *data)
{
 int groupId = data->current_variable;
 const char *tmp;

  tmp = getAttribute(attrs, "transformName");
  if(tmp) {
    data->variable_transform_name_as_attribute = true;

    data->gg->vardata[data->current_variable].collab_tform =  g_strdup(tmp);
  }

 tmp = getAttribute(attrs, "name");
 if(tmp != NULL) {
  data->gg->vardata[data->current_variable].collab = g_strdup(tmp);
  if(data->variable_transform_name_as_attribute == false)
    data->gg->vardata[data->current_variable].collab_tform = g_strdup(tmp);
 }

  tmp = getAttribute(attrs, "group");
  if(tmp) {
    groupId = asInteger(tmp);
      /* Do we need to subtract 1 from this. */
  }

  data->gg->vardata[data->current_variable].groupid_ori = groupId;

 return(true);
}


/*
   Reads the number of variables in the dataset from the attributes
   and allocates space for them in the ggobid structure.
   At this point, we have the number of records and variables
   and can initialize the data areas of the ggobid structure.

    Called in response to a <variables> tag. (Note the plural.)
 */
gboolean 
allocVariables (const CHAR **attrs, XMLParserData *data)
{
 const char *tmp = getAttribute (attrs, "count");

 if(tmp == NULL) {
    g_printerr ("No count for variables attribute\n");
    exit(101);
 }

 data->gg->ncols = asInteger(tmp);

 arrayf_alloc(&data->gg->raw, data->gg->nrows, data->gg->ncols);

 vardata_alloc(data->gg);
 vardata_init(data->gg);

 hidden_alloc(data->gg);

 return(true);
}


/*
  Reads the text in name and assigns it as the name of the
  variable currently being read within the 
  <variable> tag. The index for the variable is stored in 
  data->current_variable.

   Called when parsing free-formatted text within a <variable> tag.
 */
gboolean
setVariableName(XMLParserData *data, const CHAR *name, gint len)
{
  gchar *tmp = (gchar *) g_malloc (sizeof(gchar) * (len+1));
  gint j = data->current_edgevariable;

  tmp[len] = '\0';
  memcpy (tmp, name, len);

  /* Handle the case where we have multiple calls to the characters
     handler for the same variable because the data is split
   */
  if (data->gg->vardata[j].collab != NULL) {
    /* need to append tmp to the existing value.*/
  }

  data->gg->vardata[j].collab = tmp;

  /* Note that if we do have multiple calls to this for the same
     variable then we cannot handle the case where the 
     user does not specify the transformation variable
     unless we use a flag in XMLParserData. This is
     variable_transform_name_as_attribute.
   */
  if (data->gg->vardata[j].collab_tform == NULL) {
    data->gg->vardata[j].collab_tform = g_strdup (tmp);
  }

  return (true);
}


/*
  The segments tag should be told the number of segments 
  being specified. This is read and the number of segments
  in the ggobid structure is set to this.

  Called for <segments> tag.
 */
gboolean
allocSegments(const CHAR **attrs, XMLParserData *data)
{
  const char *tmp = getAttribute (attrs, "count");

  if (tmp) {
    gint value = asInteger (tmp);
    data->gg->nsegments = value;
    segments_alloc (value, data->gg);
/*  br_line_color_alloc(data->gg);*/
    br_line_color_init (data->gg);
  }
  return (tmp != NULL);
}


/*
  Reads the specification of a segment.
  Called for <segment> tag.
 */
gboolean
addConnection(const CHAR **attrs, XMLParserData *data)
{
 int i = data->current_segment;
 gboolean ok = false;
 const char *tmp;
 int source=-1, dest=-1;
 int value;
  tmp = getAttribute(attrs, "source");
  if(tmp) {
    source = rowId(tmp, data);
  }
  tmp = getAttribute(attrs, "destination");
  if(tmp) {
    dest = rowId(tmp, data);
  }

  ok = source > -1 && dest > -1;

  if(ok) {
    data->gg->segment_endpoints[i].a = MIN(source, dest) + 1;
    data->gg->segment_endpoints[i].b = MAX(source, dest) + 1;
  }
 
  value = data->defaults.lineColor;
  tmp = getAttribute(attrs, "color");
  if(tmp) {
    value = asInteger(tmp);
  }
  if(value > -1 && value < NCOLORS) {
    data->gg->line.color.vals[i] =
      data->gg->line.color_now.vals[i] =
      data->gg->line.color_prev.vals[i] = value;
  }

 return(ok);
}

/*
   Reads the number of variables in the dataset from the attributes
   and allocates space for them in the ggobid structure.
   At this point, we have the number of records and variables
   and can initialize the data areas of the ggobid structure.

   Called in response to an <edgevariables> tag. (Note the plural.)
 */
gboolean 
allocEdgeVariables (const CHAR **attrs, XMLParserData *data)
{
  const char *tmp = getAttribute (attrs, "count");

  if (tmp == NULL) {
    g_printerr ("The edge variable attribute requires a count\n");
    exit (101);
  }

  data->gg->edge.ncols = asInteger (tmp);

/*
  arrayf_alloc (&data->gg->raw, data->gg->nrows, data->gg->ncols);
  vardata_alloc (data->gg);
  vardata_init (data->gg);
  hidden_alloc (data->gg);
*/

  return (true);
}

/*
   Read the declaration of an edge variable, gathering its information
   from the specified attributes.
   This includes its name, transformation name, etc.

   Called in response to an <edgevariable> tag.
 */

gboolean
newEdgeVariable (const CHAR **attrs, XMLParserData *data)
{
  const gchar *tmp;

  tmp = getAttribute (attrs, "name");
  if (tmp != NULL) {
    gint k = data->current_edgevariable;
    data->gg->edge.vardata[k].collab = g_strdup (tmp);
    data->gg->edge.vardata[k].collab_tform = g_strdup (tmp);
  }

  return (true);
}

gboolean
setDatasetEdgeInfo (const CHAR **attrs, XMLParserData *data)
{
  const gchar *tmp = getAttribute (attrs, "count");

  if (tmp == NULL) {
    g_printerr ("edgevariables has no count attribute\n");
    exit(101);
  }

  data->gg->edge.n = asInteger (tmp);
  data->gg->edge.n_in_plot = data->gg->edge.n;  /*-- for now --*/

/*
  rowlabels_alloc (data->gg);
  br_glyph_ids_alloc (data->gg);
  br_glyph_ids_init (data->gg);

  br_color_ids_alloc (data->gg);
  br_color_ids_init (data->gg);

  tmp = getAttribute(attrs, "missingValue");
  if (tmp != NULL) {
    data->NA_identifier = g_strdup (tmp);
  }

  setGlyph (attrs, data, -1);  
  setColor (attrs, data, -1);
  setHidden (attrs, data, -1, ROW);
*/

  return (true);
}

gboolean 
newEdgeRecord (const CHAR **attrs, XMLParserData *data)
{
  const gchar *tmp;
  gint i = data->current_edgerecord;
  data->current_edgeelement = 0;

  tmp = getAttribute (attrs, "source");
  if (tmp) {
    gint k = atoi (tmp);
    data->gg->edge.head.vals[i] = k;
  }
  tmp = getAttribute (attrs, "destination");
  if (tmp) {
    gint k = atoi (tmp);
    data->gg->edge.tail.vals[i] = k;
  }

  tmp = getAttribute (attrs, "label");
  if (tmp) {
    gchar *stmp = g_strdup (tmp);
    g_array_insert_val (data->gg->edge.lbl, i, stmp);
  }

/*
  setColor (attrs, data, i);
  setGlyph (attrs, data, i);
  setHidden (attrs, data, i, LINE);
 
  tmp = getAttribute (attrs, "id");
  if (tmp) {
    if(data->rowIds == NULL) {
      data->rowIds = (gchar **) g_malloc(data->gg->edge.n * sizeof (gchar *));
      memset(data->rowIds, '\0', data->gg->edge.n);
    }
    data->rowIds[i] = g_strdup(tmp);
  }
*/

/* Might also deal with group (lgroups), lineType, lineWidth */


  return(true);
}

/*
  Read the values for this record from free-formatted text. The entries
  are white-space delimited. They should not have quotes or anything
  that needs to be escaped.
*/

gboolean
setEdgeRecordValues (XMLParserData *data, const CHAR *line, gint len)
{
  gint i = data->current_edgerecord;
  gdouble value;
  const gchar *tmp = strtok ((gchar*) line, " \t\n");

  while (tmp) {
    value = asNumber (tmp);
    data->gg->edge.raw.vals[i][data->current_edgeelement++] = value;
    tmp = strtok (NULL, " \t\n");
  }

  return (true);
}

/*
  Reads the text in name and assigns it as the name of the
  variable currently being read within the 
  <variable> tag. The index for the variable is stored in 
  data->current_variable.

   Called when parsing free-formatted text within a <variable> tag.
 */
gboolean
setEdgeVariableName (XMLParserData *data, const CHAR *name, gint len)
{
  gchar *tmp = (gchar *) g_malloc (sizeof(gchar) * (len+1));
  gint j = data->current_edgevariable;

  tmp[len] = '\0';
  memcpy (tmp, name, len);

  /* Handle the case where we have multiple calls to the characters
     handler for the same variable because the data is split
   */
  if (data->gg->edge.vardata[j].collab != NULL) {
    /* need to append tmp to the existing value.*/
  }

  data->gg->edge.vardata[j].collab = tmp;

  /* Note that if we do have multiple calls to this for the same
     variable then we cannot handle the case where the 
     user does not specify the transformation variable
     unless we use a flag in XMLParserData. This is
     variable_transform_name_as_attribute.
   */
  if (data->gg->edge.vardata[j].collab_tform == NULL) {
    data->gg->edge.vardata[j].collab_tform = g_strdup (tmp);
  }

  return (true);
}

/*------------------ end of edges section ------------------------------*/

gint
rowId (const gchar *tmp, XMLParserData *data)
{
  gint value = atoi(tmp) - 1;
  if (value < 0) {
   /* Now look up the ids for the rows. */
   gint i;
   for (i=0; i < data->gg->nrows; i++) {
/*   if (strcmp(tmp,data->gg->rowlab[i]) == 0*/
     if (strcmp (tmp, g_array_index (data->gg->rowlab, gchar *, i)) == 0 ||
         (data->rowIds != NULL && data->rowIds[i] &&
          strcmp(tmp,data->rowIds[i]) == 0))
      {
        value = i;
        break;
      }
    } 
  }
  return (value);
}




/*
 Prints the attributes.
 For debugging.
*/
void
showAttributes (const CHAR **attrs)
{
  const CHAR **tmp;
  tmp = attrs;
  while (tmp && tmp[0]) {
    g_printerr ("\t %s=%s\n", tmp[0], tmp[1]);
    tmp += 2;
  }
}

/* Finds the directory associated with the specified file.
   Strips away the basename by looking for the last 
   directory separator.
 */
gchar *
getFileDirectory(const gchar *filename)
{

 char *tmp;
  tmp =  strrchr(filename, DIR_SEPARATOR);
  if(tmp) {
    int n = tmp - filename + 2;
    tmp = g_malloc(n*sizeof(char));
    memcpy(tmp, filename, n);
    tmp[n-1] = '\0';
  } else
   tmp = g_strdup("./");
 
 return(tmp);
}

/*
  Checks for files with different extensions.

  The directory is needed so that we can resolve
  files relative to the location of the original 
  file from which it was referenced,
   e.g. colormap file reference from flea.xml
   should be relative to that one.
 */

gchar *
find_xml_file(const gchar *filename, const gchar *dir, ggobid *gg)
{
  int i;
  gchar* name = NULL;
  FILE *f;
  int dirlen = 0;
  const gchar *suffixes[] = {"", ".xml", ".xml.gz", ".xmlz"};
  int nsuffixes = sizeof(suffixes)/sizeof(suffixes[0]);

  if(dir)
    dirlen = strlen(dir);

    /* If filename starts with a /, so it is an absolute name,
       then ignore the directory argument.
     */
  if(filename[0] == DIR_SEPARATOR)
    dirlen = 0;

  for(i = 0; i < nsuffixes;i++) {
    name = g_malloc(sizeof(char)*(dirlen + strlen(filename)+strlen(suffixes[i]) + 2));
    sprintf(name,"%s%s%s", dirlen ? dir : "", filename,suffixes[i]);
    if((f = fopen(name,"r")) != NULL) {
      fclose(f);
      break;
    }
    if(name) {
      g_free(name);
      name = NULL;
    }
  }

  if(name == NULL) {
    /* If we can't find the file, then we should return the filename
       as it might be an http or ftp prefix. Could check this. Later,
       when we know more about the possibilities to expect.
     */
    name = g_strdup(filename);
  }

 return(name);
}

/*

  This is reentrant.  
  First we check the size attribute. Then we check the
  for the specification of an external file.
 */
gboolean
setColorMap(const CHAR **attrs, XMLParserData *data)
{
 const gchar *tmp, *file; 
 int size = 0;
 tmp = getAttribute(attrs, "size");
 file = getAttribute(attrs, "file");

 if(tmp)
   size = asInteger(tmp);
 else {
  if(file == NULL)
   return(false);
 }

 if(file) {
   const gchar *type = getAttribute(attrs, "type");
   if(type != NULL) {
     if(strcmp("xml", type) == 0)
       xmlParseColorMap(file, size, data);
     else
       asciiParseColorMap(file, size, data);
   } else {
       xmlParseColorMap(file, size, data);
   }
 }

 if(size > 0 || file) {
  if(file) {
    data->gg->ncolors +=  size;
    data->gg->default_color_table = (GdkColor *) g_realloc (data->gg->default_color_table , data->gg->ncolors * sizeof (GdkColor));
  } else {
    data->gg->ncolors = size;
    data->gg->default_color_table = (GdkColor *) g_malloc (size * sizeof (GdkColor));
  }
 }


 return(true);
}

gboolean
setColormapEntry(const CHAR **attrs, XMLParserData *data)
{
 const gchar * const names[] = {"r", "g", "b"};
 double vals[3] = {-1., -1. , -1.};
 const gchar *tmp;
 gboolean ok = true;
 int which = data->current_color, i;

 GdkColor *color;
 GdkColormap *cmap = gdk_colormap_get_system ();


 tmp = getAttribute(attrs, "id");

 if(tmp) {
   if(strcmp("bg",tmp) == 0) {
     which = -1;
     color = &data->gg->bg_color;
   }
   else  if(strcmp("fg",tmp) == 0) {
     which = -1;
     color = &data->gg->bg_color;
   }
   else {
       /* Note that we set the current color to this index.
          Thus we can skip values, etc.
        */
     which = data->current_color = asInteger(tmp) - 1;
     color = data->gg->default_color_table + which;
   }
 } else {
     color = data->gg->default_color_table + data->current_color;
 }

 for(i = 0; i < 3; i++) {
  const gchar *tmp1 = getAttribute(attrs, (char *) names[i]);
  if(tmp1) {
   vals[i] = asNumber(tmp1);
  } else {
    ok = false;
    break;
  }
 }

 if(ok) {
   setColorValues(color, vals);

   /* If this is a foreground or background setting, then get the color.
      Otherwise, wait until we have finished the entire 
    */
  if(which < 0)
    gdk_colormap_alloc_color(cmap, color, false, true);
 }

 return(ok);
}

/*
  An RGB value in simple text form.
 */
gboolean
setColorValue(XMLParserData *data, const CHAR *line, int len)
{

 double values[3] = {-1, -1, -1};
 int which = 0;
 const char *tmp = strtok((char*) line, " \t\n");

 GdkColor *color = data->gg->default_color_table + data->current_color;
 
 while(tmp) {
  values[which++] = asNumber(tmp);
  tmp = strtok(NULL, " \t\n");
 }

 if(which == 3)
  setColorValues(color, values);

 return(true); 
}




void
setColorValues(GdkColor *color, double *vals)
{
   color->red = (guint16) (vals[0]*65535.0);
   color->green = (guint16) (vals[1]*65535.0);
   color->blue = (guint16) (vals[2]*65535.0);
}


/*
 The colormap file will have its own size.
 */

gboolean
xmlParseColorMap(const gchar *fileName, int size, XMLParserData *data)
{

 xmlParserCtxtPtr ctx;
   
 char *tmp, *tmp1;

 tmp = getFileDirectory(data->gg->filename);
 tmp1 = find_xml_file(fileName, tmp, data->gg);

 if(tmp1) {
  ctx  = xmlCreateFileParserCtxt(tmp1);

  if(ctx == NULL)
   return(false);

  ctx->userData = data;
  ctx->sax = data->handlers;
  data->reading_colormap_file_p = true;

  xmlParseDocument(ctx);

  ctx->sax = NULL;
  xmlFreeParserCtxt(ctx);

  data->reading_colormap_file_p = false;

  g_free(tmp1);
 }

  g_free(tmp);

 return(size == data->gg->ncolors);
}


/*
  Reads color map entries from an ASCII file as a rectangular array
  of size, at most, size by 3 rows.

  Doesn't do anything at the moment.
 */

gboolean
asciiParseColorMap(const gchar *fileName, int size, XMLParserData *data)
{

 return(false);
}
