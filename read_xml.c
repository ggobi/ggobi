#include <libxml/parserInternals.h>

#include <stdlib.h>
#include <string.h>

#include "read_xml.h"

#include "ggobi.h"
#include "externs.h"


void startXMLElement(void *user_data, const CHAR *name, const CHAR **attrs);
void endXMLElement(void *user_data, const CHAR *name);
void characters(void *user_data, const CHAR *ch, int len);


const gchar * const xmlDataTagNames[] = {
                                          "ggobidata",
                                          "description",
                                          "record",
                                          "records",
                                          "variables",
                                          "variable",
                                          "data",
                                          "segments",
                                          "segment",
                                          ""
                                         };



/*
  We also need a version that takes a FILE*
  and reads from it. This is so that we can
  handle reading from standard input.
 */


gboolean
data_xml_read(const gchar *filename, ggobid *gg)
{
 xmlSAXHandlerPtr xmlParserHandler;
 xmlParserCtxtPtr ctx = (xmlParserCtxtPtr) g_malloc(sizeof(xmlParserCtxtPtr));
 XMLParserData data;
  gchar* name = g_malloc(sizeof(char)* strlen(filename)+4);
  sprintf(name,"%s%s", filename,".xml");

  initParserData(&data, gg);

  xmlParserHandler = (xmlSAXHandlerPtr) g_malloc(sizeof(xmlSAXHandler));

  xmlParserHandler->startElement = startXMLElement;
  xmlParserHandler->endElement = endXMLElement;
  xmlParserHandler->characters = characters;

  ctx = xmlCreateFileParserCtxt(name);
  if(ctx == NULL) {
   xml_warning("File error:", name, "Can't open file ", &data);
   return(false);
  }

  ctx->userData = &data;
  ctx->sax = xmlParserHandler;

  /*
 xmlDocPtr doc;
    doc = xmlParseFile(name);
  */

  xmlParseDocument(ctx);

  ctx->sax = NULL;
  xmlFreeParserCtxt(ctx);


  vgroups_sort(gg);
  { int j;
  for (j=0; j<gg->ncols; j++)
    gg->vardata[j].groupid = gg->vardata[j].groupid_ori;
  }

  if(gg->nsegments < 1)
   segments_create(gg);


 return(1);
}

void
initParserData(XMLParserData *data, ggobid *gg)
{
  data->gg=gg;
  data->current_record = 0;
  data->current_variable = 0;
  data->current_element = 0;
  data->current_segment = 0;
  data->state = UNKNOWN;
  data->terminateStrings = true;
  data->NA_identifier = NULL;
  data->rowIds = NULL;

  data->defaults.color = -1;
  data->defaults.glyphType = -1;
  data->defaults.glyphSize = -1;
  data->defaults.lineWidth = -1;
  data->defaults.lineColor = -1;
}

void 
startXMLElement(void *user_data, const CHAR *name, const CHAR **attrs)
{
 XMLParserData *data = (XMLParserData*)user_data;
 enum xmlDataState type = tagType(name, false);

 data->state = type;

 switch(type) {
   case VARIABLE:
     newVariable(attrs, data);
    break;
   case VARIABLES:
     allocVariables(attrs, data);
    break;
   case TOP:
     setDatasetInfo(attrs, data);
    break;
   case RECORD:
     newRecord(attrs, data);
    break;
   case CONNECTIONS:
    allocSegments(attrs, data);
    break;
   case CONNECTION:
     addConnection(attrs, data);
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
   case CONNECTION:
     data->current_segment++;
     break;
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


void 
characters(void *user_data, const CHAR *ch, int len)
{
 char *tmp;
 int dlen = len;
 const CHAR *c;
 XMLParserData *data = (XMLParserData*)user_data;

 c = skipWhiteSpace(ch, &dlen);
 if(dlen < 1 || c[0] == '\n')
  return;

 if(data->terminateStrings) {
  tmp = g_malloc(sizeof(char)*(dlen+1));

  memcpy(tmp, c, dlen);
  memset(tmp+dlen, '\0', 1);

  c = tmp;
 }


 switch(data->state) {
   case RECORD:
     setRecordValues(data, c, dlen);
   break;
   case VARIABLE:
     setVariableName(data, c, dlen);
   break;
   default:
     break;

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
setDatasetInfo(const CHAR **attrs, XMLParserData *data)
{
 const char *tmp = getAttribute(attrs, "numRecords");

 if(tmp == NULL) {
    fprintf(stderr, "No numRecords attribute\n");
    exit(101);
 }

 data->gg->nrows = asInteger(tmp);
 data->gg->nrows_in_plot = data->gg->nrows;  /*-- for now --*/
 data->gg->nrgroups = 0;              /*-- for now --*/

 rowlabels_alloc(data->gg);
 br_glyph_ids_alloc(data->gg);
 br_glyph_ids_init(data->gg);

 br_color_ids_alloc(data->gg);
 br_color_ids_init(data->gg);


  tmp = getAttribute(attrs, "missingValue");
  if(tmp != NULL) {
    data->NA_identifier = g_strdup(tmp);
  }

 setGlyph(attrs, data, -1);  
 setColor(attrs, data, -1);
 /*
 setLineColor(attrs, data, -1);
 setLineWidth(attrs, data, -1);  
 */

 return(true);
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
  if(tmp)
    data->gg->rowlab[data->current_record] = g_strdup(tmp);


  setColor(attrs, data, i);
  setGlyph(attrs, data, i);
 
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
     data->gg->color_ids[i] = data->gg->color_now[i] = data->gg->color_prev[i] = value;    
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
  if(tmp) {
   value = asInteger(tmp);
  }

  if(value < 0 || value > NGLYPHSIZES) {
    if(tmp)
      xml_warning("glyphSize", tmp, "Out of range", data);
  } else {
   if(i < 0)
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
         data->gg->glyph_ids[j].type = data->gg->glyph_now[j].type = data->gg->glyph_prev[j].type = value;       
      } else {
        value = atoi(next);
        if(i < 0)
            data->defaults.glyphSize = value;
        else
        data->gg->glyph_ids[j].size = data->gg->glyph_now[j].size = data->gg->glyph_prev[j].size = value;     
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
 fprintf(stderr,"Incorrect data (record %d)\n",  data->current_record);
 fprintf(stderr,"\t%s %s: value = %s\n",  attribute, msg, value);
}


gboolean
setRecordValues(XMLParserData *data, const CHAR *line, int len)
{

 const char *tmp = strtok((char*) line, " \t\n");
 double value;

 while(tmp) {
  value = asNumber(tmp);
  data->gg->raw.data[data->current_record][data->current_element++] = value;
  tmp = strtok(NULL, " \t\n");
 }


 return(true);
}

double
asNumber(const char *sval)
{
  return(atof(sval));
}

gboolean
newVariable(const CHAR **attrs, XMLParserData *data)
{
 int groupId = data->current_variable;
 const char *tmp = getAttribute(attrs, "name");
 if(tmp != NULL) {
  data->gg->vardata[data->current_variable].collab = g_strdup(tmp);
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


gboolean 
allocVariables(const CHAR **attrs, XMLParserData *data)
{

 const char *tmp = getAttribute(attrs, "count");

 if(tmp == NULL) {
    fprintf(stderr, "No count for variables attribute\n");
    exit(101);
 }

 data->gg->ncols = asInteger(tmp);

 arrayf_alloc(&data->gg->raw, data->gg->nrows, data->gg->ncols);

 vardata_alloc(data->gg);
 vardata_init(data->gg);

 hidden_alloc(data->gg);

 return(true);
}


gboolean
setVariableName(XMLParserData *data, const CHAR *name, int len)
{

 char *tmp = (char *) g_malloc(sizeof(char) * (len+1));

  tmp[len] = '\0';
  memcpy(tmp, name, len);
  /*
  int i; 
  for(i = 0; i < len; i++)
    tmp[i] = name[i];
  */

  /*  snprintf(tmp, len, "%s", name); */

  data->gg->vardata[data->current_variable].collab = tmp;
  data->gg->vardata[data->current_variable].collab_tform = g_strdup(tmp);


 return(true);
}


gboolean
allocSegments(const CHAR **attrs, XMLParserData *data)
{
 const char *tmp = getAttribute(attrs, "count");

 if(tmp) {
  int value = asInteger(tmp);
  data->gg->nsegments = value;
  segments_alloc(value, data->gg);
  br_line_color_alloc(data->gg);
  br_line_color_init(data->gg);
 }
 return(tmp != NULL);
}


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
    data->gg->line_color[i] = data->gg->line_color_now[i] =
           data->gg->line_color_prev[i] = value;
  }
  

 return(ok);
}

int
rowId(const char *tmp, XMLParserData *data)
{
 int value = atoi(tmp) - 1;
 if(value < 0) {
   /* Now look up the ids for the rows. */
   int i;
   for(i=0; i < data->gg->nrows; i++) {
     if(strcmp(tmp,data->gg->rowlab[i]) == 0
	|| (data->rowIds != NULL && data->rowIds[i] && strcmp(tmp,data->rowIds[i]) == 0)) {
       value =i;
       break;
     }
   } 


 }
 return(value);
}




/*
 Prints the attributes.
 For debugging.
*/
void
showAttributes(const CHAR **attrs)
{
 const CHAR **tmp;
 tmp = attrs;
 while(tmp && tmp[0]) {
  fprintf(stderr, "\t %s=%s\n", tmp[0], tmp[1]);
  tmp += 2;
 }
}
