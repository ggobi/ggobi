/* read_array.c */
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

#ifndef WIN32
static void
stdin_empty (gint arg)
{
  if (!gotone) {
    g_printerr ("ggobi requires a filename or some data from stdin\n");
    exit (0);
  }
}
#endif


static void
read_binary (FILE *fp, datad *d, ggobid *gg)
{
  gint i, j, nr, nc;
  gint onesize = sizeof (gfloat);
  gint out;
  vartabled *vt;

  fread ((gchar *) &nr, sizeof (gint), 1, fp);
  fread ((gchar *) &nc, sizeof (gint), 1, fp);
  d->ncols = nc;

  /*
   * As soon as the number of columns is known, allocate vartable.
  */
  vartable_alloc (d);
  vartable_init (d);

  d->nrows = nr;

  arrayf_alloc (&d->raw, d->nrows, d->ncols);

  for (i=0; i<d->nrows; i++) {
    for (j=0; j<d->ncols; j++) {
      out = fread ((gchar *) &d->raw.vals[i][j], onesize, 1, fp);
      if (out != 1) {
        fclose (fp);
        g_error("problem in reading the binary data file\n");

      } else if (d->raw.vals[i][j] == FLT_MAX) {
        d->raw.vals[i][j] = 0.0;

        /* Allocate the missing values array */
        if (d->nmissing == 0)
          arrays_alloc_zero (&d->missing, d->nrows, d->ncols);
        d->missing.vals[i][j] = 1;
        vt = vartable_element_get (j, d);
        vt->nmissing++;
        d->nmissing++;
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
	  g_error("Comment lines must begin with # or %%; I found a line beginning with '%c'", ch);
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
row1_read (FILE *fp, gfloat *row1, gshort *row1_missing, datad *d, ggobid *gg) {

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
		fclose (fp);
        g_error("error in reading first row of data");
      } else {

        if (g_strcasecmp (word, "na") == 0 || strcmp (word, ".") == 0) {
          d->nmissing++;
          row1_missing[ncols] = 1;

        } else {
          row1[ncols] = (gfloat) atof (word);
        }

        ncols++;
        gotone = true;  /*-- suppress the alarm -- the file pointer is ok --*/

        if (d->ncols >= MAXNCOLS) {
          g_error("This file has more than %d columns.  In order to read it in, "
		  	"increase MAXNCOLS in defines.h and recompile", MAXNCOLS);
        }
      }
    }
  }

  return ncols;
}

static gboolean
ReadAscii (FILE *fp, datad *d, ggobid *gg)
{
  gint j, jrows, nrows, jcols, fs;
  gint nitems;
  gint nblocks;
  gchar word[64];
  gfloat row1[MAXNCOLS];
  gshort row1_missing[MAXNCOLS];
  vartabled *vt;

  /*-- Read in the first row of the data and calculate ncols. --*/
  d->ncols = row1_read (fp, row1, row1_missing, d, gg);

  /*-- Once the number of columns is known, allocate vartable. --*/
  vartable_alloc (d);
  vartable_init (d);

/*
 * allocate the first block.
*/
  d->nrows = 0;
  arrayf_alloc (&d->raw, BLOCKSIZE, d->ncols);
  if (d->nmissing > 0)
    arrays_alloc_zero (&d->missing, BLOCKSIZE, d->ncols);

  /*-- copy the values in row1 to the main array --*/
  for (j=0; j<d->ncols; j++)
    d->raw.vals[0][j] = row1[j];
  if (d->nmissing > 0) {
    for (j=0; j<d->ncols; j++)
      d->missing.vals[0][j] = row1_missing[j];
  }


/*-- Read, reallocating as needed.  Determine nrows for the read_all case. --*/
  nblocks = 1;
  nitems = d->ncols;
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
      return(false);
    }
    else {
      nitems++;

      if (g_strcasecmp (word, "na") == 0 || strcmp (word, ".") == 0) {

        if (d->nmissing == 0) {
          /*
           * When the first "na" or "." has been encountered,
           * allocate space to contain the missing values matrix.
           * Initialize all previous values to 0.
          */
          arrays_alloc_zero (&d->missing, nblocks*BLOCKSIZE, d->ncols);
        }

        d->nmissing++;
        vt = vartable_element_get (jcols, d);
        vt->nmissing++;
        d->missing.vals[nrows][jcols] = 1;
        d->raw.vals[nrows][jcols] = 0.0;
      }
      else {  /*-- not missing --*/
        d->raw.vals[nrows][jcols] = (gfloat) atof (word);
      }

      jcols++;
      if (jcols == d->ncols)  /*-- we just completed a row --*/
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

        arrayf_add_rows (&d->raw, nblocks*BLOCKSIZE);
        if (d->nmissing > 0)
          arrays_add_rows (&d->missing, nblocks*BLOCKSIZE);
      }
    }
  }

  /*-- Close the data file --*/
  if (fclose (fp) == EOF)
    g_printerr ("ReadAscii: error in fclose");

  d->nrows = nrows;

  if(sessionOptions->verbose == GGOBI_VERBOSE)
    g_printerr ("size of data: %d x %d\n", d->nrows, d->ncols);

  if (nitems != d->nrows * d->ncols) {
    g_error("ReadAscii: nrows*ncols != nitems read (nrows= %d, ncols= %d, nitems read= %d)",
		d->nrows, d->ncols, nitems);
  } else if (nitems == 0) {
    g_error("No data was read");
  }
  else {  /*-- nitems ok --*/
    /*
     * One last free and realloc to make these arrays take up exactly
     * the amount of space they need.
    */
    arrayf_free (&d->raw, d->nrows, d->ncols);
    if (d->nmissing)
      arrays_free (&d->missing, d->nrows, d->ncols);
  }

  return(true);
}


gboolean
read_ascii(InputDescription *desc, ggobid *gg, GGobiPluginInfo *plugin)
{
   return(read_ascii_data(desc, gg));
}


InputDescription *
read_ascii_input_description(const char * const fileName, const char * const modeName, 
			   ggobid *gg, GGobiPluginInfo *info)
{
  InputDescription *desc;
  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  if(!canRead(fileName)) {
     desc->fileName = g_malloc((strlen(fileName) + 5) * sizeof(gchar));
     sprintf(desc->fileName, "%s.dat", fileName);
  } else
     desc->fileName = g_strdup(fileName);
  desc->mode = ascii_data;
  desc->desc_read_input = &read_ascii;	
  completeFileDesc(fileName, desc);

  return(desc);
}


gboolean
read_ascii_data(InputDescription *desc, ggobid *gg)
{
  datad *d;

  d = datad_new (NULL, gg);

  if (array_read (d, desc, gg) == false) {
    /* Somewhere, we have to arrange to throw away the datad
     * and get it out of the list.
    */
    return(false);
  }

  d->nrows_in_plot = d->nrows;    /*-- for now --*/

  missing_values_read (desc, true, d, gg);
      
  collabels_read (desc, true, d, gg);
  rowlabels_read (desc, true, d, gg);
      
  point_glyphs_read (desc, true, d, gg);
  point_colors_read (desc, true, d, gg);
  hidden_read (desc, true, d, gg);
    
  return (true);
}

/*----------------------------------------------------------------------*/
/*              End of section on reading ascii files                   */
/*----------------------------------------------------------------------*/

gboolean
array_read (datad *d, InputDescription *desc, ggobid *gg)
{
  gchar fname[128];
  FILE *fp;

/*
 * Check file exists and open it - for stdin no open needs to be done
 * only assigning fp to be stdin.
*/
  if (strcmp ((gchar *) desc->fileName, "stdin") == 0) {
    fp = stdin;

#ifndef WIN32
    /*
     * If reading from stdin, set an alarm.  If no data has 
     * been read after 3 seconds, print an error message and exit.
    */
    alarm ((unsigned) 3);
    signal (SIGALRM, stdin_empty);
#endif

    return(ReadAscii (fp, d, gg));
  }
  else
  {
    /*
     * Try fname.bin before fname, to see whether there is a binary
     * data file available.  If there is, call read_binary ().
     * ... except that there's no support for reading binary data now.
    */
    if(desc->mode == binary_data) {
      if ((fp = fopen (desc->fileName, "rb")) != NULL) {
        read_binary (fp, d, gg);
        d->name = g_strdup(fname);
        d->nickname = g_strndup (d->name, 5);
      } else
        return(false);
    }  else {
      if ( (fp = fopen(desc->fileName, "r")) != NULL) {
        gchar *sep = g_strdup_printf ("%c", G_DIR_SEPARATOR);
        gchar *name = NULL;
        gchar **words = g_strsplit ((const gchar *) desc->baseName,
          (const gchar *) sep, 0);
        gchar **p;

        if (!words) {
          fclose(fp);
/**/      return (false);
	}

        for (p=words; *p; p++) {
          if (**p) {
            name = p[0];
          }
        }

        ReadAscii (fp, d, gg);
        /*-- set the name to the filename with all the directory
             information stripped out --*/
        d->name = (name != NULL && strlen(name)) > 0 ?
          g_strdup (name) : g_strdup(gg->input->baseName);
        d->nickname = g_strndup (d->name, 5);

        g_strfreev (words);
      }
      else {
        return(false);
      }
    }
  }

  return(true);
}


#ifdef UNUSED
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
#endif



#ifdef USE_DEPRECATED
void
strip_suffixes (ggobid *gg)
{
/*
 * Find the name of the data file excluding certain suffixes:
 * .bin, .dat
*/
  gint i, nchars;
  gboolean foundit = false;
  gint nsuffixes = 3;
  static gchar *suffix[] = {
    ".bin",
    ".xml",
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
#endif
