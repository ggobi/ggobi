/* utils.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"



#ifdef __cplusplus
extern "C" {
#endif

extern void sgenrand (unsigned long);
extern double genrand (void);
extern void lsgenrand(unsigned long seed_array);

#ifdef __cplusplus
}
#endif

gint
sqdist (gint x1, gint y1, gint x2, gint y2) {
  return ((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

void
init_random_seed () {
  sgenrand ((glong) time ((glong *) 0));
}


/* returns a random number on [0.0,1.0] */
gdouble
randvalue (void) {
  return (genrand ());   
}

/*-- returns two random numbers on [-1,1] --*/
void
rnorm2 (gdouble *drand, gdouble *dsave) {
  *drand = 2.0 * genrand () - 1.0;
  *dsave = 2.0 * genrand () - 1.0;
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
  guint i;
  static gchar* gnames[] =
    {"plus", "x", "openrectangle",  "filledrectangle", "opencircle",
    "filledcircle", "point"};
  for (i=0; i < sizeof(gnames)/sizeof(gnames[0]); i++) 
    names[i] = gnames[i];
  return (7);
}

/* ---------------------------------------------------------------------*/
/*                     Missing data routines                            */
/* ---------------------------------------------------------------------*/

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
/*        figuring out if a widget has been initialized:                 */
/*-----------------------------------------------------------------------*/

/*
 * This pair of routines is useful if a widget needs some kind of
 * configuration when it is mapped for the very first time.
*/

gboolean
widget_initialized (GtkWidget *w) {
  gboolean initd = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (w),
    "initialized"));

  return (initd != (gboolean) NULL && initd == true) ? true : false;
}
void
widget_initialize (GtkWidget *w, gboolean initd) {
  gtk_object_set_data (GTK_OBJECT (w),
    "initialized",
    GINT_TO_POINTER (initd));
}

/*-----------------------------------------------------------------------*/
/*  a routine gtk should have for finding out which option is selected   */
/*-----------------------------------------------------------------------*/

/**
 * option_menu_index:
 * @optionmenu: a gtkoptionmenu
 * 
 * Tries to find out (in an ugly way) the selected
 * item in @optionmenu
 * 
 * Return value: the selected index
 **/
gint
option_menu_index (GtkOptionMenu *optionmenu)
{
  GtkMenu *menu;
  GtkMenuItem *selected;
  GList *iterator;
  gint index = -1;
  gint i = 0;

  g_return_val_if_fail (optionmenu != NULL, -1);

  menu = (GtkMenu *) gtk_option_menu_get_menu (optionmenu);
  iterator = GTK_MENU_SHELL (menu)->children;
  selected = (GtkMenuItem *) gtk_menu_get_active (menu);

  while (iterator) {

    if (iterator->data == selected) {
      index = i;
      break;
    }

    iterator = iterator->next;
    i++;
  }

  return index;
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
print_attachments (ggobid *gg) {
  GList *l;
  GtkTableChild *child;

  g_printerr ("attachments:\n");
  for (l=(GTK_TABLE (gg->current_display->table))->children; l; l=l->next) {
    child = (GtkTableChild *) l->data;
    g_printerr (" %d %d, %d %d\n",
      child->left_attach, child->right_attach,
      child->top_attach, child->bottom_attach);
  }
}

gint
address_check (datad *d, ggobid *gg) 
{
  g_printerr ("::: vars.h :::\n");
  g_printerr ("data_mode %d world %d nseg %d rowlab %s jitfac %f\n",
    gg->data_mode, (gint) d->world.vals[0][0], gg->nedges,
    g_array_index (d->rowlab, gchar *, 0), d->jitter.factor);

  return 1;
}

/* ---------------------------------------------------------------------*/
/*     Used in deleting: figure out which elements to keep              */
/* ---------------------------------------------------------------------*/

gint
find_keepers (gint ncols_current, gint nc, gint *cols, gint *keepers)
{
  gint nkeepers;
  gint j, k;

  j = nkeepers = k = 0;
  while (j < ncols_current) {
    while (j < cols[k] && j < ncols_current) {
      keepers[nkeepers++] = j++;
    }
    k++;
    j++;
  }
  if (nkeepers != ncols_current - nc) {
    g_printerr ("your logic is wrong! ncols_to_keep = %d\n", nkeepers);
    exit (1);
  }

  return nkeepers;
}

/*--- set the data_mode depending on the suffix of the filename --*/
DataMode
data_mode_set (gchar *filename)
{
  gint len = strlen (filename);
  gchar *suffix = (gchar *) &filename[len-4];
  DataMode data_mode = ascii_data;

  if (strcmp (suffix, ".dat") == 0)
    data_mode = ascii_data;
  else if (strcmp (suffix, ".bin") == 0)
    data_mode = binary_data;
  else if (strcmp (suffix, ".xml") == 0)
    data_mode = xml_data;

  return data_mode;
}
