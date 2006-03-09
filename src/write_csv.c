#include <string.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>
#include "writedata.h"
#include "vars.h"
#include "vartable.h"
#include "externs.h"

static void
writeFloat(FILE *f, double value)
{
  fprintf(f, "%g", value); 
}
static void
writeInteger(FILE *f, double value)
{
  fprintf(f, "%d", (gint)value); 
}

gboolean
write_csv_header (gint *cols, gint ncols, FILE *f, GGobiData *d, ggobid *gg)
{
  gboolean ok = true;
  gint j, jcol, rval;
  vartabled *vt;

  fprintf (f, "\"\",");
  for (j=0; j<ncols; j++) {
    jcol = cols[j];
    vt = vartable_element_get (jcol, d);
    rval = fprintf (f, "\"%s\"", 
             g_strstrip((gg->save.stage == TFORMDATA) ? vt->collab_tform : vt->collab));
    if (rval < 0) {
      ok = false;
      break;
    }
    if (j < ncols-1)
      fprintf(f, ",");
  }
  fprintf (f, "\n");

  return ok;
}

void
write_csv_cell(gint i, gint j, FILE *f, GGobiData *d, ggobid *gg)
{
  vartabled *vt;
  gchar *lname;

  vt = vartable_element_get (j, d);

  switch (vt->vartype) {
  case categorical:
    lname = g_strstrip(level_name_from_tform_value(i, j, vt, d));
    fprintf(f, "\"%s\"", lname);
    break;
  case integer:
  case counter:
    writeInteger (f, (gg->save.stage == TFORMDATA) ? d->tform.vals[i][j] :
                                                   d->raw.vals[i][j]);
    break;
  case uniform:
  case real:
    writeFloat (f, (gg->save.stage == TFORMDATA) ? d->tform.vals[i][j] :
                                                   d->raw.vals[i][j]);
    break;
  }
}

gboolean
write_csv_record (gint i, gint *cols, gint ncols, FILE *f, GGobiData *d, ggobid *gg)
{
  gboolean ok = true;
  gchar *gstr;
  gint j, jcol;

  /*-- row label if present; else index --*/
  if (d->rowlab && d->rowlab->data
      && (gstr = (gchar *) g_array_index (d->rowlab, gchar *, i))) 
  { 
    fprintf(f, "\"%s\",", g_strstrip(gstr));
  } else
    fprintf(f, "\"%d\",", i);

  /* Source and destination, as strings, if edges are present */
  if (gg->save.edges_p && d->edge.n) {
    fprintf(f, "\"%s\",", g_strstrip(d->edge.sym_endpoints->a));
    fprintf(f, "\"%s\",", g_strstrip(d->edge.sym_endpoints->b));
  }

  /* record */  
  for(j = 0; j < ncols; j++) {
    jcol = cols[j];
  
    /*-- if missing, figure out what to write --*/
    if (d->nmissing > 0 && d->missing.vals[i][jcol] &&
      gg->save.missing_ind != MISSINGSIMPUTED)
    {
      if (gg->save.missing_ind == MISSINGSNA) {
        fprintf (f, "na");
      }  else if (gg->save.missing_ind == MISSINGSDOT) {
        fprintf (f, ".");
      } 
    } else {  /*-- if not missing, just write the data --*/
      write_csv_cell (i, jcol, f, d, gg);
    }
    if (j < ncols-1 )
      fprintf(f, ",");
  }

  return ok;
}

gboolean
write_csv_records (gint *cols, gint ncols, FILE *f, GGobiData *d, ggobid *gg)
{
  gboolean ok = true;
  gint i, m;

  if (gg->save.row_ind == ALLROWS) {
    for (i = 0; i < d->nrows; i++) {
      write_csv_record (i, cols, ncols, f, d, gg);
      fprintf(f, "\n");
    }
  } else {  /*-- if displaying visible rows only --*/
    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot.els[i];
      write_csv_record (m, cols, ncols, f, d, gg);
      fprintf(f, "\n");
    }
  }

  return ok;
}



gboolean
write_csv_file (FILE *f, GGobiData *d, ggobid *gg)
{
  gboolean ok = false;
  gint j;
  gint *cols, ncols;

  ncols = 0;
  if (gg->save.column_ind == ALLCOLS) {
    cols = (gint *) g_malloc (d->ncols * sizeof(gint));
    for(j = 0; j < d->ncols; j++) {
      cols[j] = j;
      ncols++;
    }
  } else if (gg->save.column_ind == SELECTEDCOLS) {
    /*-- work out which columns to save --*/
    cols = (gint *) g_malloc (d->ncols * sizeof (gint));
    ncols = selected_cols_get (cols, d, gg);
    if (ncols == 0)  // backup source of column selection
      ncols = plotted_cols_get (cols, d, gg);
  }

  if (ncols) {
    if (write_csv_header (cols, ncols, f, d, gg))
      if (write_csv_records (cols, ncols, f, d, gg))
        ok = true;

    g_free (cols);
  }

  return ok;
}

gboolean
write_csv (const gchar *filename,  ggobid *gg)
{
  FILE *f;
  gboolean ok = false;
  GGobiData *d = NULL;
  gint nd = g_slist_length(gg->d);;

  /* By default, write only a single datad */
  if (nd > 0) {
    if (nd == 1)
      d = gg->d->data;
    else {
      if (gg->current_display->d != NULL)
        d = gg->current_display->d; 
    }

    if (d) {
      f = fopen (filename, "w");
      if (f) {
        if (write_csv_file (f, d, gg))
          ok = true;

        fclose(f);
      }
    }
  }
  return ok;
}
