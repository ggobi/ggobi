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

int getPreviousFiles(const xmlDocPtr doc, GGobiInitInfo *info);
xmlNode *getXMLElement(const xmlDocPtr doc, const char *tagName);
DataMode getPreviousInput(xmlNode *node, InputDescription *input);
DataMode getInputType(xmlNode *node);


GGobiInitInfo *
read_init_file(const char *filename)
{
  xmlDocPtr  doc;
  GGobiInitInfo *info;

  xmlSubstituteEntitiesDefault(1);   
  xmlDoValidityCheckingDefaultValue = false;
  filename = g_strdup(filename);
  doc = xmlParseFile(filename); 
  if(doc == NULL)
      return((GGobiInitInfo *) NULL);
  info = g_malloc(sizeof(GGobiInitInfo));

  info->numInputs = 0;
  info->inputs = NULL;
  info->filename = g_strdup(filename);

  getPreviousFiles(doc, info);

  return(info);
}

    /* Find a node with a secified tag Name */
xmlNode *
getXMLElement(const xmlDocPtr doc, const char *tagName)
{
    xmlNode *node = xmlDocGetRootElement(doc);
    node = node->children;
    while(node) {
	if(strcmp(node->name, tagName) == 0) {
            return(node);
	}
	node = node->next;
    }

  return(node);
}

int
getPreviousFiles(const xmlDocPtr doc, GGobiInitInfo *info)
{
    xmlNode *node, *el;
    int n, i;
    node = getXMLElement(doc, "previousFiles");

    n = 0;
    el = node->children;
    while(el) {
        if(el->type != XML_TEXT_NODE)
	  n++;
        el = el->next;
    }

    info->inputs = g_malloc(n*sizeof(InputDescription));
    info->numInputs = n;

    el = node->children;
    for(i = 0; el ; el = el->next) {
	if(el->type != XML_TEXT_NODE) {
	  getPreviousInput(el, info->inputs + i);
          i++;
	}
    }
    return(n);
}


DataMode
getPreviousInput(xmlNode *node, InputDescription *input)
{
   const char *tmp;
   DataMode mode = getInputType(node);
   input->mode = mode;
   if((tmp = xmlGetProp(node, "name"))) {
     input->fileName = g_strdup(tmp);
   }


   /* This shold be connected to 
        completeFileDesc(input->fileName, input);
   */
   if(input->fileName) {
       char *ptr, *tmp1, *tmp2;
       int i;
         tmp1 = strrchr(input->fileName, G_DIR_SEPARATOR);
         tmp2 = strrchr(tmp1, '.');
         if(tmp2)
            input->givenExtension = g_strdup(tmp2+1);
	 input->baseName = g_malloc((tmp2 - tmp1 +1)*sizeof(char));
         for(i = 0, ptr = tmp1 + 1 ; ptr < tmp2; ptr++, i++) {   
	     input->baseName[i] = *ptr;
	 }
	 input->baseName[i] = '\0';
         input->dirName = g_malloc((tmp1 - input->fileName +1)*sizeof(char));
         for(i=0, ptr = input->fileName; ptr < tmp1; ptr++, i++) {   
	     input->dirName[i] = *ptr;
	 }
         input->dirName[i] = '\0';
     }

   input->canVerify = 0;

   return(mode);
}

DataMode
getInputType(xmlNode *node)
{
    const CHAR *tag;
    const CHAR *mode;
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

