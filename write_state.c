/*
 This is used to display the state of a ggobi instance and its sub-elements:
 its displays, their splots in XML format.
 */

#include <libxml/tree.h>
#include "ggobi.h"

#include "GGobiAPI.h"

#include <math.h>

#include "plugin.h"
#include "externs.h"

#if XML_USE_CHILDS
#define XML_DOC_ROOT(doc) (doc)->root
#else
#define XML_DOC_ROOT(doc) (doc)->children
#endif

xmlNodePtr add_xml_display(displayd *dpy, xmlDocPtr doc);
void add_xml_scatterplot_variables(xmlNodePtr node, GList *plots, displayd *dpy);
void add_xml_tsplot_variables(xmlNodePtr node, GList *plots, displayd *dpy);
void add_xml_scatmat_variables(xmlNodePtr node, GList *plot, displayd *dpy);
const char *addVariable(xmlNodePtr node, int which, datad *d);
void add_xml_parcoords_variables(xmlNodePtr node, GList *plots, displayd *dpy);


xmlDocPtr create_ggobi_xml(ggobid *gg, xmlDocPtr doc);
gboolean create_preferences_xml(GGobiOptions *sessionOptions, xmlDocPtr doc);
gboolean create_plugins_xml(GGobiOptions *sessionOptions, xmlDocPtr doc);
gboolean create_plugin_xml(GGobiPluginInfo *plugin,  xmlNodePtr doc);


const char *getDisplayTypeName(displayd *dpy);
const char *getDataName(displayd *dpy);


gboolean
saveDOMToFile(xmlDocPtr doc, const char *fileName)
{
    int status;

    xmlIndentTreeOutput = TRUE;
    if(sessionOptions->info->compress > 0) {
	int compressionLevel;
	compressionLevel = xmlGetDocCompressMode(doc);
	xmlSetDocCompressMode(doc, sessionOptions->info->compress);
	status = xmlSaveFile(fileName, doc);
	xmlSetDocCompressMode(doc, compressionLevel);
    } else {
	xmlChar *mem;
	int size;
	FILE *f;
	xmlDocDumpFormatMemoryEnc(doc, &mem, &size, NULL, TRUE);
	if( (f = fopen(fileName, "w"))) {
	    fprintf(f, "%s", mem);
	    status = 1;
	    fclose(f);
	}
	xmlFree(mem);
    }


    if(status < 0) {
        char buf[1000];
	sprintf(buf, "%s\n%s", "Couldn't save session in file ", fileName);
	quick_message(buf, true);
    }
    return(status > 0);
}

/**
 Create the XML document tree (DOM) representing the
 given ggobid. This contains 
   o the preferences;
   o previous files visited;
   o ggobi instances in existence
       + data source
       + displays
         - plots
   o plugins
       + name, interactive, options
       + library & symbols
       + active
*/
gboolean
ggobi_write_session(const char *fileName)
{
  xmlDocPtr doc;
  ggobid *gg;
  gboolean ans;
  int i;

     /* Create the document */
  doc = xmlNewDoc("1.0");
  XML_DOC_ROOT(doc) = xmlNewDocNode(doc, NULL, "ggobirc", NULL);

  create_preferences_xml(sessionOptions, doc);

  for(i = 0; i < GGobi_getNumGGobis(); i++) {
      gg = GGobi_ggobi_get(i);
      create_ggobi_xml(gg, doc);
  }

  create_plugins_xml(sessionOptions, doc);

  ans = saveDOMToFile(doc, fileName);
  xmlFreeDoc(doc);

  return(ans);
}

 /*
  Write an XML representation of the given ggobid instance
  to the specified filename.
 */
void 
write_ggobi_as_xml(ggobid *gg, const char *fileName, xmlDocPtr doc)
{
    doc = create_ggobi_xml(gg, doc);
    saveDOMToFile(doc, fileName);
}


/**

 */
xmlDocPtr
create_ggobi_xml(ggobid *gg, xmlDocPtr doc)
{
 GList *els, *el;
 xmlNodePtr node;

 els = gg->displays;
 if(!els) 
     return(NULL);

 if(doc == NULL) {
          /* Create the document */
	doc = xmlNewDoc("1.0");
	XML_DOC_ROOT(doc) = xmlNewDocNode(doc, NULL, "ggobi", NULL);
 }

 el = els;
 while(el) {
   node = add_xml_display((displayd *) el->data, doc);
   el = el->next;
 }

 return(doc);
}

/*
  Add the display from a ggobid to the XML DOM.
 */
xmlNodePtr
add_xml_display(displayd *dpy, xmlDocPtr doc)
{
  GList *plots;
  xmlNodePtr node;

    /* Create a display tag with attributes `type' and `data'
       specifying the type of the display and the name of the
       dataset in which the variables are to be found.
     */
  node = xmlNewChild(XML_DOC_ROOT(doc), NULL, "display", NULL);
  xmlSetProp(node, "type", getDisplayTypeName(dpy));
  xmlSetProp(node, "data", getDataName(dpy));


  plots = dpy->splots;
  switch(dpy->displaytype) {
    case scatterplot:
      add_xml_scatterplot_variables(node, plots, dpy);
    break;
    case scatmat:
      add_xml_scatmat_variables(node, plots, dpy);
    break;
    case parcoords:
      add_xml_parcoords_variables(node, plots, dpy);
    break;
    case tsplot:
      add_xml_tsplot_variables(node, plots, dpy);
    break;
    case unknown_display_type:
    break;
  }

 return(node);
}

/*
  Write out the variables in a parallel coordinates plot
  to the current node in the  XML tree.
 */
void
add_xml_parcoords_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot;
  while(plots) {
    plot = (splotd *) plots->data;
    addVariable(node, plot->p1dvar, dpy->d);
    plots = plots->next;
  }
}


/*
  Write out the variables in a time series plot
  to the current node in the XML tree.
 */
void
add_xml_tsplot_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot;
  while(plots) {
    plot = (splotd *)plots->data;
    addVariable(node, plot->p1dvar, dpy->d);
    plots = plots->next;
  }
}

/*
  Write out the variables in a scatter plot matrix
  to the current node in the XML tree.
 */
void
add_xml_scatmat_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot = plots->data;
  int n, n1, i;

  n1 = g_list_length(plots);
  n = sqrt(n1);

  for(i = 0; i < n1 ; i+=n) {
      plot = (splotd *) g_list_nth_data(plots, i);
      addVariable(node, plot->xyvars.x, dpy->d);
  }
}

/*
  Write out the variables in a scatterplot
  to the current node in the XML tree.
 */
void
add_xml_scatterplot_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot = (splotd *)plots->data;
  addVariable(node, plot->xyvars.x, dpy->d);
  addVariable(node, plot->xyvars.y, dpy->d);
}


/*
  Utility method for adding an XML node of the form
   <variable name="variable-name" />
  Returns the name.
 */

const char *
addVariable(xmlNodePtr node, gint j, datad *d)
{
  extern vartabled * vartable_element_get (gint, datad *);
  const char *name;
  vartabled *vt = vartable_element_get (j, d);

  node = xmlNewChild(node, NULL,"variable", NULL);
  name = vt->collab;
  xmlSetProp(node, "name", name);

  return(name);   
}

/*
  Get the name of the display type.
  Doesn't handle mixed types that can be created in R.
 */
const char *
getDisplayTypeName(displayd *dpy)
{
  const gchar *val;

  switch(dpy->displaytype) {
    case scatterplot:
      val = "scatterplot";
    break;
    case scatmat:
      val = "scatmat";
    break;
    case parcoords:
      val = "parcoords";
    break;
    case tsplot:
      val = "tsplot";
    break;
    default:
      val = "";
    break;
  }

    return(val);
}

/*
  Get the name of the data set associated with the display.
 */
const char *
getDataName(displayd *dpy)
{
  return((const char *) dpy->d->name);
}


gboolean
create_preferences_xml(GGobiOptions *sessionOptions, xmlDocPtr doc)
{
  xmlNodePtr node, kid;
  char buf[20];
  GGobiInitInfo *info = sessionOptions->info;

  node = xmlNewChild(XML_DOC_ROOT(doc), NULL, "preferences", NULL);
  if(info->colorSchemeFile) {
      kid = xmlNewChild(node, NULL, "colorschemes", NULL);   
      xmlSetProp(kid, "file", info->colorSchemeFile);
  }
   /* Foreground and background */

  if(info->glyph.type != -1) {
     kid = xmlNewChild(node, NULL, "glyph", NULL);
     sprintf(buf, "%s", GGobi_getGlyphTypeName(info->glyph.type));
     xmlSetProp(kid, "type", buf);
     sprintf(buf, "%d", info->glyph.size);
     xmlSetProp(kid, "size", buf);
  }

  kid = xmlNewChild(node, NULL, "numDefaultPlotVars", NULL);
  sprintf(buf, "%d", info->numScatMatrixVars);
  xmlSetProp(kid, "scatmat", buf);
  sprintf(buf, "%d", info->numParCoordsVars);
  xmlSetProp(kid, "parcoords", buf);
  sprintf(buf, "%d", info->numTimePlotVars);
  xmlSetProp(kid, "timeplot", buf);

  kid = xmlNewChild(node, NULL, "autoplot", NULL);
  xmlSetProp(kid, "on", info->createInitialScatterPlot ? "TRUE" : "FALSE");
  kid = xmlNewChild(node, NULL, "allowNoDisplays", NULL);
  xmlSetProp(kid, "on", info->createInitialScatterPlot ? "TRUE" : "FALSE");
  kid = xmlNewChild(node, NULL, "quitOnLastGGobi", NULL);
  xmlSetProp(kid, "on", info->createInitialScatterPlot ? "TRUE" : "FALSE");

  kid = xmlNewChild(node, NULL, "sessionFile", NULL);
  xmlSetProp(kid, "file", info->sessionFile);  
  sprintf(buf, "%d", info->compress);
  xmlSetProp(kid, "compress", buf);  

  return(true);
}


gboolean 
create_plugins_xml(GGobiOptions *sessionOptions, xmlDocPtr doc)
{
  xmlNodePtr node;
  GList *el;
  node = xmlNewChild(XML_DOC_ROOT(doc), NULL, "plugins", NULL);

  el = sessionOptions->info->plugins;
  while(el) {
      create_plugin_xml((GGobiPluginInfo *) el->data, node);
      el = el->next;
  }

  el = sessionOptions->info->inputPlugins;
  while(el) {
      create_plugin_xml((GGobiPluginInfo *) el->data, node);
      el = el->next;
  }
  return(true);
}

gboolean
create_plugin_xml(GGobiPluginInfo *plugin,  xmlNodePtr doc)
{
    xmlNodePtr node, el;

    node = xmlNewChild(doc, NULL, 
		       plugin->type == GENERAL_PLUGIN ? "plugin" : "inputPlugin", NULL);

    xmlSetProp(node, "name", plugin->details->name);
    if(plugin->details->language)
	xmlSetProp(node, "language", plugin->details->language);

    if(plugin->type == INPUT_PLUGIN) {
	xmlSetProp(node, "interactive", plugin->info.i->interactive ? "TRUE" : "FALSE");
    }

    el = xmlNewChild(node, NULL, "description", plugin->details->description);
    el = xmlNewChild(node, NULL, "author", plugin->details->author);

    if(plugin->type == INPUT_PLUGIN) {
	xmlSetProp(node, "modeName", plugin->info.i->modeName);
    }    

    el = xmlNewChild(node, NULL, "dll", NULL);
    xmlSetProp(el, "name", plugin->details->dllName);

    el = xmlNewChild(el, NULL, "init", NULL);
    if(plugin->details->onLoad)
	xmlSetProp(el, "onLoad", plugin->details->onLoad);
    if(plugin->details->onUnload)
	xmlSetProp(el, "onUnload", plugin->details->onUnload);

    
    if(plugin->type == GENERAL_PLUGIN) {
	GGobiGeneralPluginInfo *info = plugin->info.g;
	if(info->onCreate)
	    xmlSetProp(el, "onCreate", info->onCreate);
	if(info->onClose)
	    xmlSetProp(el, "onClose", info->onClose);
	if(info->onUpdateDisplay)
	    xmlSetProp(el, "onUpdateDisplay", info->onUpdateDisplay);
    } else {
	GGobiInputPluginInfo *info = plugin->info.i;
	if(info->read_symbol_name)
	    xmlSetProp(el, "read", info->read_symbol_name);
	if(info->getDescription)
	    xmlSetProp(el, "description", info->getDescription);
    }

    

    return(true);
}
