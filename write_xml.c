#include "write_xml.h"
/*
  Takes the current data in the specified ggobid structure
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


XmlWriteInfo *updateXmlWriteInfo(datad *d, ggobid *gg, XmlWriteInfo *info);

gboolean
write_xml (const gchar *filename,  ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
  FILE *f;
  gboolean ok = false;
/*
 *datad *d;
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
write_xml_stream (FILE *f, ggobid *gg, const gchar *filename, XmlWriteInfo *xmlWriteInfo)
{
 gint numDatasets, i;
 datad *d;
  numDatasets = g_slist_length(gg->d);
  write_xml_header (f, -1, gg, xmlWriteInfo);

  for(i = 0; i < numDatasets; i++) {
    d = (datad *) g_slist_nth_data(gg->d, i);
    if(xmlWriteInfo->useDefault)
      updateXmlWriteInfo(d, gg, xmlWriteInfo);
    write_xml_dataset(f, d, gg, xmlWriteInfo);
  }

  write_xml_footer(f, gg, xmlWriteInfo);
  return(true);
}

gboolean
write_xml_dataset(FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
    write_dataset_header (f, d, gg, xmlWriteInfo);
    write_xml_description (f, gg, xmlWriteInfo);
    write_xml_variables (f, d, gg, xmlWriteInfo);
    write_xml_records (f, d, gg, xmlWriteInfo);
    /*-- skip for now, because there's no need to write the default edges --*/
    /*    write_xml_edges(f, d, gg);*/
    write_dataset_footer(f, gg, xmlWriteInfo);

  return(true);
}

gboolean
write_xml_header (FILE *f, int numDatasets, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f, "<?xml version=\"1.0\"?>");
 fprintf(f, "\n");
 fprintf(f, "<!DOCTYPE ggobidata SYSTEM \"ggobi.dtd\">");
 fprintf(f, "\n\n");

 if(numDatasets < 0)
    numDatasets = g_slist_length(gg->d);

 fprintf(f, "<ggobidata count=\"%d\">\n", numDatasets);

/* fflush(f);*/

 return(true);
}

gboolean
write_xml_description (FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"<description>\n");
/*XXX*/

 fprintf(f,"</description>\n");

 return(true);
}

gboolean
write_xml_variables (FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
  gint j;


  if (gg->save.column_ind == ALLCOLS) {
    fprintf(f,"<variables count=\"%d\">\n", d->ncols); 
    for(j = 0; j < d->ncols; j++) {
      write_xml_variable (f, d, gg, j, xmlWriteInfo);
      fprintf(f,"\n");
    }
  } else if (gg->save.column_ind == SELECTEDCOLS) {
    /*-- work out which columns to save --*/
    gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
    gint ncols = selected_cols_get (cols, d, gg);

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
write_xml_variable(FILE *f, datad *d, ggobid *gg, gint j,
  XmlWriteInfo *xmlWriteInfo)
{
  vartabled *vt = vartable_element_get (j, d);

  if (vt->vartype == categorical) {
    gint k;
    fprintf(f, "  <categoricalvariable name=\"%s\"",
      (gg->save.stage == TFORMDATA) ? vt->collab_tform : vt->collab);
    fprintf(f, " nickname=\"%s\">\n", vt->nickname);
    fprintf(f, "    <levels count=\"%d\">\n", vt->nlevels);
    for (k=0; k<vt->nlevels; k++) {
      fprintf(f, "      <level value=\"%d\"> %s </level>\n",
	    vt->level_values[k],
        vt->level_names[k]);
    }
    fprintf(f, "    </levels>\n");
    fprintf(f, "  </categoricalvariable>");
  } else if (vt->vartype == real) {
    fprintf(f, "  <realvariable name=\"%s\"",
      (gg->save.stage == TFORMDATA) ? vt->collab_tform : vt->collab);
    fprintf(f, " nickname=\"%s\" />", vt->nickname);
  } else if (vt->vartype == integer) {
    fprintf(f, "  <integervariable name=\"%s\"",
      (gg->save.stage == TFORMDATA) ? vt->collab_tform : vt->collab);
    fprintf(f, " nickname=\"%s\" />", vt->nickname);
  } else if (vt->vartype == counter) {
    fprintf(f, "  <countervariable name=\"%s\"",
      (gg->save.stage == TFORMDATA) ? vt->collab_tform : vt->collab);
    fprintf(f, " nickname=\"%s\" />", vt->nickname);
  }

  return(true);
}

gboolean
write_edge_record_p (gint i, datad *e, ggobid *gg)
{
/*
 * If e is an edge set, then
 * loop over all other datads and test their rowids to decide
 * whether this case should be drawn.  Use sampled and hidden.
*/
  gboolean save_case = true;
  datad *d;
  GSList *l;
  gint a, b;

  if (e->edge.n == e->nrows) {
    for (l = gg->d; l; l=l->next) {
      d = (datad *) l->data;
      if (d != e) {
        if (d->rowid.idv.nels > e->edge.endpoints[i].a &&
            d->rowid.idv.nels > e->edge.endpoints[i].b)
        {
          a = d->rowid.idv.els[e->edge.endpoints[i].a];
          b = d->rowid.idv.els[e->edge.endpoints[i].b];
          if (!d->sampled.els[a] || !d->sampled.els[b] ||
              d->hidden.els[a] || d->hidden.els[b])
          {
            save_case = false;
            break;
          }
        }
      }
    }
  }
  return save_case;
}

gboolean
write_xml_records(FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 gint i, m, n;


 /*-- figure out how many records we're about to save.  --*/
 if (gg->save.row_ind == ALLROWS)
   n = d->nrows;
 else if (gg->save.row_ind == DISPLAYEDROWS) {
   n = 0;
   for (i=0; i<d->nrows_in_plot; i++) {
     if (!d->hidden.els[ d->rows_in_plot[i] ]) {
       if (d->edge.n == d->nrows) {  /* ie, if this datad has edges? */
         if (write_edge_record_p (i, d, gg))
           n++;
       } else {
         n++;
       }
     }
   }
 }

 fprintf(f, "<records ");
 fprintf(f, "count=\"%d\"", n);
 if(xmlWriteInfo->useDefault) {
/*
   fprintf(f, " glyphSize=\"%s\"", xmlWriteInfo->defaultGlyphSizeName);
   fprintf(f, " glyphType=\"%s\"", xmlWriteInfo->defaultGlyphTypeName);
*/
   fprintf(f, " glyph=\"%s %s\"",
    xmlWriteInfo->defaultGlyphTypeName,
    xmlWriteInfo->defaultGlyphSizeName);
   fprintf(f, " color=\"%s\"", xmlWriteInfo->defaultColorName);
 }

 if (d->nmissing > 0) {
   if (gg->save.missing_ind == MISSINGSNA)
     fprintf(f, " missingValue=\"%s\"", "na");
   else if (gg->save.missing_ind == MISSINGSDOT)
     fprintf(f, " missingValue=\"%s\"", ".");
   /*-- otherwise write the "imputed" value --*/
 }
 fprintf(f, ">\n");


 if (gg->save.row_ind == ALLROWS) {
   for (i = 0; i < d->nrows; i++) {
     write_xml_record (f, d, gg, i, xmlWriteInfo);
     fprintf(f, "\n");
   }
 } else {  /*-- if displaying visible rows only --*/
   for (i=0; i<d->nrows_in_plot; i++) {
     m = d->rows_in_plot[i];
     if (!d->hidden.els[m]) {
       if (write_xml_record (f, d, gg, m, xmlWriteInfo))
         fprintf(f, "\n");
     }
   }
 }

 fprintf(f, "</records>\n");
 return(true);
}

gboolean
write_xml_record (FILE *f, datad *d, ggobid *gg, gint i,
  XmlWriteInfo *xmlWriteInfo)
{
  gint j;
  gchar *gstr, *gtypestr = NULL;

  /*-- we only need this test if we're not only saving visible cases --*/
  if (d->edge.n == d->nrows && gg->save.row_ind == DISPLAYEDROWS) {
    if (!write_edge_record_p (i, d, gg))
      return false;
  }

  fprintf(f, "<record");

  /*-- ids if present --*/
  if (d->rowid.id.nels != 0) {
    fprintf(f, " id=\"%d\"", d->rowid.id.els[i]);
  }

  /*-- if the record is hidden, indicate that --*/
  if (gg->save.row_ind == ALLROWS && d->hidden_now.els[i]) {
    fprintf(f, " hidden=\"true\"");
  }

  /*-- edges if present and requested --*/
  if (gg->save.edges_p && d->edge.n == d->nrows) {
    fprintf(f, " source=\"%d\"", d->edge.endpoints[i].a);
    fprintf(f, " destination=\"%d\"", d->edge.endpoints[i].b);
  }

  if(d->rowlab && d->rowlab->data
       && (gstr = (gchar *) g_array_index (d->rowlab, gchar *, i))) {  
     /*-- if the label contains an ampersand, write it as &amp; --*/
     if (strchr (gstr, (gint) '&')) {
       gchar *next = strtok (gstr, "&");
       fprintf(f, " label=\"%s", next);
       while (next) {
         next = strtok(NULL, "&");
         if (next)
           fprintf(f, "&amp;%s", next);
       }
       fprintf(f, "\"");
     } else {
       fprintf(f, " label=\"%s\"", gstr);
     }
  }

 if (!xmlWriteInfo->useDefault ||
     xmlWriteInfo->defaultColor != d->color.els[i])
 {
   fprintf(f, " color=\"%d\"", d->color.els[i]);
 }

/*
  fprintf(f, " glyphSize=\"%d\"", d->glyph[i].size);
  fprintf(f, " glyphType=\"%d\"", d->glyph[i].type);
*/
  if (!xmlWriteInfo->useDefault ||
     xmlWriteInfo->defaultGlyphType != d->glyph.els[i].type ||
     xmlWriteInfo->defaultGlyphSize != d->glyph.els[i].size)
  {
    switch (d->glyph.els[i].type) {
      case PLUS:
        gtypestr = "plus";
      break;
      case X:
        gtypestr = "x";
      break;
      case OR:
        gtypestr = "or";
      break;
      case FR:
        gtypestr = "fr";
      break;
      case OC:
        gtypestr = "oc";
      break;
      case FC:
        gtypestr = "fc";
      break;
      case DOT_GLYPH:
        gtypestr = ".";
      break;
      case UNKNOWN_GLYPH:
      default:
        gtypestr=NULL;
      break;
    }

    fprintf (f, " glyph=\"%s %d\"", gtypestr, d->glyph.els[i].size);
  }

  fprintf(f, ">\n");

  if (gg->save.column_ind == ALLCOLS) {
    for(j = 0; j < d->ncols; j++) {
      /*-- if missing, figure out what to write --*/
      if (d->nmissing > 0 && d->missing.vals[i][j] &&
        gg->save.missing_ind != MISSINGSIMPUTED)
      {
        if (gg->save.missing_ind == MISSINGSNA) {
          fprintf (f, "na ");
        }  else if (gg->save.missing_ind == MISSINGSDOT) {
          fprintf (f, ". ");
        } 
      } else {  /*-- if not missing, just write the data --*/
        writeFloat (f, (gg->save.stage == TFORMDATA) ? d->tform.vals[i][j] :
                                                       d->raw.vals[i][j]);
      }
      if (j < d->ncols-1 )
        fprintf(f, " ");
     }
  } else if (gg->save.column_ind == SELECTEDCOLS) {
    /*-- work out which columns to save --*/
    gint *cols = (gint *) g_malloc (d->ncols * sizeof (gint));
    gint ncols = selected_cols_get (cols, d, gg);
    for(j = 0; j < ncols; j++) {
      if (d->nmissing > 0 && d->missing.vals[i][j] &&
        gg->save.missing_ind != MISSINGSIMPUTED)
      {
        if (gg->save.missing_ind == MISSINGSNA) {
          fprintf (f, "NA ");
        }  else if (gg->save.missing_ind == MISSINGSDOT) {
          fprintf (f, ". ");
        } 
      } else {

        writeFloat (f, (gg->save.stage == TFORMDATA) ? d->tform.vals[i][j] :
                                                       d->raw.vals[i][cols[j]]);
      } 
      if (j < ncols-1 )
        fprintf(f, " ");
     }
     g_free (cols);
   }

  fprintf(f, "\n</record>");

 return (true);
}

gboolean
write_xml_edges (FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 gint i;
 if (d->edge.n < 1)
  return(true);

 fprintf(f, "<edges count=%d>\n", d->edge.n);
 for(i = 0; i < d->edge.n; i++) {
  write_xml_edge(f, d, gg, i, xmlWriteInfo);
  fprintf(f, "\n");
 }
 fprintf(f, "/edges>\n");

 return(true);
}

gboolean
write_xml_edge(FILE *f, datad *d, ggobid *gg, int i, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f, "<edge ");
 fprintf(f, "source=\"%d\" destination=\"%d\"", d->edge.endpoints[i].a
                                              , d->edge.endpoints[i].b);
 fprintf(f, " />");

 return(true);
}

void
writeFloat(FILE *f, double value)
{
 fprintf(f, "%.3f", value);
}

gboolean
write_dataset_header (FILE *f, datad *d, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"<data ");
/*fprintf(f, "numRecords=\"%d\"", d->nrows);*/
 fprintf(f, "name=\"%s\"", d->name);
 fprintf(f,">\n");

 return(true);
}

gboolean
write_dataset_footer(FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"</data>\n");
 return(true);
}

gboolean
write_xml_footer(FILE *f, ggobid *gg, XmlWriteInfo *xmlWriteInfo)
{
 fprintf(f,"</ggobidata>\n");
 return(true);
}

XmlWriteInfo *
updateXmlWriteInfo(datad *d, ggobid *gg, XmlWriteInfo *info)
{
  int i, n, numGlyphSizes;
  gint *colorCounts, *glyphTypeCounts, *glyphSizeCounts, count;
  gchar *str;
  gint ncolors = gg->activeColorScheme->n;

  colorCounts = g_malloc(sizeof(gint) * ncolors);
  glyphTypeCounts = g_malloc(sizeof(gint) * UNKNOWN_GLYPH);
  numGlyphSizes = NGLYPHSIZES;
  glyphSizeCounts = g_malloc(sizeof(gint) * numGlyphSizes);

  memset(colorCounts, '\0', sizeof(gint) * ncolors);
  memset(glyphTypeCounts, '\0', sizeof(gint) * UNKNOWN_GLYPH);
  memset(glyphSizeCounts, '\0', sizeof(gint) * numGlyphSizes);

  n = GGOBI(nrecords)(d);
  for(i = 0 ; i < n ; i++) {
    colorCounts[d->color.els[i]]++;
    glyphSizeCounts[d->glyph.els[i].size]++;
    glyphTypeCounts[d->glyph.els[i].type]++;
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

  str = (gchar *) GGOBI(getGlyphTypeName)(info->defaultGlyphType);  
  info->defaultGlyphTypeName = g_strdup(str);

  return(info);
}
