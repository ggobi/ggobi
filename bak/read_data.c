/* read_data.c */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <gtk/gtk.h>
#include "vars.h"

/* external functions */
extern void resort_vgroup_ids ();
extern void alloc_glyph_ids ();
extern void init_glyph_ids ();
extern void find_glyph_type_and_size (gint, glyphv *);
extern void alloc_color_ids ();
extern void update_world ();
extern FILE *open_xgobi_file_r (gchar *, gint, gchar **, gboolean);
/*                    */


#define INITSTRSIZE 512

void
alloc_rowlabels () {
  gint i;

g_printerr ("alloc_rowlabels\n");
  xg.rowlab = (gchar **) g_malloc (xg.nrows * sizeof (gchar *));
  for (i=0; i<xg.nrows; i++)
    xg.rowlab[i] = (gchar *) g_malloc (ROWLABLEN * sizeof (gchar));
}

gboolean
read_rowlabels (gchar *data_in, gboolean init)
{
  gint i;
  static gchar *suffixes[] = {
    ".row", ".rowlab", ".case"
  };
  gchar initstr[INITSTRSIZE];
  gint ncase;
  gboolean found = false;
  FILE *fp;

g_printerr ("read_rowlabels\n");

  if (init)
    alloc_rowlabels ();

  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 3, suffixes, true)) != NULL)
      found = true;

  /*
   * Read in case labels or initiate them to generic if no label
   * file exists
  */
    if (found)
    {
      gint k, len;
      ncase = 0;

      k = 0;  /* k is the file row */
      while (fgets (initstr, INITSTRSIZE-1, fp) != NULL)
      {
        if (xg.file_read_type == read_all ||
            k == xg.file_rows_sampled[ncase])
        {
          len = MIN ((int) strlen (initstr), ROWLABLEN-1) ;

          /* trim trailing blanks, and eliminate the carriage return */
          while (initstr[len-1] == ' ' || initstr[len-1] == '\n')
            len-- ;
          strncpy (xg.rowlab[ncase], initstr, len);
          xg.rowlab[ncase][len] = '\0' ;
  
          ncase++;
          if (ncase >= xg.nrows)
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
      if (init) {  /* apply defaults if initialization; else, do nothing */

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

void
alloc_collabels () {

g_printerr ("(alloc_collabels) ncols %d\n", xg.ncols);

  /*
   * Use ncols here; allocate space for the brushing
   * groups variable label.
  */
  xg.collab = (gchar **) g_malloc (xg.ncols * sizeof (gchar *));
  xg.collab_short = (gchar **) g_malloc (xg.ncols * sizeof (gchar *));
  xg.collab_tform1 = (gchar **) g_malloc (xg.ncols * sizeof (gchar *));
  xg.collab_tform2 = (gchar **) g_malloc (xg.ncols * sizeof (gchar *));
}

gboolean
read_collabels (gchar *data_in, gboolean init)
{
  static gchar *suffixes[] = {
    ".col", ".column", ".collab", ".var"
  };
  gint i, nvar = 0;
  gboolean found = false;
  FILE *fp;
  gchar initstr[INITSTRSIZE];
  gchar *lbl, *lbl_short;

  alloc_collabels ();

  /*
   * Check if variable label file exists, and open if so.
  */
  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 4, suffixes, true)) != NULL)
      found = true;
  
  /*
   * Read in variable labels or initiate them to generic if no label
   * file exists
  */
  if (found)
  {
    gint len;
    nvar = 0;

    while (fgets (initstr, INITSTRSIZE-1, fp) != NULL) {
      lbl = strtok ((char *)initstr, "|");
      lbl_short = strtok ((char *) NULL, "|");

      len = MIN ((int) strlen (lbl), COLLABLEN-1) ;
      while (lbl[len-1] == '\n' || lbl[len-1] == ' ')
        len-- ;
      xg.collab[nvar] = g_strndup (lbl, len) ;

      if (lbl_short == NULL || strlen (lbl_short) == 0)
        xg.collab_short[nvar] = g_strdup (xg.collab[nvar]) ;
      else {
        len = MIN ((int) strlen (lbl_short), COLLABLEN-1) ;
        while (lbl_short[len-1] == '\n' || lbl_short[len-1] == ' ')
          len-- ;
        xg.collab_short[nvar] = g_strndup (lbl_short, len) ;
      }
  
      nvar++;
      if (nvar >= xg.ncols)
        break;
    }

    if (init && nvar != xg.ncols) {
      g_printerr ("number of labels = %d, number of cols = %d\n",
        nvar, xg.ncols);

      if (xg.single_column) {
        g_free (xg.collab[1]);
        xg.collab[1] = g_strdup_printf ("%s", xg.collab[0]);
        g_free (xg.collab[0]);
        xg.collab[0] = g_strdup ("Index");

      } else {
        for (i=nvar; i<xg.ncols; i++) {
          xg.collab[i] = g_strdup_printf ("Var %d", i+1);
          xg.collab_short[i] = g_strdup_printf ("V%d", i+1);
        }
      }
    }
  }
  else
  {
    if (init) {
      for (i=0; i<xg.ncols; i++) {
        xg.collab[i] = g_strdup_printf ("Var %d", i+1);
        xg.collab_short[i] = g_strdup_printf ("V%d", i+1);
      }
    }
  }
  
  return (found);
}

/***************************** vgroups *********************************/

void
init_single_vgroup () {
  gint j;

  xg.vgroup_ids = (gint *) g_malloc (xg.ncols * sizeof (gint));
  for (j=0; j<xg.ncols; j++)
    xg.vgroup_ids[j] = 0;
}

void
alloc_vgroups () {
  if (xg.vgroup_ids_ori != NULL)
    g_free ((gpointer) xg.vgroup_ids_ori);
  if (xg.vgroup_ids != NULL)
    g_free ((gpointer) xg.vgroup_ids);

  xg.vgroup_ids_ori = (gint *) g_malloc (xg.ncols * sizeof (gint));
  xg.vgroup_ids = (gint *) g_malloc (xg.ncols * sizeof (gint));
}

gboolean
read_vgroups (gchar *data_in, gboolean init)
/*
 * Read in the grouping numbers for joint scaling of variables
*/
{
  gchar *suffixes[] = {".vgroups"};
  gint itmp, i, j;
  gboolean found = false;
  FILE *fp;

  if (init)
    alloc_vgroups ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    i = 0;
    while ((fscanf (fp, "%d", &itmp) != EOF) && (i < xg.ncols))
      xg.vgroup_ids_ori[i++] = itmp;

    /*
     * Add the vgroup_id value for the extra column.
    */
    if (i == xg.ncols)
      xg.vgroup_ids_ori[i] = i;

    if (init && i < xg.ncols) {
      g_printerr (
        "Number of variables and number of group types do not match.\n");
      g_printerr ("Creating extra generic groups.\n");

      for (j=i; j<xg.ncols; j++)
        xg.vgroup_ids_ori[j] = j;
    }

    resort_vgroup_ids ();
  }

  else {
    if (init)
      for (i=0; i<xg.ncols; i++)
        xg.vgroup_ids_ori[i] = i;
  }

  for (i=0; i<xg.ncols; i++)
    xg.vgroup_ids[i] = xg.vgroup_ids_ori[i];

  return (found);
}

/***************************** rgroups *********************************/

void
free_rgroups () {
  gint i, j;

  for (i=0; i<xg.nrgroups; i++)
    for (j=0; j<xg.rgroups[i].nels; j++)
      g_free ((gpointer) xg.rgroups[i].els);

  g_free ((gpointer) xg.rgroups);
  g_free ((gpointer) xg.rgroup_ids);
}

gboolean
read_rgroups (gchar *data_in, gboolean init)
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
g_printerr ("read_rgroups\n");

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
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
        free_rgroups ();
  
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

    xg.rgroups = (rg_struct *) g_malloc (xg.nrows * sizeof (rg_struct));
    for (i=0; i<xg.nrows; i++) {
      nels[i] = xg.nrows/10;
      xg.rgroups[i].els = (glong *)
        g_malloc ((guint) nels[i] * sizeof (glong));
      xg.rgroups[i].nels = 0;
      xg.rgroups[i].excluded = false;
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
    xg.rgroups = (rg_struct *) g_realloc ((gpointer) xg.rgroups,
      (gulong) (xg.nrgroups * sizeof (rg_struct)));
  
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
  g_printerr ("with the number between 1 and %d; using defaults,\n", NGLYPHS);
  g_printerr ("or a string and a number, with the string being one of\n");
  g_printerr ("+, x, or, ft, oc, fc, .  and the number between 1 and 5.\n");
}

gboolean
read_point_glyphs (gchar *data_in, gboolean addsuffix, gboolean reinit)
{
  gboolean ok = true;
  gchar *suffixes[] = {".glyphs"};
  gint i, k;
  gboolean found;
  FILE *fp;
  gint gid;
  glyphv glyph;
  gboolean use_defaults = false;

  if (reinit)
    alloc_glyph_ids ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (!found && reinit)
    init_glyph_ids ();

  else
  {
    enum { typeAndSize, glyphNumber } glyph_format;
    gint c, retval, gsize;
    gchar *gtype;
    gtype = g_malloc (16 * sizeof (gchar));

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
          fscanf (fp, "%d", &gsize);
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
g_printerr ("gid %d type %d size %d\n", gid, glyph.type, glyph.size);
*/

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
    fclose (fp);
  }

  if (!found || use_defaults)
    init_glyph_ids ();

  return (ok);
}

gboolean
read_point_colors (gchar *data_in, gboolean addsuffix, gboolean reinit)
{
  gboolean ok = true;
  gchar *suffixes[] = {".colors"};
  gint i, k;
  gboolean found;
  FILE *fp;
  gint id;

  if (reinit)
    alloc_color_ids ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0) {
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

    if (!found && reinit == true)
      ;  /* no need to init the ids */
    else {
      gint retval;

      i = 0; k = 0;
      while (i < xg.nrows) {  /* should there be a test on k as well? */

        retval = fscanf (fp, "%d", &id);

        if (retval <= 0) {
          /* not using show_message () here; reading before xgobi startup */
          g_printerr ("!!Error in reading colors file; using defaults.\n");
          break;
        }

        if (xg.file_read_type == read_all ||
           (xg.file_rows_sampled != NULL && k == xg.file_rows_sampled[i]))
        {
          if (id < 0  || id >= NCOLORS) {
            break;
          }

          xg.color_ids[i] = xg.color_now[i] = xg.color_prev[i] = id;

          i++;  /* increment the array index */
        }
        k++;  /* increment the file's row counter */
      }
      fclose (fp);
    }
  }
  return (ok);
}

gboolean
read_connecting_lines (gchar *rootname, gboolean startup)
  /* startup - Initializing xgobi? */
{
  gint fs, nblocks, bsize = 500;
  gboolean ok = true;
  gint jlinks = 0;
  FILE *fp;
  gchar *fname;
g_printerr ("read_connecting_lines\n");

  if ((rootname == NULL) || (strcmp (rootname, "") == 0) || 
      strcmp (rootname, "stdin") == 0) {
/* ggobi disabled
    create_default_lines ();
*/
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

  if ((fp = fopen (fname, "r")) != NULL)
  {
    gint a, b;

    xg.nlines = 0;
    /*
     * Allocate space for <bsize> connecting lines.
    */
    xg.connecting_lines = (connect_lines *) g_malloc (
      (gulong) bsize * sizeof (connect_lines));
    nblocks = 1;
    while (1)
    {
      fs = fscanf (fp, "%d %d", &a, &b);
      if (fs == EOF)
        break;
      else if (fs < 0) {
        ok = false;
/* won't change this one; called at startup */
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
          xg.connecting_lines[xg.nlines].a = a;
          xg.connecting_lines[xg.nlines].b = b;
        } else {
          xg.connecting_lines[xg.nlines].a = b;
          xg.connecting_lines[xg.nlines].b = a;
        }

        (xg.nlines)++;
        jlinks++;
        if (jlinks == bsize) {
        /*
         * Allocate space for <bsize> more connecting links.
        */
          nblocks++;

          xg.connecting_lines = (connect_lines *)
            g_realloc ((gpointer) xg.connecting_lines,
            (nblocks*bsize) * sizeof (connect_lines));
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
ggobi disabled
  else
    create_default_lines ();
*/

  if (fname != (gchar *) NULL)
    g_free ((gpointer) fname);
  return (ok);
}

gboolean
read_nlinkable (gchar *data_in, gboolean init)
/*
 * Read in the number of rows to be linked.
*/
{
  gchar *suffixes[] = {".nlinkable"};
  gint itmp;
  gboolean found = false;
  FILE *fp;
g_printerr ("read_nlinkable\n");

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
      if (!xg.excluded[i])
        xg.nlinkable_in_plot++;
  }

  if (xg.nlinkable != xg.nrows)
    g_printerr ("nlinkable = %d\n", xg.nlinkable);

  return (found);
}

/***************************** jitter *********************************/

void
alloc_jitter () {
  gint i;
  xg.jitter_data = (glong **) g_malloc0 (xg.nrows * sizeof (glong *));
  for (i=0; i<xg.nrows; i++)
    xg.jitter_data[i] = (glong *) g_malloc0 (xg.ncols * sizeof (glong));
}
void
init_jitter () {
  gint i, j;
  for (i=0; i<xg.nrows; i++)
    for (j=0; j<xg.ncols; j++)
      xg.jitter_data[i][j] = 0;
}


gboolean
read_jitter_values (gchar *data_in, gboolean reinit)
/*
 * Read in a .jit file of jittered values, nrows by ncols
*/
{
  gchar *suffixes[] = {".jit"};
  glong ltmp;
  gint i, j;
  gint nr = xg.nrows, nc = xg.ncols, icount = 0;
  gboolean found = false;
  FILE *fp;

g_printerr ("read_jitter_values\n");

  alloc_jitter ();

  if (data_in != NULL && data_in != "" && strcmp (data_in, "stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found) {
    i = j = 0;
    while ((fscanf (fp, "%ld", &ltmp) != EOF) && (icount < nr*nc)) {
      icount++;
      xg.jitter_data[i][j] = ltmp;
      j++;
      if (j == nc) {
        i++;
        j = 0;
      }
    }

    if (i < xg.nrows)
      g_printerr ("Problem in reading .jit file; not enough rows\n");
  }
  else
  {
    if (reinit)
      init_jitter ();
  }

  update_world ();

  return (found);
}

/****************************** erased ********************************/

void
alloc_erased ()
{
  if (xg.erased != NULL)  g_free (xg.erased);
  xg.erased = (gboolean *)  g_malloc (xg.nrows * sizeof (gboolean));
}
void
init_erased ()
{
  gint j;

  for (j=0; j<xg.nrows; j++)
    xg.erased[j] = false;
}
gboolean
read_erase (gchar *data_in, gboolean reinit)
/*
 * Read in the erase vector
*/
{
  gchar *suffixes[] = {".erase"};
  gint itmp, i;
  gboolean found = false;
  FILE *fp;

  if (reinit)
    alloc_erased ();

  if (data_in != NULL && data_in != "" && strcmp (data_in,"stdin") != 0)
    if ( (fp=open_xgobi_file_r (data_in, 1, suffixes, true)) != NULL)
      found = true;

  if (found)
  {
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
  }
  else
  {
    if (reinit)
      init_erased ();
  }

  return (found);
}
