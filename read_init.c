/*
  Reads initialization file.
 */
#include "read_init.h"

#if USE_XML == 1
extern int xmlDoValidityCheckingDefaultValue;
#endif


#ifdef USE_GNOME_XML
#include <gnome-xml/parser.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parserInternals.h> /* for xmlDoValidityChecking */
#else
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/parserInternals.h> /* for xmlDoValidityChecking */
#endif

#include <string.h>

#include "plugin.h"
#include "GGobiAPI.h"

#include "externs.h"

gint getPreviousFiles(const xmlDocPtr doc, GGobiInitInfo *info);
DataMode getPreviousInput(xmlNode *node, InputDescription *input);
DataMode getInputType(xmlNode *node);

void getPlugins(xmlDocPtr doc, GGobiInitInfo *info);
GGobiPluginInfo *processPlugin(xmlNodePtr node, GGobiInitInfo *info, xmlDocPtr doc);


gint getPreviousGGobiDisplays(const xmlDocPtr doc, GGobiInitInfo *info);
gint getPreviousDisplays(xmlNodePtr node, GGobiDescription *desc);
GGobiDisplayDescription* getDisplayDescription(xmlNodePtr node);
enum displaytyped getDisplayType(const xmlChar *type);
gint getPreferences(const xmlDocPtr doc, GGobiInitInfo *info);

#include "colorscheme.h"

GGobiInitInfo *
read_init_file(const char *filename)
{
  xmlDocPtr  doc;
  GGobiInitInfo *info;
  int oldValiditySetting = xmlDoValidityCheckingDefaultValue;

  xmlSubstituteEntitiesDefault(1);   
  xmlDoValidityCheckingDefaultValue = false;
  filename = g_strdup(filename);
  doc = xmlParseFile(filename); 
  if(doc == NULL)
/**/return((GGobiInitInfo *) NULL);
  info = g_malloc(sizeof(GGobiInitInfo));

  info->numInputs = 0;
  info->descriptions = NULL;
  info->filename = g_strdup(filename);

  getPreferences(doc, info);
  getPreviousFiles(doc, info);
  getPreviousGGobiDisplays(doc, info);
#if SUPPORT_PLUGINS
  getPlugins(doc, info);
#endif

  xmlDoValidityCheckingDefaultValue = oldValiditySetting;

  return(info);
}

    /* Find a node with a secified tag Name */
xmlNode *
getXMLDocElement(const xmlDocPtr doc, const char *tagName)
{
  xmlNode *node = xmlDocGetRootElement(doc);
  return(getXMLElement(node, tagName));
}

xmlNode *
getXMLElement(xmlNodePtr node, const char *tagName)
{
  node = XML_CHILDREN(node);
  while(node) {
    if(strcmp(node->name, tagName) == 0) {
      return(node);
    }
    node = node->next;
  }

  return(node);
}

gint
getPreferences(const xmlDocPtr doc, GGobiInitInfo *info)
{
  xmlNode *node, *el;
  gint n, i;
  node = getXMLDocElement(doc, "preferences");

  el = getXMLElement(node, "colorschemes");
  if(el) {
      char *tmp;
      tmp = xmlGetProp(el, "file");
      info->colorSchemeFile = g_strdup(tmp);
  }

  el = getXMLElement(node, "background");
  if(el) {
      char *tmp;
      el = getXMLElement(el, "color");
      info->bgColor = (GdkColor *) g_malloc(sizeof(GdkColor));
      getColor(el, doc, NULL, info->bgColor);
      if(gdk_colormap_alloc_color(gdk_colormap_get_system(), info->bgColor, false, true) == false)
	  fprintf(stderr, "Can't allocate background color\n"); fflush(stderr);
  }

  return(0);
}

gint
getPreviousFiles(const xmlDocPtr doc, GGobiInitInfo *info)
{
  xmlNode *node, *el;
  gint n, i;
  node = getXMLDocElement(doc, "previousFiles");

  n = 0;
  el = XML_CHILDREN(node);
  while(el) {
    if(el->type != XML_TEXT_NODE)
       n++;
     el = el->next;
  }

  info->descriptions = g_malloc(n*sizeof(GGobiDescription));
  info->numInputs = n;

  el = XML_CHILDREN(node);
  for(i = 0; el ; el = el->next) {

/* */
    if(el->type != XML_TEXT_NODE) {
/*
 * dfs; trying to get past my compiler
*/
    /*memset((void*) info->descriptions+i, '\0', sizeof(GGobiDescription));*/
      memset(info->descriptions+i, '\0', sizeof(GGobiDescription)); 
      getPreviousInput(el, &(info->descriptions[i].input));
      i++;
    }
  }
  return(n);
}


DataMode
getPreviousInput(xmlNode *node, InputDescription *input)
{
   const gchar *tmp;
   DataMode mode = getInputType(node);
   input->mode = mode;
   if((tmp = xmlGetProp(node, "name"))) {
     input->fileName = g_strdup(tmp);
   } else 
     input->fileName = NULL;


   /* This shold be connected to 
        completeFileDesc(input->fileName, input);
   */
   if(input->fileName) {
     gchar *ptr, *tmp1, *tmp2 = NULL;
     gint i;
     tmp1 = strrchr(input->fileName, G_DIR_SEPARATOR);
     if(tmp1) {
       tmp2 = strrchr(tmp1, '.');
       if(tmp2)
         input->givenExtension = g_strdup(tmp2+1);
       input->baseName = g_malloc((tmp2 - tmp1 +1)*sizeof(gchar));
       for(i = 0, ptr = tmp1 + 1 ; ptr < tmp2; ptr++, i++) {   
         input->baseName[i] = *ptr;
       }
       input->baseName[i] = '\0';
       input->dirName = g_malloc((tmp1 - input->fileName +1)*sizeof(gchar));
       for(i=0, ptr = input->fileName; ptr < tmp1; ptr++, i++) {   
         input->dirName[i] = *ptr;
       }
       input->dirName[i] = '\0';
     } else {
       input->fileName = NULL;
       input->dirName = NULL;
       input->baseName = NULL;
     }
   }

   input->canVerify = 0;

   return(mode);
}

DataMode
getInputType(xmlNode *node)
{
  const xmlChar *tag;
  const xmlChar *mode;
  DataMode val = unknown_data;

  tag = node->name;

  if(strcmp(tag,"url") == 0) {
    val = url_data;
  } else if(strcmp(tag,"database") == 0)
    val = mysql_data;
  else {
    mode = xmlGetProp(node, "mode");
    if(strcmp(tag,"file") == 0) {
      if(strcmp(mode, "xml") == 0)
        val = xml_data; 
      else if(strcmp(mode, "ascii") == 0)
        val = ascii_data; 
    }
  }
    
  return(val);
}

/*****************************************************************/

gint
getPreviousGGobiDisplays(const xmlDocPtr doc, GGobiInitInfo *info)
{
  xmlNode *node, *el;
  GGobiDescription *desc = NULL;
  gint i;
  node = getXMLDocElement(doc, "ggobis"); 
  el = XML_CHILDREN(node);
  i = 0;
  while(el) {
    if(el->type != XML_TEXT_NODE && strcmp(el->name,"ggobi") == 0) {
        /* Need to match these with the input source ids. */
      desc = info->descriptions + i;
      getPreviousDisplays(el, desc);
      i++;
    }
    el = el->next;
  }

  if(!desc)
    return(-1);

  return(g_list_length(desc->displays));
}

gint
getPreviousDisplays(xmlNodePtr node, GGobiDescription *desc)
{
  xmlNodePtr el = XML_CHILDREN(node);
  GGobiDisplayDescription *dpy;
  gint n = 0;
  desc->displays = NULL;
  while(el) {
    if(el->type != XML_TEXT_NODE && strcmp(el->name, "display") == 0) {
      dpy = getDisplayDescription(el) ;
      if(dpy) {
        desc->displays = g_list_append(desc->displays, dpy);
        n++;
      }    
    }

    el = el->next;
  }

  return(n);
}

GGobiDisplayDescription*
getDisplayDescription(xmlNodePtr node)
{
  GGobiDisplayDescription *dpy;
  xmlNodePtr el;
  gint i;
  xmlChar *tmp;

  dpy = (GGobiDisplayDescription*) g_malloc(sizeof(GGobiDisplayDescription*));
  dpy->type = getDisplayType(xmlGetProp(node, "type"));
  tmp = xmlGetProp(node, "data");
  if(tmp) {
    dpy->data = strToInteger(tmp) - 1;
  } else
    dpy->data = 0;

  dpy->numVars = 0;
 
  if(dpy->type == unknown_display_type) {
   return(dpy);
  }

  el = XML_CHILDREN(node);
  while(el) {
    if(el->type != XML_TEXT_NODE && strcmp(el->name, "variable") == 0) 
      dpy->numVars++;
    el = el->next;
  }

  dpy->varNames = (gchar **) g_malloc(dpy->numVars*sizeof(gchar*));
  for(i = 0, el = XML_CHILDREN(node); i < dpy->numVars; el = el->next) {
    if(el->type != XML_TEXT_NODE && strcmp(el->name, "variable") == 0) 
      dpy->varNames[i++] = g_strdup(xmlGetProp(el, "name"));
  }

  return(dpy);
}

enum displaytyped
getDisplayType(const xmlChar *type)
{
  enum displaytyped val = unknown_display_type;
  if(strcmp(type, "scatterplot") == 0) 
    val = scatterplot;
  else if(strcmp(type, "scatmatrix") == 0)
    val = scatmat;
  else if(strcmp(type, "parcoords") == 0)
    val = parcoords;
  else if(strcmp(type,"tsplot") == 0)
    val = tsplot;

  return(val);
}


/*****************************************************************/

#if SUPPORT_PLUGINS
/*
 Handle the plugins section, looping over each <plugin>
 tag and passing it processPlugin().
*/

void
getPlugins(xmlDocPtr doc, GGobiInitInfo *info)
{
  xmlNode *node, *el;
  GGobiPluginInfo *plugin;

  node = getXMLDocElement(doc, "plugins");

  info->plugins = NULL;

  if(node == NULL)  
      return;

  el = XML_CHILDREN(node);
  while(el) {
    if(el->type != XML_TEXT_NODE && strcmp(el->name, "plugin") == 0) {
      plugin = processPlugin(el, info, doc);
      if(plugin)
        info->plugins = g_list_append(info->plugins, plugin);
    }
    el = el->next;
  }
}

/*
  This handles the details of a <plugin> tag,
  reading the description, author, etc.
 */

#define GET_PROP_VALUE(field,name) plugin->field = ((tmp = (char *) xmlGetProp(c, name)) != NULL) ? g_strdup(tmp) : NULL

GGobiPluginInfo *
processPlugin(xmlNodePtr node, GGobiInitInfo *info, xmlDocPtr doc)
{
  xmlNodePtr el;
  gboolean load;
  const xmlChar *tmp;
  GGobiPluginInfo *plugin;
  xmlChar * val;

  plugin = (GGobiPluginInfo *) g_malloc(sizeof(GGobiPluginInfo));
  memset(plugin, '\0', sizeof(GGobiPluginInfo));
  plugin->onLoad = NULL;
  plugin->onCreate = NULL;
  plugin->onClose = NULL;
  plugin->onUnload = NULL;

  tmp = xmlGetProp(node, "name");
  if(tmp) {
    plugin->name = g_strdup((char *) tmp);
  }

  tmp = xmlGetProp(node, "load");
  if(tmp) {
    load = strcmp((char*)tmp, "immediate") == 0;
  }

  el = XML_CHILDREN(node);
  while(el) {
    if(el->type != XML_TEXT_NODE) {
      if(strcmp(el->name, "author") == 0) {
        val = xmlNodeListGetString(doc, XML_CHILDREN(el), 1);
        plugin->author = g_strdup((char*) val);
      } else if(strcmp(el->name, "description") == 0) {
        val = xmlNodeListGetString(doc, XML_CHILDREN(el), 1);
        plugin->description = g_strdup((char*) val);
      } else if(strcmp(el->name, "dll") == 0) {
        plugin->dllName = g_strdup((char*) xmlGetProp(el, "name"));
        if(XML_CHILDREN(el)) {
          xmlNodePtr c = XML_CHILDREN(el);
          while(c) {
            if(el->type != XML_TEXT_NODE && strcmp(c->name, "init") == 0) {
               GET_PROP_VALUE(onLoad, "onLoad");
               GET_PROP_VALUE(onCreate, "onCreate");
               GET_PROP_VALUE(onClose, "onClose");
               GET_PROP_VALUE(onUnload, "onUnload");
               GET_PROP_VALUE(onUpdateDisplay, "onUpdateDisplayMenu");
               break;
            }
            c = c->next;
          }
        }
      }
    }

    el = el->next;
 }

  if(load) {
    plugin->library = load_plugin_library(plugin);
    plugin->loaded = (plugin->library != NULL);

    if(plugin->loaded && plugin->onLoad) {
      OnLoad f = (OnLoad) getPluginSymbol(plugin->onLoad, plugin);
      if(f) {
        f(0, plugin);
      } else {
        gchar buf[1000];
        dynload->getError(buf, plugin);
        fprintf(stderr, "error on loading plugin library %s: %s\n",
                plugin->dllName, buf);fflush(stderr);
      }
    }
  }

  return(plugin);
}

#endif /* end of SUPPORT_PLUGINS */

gint resolveVariableName(const gchar *name, datad *d);

displayd *
createDisplayFromDescription(ggobid *gg, GGobiDisplayDescription *desc) 
{
  displayd *dpy;
  datad *data;
  gint *vars, i;

  data = (datad*) gg->d->data;

  vars = (gint*) g_malloc(sizeof(gint) * desc->numVars);
  for (i = 0; i < desc->numVars; i++)
    vars[i] = resolveVariableName(desc->varNames[i], data);
  switch(desc->type) {
    case scatterplot:
      dpy = GGOBI(newScatterplot)(vars[0], vars[1], data, gg);
    break;
    case parcoords:
      dpy = GGOBI(newParCoords)(vars, desc->numVars, data, gg);
    break;
    case scatmat:
      dpy = GGOBI(newScatmat)(vars, vars, desc->numVars,
                              desc->numVars, data, gg);
    break;
    case tsplot:
      dpy = GGOBI(newTimeSeries)(vars, desc->numVars, data, gg);
    break;
    case unknown_display_type:
    default:
      dpy = NULL;
    break;
  }

  g_free(vars);

  return(dpy);
}

gint
resolveVariableName(const char *name, datad *d)
{ 
  gint j;
  vartabled *vt;

  for (j = 0; j < d->ncols; j++) {
    vt = vartable_element_get (j, d);
    if (strcmp (vt->collab, name) == 0)
      return(j);
  }

  return(-1);
}
