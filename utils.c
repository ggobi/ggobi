/* utils.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#if defined (_WIN32)
extern gdouble floor (gdouble);    
extern gdouble ceil (gdouble);     
extern gdouble fabs (gdouble);
#include <sys/stat.h> 
#else
/* on linux, this sometimes insists on being spelled out */
extern void srand48 (glong);
extern double drand48 (void);
#endif

gint
sqdist (gint x1, gint y1, gint x2, gint y2) {
  return ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void
init_random_seed () {
#if defined (_WIN32)
  srand ((gint) time ((glong *) 0));
#elif defined (USE_RANDOM)
  srandom ((gint) time ((glong *) 0));
#else
  srand48 ((glong) time ((glong *) 0));
#endif
}

gdouble
randvalue (void) {
  gdouble drand;

#if defined (_WIN32)
  gint rval = rand ();
  drand = ((double) rval / RAND_MAX);
#elif defined (USE_RANDOM)
  /* random () returns a value on [0, (2**31)-1], or [0, INT_MAX] */
  glong lrand = (glong) random ();
  drand = (gdouble) lrand / (gdouble) INT_MAX;
#else
  drand = drand48();    /* rrand on [0.0,1.0] */
#endif

  return drand;
}

void
rnorm2 (gdouble *drand, gdouble *dsave) {

#if defined (_WIN32)
  *drand = 2.0 * randvalue() - 1.0;
  *dsave = 2.0 * randvalue() - 1.0;
#elif defined (USE_RANDOM)
  glong lrand, lsave;
  lrand = random ();
  lsave = random ();
  *drand = 2.0 * (gdouble) lrand / (gdouble) INT_MAX - 1.0;
  *dsave = 2.0 * (gdouble) lsave / (gdouble) INT_MAX - 1.0;
#else
  *drand = 2.0 * drand48() - 1.0;
  *dsave = 2.0 * drand48() - 1.0;
#endif
}

gint
fcompare (const void *x1, const void *x2)
{
  gint val = 0;
  const gfloat *f1 = (const gfloat *) x1;
  const gfloat *f2 = (const gfloat *) x2;

  if (*f1 < *f2)
    val = -1;
  else if (*f1 > *f2)
    val = 1;

  return (val);
}

/*-- used to find ranks --*/
gint
pcompare (const void *val1, const void *val2)
{
  const paird *pair1 = (const paird *) val1;
  const paird *pair2 = (const paird *) val2;

  if (pair1->f < pair2->f)
    return (-1);
  else if (pair1->f == pair2->f)
    return (0);
  else
    return (1);
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

gdouble myrint (gdouble x) {
  gdouble xrint;
  gdouble xmin = floor (x);
  gdouble xmax = ceil (x);
/*
 *  In the default rounding mode, round to nearest, rint(x) is the integer
 *  nearest x with the additional stipulation that if |rint(x)-x|=1/2 then
 *  rint(x) is even.  Other rounding modes can make rint act like floor,
 *  or like ceil, or round towards zero.
*/
  xrint = (x - xmin < xmax - x) ? xmin : xmax;

  if (fabs (xrint - x) == .5) {
    xrint = ((gint) xmin % 2 == 0) ? xmin : xmax;
  }

  return xrint;
}

/* ---------------------------------------------------------------------*/
/* The routines below have been added for the R/S connection */
/* ---------------------------------------------------------------------*/

gint
glyphIDfromName (gchar *glyphName) {
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

gint
glyphNames (gchar **names) {
  gint i;
  static gchar* gnames[] =
    {"plus", "x", "openrectangle", "filledrectangle", "opencircle",
    "filledcircle", "point"};
  for (i=0; i<7; i++) names[i] = gnames[i];
  return (7);
}

/* ---------------------------------------------------------------------*/
/*              Some I/O routines                                       */
/* ---------------------------------------------------------------------*/

/*
 * Open a file for reading
*/
FILE *open_file_r (gchar *f, gchar *suffix)
{
  FILE *fp = NULL;
  gchar *fname;

  fname = g_strdup_printf ("%s%s", f, suffix);

#ifndef _WIN32
  if (access (fname, R_OK) == 0)
#endif
    fp = fopen (fname, "r");

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
      if ((fp = open_file_r (fname, suffixes[n])) != NULL)
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

/* ---------------------------------------------------------------------*/
/*              End of I/O routines                                     */
/* ---------------------------------------------------------------------*/


/* missing routines */

GtkTableChild *
gtk_table_get_child (GtkWidget *w, gint left, gint top) {
  GtkTable *table = GTK_TABLE (w);
  GtkTableChild *ch, *child = NULL;
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

/*-----------------------------------------------------------------------*/
/*                            Debugging                                  */
/*-----------------------------------------------------------------------*/

void
print_lists (displayd *display) {
  GList *l;

  g_printerr ("columns: ");
  for (l=display->scatmat_cols; l; l=l->next)
    g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  g_printerr ("\n");
  g_printerr ("rows: ");
  for (l=display->scatmat_rows; l; l=l->next)
    g_printerr ("%d ", GPOINTER_TO_INT (l->data));
  g_printerr ("\n");
}

void
print_dims (displayd *d) {
  g_printerr ("children: %d\n", 
    g_list_length (gtk_container_children (GTK_CONTAINER (d->table))));

  g_printerr ("scatmat_ncols= %d, scatmat_nrows= %d ; ",
    g_list_length (d->scatmat_cols), g_list_length (d->scatmat_rows));
  g_printerr ("rows= %d, cols= %d\n",
    GTK_TABLE (d->table)->nrows, GTK_TABLE (d->table)->ncols);
}

void
print_attachments () {
  GList *l;
  GtkTableChild *child;

  g_printerr ("attachments:\n");
  for (l=(GTK_TABLE (current_display->table))->children; l; l=l->next) {
    child = (GtkTableChild *) l->data;
    g_printerr (" %d %d, %d %d\n",
      child->left_attach, child->right_attach,
      child->top_attach, child->bottom_attach);
  }
}

gint
get_vgroup_cols (gint k, gint *cols) {
/*
 * Return all columns in the same vgroup as k
*/
  gint j, ncols = 0;
  gint groupno = xg.vardata[k].groupid;

  for (j=0; j<xg.ncols; j++)
    if (xg.vardata[j].groupid == groupno)
       cols[ncols++] = j;

  return ncols;
}

gint
selected_cols_get (gint *cols, gboolean add_vgroups)
{
/*
 * Figure out which columns are selected.  If (add_vgroups),
 * add any columns in the same vgroup as any of those selected
*/
  gint j, ncols = 0, k, n;
  gint groupno;
  gboolean included;

  for (j=0; j<xg.ncols; j++)
    if (xg.vardata[j].selected)
      cols[ncols++] = j;

  if (add_vgroups) {
    /*-- now add their fellow vgroup members --*/
    for (j=0; j<xg.ncols; j++) {
      groupno = xg.vardata[j].groupid;
      /*-- look for j's fellow group members --*/
      for (k=0; k<xg.ncols; k++) {
        if (k != j && xg.vardata[k].groupid == groupno) {
          /*-- if k is not already in cols, add it */
          included = false;
          for (n=0; n<ncols; n++) {
            if (cols[n] == k) {
              included = true;
              break;
            }
          }
          if (!included)
            cols[ncols++] = k;
        }
      }
    }
  }
  return (ncols);
}

/*
 * When there aren't any columns in the variable statistics table,
 * this is how we find out which columns are selected for plotting.
*/
gint
plotted_cols_get (gint *cols, gboolean add_vgroups) 
{
  gint mode = mode_get ();
  gint j, ncols;
  splotd *sp = current_splot;
  displayd *display = (displayd *) sp->displayptr;

  for (j=0; j<xg.ncols; j++) {
    /*-- if j is plotted in the current splot ... --*/
    switch (display->displaytype) {
      case scatterplot:
        switch (mode) {
          case P1PLOT:
            vartable_select_var (sp->p1dvar, true);
            break;
          case XYPLOT:
            vartable_select_var (sp->xyvars.x, true);
            vartable_select_var (sp->xyvars.y, true);
            break;
        }
        break;
      case scatmat:
        break;
      case parcoords:
        break;
    }
  }

  ncols = selected_cols_get (cols, add_vgroups);

  /*
   * Having highlighted the plotted variables in the variable table
   * we now deselect them
  */
  vartable_unselect_all ();

  return ncols;
}

gint
address_check () 
{
  g_printerr ("::: vars.h :::\n");
  g_printerr ("data_mode %d world %d nseg %d rowlab %s jitfac %f\n",
    xg.data_mode, (gint) xg.world.data[0][0], xg.nsegments,
    xg.rowlab[0], xg.jitter_factor);

  return 1;
}
