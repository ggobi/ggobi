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

#if USE_XML == 1
extern int xmlDoValidityCheckingDefaultValue;
#endif

colorschemed *process_colorscheme(xmlNodePtr root, xmlDocPtr doc);
colorscaletype getColorSchemeType(const xmlChar *type);
colorsystem getColorSchemeSystem(const xmlChar *type);

gint getForegroundColor(gint index, xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
void getForegroundColors(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
gint getAnnotationColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
gint getBackgroundColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
gint getColor(xmlNodePtr node, xmlDocPtr doc, gfloat **original, GdkColor *col, gfloat min, gfloat max);

colorschemed *
read_colorscheme(gchar *fileName, GList **list)
{
  xmlDocPtr  doc;
  xmlNodePtr node;
  colorschemed *scheme;

/*  xmlSubstituteEntitiesDefault(1);    */
  doc = xmlParseFile(fileName); 
  if(doc == NULL)
      return(NULL);

   /* If this is a colormaps archive, then process each one individually. */

  node = xmlDocGetRootElement(doc);
  if(strcmp(node->name, "colormap") == 0) {
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
process_colorscheme(xmlNodePtr root, xmlDocPtr doc)
{
  colorschemed *scheme;
  xmlNodePtr node;
  const xmlChar *tmp;

  scheme = (colorschemed*) g_malloc(sizeof(colorschemed));

  scheme->name = g_strdup(xmlGetProp(root, "name"));
  scheme->type = getColorSchemeType(xmlGetProp(root, "type"));
  scheme->system = getColorSchemeSystem(xmlGetProp(root, "system"));

  scheme->system_min = 0.0;
  tmp = xmlGetProp(root, "system_min");
  if(tmp)
    scheme->system_min = (gfloat) asNumber(tmp);
  scheme->system_max = 1.0;
  tmp = xmlGetProp(root, "system_max");
  if(tmp)
    scheme->system_max = (gfloat) asNumber(tmp);

  tmp = xmlGetProp(root, "criticalvalue");
  if(tmp)
    scheme->criticalvalue = (gint) asNumber(tmp);

  tmp = xmlGetProp(root, "ncolors");
  if(tmp)
    scheme->n = (gint) asNumber(tmp);

  node = getXMLElement(root, "description");
  scheme->description = g_strdup(xmlNodeListGetString(doc,
                                 XML_CHILDREN(node),
                                 1));

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
  if(strcmp(type, "diverging") == 0)
    return(diverging);
  else if(strcmp(type, "sequential") == 0) 
    return(sequential);
  else if(strcmp(type, "spectral") == 0) 
    return(spectral);
  else if(strcmp(type, "qualitative") == 0) 
    return(qualitative);
  else 
    return(UNKNOWN_COLOR_TYPE);
}

colorsystem
getColorSchemeSystem(const xmlChar *type)
{
  if(strcmp(type, "rgb") == 0)
    return(rgb);
  else if(strcmp(type, "hsv") == 0) 
    return(hsv);
  else if(strcmp(type, "cmy") == 0) 
    return(cmy);
  else if(strcmp(type, "cmyk") == 0) 
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
  return(getColor(node, doc, &(scheme->data[index]), &scheme->rgb[index],
     scheme->system_min, scheme->system_max));
}

gint
getBackgroundColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
  return(getColor(node, doc, &scheme->bg, &scheme->rgb_bg,
     scheme->system_min, scheme->system_max));
}

gint
getAnnotationColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
  return(getColor(node, doc, &scheme->accent, &scheme->rgb_accent,
    scheme->system_min, scheme->system_max));
}

/**
 Read a color of the form
  <color><element></element><element></element>....<element></element></color>
 Puts the actual values into original and fills in the RGB settings for `col'.
 */
gint 
getColor(xmlNodePtr node, xmlDocPtr doc, gfloat **original, GdkColor *col,
 gfloat smin, gfloat smax)
{
  xmlNodePtr tmp;
  gint i = 0, numElements = 3; /* RGB only at present. */
  gfloat *vals;
  gfloat max = 65535;

/* what's this code for?  dfs */
  gchar *tmpVal;
  tmpVal = xmlGetProp(node, "max");
  if(tmpVal) {
     max /= asNumber(tmpVal);
  }

  tmp = XML_CHILDREN(node);

  vals = (gfloat *) g_malloc(3 * sizeof(gfloat));
  while(tmp) {
    xmlChar *val;
    if(tmp->type != XML_TEXT_NODE) {
      val = xmlNodeListGetString(doc, XML_CHILDREN(tmp), 1);
      vals[i] = asNumber(val) * max;
      i++;
    }
    tmp = tmp->next;
  }
  if(original)
    *original = vals;

  col->red = (guint16) ((vals[0] - smin)/(smax - smin));
  col->green = (guint16) ((vals[1] - smin)/(smax - smin));
  col->blue = (guint16) ((vals[2] - smin)/(smax - smin));

  return(numElements);
}


/**
  Find the color scheme element in the list with the specified
  name.
 */
colorschemed *
findColorSchemeByName(GList *schemes, const gchar *name)
{
  colorschemed *s;
  int i, n;

  n = g_list_length(schemes);
  for(i = 0; i < n; i++) {
   s = (colorschemed *)g_list_nth_data(schemes, i);
   if(strcmp(name, s->name) == 0)
       return(s);
  }
  return(NULL);
}
