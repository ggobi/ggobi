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

void rowlabels_free (datad *d, ggobid *gg)
{
  g_array_free (d->rowlab, true);  /* unsure about the 2nd arg */
}


void
rowlabels_alloc (datad *d, ggobid *gg) 
{
  if (d->rowlab != NULL) rowlabels_free (d, gg);

  d->rowlab = g_array_new (false, false, sizeof (gchar *));
}

gboolean
rowlabels_read (gchar *ldata_in, gboolean init, datad *d, ggobid *gg)
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
    rowlabels_alloc (d, gg);

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

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       column labels                                    */
/*------------------------------------------------------------------------*/

gboolean
collabels_read (gchar *ldata_in, gboolean init, datad *d, ggobid *gg)
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
      d->vartable[nvar].collab = g_strdup (initstr) ;

      if (nvar++ >= d->ncols)
        break;
    }

    if (init && nvar != d->ncols) {
      g_printerr ("number of labels = %d, number of cols = %d\n",
        nvar, d->ncols);

      if (d->single_column) {
        g_free (d->vartable[1].collab);
        d->vartable[1].collab = g_strdup_printf ("%s", d->vartable[0].collab);
        g_free (d->vartable[0].collab);
        d->vartable[0].collab = g_strdup ("Index");

      } else {
        for (j=nvar; j<d->ncols; j++) {
          d->vartable[j].collab = g_strdup_printf ("Var %d", j+1);
        }
      }
    }
  }
  else
  {
    if (init) {
      for (j=0; j<d->ncols; j++) {
        d->vartable[j].collab = g_strdup_printf ("Var %d", j+1);
      }
    }
  }


  for (j=0; j<d->ncols; j++) {
    d->vartable[j].collab_tform = g_strdup (d->vartable[j].collab);
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       variable groups                                  */
/*------------------------------------------------------------------------*/

gboolean
vgroups_read (gchar *ldata_in, gboolean init, datad *d, ggobid *gg)
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
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < d->ncols))
      d->vartable[i++].groupid_ori = itmp;

    if (init && i < d->ncols) {
      g_printerr (
        "Number of variables and number of group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (j=i; j<d->ncols; j++)
        d->vartable[j].groupid_ori = j;
    }

    vgroups_sort (d, gg);

  } else {

    if (init)
      for (j=0; j<d->ncols; j++)
        d->vartable[j].groupid_ori = j;
  }

  for (j=0; j<d->ncols; j++)
    d->vartable[j].groupid = d->vartable[j].groupid_ori;

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       row groups                                       */
/*------------------------------------------------------------------------*/

void
rgroups_free (datad *d, ggobid *gg) {
  gint i, j;

  for (i=0; i<d->nrgroups; i++)
    for (j=0; j<d->rgroups[i].nels; j++)
      g_free ((gpointer) d->rgroups[i].els);

  g_free ((gpointer) d->rgroups);
  g_free ((gpointer) d->rgroup_ids);
}

gboolean
rgroups_read (gchar *ldata_in, gboolean init, datad *d, ggobid *gg)
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

  if (d->nrgroups > 0) rgroups_free (d, gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0)
    if ((fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;
  
  if (!found) {
    d->nrgroups = 0;
  } else {
  
    /*
     * If this isn't the first time we've read files, then
     * see if the rgroups structures should be freed.
    */
    if (!init)
      if (d->nrgroups > 0)
        rgroups_free (d, gg);
  
    /* rgroup_ids starts by containing the values in the file */
    d->rgroup_ids = (gint *) g_malloc (d->nrows * sizeof (gint));
    nels = (gint *) g_malloc (d->nrows * sizeof (gint));
     
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < d->nrows))
      d->rgroup_ids[i++] = itmp;
  
    /* check the number of group ids read -- should be nrows */
    if (init && i < d->nrows) {
      g_printerr (
        "Number of rows and number of row group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (k=i; k<d->nrows; k++)
        d->rgroup_ids[k] = k;
    }
  
    /*
     * Initialize the global variables: nrows row groups,
     * nrows/10 elements in each group
    */

    d->rgroups = (rgroupd *) g_malloc (d->nrows * sizeof (rgroupd));
    for (i=0; i<d->nrows; i++) {
      nels[i] = d->nrows/10;
      d->rgroups[i].els = (gint *) g_malloc (nels[i] * sizeof (gint));
      d->rgroups[i].nels = 0;
      d->rgroups[i].included = true;
    }
    d->nrgroups = 0;
  
    /*
     * On this sweep, find out how many groups there are and how
     * many elements are in each group
    */
    nr = d->nrows;

    for (i=0; i<nr; i++) {
      found_rg = false;
      for (k=0; k<d->nrgroups; k++) {
  
        /* if we've found this id before ... */
        if (d->rgroup_ids[i] == d->rgroups[k].id) {
  
          /* Reallocate els[k] if necessary */
          if (d->rgroups[k].nels == nels[k]) {
            nels[k] *= 2;
            d->rgroups[k].els = (gint *)
              g_realloc ((gpointer) d->rgroups[k].els,
                (nels[k] * sizeof (gint)));
          }
  
          /* Add the element, increment the element counter */
          d->rgroups[k].els[ d->rgroups[k].nels ] = i;
          d->rgroups[k].nels++;
  
          /*
           * Now the value in rgroup_ids has to change so that
           * it can point to the correct member in the array of
           * rgroups structures
          */
          d->rgroup_ids[i] = k;
  
          found_rg = true;
          break;
        }
      }
  
      /* If it's a new group id, add it */
      if (!found_rg) {
        d->rgroups[d->nrgroups].id = d->rgroup_ids[i]; /* from file */
        d->rgroups[d->nrgroups].nels = 1;
        d->rgroups[d->nrgroups].els[0] = i;
        d->rgroup_ids[i] = d->nrgroups;  /* rgroup_ids reset to index */
        d->nrgroups++;
      }
    }
    d->nrgroups_in_plot = d->nrgroups;
  
    /* Reallocate everything now that we know how many there are */
    d->rgroups = (rgroupd *) g_realloc ((gpointer) d->rgroups,
      (gulong) (d->nrgroups * sizeof (rgroupd)));
  
    /* Now reallocate the arrays within each rgroups structure */
    for (k=0; k<d->nrgroups; k++) {
      d->rgroups[k].els = (gint *)
        g_realloc ((gpointer) d->rgroups[k].els,
                    d->rgroups[k].nels * sizeof (gint));
    }
  
    g_free ((gpointer) nels);
  }

  if (d->nrgroups != 0)
    g_printerr ("d.nrgroups=%d\n", d->nrgroups);

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
point_glyphs_read (gchar *ldata_in, gboolean reinit, datad *d, ggobid *gg)
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
    br_glyph_ids_alloc (d, gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0)
    if ((fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

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
        glyph.type = mapGlyphName (gtype);

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

      d->glyph_ids[i].type = d->glyph_now[i].type =
        d->glyph_prev[i].type = glyph.type;
      d->glyph_ids[i].size = d->glyph_now[i].size =
        d->glyph_prev[i].size = glyph.size;

      i++;  /* increment the array index */
      k++;  /* increment the file's row counter */
    }
    g_free (gtype);
    fclose (fp);
  }

  if (!found || use_defaults)
    br_glyph_ids_init (d, gg);

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
     type = (GlyphType) (i+1);
     break;
    }
  }

  return(type);
}

gboolean
point_colors_read (gchar *ldata_in, gboolean reinit, datad *d, ggobid *gg)
{
  gboolean ok = false;
  gboolean found = false;
  gchar *suffixes[] = {".colors"};
  gint i, k, retval;
  FILE *fp;
  gint id;

  if (reinit)
    br_color_ids_alloc (d, gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in, "stdin") != 0) {
    if ( (fp = open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

    if (!found && reinit == true)
      ;  /* no need to init the ids */
    else {
      ok = true;

      i = 0; k = 0;
      while (i < d->nrows) {  /* should there be a test on k as well? */

        retval = fscanf (fp, "%d", &id);
        if (retval <= 0 || id < 0 || id >= NCOLORS) {
          ok = false;
          g_printerr ("!!Error in reading colors file; using defaults.\n");
          break;
        }

        d->color_ids[i] = d->color_now[i] = d->color_prev[i] = id;

        i++;  /* increment the array index */
        k++;  /* increment the file's row counter */
      }
      fclose (fp);
    }
  }

  if (!ok)
    br_color_ids_init (d, gg);

  return (ok);
}

/*------------------------------------------------------------------------*/
/*                     lines and line colors                              */
/*------------------------------------------------------------------------*/

gboolean
line_colors_read (gchar *ldata_in, gboolean reinit, datad *d, ggobid *gg)
{
  gint i, id, retval;
  gboolean ok = false;
  FILE *fp;
  gchar *suffixes[] = {".linecolors"};

  if (reinit)
/*    br_line_color_alloc (gg);*/
    br_line_vectors_check_size (gg->nedges, gg);

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
        while (i < gg->nedges) {
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
edges_read (gchar *rootname, gboolean startup, datad *d, ggobid *gg)
  /* startup - Initializing ggobi? */
{
  gint fs, nblocks, bsize = 500;
  gboolean ok = true;
  gint jlinks = 0;
  FILE *fp;
  gchar *fname;

  if ((rootname == NULL) || (strcmp (rootname, "") == 0) || 
      strcmp (rootname, "stdin") == 0) {
/*    edges_create (gg);*/  /*-- or maybe not --*/
    return (ok);
  } else {
    fname = (gchar*) g_malloc (128 * sizeof (gchar));
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

    gg->nedges = 0;
    /*
     * Allocate space for <bsize> connecting lines.
    */
    edges_alloc (bsize, d, gg);
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

      if (a < 1 || b > d->nrows) {
        ok = false;
        g_printerr ("Entry in .lines file > number of rows or < 1\n");
        exit (1);
      }
      else {
        /*
         * Sort lines data such that a <= b
        */
        if (a <= b) {
          gg->edge_endpoints[gg->nedges].a = a;
          gg->edge_endpoints[gg->nedges].b = b;
        } else {
          gg->edge_endpoints[gg->nedges].a = b;
          gg->edge_endpoints[gg->nedges].b = a;
        }

        (gg->nedges)++;
        jlinks++;
        if (jlinks == bsize) {
          /*
           * Allocate space for <bsize> more connecting links.
          */
          nblocks++;
          edges_alloc (nblocks*bsize, d, gg);
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
 *  edges_create (gg);
*/

  if (fname != (gchar *) NULL)
    g_free ((gpointer) fname);
  return (ok);
}


/*------------------------------------------------------------------------*/
/*                          erasing                                       */
/*------------------------------------------------------------------------*/

gboolean
hidden_read (gchar *ldata_in, gboolean reinit, datad *d, ggobid *gg)
/*
 * Read in the hidden vector
*/
{
  gchar *suffixes[] = {".hide"};
  gint itmp, i;
  gboolean found = false;
  FILE *fp;

  if (reinit)
    hidden_alloc (d, gg);

  if (ldata_in != NULL && ldata_in != "" && strcmp (ldata_in,"stdin") != 0)
    if ((fp=open_ggobi_file_r (ldata_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < d->nrows)) {
      d->hidden[i] = d->hidden_now[i] = d->hidden_prev[i] =
        (gboolean) itmp;
      i++;
    }
  
    if (i < d->nrows)
      g_printerr ("Problem in reading hide file; not enough rows\n");

  } else {
    if (reinit)
      hidden_init (d, gg);
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                         missing values                                 */
/*------------------------------------------------------------------------*/


void
missing_values_read (gchar *ldata_in, gboolean init, datad *d, ggobid *gg)
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
    if (init || d->nmissing == 0)
      arrays_alloc (&d->missing, d->nrows, d->ncols);

    for (j=0; j<d->ncols; j++)
      d->vartable[j].nmissing = 0;

    j = 0;
    i = 0;
    while ((ok = fscanf (fp, "%d", &itmp)) != EOF) {
      row = i;
      col = j;
      j++;
      if (j==d->ncols) { j=0; i++; }
      if (i==d->nrows && j>0) ok = false;

      if (!ok) {
        g_print ("Problem reading %s.missing", ldata_in);
        g_print (" at row %d, column %d.\n", i, j);
        g_print ("Make sure dimensions of %s and %s.missing match\n",
          ldata_in, ldata_in);
        fclose (fp);
        exit (1);
      }

      d->missing.vals[row][col] = itmp;
      if (itmp != 0) {
        nmissing++;
        d->vartable[col].nmissing++;
      }
    }

    if (d->nmissing != 0 && d->nmissing != nmissing) {
      g_print ("I found %d missing values in your data file\n", d->nmissing);
      g_print (" but %d missing values in your .missing file.", nmissing);
      g_print ("I'll use the .missing results.\n");
    }
    d->nmissing = nmissing;

    fclose (fp);
  }
}

