#include "write_xml.h"
/*
  Takes the current data in the specified ggobid structure
  and writes the XML file corresponding to this configuration.
  This can be used to convert the original data formats to XML, if
  that is desired.
  (More likely to be correct than writing a Perl script!)
 */

gboolean
write_xml (const gchar *filename,  ggobid *gg)
{
  FILE *f;
  gboolean ok = false;
  datad *d;
  GSList *tmp = gg->d;
  f = fopen (filename,"w");
  if (f == NULL) {
   return (false);
  }

  write_xml_stream (f, gg, filename);

  fclose(f);
  return ok;
}

gboolean
write_xml_stream (FILE *f, ggobid *gg, const gchar *filename)
{
 gint numDatasets, i;
 datad *d;
  numDatasets = g_slist_length(gg->d);
  write_xml_header (f, -1, gg);

  for(i = 0; i < numDatasets; i++) {
    d = (datad *) g_slist_nth_data(gg->d, i);
    write_xml_dataset(f, d, gg);
  }

  write_xml_footer(f, gg);
  return(true);
}

gboolean
write_xml_dataset(FILE *f, datad *d, ggobid *gg)
{
    write_dataset_header (f, d, gg);
    write_xml_description (f, gg);
    write_xml_variables (f, d, gg);
    write_xml_records (f, d, gg);
    /*-- skip for now, because there's no need to write the default edges --*/
    /*    write_xml_edges(f, gg);*/
    write_dataset_footer(f, gg);

  return(true);
}

gboolean
write_xml_header (FILE *f, int numDatasets, ggobid *gg)
{

 fprintf(f, "<?xml version=\"1.0\"?>");
 fprintf(f, "\n");
 fprintf(f, "<!DOCTYPE xgobidata SYSTEM \"xgobi.dtd\">");
 fprintf(f, "\n\n");

 if(numDatasets < 0)
    numDatasets = g_slist_length(gg->d);

 fprintf(f, "<ggobidata count=\"%d\">\n", numDatasets);

/* fflush(f);*/

 return(true);
}

gboolean
write_xml_description (FILE *f, ggobid *gg)
{
 fprintf(f,"<description>\n");


 fprintf(f,"</description>\n");

 return(true);
}

gboolean
write_xml_variables (FILE *f, datad *d, ggobid *gg)
{
  gint i;
  fprintf(f,"<variables count=\"%d\">\n", d->ncols); 

  for(i = 0; i < d->ncols; i++) {
    write_xml_variable (f, d, gg, i);
    fprintf(f,"\n");
  }

  fprintf(f,"</variables>\n"); 

  return(true);
}

gboolean
write_xml_variable(FILE *f, datad *d, ggobid *gg, gint i)
{
/*
   fprintf(f, "<variable");
   fprintf(f," name=\"%s\"", gg->vartable[i].collab);
   if(strcmp(gg->vartable[i].collab, gg->vartable[i].collab_tform) != 0) {
     fprintf(f," transformName=\"%s\"", gg->vartable[i].collab_tform);
   }
   fprintf(f, " />");
*/

  fprintf(f, "<variable>");
  fprintf(f,"%s", d->vartable[i].collab);
  fprintf(f, "</variable>");

  return(true);
}

gboolean
write_xml_records(FILE *f, datad *d, ggobid *gg)
{
 int i;
 fprintf(f, "<records\n");
 fprintf(f, " count=\"%d\"", d->nrows);
 fprintf(f, ">\n");
 for(i = 0; i < d->nrows; i++) {
  write_xml_record(f, d, gg, i);
  fprintf(f, "\n");
 }
 fprintf(f, "</records>\n");

 return(true);
}

gboolean
write_xml_record (FILE *f, datad *d, ggobid *gg, gint i)
{
  gint j;
  gchar *gstr;
  fprintf(f, "<record");

  if(d->rowlab && d->rowlab->data
       && (gstr = (gchar *) g_array_index (d->rowlab, gchar *, i))) {  
     fprintf(f, " label=\"%s\"", gstr);
  }
  fprintf(f, " color=\"%d\"", d->color_ids.els[i]);
/*
  fprintf(f, " glyphSize=\"%d\"", d->glyph_ids[i].size);
  fprintf(f, " glyphType=\"%d\"", d->glyph_ids[i].type);
*/
  switch (d->glyph_ids[i].type) {
    case PLUS_GLYPH:
/*      gstr = "+";*/
      gstr = "plus";
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

  fprintf(f, " glyph=\"%s %d\"", gstr, d->glyph_ids[i].size);

  fprintf(f, ">\n");

  for(j = 0; j < d->ncols; j++) {
     writeFloat (f, d->raw.vals[i][j]);
     if (j < d->ncols-1 )
       fprintf(f, " ");
  }
  fprintf(f, "\n</record>");

 return (true);
}

gboolean
write_xml_edges (FILE *f, ggobid *gg)
{
 int i;
 if(gg->nedges < 1)
  return(true);

 fprintf(f, "<edges count=%d>\n", gg->nedges);
 for(i = 0; i < gg->nedges; i++) {
  write_xml_edge(f, gg, i);
  fprintf(f, "\n");
 }
 fprintf(f, "/edges>\n");

 return(true);
}

gboolean
write_xml_edge(FILE *f, ggobid *gg, int i)
{
 fprintf(f, "<edge ");
 fprintf(f, "source=\"%d\" destination=\"%d\"", gg->edge_endpoints[i].a
                                              , gg->edge_endpoints[i].b);
 fprintf(f, " />");

 return(true);
}

void
writeFloat(FILE *f, double value)
{
 fprintf(f, "%.3f", value);
}

gboolean
write_dataset_header (FILE *f, datad *d, ggobid *gg)
{
 fprintf(f,"<data ");
 fprintf(f, "numRecords=\"%d\"", d->nrows);
 fprintf(f,">\n");

 return(true);
}

gboolean
write_dataset_footer(FILE *f, ggobid *gg)
{
 fprintf(f,"</data>\n");
 return(true);
}

gboolean
write_xml_footer(FILE *f, ggobid *gg)
{
 fprintf(f,"</ggobidata>\n");
 return(true);
}
