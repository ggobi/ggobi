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

/*------------------------------------------------------------------------*/
/*                          row labels                                    */
/*------------------------------------------------------------------------*/

void rowlabels_free () {
  gint i;

  for (i=0; i<xg.nrows; i++)
    g_free (xg.rowlab[i]);
  g_free (xg.rowlab);
}


void
rowlabels_alloc () {
  gint i;

  if (xg.rowlab != NULL) rowlabels_free ();
  
  xg.rowlab = (gchar **) g_malloc (xg.nrows * sizeof (gchar *));
  for (i=0; i<xg.nrows; i++)
    xg.rowlab[i] = (gchar *) g_malloc (ROWLABLEN * sizeof (gchar));
}

gboolean
rowlabels_read (gchar *data_in, gboolean init)
{
  gint i;
  static gchar *suffixes[] = {
    ".row", ".rowlab", ".case"
  };
  gchar initstr[INITSTRSIZE];
  gint ncase;
  gboolean found = false;
  FILE *fp;

  if (init)
    rowlabels_alloc ();

  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ((fp = open_xgobi_file_r (data_in, 3, suffixes, true)) != NULL)
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
      if (xg.file_read_type == read_all || k == xg.file_rows_sampled[ncase])
      {
        len = MIN ((int) strlen (initstr), ROWLABLEN-1) ;

        /* trim trailing blanks, and eliminate the carriage return */
        while (initstr[len-1] == ' ' || initstr[len-1] == '\n')
          len-- ;
        initstr[len] = '\0';
        xg.rowlab[ncase] = g_strdup (initstr) ;
  
        if (ncase++ >= xg.nrows)
          break;
      }
      k++;  /* read the next row ... */
    }
  
    /*
     * If there aren't enough labels, use blank labels for
     * the remainder.
    */
    if (init && ncase != xg.nrows) {
      g_printerr ("number of labels = %d, number of rows = %d\n",
        ncase, xg.nrows);
      for (i=ncase; i<xg.nrows; i++)
        xg.rowlab[i] = g_strdup (" ");
    }
  }
  else
  {
    if (init) {  /* apply defaults if initializing; else, do nothing */

      for (i=0; i<xg.nrows; i++) {
        if (xg.file_read_type == read_all)
          xg.rowlab[i] = g_strdup_printf ("%d", i+1);
        else
          xg.rowlab[i] = g_strdup_printf ("%ld", xg.file_rows_sampled[i]+1);
      }
    }
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       column labels                                    */
/*------------------------------------------------------------------------*/

gboolean
collabels_read (gchar *data_in, gboolean init)
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
  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ((fp=open_xgobi_file_r (data_in, 4, suffixes, true)) != NULL)
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
      xg.vardata[nvar].collab = g_strdup (initstr) ;

      if (nvar++ >= xg.ncols)
        break;
    }

    if (init && nvar != xg.ncols) {
      g_printerr ("number of labels = %d, number of cols = %d\n",
        nvar, xg.ncols);

      if (xg.single_column) {
        g_free (xg.vardata[1].collab);
        xg.vardata[1].collab = g_strdup_printf ("%s", xg.vardata[0].collab);
        g_free (xg.vardata[0].collab);
        xg.vardata[0].collab = g_strdup ("Index");

      } else {
        for (j=nvar; j<xg.ncols; j++) {
          xg.vardata[j].collab = g_strdup_printf ("Var %d", j+1);
        }
      }
    }
  }
  else
  {
    if (init) {
      for (j=0; j<xg.ncols; j++) {
        xg.vardata[j].collab = g_strdup_printf ("Var %d", j+1);
      }
    }
  }


  for (j=0; j<xg.ncols; j++) {
    xg.vardata[j].collab_tform = g_strdup (xg.vardata[j].collab);
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       variable groups                                  */
/*------------------------------------------------------------------------*/

gboolean
vgroups_read (gchar *data_in, gboolean init)
/*
 * Read in the grouping numbers for joint scaling of variables
*/
{
  gchar *suffixes[] = {".vgroups"};
  gint itmp, i, j;
  gboolean found = false;
  FILE *fp;

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ((fp = open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < xg.ncols))
      xg.vardata[i++].groupid_ori = itmp;

    if (init && i < xg.ncols) {
      g_printerr (
        "Number of variables and number of group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (j=i; j<xg.ncols; j++)
        xg.vardata[j].groupid_ori = j;
    }

    vgroups_sort ();

  } else {

    if (init)
      for (j=0; j<xg.ncols; j++)
        xg.vardata[j].groupid_ori = j;
  }

  for (j=0; j<xg.ncols; j++)
    xg.vardata[j].groupid = xg.vardata[j].groupid_ori;

  return (found);
}

/*------------------------------------------------------------------------*/
/*                       row groups                                       */
/*------------------------------------------------------------------------*/

void
rgroups_free () {
  gint i, j;

  for (i=0; i<xg.nrgroups; i++)
    for (j=0; j<xg.rgroups[i].nels; j++)
      g_free ((gpointer) xg.rgroups[i].els);

  g_free ((gpointer) xg.rgroups);
  g_free ((gpointer) xg.rgroup_ids);
}

gboolean
rgroups_read (gchar *data_in, gboolean init)
/*
 * Read in the grouping numbers for joint scaling of variables
*/
{
  gchar *suffixes[] = {".rgroups"};
  gint itmp, i, k;
  gboolean found = false;
  gboolean found_rg;
  FILE *fp;
  glong *nels;
  gint nr;

  if (xg.nrgroups > 0) rgroups_free ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ((fp = open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;
  
  if (!found) {
    xg.nrgroups = 0;
  } else {
  
    /*
     * If this isn't the first time we've read files, then
     * see if the rgroups structures should be freed.
    */
    if (!init)
      if (xg.nrgroups > 0)
        rgroups_free ();
  
    /* rgroup_ids starts by containing the values in the file */
    xg.rgroup_ids = (glong *) g_malloc (xg.nrows * sizeof (glong));
    nels = (glong *) g_malloc (xg.nrows * sizeof (glong));
     
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < xg.nrows))
      xg.rgroup_ids[i++] = itmp;
  
    /* check the number of group ids read -- should be nrows */
    if (init && i < xg.nrows)
    {
      g_printerr (
        "Number of rows and number of row group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (k=i; k<xg.nrows; k++)
        xg.rgroup_ids[k] = k;
    }
  
    /*
     * Initialize the global variables: nrows row groups,
     * nrows/10 elements in each group
    */

    xg.rgroups = (rgroupd *) g_malloc (xg.nrows * sizeof (rgroupd));
    for (i=0; i<xg.nrows; i++) {
      nels[i] = xg.nrows/10;
      xg.rgroups[i].els = (glong *)
        g_malloc ((guint) nels[i] * sizeof (glong));
      xg.rgroups[i].nels = 0;
      xg.rgroups[i].included = true;
    }
    xg.nrgroups = 0;
  
/*
 * For now, only assign linkable points to rgroups.
*/
    /*
     * On this sweep, find out how many groups there are and how
     * many elements are in each group
    */
    nr = (xg.nlinkable < xg.nrows) ? xg.nlinkable : xg.nrows;

    for (i=0; i<nr; i++) {
      found_rg = false;
      for (k=0; k<xg.nrgroups; k++) {
  
        /* if we've found this id before ... */
        if (xg.rgroup_ids[i] == xg.rgroups[k].id) {
  
          /* Reallocate els[k] if necessary */
          if (xg.rgroups[k].nels == nels[k]) {
            nels[k] *= 2;
            xg.rgroups[k].els = (glong *)
              g_realloc ((gpointer) xg.rgroups[k].els,
                (nels[k] * sizeof (glong)));
          }
  
          /* Add the element, increment the element counter */
          xg.rgroups[k].els[ xg.rgroups[k].nels ] = i;
          xg.rgroups[k].nels++;
  
          /*
           * Now the value in rgroup_ids has to change so that
           * it can point to the correct member in the array of
           * rgroups structures
          */
          xg.rgroup_ids[i] = k;
  
          found_rg = true;
          break;
        }
      }
  
      /* If it's a new group id, add it */
      if (!found_rg) {
        xg.rgroups[xg.nrgroups].id = xg.rgroup_ids[i]; /* from file */
        xg.rgroups[xg.nrgroups].nels = 1;
        xg.rgroups[xg.nrgroups].els[0] = i;
        xg.rgroup_ids[i] = xg.nrgroups;  /* rgroup_ids reset to index */
        xg.nrgroups++;
      }
    }
    xg.nrgroups_in_plot = xg.nrgroups;
  
    /* Reallocate everything now that we know how many there are */
    xg.rgroups = (rgroupd *) g_realloc ((gpointer) xg.rgroups,
      (gulong) (xg.nrgroups * sizeof (rgroupd)));
  
    /* Now reallocate the arrays within each rgroups structure */
    for (k=0; k<xg.nrgroups; k++) {
      xg.rgroups[k].els = (glong *)
        g_realloc ((gpointer) xg.rgroups[k].els,
          (gulong) (xg.rgroups[k].nels * sizeof (glong)));
    }
  
    g_free ((gpointer) nels);
  }

  if (xg.nlinkable != xg.nrows)
    g_printerr ("xg.nlinkable=%d xg.nrows=%d\n", xg.nlinkable, xg.nrows);

  if (xg.nrgroups != 0)
    g_printerr ("xg.nrgroups=%ld\n", xg.nrgroups);

  return (found);
}

void
readGlyphErr (void) {
  g_printerr ("The .glyphs file must contain either one number per line,\n");
  g_printerr ("with the number between 1 and %d; using defaults,\n",
    NGLYPHS);
  g_printerr ("or a string and a number, with the string being one of\n");
  g_printerr ("+, x, or, fr, oc, fc, .  and the number between 1 and %d.\n",
    NGLYPHSIZES);
}

/*------------------------------------------------------------------------*/
/*                 point glyphs and colors                                */
/*------------------------------------------------------------------------*/

gboolean
point_glyphs_read (gchar *data_in, gboolean reinit)
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
    br_glyph_ids_alloc ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ((fp = open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (!found && reinit)
    br_glyph_ids_init ();

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
    while (i < xg.nrows) {  /* should there be a test on k as well? */

      if (glyph_format == glyphNumber) {
        retval = fscanf (fp, "%d", &gid);
      } else {
        fscanf (fp, "%s", gtype);
        gsize = 1;
        if (strcmp (gtype, ".") != 0)
          retval = fscanf (fp, "%d", &gsize);
      }

      if (retval <= 0) {
        /* not using show_message () here; reading before xgobi startup */
        g_printerr ("!Error in reading glyphs file; using defaults.\n");
        use_defaults = true;
        break;
      }

      if (xg.file_read_type == read_all ||
         (xg.file_rows_sampled != NULL && k == xg.file_rows_sampled[i]))
      {
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
          if (strcmp (gtype, "+") == 0)
            glyph.type = 1;
          else if (g_strcasecmp (gtype, "x") == 0)
            glyph.type = 2;
          else if (g_strcasecmp (gtype, "or") == 0)
            glyph.type = 3;
          else if (g_strcasecmp (gtype, "fr") == 0)
            glyph.type = 4;
          else if (g_strcasecmp (gtype, "oc") == 0)
            glyph.type = 5;
          else if (g_strcasecmp (gtype, "fc") == 0)
            glyph.type = 6;
          else if (g_strcasecmp (gtype, ".") == 0)
            glyph.type = 7;
          else {
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

        xg.glyph_ids[i].type = xg.glyph_now[i].type =
          xg.glyph_prev[i].type = glyph.type;
        xg.glyph_ids[i].size = xg.glyph_now[i].size =
          xg.glyph_prev[i].size = glyph.size;

        i++;  /* increment the array index */
      }
      k++;  /* increment the file's row counter */
    }
    g_free (gtype);
    fclose (fp);
  }

  if (!found || use_defaults)
    br_glyph_ids_init ();

  return (ok);
}

gboolean
point_colors_read (gchar *data_in, gboolean reinit)
{
  gboolean ok = false;
  gboolean found = false;
  gchar *suffixes[] = {".colors"};
  gint i, k, retval;
  FILE *fp;
  gint id;

  if (reinit)
    br_color_ids_alloc ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0) {
    if ( (fp = open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

    if (!found && reinit == true)
      ;  /* no need to init the ids */
    else {
      ok = true;

      i = 0; k = 0;
      while (i < xg.nrows) {  /* should there be a test on k as well? */

        retval = fscanf (fp, "%d", &id);
        if (retval <= 0 || id < 0 || id >= NCOLORS) {
          ok = false;
          g_printerr ("!!Error in reading colors file; using defaults.\n");
          break;
        }

        if (xg.file_read_type == read_all ||
           (xg.file_rows_sampled != NULL && k == xg.file_rows_sampled[i]))
        {
          xg.color_ids[i] = xg.color_now[i] = xg.color_prev[i] = id;

          i++;  /* increment the array index */
        }
        k++;  /* increment the file's row counter */
      }
      fclose (fp);
    }
  }

  if (!ok)
    br_color_ids_init ();

  return (ok);
}

/*------------------------------------------------------------------------*/
/*                     lines and line colors                              */
/*------------------------------------------------------------------------*/

gboolean
line_colors_read (gchar *data_in, gboolean reinit)
{
  gint i, id, retval;
  gboolean ok = false;
  FILE *fp;
  gchar *suffixes[] = {".linecolors"};

  if (reinit)
    br_line_color_ids_alloc ();

  if (!mono_p) {
    /*
     * Check if line colors file exists.
    */
    if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0) {
      if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
          ok = true;

      if (!ok && reinit == true)
        ;  /* no need to init the ids */
      else {
        /*
         * read integers between 0 and 9, indices of the colors
        */

        i = 0;
        while (i < xg.nsegments) {
          retval = fscanf (fp, "%d", &id);
          if (retval <= 0 || id < 0 || id >= NCOLORS) {
            ok = false;
            g_printerr ("!!Error in reading line colors; using defaults.\n");
            break;
          }

          xg.line_color_ids[i] = xg.line_color_now[i] =
            xg.line_color_prev[i] = id;
          i++;
        }
        fclose(fp);
      }
    }
  }

  if (!ok)
    br_line_color_ids_init ();

  return (ok);
}

gboolean
segments_read (gchar *rootname, gboolean startup)
  /* startup - Initializing xgobi? */
{
  gint fs, nblocks, bsize = 500;
  gboolean ok = true;
  gint jlinks = 0;
  FILE *fp;
  gchar *fname;

  if ((rootname == NULL) || (strcmp (rootname, "") == 0) || 
      strcmp (rootname, "stdin") == 0) {
    segments_create ();
    return (ok);
  } else {
    fname = g_malloc (128 * sizeof (gchar));
    /* This is for the in-process case */
    if (rootname == (gchar *) NULL)
      strcpy (fname, xg.filename);
    /* This is for the startup case */
    else
      strcpy (fname, rootname);
    strcat (fname, ".lines");
  }

  if ((fp = fopen (fname, "r")) != NULL) {
    gint a, b;

    xg.nsegments = 0;
    /*
     * Allocate space for <bsize> connecting lines.
    */
    segments_alloc (bsize);
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

      if (a < 1 || b > xg.nrows) {
        ok = false;
        g_printerr ("Entry in .lines file > number of rows or < 1\n");
        exit (1);
      }
      else {
        /*
         * Sort lines data such that a <= b
        */
        if (a <= b) {
          xg.segment_endpoints[xg.nsegments].a = a;
          xg.segment_endpoints[xg.nsegments].b = b;
        } else {
          xg.segment_endpoints[xg.nsegments].a = b;
          xg.segment_endpoints[xg.nsegments].b = a;
        }

        (xg.nsegments)++;
        jlinks++;
        if (jlinks == bsize) {
          /*
           * Allocate space for <bsize> more connecting links.
          */
          nblocks++;
          segments_alloc (nblocks*bsize);
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
  else
    segments_create ();

  if (fname != (gchar *) NULL)
    g_free ((gpointer) fname);
  return (ok);
}

/*------------------------------------------------------------------------*/
/*                   the single integer nlinkable                         */
/*------------------------------------------------------------------------*/

gboolean
nlinkable_read (gchar *data_in, gboolean init)
/*
 * Read in the number of rows to be linked.
*/
{
  gchar *suffixes[] = {".nlinkable"};
  gint itmp;
  gboolean found = false;
  FILE *fp;

  /*
   * Initialize nlinkable to be all the rows; if not
   * initializing, leave its value alone.
  */
  if (init)
    xg.nlinkable = xg.nrows;

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;
  
  if (found) {
    fscanf (fp, "%d", &itmp);
    if (itmp > 0 && itmp <= xg.nrows)
      xg.nlinkable = itmp;
    fclose (fp);
  }

  if (xg.nrows_in_plot == xg.nrows) xg.nlinkable_in_plot = xg.nlinkable;
  else {
    gint i;
    xg.nlinkable_in_plot = 0;
    for (i=0; i<xg.nlinkable; i++)
      if (xg.included[i])
        xg.nlinkable_in_plot++;
  }

  if (xg.nlinkable != xg.nrows)
    g_printerr ("nlinkable = %d\n", xg.nlinkable);

  return (found);
}

/*------------------------------------------------------------------------*/
/*                          erasing                                       */
/*------------------------------------------------------------------------*/

void
erase_alloc ()
{
  if (xg.erased != NULL)  g_free (xg.erased);
  xg.erased = (gboolean *)  g_malloc (xg.nrows * sizeof (gboolean));
}
void
erase_init ()
{
  gint j;

  for (j=0; j<xg.nrows; j++)
    xg.erased[j] = false;
}
gboolean
erase_read (gchar *data_in, gboolean reinit)
/*
 * Read in the erase vector
*/
{
  gchar *suffixes[] = {".erase"};
  gint itmp, i;
  gboolean found = false;
  FILE *fp;

  if (reinit)
    erase_alloc ();

  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ((fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    gint k = 0;  /* k is the file row, used if file_read_type != read_all */
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < xg.nrows)) {
      if (xg.file_read_type == read_all || k == xg.file_rows_sampled[i]) {
        xg.erased[i++] = (gboolean) itmp;
      }
      k++;
    }
  
    if (i < xg.nrows)
      g_printerr ("Problem in reading erase file; not enough rows\n");

  } else {
    if (reinit)
      erase_init ();
  }

  return (found);
}

/*------------------------------------------------------------------------*/
/*                         missing values                                 */
/*------------------------------------------------------------------------*/


void
missing_values_read (gchar *data_in, gboolean init)
{
  gchar *suffixes[] = {".missing"};
  gint i, j, ok, itmp, row, col;
  gint nmissing = 0;
  FILE *fp;
  gboolean found = false;

  if (xg.file_read_type != read_all)
    return;

  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ((fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    if (init || xg.nmissing == 0)
      arrays_alloc (&xg.missing, xg.nrows, xg.ncols);

    for (j=0; j<xg.ncols; j++)
      xg.vardata[j].nmissing = 0;

    j = 0;
    i = 0;
    while ((ok = fscanf (fp, "%d", &itmp)) != EOF) {
      row = i;
      col = j;
      j++;
      if (j==xg.ncols) { j=0; i++; }
      if (i==xg.nrows && j>0) ok = false;

      if (!ok) {
        g_print ("Problem reading %s.missing", data_in);
        g_print (" at row %d, column %d.\n", i, j);
        g_print ("Make sure dimensions of %s and %s.missing match\n",
          data_in, data_in);
        fclose (fp);
        exit (1);
      }

      xg.missing.data[row][col] = itmp;
      if (itmp != 0) {
        nmissing++;
        xg.vardata[col].nmissing++;
      }
    }

    if (xg.nmissing != 0 && xg.nmissing != nmissing) {
      g_print ("I found %d missing values in your data file\n", xg.nmissing);
      g_print (" but %d missing values in your .missing file.", nmissing);
      g_print ("I'll use the .missing results.\n");
    }
    xg.nmissing = nmissing;

    fclose (fp);
  }
}

