/* utils.c */

#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#ifndef USE_DRAND48
#include <sys/types.h>
extern void srandom (unsigned int __seed);
extern int32_t random (void);
#endif

#include <gtk/gtk.h>

#include "vars.h"

gint
sqdist (gint x1, gint y1, gint x2, gint y2) {
  return ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void
init_random_seed () {
#ifdef USE_DRAND48
  /* initialize a seed for drand48() and lrand48() */
  (void) srand48((glong) time ((glong *) 0));
#else
  /* initialize a seed for random (); */
  srandom ((gint) time ((glong *) 0));
#endif
}

gdouble
randvalue (void) {
  gdouble drand;

#ifdef USE_DRAND48
  drand = drand48();    /* rrand on [0.0,1.0] */
#else
  /* random () returns a value on [0, (2**31)-1], or [0, INT_MAX] */
  glong lrand = (glong) random ();
  drand = (gdouble) lrand / (gdouble) INT_MAX;
#endif

  return drand;
}

void
rnorm2(gdouble *drand, gdouble *dsave) {

#ifdef USE_DRAND48
  *drand = 2.0 * drand48() - 1.0;
  *dsave = 2.0 * drand48() - 1.0;
#else
  glong lrand, lsave;
  lrand = random ();
  lsave = random ();
  *drand = 2.0 * (gdouble) lrand / (gdouble) INT_MAX - 1.0;
  *dsave = 2.0 * (gdouble) lsave / (gdouble) INT_MAX - 1.0;
#endif

}


void
add_vgroups (gint *cols, gint *ncols)
/*
 * If one of the chosen columns is in a vgroup,
 * add its comrades (unless they're already present)
*/
{
  gint nc = *ncols;
  gint j, k, n;

  for (j=0; j<nc; j++) {
    gint vg = xg.vgroup_ids[cols[j]];

    for (k=0; k<xg.ncols; k++) {
      if (xg.vgroup_ids[k] == vg && k != cols[j]) {
        /* Got one; if it isn't already in cols, add it */
        gboolean addit = true;
        for (n=0; n<nc; n++) {
          if (cols[n] == k) {
            addit = false;
            break;
          }
        }
        if (addit) cols[(*ncols)++] = k;
        if (*ncols >= xg.ncols)
          break;
      }
    }
  }
}

gint
fcompare (const void *x1, const void *x2)
{
  gint val = 0;
  gfloat *f1 = (gfloat *) x1;
  gfloat *f2 = (gfloat *) x2;

  if (*f1 < *f2)
    val = -1;
  else if (*f1 > *f2)
    val = 1;

  return (val);
}

void
resort_vgroup_ids (gint *group_ids) {
  gint maxid, i, id, newid, j;
  gboolean found;

  /*
   * Find maximum vgroup id.
  */
  maxid = 0;
  for (i=1; i<xg.ncols; i++) {
    if (group_ids[i] > maxid)
      maxid = group_ids[i];
  }

  /*
   * Find minimum vgroup id, set it to 0.  Find next, set it to 1; etc.
  */
  id = 0;
  newid = -1;
  while (id <= maxid) {
    found = false;
    for (j=0; j<xg.ncols; j++) {
      if (group_ids[j] == id) {
        newid++;
        found = true;
        break;
      }
    }
    if (found)
      for (j=0; j<xg.ncols; j++)
        if (group_ids[j] == id)
          group_ids[j] = newid;
    id++;
  }
}

/* Not used anywhere yet ... */
void
fshuffle (gfloat *x, gint n) {
/*
 * Knuth, Seminumerical Algorithms, Vol2; Algorithm P.
*/
  gint i, k;
  gfloat f;

  for (i=0; i<n; i++) {
    k = (gint) (randvalue () * (gdouble) i);
    f = x[i];
    x[i] = x[k];
    x[k] = f;
  }
}

/* ---------------------------------------------------------------------*/
/* The routines below have been added for the R/S connection */

gint glyphIDfromName (gchar *glyphName) {
  gint id = -1;

  if (g_strcasecmp (glyphName, "plus") == 0)
    id = PLUS_GLYPH;
  else if (g_strcasecmp (glyphName, "x") == 0)
    id = X_GLYPH;
  else if (g_strcasecmp (glyphName, "point") == 0)
    id = POINT_GLYPH;
  else if ((g_strcasecmp (glyphName, "open rectangle") == 0) ||
           (g_strcasecmp (glyphName, "open_rectangle") == 0) ||
           (g_strcasecmp (glyphName, "openrectangle") == 0))
    id = OPEN_RECTANGLE;
  else if ((g_strcasecmp (glyphName, "filled rectangle") == 0) ||
           (g_strcasecmp (glyphName, "filled_rectangle") == 0) ||
           (g_strcasecmp (glyphName, "filledrectangle") == 0))
    id = FILLED_RECTANGLE;
  else if ((g_strcasecmp (glyphName, "open circle") == 0) ||
           (g_strcasecmp (glyphName, "open_circle") == 0) ||
           (g_strcasecmp (glyphName, "opencircle") == 0))
    id = OPEN_CIRCLE;
  else if ((g_strcasecmp (glyphName, "filled circle") == 0) ||
           (g_strcasecmp (glyphName, "filled_circle") == 0) ||
           (g_strcasecmp (glyphName, "filledcircle") == 0))
    id = FILLED_CIRCLE;

  return id;
}

gint glyphNames (gchar **names) {
  gint i;
  static gchar* glyphNames[] =
    {"plus", "x", "openrectangle", "filledrectangle", "opencircle",
    "filledcircle", "point"};
  for (i=0; i<7; i++) names[i] = glyphNames[i];
  return (7);
}

gint varno_from_name (gchar *name) {
  gint i, varno = -1;

  for (i = 0; i < xg.ncols; i++) {
    if (strcmp (name, xg.collab[i]) == 0) {
      varno = i;
      break;
    }
  }
  return varno;

}

/* ---------------------------------------------------------------------*/
/*              Some I/O routines                                       */

/*
 * Open a file for reading
*/
FILE *open_file_r (gchar *f, gchar *suffix)
{
  FILE *fp = NULL;
  gchar *fname;
  struct stat buf;
  gboolean found = false;

  fname = g_strdup_printf ("%s%s", f, suffix);

  found = (stat (fname, &buf) == 0);
  if (found) {
    if (!S_ISDIR (buf.st_mode))
      fp = fopen (fname, "r");
  }

  if (fp != NULL) {
    /*
     * Make sure it isn't an empty file -- get a single character
    */
    gint ch = getc (fp);
    if (ch == EOF) {
      g_printerr ("%s is an empty file!\n", fname);
      fclose (fp);
      fp = NULL;
    } else ungetc (ch, fp);
  }

  g_free (fname);
  return fp;
}


FILE *open_xgobi_file_r (gchar *fname, gint nsuffixes, gchar **suffixes,
  gboolean optional)
{
  FILE *fp = NULL;
  gint n;

  if (nsuffixes == 0)
    fp = open_file_r (fname, "");

  else {
    for (n=0; n<nsuffixes; n++) {
      if ( (fp = open_file_r (fname, suffixes[n])) != NULL)
        break;
    }
  }

  if (fp == NULL && !optional) {
    GString *gstr = g_string_new ("Unable to open ");
    if (nsuffixes > 0) {
      for (n=0; n<nsuffixes; n++) {
        if (n < nsuffixes-1)
          g_string_sprintfa (gstr, " %s%s or", fname, suffixes[n]);
        else
          g_string_sprintfa (gstr, " %s%s", fname, suffixes[n]);
      }

    } else 
      g_string_sprintfa (gstr, "%s", fname);

    g_printerr ("%s\n", gstr->str);
    g_string_free (gstr, true);
  }

  return fp;
}

/* missing routines */

GtkTableChild *
gtk_table_get_child (GtkWidget *w, gint left, gint top) {
  GtkTable *table = GTK_TABLE (w);
  GtkTableChild *ch, *child = null;
  GList *l;

  for (l=table->children; l; l=l->next) {
    ch = (GtkTableChild *) l->data;
    if (ch->left_attach == left && ch->top_attach == top) {
      child = ch;
      break;
    }
  }
  return child;
}

GList *
g_list_remove_nth (GList *list, gint indx) {
  GList *tmp = list;
  gint k = 0;

  while (tmp) {
    if (k != indx)
      tmp = tmp->next;
    else {
      if (tmp->prev)
        tmp->prev->next = tmp->next;
      if (tmp->next)
        tmp->next->prev = tmp->prev;
      
      if (list == tmp)
        list = list->next;
      
      g_list_free_1 (tmp);
      break;
    }
    k++;
  }
  return list;
}

GList *
g_list_replace_nth (GList *list, gpointer item, gint indx) {

  /*
   * remove the item that's there now
  */
  list = g_list_remove_nth (list, indx);

  /*
   * insert the replacement
  */
  list = g_list_insert (list, item, indx);

  return list;
}
