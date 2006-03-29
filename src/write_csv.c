#include <string.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>
#include "writedata.h"
#include "vars.h"
#include "vartable.h"
#include "externs.h"

gboolean
write_csv_header (gint *cols, gint ncols, FILE *f, GGobiData *d, ggobid *gg)
{
  gboolean ok = true;
  gint j, jcol, rval;

  fprintf (f, "\"\",");
  for (j=0; j<ncols; j++) {
    jcol = cols[j];
    rval = fprintf (f, "\"%s\"", 
             g_strstrip((gg->save.stage == TFORMDATA) ? ggobi_data_get_transformed_col_name(d, jcol) : ggobi_data_get_col_name(d, jcol)));
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
  gchar* value = ggobi_data_get_string_value(d, i, j, gg->save.stage == TFORMDATA);

  switch (ggobi_data_get_col_type(d, j)) {
  case categorical:
    fprintf(f, "\"%s\"", value);
    break;
  default:
    fprintf(f, "%s", value);
  }
}

gboolean
write_csv_record (gint i, gint *cols, gint ncols, FILE *f, GGobiData *d, ggobid *gg)
{
  gboolean ok = true;
  gint j, jcol;

  fprintf(f, "\"%s\",", ggobi_data_get_row_id(d, i));

  /* Source and destination, as strings, if edges are present */
  if (gg->save.edges_p && d->edge.n) {
    fprintf(f, "\"%s\",", g_strstrip(d->edge.sym_endpoints->a));
    fprintf(f, "\"%s\",", g_strstrip(d->edge.sym_endpoints->b));
  }

  /* record */  
  for(j = 0; j < ncols; j++) {
    jcol = cols[j];
  
    /*-- if missing, figure out what to write --*/
    if (ggobi_data_is_missing(d, i, jcol) && gg->save.missing_ind != MISSINGSIMPUTED)
    {
      switch (gg->save.missing_ind) {
      case MISSINGSNA:
        fprintf (f, "NA");
        break;
      default: 
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
  } else { // gg->save.column_ind==SELECTEDCOLS
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
  }

  if (d) {
    f = fopen (filename, "w");
    if (f) {
      if (write_csv_file (f, d, gg))
        ok = true;

      fclose(f);
    }
  }
  return ok;
}
