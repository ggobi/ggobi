#ifndef WRITE_STATE_H
#define WRITE_STATE_H

#include "ggobi.h"

void write_ggobi_as_xml(ggobid *gg, const char *fileName);
gboolean saveDOMToFile(xmlDocPtr doc, const char *fileName);
gboolean ggobi_write_session(const char *fileName);
xmlDocPtr create_ggobi_xml(ggobid *gg, xmlDocPtr doc);

#endif 
