/* read_data.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define INITSTRSIZE 512


/* Make certain this matches GlyphType. */
const gchar * const GlyphNames[] = {
/*        ".", "+", "x", "or", "fr", "oc", "fc", "" */
          ".", "plus", "x", "oc", "or", "fc", "fr", ""
        };


gchar *
findAssociatedFile(InputDescription *desc, const gchar * const *extensions,
  gint numExtensions, gint *which, gboolean isError)
{
  gint i;
  gchar buf[100];

  if (desc->fileName == NULL ||
      desc->fileName[0] == '\0' ||
      strcmp (desc->fileName, "stdin") == 0)
  {
    return(NULL);
  }  

  for(i = 0; i < numExtensions; i++) {
    if(extensions[i] && extensions[i][0])
      sprintf(buf, "%s.%s", desc->baseName, extensions[i]);
    else
      sprintf(buf, "%s", desc->baseName);

    if(check_file_exists(buf)) {
      if(which)
        *which = i;
      return(g_strdup(buf));
    }
  }

  return(NULL);
}


/*------------------------------------------------------------------------*/
/*                          row labels                                    */
/*------------------------------------------------------------------------*/

void rowlabels_free (datad *d)
{
  g_array_free (d->rowlab, true);
}


void
rowlabels_alloc (datad *d) 
{
  if (d->rowlab != NULL) rowlabels_free (d);
  d->rowlab = g_array_new (false, false, sizeof (gchar *));
}

void
rowlabels_add (gchar **labels, gint nnewlabels, datad *d) 
{
  d->rowlab = g_array_append_vals (d->rowlab, (gconstpointer) labels,
    nnewlabels);

  g_assert (d->rowlab->len == d->nrows);
}

gboolean
rowlabels_read (InputDescription *desc, gboolean init, datad *d, ggobid *gg)
{
  gint i;
  static const gchar *const suffixes[] = {
    "row", "rowlab", "case"
  };
  gchar initstr[INITSTRSIZE];
  gchar *lbl;
  gint ncase;
  gboolean found = true;
  FILE *fp;

  gint whichSuffix;
  gchar *fileName;

  if (init)
    rowlabels_alloc (d);

  fileName = findAssociatedFile (desc, suffixes,
    sizeof(suffixes)/sizeof(suffixes[0]), &whichSuffix, false);
  if (fileName == NULL)
    found = false;

  if( ( fp = fopen(fileName, "r") ) == NULL ) {
    g_free(fileName);
    found = false;
  }

  /*
   * Read in case labels or initiate them to generic if no label
   * file exists
  */
  if (found) {
    gint k, len;
    ncase = 0;

    k = 0;  /* k is the file row */
    while (fgets (initstr, INITSTRSIZE-1, fp) != NULL) {
      len = MIN ((int) strlen (initstr), ROWLABLEN-1) ;

      /* trim trailing blanks, and eliminate the carriage return */
      while (initstr[len-1] == ' ' || initstr[len-1] == '\n')
        len-- ;
      initstr[len] = '\0';
      lbl = g_strdup (initstr);
      g_array_append_val (d->rowlab, lbl);
  
      if (ncase++ >= d->nrows)
        break;
      k++;  /* read the next row ... */
    }
  
    /*
     * If there aren't enough labels, use blank labels for
     * the remainder.
    */
    if (init && ncase != d->nrows) {
      g_printerr ("number of labels = %d, number of rows = %d\n",
        ncase, d->nrows);
      for (i=ncase; i<d->nrows; i++) {
        lbl = g_strdup (" ");
        g_array_append_val (d->rowlab, lbl);
      }
    }
  }
  else
  {
    if (init) {  /* apply defaults if initializing; else, do nothing */

      for (i=0; i<d->nrows; i++) {
        lbl = g_strdup_printf ("%d", i+1);
        g_array_append_val (d->rowlab, lbl);
      }
    }
  }


  if(found) {
    addInputSuffix(desc, suffixes[whichSuffix]);
  }
  g_free(fileName);

 return (found);
}

/*------------------------------------------------------------------------*/
/*                       column labels                                    */
/*------------------------------------------------------------------------*/

static void
collabels_process_word (gchar *word, gint field, gint j, datad *d) 
{
  gfloat var;
  vartabled *vt = vartable_element_get (j, d);

  /*-- remove leading and trailing whitespace --*/
  g_strstrip (word);

  switch (field) {
    case 0:
      vt->lim_specified_p = false;
      vt->collab = g_strdup (word) ;
      /*-- if word is shorter than 2 characters, g_strndup pads with nulls --*/
      vt->nickname = g_strndup (word, 2) ;
    break;
    case 1:
      var = atof (word);
      /*-- don't set lim_specified_p to true unless both are present --*/
      vt->lim_specified.min =
        vt->lim_specified_tform.min = var;
    break;
    case 2:
      var = atof (word);
      vt->lim_specified_p = true;
      vt->lim_specified.max =
        vt->lim_specified_tform.max = var;
    break;
    default:
      /*-- bail out: too many fields --*/
      g_printerr ("Too many fields in row %d of collab file\n", j+1);
      exit (1);
  }
}

/*
 * Change: we'll no longer support blanks in column names,
 * because we want the option of adding meaningful 2nd and 3rd
 * fields, which will contain min and max range values.
*/
gboolean
collabels_read (InputDescription *desc, gboolean init, datad *d, ggobid *gg)
{
  static const gchar * const suffixes[] = {
    "col", "column", "collab", "var"
  };
  gint j, nvar = 0;
  gboolean found = true;
  FILE *fp;
  vartabled *vt;

  gchar *fileName;
  gint whichSuffix;
  gchar str[INITSTRSIZE];

  fileName = findAssociatedFile(desc, suffixes,
    sizeof(suffixes)/sizeof(suffixes[0]), &whichSuffix, false);
  if(fileName == NULL)
    found = false;

  if( found && ( fp = fopen(fileName, "r") ) == NULL ) {
    g_free (fileName);
    found = false;
  }


  /*
   * Read in variable labels or initiate them to generic if no label
   * file exists
  */
  if (found) {
    gint ch, len = 0, field = 0;
    gboolean fieldsep = false;
    nvar = 0;

    while ((ch = fgetc (fp)) != EOF) {

      /*-- blank or tab --*/
      /*if (ch == ' ' || ch == '	') */
      if (ch == '|') {
        fieldsep = true;

      } else if (ch == '\n') {
        /*-- process preceding string --*/
        str[len] = '\0';
        collabels_process_word (str, field, nvar, d);
        field = len = 0;
        nvar++;
        if (nvar >= d->ncols)
          break;
        fieldsep = false;
      } else {  /*-- process the next character --*/

        /*-- if following a field separator, process string --*/
        if (fieldsep && len > 0) {
          /*-- process string --*/
          str[len] = '\0';
          collabels_process_word (str, field, nvar, d);

          field++;
          len = 0;
        }

        if (field == 0 && len == COLLABLEN-1) {
          ;  /*-- make sure the column label isn't too long */
        } else {
          /*-- append character to str --*/
          str[len] = ch;
          len++;
          if (len > INITSTRSIZE)
            break;
          fieldsep = false;
        }
      }
    }

    if (init && nvar != d->ncols) {
      g_printerr ("number of labels = %d, number of cols = %d\n",
        nvar, d->ncols);
      for (j=nvar; j<d->ncols; j++) {
        vt = vartable_element_get (j, d);
        vt->collab = g_strdup_printf ("Var %d", j+1);
      }
    }
  }
  else
  {
    if (init) {
      for (j=0; j<d->ncols; j++) {
        vt = vartable_element_get (j, d);
        vt->lim_specified_p = false;
        vt->collab = g_strdup_printf ("Var %d", j+1);
      }
    }
  }

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    vt->collab_tform = g_strdup (vt->collab);
  }

  if(found) {
    addInputSuffix(desc, suffixes[whichSuffix]);
  }
  g_free(fileName);

  return (found);
}

/*------------------------------------------------------------------------*/
/*                 point glyphs and colors                                */
/*------------------------------------------------------------------------*/

static void
readGlyphErr (void) {
  g_printerr ("The .glyphs file must contain either one number per line,\n");
  g_printerr ("with the number between 0 and %d; using defaults,\n",
    NGLYPHS);
  g_printerr ("or a string and a number, with the string being one of\n");
  g_printerr ("plus, x, or, fr, oc, fc, .  and the number between 1 and %d.\n",
/*  g_printerr ("+, x, or, fr, oc, fc, .  and the number between 1 and %d.\n",*/
    NGLYPHSIZES-1);
}

gboolean
point_glyphs_read (InputDescription *desc, gboolean reinit,
  datad *d, ggobid *gg)
{
  gboolean ok = true;
  static const gchar * const suffixes[] = {"glyphs"};
  gint i, k;
  gboolean found = true;
  FILE *fp;
  gint gid;
  glyphd glyph;
  gboolean use_defaults = false;

  gchar *fileName;
  gint whichSuffix;


  if (reinit)
    br_glyph_ids_alloc (d);

  fileName = findAssociatedFile (desc, suffixes,
    sizeof(suffixes)/sizeof(suffixes[0]), &whichSuffix, false);
  if (fileName == NULL)
    found = false;

  if (found && ( fp = fopen(fileName, "r") ) == NULL ) {
    found = false;
  }

  if (!found && reinit)
    br_glyph_ids_init (d, gg);
  else
  {
    enum { typeAndSize, glyphNumber } glyph_format;
    gint c, retval, gsize;
    gchar *gtype = (gchar *) g_malloc (16 * sizeof (gchar));

    /*
     * For the first row, find out if we're going to be reading
     * %s %d (typeAndSize) or %d (glyphNumber)
    */
    c = getc (fp);
    glyph_format = isdigit (c) ? glyphNumber : typeAndSize;

    ungetc (c, fp);
    i = 0; k = 0;
    while (i < d->nrows) {  /* should there be a test on k as well? */

      if (glyph_format == glyphNumber) {
        retval = fscanf (fp, "%d", &gid);
      } else {
        fscanf (fp, "%s", gtype);
/*-- we're writing out size=1 for point glyphs, so this 'if' isn't right --*/
/*-- wrong --*/
/*
        gsize = 1;
        if (strcmp (gtype, ".") != 0)
*/
          retval = fscanf (fp, "%d", &gsize);
      }

      if (retval <= 0) {
        /* not using show_message () here; reading before ggobi startup */
        g_printerr ("!Error in reading glyphs file; using defaults.\n");
        use_defaults = true;
        break;
      }

      /*-- If the input is a single number on a line --*/
      /*-- gid is on 0:48; type from 0 to 6, size from 0 to 7 --*/
      if (glyph_format == glyphNumber) {
        if (gid < 0 || gid > NGLYPHS) {
          g_printerr ("illegal glyph number: %d; using defaults\n", gid);
          use_defaults = true;
          break;
        }

        find_glyph_type_and_size (gid, &glyph);

      /*-- Else if the input is a string and a number --*/
      } else {
        glyph.type = mapGlyphName (gtype);

        if (glyph.type == UNKNOWN_GLYPH) {
          readGlyphErr ();
          use_defaults = true;
          break;
        }

        glyph.size = gsize;
        if (gsize < 0 || gsize >= NGLYPHSIZES) {
          use_defaults = true;
          readGlyphErr ();
        }
      }

      if (use_defaults) {
        break;
      }

      d->glyph.els[i].type = d->glyph_now.els[i].type =
        d->glyph_prev.els[i].type = glyph.type;
      d->glyph.els[i].size = d->glyph_now.els[i].size =
        d->glyph_prev.els[i].size = glyph.size;

      i++;  /* increment the array index */
      k++;  /* increment the file's row counter */
    }
    g_free (gtype);
    fclose (fp);
  }

  if (!found || use_defaults)
    br_glyph_ids_init (d, gg);


  if(found) {
    addInputSuffix(desc, suffixes[whichSuffix]);
  }
  g_free(fileName);

  return (ok);
}

GlyphType 
mapGlyphName (const gchar *gtype)
{
  GlyphType type;
  gint i;

  type = UNKNOWN_GLYPH;
  for (i = 0; i < sizeof (GlyphNames)/sizeof (GlyphNames[0]) - 1; i++) {
    if (strcmp(gtype, GlyphNames[i]) == 0) {
      type = (GlyphType) (i);
      break;
    }
  }

  return(type);
}

gboolean
point_colors_read (InputDescription *desc, gboolean reinit,
  datad *d, ggobid *gg)
{
  gboolean ok = false;
  gboolean found = true;
  const gchar * const suffixes[] = {"colors"};
  gint i, k, retval;
  FILE *fp;
  gint id;
 
  gchar *fileName;
  gint whichSuffix;


  if (reinit)
    br_color_ids_alloc (d, gg);


  fileName = findAssociatedFile(desc, suffixes,
    sizeof(suffixes)/sizeof(suffixes[0]), &whichSuffix, false);

  if(fileName) {
    found = true;
  } else
    found = false;

  if(found && ( fp = fopen(fileName, "r") ) == NULL ) {
    g_free(fileName);
    return(false);
  }


    if (!found && reinit == true)
      ;  /* no need to init the ids */
    else {
      ok = true;

      i = 0; k = 0;
      while (i < d->nrows) {  /* should there be a test on k as well? */

        retval = fscanf (fp, "%d", &id);
        if (retval <= 0 || id < 0 || id >= MAXNCOLORS) {
          ok = false;
          g_printerr ("!!Error in reading colors file; using defaults.\n");
          break;
        }

        d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i] = id;

        i++;  /* increment the array index */
        k++;  /* increment the file's row counter */
      }
      fclose (fp);
    }


  if (!ok)
    br_color_ids_init (d, gg);

  if(found) {
    addInputSuffix(desc, suffixes[whichSuffix]);
  }
  g_free(fileName);

  return (ok);
}

/*------------------------------------------------------------------------*/
/*                          erasing                                       */
/*------------------------------------------------------------------------*/

gboolean
hidden_read (InputDescription *desc, gboolean reinit, datad *d, ggobid *gg)
/*
 * Read in the hidden vector
*/
{
  static const gchar *const suffixes[] = {"hide"};
  gint itmp, i;
  gboolean found = true;
  FILE *fp;
  gchar *fileName;
  gint whichSuffix;

  if (reinit)
    br_hidden_alloc (d);

  fileName = findAssociatedFile(desc, suffixes,
    sizeof(suffixes)/sizeof(suffixes[0]), &whichSuffix, false);
  if(fileName == NULL)
    found = false;

  if (found) {
    if ((fp = fopen(fileName, "r")) == NULL ) {
      found = false;
    }
  }

  if (found) {
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < d->nrows)) {
      d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i] =
        (gboolean) itmp;
      i++;
    }
  
    if (i < d->nrows) {
      g_printerr ("Problem in reading hide file; not enough rows\n");
    } else
       addInputSuffix(desc, suffixes[whichSuffix]);
  } else {
    if (reinit)
      br_hidden_init (d);
  }

  if(fileName) 
    g_free(fileName);

  return (found);
}

/*------------------------------------------------------------------------*/
/*                         missing values                                 */
/*------------------------------------------------------------------------*/


gboolean
missing_values_read (InputDescription *desc, gboolean init, datad *d,
  ggobid *gg)
{
  static const gchar *const suffixes[] = {"missing"};
  gint i, j, ok, itmp, row, col;
  gint nmissing = 0;
  FILE *fp;
  vartabled *vt;
 
  gint whichSuffix;
  gchar *fileName;

  fileName = findAssociatedFile(desc, suffixes,
    sizeof(suffixes)/sizeof(suffixes[0]), &whichSuffix, false);
  if(fileName == NULL)
    return(false);

  if( ( fp = fopen(fileName, "r") ) == NULL ) {
    g_free(fileName);
    return(false);
  }

  if (init || d->nmissing == 0)
    arrays_alloc (&d->missing, d->nrows, d->ncols);

  for (j=0; j<d->ncols; j++) {
    vt = vartable_element_get (j, d);
    vt->nmissing = 0;
  }

  j = 0;
  i = 0;
  while ((ok = fscanf (fp, "%d", &itmp)) != EOF) {
    row = i;
    col = j;
    j++;
    if (j==d->ncols) { j=0; i++; }
    if (i==d->nrows && j>0) ok = false;

    if (!ok) {
      g_print ("Problem reading %s", fileName);
      g_print (" at row %d, column %d.\n", i, j);
      g_print ("Make sure dimensions of %s and %s match\n",
                desc->fileName, fileName);
      fclose (fp);

      g_free(fileName);
      return(false);
    }

    d->missing.vals[row][col] = itmp;
    if (itmp != 0) {
      nmissing++;
      vt = vartable_element_get (col, d);
      vt->nmissing++;
    }
  }

  if (d->nmissing != 0 && d->nmissing != nmissing) {
    g_print ("I found %d missing values in your data file\n", d->nmissing);
    g_print (" but %d missing values in your .missing file.", nmissing);
    g_print ("I'll use the .missing results.\n");
  }
  d->nmissing = nmissing;

  fclose (fp);
  addInputSuffix(desc, suffixes[whichSuffix]);

  g_free(fileName);

 return(true);
}

