/* read_data.c */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#define INITSTRSIZE 512


/* Make certain this matches GlyphTypes. */
const gchar * const GlyphNames[] = {
/*          "+", "x", "or", "fr", "oc", "fc", ".", ""*/
          "plus", "x", "or", "fr", "oc", "fc", ".", ""
        };


/*------------------------------------------------------------------------*/
/*                          row labels                                    */
/*------------------------------------------------------------------------*/

void rowlabels_free (ggobid *gg) {
/*
  gint i;
  for (i=0; i<gg->nrows; i++)
    g_free (gg->rowlab[i]);
  g_free (gg->rowlab);
*/
  g_array_free (gg->rowlab, true);  /* unsure about the 2nd arg */
}


void
rowlabels_alloc (ggobid *gg) 
{
/* gint i; */
  if (gg->rowlab != NULL) rowlabels_free (gg);
/*
  gg->rowlab = (gchar **) g_malloc (gg->nrows * sizeof (gchar *));
  for (i=0; i<gg->nrows; i++)
    gg->rowlab[i] = (gchar *) g_malloc (ROWLABLEN * sizeof (gchar));
*/
  gg->rowlab = g_array_new (false, false, sizeof (gchar *));
}

gboolean
rowlabels_read (gchar *ldata_in, gboolean init, ggobid *gg)
{
  gint i;
  static gchar *suffixes[] = {
    ".row", ".rowlab", ".case"
  };
  gchar initstr[INITSTRSIZE];
  gchar *lbl;
  gint ncase;
  gboolean found = false;
  FILE *fp;

  if (init)
    rowlabels_alloc (gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in,"stdin") != 0)
    if ((fp = open_ggobi_file_r (ldata_in, 3, suffixes, true)) != NULL)
      found = true;

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
      g_array_append_val (gg->rowlab, lbl);
  
      if (ncase++ >= gg->nrows)
        break;
      k++;  /* read the next row ... */
    }
  
    /*
     * If there aren't enough labels, use blank labels for
     * the remainder.
    */
    if (init && ncase != gg->nrows) {
      g_printerr ("number of labels = %d, number of rows = %d\n",
        ncase, gg->nrows);
      for (i=ncase; i<gg->nrows; i++) {
        lbl = g_strdup (" ");
        g_array_append_val (gg->rowlab, lbl);
      }
    }
  }
  else
  {
    if (init) {  /* apply defaults if initializing; else, do nothing */

      for (i=0; i<gg->nrows; i++) {
        lbl = g_strdup_printf ("%d", i+1);
        g_array_append_val (gg->rowlab, lbl);
      }
    }
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       column labels                                    */
/*------------------------------------------------------------------------*/

gboolean
collabels_read (gchar *ldata_in, gboolean init, ggobid *gg)
{
  static gchar *suffixes[] = {
    ".col", ".column", ".collab", ".var"
  };
  gint j, nvar = 0;
  gboolean found = false;
  FILE *fp;
  gchar initstr[INITSTRSIZE];

  /*
   * Check if variable label file exists, and open if so.
  */
  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in,"stdin") != 0)
    if ((fp=open_ggobi_file_r (ldata_in, 4, suffixes, true)) != NULL)
      found = true;

  /*
   * Read in variable labels or initiate them to generic if no label
   * file exists
  */
  if (found) {
    gint len;
    nvar = 0;

    while (fgets (initstr, INITSTRSIZE-1, fp) != NULL) {

      len = MIN ((gint) strlen (initstr), COLLABLEN-1) ;
     
      /* trim trailing blanks, and eliminate the carriage return */
      while (initstr[len-1] == '\n' || initstr[len-1] == ' ')
        len-- ;
      initstr[len] = '\0';
      gg->vardata[nvar].collab = g_strdup (initstr) ;

      if (nvar++ >= gg->ncols)
        break;
    }

    if (init && nvar != gg->ncols) {
      g_printerr ("number of labels = %d, number of cols = %d\n",
        nvar, gg->ncols);

      if (gg->single_column) {
        g_free (gg->vardata[1].collab);
        gg->vardata[1].collab = g_strdup_printf ("%s", gg->vardata[0].collab);
        g_free (gg->vardata[0].collab);
        gg->vardata[0].collab = g_strdup ("Index");

      } else {
        for (j=nvar; j<gg->ncols; j++) {
          gg->vardata[j].collab = g_strdup_printf ("Var %d", j+1);
        }
      }
    }
  }
  else
  {
    if (init) {
      for (j=0; j<gg->ncols; j++) {
        gg->vardata[j].collab = g_strdup_printf ("Var %d", j+1);
      }
    }
  }


  for (j=0; j<gg->ncols; j++) {
    gg->vardata[j].collab_tform = g_strdup (gg->vardata[j].collab);
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       variable groups                                  */
/*------------------------------------------------------------------------*/

gboolean
vgroups_read (gchar *ldata_in, gboolean init, ggobid *gg)
/*
 * Read in the grouping numbers for joint scaling of variables
*/
{
  gchar *suffixes[] = {".vgroups"};
  gint itmp, i, j;
  gboolean found = false;
  FILE *fp;

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0)
    if ((fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < gg->ncols))
      gg->vardata[i++].groupid_ori = itmp;

    if (init && i < gg->ncols) {
      g_printerr (
        "Number of variables and number of group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (j=i; j<gg->ncols; j++)
        gg->vardata[j].groupid_ori = j;
    }

    vgroups_sort (gg);

  } else {

    if (init)
      for (j=0; j<gg->ncols; j++)
        gg->vardata[j].groupid_ori = j;
  }

  for (j=0; j<gg->ncols; j++)
    gg->vardata[j].groupid = gg->vardata[j].groupid_ori;

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       row groups                                       */
/*------------------------------------------------------------------------*/

void
rgroups_free (ggobid *gg) {
  gint i, j;

  for (i=0; i<gg->nrgroups; i++)
    for (j=0; j<gg->rgroups[i].nels; j++)
      g_free ((gpointer) gg->rgroups[i].els);

  g_free ((gpointer) gg->rgroups);
  g_free ((gpointer) gg->rgroup_ids);
}

gboolean
rgroups_read (gchar *ldata_in, gboolean init, ggobid *gg)
/*
 * Read in the grouping numbers for joint scaling of variables
*/
{
  gchar *suffixes[] = {".rgroups"};
  gint itmp, i, k;
  gboolean found = false;
  gboolean found_rg;
  FILE *fp;
  gint *nels;
  gint nr;

  if (gg->nrgroups > 0) rgroups_free (gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0)
    if ((fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;
  
  if (!found) {
    gg->nrgroups = 0;
  } else {
  
    /*
     * If this isn't the first time we've read files, then
     * see if the rgroups structures should be freed.
    */
    if (!init)
      if (gg->nrgroups > 0)
        rgroups_free (gg);
  
    /* rgroup_ids starts by containing the values in the file */
    gg->rgroup_ids = (gint *) g_malloc (gg->nrows * sizeof (gint));
    nels = (gint *) g_malloc (gg->nrows * sizeof (gint));
     
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < gg->nrows))
      gg->rgroup_ids[i++] = itmp;
  
    /* check the number of group ids read -- should be nrows */
    if (init && i < gg->nrows)
    {
      g_printerr (
        "Number of rows and number of row group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (k=i; k<gg->nrows; k++)
        gg->rgroup_ids[k] = k;
    }
  
    /*
     * Initialize the global variables: nrows row groups,
     * nrows/10 elements in each group
    */

    gg->rgroups = (rgroupd *) g_malloc (gg->nrows * sizeof (rgroupd));
    for (i=0; i<gg->nrows; i++) {
      nels[i] = gg->nrows/10;
      gg->rgroups[i].els = (gint *) g_malloc (nels[i] * sizeof (gint));
      gg->rgroups[i].nels = 0;
      gg->rgroups[i].included = true;
    }
    gg->nrgroups = 0;
  
    /*
     * On this sweep, find out how many groups there are and how
     * many elements are in each group
    */
    nr = gg->nrows;

    for (i=0; i<nr; i++) {
      found_rg = false;
      for (k=0; k<gg->nrgroups; k++) {
  
        /* if we've found this id before ... */
        if (gg->rgroup_ids[i] == gg->rgroups[k].id) {
  
          /* Reallocate els[k] if necessary */
          if (gg->rgroups[k].nels == nels[k]) {
            nels[k] *= 2;
            gg->rgroups[k].els = (gint *)
              g_realloc ((gpointer) gg->rgroups[k].els,
                (nels[k] * sizeof (gint)));
          }
  
          /* Add the element, increment the element counter */
          gg->rgroups[k].els[ gg->rgroups[k].nels ] = i;
          gg->rgroups[k].nels++;
  
          /*
           * Now the value in rgroup_ids has to change so that
           * it can point to the correct member in the array of
           * rgroups structures
          */
          gg->rgroup_ids[i] = k;
  
          found_rg = true;
          break;
        }
      }
  
      /* If it's a new group id, add it */
      if (!found_rg) {
        gg->rgroups[gg->nrgroups].id = gg->rgroup_ids[i]; /* from file */
        gg->rgroups[gg->nrgroups].nels = 1;
        gg->rgroups[gg->nrgroups].els[0] = i;
        gg->rgroup_ids[i] = gg->nrgroups;  /* rgroup_ids reset to index */
        gg->nrgroups++;
      }
    }
    gg->nrgroups_in_plot = gg->nrgroups;
  
    /* Reallocate everything now that we know how many there are */
    gg->rgroups = (rgroupd *) g_realloc ((gpointer) gg->rgroups,
      (gulong) (gg->nrgroups * sizeof (rgroupd)));
  
    /* Now reallocate the arrays within each rgroups structure */
    for (k=0; k<gg->nrgroups; k++) {
      gg->rgroups[k].els = (gint *)
        g_realloc ((gpointer) gg->rgroups[k].els,
                    gg->rgroups[k].nels * sizeof (gint));
    }
  
    g_free ((gpointer) nels);
  }

  if (gg->nrgroups != 0)
    g_printerr ("gg.nrgroups=%d\n", gg->nrgroups);

  return (found);
}

void
readGlyphErr (void) {
  g_printerr ("The .glyphs file must contain either one number per line,\n");
  g_printerr ("with the number between 1 and %d; using defaults,\n",
    NGLYPHS);
  g_printerr ("or a string and a number, with the string being one of\n");
  g_printerr ("plus, x, or, fr, oc, fc, .  and the number between 1 and %d.\n",
/*  g_printerr ("+, x, or, fr, oc, fc, .  and the number between 1 and %d.\n",*/
    NGLYPHSIZES);
}

/*------------------------------------------------------------------------*/
/*                 point glyphs and colors                                */
/*------------------------------------------------------------------------*/

gboolean
point_glyphs_read (gchar *ldata_in, gboolean reinit, ggobid *gg)
{
  gboolean ok = true;
  gchar *suffixes[] = {".glyphs"};
  gint i, k;
  gboolean found = false;
  FILE *fp;
  gint gid;
  glyphv glyph;
  gboolean use_defaults = false;

  if (reinit)
    br_glyph_ids_alloc (gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0)
    if ((fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

  if (!found && reinit)
    br_glyph_ids_init (gg);

  else
  {
    enum { typeAndSize, glyphNumber } glyph_format;
    gint c, retval, gsize;
    gchar *gtype = g_malloc (16 * sizeof (gchar));

    /*
     * For the first row, find out if we're going to be reading
     * %s %d (typeAndSize) or %d (glyphNumber)
    */
    c = getc (fp);
    glyph_format = isdigit (c) ? glyphNumber : typeAndSize;

    ungetc (c, fp);
    i = 0; k = 0;
    while (i < gg->nrows) {  /* should there be a test on k as well? */

      if (glyph_format == glyphNumber) {
        retval = fscanf (fp, "%d", &gid);
      } else {
        fscanf (fp, "%s", gtype);
        gsize = 1;
        if (strcmp (gtype, ".") != 0)
          retval = fscanf (fp, "%d", &gsize);
      }

      if (retval <= 0) {
        /* not using show_message () here; reading before ggobi startup */
        g_printerr ("!Error in reading glyphs file; using defaults.\n");
        use_defaults = true;
        break;
      }

      /*
       * If the input is a single number on a line
      */
      if (glyph_format == glyphNumber) {

        if (gid < 1 || gid > NGLYPHS) {
          use_defaults = true;
          break;
        }

        find_glyph_type_and_size (gid, &glyph);

      /*
       * Else if the input is a string and a number
      */
      } else {
        glyph.type = mapGlyphName(gtype);

        if (glyph.type == UNKNOWN_GLYPH) {
          readGlyphErr ();
          use_defaults = true;
          break;
        }

        glyph.size = gsize;
        if (gsize < 1 || gsize > 5) {
          use_defaults = true;
          readGlyphErr ();
        }
      }

      if (use_defaults) {
        break;
      }

      gg->glyph_ids[i].type = gg->glyph_now[i].type =
        gg->glyph_prev[i].type = glyph.type;
      gg->glyph_ids[i].size = gg->glyph_now[i].size =
        gg->glyph_prev[i].size = glyph.size;

      i++;  /* increment the array index */
      k++;  /* increment the file's row counter */
    }
    g_free (gtype);
    fclose (fp);
  }

  if (!found || use_defaults)
    br_glyph_ids_init (gg);

  return (ok);
}

GlyphType 
mapGlyphName (const gchar *gtype)
{
  GlyphType type;
  int i;

  type = UNKNOWN_GLYPH;
  for (i = 0; i < sizeof (GlyphNames)/sizeof (GlyphNames[0]) - 1; i++) {
    if (strcmp(gtype, GlyphNames[i]) == 0) {
     type = i+1;
     break;
    }
  }

  return(type);
}

gboolean
point_colors_read (gchar *ldata_in, gboolean reinit, ggobid *gg)
{
  gboolean ok = false;
  gboolean found = false;
  gchar *suffixes[] = {".colors"};
  gint i, k, retval;
  FILE *fp;
  gint id;

  if (reinit)
    br_color_ids_alloc (gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0) {
    if ( (fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

    if (!found && reinit == true)
      ;  /* no need to init the ids */
    else {
      ok = true;

      i = 0; k = 0;
      while (i < gg->nrows) {  /* should there be a test on k as well? */

        retval = fscanf (fp, "%d", &id);
        if (retval <= 0 || id < 0 || id >= NCOLORS) {
          ok = false;
          g_printerr ("!!Error in reading colors file; using defaults.\n");
          break;
        }

        gg->color_ids[i] = gg->color_now[i] = gg->color_prev[i] = id;

        i++;  /* increment the array index */
        k++;  /* increment the file's row counter */
      }
      fclose (fp);
    }
  }

  if (!ok)
    br_color_ids_init (gg);

  return (ok);
}

/*------------------------------------------------------------------------*/
/*                     lines and line colors                              */
/*------------------------------------------------------------------------*/

gboolean
line_colors_read (gchar *ldata_in, gboolean reinit, ggobid *gg)
{
  gint i, id, retval;
  gboolean ok = false;
  FILE *fp;
  gchar *suffixes[] = {".linecolors"};

  if (reinit)
/*    br_line_color_alloc (gg);*/
    br_line_vectors_check_size (gg->nsegments, gg);

  if (!gg->mono_p) {
    /*
     * Check if line colors file exists.
    */
    if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0) {
      if ( (fp=open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
          ok = true;

      if (!ok && reinit == true)
        ;  /* no need to init the ids */
      else {
        /*
         * read integers between 0 and 9, indices of the colors
        */

        i = 0;
        while (i < gg->nsegments) {
          retval = fscanf (fp, "%d", &id);
          if (retval <= 0 || id < 0 || id >= NCOLORS) {
            ok = false;
            g_printerr ("!!Error in reading line colors; using defaults.\n");
            break;
          }

          gg->line.color.vals[i] = gg->line.color_now.vals[i] =
            gg->line.color_prev.vals[i] = id;
          i++;
        }
        fclose(fp);
      }
    }
  }

  if (!ok)
    br_line_color_init (gg);

  return (ok);
}

gboolean
segments_read (gchar *rootname, gboolean startup, ggobid *gg)
  /* startup - Initializing ggobi? */
{
  gint fs, nblocks, bsize = 500;
  gboolean ok = true;
  gint jlinks = 0;
  FILE *fp;
  gchar *fname;

  if ((rootname == NULL) || (strcmp (rootname, "") == 0) || 
      strcmp (rootname, "stdin") == 0) {
/*    segments_create (gg);*/
    return (ok);
  } else {
    fname = g_malloc (128 * sizeof (gchar));
    /* This is for the in-process case */
    if (rootname == (gchar *) NULL)
      strcpy (fname, gg->filename);
    /* This is for the startup case */
    else
      strcpy (fname, rootname);
    strcat (fname, ".lines");
  }

  if ((fp = fopen (fname, "r")) != NULL) {
    gint a, b;

    gg->nsegments = 0;
    /*
     * Allocate space for <bsize> connecting lines.
    */
    segments_alloc (bsize, gg);
    nblocks = 1;
    while (1)
    {
      fs = fscanf (fp, "%d %d", &a, &b);
      if (fs == EOF)
        break;
      else if (fs < 0) {
        ok = false;
        g_printerr ("Error in reading .lines file\n");
        exit (1);
      }

      if (a < 1 || b > gg->nrows) {
        ok = false;
        g_printerr ("Entry in .lines file > number of rows or < 1\n");
        exit (1);
      }
      else {
        /*
         * Sort lines data such that a <= b
        */
        if (a <= b) {
          gg->segment_endpoints[gg->nsegments].a = a;
          gg->segment_endpoints[gg->nsegments].b = b;
        } else {
          gg->segment_endpoints[gg->nsegments].a = b;
          gg->segment_endpoints[gg->nsegments].b = a;
        }

        (gg->nsegments)++;
        jlinks++;
        if (jlinks == bsize) {
          /*
           * Allocate space for <bsize> more connecting links.
          */
          nblocks++;
          segments_alloc (nblocks*bsize, gg);
          jlinks = 0;
        }
      }
    } /* end while */
    /*
     * Close the data file
    */
    if (fclose (fp) == EOF)
      g_printerr ("Error in closing .lines file");
  }
/*
 *else
 *  segments_create (gg);
*/

  if (fname != (gchar *) NULL)
    g_free ((gpointer) fname);
  return (ok);
}


/*------------------------------------------------------------------------*/
/*                          erasing                                       */
/*------------------------------------------------------------------------*/

gboolean
hidden_read (gchar *ldata_in, gboolean reinit, ggobid *gg)
/*
 * Read in the hidden vector
*/
{
  gchar *suffixes[] = {".hide"};
  gint itmp, i;
  gboolean found = false;
  FILE *fp;

  if (reinit)
    hidden_alloc (gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in,"stdin") != 0)
    if ((fp=open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < gg->nrows)) {
      gg->hidden[i] = gg->hidden_now[i] = gg->hidden_prev[i] =
        (gboolean) itmp;
      i++;
    }
  
    if (i < gg->nrows)
      g_printerr ("Problem in reading hide file; not enough rows\n");

  } else {
    if (reinit)
      hidden_init (gg);
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                         missing values                                 */
/*------------------------------------------------------------------------*/


void
missing_values_read (gchar *ldata_in, gboolean init, ggobid *gg)
{
  gchar *suffixes[] = {".missing"};
  gint i, j, ok, itmp, row, col;
  gint nmissing = 0;
  FILE *fp;
  gboolean found = false;

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in,"stdin") != 0)
    if ((fp=open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    if (init || gg->nmissing == 0)
      arrays_alloc (&gg->missing, gg->nrows, gg->ncols);

    for (j=0; j<gg->ncols; j++)
      gg->vardata[j].nmissing = 0;

    j = 0;
    i = 0;
    while ((ok = fscanf (fp, "%d", &itmp)) != EOF) {
      row = i;
      col = j;
      j++;
      if (j==gg->ncols) { j=0; i++; }
      if (i==gg->nrows && j>0) ok = false;

      if (!ok) {
        g_print ("Problem reading %s.missing", ldata_in);
        g_print (" at row %d, column %d.\n", i, j);
        g_print ("Make sure dimensions of %s and %s.missing match\n",
          ldata_in, ldata_in);
        fclose (fp);
        exit (1);
      }

      gg->missing.vals[row][col] = itmp;
      if (itmp != 0) {
        nmissing++;
        gg->vardata[col].nmissing++;
      }
    }

    if (gg->nmissing != 0 && gg->nmissing != nmissing) {
      g_print ("I found %d missing values in your data file\n", gg->nmissing);
      g_print (" but %d missing values in your .missing file.", nmissing);
      g_print ("I'll use the .missing results.\n");
    }
    gg->nmissing = nmissing;

    fclose (fp);
  }
}

