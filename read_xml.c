#if 1
#include <libxml/parserInternals.h>
#else
#include <parserInternals.h>
#endif


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
   which record, variable or edge is next in the stream. These are 
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
#include <gtk/gtk.h>

#include "read_xml.h"

#include "externs.h"

#include "GGobiAPI.h"


void startXMLElement(void *user_data, const xmlChar *name, const xmlChar **attrs);
void endXMLElement(void *user_data, const xmlChar *name);
void Characters(void *user_data, const xmlChar *ch, gint len);

const gchar *XMLSuffixes[] = {"", ".xml", ".xml.gz", ".xmlz"};

const gchar * const xmlDataTagNames[] = {
                                          "ggobidata",
                                          "data",        /* DATASET */
                                          "description",
                                          "record",
                                          "records",
                                          "variables",
                                          "variable",
                                          "colormap",
                                          "color",
/* New for handling categorical variables. */
                                          "realvariable",
                                          "categoricalvariable",
                                          "levels",
                                          "level",
                                          ""
                                         };





void
ggobi_XML_warning_handler(void *data, const gchar *msg, ...)
{
    xmlParserCtxtPtr p = (xmlParserCtxtPtr) ((XMLParserData*) data)->parser;
    fprintf(stderr, "Warning from XML parsing [%d, %d]: %s", p->input->line, p->input->col, msg);
/*
    fprintf(stderr, msg, ...); 
*/
    fflush(stderr);  
}
void
ggobi_XML_error_handler(void *data, const gchar *msg, ...)
{
    xmlParserCtxtPtr p = (xmlParserCtxtPtr) ((XMLParserData*) data)->parser;
    fprintf(stderr, "Error in XML parsing [line %d, column %d]: %s", p->input->line, p->input->col, msg);
/*
    fprintf(stderr, msg, ...); 
*/
    fflush(stderr);
}


/*
  We also need a version that takes a FILE*
  and reads from it. This is so that we can
  handle reading from standard input.

  The DOM style parsing can be initiated very simply.

  xmlDocPtr doc;
    doc = xmlParseFile(name);
 */



gboolean
data_xml_read (InputDescription *desc, ggobid *gg)
{
 xmlSAXHandlerPtr xmlParserHandler;
 xmlParserCtxtPtr ctx = (xmlParserCtxtPtr) g_malloc(sizeof(xmlParserCtxtPtr));
 XMLParserData data;
 gboolean ok = false;  
 gchar *name = g_strdup(desc->fileName); /* find_xml_file(desc->fileName, NULL, gg); */

  if (name == NULL)
    return (false);

  if (strcmp(name, desc->fileName) != 0) {
    g_printerr("Different input file name and resolved file name. Please report.\n");
 }

  xmlParserHandler = (xmlSAXHandlerPtr) g_malloc(sizeof(xmlSAXHandler));
  /* Make certain this is initialized so that we don't have any references
     to unwanted routines!
   */
  memset(xmlParserHandler, '\0', sizeof(xmlSAXHandler));

  xmlParserHandler->startElement = startXMLElement;
  xmlParserHandler->endElement = endXMLElement;
  xmlParserHandler->characters = Characters;

  xmlParserHandler->error = ggobi_XML_error_handler;
  xmlParserHandler->warning = ggobi_XML_warning_handler;

  initParserData(&data, xmlParserHandler, gg);

  ctx = xmlCreateFileParserCtxt(name);
  if(ctx == NULL) {
   xml_warning("File error:", name, "Can't open file ", &data);
   g_free (name);
   return(false);
  }

  ctx->validate = 1;

  ctx->userData = &data;
  data.parser = ctx;
  data.input = desc;
  ctx->sax = xmlParserHandler;

  xmlParseDocument(ctx);

  ctx->sax = NULL;
  xmlFreeParserCtxt(ctx);

  g_free(xmlParserHandler);
  g_free (name);

  {
    GSList *l;
    datad *d;
    ok = true;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      /* ok &= (d->ncols > 0 && d->nrows > 0); */
      /*-- a simple edge set has no variables --*/
      ok &= (d->nrows > 0);
    }
  }
      
  return (ok);
}

void
initParserData(XMLParserData *data, xmlSAXHandlerPtr handler, ggobid *gg)
{
  data->gg=gg;
  data->current_record = 0;
  data->current_variable = 0;
  data->current_element = 0;
  data->current_data = NULL;

  data->current_color = 0;
  data->reading_colormap_file_p = false;
  data->state = UNKNOWN;
  data->terminateStrings_p = true;
  data->NA_identifier = NULL;
  data->rowIds = NULL;
  data->handlers = handler;
  data->defaults.color = -1;
#ifndef SUPPORT_PLUGINS
  data->defaults.glyphType = -1;
  data->defaults.glyphSize = -1;
#else
  data->defaults.glyphType = sessionOptions->info->glyph.type;
  data->defaults.glyphSize = sessionOptions->info->glyph.size;
#endif
  data->defaults.edgeWidth = -1;  /*-- this has no home in ggobi --*/
  data->defaults.hidden = false;
}

void 
startXMLElement(void *user_data, const xmlChar *name, const xmlChar **attrs)
{
 XMLParserData *data = (XMLParserData*)user_data;
 enum xmlDataState type = tagType(name, false);

 data->state = type;

 switch (type) {
   case VARIABLES:
     allocVariables (attrs, data);
   break;
   case VARIABLE:
   case REAL_VARIABLE:
   case CATEGORICAL_VARIABLE:
     newVariable (attrs, data, name);
   break;

   case CATEGORICAL_LEVELS:
     categoricalLevels(attrs, data);
   break;
   case CATEGORICAL_LEVEL:
     setLevelIndex(attrs, data);
   break;

   case RECORDS: /* Used to be DATASET */
     setDatasetInfo(attrs, data);
   break;
   case TOP:
     setGeneralInfo(attrs, data);
   break;
   case RECORD:
     newRecord(attrs, data);
   break;
   case COLORMAP:
     setColorMap(attrs, data);
   break;
   case COLOR:
     setColormapEntry(attrs, data);
   break;
   case DESCRIPTION:
     /* description text pending */
   break;
   case DATASET:
     setDataset(attrs, data);
   break;
   
   default:
       fprintf(stderr, "Unrecognized XML state\n"); fflush(stderr);    
   break;
 }
}

gint
setLevelIndex(const xmlChar **attrs, XMLParserData *data)
{
  const gchar *tmp = getAttribute(attrs, "value");
  gint itmp;
  datad *d = getCurrentXMLData(data);
  vartabled *el = vartable_element_get (data->current_variable, d);

  data->current_level++; /*-- current_level here ranges from 0 : nlevels-1 --*/

/*-- dfs: placeholder for proper debugging --*/
  if (g_list_length (el->level_values) == el->nlevels)
    g_printerr ("trouble: adding too many levels to %s\n", el->collab);
/* */

  if (tmp != NULL) {
    itmp = strToInteger (tmp);
    if (itmp < 0) g_printerr ("trouble: levels must be >= 0\n");
    el->level_values = g_list_append (el->level_values,
      GINT_TO_POINTER(itmp));
  } else {
    el->level_values = g_list_append (el->level_values,
      GINT_TO_POINTER(data->current_level));
  }
 
  return(data->current_level);
}

void
categoricalLevels(const xmlChar **attrs, XMLParserData *data)
{
  datad *d = getCurrentXMLData(data);
  vartabled *el = vartable_element_get (data->current_variable, d);

  const gchar *tmp = getAttribute(attrs, "count");

  if (tmp != NULL) {
    el->nlevels = strToInteger(tmp);
    el->level_values = NULL;
    el->level_names = g_array_new (false, false, sizeof(gchar *));       
/*
    g_array_set_size(el->level_names, el->nlevels);
*/
  }

  data->current_level = -1; /* We'll increment the first one. */

  if(el->nlevels < 1) {
    fprintf(stderr, "Levels for %s mis-specified\n", el->collab);
    fflush(stderr); 
  }
}

void
addLevel(XMLParserData *data, const gchar *c, gint len)
{
  datad *d = getCurrentXMLData(data);
  vartabled *el = vartable_element_get (data->current_variable, d);

  gchar *val = g_strdup(c);

/*-- dfs: placeholder for proper debugging --*/
  if (el->level_names->len == el->nlevels)
    g_printerr ("trouble: adding too many levels to %s\n", el->collab);

  g_array_append_val(el->level_names, val);
}


void endXMLElement(void *user_data, const xmlChar *name)
{
 XMLParserData *data = (XMLParserData*)user_data;
 enum xmlDataState type = tagType(name, true);

 switch(type) {
   case RECORD:
     data->current_record++;
   break;
   case VARIABLE:
   case REAL_VARIABLE:
   case CATEGORICAL_VARIABLE:
     data->current_variable++;
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
   break;
   case CATEGORICAL_LEVELS:
   break;
   case CATEGORICAL_LEVEL:
   break;
   default:
     data = NULL; /* just any code so we can stop.*/
   break;
  }
}


XmlTagType
tagType(const xmlChar *name, gboolean endTag)
{
 gint n = sizeof(xmlDataTagNames)/sizeof(xmlDataTagNames)[0] - 1; 
 gint i;
 const gchar *tmp = (const gchar *)name;
 /*
  if(endTag) {
   tmp++;
  }
 */

  for(i = 0; i < n; i++) {
    if(strcmp(tmp, xmlDataTagNames[i]) == 0) {
     return((enum xmlDataState) i);
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
Characters(void *user_data, const xmlChar *ch, gint len)
{
 gchar *tmp = NULL;
 gint dlen = len;
 const xmlChar *c;
 XMLParserData *data = (XMLParserData*)user_data;

 c = (const xmlChar *) skipWhiteSpace(ch, &dlen);
 if(dlen < 1 || c[0] == '\n')
  return;

 if(data->terminateStrings_p) {
  tmp = (gchar *) g_malloc(sizeof(gchar)*(dlen+1));

  memcpy(tmp, c, dlen);
  memset(tmp+dlen, '\0', 1);

  c = (const xmlChar *) tmp;
 }

 switch(data->state) {
   case RECORD:
     setRecordValues (data, c, dlen);
   break;
   case VARIABLE:
   case CATEGORICAL_VARIABLE:
   case REAL_VARIABLE:
     setVariableName (data, c, dlen);
   break;
   case COLOR:
     setColorValue (data, c, dlen);
     break;
   case CATEGORICAL_LEVEL:
     addLevel(data, (const gchar *) c, dlen);
   break;
   default:
   break;

 }

 if(data->terminateStrings_p) {
  g_free(tmp);
 }
}

const xmlChar *
skipWhiteSpace(const xmlChar *ch, gint *len)
{
 const xmlChar *tmp = ch;
  while(*len >= 0) {
   if(*len == 0 || (tmp[0] != ' ' && tmp[0] != '\t' && tmp[0] != '\n'))
    break;
   tmp++;
   (*len)--;
  }

  return(tmp);
}


/**
  Called in response to a ggobidata tag which contains
  the different datasets. The count element here is the
  number of datasets to expect.
 */
gboolean
setGeneralInfo (const xmlChar **attrs, XMLParserData *data)
{
  const gchar *tmp = getAttribute(attrs, "count");

  if (tmp != NULL) {
    data->expectedDatasetCount = strToInteger(tmp);
  }

  return(true);
}


gboolean
setDatasetInfo (const xmlChar **attrs, XMLParserData *data)
{
  const gchar *tmp = getAttribute(attrs, "count");
  datad *d = getCurrentXMLData(data);

  if (tmp == NULL) {
    g_printerr ("No count attribute\n");
    exit(101);
  }

  d->nrows = strToInteger(tmp);
  d->nrows_in_plot = d->nrows;  /*-- for now --*/

  rowlabels_alloc (d, data->gg);
  br_glyph_ids_alloc (d);
  br_glyph_ids_init (d, data->gg);

  d->edge.n = 0;

  br_color_ids_alloc (d, data->gg);
  br_color_ids_init (d, data->gg);

  setDefaultDatasetValues(attrs, data);

  if (tmp) {
    arrayf_alloc (&d->raw, d->nrows, d->ncols);
    hidden_alloc (d);
  }

  data->current_variable = 0;
  data->current_record = 0;
  data->current_variable = 0;
  data->current_element = 0;

/*-- dfs: this seems to be needed; are there more? --*/
  data->rowIds = NULL;

  return (true);
}

gboolean
setDefaultDatasetValues(const xmlChar **attrs, XMLParserData *data)
{

 const gchar * tmp = getAttribute(attrs, "missingValue");
  if(tmp != NULL) {
    data->NA_identifier = g_strdup(tmp);
  }

  setGlyph (attrs, data, -1);  
  setColor (attrs, data, -1);
  setHidden (attrs, data, -1);
 return(true);
}

gint
strToInteger(const gchar *tmp)
{
 gint value;

  value = atoi(tmp);

 return(value);
}


const gchar *
getAttribute(const xmlChar **attrs, gchar *name)
{
 const xmlChar **tmp = attrs;
 while(tmp && tmp[0]) {
  if(strcmp(name, (const gchar *)tmp[0]) == 0)
      return((const gchar *)tmp[1]);
   tmp += 2;
 }

 return(NULL);
}

gboolean 
newRecord(const xmlChar **attrs, XMLParserData *data)
{
  datad *d = getCurrentXMLData(data);

  d->readXMLRecord(attrs, data);

 return(true);
}

gboolean
setHidden(const xmlChar **attrs, XMLParserData *data, gint i)
{
  const gchar *tmp;
  datad *d = getCurrentXMLData(data);

  tmp = getAttribute(attrs, "hidden");
  if(tmp) {
    gboolean hidden = asLogical(tmp);

    if (i < 0) {
      data->defaults.hidden = hidden;
    } else
      d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i] = hidden;
  }

 return(tmp != NULL);
}

gboolean
asLogical(const gchar *sval)
{
 guint i;
 gboolean val = false;
 const gchar *const trues[] = {"T","true", "True","1"};
  for(i = 0; i < sizeof(trues)/sizeof(trues[0]); i++) {
    if(strcmp(sval, trues[i]) == 0)
      return(true);
  }

 return(val);
}

gboolean
setColor(const xmlChar **attrs, XMLParserData *data, gint i)
{
  const gchar *tmp;
  gint value = data->defaults.color;
  datad *d = getCurrentXMLData(data);

  tmp = getAttribute(attrs, "color");
  if(tmp) {
    value = strToInteger(tmp);
  }

  if(value < 0 || value > MAXNCOLORS) {
    if(tmp)
      xml_warning("color", tmp, "Out of range", data);
  } else {
    if(i < 0)
     data->defaults.color = value;
    else 
     d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] = value;
  }

 return (value != -1);
}

gboolean
setGlyph(const xmlChar **attrs, XMLParserData *data, gint i)
{
  const gchar *tmp;
  gint value;
  datad *d = getCurrentXMLData(data);

  /*-- glyphSize  0:7 --*/
  value = data->defaults.glyphSize;
  tmp = getAttribute(attrs, "glyphSize");
  if (tmp) {
   value = strToInteger(tmp);
  }

  if (value < 0 || value >= NGLYPHSIZES) {
    if (tmp)
      xml_warning ("glyphSize", tmp, "Out of range", data);
  } else {
     if (i < 0) {
      data->defaults.glyphSize = value;
    } else
      d->glyph.els[i].size = d->glyph_now.els[i].size 
             = d->glyph_prev.els[i].size = value;
  }


  /*-- glyphType  0:6 --*/
  value = data->defaults.glyphType;
  tmp = getAttribute(attrs, "glyphType");
  if(tmp) {
   value = strToInteger(tmp);
  }

  if(value < 0 || value >= NGLYPHTYPES) {
    if(tmp)
      xml_warning("glyphType", tmp, "Out of range", data);
  } else {
    if(i < 0)
      data->defaults.glyphType = value;
    else
      d->glyph.els[i].type = d->glyph_now.els[i].type = 
           d->glyph_prev.els[i].type = value;
  }


  tmp = getAttribute(attrs, "glyph");
  if(tmp != NULL) {
    const gchar *next;
    gint j;
    next = tmp;
    next = strtok((gchar *)tmp, " ");
    j = 0;
    while(next) {
      if(j == 0) {
         value = mapGlyphName(next);
          if(i < 0)
            data->defaults.glyphType = value;
          else
            d->glyph.els[i].type = d->glyph_now.els[i].type =
              d->glyph_prev.els[i].type = value;       
      } else {
        value = atoi(next);
        if(i < 0) { 
          if(data->defaults.glyphSize < 0) 
	    data->defaults.glyphSize = value;
        } else
          d->glyph.els[i].size = d->glyph_now.els[i].size =
            d->glyph_prev.els[i].size = value;     
      }
     j++;
     next = strtok(NULL, " ");
    }
  }

  return (value != -1);
}


void
xml_warning(const gchar *attribute, const gchar *value, const gchar *msg,
  XMLParserData *data)
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
setRecordValues (XMLParserData *data, const xmlChar *line, gint len)
{
  gdouble value;
  const gchar *tmp = strtok((gchar*) line, " \t\n");
  datad *d = getCurrentXMLData(data);
  vartabled *vt;

  while (tmp) {
    if (data->NA_identifier && strcmp (tmp, data->NA_identifier) == 0) {
      if (d->nmissing == 0) {
        arrays_alloc (&d->missing, d->nrows, d->ncols);
        arrays_zero (&d->missing);
      }
      d->missing.vals[data->current_record][data->current_element] = 1;
      vt = vartable_element_get (data->current_element, d);
      vt->nmissing++;
      d->raw.vals[data->current_record][data->current_element] = 0;
      d->nmissing++;

    } else {
      value = asNumber (tmp);
      d->raw.vals[data->current_record][data->current_element] = value;
    }
    data->current_element++;
    tmp = strtok (NULL, " \t\n");
  }

  return (true);
}

/*
  Convert the specified string to a numeric value.
 */
gdouble
asNumber(const char *sval)
{
  return(atof(sval));
}




/*
   Read the declaration of a variable, gathering its information
   from the specified attributes.
   This includes its name, transformation name, etc.

   Called in response to a <variable>, <realvariable> or <categoricalvariable> tag.
 */

gboolean
newVariable(const xmlChar **attrs, XMLParserData *data, const xmlChar *tagName)
{
  const gchar *tmp, *tmp1;
  datad *d = getCurrentXMLData(data);
  vartabled *el = vartable_element_get (data->current_variable, d);

  data->variable_transform_name_as_attribute = false;
  tmp = getAttribute(attrs, "transformName");
  if (tmp) {
    data->variable_transform_name_as_attribute = true;
    el->collab_tform = g_strdup(tmp);
  }

 tmp = getAttribute(attrs, "name");
 if(tmp != NULL) {
   el->collab = g_strdup(tmp);
   if (data->variable_transform_name_as_attribute == false)
      el->collab_tform = g_strdup(tmp);
 }

 tmp = getAttribute(attrs, "min");
 tmp1 = getAttribute(attrs, "max");
 if(tmp && tmp1) {
     gdouble mn, mx;
     mn = asNumber(tmp);
     mx = asNumber(tmp1);
     el->lim_specified.min = mn < mx ? mn : mx;
     el->lim_specified.max = mn > mx ? mn : mx;
     /* ? */
     el->lim_specified_tform.min = el->lim_specified.min;
     el->lim_specified_tform.max = el->lim_specified.max;

     if(mn > mx) {
       fprintf(stderr,
         "Minimum is greater than maximum for variable %s\n", el->collab);
       fflush(stderr);
     }
     el->lim_specified_p = true;
 }


 if (strcmp(tagName, "categoricalvariable") == 0) {
     el->categorical_p = true;
 }

  return (true);
}


/*
   Reads the number of variables in the dataset from the attributes
   and allocates space for them in the ggobid structure.
   At this point, we have the number of records and variables
   and can initialize the data areas of the ggobid structure.

    Called in response to a <variables> tag. (Note the plural.)
 */
gboolean 
allocVariables (const xmlChar **attrs, XMLParserData *data)
{
  const gchar *tmp = getAttribute (attrs, "count");
  datad *d = getCurrentXMLData(data);

  if(tmp == NULL) {
    g_printerr ("No count for variables attribute\n");
    exit(101);
  }

  d->ncols = strToInteger(tmp);

  arrayf_alloc (&d->raw, d->nrows, d->ncols);
  hidden_alloc (d);

  vartable_alloc (d);
  vartable_init (d);

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
setVariableName(XMLParserData *data, const xmlChar *name, gint len)
{
  gchar *tmp = (gchar *) g_malloc (sizeof(gchar) * (len+1));
  datad *d = getCurrentXMLData(data);
  vartabled *el = vartable_element_get (data->current_variable, d);

  tmp[len] = '\0';
  memcpy (tmp, name, len);

  /* Handle the case where we have multiple calls to the characters
     handler for the same variable because the data is split
   */
  if (el->collab != NULL) {
    /* need to append tmp to the existing value.*/
  }

  el->collab = tmp;

  /* Note that if we do have multiple calls to this for the same
     variable then we cannot handle the case where the 
     user does not specify the transformation variable
     unless we use a flag in XMLParserData. This is
     variable_transform_name_as_attribute.
   */
  if (el->collab_tform == NULL) {
    el->collab_tform = g_strdup (tmp);
  }

  return (true);
}

/*----------------------------------------------------------------------*/

gint
rowId (const gchar *tmp, XMLParserData *data)
{
  datad *d = getCurrentXMLData(data);
  gint value = atoi(tmp) - 1;

  if (value < 0) {
   /* Now look up the ids for the rows. */
   gint i;
   for (i=0; i < d->nrows; i++) {
     if (strcmp (tmp, g_array_index (d->rowlab, gchar *, i)) == 0 ||
         (data->rowIds != NULL && data->rowIds[i] &&
          strcmp(tmp, data->rowIds[i]) == 0))
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
showAttributes (const xmlChar **attrs)
{
  const xmlChar **tmp;
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

 gchar *tmp;
  tmp =  strrchr(filename, G_DIR_SEPARATOR);
  if(tmp) {
    gint n = tmp - filename + 2;
    tmp = (gchar*) g_malloc(n*sizeof(gchar));
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
  gint i;
  gchar* name = NULL;
  FILE *f;
  gint dirlen = 0;
  const gchar **suffixes = XMLSuffixes;
  gint nsuffixes = sizeof(suffixes)/sizeof(suffixes[0]);

  if(dir)
    dirlen = strlen(dir);

    /* If filename starts with a /, so it is an absolute name,
       then ignore the directory argument.
     */
  if(filename[0] == G_DIR_SEPARATOR)
    dirlen = 0;

  for(i = 0; i < nsuffixes;i++) {
    name = (gchar*) g_malloc(sizeof(gchar) *
      (dirlen + strlen(filename)+strlen(suffixes[i]) + 2));
    sprintf(name,"%s/%s%s", dirlen ? dir : "", filename, suffixes[i]);
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
setColorMap(const xmlChar **attrs, XMLParserData *data)
{
 const gchar *tmp, *file; 
 gint size = 0;
 tmp = getAttribute(attrs, "size");
 file = getAttribute(attrs, "file");

 if(tmp)
   size = strToInteger(tmp);
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

 /*
  * This appends the colors, but I don't want to allow more than
  * MAXNCOLORS colors no matter what the user asks for, so I'll be
  * ignoring some of the colors in the set.  -- dfs
 */
 if(size > 0 || file) {
  ggobid *gg = data->gg;
  if(file) {
    gg->ncolors += size;
    gg->ncolors = MIN (gg->ncolors, MAXNCOLORS);
    gg->color_table = (GdkColor *)
      g_realloc (gg->color_table, gg->ncolors * sizeof (GdkColor));
    gg->colorNames = (gchar **)
      g_realloc (gg->colorNames, gg->ncolors * sizeof (gchar *));
    memset(gg->colorNames + (gg->ncolors-size), '\0', size*sizeof(gchar *));
  } else {
    gg->ncolors = size;
    gg->ncolors = MIN (gg->ncolors, MAXNCOLORS);
    gg->color_table = (GdkColor *) g_malloc (size * sizeof (GdkColor));
    gg->colorNames = (gchar **) g_malloc (size * sizeof (gchar *));
    memset(gg->colorNames, '\0', size * sizeof (gchar *));
  }
 }


 return(true);
}

gboolean
setColormapEntry(const xmlChar **attrs, XMLParserData *data)
{
 const gchar * const names[] = {"r", "g", "b"};
 gdouble vals[3] = {-1., -1. , -1.};
 const gchar *tmp;
 gboolean ok = true;
 gint which = data->current_color, i;
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
     which = data->current_color = strToInteger(tmp) - 1;
     color = data->gg->color_table + which;
   }
 } else {
     color = data->gg->color_table + data->current_color;
 }

 for(i = 0; i < 3; i++) {
  const gchar *tmp1 = getAttribute(attrs, (gchar *) names[i]);
  if(tmp1) {
   vals[i] = asNumber(tmp1);
  } else {
    ok = false;
    break;
  }
 }

 if(which > -1 && which < data->gg->ncolors && (tmp = getAttribute(attrs, "name") ) ) {
    data->gg->colorNames[which] = g_strdup(tmp);
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
setColorValue(XMLParserData *data, const xmlChar *line, gint len)
{

 gdouble values[3] = {-1, -1, -1};
 gint which = 0;
 const gchar *tmp = strtok((gchar*) line, " \t\n");

 GdkColor *color = data->gg->color_table + data->current_color;
 
 while(tmp) {
  values[which++] = asNumber(tmp);
  tmp = strtok(NULL, " \t\n");
 }

 if(which == 3)
  setColorValues(color, values);

 return(true); 
}



void
setColorValues(GdkColor *color, gdouble *vals)
{
   color->red = (guint16) (vals[0]*65535.0);
   color->green = (guint16) (vals[1]*65535.0);
   color->blue = (guint16) (vals[2]*65535.0);
}


/*
 The colormap file will have its own size.
 */

gboolean
xmlParseColorMap(const gchar *fileName, gint size, XMLParserData *data)
{

 xmlParserCtxtPtr ctx;
 gchar *tmp, *tmp1;

 tmp = g_strdup(data->input->dirName);   /* getFileDirectory(data->input->filename); */
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

   addInputFile(data->input, tmp1);
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
asciiParseColorMap(const gchar *fileName, gint size, XMLParserData *data)
{

 return(false);
}


gboolean
setDataset(const xmlChar **attrs, XMLParserData *parserData) 
{
  datad *data;
  gchar *name;
  const gchar *tmp;

  data = datad_new(NULL, parserData->gg);
  data->readXMLRecord = readXMLRecord;

  tmp = getAttribute(attrs, (gchar *) "name");
  if(tmp == NULL) {
    name = (gchar *) malloc(sizeof(gchar)*8);
    sprintf(name, "data%d", (gint) g_slist_length(parserData->gg->d));
  } else
    name = g_strdup(tmp);

  data->name = name;
  parserData->current_data = data;

 return(true);
}


datad *
getCurrentXMLData(XMLParserData* parserData)
{
  datad *data = parserData->current_data;
  if(data == NULL) {
    data = datad_new(NULL, parserData->gg);
    parserData->current_data = data;
  }
  if(data->input == NULL)
    data->input = parserData->input;
  return(data);
}

gboolean
readXMLRecord(const xmlChar **attrs, XMLParserData *data)
{
  datad *d = getCurrentXMLData(data);
  const gchar *tmp;
  gchar *stmp;
  gint i = data->current_record;
  gint start, end;

  data->current_element = 0;

  tmp = getAttribute(attrs, "label");
  if(!tmp) {
      /* Length is to hold the current record number as a string. */
    stmp = g_malloc(sizeof(gchar) * 10);
    g_snprintf(stmp, 9, "%d", i);
  } else
    stmp = g_strdup (tmp);

  g_array_insert_val (d->rowlab, data->current_record, stmp);

  setColor(attrs, data, i);
  setGlyph(attrs, data, i);
  setHidden(attrs, data, i);

/*
 * Probably something's missing here:  if any record has an
 * id, then does every record need one?  I think so.  -- dfs
*/
 
  tmp = getAttribute(attrs, "id");
  if(tmp) {
    if (data->rowIds == NULL) {
/*-- dfs;  when should data->rowIds be freed?  in endXMLElement? --*/
     data->rowIds = (gchar **) g_malloc(d->nrows * sizeof(gchar *));
     memset(data->rowIds, '\0', d->nrows);
    }
    if (d->rowid.id.nels == 0) {
      rowids_alloc (d);
    }

    data->rowIds[i] = g_strdup(tmp);
    d->rowid.id.els[i] = strToInteger (tmp);
  }

/*
 * Probably something's missing here:  if edges should be
 * present, then every record should have a source and an
 * endpoint, and there's no validation going on now. --dfs
*/

  /* Read the edge source and destination pair if, present. */
  tmp = getAttribute(attrs, "source");   
  if (tmp != (const gchar *) NULL) {
    start = strToInteger(tmp);
    tmp = getAttribute(attrs, "destination");   
    if (tmp != (const gchar *) NULL) {
      end = strToInteger(tmp);

      /*-- if encountering the first edge, allocate endpoints array --*/
      if (d->edge.n == 0) {
        edges_alloc (d->nrows, d);
      }

      d->edge.endpoints[i].a = start;
      d->edge.endpoints[i].b = end;
    }
  }

  return(true);
}


