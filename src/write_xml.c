/* write_xml.c */
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

#include "write_xml.h"
/*
  Takes the current data in the specified GGobiSession structure
  and writes the XML file corresponding to this configuration.
  This can be used to convert the original data formats to XML, if
  that is desired.  (More likely to be correct than writing a Perl
  script!)  Alas, it can't be relied upon for all file conversions,
  because ggobi doesn't read all xgobi-style data files, with
  .lines as a prominent example.  ggobi also doesn't read
  xgobi-style .colors files.
 */
#include <string.h>
#include <strings.h>
#include "writedata.h"

extern const gchar* const GlyphNames[];

XmlWriteInfo *updateXmlWriteInfo(GGobiStage *d, GGobiSession *gg, XmlWriteInfo *info);

/* if a string contains an ampersand, write it as &amp; ... etc ... --*/
static void
write_xml_string(FILE *f, const gchar *str)
{
  gchar *fmtstr = g_markup_printf_escaped("%s", str);
  fprintf(f, fmtstr);
  g_free(fmtstr);
}

gboolean
write_xml (const gchar *filename,  GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
  FILE *f;
  gboolean ok = false;
/*
 *GGobiStage *d;
 *GSList *tmp = gg->d;
*/
  f = fopen (filename,"w");
  if (f == NULL) {
   return (false);
  }

  write_xml_stream (f, gg, filename, xmlWriteInfo);

  fclose(f);
  return ok;
}

gboolean
write_xml_stream (FILE *f, GGobiSession *gg, const gchar *filename, XmlWriteInfo *xmlWriteInfo)
{
 gint numDatasets, i;
 GGobiStage *d;
 numDatasets = g_slist_length(gg->d);
g_printerr ("numDatasets %d\n", numDatasets);

 write_xml_header (f, -1, gg, xmlWriteInfo);

  for(i = 0; i < numDatasets; i++) {
    d = (GGobiStage *) g_slist_nth_data(gg->d, i);
    if(xmlWriteInfo->useDefault)
      updateXmlWriteInfo(d, gg, xmlWriteInfo);
    write_xml_dataset(f, d, gg, xmlWriteInfo);
  }

  write_xml_footer(f, gg, xmlWriteInfo);
  return(true);
}

gboolean
write_xml_dataset(FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
  if (ggobi_stage_get_n_edges(d) && !ggobi_stage_has_vars(d)) {
    write_xml_edges(f, d, gg, xmlWriteInfo);
  } else {
    write_dataset_header (f, d, gg, xmlWriteInfo);
    write_xml_description (f, gg, xmlWriteInfo);
    write_xml_variables (f, d, gg, xmlWriteInfo);
    write_xml_records (f, d, gg, xmlWriteInfo);
    write_dataset_footer(f, gg, xmlWriteInfo);
  }

  return(true);
}

gboolean
write_xml_header (FILE *f, int numDatasets, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f, "<?xml version=\"1.0\"?>");
 fprintf(f, "\n");
 fprintf(f, "<!DOCTYPE GGobiSessionata SYSTEM \"ggobi.dtd\">");
 fprintf(f, "\n\n");

 if(numDatasets < 0)
    numDatasets = g_slist_length(gg->d);

 fprintf(f, "<GGobiSessionata count=\"%d\">\n", numDatasets);

/* fflush(f);*/

 return(true);
}

gboolean
write_xml_description (FILE *f, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"<description>\n");
/*XXX*/
 fprintf(f, "This is XML created by GGobi\n");

 fprintf(f,"</description>\n");

 return(true);
}

gboolean
write_xml_variables (FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
  gint j;

  if (gg->save.column_ind == ALLCOLS) {
    fprintf(f,"<variables count=\"%d\">\n", d->n_cols); 
    for(j = 0; j < d->n_cols; j++) {
      write_xml_variable (f, d, gg, j, xmlWriteInfo);
      fprintf(f,"\n");
    }
  } else if (gg->save.column_ind == SELECTEDCOLS) {
    /*-- work out which columns to save --*/
    gint *cols;
    gint ncols = selected_cols_get (&cols, d, gg);
    if (ncols == 0) {
      cols = (gint *) g_malloc (d->n_cols * sizeof (gint));
      ncols = plotted_cols_get (cols, d, gg);
    }
    fprintf(f,"<variables count=\"%d\">\n", ncols); 
    for(j = 0; j < ncols; j++) {
      write_xml_variable (f, d, gg, cols[j], xmlWriteInfo);
      fprintf(f,"\n");
    }

    g_free (cols);
  }

  fprintf(f,"</variables>\n"); 

  return(true);
}

gboolean
write_xml_variable(FILE *f, GGobiStage *d, GGobiSession *gg, gint j,
  XmlWriteInfo *xmlWriteInfo)
{
  GGobiVariable *var = ggobi_stage_get_variable(d, j);
  gchar* varname = g_strstrip(
    (gg->save.stage == TFORMDATA) ? 
      ggobi_stage_get_col_name(d, j) : 
      ggobi_stage_get_col_name(d, j)
  );
  
  if (GGOBI_VARIABLE_IS_CATEGORICAL(var)) {
    gint k, *values;
    fprintf(f, "  <categoricalvariable name=\"%s\"", ggobi_variable_get_name(var));
    if (ggobi_variable_get_nickname(var))
      fprintf(f, " nickname=\"%s\"", ggobi_variable_get_nickname(var));
    fprintf(f, ">\n");
    fprintf(f, "    <levels count=\"%d\">\n", ggobi_variable_get_n_levels(var));
    values = ggobi_variable_get_level_values(var);
    for (k=0; k<ggobi_variable_get_n_levels(var); k++) {
      fprintf(f, "      <level value=\"%d\">", values[k]);
      /* Add any needed html/xml markup to level names */
      write_xml_string(f, ggobi_variable_get_level_name(var, values[k]));
      fprintf(f, "</level>\n");
    }

    fprintf(f, "    </levels>\n");
    fprintf(f, "  </categoricalvariable>");
  } else {
    GGobiVariableType type = ggobi_variable_get_vartype(var);
    gchar *nickname = ggobi_variable_get_nickname(var);
    fprintf(f, "   <");
    if (type == GGOBI_VARIABLE_REAL)    fprintf(f, "realvariable");
    else if (type == GGOBI_VARIABLE_INTEGER) fprintf(f, "integervariable");
    else if (type == GGOBI_VARIABLE_COUNTER) fprintf(f, "countervariable");
    fprintf(f, " name=\"%s\"", varname);
    if (nickname) {
      fprintf(f, " nickname=\"%s\"", nickname);
      g_free(nickname);
    }
    fprintf(f, "/>");
  } 

  return(true);
}

static void
writeFloat(FILE *f, double value)
{
  /*fprintf(f, "%.3f", value);*/
  fprintf(f, "%g", value); 
}

gboolean
write_xml_records(FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
  gint i, n = 0;

  /* use the filter stage if only saving the visible rows */
  if (gg->save.row_ind == DISPLAYEDROWS)
    d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_FILTER);

  n = d->n_rows;
  
  fprintf(f, "<records ");
  fprintf(f, "count=\"%d\"", n);
  if(xmlWriteInfo->useDefault) {
    fprintf(f, " glyph=\"%s %s\"",
      xmlWriteInfo->defaultGlyphTypeName,
      xmlWriteInfo->defaultGlyphSizeName);
    fprintf(f, " color=\"%s\"", xmlWriteInfo->defaultColorName);
  }

  if (ggobi_stage_has_missings(d)) {
    if (gg->save.missing_ind == MISSINGSNA)
      fprintf(f, " missingValue=\"%s\"", "na");
    else if (gg->save.missing_ind == MISSINGSDOT)
      fprintf(f, " missingValue=\"%s\"", ".");
    /*-- otherwise write the "imputed" value --*/
  }
  fprintf(f, ">\n");


  for (i = 0; i < d->n_rows; i++) {
    fprintf(f, "<record");
    write_xml_record (f, d, gg, i, xmlWriteInfo);
    fprintf(f, "\n</record>\n");
  }

  fprintf(f, "</records>\n");
  return(true);
}


/*
 * I want this to write <edge> records as well as <record> records.
*/
gboolean
write_xml_record (FILE *f, GGobiStage *d, GGobiSession *gg, gint i,
  XmlWriteInfo *xmlWriteInfo)
{
  gint j;
  
  if (gg->save.stage == TFORMDATA)
    d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_DISPLAY);
  
  /*-- ids if present --*/
  fprintf(f, " id=\"%s\"", ggobi_stage_get_row_id(d, i));

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  /*-- if the record is hidden, indicate that --*/
  if (GGOBI_STAGE_GET_ATTR_HIDDEN(d, i)) {
    fprintf(f, " hidden=\"true\"");
  }

  /*-- edges if present and requested --*/
  if (gg->save.edges_p && ggobi_stage_get_n_edges(d) && i < ggobi_stage_get_n_edges(d)) {
    fprintf(f, " source=\"%s\"", ggobi_stage_get_edge_data(d)->sym_endpoints[i].a);
    fprintf(f, " destination=\"%s\"", ggobi_stage_get_edge_data(d)->sym_endpoints[i].b);
  }

  if (!xmlWriteInfo->useDefault ||
      xmlWriteInfo->defaultColor != GGOBI_STAGE_GET_ATTR_COLOR(d, i))
  {
    fprintf(f, " color=\"%d\"", (gint) GGOBI_STAGE_GET_ATTR_COLOR(d, i));
  }


  if (!xmlWriteInfo->useDefault ||
     xmlWriteInfo->defaultGlyphType != GGOBI_STAGE_GET_ATTR_TYPE(d, i) ||
     xmlWriteInfo->defaultGlyphSize != GGOBI_STAGE_GET_ATTR_SIZE(d, i)) 
  {
    fprintf (f, " glyph=\"%s %d\"", 
      GlyphNames[ GGOBI_STAGE_GET_ATTR_TYPE(d, i)], 
      (gint) GGOBI_STAGE_GET_ATTR_SIZE(d, i));
  }

  fprintf(f, ">\n");

  if (gg->save.column_ind == ALLCOLS && ggobi_stage_has_vars(d)) {
    for(j = 0; j < d->n_cols; j++) {
      /*-- if missing, figure out what to write --*/
      if (ggobi_stage_is_missing(d, i, j) &&
        gg->save.missing_ind != MISSINGSIMPUTED)
      {
        if (gg->save.missing_ind == MISSINGSNA) {
          fprintf (f, "na ");
        }  else if (gg->save.missing_ind == MISSINGSDOT) {
          fprintf (f, ". ");
        } 
      } else {  /*-- if not missing, just write the data --*/
        writeFloat (f, ggobi_stage_get_raw_value(d, i, j));
      }
      if (j < d->n_cols-1 )
        fprintf(f, " ");
     }
  } else if (gg->save.column_ind == SELECTEDCOLS && ggobi_stage_has_vars(d)) {
    /*-- work out which columns to save --*/
    gint *cols;
    gint ncols = selected_cols_get (&cols, d, gg);
    if (ncols == 0) {
      cols = (gint *) g_malloc (d->n_cols * sizeof (gint));
      ncols = plotted_cols_get (cols, d, gg);
    }
    for(j = 0; j < ncols; j++) {
      if (ggobi_stage_is_missing(d, i, j) &&
        gg->save.missing_ind != MISSINGSIMPUTED)
      {
        if (gg->save.missing_ind == MISSINGSNA) {
          fprintf (f, "NA ");
        }  else if (gg->save.missing_ind == MISSINGSDOT) {
          fprintf (f, ". ");
        } 
      } else {

        writeFloat (f, ggobi_stage_get_raw_value(d, i, cols[j]));
      } 
      if (j < ncols-1 )
        fprintf(f, " ");
     }
     g_free (cols);
   }

 return (true);
}

gboolean
write_xml_edges (FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
  gint i;
  if (!ggobi_stage_get_n_edges(d))
    return true;

  /*
  fprintf(f, "<edges count=\"%d\" name=\"%s\">\n", ggobi_stage_get_n_edges(d),
  d->name); There seems to be a need to write the defaults in the case
  where we started with ascii data and added edges; in this case the
  edges are written as <edges> rather than as a new datad, and maybe
  that's the problem.   dfs
  */
  fprintf(f, 
    "<edges count=\"%d\" name=\"%s\" color=\"%d\" glyphType=\"%s\" glyphSize=\"%s\">\n",
    ggobi_stage_get_n_edges(d), d->name, 
    xmlWriteInfo->defaultColor,
    xmlWriteInfo->defaultGlyphTypeName, 
    xmlWriteInfo->defaultGlyphSizeName);

  for(i = 0; i < ggobi_stage_get_n_edges(d); i++) {
    fprintf(f, "<edge");
    write_xml_record (f, d, gg, i, xmlWriteInfo);
    fprintf(f, "</edge>\n");
  }
  fprintf(f, "</edges>\n");

 return(true);
}

/*
gboolean
write_xml_edge(FILE *f, GGobiStage *d, GGobiSession *gg, int i, XmlWriteInfo *xmlWriteInfo)
{
  fprintf(f, " <edge ");
  fprintf(f, "source=\"%s\" destination=\"%s\"", ggobi_stage_get_edge_data(d)->sym_endpoints[i].a
                                               , ggobi_stage_get_edge_data(d)->sym_endpoints[i].b);
  fprintf(f, " />");

  return(true);
}
*/

gboolean
write_dataset_header (FILE *f, GGobiStage *d, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"<data ");
/*fprintf(f, "numRecords=\"%d\"", d->n_rows;*/
 fprintf(f, "name=\"%s\"", d->name);
 fprintf(f,">\n");

 return(true);
}

gboolean
write_dataset_footer(FILE *f, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"</data>\n");
 return(true);
}

gboolean
write_xml_footer(FILE *f, GGobiSession *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"</GGobiSessionata>\n");
 return(true);
}

XmlWriteInfo *
updateXmlWriteInfo(GGobiStage *d, GGobiSession *gg, XmlWriteInfo *info)
{
  int i, n, numGlyphSizes;
  gint *colorCounts, *glyphTypeCounts, *glyphSizeCounts, count;
  gchar *str;
  gint ncolors = gg->activeColorScheme->n;

  colorCounts = g_malloc0(sizeof(gint) * ncolors);
  glyphTypeCounts = g_malloc0(sizeof(gint) * UNKNOWN_GLYPH);
  numGlyphSizes = NGLYPHSIZES;
  glyphSizeCounts = g_malloc0(sizeof(gint) * numGlyphSizes);

  GGOBI_STAGE_ATTR_INIT_ALL(d);  
  n = ggobi_nrecords(d);
  for(i = 0 ; i < n ; i++) {
    colorCounts[ GGOBI_STAGE_GET_ATTR_COLOR(d, i)]++;
    glyphSizeCounts[ GGOBI_STAGE_GET_ATTR_SIZE(d, i)]++;
    glyphTypeCounts[ GGOBI_STAGE_GET_ATTR_TYPE(d, i)]++;
  }

  count = -1;
  for(i = 0 ; i < ncolors; i++) {
    if(colorCounts[i] > count) {
      info->defaultColor = i;
      count= colorCounts[i];
    }
  }

  count = -1;
  for(i = 0 ; i < UNKNOWN_GLYPH; i++) {
    if(glyphTypeCounts[i] > count) {
      info->defaultGlyphType = i;
      count= glyphTypeCounts[i];
    }
  }

  count = -1;
  for(i = 0 ; i < numGlyphSizes; i++) {
    if(glyphSizeCounts[i] > count) {
      info->defaultGlyphSize = i;
      count= glyphSizeCounts[i];
    }
  }

/*
  if(gg->colorNames && (str = gg->colorNames[info->defaultColor]) && str[0])
    info->defaultColorName = g_strdup(str);
  else {
*/
    info->defaultColorName = str = g_malloc(5 * sizeof(char));
    sprintf(str, "%d", info->defaultColor);
/*
  }
*/

  info->defaultGlyphSizeName = str = g_malloc(5 * sizeof(char));
  sprintf(str, "%d", info->defaultGlyphSize);

  info->defaultGlyphTypeName = g_strdup(GlyphNames[info->defaultGlyphType]);

  return(info);
}
