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


void startXMLElement(void *user_data, const CHAR *name, const CHAR **attrs);
void endXMLElement(void *user_data, const CHAR *name);
void Characters(void *user_data, const CHAR *ch, int len);


const gchar *XMLSuffixes[] = {"", ".xml", ".xml.gz", ".xmlz"};

const gchar * const xmlDataTagNames[] = {
                                          "ggobidata",
                                          "data",
                                          "description",
                                          "record",
                                          "records",
                                          "variables",
                                          "variable",
                                          "data",

                                          "edges",
                                          "edge",
                                          "edgerecord",
                                          "edgerecords",
                                          "edgevariables",
                                          "edgevariable",

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
data_xml_read (InputDescription *desc, ggobid *gg)
{
 xmlSAXHandlerPtr xmlParserHandler;
 xmlParserCtxtPtr ctx = (xmlParserCtxtPtr) g_malloc(sizeof(xmlParserCtxtPtr));
 XMLParserData data;
 gboolean ok = false;  
 gchar *name = g_strdup(desc->fileName); /* find_xml_file(desc->fileName, NULL, gg); */
  
  if(name == NULL)
    return(false);

 if(strcmp(name, desc->fileName) != 0) {
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

  initParserData(&data, xmlParserHandler, gg);

  ctx = xmlCreateFileParserCtxt(name);
  if(ctx == NULL) {
   xml_warning("File error:", name, "Can't open file ", &data);
   return(false);
  }

  ctx->userData = &data;
  data.input = desc;
  ctx->sax = xmlParserHandler;

  xmlParseDocument(ctx);

  ctx->sax = NULL;
  xmlFreeParserCtxt(ctx);

  g_free(xmlParserHandler);


  {
    GSList *l;
    datad *d;
    ok = true;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      ok &= (d->ncols > 0 && d->nrows > 0);
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
  data->current_edge = 0;

  data->current_edgevariable = 0;
  data->current_edgerecord = 0;
  data->current_edgeelement = 0;

  data->current_data = NULL;

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
  data->defaults.edgeWidth = -1;
  data->defaults.edgeColor = -1;
  data->defaults.edgeHidden = false;
  data->defaults.hidden = false;
}

void 
startXMLElement(void *user_data, const CHAR *name, const CHAR **attrs)
{
 XMLParserData *data = (XMLParserData*)user_data;
 enum xmlDataState type = tagType(name, false);

 data->state = type;

 switch (type) {
   case VARIABLES:
     allocVariables (attrs, data);
     break;
   case VARIABLE:
     newVariable (attrs, data);
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
     data->current_edge++;
     break;

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
     data = NULL; /* just any code so we can stop.*/
     break;
 }
}


XmlTagType
tagType(const CHAR *name, gboolean endTag)
{
 int n = sizeof(xmlDataTagNames)/sizeof(xmlDataTagNames)[0] - 1; 
 int i;
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
Characters(void *user_data, const CHAR *ch, int len)
{
 char *tmp;
 int dlen = len;
 const CHAR *c;
 XMLParserData *data = (XMLParserData*)user_data;

 c = (const CHAR *) skipWhiteSpace(ch, &dlen);
 if(dlen < 1 || c[0] == '\n')
  return;

 if(data->terminateStrings_p) {
  tmp = (char *) g_malloc(sizeof(char)*(dlen+1));

  memcpy(tmp, c, dlen);
  memset(tmp+dlen, '\0', 1);

  c = (const CHAR *) tmp;
 }


 switch(data->state) {
   case RECORD:
     setRecordValues (data, c, dlen);
   break;
   case VARIABLE:
     setVariableName (data, c, dlen);
   break;
   case COLOR:
     setColorValue (data, c, dlen);
   default:
     break;

 }

 if(data->terminateStrings_p) {
  g_free(tmp);
 }
}

const CHAR *
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


/**
  Called in response to a ggobidata tag which contains
  the different datasets. The count element here is the
  number of datasets to expect.
 */
gboolean
setGeneralInfo (const CHAR **attrs, XMLParserData *data)
{
  const char *tmp = getAttribute(attrs, "count");

  if (tmp != NULL) {
    data->expectedDatasetCount = asInteger(tmp);
  }

  return(true);
}


gboolean
setDatasetInfo (const CHAR **attrs, XMLParserData *data)
{
  const char *tmp = getAttribute(attrs, "count");
  datad *d = getCurrentXMLData(data);

  if (tmp == NULL) {
    g_printerr ("No count attribute\n");
    exit(101);
  }

  d->nrows = asInteger(tmp);
  d->nrows_in_plot = d->nrows;  /*-- for now --*/
  d->nrgroups = 0;              /*-- for now --*/

  rowlabels_alloc (d, data->gg);
  rowids_alloc (d, data->gg);  /* dfs */
  br_glyph_ids_alloc (d);
  br_glyph_ids_init (d, data->gg);

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

/*
 * I have just realized that this is wrong.  EdgeData doesn't
 * have edges!  It's linked to other data that <does> have
 * edges.  What a gaffe.
*/
  if(d->edgeData) {
    /*alloc_edgeIDs(d);*/
    edges_alloc (d->nrows, d);  /* dfs */
  }


  /*
  setLineColor (attrs, data, -1);
  setLineWidth (attrs, data, -1);  
  */

  return (true);
}

gboolean
setDefaultDatasetValues(const CHAR **attrs, XMLParserData *data)
{

 const gchar * tmp = getAttribute(attrs, "missingValue");
  if(tmp != NULL) {
    data->NA_identifier = g_strdup(tmp);
  }

  setGlyph (attrs, data, -1);  
  setColor (attrs, data, -1);
  setHidden (attrs, data, -1, ROW);
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
  if(strcmp(name, (const char *)tmp[0]) == 0)
      return((const gchar *)tmp[1]);
   tmp += 2;
 }

 return(NULL);
}

gboolean 
newRecord(const CHAR **attrs, XMLParserData *data)
{
  datad *d = getCurrentXMLData(data);

  d->readXMLRecord(attrs, data);

 return(true);
}

gboolean
setHidden(const CHAR **attrs, XMLParserData *data, int i, enum HiddenType type)
{
  const char *tmp;
  datad *d = getCurrentXMLData(data);

  tmp = getAttribute(attrs, "hidden");
  if(tmp) {
    gboolean hidden = asLogical(tmp);


    if (i < 0) {
     if (type == ROW)
       data->defaults.hidden = hidden;
     else {
       data->defaults.edgeHidden = hidden;
     }     
    } else
     if(type == ROW)
       d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i] = hidden;
     else {
       d->edge.hidden.els[i] = d->edge.hidden_now.els[i] =
         d->edge.hidden_prev.els[i] = hidden;
     }
  }

 return(tmp != NULL);
}

gboolean
asLogical(const gchar *sval)
{
 unsigned int i;
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
  const gchar *tmp;
  gint value = data->defaults.color;
  datad *d = getCurrentXMLData(data);

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
     d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] = value;
  }

 return (value != -1);
}

gboolean
setGlyph(const CHAR **attrs, XMLParserData *data, gint i)
{
  const gchar *tmp;
  gint value;
  datad *d = getCurrentXMLData(data);

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
     d->glyph[i].size = d->glyph_now[i].size 
             = d->glyph_prev[i].size = value;
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
     d->glyph[i].type = d->glyph_now[i].type = 
           d->glyph_prev[i].type = value;
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
            d->glyph[i].type = d->glyph_now[i].type =
              d->glyph_prev[i].type = value;       
      } else {
        value = atoi(next);
        if(i < 0)
          data->defaults.glyphSize = value;
        else
          d->glyph[i].size = d->glyph_now[i].size =
            d->glyph_prev[i].size = value;     
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
setRecordValues (XMLParserData *data, const CHAR *line, gint len)
{
  gdouble value;
  const gchar *tmp = strtok((gchar*) line, " \t\n");
  datad *d = getCurrentXMLData(data);

  while (tmp) {
    value = asNumber (tmp);
    d->raw.vals[data->current_record][data->current_element++] = value;
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
  const gchar *tmp;
  datad *d = getCurrentXMLData(data);

  tmp = getAttribute(attrs, "transformName");
  if (tmp) {
    data->variable_transform_name_as_attribute = true;

    d->vartable[data->current_variable].collab_tform =  g_strdup(tmp);
  }

 tmp = getAttribute(attrs, "name");
 if(tmp != NULL) {
  d->vartable[data->current_variable].collab = g_strdup(tmp);
  if (data->variable_transform_name_as_attribute == false)
    d->vartable[data->current_variable].collab_tform = g_strdup(tmp);
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
allocVariables (const CHAR **attrs, XMLParserData *data)
{
  const gchar *tmp = getAttribute (attrs, "count");
  datad *d = getCurrentXMLData(data);

  if(tmp == NULL) {
    g_printerr ("No count for variables attribute\n");
    exit(101);
  }

  d->ncols = asInteger(tmp);

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
setVariableName(XMLParserData *data, const CHAR *name, gint len)
{
  gchar *tmp = (gchar *) g_malloc (sizeof(gchar) * (len+1));
  gint j = data->current_variable;
  datad *d = getCurrentXMLData(data);

  tmp[len] = '\0';
  memcpy (tmp, name, len);

  /* Handle the case where we have multiple calls to the characters
     handler for the same variable because the data is split
   */
  if (d->vartable[j].collab != NULL) {
    /* need to append tmp to the existing value.*/
  }

  d->vartable[j].collab = tmp;

  /* Note that if we do have multiple calls to this for the same
     variable then we cannot handle the case where the 
     user does not specify the transformation variable
     unless we use a flag in XMLParserData. This is
     variable_transform_name_as_attribute.
   */
  if (d->vartable[j].collab_tform == NULL) {
    d->vartable[j].collab_tform = g_strdup (tmp);
  }

  return (true);
}




/*------------------ end of edges section ------------------------------*/

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
  tmp =  strrchr(filename, G_DIR_SEPARATOR);
  if(tmp) {
    int n = tmp - filename + 2;
    tmp = (char*) g_malloc(n*sizeof(char));
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
  const char **suffixes = XMLSuffixes;
  int nsuffixes = sizeof(suffixes)/sizeof(suffixes[0]);

  if(dir)
    dirlen = strlen(dir);

    /* If filename starts with a /, so it is an absolute name,
       then ignore the directory argument.
     */
  if(filename[0] == G_DIR_SEPARATOR)
    dirlen = 0;

  for(i = 0; i < nsuffixes;i++) {
    name = (char*) g_malloc(sizeof(char)*(dirlen + strlen(filename)+strlen(suffixes[i]) + 2));
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
    data->gg->colorNames = (gchar **) g_realloc (data->gg->colorNames, data->gg->ncolors * sizeof (gchar *));
    memset(data->gg->colorNames + (data->gg->ncolors-size), '\0', size*sizeof(gchar *));
  } else {
    data->gg->ncolors = size;
    data->gg->default_color_table = (GdkColor *) g_malloc (size * sizeof (GdkColor));
    data->gg->colorNames = (gchar **) g_malloc (size * sizeof (gchar *));
    memset(data->gg->colorNames, '\0', size * sizeof (gchar *));
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
asciiParseColorMap(const gchar *fileName, int size, XMLParserData *data)
{

 return(false);
}


gboolean
setDataset(const CHAR **attrs, XMLParserData *parserData) 
{
  datad *data;
  const gchar *tmp;
  gchar *name;
  tmp = getAttribute(attrs, (char*) "nodeData");

#ifdef USE_CLASSES
  if(tmp) {
    data = new EdgeDatad(1);
    //    data = new edgeDatad(parserData->gg);
  } else 
    data = new datad(parserData->gg);
#else
  data = datad_new(NULL, parserData->gg);
  if(tmp) {
    data->readXMLRecord = readXMLEdgeRecord;
    data->edgeData = true;
  } else {
    data->readXMLRecord = readXMLRecord;
    data->edgeData = false;
  }
#endif


  tmp = getAttribute(attrs, (char *) "name");
  if(tmp == NULL) {
    name = (gchar *) malloc(sizeof(gchar)*8);
    sprintf(name, "data%d", (int) g_slist_length(parserData->gg->d));
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

#ifndef USE_CLASSES
gboolean
readXMLRecord(const CHAR **attrs, XMLParserData *data)
{
  datad *d = getCurrentXMLData(data);
  const gchar *tmp;
  gchar *stmp;
  gint i = data->current_record;

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
  setHidden(attrs, data, i, EDGE);
 
  tmp = getAttribute(attrs, "id");
  if(tmp) {
    if (data->rowIds == NULL) {
     data->rowIds = (gchar **) g_malloc(d->nrows * sizeof(gchar *));
     memset(data->rowIds, '\0', d->nrows);
    }

    data->rowIds[i] = g_strdup(tmp);

    d->rowid.id.els[i] = asInteger (tmp);
  } else {

    d->rowid.id.els[i] = i;

  }

  return(true);
}

gboolean
readXMLEdgeRecord(const CHAR **attrs, XMLParserData *data)
{
  const gchar *tmp;
  gint index = data->current_record;
  datad *d = getCurrentXMLData(data);
  gint start, end;

  gboolean ans = readXMLRecord(attrs, data);

    /* Now read the node source and destination pair. */
  tmp = getAttribute(attrs, "source");   
  if(tmp == (const gchar *)NULL || tmp[0] == (const gchar)NULL) {
    char buf[512]; 
    sprintf(buf,"No source attribute for record %d in edge data %s\n",index, d->name);
    g_printerr (buf);
    exit(103);
  }
  start = asInteger(tmp);

  tmp = getAttribute(attrs, "destination");   
  if(tmp == (const gchar *)NULL || tmp[0] == (const gchar)NULL) {
    char buf[512]; 
    sprintf(buf,"No destination attribute for record %d in edge data %s\n",index, d->name);
    g_printerr (buf);
    exit(103);
  }
  end = asInteger(tmp);

  if (start > end) {
    d->edge.endpoints[index].a = end;
    d->edge.endpoints[index].b = start;
  } else {
    d->edge.endpoints[index].a = start;
    d->edge.endpoints[index].b = end;
  }

/*
  d->sourceid.id.els[index] = asInteger(tmp);
  d->destid.id.els[index] = asInteger(tmp);
*/
  return(ans);
}

/*
gint
alloc_edgeIDs(datad *d)
{
  size_t sz = d->nrows * sizeof(guint);
  d->sourceID = (guint *) g_malloc(sz);
  memset(d->sourceID, '\0', sz);
  d->destinationID = (guint *) g_malloc(sz);
  memset(d->destinationID, '\0', sz);
 return(d->nrows);
}
*/

#else /* So using classes */
gboolean
datad::readXMLRecord(const CHAR **attrs, XMLParserData *data)
{
  const gchar *tmp;
  gint i = data->current_record;

  data->current_element = 0;

  tmp = getAttribute(attrs, "label");
  if (tmp) {
    gchar *stmp = g_strdup (tmp);
    g_array_insert_val (rowlab, data->current_record, stmp);
  }


  setColor(attrs, data, i);
  setGlyph(attrs, data, i);
  setHidden(attrs, data, i, EDGE);
 
  tmp = getAttribute(attrs, "id");
  if(tmp) {
    if(data->rowIds == NULL) {
     data->rowIds = (gchar **) g_malloc(nrows * sizeof(gchar *));
     memset(data->rowIds, '\0', nrows);
    }

    data->rowIds[i] = g_strdup(tmp);
  }

  return(true);
}


gboolean
EdgeDatad::readXMLRecord(const CHAR **attrs, XMLParserData *data)
{
  datad::readXMLRecord(attrs, data);

  return(true);
}
#endif
