/* read_array.c */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include "vars.h"

/* external functions */
extern FILE *open_xgobi_file_r (gchar *, gint, gchar **, gboolean);
extern void init_missing_array ();
extern gdouble randvalue ();
/*                    */

/* Not yet here yet from the original:  make_scatmat, Sread_array */

#define BLOCKSIZE 1000

gboolean gotone = false;

/*ARGSUSED*/
static void
stdin_empty (gint arg)
{
  if (!gotone) {
    g_printerr ("xgobi requires a filename or some data from stdin\n");
    exit (0);
  }
}

void
strip_suffixes ()
{
/*
 * Find the name of the data file excluding certain suffixes:
 * .bin, .missing, .dat
*/
  gint i, nchars;
  gboolean foundit = false;
  static gchar *suffix[] = {
    ".bin",
    ".missing",
    ".dat"
  };

g_print ("(strip_suffixes) filename = %s\n", xg.filename);

  for (i=0; i<3; i++) {
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

  fread ((gchar *) &nr, sizeof (nr), 1, fp);
  fread ((gchar *) &nc, sizeof (nc), 1, fp);

  xg.ncols = nc;

  if (xg.file_read_type == read_all)
    xg.nrows = nr;
  else
    xg.nrows = xg.file_sample_size;

  xg.file_rows_sampled = (glong *) g_malloc (xg.nrows * sizeof (glong));

  xg.raw_data = (gfloat **) g_malloc (xg.nrows * sizeof (gfloat *));
  for (i=0; i<xg.nrows; i++)
    xg.raw_data[i] = (gfloat *) g_malloc (xg.ncols * sizeof (gfloat));

  if (xg.file_read_type == read_block) {
    fseek (fp,
      (glong) (sizeof (gulong) * 2 +
              onesize * (xg.file_start_row * xg.ncols)),
      SEEK_SET);
  }

  if (xg.file_read_type == read_all) {

    for (i=0; i<xg.nrows; i++) {
      for (j=0; j<xg.ncols; j++) {
        out = fread ((gchar *) &xg.raw_data[i][j], onesize, 1, fp);
        if (out != 1) {
          g_printerr ("problem in reading the binary data file\n");
          fclose (fp);
          exit (0);

        } else if (xg.raw_data[i][j] == FLT_MAX) {
          xg.raw_data[i][j] = 0.0;
          /* Allocate the missing values array */
          if (!xg.missing_values_p) {
            init_missing_array (xg.nrows, xg.ncols);
            xg.missing_values_p = true;
          }

          xg.missing[i][j] = 1;
          xg.nmissing++;
        }
      }
    }

  } else if (xg.file_read_type == read_block) {
    gint q;

    for (i=0, q=xg.file_start_row; i<xg.nrows; i++, q++) {
      xg.file_rows_sampled[i] = q;
      for (j=0; j<xg.ncols; j++) {
        out = fread ((gchar *) &xg.raw_data[i][j], onesize, 1, fp);
        if (out != 1) {
          g_printerr ("problem in reading the binary data file\n");
          fclose (fp);
          exit (0);

        } else if (xg.raw_data[i][j] == FLT_MAX) {
          xg.raw_data[i][j] = 0.0;
          /* Allocate the missing values array */
          if (!xg.missing_values_p) {
            init_missing_array (xg.nrows, xg.ncols);
            xg.missing_values_p = true;
          }

          xg.missing[i][j] = 1;
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
              out = fread ((gchar *) &xg.raw_data[i][j], onesize, 1, fp);
              if (out != 1) {
                g_printerr ("problem in reading the binary data file\n");
                fclose (fp);
                exit (0);
              } else if (xg.raw_data[i][j] == FLT_MAX) {
                xg.raw_data[i][j] = 0.0;

                /* Allocate the missing values array */
                if (!xg.missing_values_p) {
                  init_missing_array (xg.nrows, xg.ncols);
                  xg.missing_values_p = true;
                }
                xg.missing[i][j] = 1;
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

/********************************************************************
*          Reading ascii files                                      *
*********************************************************************/

void
alloc_rawdata_block (gint nblocks) {
/*
 * Allocate space for nblocks*BLOCKSIZE rows
*/
  gint i;
  gulong nr = nblocks * BLOCKSIZE;

  if (nblocks == 1)
    xg.raw_data = (gfloat **) g_malloc (nr * sizeof (gfloat *));
  else
    xg.raw_data = (gfloat **) g_realloc ((gpointer) xg.raw_data,
      nr * sizeof (gfloat *));

  for (i=BLOCKSIZE*(nblocks-1); i<BLOCKSIZE*nblocks; i++)
    xg.raw_data[i] = (gfloat *)
      g_malloc (xg.ncols * sizeof (gfloat));

}     

static void
alloc_missing_block (gint nblocks) {
/*
 * Allocate missing for nblocks*BLOCKSIZE rows
*/
  gint i;
  gulong nr = nblocks * BLOCKSIZE;

  if (nblocks == 1)
    xg.missing = (gshort **) g_malloc (nr * sizeof (gshort *));
  else {
g_printerr ("reallocating missing %d\n", xg.nmissing);
    xg.missing = (gshort **) g_realloc ((gpointer) xg.missing,
      nr * sizeof (gshort *));
  }

  for (i=(nblocks-1)*BLOCKSIZE; i<nblocks*BLOCKSIZE; i++)
    xg.missing[i] = (gshort *) g_malloc0(xg.ncols * sizeof (gshort));
}

gboolean
find_data_start (FILE *fp)
{
  gint ch;
  gboolean morelines = true;
  gboolean comment_line = true;

  while (comment_line)
  {
    /* skip white space */
    while (1)
    {
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
      xg.file_rows_sampled = (gulong *) g_malloc (xg.nrows * sizeof (gulong));
      for (i=0, k=xg.file_start_row; i<xg.nrows; i++, k++)
        xg.file_rows_sampled[i] = k;
      break;

    case draw_sample:

      xg.nrows = xg.file_sample_size;
      xg.file_rows_sampled = (glong *)
        g_malloc ((gulong) xg.nrows * sizeof (glong));

      n = xg.nrows;
      if (n > 0 && n < xg.file_length) { 
        for (t=0, m=0; t<xg.file_length && m<n; t++) {

          rrand = (gfloat) randvalue ();
          if ( ((float)(xg.file_length - t) * rrand) < (float)(n - m) )
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

void
read_ascii (FILE *fp)
{
  register gint ch;
  gint i, j, k, jrows, nrows, jcols, fs;
  gint nitems;
  gfloat row1[MAXNCOLS];
  gshort row1_missing[MAXNCOLS];
  gint nblocks;
  gchar word[64];

  /* Initialize these before starting */
  for (k=0; k<MAXNCOLS; k++) {
    row1_missing[k] = 0;
    row1[k] = 0.0;
  }
  xg.ncols = 0;

  init_file_rows_sampled ();

/*
 * Find the index of the first row of data that we're interested in.
*/

  nrows = 0;
  if (xg.file_read_type == read_all) {
    if (find_data_start (fp) == false)
      return;

  } else {  /* if -only was used on the command line */
    if (!seek_to_file_row (nrows, fp))
      return;
  }

/*
 * Read in the first row of the data file and calculate ncols.
*/

  gotone = true;

/*
 * I've left behind some checking that's done in bak/read_array.c --
 * test xgobi on a text file and see what happens.
*/

  while ( (ch = getc (fp)) != '\n') {

    if (ch == '\t' || ch == ' ')
      ;

    else if (ungetc (ch, fp) == EOF || fscanf (fp, "%s", word) < 0 ) {
      g_printerr ("read_array: error in reading first row of data\n");
      fclose (fp);
      exit (0);

    } else {

      if (g_strcasecmp (word, "na") == 0 || strcmp (word, ".") == 0) {
        xg.missing_values_p = true;
        xg.nmissing++;
        row1_missing[xg.ncols] = 1;

      } else {
        row1[xg.ncols] = (gfloat) atof (word);
      }
      xg.ncols++ ;

      if (xg.ncols >= MAXNCOLS) {
        g_printerr (
          "This file has more than %d columns.  In order to read it in,\n",
          MAXNCOLS);
        g_printerr (
         "increase MAXNCOLS in defines.h and recompile.\n");
        exit (0);
      }
    }
  }

/*
 * If we're reading everything, allocate the first block.
 * If -only has been used, allocate the whole shebang.
*/
  if (xg.file_read_type == read_all) {

    xg.nrows = 0;
    alloc_rawdata_block (1);
    if (xg.missing_values_p)
      alloc_missing_block (1);

  } else {  /* -only has been used */

    xg.nrows = xg.file_sample_size;
    xg.raw_data = (gfloat **) g_malloc (xg.nrows * sizeof (gfloat *));
    for (i=0; i<xg.nrows; i++)
      xg.raw_data[i] = (gfloat *) g_malloc (xg.ncols * sizeof (gfloat));

    if (xg.missing_values_p)
      init_missing_array (xg.nrows, xg.ncols);
  }

/*
 * Fill in the first row
*/
  for (j=0; j<xg.ncols; j++)
    xg.raw_data[0][j] = row1[j];
  if (xg.missing_values_p) {
    for (j=0; j<xg.ncols; j++)
      xg.missing[0][j] = row1_missing[j];
  }
  nrows++;

/*
 * Read data, reallocating as needed.  Determine nrows for the read_all case.
*/
  nblocks = 1;
  nitems = xg.ncols;
  jrows = 1;
  jcols = 0;
  while (1)
  {

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

    if (fs == EOF)
      break;
    else if (fs < 0)
    {
      g_printerr ("Problem with input data\n");
      fclose (fp);
      exit (0);
    }
    else
    {
      nitems++;

      if ( g_strcasecmp (word, "na") == 0 || strcmp (word, ".") == 0 ) {

        if (!xg.missing_values_p) {
          xg.missing_values_p = true;
          /*
           * Only when the first "na" or "." has been encountered
           * is it necessary to allocate space to contain the
           * missing values matrix.  Initialize all previous values
           * to 0.
          */
          if (xg.file_read_type == read_all) {
            alloc_missing_block (nblocks);
            for (i=BLOCKSIZE*(nblocks-1); i<BLOCKSIZE*nblocks; i++) {
              for (k=0; k<xg.ncols; k++)
                xg.missing[i][k] = 0;
            }
          } else {
            ;
            /*init_missing_array (xg.nrows, xg.ncols);*/
          }
        }

        xg.nmissing++;
        xg.missing[nrows][jcols] = 1;
        xg.raw_data[nrows][jcols] = 0.0;
      }
      else
        xg.raw_data[nrows][jcols] = (gfloat) atof (word);

      jcols++;
      if (jcols == xg.ncols)
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

          alloc_rawdata_block (nblocks);

          if (xg.missing_values_p)
            alloc_missing_block (nblocks);
        }

      } else {  /* -only was used */
        if (nrows >= xg.nrows)
          break;
      }
    }
  }

/*
 * Close the data file
*/
  if (fclose (fp) == EOF)
    g_printerr ("read_array: error in fclose");

  if (xg.file_read_type == read_all)
    xg.nrows = nrows;

  g_print ("size of data: %d x %d\n", xg.nrows, xg.ncols);

  if ( nitems != xg.nrows * xg.ncols )
  {
    g_printerr ("read_array: nrows*ncols != nitems read\n");
    g_printerr ("(nrows %d, ncols %d, nitems read %d)\n",
      xg.nrows, xg.ncols, nitems);
    exit (0);
  }
  else if (nitems == 0)
  {
    g_printerr ("No data read\n");
    exit (0);
  }
  else
  {
    /*
     * If we haven't yet encountered a missing value, free up
     * the whole matrix.
    */
    if (!xg.missing_values_p)
      xg.missing = (gshort **) NULL;

    if (xg.file_read_type == read_all) {
      /*
       * One last free and realloc to make raw_data take up exactly
       * the amount of space it needs.
      */
      for (i=xg.nrows; i<BLOCKSIZE*nblocks; i++)
        g_free ((gpointer) xg.raw_data[i]);

      xg.raw_data = (gfloat **) g_realloc ((gpointer) xg.raw_data,
        xg.nrows * sizeof (gfloat *));

      if (xg.missing_values_p) {
        for (i=xg.nrows; i<BLOCKSIZE*nblocks; i++)
          g_free ((gpointer) xg.missing[i]);
g_printerr ("(null?) %d\n", xg.nmissing);
        xg.missing = (gshort **) g_realloc ((gpointer) xg.missing,
          xg.nrows * sizeof (gshort *));
      }
    }

    /*
     * If the data contains only one column, add a second,
     * the numbers 1:nrows -- and let the added column be
     * the first column?
    */
    xg.single_column = false;
    if (xg.ncols == 1)
    {
      xg.single_column = true;
      xg.ncols = 2;
      xg.ncols = 3;
      for (i=0; i<xg.nrows; i++)
      {
        xg.raw_data[i] = (gfloat *) g_realloc (
          (gpointer) xg.raw_data[i],
          (guint) 3 * sizeof (gfloat));
        xg.raw_data[i][1] = xg.raw_data[i][0] ;
        xg.raw_data[i][0] = (gfloat) (i+1) ;

        /* And populate a column of missing values with 0s, if needed */
        if (xg.missing_values_p)
        {
          xg.missing[i] = (gshort *) g_realloc (
            (gpointer) xg.missing[i],
            (guint) 3 * sizeof (gshort));
          xg.missing[i][1] = 0 ;
        }
      }
    }
  }
}

/********************************************************************
*          End of section on reading ascii files                    *
*********************************************************************/

void
read_array ()
{
  gchar fname[128];
  FILE *fp;

/*
 * Check file exists and open it - for stdin no open needs to be done
 * only assigning fp to be stdin.
*/
  if (strcmp ((gchar *) xg.fname, "stdin") == 0) {
    fp = stdin;

    /*
     * If reading from stdin, set an alarm.  If after 5 seconds,
     * no data has been read, print an error message and exit.
    */
    alarm ((guint) 5);
    signal (SIGALRM, stdin_empty);

    read_ascii (fp);
  }
  else
  {
    /* 
     * Are we reading the missing data into xg.raw_data ?
    */
    if (strcmp (
         ".missing",
         &xg.filename[strlen (xg.filename) - strlen (".missing")]
       ) == 0)
    {
      if ((fp = fopen (xg.filename, "r")) != NULL)
      {
        xg.missing_values_xgobi_p = true;
        xg.missing_values_p = true;
        read_ascii (fp);

        /*
         * extend the title
        title = (gchar *) g_malloc (256 * sizeof (gchar));
        */
      }
      else
      {
        g_printerr ("The file %s can't be opened for reading.\n",
          xg.filename);
        exit (0);
      }
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
      else
      {
        static gchar *suffixes[] = {"", ".dat"};
        if ( (fp=open_xgobi_file_r (xg.fname, 2, suffixes, false)) != NULL)
          read_ascii (fp);
      }
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
g_printerr ("find_root_name_of_data\n");

  pf = fname;
  while (fname[j] != '\0') {
    if (fname[j] == G_DIR_SEPARATOR)
      pf = &fname[j+1];
    j++;
  }

  title = g_strdup_printf (pf);
}

