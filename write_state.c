/*
 This is used to display the state of a ggobi instance and its sub-elements:
 its displays, their splots in XML format.
 */

#include <libxml/tree.h>
#include "ggobi.h"

#include <math.h>


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
xmlDocPtr create_ggobi_xml(ggobid *gg);


const char *getDisplayTypeName(displayd *dpy);
const char *getDataName(displayd *dpy);


 /*
  Write an XML representation of the given ggobid instance
  to the specified filename.
 */
void 
write_ggobi_as_xml(ggobid *gg, const char *fileName)
{
  xmlDocPtr doc;
    doc = create_ggobi_xml(gg);
#if 0
    xmlSaveFileEnc(fileName, doc, NULL); 
#else
    xmlSaveFile(fileName, doc); 
#endif
    xmlFreeDoc(doc);
}


/*
 Create the XML document tree (DOM) representing the
 given ggobid. This contains the descriptions of the displays.
 */
xmlDocPtr
create_ggobi_xml(ggobid *gg)
{
 GList *els, *el;
 xmlDocPtr doc;
 xmlNodePtr node;

 els = gg->displays;
 if(!els) 
     return(NULL);

     /* Create the document */
 doc = xmlNewDoc("1.0");
 XML_DOC_ROOT(doc) = xmlNewDocNode(doc, NULL, "ggobi", NULL);

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
