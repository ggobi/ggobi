#include "write_xml.h"
/*
  Takes the current data in the specified ggobid structure
  and writes the XML file corresponding to this configuration.
  This can be used to convert the original data formats to XML, if
  that is desired.
  (More likely to be correct than writing a Perl script!)
 */

gboolean
write_xml(const gchar *filename, ggobid *gg)
{
  FILE *f;
  gboolean ok = false;

  f = fopen (filename,"w");
  if (f == NULL) {
   return (false);
  }

  ok = write_xml_stream(f, gg, filename);
  fclose(f);
  return ok;
}

gboolean
write_xml_stream(FILE *f, ggobid *gg, const gchar *filename)
{
  write_xml_header(f, gg);
  write_dataset_header(f, gg);
  write_xml_description(f, gg);
  write_xml_variables(f, gg);
  write_xml_records(f, gg);
/*-- skip for now, because there's no need to write the default segments --*/
/*    write_xml_segments(f, gg);*/
  write_dataset_footer(f, gg);

  return(true);
}

gboolean
write_xml_header(FILE *f, ggobid *gg)
{

 fprintf(f, "<?xml version=\"1.0\"?>");
 fprintf(f, "\n");
 fprintf(f, "<!DOCTYPE xgobidata SYSTEM \"xgobi.dtd\">");
 fprintf(f, "\n\n");

/* fflush(f);*/

 return(true);
}

gboolean
write_xml_description(FILE *f, ggobid *gg)
{
 fprintf(f,"<description>\n");


 fprintf(f,"</description>\n");

 return(true);
}

gboolean
write_xml_variables(FILE *f, ggobid *gg)
{
 int i;
 fprintf(f,"<variables count=\"%d\">\n", gg->ncols); 

  for(i = 0; i < gg->ncols; i++) {
   write_xml_variable(f, gg, i);
   fprintf(f,"\n");
  }

 fprintf(f,"</variables>\n"); 

 return(true);
}

gboolean
write_xml_variable(FILE *f, ggobid *gg, int i)
{
/*
   fprintf(f, "<variable");
   fprintf(f," name=\"%s\"", gg->vardata[i].collab);
   if(strcmp(gg->vardata[i].collab, gg->vardata[i].collab_tform) != 0) {
     fprintf(f," transformName=\"%s\"", gg->vardata[i].collab_tform);
   }
   fprintf(f, " />");
*/

  fprintf(f, "<variable>");
  fprintf(f,"%s", gg->vardata[i].collab);
  fprintf(f, "</variable>");

  return(true);
}

gboolean
write_xml_records(FILE *f, ggobid *gg)
{
 int i;
 fprintf(f, "<records\n");
 fprintf(f, " numRecords=\"%d\"", gg->nrows);
 fprintf(f, ">\n");
 for(i = 0; i < gg->nrows; i++) {
  write_xml_record(f, gg, i);
  fprintf(f, "\n");
 }
 fprintf(f, "</records>\n");

 return(true);
}

gboolean
write_xml_record(FILE *f, ggobid *gg, int i)
{
  int j;
  gchar *gstr;
  fprintf(f, "<record");

  fprintf(f, " label=\"%s\"", gg->rowlab[i]);
  fprintf(f, " color=\"%d\"", gg->color_ids[i]);
/*
  fprintf(f, " glyphSize=\"%d\"", gg->glyph_ids[i].size);
  fprintf(f, " glyphType=\"%d\"", gg->glyph_ids[i].type);
*/
  switch (gg->glyph_ids[i].type) {
    case PLUS_GLYPH:
      gstr = "+";
      break;
    case X_GLYPH:
      gstr = "x";
      break;
    case OPEN_RECTANGLE:
      gstr = "or";
      break;
    case FILLED_RECTANGLE:
      gstr = "fr";
      break;
    case OPEN_CIRCLE:
      gstr = "oc";
      break;
    case FILLED_CIRCLE:
      gstr = "fc";
      break;
    case POINT_GLYPH:
      gstr = ".";
      break;
  }

  fprintf(f, " glyph=\"%s %d\"", gstr, gg->glyph_ids[i].size);

  fprintf(f, ">\n");

  for(j = 0; j < gg->ncols; j++) {
     writeFloat (f, gg->raw.data[i][j]);
     if (j < gg->ncols-1 )
       fprintf(f, " ");
  }
  fprintf(f, "\n</record>");

 return (true);
}

gboolean
write_xml_segments(FILE *f, ggobid *gg)
{
 int i;
 if(gg->nsegments < 1)
  return(true);

 fprintf(f, "<segments count=%d>\n", gg->nsegments);
 for(i = 0; i < gg->nsegments; i++) {
  write_xml_segment(f, gg, i);
  fprintf(f, "\n");
 }
 fprintf(f, "/segments>\n");

 return(true);
}

gboolean
write_xml_segment(FILE *f, ggobid *gg, int i)
{
 fprintf(f, "<segment ");
 fprintf(f, "source=\"%d\" destination=\"%d\"", gg->segment_endpoints[i].a
                                              , gg->segment_endpoints[i].b);
 fprintf(f, " />");

 return(true);
}

void
writeFloat(FILE *f, double value)
{
 fprintf(f, "%.3f", value);
}

gboolean
write_dataset_header(FILE *f, ggobid *gg)
{
 fprintf(f,"<ggobidata ");
 fprintf(f, "numRecords=\"%d\"", gg->nrows);
 fprintf(f,">\n");

 return(true);
}

gboolean
write_dataset_footer(FILE *f, ggobid *gg)
{
 fprintf(f,"</ggobidata>\n");
 return(true);
}
