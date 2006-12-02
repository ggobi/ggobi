#include <string.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <gtk/gtk.h>
#include "writedata.h"
#include "vars.h"
#include "ggobi-variable.h"
#include "externs.h"

gboolean
write_csv_header (gint *cols, gint ncols, FILE *f, GGobiStage *d, ggobid *gg)
{
  gboolean ok = true;
  gint j, jcol, rval;

  fprintf (f, "\"\",");
  for (j=0; j<ncols; j++) {
    jcol = cols[j];
    rval = fprintf (f, "\"%s\"", 
             g_strstrip((gg->save.stage == TFORMDATA) ? ggobi_stage_get_col_name(d, jcol) : ggobi_stage_get_col_name(d, jcol)));
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
write_csv_cell(gint i, gint j, FILE *f, GGobiStage *d, ggobid *gg)
{
  gchar* value = ggobi_stage_get_string_value(d, i, j);

  switch (ggobi_stage_get_col_type(d, j)) {
  case GGOBI_VARIABLE_CATEGORICAL:
    fprintf(f, "\"%s\"", value);
    break;
  default:
    fprintf(f, "%s", value);
  }
}

gboolean
write_csv_record (gint i, gint *cols, gint ncols, FILE *f, GGobiStage *d, ggobid *gg)
{
  gboolean ok = true;
  gint j, jcol;

  fprintf(f, "\"%s\",", ggobi_stage_get_row_id(d, i));

  /* Source and destination, as strings, if edges are present */
  if (gg->save.edges_p && ggobi_stage_get_n_edges(d)) {
    fprintf(f, "\"%s\",", g_strstrip(ggobi_stage_get_edge_data(d)->sym_endpoints->a));
    fprintf(f, "\"%s\",", g_strstrip(ggobi_stage_get_edge_data(d)->sym_endpoints->b));
  }

  /* record */  
  for(j = 0; j < ncols; j++) {
    jcol = cols[j];
  
    /*-- if missing, figure out what to write --*/
    if (ggobi_stage_is_missing(d, i, jcol) && gg->save.missing_ind != MISSINGSIMPUTED)
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
write_csv_records (gint *cols, gint ncols, FILE *f, GGobiStage *d, ggobid *gg)
{
  gboolean ok = true;
  gint i;

  if (gg->save.row_ind == ALLROWS) {
    for (i = 0; i < d->n_rows; i++) {
      write_csv_record (i, cols, ncols, f, d, gg);
      fprintf(f, "\n");
    }
  } else { 
    /*-- if displaying visible rows only --*/
    d = ggobi_stage_find(d, GGOBI_MAIN_STAGE_FILTER); 
    for (i=0; i<d->n_rows; i++) {
      write_csv_record (i, cols, ncols, f, d, gg);
      fprintf(f, "\n");
    }
  }

  return ok;
}



gboolean
write_csv_file (FILE *f, GGobiStage *d, ggobid *gg)
{
  gboolean ok = false;
  gint j;
  gint *cols, ncols;

  ncols = 0;
  if (gg->save.column_ind == ALLCOLS) {
    cols = (gint *) g_malloc (d->n_cols * sizeof(gint));
    for(j = 0; j < d->n_cols; j++) {
      cols[j] = j;
      ncols++;
    }
  } else { // gg->save.column_ind==SELECTEDCOLS
    /*-- work out which columns to save --*/
    ncols = selected_cols_get (&cols, d, gg);
    if (ncols == 0)  { // backup source of column selection
      cols = (gint *) g_malloc (d->n_cols * sizeof (gint));
      ncols = plotted_cols_get (cols, d, gg);
    }
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
  GGobiStage *d = NULL;
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
