#include "ggobi.h"

#include <string.h>

#ifdef USE_GNOME_XML
#include <gnome-xml/parser.h>
#include <gnome-xml/tree.h>
#else
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

#include "read_init.h"
#include "read_xml.h"

/*
#if USE_XML == 1
extern int xmlDoValidityCheckingDefaultValue;
#endif
*/

colorschemed *process_colorscheme(xmlNodePtr root, xmlDocPtr doc);
colorscaletype getColorSchemeType(const xmlChar *type);
colorsystem getColorSchemeSystem(const xmlChar *type);

gint getForegroundColor(gint index, xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
void getForegroundColors(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
gint getAnnotationColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
gint getBackgroundColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
gint getColor(xmlNodePtr node, xmlDocPtr doc, gfloat **original, GdkColor *col);

colorschemed *
read_colorscheme(gchar *fileName, GList **list)
{
  xmlDocPtr  doc;
  xmlNodePtr node;
  colorschemed *scheme;

  if(!canRead(fileName) && !(strncmp("http", fileName, 4) == 0 || strncmp("ftp", fileName, 3) == 0))
      return(NULL);

/*  xmlSubstituteEntitiesDefault(1);    */
  doc = xmlParseFile(fileName); 
  if(doc == NULL)
      return(NULL);

   /* If this is a colormaps archive, then process each one individually. */

  node = xmlDocGetRootElement(doc);
  if(strcmp((char *)node->name, "colormap") == 0) {
      scheme = process_colorscheme(node, doc);
      if(list) {
        *list = g_list_append(*list, scheme);
      }
      return(scheme);
  }

  node = XML_CHILDREN(node); 
  while(node) {
    if(node->type != XML_TEXT_NODE && node->type != XML_COMMENT_NODE) {
      scheme = process_colorscheme(node, doc);
      if(list)
        *list = g_list_append(*list, scheme);
      }

      node = node->next;
  }
  xmlFreeDoc(doc);

  return(scheme);
}

colorschemed *
alloc_colorscheme()
{
  colorschemed *scheme;

  scheme = (colorschemed*) g_malloc(sizeof(colorschemed));
  memset(scheme, '\0', sizeof(colorschemed));

  scheme->rgb = NULL;
  scheme->rgb_bg.pixel = -1;
  scheme->rgb_accent.pixel = -1;
  scheme->colorNames = g_array_new(false, false, sizeof(gchar *));

  return(scheme);
}

colorschemed *
process_colorscheme(xmlNodePtr root, xmlDocPtr doc)
{
  colorschemed *scheme;
  xmlNodePtr node;
  const xmlChar *tmp;
  xmlChar *val;

  scheme = alloc_colorscheme();

  scheme->name = g_strdup((gchar *) xmlGetProp(root, (xmlChar *) "name"));
  scheme->type =  getColorSchemeType(xmlGetProp(root, (xmlChar *) "type"));
  scheme->system = getColorSchemeSystem(xmlGetProp(root, (xmlChar *) "system"));

/*
  scheme->system_min = 0.0;
  tmp = xmlGetProp(root, "system_min");
  if(tmp)
    scheme->system_min = (gfloat) asNumber(tmp);
  scheme->system_max = 1.0;
  tmp = xmlGetProp(root, "system_max");
  if(tmp)
    scheme->system_max = (gfloat) asNumber(tmp);
*/

  tmp = xmlGetProp(root, (xmlChar *) "criticalvalue");
  if(tmp)
    scheme->criticalvalue = (gint) asNumber((char *)tmp);

  tmp = xmlGetProp(root, (xmlChar *) "ncolors");
  if(tmp)
    scheme->n = (gint) asNumber((char *)tmp);

  node = getXMLElement(root, "description");
  val = xmlNodeListGetString(doc, XML_CHILDREN(node), 1);
  scheme->description = g_strdup((gchar *) val);
  g_free (val);

  node = getXMLElement(root, "foreground");
  getForegroundColors(node, doc, scheme);

  node = getXMLElement(root, "background");
  if(node)
    node = getXMLElement(node, "color");
  getBackgroundColor(node, doc, scheme);

  node = getXMLElement(root, "annotations");
  if(node)
    node = getXMLElement(node, "color");
  getAnnotationColor(node, doc, scheme);

  return(scheme);
}

colorscaletype
getColorSchemeType(const xmlChar *type)
{
  if(strcmp((char *) type, "diverging") == 0)
    return(diverging);
  else if(strcmp((char *) type, "sequential") == 0) 
    return(sequential);
  else if(strcmp((char *) type, "spectral") == 0) 
    return(spectral);
  else if(strcmp((char *) type, "qualitative") == 0) 
    return(qualitative);
  else 
    return(UNKNOWN_COLOR_TYPE);
}

colorsystem
getColorSchemeSystem(const xmlChar *type)
{
  if(strcmp((char *) type, "rgb") == 0)
    return(rgb);
  else if(strcmp((char *) type, "hsv") == 0) 
    return(hsv);
  else if(strcmp((char *) type, "cmy") == 0) 
    return(cmy);
  else if(strcmp((char *) type, "cmyk") == 0) 
    return(cmyk);
  else 
    return(UNKNOWN_COLOR_SYSTEM);
}

/**
  Read the foreground colors node, processing each of the 
  colors using getForegroundColor().
 */
void
getForegroundColors(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
  gint n = 0;
  xmlNodePtr tmp;
  tmp = XML_CHILDREN(node);
  while(tmp) {
    if(tmp->type != XML_TEXT_NODE)
      n++;
    tmp = tmp->next;
  }

  scheme->n = n;
  scheme->data= (gfloat**) g_malloc(n * sizeof(gfloat*));
  scheme->rgb = (GdkColor*) g_malloc(n * sizeof(GdkColor));

  tmp = XML_CHILDREN(node);
  n = 0;
  while(tmp) {
    if(tmp->type != XML_TEXT_NODE) {
      getForegroundColor(n, tmp, doc, scheme);
      n++;
    }
    tmp = tmp->next;
  }
}


gint
getForegroundColor(gint index, xmlNodePtr node, xmlDocPtr doc,
  colorschemed *scheme)
{
    gint value;
    gchar *name;
    xmlChar* ptr;
    value = getColor(node, doc, &(scheme->data[index]), &scheme->rgb[index]);

    ptr =  xmlGetProp(node, (xmlChar *) "name");
    {
      gchar *tmp;
      tmp = name = (gchar *) g_malloc(sizeof(gchar) * (strlen(ptr) + 1));
      while(ptr[0]) {
	  *tmp++ = *ptr++;
      }
      tmp[0] = '\0';
    }

    g_array_append_val(scheme->colorNames, name);

    return(value);
}

gint
getBackgroundColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
  return(getColor(node, doc, &scheme->bg, &scheme->rgb_bg));
}

gint
getAnnotationColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
  return(getColor(node, doc, &scheme->accent, &scheme->rgb_accent));
}

/**
 Read a color of the form
  <color><element></element><element></element>....<element></element></color>
 Puts the actual values into original and fills in the RGB settings for `col'.
 */
gint 
getColor(xmlNodePtr node, xmlDocPtr doc, gfloat **original, GdkColor *col)
{
  xmlNodePtr tmp;
  gint i = 0, numElements = 3; /* RGB only at present. */
  gfloat *vals;
  gfloat colorsystem_min = 0.0;
  gfloat colorsystem_max = 1.0;
  gfloat max = 65535;

  /*-- color values must be scaled onto [0,65535] --*/
  gchar *tmpVal;
  tmpVal = (gchar *) xmlGetProp(node, (xmlChar *) "min");
  if(tmpVal) {
     colorsystem_min /= asNumber(tmpVal);
  }
  tmpVal = (gchar *) xmlGetProp(node, (xmlChar *) "max");
  if(tmpVal) {
     colorsystem_max /= asNumber(tmpVal);
  }

  tmp = XML_CHILDREN(node);

  vals = (gfloat *) g_malloc(3 * sizeof(gfloat));
  while(tmp) {
    xmlChar *val;
    if(tmp->type != XML_TEXT_NODE) {
      val = xmlNodeListGetString(doc, XML_CHILDREN(tmp), 1);
      vals[i] = asNumber((char *)val);
      g_free (val);
      i++;
    }
    tmp = tmp->next;
  }
  if(original)
    *original = vals;

  /*-- scale onto [0,1] --*/
  for (i=0; i<3; i++)
    vals[i] = (vals[i] - colorsystem_min)/(colorsystem_max - colorsystem_min);
  
  /*-- scale onto [0,65535] --*/
  col->red = (guint16) (max * vals[0]);
  col->green = (guint16) (max * vals[1]);
  col->blue = (guint16) (max * vals[2]);

  return(numElements);
}


