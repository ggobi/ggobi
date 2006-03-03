/* write_state.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

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

#include "write_state.h"

#if XML_USE_CHILDS
#define XML_DOC_ROOT(doc) (doc)->root
#else
#define XML_DOC_ROOT(doc) (doc)->children
#endif

xmlNodePtr add_xml_display(displayd *dpy, xmlDocPtr doc);
void add_xml_scatterplot_variables(xmlNodePtr node, GList *plots, displayd *dpy);
void add_xml_scatmat_variables(xmlNodePtr node, GList *plot, displayd *dpy);
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
    }
    else {
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
  doc = xmlNewDoc(BAD_CAST("1.0"));
  XML_DOC_ROOT(doc) = xmlNewDocNode(doc, NULL, BAD_CAST("ggobirc"), NULL);

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


void add_brush_info(xmlNodePtr node, GGobiData *d, ggobid *gg);

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
        doc = xmlNewDoc(BAD_CAST("1.0"));
        XML_DOC_ROOT(doc) = xmlNewDocNode(doc, NULL, BAD_CAST("ggobi"), NULL);
 }

 el = els;
 while(el) {
   node = add_xml_display((displayd *) el->data, doc);
   el = el->next;
 }

 /* write out the brushing information. */
 {
   GSList *el;
  for(el = gg->d; el; el = el->next) {
   GGobiData *d = (GGobiData *)el->data;
   if(d->npts_under_brush > 0) {
      add_brush_info(XML_DOC_ROOT(doc), d, gg);
      break;
   }
 }
 }

 return(doc);
}

void
add_brush_info(xmlNodePtr node, GGobiData *d, ggobid *gg)
{
  char buf[10];
  gint i;
  xmlNodePtr brushNode;

  if(d->npts_under_brush < 1)
    return;

  brushNode = xmlNewChild(node, NULL, BAD_CAST("brush"), NULL);
  sprintf(buf, "%d", d->npts_under_brush);
  xmlSetProp(brushNode, BAD_CAST("count"), BAD_CAST(buf));
  xmlSetProp(brushNode, BAD_CAST("datasetName"), BAD_CAST(d->name));
  for(i = 0; i < d->nrows;i++) {
    if(d->pts_under_brush.els[i]) {
      xmlNodePtr tmp;
      tmp = xmlNewChild(brushNode, NULL, BAD_CAST("int"), NULL);
      sprintf(buf, "%d", i);
      xmlSetProp(tmp, BAD_CAST("value"), BAD_CAST(buf));
    }
  }
}

/*
  Add the display from a ggobid to the XML DOM.
 */
xmlNodePtr
add_xml_display(displayd *dpy, xmlDocPtr doc)
{
  GList *plots;
  xmlNodePtr node;
  gchar buf[20];
  int ctr = 0;
  //char *props[] = {"width", "height"};

    /* Create a display tag with attributes `type' and `data'
       specifying the type of the display and the name of the
       dataset in which the variables are to be found.
     */

  node = xmlNewChild(XML_DOC_ROOT(doc), NULL, BAD_CAST("display"), NULL);
  xmlSetProp(node, BAD_CAST("type"), BAD_CAST(getDisplayTypeName(dpy)));
  xmlSetProp(node, BAD_CAST("data"), BAD_CAST(getDataName(dpy)));
  if(dpy->ggobi->current_display == dpy) {
     xmlSetProp(node, BAD_CAST("active"),  BAD_CAST("true"));
  }

    /* Index of the active plot within this display (regardless of whether the
       display is active). */
  for(plots = dpy->splots; plots; plots = plots->next, ctr++) {
     if(dpy->current_splot == plots->data) {
         sprintf(buf, "%d", ctr);
         xmlSetProp(node, BAD_CAST("activePlotIndex"), BAD_CAST(buf));
         break;
     }
  }
   
  /* write the width and height information so we can restore these.
     Currently, the query only returns -1! */
	 /* should implement these as GObject properties, but I think the state
	 will be saved via R (Rggobi) instead of here anyway - mfl */
/*
#ifndef GTK_2_0
  for(i = 0; i < sizeof(props)/sizeof(props[0]); i++) {
    arg.name = props[i];
    gtk_object_arg_get(GTK_OBJECT(dpy), &arg, NULL);
    sprintf(buf, "%d", arg.d.int_data);
    xmlSetProp(node, props[i], buf);
  }
#endif*/

  plots = dpy->splots;
  if(GGOBI_IS_EXTENDED_DISPLAY(dpy)) {
    GGobiExtendedDisplayClass *klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS(dpy);
    if(klass->xml_describe) {
      klass->xml_describe(node, plots, dpy);
    } else {
      xmlSetProp(node, BAD_CAST("unsupported"), BAD_CAST("true"));
      g_printerr("No method for generating XML description of %s display type\n", klass->titleLabel); 
    }
  }

 return(node);
}

/*
  Utility method for adding an XML node of the form
   <variable name="variable-name" />
  Returns the name.
 */

xmlNodePtr
XML_addVariable(xmlNodePtr node, gint j, GGobiData *d)
{
  extern vartabled * vartable_element_get (gint, GGobiData *);
  xmlNodePtr newNode;
  vartabled *vt = vartable_element_get (j, d);

  newNode = xmlNewChild(node, NULL,BAD_CAST("variable"), NULL);
  xmlSetProp(newNode, BAD_CAST("name"), BAD_CAST(vt->collab));

  return(newNode);   
}

/*
  Get the name of the display type.
  Doesn't handle mixed types that can be created in R.
 */
const char *
getDisplayTypeName(displayd *dpy)
{
  const gchar *val = "";
  if(GGOBI_IS_EXTENDED_DISPLAY(dpy)) {
      /* Perhaps compute the name of the type and use that. */
    val = g_type_name(G_OBJECT_TYPE(G_OBJECT(dpy)));
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

  node = xmlNewChild(XML_DOC_ROOT(doc), NULL, BAD_CAST("preferences"), NULL);
  if(info->colorSchemeFile) {
      kid = xmlNewChild(node, NULL, BAD_CAST("colorschemes"), NULL);   
      xmlSetProp(kid, BAD_CAST("file"), BAD_CAST(info->colorSchemeFile));
  }
   /* Foreground and background */

  if(info->glyph.type != -1) {
     kid = xmlNewChild(node, NULL, BAD_CAST("glyph"), NULL);
     sprintf(buf, "%s", GGobi_getGlyphTypeName(info->glyph.type));
     xmlSetProp(kid, BAD_CAST("type"), BAD_CAST(buf));
     sprintf(buf, "%d", info->glyph.size);
     xmlSetProp(kid, BAD_CAST("size"), BAD_CAST(buf));
  }

  kid = xmlNewChild(node, NULL, BAD_CAST("numDefaultPlotVars"), NULL);
  sprintf(buf, "%d", info->numScatMatrixVars);
  xmlSetProp(kid, BAD_CAST("scatmat"), BAD_CAST(buf));
  sprintf(buf, "%d", info->numParCoordsVars);
  xmlSetProp(kid, BAD_CAST("parcoords"), BAD_CAST(buf));
  sprintf(buf, "%d", info->numTimePlotVars);
  xmlSetProp(kid, BAD_CAST("timeplot"), BAD_CAST(buf));

  kid = xmlNewChild(node, NULL, BAD_CAST("autoplot"), NULL);
  xmlSetProp(kid, BAD_CAST("on"), BAD_CAST(info->createInitialScatterPlot ? "TRUE" : "FALSE"));
  kid = xmlNewChild(node, NULL, BAD_CAST("allowNoDisplays"), NULL);
  xmlSetProp(kid, BAD_CAST("on"), BAD_CAST(info->createInitialScatterPlot ? "TRUE" : "FALSE"));
  kid = xmlNewChild(node, NULL, BAD_CAST("quitOnLastGGobi"), NULL);
  xmlSetProp(kid, BAD_CAST("on"), BAD_CAST(info->createInitialScatterPlot ? "TRUE" : "FALSE"));

  kid = xmlNewChild(node, NULL, BAD_CAST("sessionFile"), NULL);
  xmlSetProp(kid, BAD_CAST("file"), BAD_CAST(info->sessionFile));  
  sprintf(buf, "%d", info->compress);
  xmlSetProp(kid, BAD_CAST("compress"), BAD_CAST(buf));  

  return(true);
}


gboolean 
create_plugins_xml(GGobiOptions *sessionOptions, xmlDocPtr doc)
{
  xmlNodePtr node;
  GList *el;
  node = xmlNewChild(XML_DOC_ROOT(doc), NULL, BAD_CAST("plugins"), NULL);

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
                       BAD_CAST(plugin->type == GENERAL_PLUGIN ? "plugin" : "inputPlugin"), NULL);

    xmlSetProp(node, BAD_CAST("name"), BAD_CAST(plugin->details->name));
    if(plugin->details->language)
        xmlSetProp(node, BAD_CAST("language"), BAD_CAST(plugin->details->language));

    if(plugin->type == INPUT_PLUGIN) {
        xmlSetProp(node, BAD_CAST("interactive"), BAD_CAST(plugin->info.i->interactive ? "TRUE" : "FALSE"));
    }

    el = xmlNewChild(node, NULL, BAD_CAST("description"), BAD_CAST(plugin->details->description));
    el = xmlNewChild(node, NULL, BAD_CAST("author"), BAD_CAST(plugin->details->author));

    if(plugin->type == INPUT_PLUGIN) {
        gchar buf[10];
	gint k;

        el = xmlNewChild(node, NULL, BAD_CAST("modeNames"), NULL);
	sprintf(buf, "%d", plugin->info.i->numModeNames);
	xmlSetProp(el,  BAD_CAST("numNodes"), BAD_CAST(buf));

	for(k = 0; k < plugin->info.i->numModeNames; k++)
   	   xmlNewChild(el, NULL, BAD_CAST("modeName"), BAD_CAST(plugin->info.i->modeNames[k]));
    }    

    el = xmlNewChild(node, NULL, BAD_CAST("dll"), NULL);
    xmlSetProp(el, BAD_CAST("name"), BAD_CAST(plugin->details->dllName));

    el = xmlNewChild(el, NULL, BAD_CAST("init"), NULL);
    if(plugin->details->onLoad)
        xmlSetProp(el, BAD_CAST("onLoad"), BAD_CAST(plugin->details->onLoad));
    if(plugin->details->onUnload)
        xmlSetProp(el, BAD_CAST("onUnload"), BAD_CAST(plugin->details->onUnload));

    
    if(plugin->type == GENERAL_PLUGIN) {
        GGobiGeneralPluginInfo *info = plugin->info.g;
        if(info->onCreate)
            xmlSetProp(el, BAD_CAST("onCreate"), BAD_CAST(info->onCreate));
        if(info->onClose)
            xmlSetProp(el, BAD_CAST("onClose"), BAD_CAST(info->onClose));
        if(info->onUpdateDisplay)
            xmlSetProp(el, BAD_CAST("onUpdateDisplay"), BAD_CAST(info->onUpdateDisplay));
    } else {
        GGobiInputPluginInfo *info = plugin->info.i;
        if(info->read_symbol_name)
            xmlSetProp(el, BAD_CAST("read"), BAD_CAST(info->read_symbol_name));
        if(info->getDescription)
            xmlSetProp(el, BAD_CAST("description"), BAD_CAST(info->getDescription));
    }

    

    return(true);
}
