#ifndef WRITE_XML_H
#define WRITE_XML_H

#include <stdio.h>

#include "ggobi.h"

#ifdef __cplusplus 
extern "C" {
#endif

gboolean write_xml(const gchar *filename, ggobid *gg);
gboolean write_xml_stream(FILE *f, ggobid *gg, const gchar *filename);
gboolean write_xml_header(FILE *f, ggobid *gg);
gboolean write_xml_description(FILE *f, ggobid *gg);
gboolean write_xml_variables(FILE *f, ggobid *gg);
gboolean write_xml_variable(FILE *f, ggobid *gg, int i);
gboolean write_xml_records(FILE *f, ggobid *gg);
gboolean write_xml_record(FILE *f, ggobid *gg, int i);

gboolean write_xml_edges(FILE *f, ggobid *gg);
gboolean write_xml_edge(FILE *f, ggobid *gg, int i);

gboolean write_dataset_header(FILE *f, ggobid *gg);
gboolean write_dataset_footer(FILE *f, ggobid *gg);

void writeFloat(FILE *f, double value);

#ifdef __cplusplus 
}
#endif

#endif
