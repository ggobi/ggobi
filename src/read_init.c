/* read_init.c */
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
  Reads initialization file.
 */
#include "read_init.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/parserInternals.h> /* for xmlDoValidityChecking */

#include <string.h>

#include "plugin.h"

#include "GGobiAPI.h"

#include "externs.h"
#include "tour.h"

#include "ggobi-plugin-factory.h"

const gchar *const GlyphNames[] = {
  /*        ".", "+", "x", "or", "fr", "oc", "fc", "" */
  ".", "plus", "x", "oc", "or", "fc", "fr", ""
};



gint getPreviousFiles (const xmlDocPtr doc, GGobiInitInfo * info);
void parsePreviousInput (xmlNode * node, GGobiDescription *desc);

gboolean getLogicalPreference (xmlNodePtr node, const char *elName,
                               gboolean defaultValue);

int getPlugins (xmlDocPtr doc, GGobiInitInfo * info);
                       
gint getPreviousGGobiDisplays (const xmlDocPtr doc, GGobiInitInfo * info);
GGobiDisplayDescription *getDisplayDescription (xmlNodePtr node);
gint getPreferences (const xmlDocPtr doc, GGobiInitInfo * info);


GGobiInitInfo *
read_init_file (const gchar * filename, GGobiInitInfo * info)
{
  xmlDocPtr doc;
  gchar *fileName = NULL;
  gint oldValiditySetting = xmlDoValidityCheckingDefaultValue;

  xmlSubstituteEntitiesDefault (1);
  xmlDoValidityCheckingDefaultValue = false;

  if (sessionOptions->verbose == GGOBI_VERBOSE)
    g_printerr ("Reading initialization file %s\n", filename);

  fileName = g_strdup (filename);
  doc = xmlParseFile (fileName);
  if (doc == NULL) {
    return (info);
  }
  if (info == NULL)
    info = (GGobiInitInfo *) g_malloc (sizeof (GGobiInitInfo));

  info->numInputs = 0;
  info->descriptions = NULL;
  info->filename = g_strdup (filename);

  getPreferences (doc, info);
  getPreviousFiles (doc, info);
  getPreviousGGobiDisplays (doc, info);
  info->plugins = NULL;
  getPlugins (doc, info);

  xmlDoValidityCheckingDefaultValue = oldValiditySetting;

  /* Causes a crash when started with -init notes/ggobirc,
     but not if there is a -colorschemes filename
     g_free(fileName); 
   */

  /* Should release the doc object also. */
  xmlFreeDoc (doc);
  return (info);
}

    /* Find a node with a secified tag Name */
xmlNode *
getXMLDocElement (const xmlDocPtr doc, const char *tagName)
{
  xmlNode *node = xmlDocGetRootElement (doc);
  return (getXMLElement (node, tagName));
}

xmlNode *
getXMLElement (xmlNodePtr node, const char *tagName)
{
  if (xmlStrcmp (node->name, BAD_CAST (tagName)) == 0)
    return (node);
  node = XML_CHILDREN (node);
  while (node) {
    if (xmlStrcmp (node->name, BAD_CAST (tagName)) == 0) {
      return (node);
    }
    node = node->next;
  }

  return (node);
}


static void
getTourSpeedValue (xmlNodePtr node, const xmlDocPtr doc, const gchar * name,
                   gfloat * value)
{
  xmlNodePtr el = getXMLElement (node, name);
  if (el) {
    xmlChar *tmp;
    gfloat val;
    tmp = xmlNodeListGetString (doc, XML_CHILDREN (el), 1);
    val = atof ((char *) tmp);
    if (val > 0 && val < MAX_TOUR_SPEED)
      *value = val;
    else
      g_printerr ("Value for %s in preferences file is invalid: %f\n", name,
                  val);
  }
}


gint
getPreferences (const xmlDocPtr doc, GGobiInitInfo * info)
{
  xmlNode *node, *el;
  /*gint n, i; */
  node = getXMLDocElement (doc, "preferences");
  if (!node)
    return (-1);

  /* Don't read this setting if the user has specified a value in
     the command line argument. */
  if (info->colorSchemeFile == NULL) {
    el = getXMLElement (node, "colorschemes");
    if (el) {
      gchar *tmp;
      tmp = (gchar *) xmlGetProp (el, (xmlChar *) "file");
      info->colorSchemeFile = g_strdup (tmp);
    }
  }

  info->bgColor = NULL;  /*-- this needs to be initialized --*/
  el = getXMLElement (node, "background");
  if (el) {
    el = getXMLElement (el, "color");
    if (el) {
      info->bgColor = (GdkColor *) g_malloc (sizeof (GdkColor));
      getColor (el, doc, NULL, info->bgColor);
      if (gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                    info->bgColor, false, true) == false) {
        g_printerr ("Can't allocate background color\n");
      }
    }
  }

  info->fgColor = NULL;  /*-- this needs to be initialized --*/
  el = getXMLElement (node, "foreground");
  if (el) {
    el = getXMLElement (el, "color");
    if (el) {
      info->fgColor = (GdkColor *) g_malloc (sizeof (GdkColor));
      getColor (el, doc, NULL, info->fgColor);
      if (gdk_colormap_alloc_color (gdk_colormap_get_system (),
                                    info->fgColor, false, true) == false) {
        g_printerr ("Can't allocate foreground color\n");
      }
    }
  }


  el = getXMLElement (node, "glyph");
  if (el) {
    gchar *tmp;

    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "type");
    if (tmp) {
      info->glyph.type = mapGlyphName (tmp);
    }
    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "size");
    if (tmp) {
      info->glyph.size = as_number (tmp);
    }
  }

  /* Whether to use check menu items. 
     XXX should this be in info or sessionOptions.
   */
  sessionOptions->useRadioMenuItems =
    getLogicalPreference (node, "useRadioMenuItems", false);

  info->createInitialScatterPlot =
    getLogicalPreference (node, "autoplot", true);

  /* If we autoplot, then we will by default expect there to be
     at least one plot, so our default value for allowNoDisplays
     is the negation of autoplot. */
  info->allowCloseLastDisplay =
    getLogicalPreference (node, "allowNoDisplays",
                          !info->createInitialScatterPlot);

  info->quitWithNoGGobi =
    getLogicalPreference (node, "quitOnLastGGobi",
                          info->allowCloseLastDisplay);

  el = getXMLElement (node, "numDefaultPlotVars");
  if (el) {
    gchar *tmp;
    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "scatmat");
    if (tmp) {
      info->numScatMatrixVars = as_number (tmp);
    }

    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "parcoords");
    if (tmp) {
      info->numParCoordsVars = as_number (tmp);
    }

    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "timeplot");
    if (tmp) {
      info->numTimePlotVars = as_number (tmp);
    }
  }

  el = getXMLElement (node, "sessionFile");
  if (el) {
    gchar *tmp;
    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "name");
    if (tmp)
      info->sessionFile = g_strdup (tmp);
    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "compress");
    if (tmp)
      info->compress = as_number (tmp);
  }


  getTourSpeedValue (node, doc, "tourSpeed",
                     &sessionOptions->defaultTourSpeed);
  getTourSpeedValue (node, doc, "tour1dSpeed",
                     &sessionOptions->defaultTourSpeed);

  return (0);
}

gboolean
getLogicalPreference (xmlNodePtr node, const char *elName,
                      gboolean defaultValue)
{
  xmlNodePtr el;
  gboolean val = defaultValue;
  el = getXMLElement (node, elName);
  if (el) {
    gchar *tmp;
    tmp = (gchar *) xmlGetProp (el, (xmlChar *) "on");
    if (tmp) {
      val = as_logical (tmp);
    }
    else {
      val = true;
    }
  }
  return (val);
}

gint
getPreviousFiles (const xmlDocPtr doc, GGobiInitInfo * info)
{
  xmlNode *node, *el;
  gint n, i;
  node = getXMLDocElement (doc, "previousFiles");

  if (node == NULL)
    return (0);

  n = 0;
  el = XML_CHILDREN (node);
  while (el) {
    if (el->type != XML_TEXT_NODE)
      n++;
    el = el->next;
  }

  info->descriptions = g_malloc0 (n * sizeof (GGobiDescription));
  info->numInputs = n;

  el = XML_CHILDREN (node);
  for (i = 0; el; el = el->next) {

/* */
    if (el->type != XML_TEXT_NODE) {
/*
 * dfs; trying to get past my compiler
*/
      /*memset((void*) info->descriptions+i, '\0', sizeof(GGobiDescription)); */
      parsePreviousInput (el, info->descriptions+i);
      i++;
    }
  }
  return (n);
}

void
parsePreviousInput (xmlNode * node, GGobiDescription *desc)
{
  GGobiInputSource *source;
  
  xmlChar *mode = xmlGetProp (node, (xmlChar *) "mode");
  xmlChar *uri = xmlGetProp (node, (xmlChar *) "name");
  
  source = create_input_source((const gchar *)uri, (const gchar *)mode);
  
  xmlFree(mode);
  xmlFree(uri);
  
  desc->source = source;
}

/*****************************************************************/

gint
getPreviousGGobiDisplays (const xmlDocPtr doc, GGobiInitInfo * info)
{
  xmlNode *node, *el;
  GGobiDescription *desc = NULL;
  gint i;
  node = getXMLDocElement (doc, "ggobis");
  if (node) {
    el = XML_CHILDREN (node);
    i = 0;
    while (el) {
      if (el->type != XML_TEXT_NODE
          && strcmp ((char *) el->name, "ggobi") == 0) {
        /* Need to match these with the input source ids. */
        desc = info->descriptions + i;
        getPreviousDisplays (el, desc);
        i++;
      }
      el = el->next;
    }
  }

  if (!desc)
    return (-1);

  return (g_list_length (desc->displays));
}

gint
getPreviousDisplays (xmlNodePtr node, GGobiDescription * desc)
{
  xmlNodePtr el = XML_CHILDREN (node);
  GGobiDisplayDescription *dpy;
  gint n = 0;
  desc->displays = NULL;

  while (el) {
    if (el->type != XML_TEXT_NODE
        && strcmp ((char *) el->name, "display") == 0) {
      dpy = getDisplayDescription (el);
      if (dpy) {
        desc->displays = g_list_append (desc->displays, dpy);
        n++;
      }
    }

    el = el->next;
  }

  return (n);
}

GGobiDisplayDescription *
getDisplayDescription (xmlNodePtr node)
{
  GGobiDisplayDescription *dpy;
  xmlNodePtr el;
  gint i;
  xmlChar *tmp;

  dpy =
    (GGobiDisplayDescription *) g_malloc0 (sizeof (GGobiDisplayDescription));
  dpy->canRecreate = true;

  tmp = xmlGetProp (node, (xmlChar *) "type");
  dpy->typeName = g_strdup ((gchar *) tmp);
  tmp = xmlGetProp (node, (xmlChar *) "data");
  if (tmp) {
    dpy->data = atoi ((char *) tmp) - 1;
    if (dpy->data < 0)
      dpy->datasetName = g_strdup ((gchar *) tmp);
  }
  else
    dpy->data = 0;

  if (xmlGetProp (node, (xmlChar *) "unsupported"))
    dpy->canRecreate = false;

  dpy->numVars = 0;


  el = XML_CHILDREN (node);
  while (el) {
    if (el->type != XML_TEXT_NODE
        && strcmp ((char *) el->name, "variable") == 0)
      dpy->numVars++;
    el = el->next;
  }

  dpy->varNames = (gchar **) g_malloc (dpy->numVars * sizeof (gchar *));
  for (i = 0, el = XML_CHILDREN (node); i < dpy->numVars; el = el->next) {
    if (el->type != XML_TEXT_NODE
        && strcmp ((char *) el->name, "variable") == 0) {
      dpy->varNames[i++] =
        g_strdup ((char *) xmlGetProp (el, (xmlChar *) "name"));
    }
  }

  return (dpy);
}




/*****************************************************************/

/*
 Handle the plugins section, looping over each <plugin>
 tag and passing it the appropriate factory.
*/
gint
processPluginNodes (xmlNode * el, GGobiInitInfo * info, xmlDocPtr doc)
{
  GGobiPluginFactory *factory;
  
  if (el == NULL)
    return (-1);

  factory = ggobi_plugin_factory_new();
  
  while (el) {
    GGobiPlugin *plugin;
    if (el->type != XML_TEXT_NODE) {
      if (strcmp ((char *) el->name, "plugin") == 0) {
        if ((plugin = ggobi_plugin_factory_create(factory, el, doc))) {
          if (!g_type_module_use(G_TYPE_MODULE(plugin))) {
            g_critical("Failed to load plugin '%s'", G_TYPE_MODULE(plugin)->name);
            g_object_unref(G_OBJECT(plugin));
          } else {
            g_type_module_unuse(G_TYPE_MODULE(plugin));
            info->plugins = g_list_append (info->plugins, plugin);
          }
        }
      } else 
        g_warning("Element with invalid name '%s' within a 'plugins' element", el->name);
    }
    el = el->next;
  }
  
  g_object_unref(factory);
  
  return (g_list_length(info->plugins));
}

gint
getPlugins (xmlDocPtr doc, GGobiInitInfo * info)
{
  xmlNode *pluginsNode, *pluginNodes = NULL;
  
  pluginsNode = getXMLDocElement (doc, "plugins");
  if (pluginsNode)
     pluginNodes = XML_CHILDREN (pluginsNode);
  else pluginNodes = getXMLDocElement(doc, "plugin");
    
  return (processPluginNodes (pluginNodes, info, doc));
}

/************************************************************************/

gint resolveVariableName (const gchar * name, GGobiStage * d);

displayd *
createExtendedDisplay (const gchar * const type, gint * vars, gint numVars,
                       GGobiStage * d, ggobid * gg)
{
  displayd *dpy;

  GGobiExtendedDisplayClass *klass;
  GType gtype = g_type_from_name (type);
  klass = g_type_class_peek (gtype);
  if (!klass->createWithVars) {
    g_printerr ("Cannot currently handle the extended display %s type.",
                type);
    return (NULL);
  }
  dpy = klass->createWithVars (false, numVars, vars, d, gg);
  if (!dpy)
    return (NULL);

/*XXX does this get done via a callback. */
  display_add (dpy, gg);

  return (dpy);
}


displayd *
createDisplayFromDescription (ggobid * gg, GGobiDisplayDescription * desc)
{
  displayd *dpy = NULL;
  GGobiStage *data = NULL;
  gint *vars, i;

  if (desc->canRecreate == false)
    return (NULL);

  if (desc->data > -1) {
    data = (GGobiStage *) g_slist_nth_data (gg->d, desc->data);
  }
  else if (desc->datasetName && desc->datasetName[0]) {
    GGobiStage *tmp;
    GSList *l;
    for (l = gg->d; l; l = l->next) {
      tmp = (GGobiStage *) l->data;
      if (strcmp (desc->datasetName, tmp->name) == 0) {
        data = tmp;
        break;
      }
    }
  }

  if (!data) {
    g_printerr ("Cannot resolve dataset ");
    if (desc->data > -1)
      g_printerr ("with index %d\n", desc->data + 1);
    else
      g_printerr ("named `%s'\n", desc->datasetName);

    return (NULL);
  }

  vars = (gint *) g_malloc (sizeof (gint) * desc->numVars);
  for (i = 0; i < desc->numVars; i++)
    vars[i] = resolveVariableName (desc->varNames[i], data);

  dpy = createExtendedDisplay (desc->typeName, vars, desc->numVars, data, gg);

  g_free (vars);

  return (dpy);
}


gint
resolveVariableName (const gchar * name, GGobiStage * d)
{
  gint j;

  for (j = 0; j < d->n_cols; j++) {
    if (strcmp (ggobi_stage_get_col_name(d, j), name) == 0)
      return (j);
  }

  return (-1);
}

void
readPluginFile (const char *const fileName, GGobiInitInfo * info)
{
  xmlDocPtr doc;
  
  doc = xmlParseFile (fileName);
  if (doc == NULL) {
    g_critical("Failed to parse the xml file `%s'", fileName);
    return;
  }

  getPlugins (doc, sessionOptions->info);

  xmlFreeDoc (doc);
}


GlyphType
mapGlyphName (const gchar * gtype)
{
  GlyphType type;
  gint i;

  type = UNKNOWN_GLYPH;
  for (i = 0; i < sizeof (GlyphNames) / sizeof (GlyphNames[0]) - 1; i++) {
    if (strcmp (gtype, GlyphNames[i]) == 0) {
      type = (GlyphType) (i);
      break;
    }
  }

  return (type);
}
