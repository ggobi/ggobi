/* read_array.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define BLOCKSIZE 1000

static gboolean gotone = false;

#ifndef _WIN32
static void
stdin_empty (gint arg)
{
  if (!gotone) {
    g_printerr ("xgobi requires a filename or some data from stdin\n");
    exit (0);
  }
}
#endif

void
strip_suffixes ()
{
/*
 * Find the name of the data file excluding certain suffixes:
 * .bin, .dat
*/
  gint i, nchars;
  gboolean foundit = false;
  gint nsuffixes = 2;
  static gchar *suffix[] = {
    ".bin",
    ".dat"
  };

  for (i=0; i<nsuffixes; i++) {
    nchars = strlen (xg.filename) - strlen (suffix[i]) ;
    if (strcmp (suffix[i], (gchar *) &xg.filename[nchars]) == 0) {
      xg.fname = g_strndup (xg.filename, nchars);
      foundit = true;
      break;
    }
  }

  if (!foundit)
    xg.fname = g_strdup (xg.filename);
}

void
read_binary (FILE *fp)
{
  gint i, j, nr, nc;
  gint onesize = sizeof (gfloat);
  gint out;

  fread ((gchar *) &nr, sizeof (gint), 1, fp);
  fread ((gchar *) &nc, sizeof (gint), 1, fp);
  xg.ncols = nc;

  /*
   * As soon as the number of columns is known, allocate vardata.
  */
  vardata_alloc ();
  vardata_init ();

  xg.nrows = (xg.file_read_type == read_all) ?  nr : xg.file_sample_size;

  xg.file_rows_sampled = (glong *) g_realloc (xg.file_rows_sampled,
                                              xg.nrows * sizeof (glong));

  arrayf_alloc (&xg.raw, xg.nrows, xg.ncols);

  if (xg.file_read_type == read_block) {
    fseek (fp,
      (glong) (sizeof (glong) * 2 + onesize * (xg.file_start_row * xg.ncols)),
      SEEK_SET);
  }

  if (xg.file_read_type == read_all) {

    for (i=0; i<xg.nrows; i++) {
      for (j=0; j<xg.ncols; j++) {
        out = fread ((gchar *) &xg.raw.data[i][j], onesize, 1, fp);
        if (out != 1) {
          g_printerr ("problem in reading the binary data file\n");
          fclose (fp);
          exit (0);

        } else if (xg.raw.data[i][j] == FLT_MAX) {
          xg.raw.data[i][j] = 0.0;

          /* Allocate the missing values array */
          if (xg.nmissing == 0)
            arrays_alloc (&xg.missing, xg.nrows, xg.ncols);
          xg.missing.data[i][j] = 1;
          xg.vardata[j].nmissing++;
          xg.nmissing++;
        }
      }
    }

  } else if (xg.file_read_type == read_block) {
    gint q;

    for (i=0, q=xg.file_start_row; i<xg.nrows; i++, q++) {
      xg.file_rows_sampled[i] = q;
      for (j=0; j<xg.ncols; j++) {
        out = fread ((gchar *) &xg.raw.data[i][j], onesize, 1, fp);
        if (out != 1) {
          g_printerr ("problem in reading the binary data file\n");
          fclose (fp);
          exit (0);

        } else if (xg.raw.data[i][j] == FLT_MAX) {
          xg.raw.data[i][j] = 0.0;
          /* Allocate the missing values array */
          if (xg.nmissing == 0)
            arrays_alloc (&xg.missing, xg.nrows, xg.ncols);
          xg.missing.data[i][j] = 1;
          xg.vardata[j].nmissing++;
          xg.nmissing++;
        }
      }
    }

  } else if (xg.file_read_type == draw_sample) {

    gint t, m, n;
    gfloat rrand;

    /* This is the number of rows we will sample */
    n = xg.nrows;
    if (n > 0 && n < xg.file_length) {

      for (t=0, m=0; t<xg.file_length && m<n; t++) {

        rrand = (gfloat) randvalue ();

        if ( ((gfloat)(xg.file_length - t) * rrand) < (gfloat)(n - m) )
        {
          /* seek forward in the file to row to be read */
          if (fseek (fp,
              (glong)(sizeof (glong)*2 + onesize * t * xg.ncols),
              SEEK_SET) == 0)
          {
            xg.file_rows_sampled[m] = t;

            for (j=0; j<xg.ncols; j++) {
              out = fread ((gchar *) &xg.raw.data[i][j], onesize, 1, fp);
              if (out != 1) {
                g_printerr ("problem in reading the binary data file\n");
                fclose (fp);
                exit (0);
              } else if (xg.raw.data[i][j] == FLT_MAX) {
                xg.raw.data[i][j] = 0.0;

                /*-- allocate the gshort missing values array --*/
                arrays_alloc_zero (&xg.missing, xg.nrows, xg.ncols);
                xg.missing.data[i][j] = 1;
                xg.vardata[j].nmissing++;
                xg.nmissing++;
              }
            }
            m++;
          }
        }
      }
    }
  }

  if (fclose (fp) == EOF)
    g_printerr ("binary_read: error in fclose");
}

/*------------------------------------------------------------------*/
/*          Reading ascii files                                     */
/*------------------------------------------------------------------*/


gboolean
find_data_start (FILE *fp)
{
  gint ch;
  gboolean morelines = true;
  gboolean comment_line = true;

  /*-- Don't attempt to handle comments when the data is from stdin --*/
  if (fp == stdin)
/**/return true;

  while (comment_line) {
    /* skip white space */
    while (1) {
      ch = getc (fp);
      if (ch == '\t' || ch == ' ' || ch == '\n')
        ;
      else
        break;
    }

    /*
     * If we've crept up on an EOF, set morelines to False.
    */
    if (ch == EOF)
    {
      morelines = false;
      break;
    }

    /* Comment lines must begin with a punctuation character */
    else if (ispunct (ch) && ch != '-' && ch != '+' && ch != '.')
    {
      g_printerr ("Skipping a comment line beginning with '%c'\n", ch);
      while ((ch = getc (fp)) != '\n')
        ;
    }
    else if (isalpha (ch) && ch != 'n' && ch != 'N')
    {
      g_printerr ("Comment lines must begin with # or %%;\n");
      g_printerr ("I found a line beginning with '%c'\n", ch);
      exit (1);
    }
    else
    {
      comment_line = false;
      ungetc (ch, fp);
    }
  }

  return (morelines);
}

static void
init_file_rows_sampled () {
  gint i, k, t, m, n;
  gfloat rrand;

  switch (xg.file_read_type) {
    case read_all:
      xg.file_rows_sampled = NULL;
      break;

    case read_block:
      xg.nrows = xg.file_sample_size;
      xg.file_rows_sampled = (gulong *) g_realloc (xg.file_rows_sampled,
                                                   xg.nrows * sizeof (gulong));
      for (i=0, k=xg.file_start_row; i<xg.nrows; i++, k++)
        xg.file_rows_sampled[i] = k;
      break;

    case draw_sample:

      xg.nrows = xg.file_sample_size;
      xg.file_rows_sampled = (gulong *) g_realloc (xg.file_rows_sampled,
                                                   xg.nrows * sizeof (gulong));

      n = xg.nrows;
      if (n > 0 && n < xg.file_length) { 
        for (t=0, m=0; t<xg.file_length && m<n; t++) {

          rrand = (gfloat) randvalue ();
          if ( ((gfloat)(xg.file_length - t) * rrand) < (gfloat)(n - m) )
            xg.file_rows_sampled[m++] = t;
        }
      }
      break;

    default:
      g_printerr ("Impossible value for xg.file_read_type: %d\n",
        xg.file_read_type);
  }
}

static gboolean
seek_to_file_row (gint array_row, FILE *fp) {
  gint i, ch, file_row;
  static gint prev_file_row = 0;
  gboolean ok = true;

  if (array_row >= xg.file_sample_size) {
    return false;
  }

  /* Identify the row number of the next file row we want. */
  file_row = xg.file_rows_sampled[array_row];

  for (i=prev_file_row; i<file_row; i++) {
    if (!find_data_start (fp)) {
      ok = false;
      break;
    } else {
      /* skip a line */
      while ((ch = getc (fp)) != '\n') {
        ;
      }
    }
  }

  /* add one to step over the one we're about to read in */
  prev_file_row = file_row+1; 

  return ok;
}

gint
row1_read (FILE *fp, gfloat *row1, gshort *row1_missing) {

  gint j, ch;
  gboolean found_row = true;
  gint ncols = 0;
  gchar word[64];

  /*-- Initialize --*/
  for (j=0; j<MAXNCOLS; j++) {
    row1_missing[j] = 0;
    row1[j] = 0.0;
  }

  /*-- Find the index of the first row of data that we're interested in. --*/
  if (xg.file_read_type == read_all) {
    if (!find_data_start (fp))
      found_row = false;

  } else {  /* if -only was used on the command line */
    if (!seek_to_file_row (0, fp))
      found_row = false;
  }

  if (found_row) {  /*-- start reading the first line --*/
    while ((ch = getc(fp)) != '\n') {
  
      if (ch == '\t' || ch == ' ')
        ;

      else if (ungetc (ch, fp) == EOF || fscanf (fp, "%s", word) < 0 ) {
        g_printerr ("error in reading first row of data\n");
        fclose (fp);
        exit (0);

      } else {

        if (g_strcasecmp (word, "na") == 0 || strcmp (word, ".") == 0) {
          xg.nmissing++;
          row1_missing[ncols] = 1;

        } else {
          row1[ncols] = (gfloat) atof (word);
        }

        ncols++;
        gotone = true;  /*-- suppress the alarm -- the file pointer is ok --*/

        if (xg.ncols >= MAXNCOLS) {
          g_printerr (
            "This file has more than %d columns.  In order to read\n", MAXNCOLS);
          g_printerr (" it in, increase MAXNCOLS in defines.h and recompile.\n");
          exit (0);
        }
      }
    }
  }

  return ncols;
}

void
read_ascii (FILE *fp)
{
  gint j, jrows, nrows, jcols, fs;
  gint nitems;
  gint nblocks;
  gchar word[64];
  gfloat row1[MAXNCOLS];
  gshort row1_missing[MAXNCOLS];

  init_file_rows_sampled ();

  /*-- Read in the first row of the data and calculate ncols. --*/
  xg.ncols = row1_read (fp, row1, row1_missing);

  /*-- Once the number of columns is known, allocate vardata. --*/
  vardata_alloc ();
  vardata_init ();

/*
 * If we're reading everything, allocate the first block.
 * If -only has been used, allocate the whole shebang.
*/
  if (xg.file_read_type == read_all) {

    xg.nrows = 0;
    arrayf_alloc (&xg.raw, BLOCKSIZE, xg.ncols);
    if (xg.nmissing > 0)
      arrays_alloc_zero (&xg.missing, BLOCKSIZE, xg.ncols);

  } else {  /* -only has been used */

    xg.nrows = xg.file_sample_size;
    arrayf_alloc (&xg.raw, xg.nrows, xg.ncols);
    if (xg.nmissing > 0)
      arrays_alloc_zero (&xg.missing, xg.nrows, xg.ncols);
  }

  /*-- copy the values in row1 to the main array --*/
  for (j=0; j<xg.ncols; j++)
    xg.raw.data[0][j] = row1[j];
  if (xg.nmissing > 0) {
    for (j=0; j<xg.ncols; j++)
      xg.missing.data[0][j] = row1_missing[j];
  }


/*-- Read, reallocating as needed.  Determine nrows for the read_all case. --*/
  nblocks = 1;
  nitems = xg.ncols;
  jrows = 1;
  nrows = 1;
  jcols = 0;
  while (true) {
    if (jcols == 0) {
      if (xg.file_read_type == read_all) {
        if (!find_data_start (fp))
          break;

      } else {  /* if -only was used on the command line */
        if (!seek_to_file_row (nrows, fp))
          break;
      }
    }

    fs = fscanf (fp, "%s", word);

    if (fs == EOF) {
      break;
    } else if (fs < 0) {
      g_printerr ("Problem with input data\n");
      fclose (fp);
      exit (0);
    }
    else {
      nitems++;

      if (g_strcasecmp (word, "na") == 0 || strcmp (word, ".") == 0) {

        if (xg.nmissing == 0) {
          /*
           * When the first "na" or "." has been encountered,
           * allocate space to contain the missing values matrix.
           * Initialize all previous values to 0.
          */
          if (xg.file_read_type == read_all) {
            arrays_alloc (&xg.missing, nblocks*BLOCKSIZE, xg.ncols);
          } else {
            arrays_alloc (&xg.missing, xg.nrows, xg.ncols);
          }
        }

        xg.nmissing++;
        xg.vardata[jcols].nmissing++;
        xg.missing.data[nrows][jcols] = 1;
        xg.raw.data[nrows][jcols] = 0.0;
      }
      else {  /*-- not missing --*/
        xg.raw.data[nrows][jcols] = (gfloat) atof (word);
      }

      jcols++;
      if (jcols == xg.ncols)  /*-- we just completed a row --*/
      {
        jcols = 0;
        nrows++;
        jrows++;
      }

      if (xg.file_read_type == read_all) {
        if (jrows == BLOCKSIZE) {
          jrows = 0;
          nblocks++;
          if (nblocks%20 == 0)
            g_printerr ("reallocating; n > %d\n", nblocks*BLOCKSIZE);

          arrayf_add_rows (&xg.raw, nblocks*BLOCKSIZE);
          if (xg.nmissing > 0)
            arrays_add_rows (&xg.missing, nblocks*BLOCKSIZE);
        }

      } else {  /* -only was used */
        if (nrows >= xg.nrows)
          break;
      }
    }
  }

  /*-- Close the data file --*/
  if (fclose (fp) == EOF)
    g_printerr ("read_ascii: error in fclose");

  if (xg.file_read_type == read_all)
    xg.nrows = nrows;

  g_print ("size of data: %d x %d\n", xg.nrows, xg.ncols);

  if (nitems != xg.nrows * xg.ncols) {
    g_printerr ("read_ascii: nrows*ncols != nitems read\n");
    g_printerr ("(nrows= %d, ncols= %d, nitems read= %d)\n",
      xg.nrows, xg.ncols, nitems);
    exit (0);
  } else if (nitems == 0) {
    g_printerr ("No data was read\n");
    exit (0);
  }
  else {  /*-- nitems ok --*/
    if (xg.file_read_type == read_all) {
      /*
       * One last free and realloc to make these arrays take up exactly
       * the amount of space they need.
      */
      arrayf_free (&xg.raw, xg.nrows, xg.ncols);
      arrays_free (&xg.missing, xg.nrows, xg.ncols);
    }
  }
}

/*----------------------------------------------------------------------*/
/*              End of section on reading ascii files                   */
/*----------------------------------------------------------------------*/

void
array_read ()
{
  gchar fname[128];
  FILE *fp;

/*
 * Check file exists and open it - for stdin no open needs to be done
 * only assigning fp to be stdin.
*/
  if (strcmp ((gchar *) xg.fname, "stdin") == 0) {
    fp = stdin;

#ifndef _WIN32
    /*
     * If reading from stdin, set an alarm.  If no data has 
     * been read after 3 seconds, print an error message and exit.
    */
    alarm ((unsigned) 3);
    signal (SIGALRM, stdin_empty);
#endif

    read_ascii (fp);
  }
  else
  {
    /*
     * Try fname.bin before fname, to see whether there is a binary
     * data file available.  If there is, call read_binary ().
    */
    strcpy (fname, (gchar *) xg.fname);
    strcat (fname, ".bin");

    if ((fp = fopen (fname, "rb")) != NULL)
      read_binary (fp);

    /*
     * If not, look for an ASCII file
    */
    else {
      static gchar *suffixes[] = {"", ".dat"};
      if ( (fp=open_xgobi_file_r (xg.fname, 2, suffixes, false)) != NULL)
        read_ascii (fp);
      else
        exit (1);
        
    }
  }
}

void
find_root_name_of_data (gchar *fname, gchar *title)
/*
 * Strip off preceding directory names and find the final filename
 * to use in resetting the title and iconName.
*/
{
  gchar *pf;
  gint j = 0;

  pf = fname;
  while (fname[j] != '\0') {
    if (fname[j] == G_DIR_SEPARATOR)
      pf = &fname[j+1];
    j++;
  }

  title = g_strdup_printf (pf);
}

