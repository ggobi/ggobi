#include "ggobi.h"
#include "colorscheme.h"

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

int getForegroundColor(int index, xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
void getForegroundColors(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
int getAnnotationColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
int getBackgroundColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme);
int getColor(xmlNodePtr node, xmlDocPtr doc, float **original, GdkColor *col);

colorschemed *
read_colorscheme(char *fileName, GList **list)
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
    if(node->type != XML_TEXT_NODE) {
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

 tmp = xmlGetProp(root, "criticalvalue");
 if(tmp)
     scheme->criticalvalue = (int) asNumber(tmp);

 tmp = xmlGetProp(root, "ncolors");
 if(tmp)
     scheme->n = (int) asNumber(tmp);

 node = getXMLElement(root, "description");
 scheme->description = g_strdup(xmlNodeListGetString(doc, XML_CHILDREN(node), 1));

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
    return(diverging);
  else if(strcmp(type, "spectral") == 0) 
    return(spectral);
  else if(strcmp(type, "categorical") == 0) 
    return(categorical);
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

int
getForegroundColor(int index, xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
    return(getColor(node, doc, &(scheme->data[index]), &scheme->rgb[index]));
}

int
getBackgroundColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
  return(getColor(node, doc, &scheme->bg, &scheme->rgb_bg));
}

int
getAnnotationColor(xmlNodePtr node, xmlDocPtr doc, colorschemed *scheme)
{
   return(getColor(node, doc, &scheme->accent, &scheme->rgb_accent));
}

/**
 Read a color of the form
  <color><element></element><element></element>....<element></element></color>
 Puts the actual values into original and fills in the RGB settings for `col'.
 */
int 
getColor(xmlNodePtr node, xmlDocPtr doc, float **original, GdkColor *col)
{
  xmlNodePtr tmp;
  int i = 0, numElements = 3; /* RGB only at present. */
  float *vals;
  tmp = XML_CHILDREN(node);

  vals = (float *) g_malloc(3 * sizeof(float));
  while(tmp) {
    xmlChar *val;
    if(tmp->type != XML_TEXT_NODE) {
      val = xmlNodeListGetString(doc, XML_CHILDREN(tmp), 1);
      vals[i] = asNumber(val);
      i++;
    }
    tmp = tmp->next;
  }
  if(original)
    *original = vals;

  col->red = vals[0];
  col->green = vals[1];
  col->blue = vals[2];

  return(numElements);
}


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
