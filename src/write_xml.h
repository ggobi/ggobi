#ifndef WRITE_XML_H
#define WRITE_XML_H

#include <stdio.h>

#include <libxml/parser.h>
#include "vars.h"
#include "externs.h"

#include <gtk/gtk.h>

  /*
    An instance of this structure is passed to all the routines
    that are called when writing the XML representation of a
    GGobiSession object (its datad elements). 
    Thus it can be used to store additional information that
    may be needed by the output engine for particular elements
    and attributes.
    For example, we might add the description text here.
    Also, we might add the precision (number of digits after
    the decimal point) here and look at it in writeDouble().
    Or we might have an `inline' field to indicate whether the colormap
    should be output directly into the file/stream or separately.
    So basically, this can also hold options.
   */

typedef struct {

 gboolean useDefault;
 gint   defaultGlyphSize;
 gint   defaultGlyphType;
 gchar *defaultGlyphTypeName;
 gchar *defaultGlyphSizeName;
 gshort defaultColor;
 gchar *defaultColorName;

} XmlWriteInfo;




gboolean write_xml (const gchar *filename, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_stream (FILE *f, GGobiSession *gg, const gchar *, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_dataset(FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_header (FILE *f, int numDatasets, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_footer(FILE *f, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_description (FILE *f, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_variables (FILE *f, GGobiStage *, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_variable (FILE *f, GGobiStage *, GGobiSession *gg, gint i, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_records (FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_record (FILE *f, GGobiStage *d, GGobiSession *gg, gint i, XmlWriteInfo *xmlWriteInfo);

gboolean write_xml_edges(FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_xml_edge(FILE *f, GGobiStage *d, GGobiSession *gg, gint i, XmlWriteInfo *xmlWriteInfo);

gboolean write_dataset_header(FILE *f, GGobiStage *, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);
gboolean write_dataset_footer(FILE *f, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo);

#endif
