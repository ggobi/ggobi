#ifndef WRITE_XML_H
#define WRITE_XML_H

#include <stdio.h>

#include "ggobi.h"
#include "datad.h"

#ifdef __cplusplus 
extern "C" {
#endif

gboolean write_xml (const gchar *, datad *, ggobid *);
gboolean write_xml_stream (FILE *f, datad *, ggobid *gg, const gchar *);
gboolean write_xml_header (FILE *f, ggobid *gg);
gboolean write_xml_description (FILE *f, ggobid *gg);
gboolean write_xml_variables (FILE *f, datad *, ggobid *gg);
gboolean write_xml_variable (FILE *f, datad *, ggobid *gg, gint i);
gboolean write_xml_records (FILE *f, datad *d, ggobid *gg);
gboolean write_xml_record (FILE *f, datad *d, ggobid *gg, gint i);

gboolean write_xml_edges(FILE *f, ggobid *gg);
gboolean write_xml_edge(FILE *f, ggobid *gg, gint i);

gboolean write_dataset_header(FILE *f, datad *, ggobid *gg);
gboolean write_dataset_footer(FILE *f, ggobid *gg);

void writeFloat(FILE *f, double value);

#ifdef __cplusplus 
}
#endif

#endif
