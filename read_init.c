/*
  Reads initialization file.
 */
#include "read_init.h"
#include "read_xml.h" /* for asNumber() */

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

#ifdef SUPPORT_PLUGINS
#include "plugin.h"
#endif

#include "GGobiAPI.h"

#include "externs.h"

gint     getPreviousFiles(const xmlDocPtr doc, GGobiInitInfo *info);
DataMode getPreviousInput(xmlNode *node, InputDescription *input);
DataMode getInputType(xmlNode *node);

#ifdef SUPPORT_PLUGINS

gboolean getLogicalPreference(xmlNodePtr node, const char *elName, gboolean defaultValue);

void getPlugins(xmlDocPtr doc, GGobiInitInfo *info);
GGobiPluginInfo *processPlugin(xmlNodePtr node, GGobiInitInfo *info, xmlDocPtr doc);
GGobiPluginInfo *processInputPlugin(xmlNodePtr node, GGobiInitInfo *info, xmlDocPtr doc);
void getPluginSymbols(xmlNodePtr node, GGobiPluginInfo *plugin, xmlDocPtr doc, gboolean isLanguage);


void getInputPluginValues(xmlNodePtr node, GGobiInputPluginInfo *plugin, xmlDocPtr doc);
gboolean getPluginDetails(xmlNodePtr node, GGobiPluginDetails *plugin, xmlDocPtr doc);
gboolean loadPluginLibrary(GGobiPluginDetails *plugin, GGobiPluginInfo *realPlugin);

gboolean getPluginLanguage(xmlNodePtr node, GGobiPluginInfo *gplugin,  GGobiPluginType type, GGobiInitInfo *info);
#endif

GHashTable *getPluginNamedOptions(xmlNodePtr node, GGobiPluginDetails *info, xmlDocPtr doc);
GSList *getPluginUnnamedArguments(xmlNodePtr node, GGobiPluginDetails *info, xmlDocPtr doc);
gboolean getPluginOptions(xmlNodePtr node, GGobiPluginDetails *info, xmlDocPtr doc);


GSList *getPluginDependencies(xmlNodePtr node, GGobiPluginDetails *info, xmlDocPtr doc);


gint getPreviousGGobiDisplays(const xmlDocPtr doc, GGobiInitInfo *info);
GGobiDisplayDescription* getDisplayDescription(xmlNodePtr node);
gint getPreferences(const xmlDocPtr doc, GGobiInitInfo *info);


GGobiInitInfo *
read_init_file(const gchar *filename, GGobiInitInfo *info)
{
  xmlDocPtr  doc;
  gchar *fileName;
  gint oldValiditySetting = xmlDoValidityCheckingDefaultValue;

  xmlSubstituteEntitiesDefault(1);   
  xmlDoValidityCheckingDefaultValue = false;
  fileName = g_strdup(filename);
  doc = xmlParseFile(fileName); 
  if(doc == NULL)
      return(info);
  if(info == NULL)
      info = (GGobiInitInfo *) g_malloc(sizeof(GGobiInitInfo));

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
  /* Causes a crash when started with -init notes/ggobirc,
     but not if there is a -colorschemes filename
     g_free(fileName); 
   */

  /* Should release the doc object also. */
  xmlFreeDoc(doc);
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
    if(strcmp((const char *) node->name, tagName) == 0) {
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
  /*gint n, i;*/
  node = getXMLDocElement(doc, "preferences");
  if(!node)
      return(-1);

    /* Don't read this setting if the user has specified a value in
       the command line argument. */
  if(info->colorSchemeFile == NULL) {
    el = getXMLElement(node, "colorschemes");
    if(el) {
      gchar *tmp;
      tmp = (gchar *) xmlGetProp(el, (xmlChar *) "file");
      info->colorSchemeFile = g_strdup(tmp);
    }
  }

  info->bgColor = NULL;  /*-- this needs to be initialized --*/
  el = getXMLElement(node, "background");
  if(el) {
    el = getXMLElement(el, "color");
    if(el) {
      info->bgColor = (GdkColor *) g_malloc(sizeof(GdkColor));
      getColor(el, doc, NULL, info->bgColor);
      if (gdk_colormap_alloc_color(gdk_colormap_get_system(),
           info->bgColor, false, true) == false)
      {
        g_printerr ("Can't allocate background color\n");
      }
    }
  }

  info->fgColor = NULL;  /*-- this needs to be initialized --*/
  el = getXMLElement(node, "foreground");
  if(el) {
    el = getXMLElement(el, "color");
    if(el) {
      info->fgColor = (GdkColor *) g_malloc(sizeof(GdkColor));
      getColor(el, doc, NULL, info->fgColor);
      if (gdk_colormap_alloc_color(gdk_colormap_get_system(),
                 info->fgColor, false, true) == false)
      {
        g_printerr ("Can't allocate foreground color\n");
      }
    }
  }


  el = getXMLElement(node, "glyph");
  if(el) {
    gchar *tmp;

    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "type");
    if(tmp) {
      info->glyph.type = mapGlyphName(tmp);
    }
    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "size");
    if(tmp) {
      info->glyph.size = asNumber(tmp);
    }
  }

  info->createInitialScatterPlot = getLogicalPreference(node, "autoplot", true);
   /* If we autoplot, then we will by default expect there to be
      at least one plot, so our default value for allowNoDisplays
      is the negation of autoplot. */
  info->allowCloseLastDisplay = getLogicalPreference(node, "allowNoDisplays", !info->createInitialScatterPlot);

  info->quitWithNoGGobi = getLogicalPreference(node, "quitOnLastGGobi", info->allowCloseLastDisplay);

  el = getXMLElement(node, "numDefaultPlotVars");
  if(el) {
    gchar *tmp;
    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "scatmat");
    if(tmp) {
      info->numScatMatrixVars = asNumber(tmp);
    }

    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "parcoords");
    if(tmp) {
      info->numParCoordsVars = asNumber(tmp);
    }

    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "timeplot");
    if(tmp) {
      info->numTimePlotVars = asNumber(tmp);
    }
  }

  el = getXMLElement(node, "sessionFile");
  if(el) {
    gchar *tmp;
    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "name");
    if(tmp)
      info->sessionFile = g_strdup(tmp);
    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "compress");
    if(tmp)
      info->compress = asNumber(tmp);
  }

  return(0);
}

gboolean
getLogicalPreference(xmlNodePtr node, const char *elName, gboolean defaultValue)
{
  xmlNodePtr el;
  gboolean val = defaultValue;
  el = getXMLElement(node, elName);
  if(el) {
    gchar *tmp;
    tmp = (gchar *) xmlGetProp(el, (xmlChar *) "on");
    if(tmp) {
      val = asLogical(tmp);
    } else {
      val = true;
    }
  }
  return(val);
}

gint
getPreviousFiles(const xmlDocPtr doc, GGobiInitInfo *info)
{
  xmlNode *node, *el;
  gint n, i;
  node = getXMLDocElement(doc, "previousFiles");

  if(node == NULL)
      return(0);

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
  if((tmp = (gchar *) xmlGetProp(node, (xmlChar *) "name"))) {
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

  if(strcmp((char *)tag, "url") == 0) {
    val = url_data;
  } else if(strcmp((char *)tag, "database") == 0)
    val = mysql_data;
  else {
    mode = xmlGetProp(node, (xmlChar *) "mode");
    if(strcmp((char *)tag,"file") == 0) {
      if(strcmp((char *)mode, "xml") == 0)
        val = xml_data; 
      else if(strcmp((char *)mode, "ascii") == 0)
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
  if (node) {
    el = XML_CHILDREN(node);
    i = 0;
    while(el) {
      if(el->type != XML_TEXT_NODE && strcmp((char *)el->name,"ggobi") == 0) {
          /* Need to match these with the input source ids. */
        desc = info->descriptions + i;
        getPreviousDisplays(el, desc);
        i++;
      }
      el = el->next;
    }
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
    if(el->type != XML_TEXT_NODE && strcmp((char *)el->name, "display") == 0) {
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

  dpy = (GGobiDisplayDescription*) g_malloc(sizeof(GGobiDisplayDescription));
  tmp = xmlGetProp(node, (xmlChar *) "type");
  dpy->typeName = g_strdup(tmp);
  tmp = xmlGetProp(node, (xmlChar *) "data");
  if(tmp) {
    dpy->data = strToInteger((char *)tmp) - 1;
    if(dpy->data < 0)
      dpy->datasetName = g_strdup(tmp);
  } else
    dpy->data = 0;

  if(xmlGetProp(node, (xmlChar *) "unsupported"))
    dpy->canRecreate = false;

  dpy->numVars = 0;


  el = XML_CHILDREN(node);
  while(el) {
    if(el->type != XML_TEXT_NODE && strcmp((char *)el->name, "variable") == 0) 
      dpy->numVars++;
    el = el->next;
  }

  dpy->varNames = (gchar **) g_malloc(dpy->numVars*sizeof(gchar*));
  for(i = 0, el = XML_CHILDREN(node); i < dpy->numVars; el = el->next) {
    if(el->type != XML_TEXT_NODE && strcmp((char *)el->name, "variable") == 0) {
      dpy->varNames[i++] = g_strdup((char *)xmlGetProp(el, (xmlChar *) "name"));
    }
  }

  return(dpy);
}




/*****************************************************************/

#if SUPPORT_PLUGINS
/*
 Handle the plugins section, looping over each <plugin>
 tag and passing it processPlugin().
*/


GGobiPluginInfo*
getLanguagePlugin(GList *plugins, const char* name)
{
  GList *el = plugins;

  while(el) {
    GGobiPluginInfo *info;
    info = (GGobiPluginInfo *) el->data;
    if(strcmp(info->details->name, name) == 0)
      return(info);
    el = el->next;
  }
  return(NULL);
}


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
   if(el->type != XML_TEXT_NODE) {
     if(strcmp((char *)el->name, "plugin") == 0) {
       plugin = processPlugin(el, info, doc);
       if(plugin)
         info->plugins = g_list_append(info->plugins, plugin);
       } else if(strcmp((char *)el->name, "inputPlugin") == 0) {
         GGobiPluginInfo *inputPlugin = processInputPlugin(el, info, doc);
         if(inputPlugin)
           info->inputPlugins = g_list_append(info->inputPlugins, inputPlugin);
       }
     }
    el = el->next;
  }
}

/*
  This handles the details of a <plugin> tag,
  reading the description, author, etc.
 */

#define GET_PROP_VALUE(field,name) symInfo->field = ((tmp = (char *) xmlGetProp(c, name)) != NULL) ? g_strdup(tmp) : NULL


/**
  Get all the configuration values and optional settings for this plugin.
 */
GGobiPluginInfo *
processPlugin(xmlNodePtr node, GGobiInitInfo *info, xmlDocPtr doc)
{
  gboolean load;
  GGobiPluginInfo *plugin;
  gboolean isLanguage;

  plugin = (GGobiPluginInfo *) g_malloc(sizeof(GGobiPluginInfo));
  memset(plugin, '\0', sizeof(GGobiPluginInfo));
  plugin->details = g_malloc(sizeof(GGobiPluginDetails));
  memset(plugin->details, '\0', sizeof(GGobiPluginDetails));
  plugin->info.g = g_malloc(sizeof(GGobiGeneralPluginInfo));
  memset(plugin->info.g, '\0', sizeof(GGobiGeneralPluginInfo));

  load = getPluginDetails(node, plugin->details, doc);
  
  isLanguage = (xmlGetProp(node, (xmlChar *) "providesLanguage") != NULL);

  getPluginSymbols(node, plugin, doc, isLanguage);
  getPluginOptions(node, plugin->details, doc);

  plugin->details->depends = getPluginDependencies(node, plugin->details, doc);

     /* Weird casting going on here to avoid a void*. */
  getPluginLanguage(node, plugin, GENERAL_PLUGIN, info);

  if(load) {
    loadPluginLibrary(plugin->details, plugin);
  }

  return(plugin);
}

/**
  Pick up and store the named and unnamed arguments for this plugin.
  These will be interpreted in a plugin-specific manner.
 */
gboolean
getPluginOptions(xmlNodePtr node, GGobiPluginDetails *details, xmlDocPtr doc)
{
  xmlNodePtr c;
  c = getXMLElement(node, "options");
  if(!c)
    return(false);

  details->args = getPluginUnnamedArguments(c, details, doc);
  details->namedArgs = getPluginNamedOptions(c, details, doc);

  return(true);
}

/**
  Collect the options for the plugin  that are enclosed within a
  <args></args> element of the form <arg>value</arg>.  The different 
  `value's are stored in a simple single-linked list.
 */
GSList *
getPluginUnnamedArguments(xmlNodePtr node, GGobiPluginDetails *details, xmlDocPtr doc)
{
  GSList *l = NULL;
  xmlNodePtr c, el;
  c = getXMLElement(node, "args");
  if(!c)
    return(NULL);

  el = XML_CHILDREN(c);
  while(el) {
    if(el->type != XML_TEXT_NODE && el->type != XML_COMMENT_NODE) {
      xmlChar *val;
      val = xmlNodeListGetString(doc, XML_CHILDREN(el), 1);
      l = g_slist_append(l, g_strdup((gchar *)val));
    }
    el = el->next;
  }
  return(l);
}


/**
  Collect the elements in the <options><named>...</named></options>
  into a hashtable with elements indexed by 
  the name of the element and value  being the string contents of that element.
  Each element is assumed to be a simple text element.
 */
GHashTable *
getPluginNamedOptions(xmlNodePtr node, GGobiPluginDetails *details, xmlDocPtr doc)
{
  GHashTable *tbl;
  xmlNodePtr c, el;
  c = getXMLElement(node, "named");
  if(!c)
    return(NULL);    

  tbl = g_hash_table_new(g_str_hash, g_str_equal);
  el = XML_CHILDREN(c);
  while(el) {
    if(el->type != XML_TEXT_NODE && el->type != XML_COMMENT_NODE) {
      xmlChar *val;
      val = xmlNodeListGetString(doc, XML_CHILDREN(el), 1);
      g_hash_table_insert(tbl,
                          g_strdup((gchar *)el->name),
                          g_strdup((gchar *)val));
    }
    el = el->next;
  }
  return(tbl);
}


/**
  Pick up the names of all the plugins on which this one depends.
  Then when we load this plugin, we will ensure that those plugins
  are also loaded.
 */
GSList *
getPluginDependencies(xmlNodePtr node, GGobiPluginDetails *info, xmlDocPtr doc)
{
  GSList *list = NULL;
  xmlNodePtr c, el;
  c = getXMLElement(node, "dependencies");
  if(!c)
    return(NULL);
 
  el = XML_CHILDREN(c);
  while(el) {
    if(el->type != XML_TEXT_NODE && el->type != XML_COMMENT_NODE) {
      xmlChar *val;
      val = xmlGetProp(el, (xmlChar *) "name");
      if(val) {
        list = g_slist_append(list, g_strdup((gchar *) val));
      }
    }
    el = el->next;
  }
  return(list);
}


void
getPluginSymbols(xmlNodePtr node, GGobiPluginInfo *plugin, xmlDocPtr doc, gboolean isLanguage)
{
  xmlNodePtr c;
  GGobiGeneralPluginInfo *symInfo = plugin->info.g;
  const xmlChar *tmp;

  c = getXMLElement(node,"dll");
  if(!c)
    return;
  c = getXMLElement(c, "init");
  if(!c)
    return;

  GET_PROP_VALUE(onCreate, "onCreate");
  GET_PROP_VALUE(onClose, "onClose");
  GET_PROP_VALUE(onUpdateDisplay, "onUpdateDisplayMenu");

  if(isLanguage) {
    tmp = xmlGetProp(c, (xmlChar *) "processPlugin");
    if(tmp) {
      GGobiLanguagePluginData *data;
      data = (GGobiLanguagePluginData *)
        g_malloc(sizeof(GGobiLanguagePluginData));
      data->processPluginName = g_strdup((gchar *)tmp);
      plugin->data = data;
    } else {
      fprintf(stderr, "No `processPlugin' entry found for language plugin!\n");
      fflush(stderr);
    }
  }
}


gboolean
loadPluginLibrary(GGobiPluginDetails *plugin, GGobiPluginInfo *realPlugin)
{
  /* If it has already been loaded, just return. */
  if(plugin->loaded) {
    return(true);
  }

   /* Load any plugins on which this one depends. Make certain they 
      are fully loaded and initialized. Potential for inter-dependencies
      that would make this an infinite loop. Hope the user doesn't get this
      wrong as there are no checks at present.
    */
  if(plugin->depends) {
    GSList *el = plugin->depends;
    while(el) {
      gchar *tmp = (gchar*) el->data;
      GGobiPluginInfo *info;
      info = getLanguagePlugin(sessionOptions->info->plugins, tmp);
      if(sessionOptions->verbose == GGOBI_VERBOSE) {
        fprintf(stderr, "Loading dependent plugin %s\n", tmp);fflush(stderr);
      }
      loadPluginLibrary(info->details, info);
      el = el->next;
    }
  }

  plugin->library = load_plugin_library(plugin);
  plugin->loaded = (plugin->library != NULL);

  if(plugin->loaded && plugin->onLoad) {
    OnLoad f = (OnLoad) getPluginSymbol(plugin->onLoad, plugin);
    if(f) {
      f(0, realPlugin);
    } else {
      gchar buf[1000];
      dynload->getError(buf, plugin);
      fprintf(stderr, "error on loading plugin library %s: %s\n",
              plugin->dllName, buf);fflush(stderr);
    }
  }
  return(false);
}

gboolean 
getPluginDetails(xmlNodePtr node, GGobiPluginDetails *plugin, xmlDocPtr doc)
{
  gboolean load = false;
  const xmlChar *tmp;
  GGobiPluginDetails *symInfo = plugin;
  xmlChar * val;
  xmlNodePtr el;

  tmp = xmlGetProp(node, (xmlChar *) "name");
  if(tmp) {
    plugin->name = g_strdup((char *) tmp);
  }

  tmp = xmlGetProp(node, (xmlChar *) "load");
  if(tmp) {
    load = strcmp((char*)tmp, "immediate") == 0;
  }

  el = XML_CHILDREN(node);
  while(el) {
    if(el->type != XML_TEXT_NODE) {
      if(strcmp((char *) el->name, "author") == 0) {
        val = xmlNodeListGetString(doc, XML_CHILDREN(el), 1);
        plugin->author = g_strdup((char*) val);
      } else if(strcmp((char *) el->name, "description") == 0) {
        val = xmlNodeListGetString(doc, XML_CHILDREN(el), 1);
        plugin->description = g_strdup((char*) val);
      } else if(strcmp((char *) el->name, "dll") == 0) {
        plugin->dllName = g_strdup((char*) xmlGetProp(el, (xmlChar *) "name"));
        if(XML_CHILDREN(el)) {
          xmlNodePtr c = XML_CHILDREN(el);
          while(c) {
            if(el->type != XML_TEXT_NODE &&
              strcmp((char *)c->name, "init") == 0)
            {
              GET_PROP_VALUE(onLoad, "onLoad");
              GET_PROP_VALUE(onUnload, "onUnload");
              break;
            }
            c = c->next;
          }
        }
      }
    }

    el = el->next;
 }

 return(load);
}

void
fixJavaClassName(gchar *name)
{
  gchar *p = name;
    
  while(p && (p = strchr(p, '.')) != NULL) {
    p[0] = '/';
    p++;
  }
}

gboolean
setLanguagePluginInfo(GGobiPluginDetails *details, const char *language, GGobiInitInfo *info)
{
  GGobiPluginInfo *tmp = getLanguagePlugin(info->plugins, language);
  if(!tmp) {
    return(false);
  } else {
    GGobiPluginDetails *jdetails = tmp->details;
    details->dllName = g_strdup(jdetails->dllName);
    details->library = jdetails->library;
    details->loaded = 0;

    /*    details->depends = g_slist_append(details->depends, tmp); */
    details->depends = g_slist_append(details->depends,
                                      g_strdup(jdetails->name));
  }
  return(true);
}

gboolean
getPluginLanguage(xmlNodePtr node, GGobiPluginInfo *plugin,
  GGobiPluginType type, GGobiInitInfo *info)
{
  gboolean done = false;
  const xmlChar *tmp;
  tmp = xmlGetProp(node, (xmlChar *) "language");

  if(tmp) {
    GGobiPluginInfo *langPlugin = getLanguagePlugin(info->plugins,
                                                    (char *) tmp);
    GGobiLanguagePluginData *d;
    ProcessPluginInfo f;

    if(langPlugin == NULL) {
      fprintf(stderr, "No language plugin %s\n", (char *) tmp); fflush(stderr);
      return(false);
    }
    d = (GGobiLanguagePluginData *) langPlugin->data;
    loadPluginLibrary(langPlugin->details, langPlugin);
    if(d) {
      f = (ProcessPluginInfo) getPluginSymbol(d->processPluginName,
                                              langPlugin->details);
      if(f) {
        done = f(node, plugin, type, langPlugin, info);
      }
    }
    if(done == false)
      fprintf(stderr, "Problem processing language plugin processor.");
  } else
  done = true;

  return(done);
}


GGobiPluginInfo *
processInputPlugin(xmlNodePtr node, GGobiInitInfo *info, xmlDocPtr doc)
{
  GGobiPluginInfo *plugin;
  gboolean load;

  plugin = (GGobiPluginInfo *) g_malloc(sizeof(GGobiPluginInfo));
  memset(plugin, '\0', sizeof(GGobiPluginInfo)); 
  plugin->details = g_malloc(sizeof(GGobiPluginDetails));
  memset(plugin->details, '\0', sizeof(GGobiPluginDetails));
  plugin->info.i = g_malloc(sizeof(GGobiInputPluginInfo));
  memset(plugin->info.i, '\0', sizeof(GGobiInputPluginInfo));


  load = getPluginDetails(node, plugin->details, doc);

  getInputPluginValues(node, plugin->info.i, doc);

  getPluginOptions(node, plugin->details, doc);
  plugin->details->depends = getPluginDependencies(node, plugin->details, doc);

  getPluginLanguage(node, plugin, INPUT_PLUGIN, info);

  if(load) {
    loadPluginLibrary(plugin->details, (GGobiPluginInfo*) plugin);
  }

  return(plugin);
}

void
getInputPluginValues(xmlNodePtr node, GGobiInputPluginInfo *plugin,
  xmlDocPtr doc)
{
  xmlNodePtr c;
  GGobiInputPluginInfo *symInfo = plugin;
  const xmlChar *tmp;

  tmp = xmlGetProp(node, (xmlChar *) "interactive");
  if(tmp) {
    plugin->interactive = (tmp[0] == 'T' || tmp[0] == 't');
  }

  c = getXMLElement(node, "modeName");
  if(c) {
    xmlChar *val = xmlNodeListGetString(doc, XML_CHILDREN(c), 1);      
    plugin->modeName = (char *) val;
  }

  c = getXMLElement(node,"dll");
  if(!c)
    return;

  c = getXMLElement(c, "init");
  if(!c)
    return;

  GET_PROP_VALUE(read_symbol_name, "read");
  GET_PROP_VALUE(probe_symbol_name, "probe");
  GET_PROP_VALUE(getDescription, "description");
}

#endif /* end of SUPPORT_PLUGINS */

gint resolveVariableName(const gchar *name, datad *d);

displayd *
createExtendedDisplay(const gchar * const type, gint *vars, gint numVars, datad *d, ggobid *gg)
{
  displayd *dpy;

  GtkGGobiExtendedDisplayClass *klass;
  GtkType gtype = gtk_type_from_name(type);
  klass = gtk_type_class(gtype);
  if(!klass->createWithVars) {
     g_printerr("Cannot currently handle the extended display %s type.", type);
     return(NULL);
  }
  dpy = klass->createWithVars(false, numVars, vars, d, gg);
  if(!dpy)
    return(NULL);

  display_add(dpy, gg);

  return(dpy);
}


displayd *
createDisplayFromDescription(ggobid *gg, GGobiDisplayDescription *desc) 
{
  displayd *dpy = NULL;
  datad *data = NULL;
  gint *vars, i;

  if(desc->canRecreate == false)
    return(NULL);

  if(desc->data > -1) {
     data = (datad *)  g_slist_nth_data(gg->d, desc->data);
  } else if(desc->datasetName && desc->datasetName[0]) {
     datad *tmp;
     GSList *l;
     for(l = gg->d; l ; l = l->next) {
        tmp = (datad *) l->data;
	if(strcmp(desc->datasetName, tmp->name) == 0) {
	  data = tmp;
   	  break;
	}
     }
  }

  if(!data) {
    g_printerr("Cannot resolve dataset ");
    if(desc->data > -1)
       g_printerr("with index %d\n", desc->data + 1);
    else
       g_printerr("named `%s'\n", desc->datasetName);

    return(NULL);
  }

  vars = (gint*) g_malloc(sizeof(gint) * desc->numVars);
  for (i = 0; i < desc->numVars; i++)
    vars[i] = resolveVariableName(desc->varNames[i], data);

  dpy = createExtendedDisplay(desc->typeName, vars, desc->numVars, data, gg);

  g_free(vars);

  return(dpy);
}


gint
resolveVariableName(const gchar *name, datad *d)
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

