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
    g_printerr ("ggobi requires a filename or some data from stdin\n");
    exit (0);
  }
}
#endif

void
strip_suffixes (ggobid *gg)
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
    nchars = strlen (gg->filename) - strlen (suffix[i]) ;
    if (strcmp (suffix[i], (gchar *) &gg->filename[nchars]) == 0) {
      gg->fname = g_strndup (gg->filename, nchars);
      foundit = true;
      break;
    }
  }

  if (!foundit)
    gg->fname = g_strdup (gg->filename);
}

void
read_binary (FILE *fp, ggobid *gg)
{
  gint i, j, nr, nc;
  gint onesize = sizeof (gfloat);
  gint out;

  fread ((gchar *) &nr, sizeof (gint), 1, fp);
  fread ((gchar *) &nc, sizeof (gint), 1, fp);
  gg->ncols = nc;

  /*
   * As soon as the number of columns is known, allocate vardata.
  */
  vardata_alloc (gg);
  vardata_init (gg);

  gg->nrows = nr;

  arrayf_alloc (&gg->raw, gg->nrows, gg->ncols);

  for (i=0; i<gg->nrows; i++) {
    for (j=0; j<gg->ncols; j++) {
      out = fread ((gchar *) &gg->raw.vals[i][j], onesize, 1, fp);
      if (out != 1) {
        g_printerr ("problem in reading the binary data file\n");
        fclose (fp);
        exit (0);

      } else if (gg->raw.vals[i][j] == FLT_MAX) {
        gg->raw.vals[i][j] = 0.0;

        /* Allocate the missing values array */
        if (gg->nmissing == 0)
          arrays_alloc (&gg->missing, gg->nrows, gg->ncols);
        gg->missing.vals[i][j] = 1;
        gg->vardata[j].nmissing++;
        gg->nmissing++;
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

gint
row1_read (FILE *fp, gfloat *row1, gshort *row1_missing, ggobid *gg) {

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
  if (!find_data_start (fp))
    found_row = false;

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
          gg->nmissing++;
          row1_missing[ncols] = 1;

        } else {
          row1[ncols] = (gfloat) atof (word);
        }

        ncols++;
        gotone = true;  /*-- suppress the alarm -- the file pointer is ok --*/

        if (gg->ncols >= MAXNCOLS) {
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
read_ascii (FILE *fp, ggobid *gg)
{
  gint j, jrows, nrows, jcols, fs;
  gint nitems;
  gint nblocks;
  gchar word[64];
  gfloat row1[MAXNCOLS];
  gshort row1_missing[MAXNCOLS];

  /*-- Read in the first row of the data and calculate ncols. --*/
  gg->ncols = row1_read (fp, row1, row1_missing, gg);

  /*-- Once the number of columns is known, allocate vardata. --*/
  vardata_alloc (gg);
  vardata_init (gg);

/*
 * allocate the first block.
*/
  gg->nrows = 0;
  arrayf_alloc (&gg->raw, BLOCKSIZE, gg->ncols);
  if (gg->nmissing > 0)
    arrays_alloc_zero (&gg->missing, BLOCKSIZE, gg->ncols);

  /*-- copy the values in row1 to the main array --*/
  for (j=0; j<gg->ncols; j++)
    gg->raw.vals[0][j] = row1[j];
  if (gg->nmissing > 0) {
    for (j=0; j<gg->ncols; j++)
      gg->missing.vals[0][j] = row1_missing[j];
  }


/*-- Read, reallocating as needed.  Determine nrows for the read_all case. --*/
  nblocks = 1;
  nitems = gg->ncols;
  jrows = 1;
  nrows = 1;
  jcols = 0;
  while (true) {
    if (jcols == 0) {
      if (!find_data_start (fp))
        break;
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

        if (gg->nmissing == 0) {
          /*
           * When the first "na" or "." has been encountered,
           * allocate space to contain the missing values matrix.
           * Initialize all previous values to 0.
          */
          arrays_alloc (&gg->missing, nblocks*BLOCKSIZE, gg->ncols);
        }

        gg->nmissing++;
        gg->vardata[jcols].nmissing++;
        gg->missing.vals[nrows][jcols] = 1;
        gg->raw.vals[nrows][jcols] = 0.0;
      }
      else {  /*-- not missing --*/
        gg->raw.vals[nrows][jcols] = (gfloat) atof (word);
      }

      jcols++;
      if (jcols == gg->ncols)  /*-- we just completed a row --*/
      {
        jcols = 0;
        nrows++;
        jrows++;
      }

      if (jrows == BLOCKSIZE) {
        jrows = 0;
        nblocks++;
        if (nblocks%20 == 0)
          g_printerr ("reallocating; n > %d\n", nblocks*BLOCKSIZE);

        arrayf_add_rows (&gg->raw, nblocks*BLOCKSIZE);
        if (gg->nmissing > 0)
          arrays_add_rows (&gg->missing, nblocks*BLOCKSIZE);
      }
    }
  }

  /*-- Close the data file --*/
  if (fclose (fp) == EOF)
    g_printerr ("read_ascii: error in fclose");

  gg->nrows = nrows;

  g_print ("size of data: %d x %d\n", gg->nrows, gg->ncols);

  if (nitems != gg->nrows * gg->ncols) {
    g_printerr ("read_ascii: nrows*ncols != nitems read\n");
    g_printerr ("(nrows= %d, ncols= %d, nitems read= %d)\n",
      gg->nrows, gg->ncols, nitems);
    exit (0);
  } else if (nitems == 0) {
    g_printerr ("No data was read\n");
    exit (0);
  }
  else {  /*-- nitems ok --*/
    /*
     * One last free and realloc to make these arrays take up exactly
     * the amount of space they need.
    */
    arrayf_free (&gg->raw, gg->nrows, gg->ncols);
    if (gg->nmissing)
      arrays_free (&gg->missing, gg->nrows, gg->ncols);
  }

  /*-- finally handle the 1-column case: insert a column --*/
  gg->single_column = (gg->ncols == 1);
  if (gg->single_column) {
    gint i;
    arrayf_add_cols (&gg->raw, 2);
    for (i=0; i<gg->nrows; i++) {
      gg->raw.vals[i][1] = gg->raw.vals[i][0];
      gg->raw.vals[i][0] = (gfloat) (i+1);
    }

    if (gg->nmissing) {
      arrays_add_cols (&gg->missing, 2);
      for (i=0; i<gg->nrows; i++) {
        gg->missing.vals[i][1] = gg->missing.vals[i][0];
        gg->missing.vals[i][0] = 0;
      }
    }
  }

}

/*----------------------------------------------------------------------*/
/*              End of section on reading ascii files                   */
/*----------------------------------------------------------------------*/

void
array_read (ggobid *gg)
{
  gchar fname[128];
  FILE *fp;

/*
 * Check file exists and open it - for stdin no open needs to be done
 * only assigning fp to be stdin.
*/
  if (strcmp ((gchar *) gg->fname, "stdin") == 0) {
    fp = stdin;

#ifndef _WIN32
    /*
     * If reading from stdin, set an alarm.  If no data has 
     * been read after 3 seconds, print an error message and exit.
    */
    alarm ((unsigned) 3);
    signal (SIGALRM, stdin_empty);
#endif

    read_ascii (fp, gg);
  }
  else
  {
    /*
     * Try fname.bin before fname, to see whether there is a binary
     * data file available.  If there is, call read_binary ().
    */
    strcpy (fname, (gchar *) gg->fname);
    strcat (fname, ".bin");

    if ((fp = fopen (fname, "rb")) != NULL)
      read_binary (fp, gg);

    /*
     * If not, look for an ASCII file
    */
    else {
      static gchar *suffixes[] = {"", ".dat"};
      if ( (fp=open_ggobi_file_r (gg->fname, 2, suffixes, false)) != NULL)
        read_ascii (fp, gg);
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

