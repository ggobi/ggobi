#ifndef WRITE_XML_H
#define WRITE_XML_H

#include <stdio.h>

#include "vars.h"
#include "externs.h"

#include <gtk/gtk.h>

#ifdef __cplusplus 
extern "C" {
#endif


typedef struct {

 gboolean useDefault;
 gint   defaultGlyphSize;
 gint   defaultGlyphType;
 gchar *defaultGlyphTypeName;
 gchar *defaultGlyphSizeName;
 gshort defaultColor;
 gchar *defaultColorName;

} XmlWriteInfo;




gboolean write_xml (const gchar *filename, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_stream (FILE *f, ggobid *gg, const gchar *, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_dataset(FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_header (FILE *f, int numDatasets, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_footer(FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_description (FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_variables (FILE *f, datad *, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_variable (FILE *f, datad *, ggobid *gg, gint i, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_records (FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_record (FILE *f, datad *d, ggobid *gg, gint i, XmlWriteInfo *xmlWriteInfo);

gboolean write_xml_edges(FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_edge(FILE *f, ggobid *gg, gint i, XmlWriteInfo *xmlWriteInfo);

gboolean write_dataset_header(FILE *f, datad *, ggobid *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_dataset_footer(FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo);

void writeFloat(FILE *f, double value);

#ifdef __cplusplus 
}
#endif

#endif
