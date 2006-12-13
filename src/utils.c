/* utils.c */
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

gint
sqdist (gint x1, gint y1, gint x2, gint y2)
{
  return ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/*
  Convert the specified string to a numeric value.
  This is now done in a locale-insensitive way, so all input that uses this
  function needs to use the C locale (ie decimal points are '.')
 */
gdouble as_number (const char *sval)
{
  return (g_ascii_strtod (sval, NULL));
}
  
gboolean
is_numeric (const gchar * str)
{
  gchar *end;
  g_ascii_strtod (str, &end);
  return (end - str == strlen(str));    
}

gboolean as_logical (const gchar * sval)
{
  guint i;
  gboolean val = false;
  const gchar *const trues[] = { "T", "true", "True", "1" };
  for (i = 0; i < sizeof (trues) / sizeof (trues[0]); i++)
    {
      if (strcmp (sval, trues[i]) == 0)
        return (true);
    }

  return (val);
}

/* returns a random number on [0.0,1.0] */
gdouble
randvalue (void)
{
  return g_random_double ();
}

/*-- returns two random numbers on [-1,1] --*/
void
rnorm2 (gdouble * drand, gdouble * dsave)
{
  *drand = g_random_double_range (-1, 1);
  *dsave = g_random_double_range (-1, 1);
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
/* new rank finder that doesn't need paird, thanks to g_qsort_with_data() */
gint
rank_compare (gconstpointer val1, gconstpointer val2, gpointer data)
{
  gint ind1 = GPOINTER_TO_INT(val1);
  gint ind2 = GPOINTER_TO_INT(val2);
  const gdouble *array = data;
  
  if (array[ind1] < array[ind2])
    return (-1);
  else if (array[ind1] == array[ind2])
    return (0);
  else
    return (1);
}

/* Not used anywhere yet ... */
void
fshuffle (gfloat * x, gint n)
{
/*
 * Knuth, Seminumerical Algorithms, Vol2; Algorithm P.
*/
  gint i, k;
  gfloat f;

  for (i = 0; i < n; i++) {
    k = (gint) (randvalue () * (gdouble) i);
    f = x[i];
    x[i] = x[k];
    x[k] = f;
  }
}

/* ---------------------------------------------------------------------*/
/* The routines below have been added for the R/S connection */
/* ---------------------------------------------------------------------*/

GlyphType
glyphIDfromName (gchar * glyphName)
{
  GlyphType id = UNKNOWN_GLYPH;

  if (g_ascii_strcasecmp (glyphName, "plus") == 0)
    id = PLUS;
  else if (g_ascii_strcasecmp (glyphName, "x") == 0)
    id = X;
  else if (g_ascii_strcasecmp (glyphName, "point") == 0)
    id = DOT_GLYPH;
  else if ((g_ascii_strcasecmp (glyphName, "open rectangle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "open_rectangle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "openrectangle") == 0))
    id = OR;
  else if ((g_ascii_strcasecmp (glyphName, "filled rectangle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "filled_rectangle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "filledrectangle") == 0))
    id = FR;
  else if ((g_ascii_strcasecmp (glyphName, "open circle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "open_circle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "opencircle") == 0))
    id = OC;
  else if ((g_ascii_strcasecmp (glyphName, "filled circle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "filled_circle") == 0) ||
           (g_ascii_strcasecmp (glyphName, "filledcircle") == 0))
    id = FC;

  return id;
}

gint
glyphNames (gchar ** names)
{
  guint i;
  static const gchar *const gnames[] =
    { "plus", "x", "openrectangle", "filledrectangle", "opencircle",
    "filledcircle", "point"
  };
  for (i = 0; i < sizeof (gnames) / sizeof (gnames[0]); i++)
    names[i] = (gchar *) gnames[i];
  return (NGLYPHTYPES);
}

/* ---------------------------------------------------------------------*/
/*                     Missing data routines                            */
/* ---------------------------------------------------------------------*/

GtkTableChild *
gtk_table_get_child (GtkWidget * w, gint left, gint top)
{
  GtkTable *table = GTK_TABLE (w);
  GtkTableChild *ch, *child = NULL;
  GList *l;

  for (l = table->children; l; l = l->next) {
    ch = (GtkTableChild *) l->data;
    if (ch->left_attach == left && ch->top_attach == top) {
      child = ch;
      break;
    }
  }
  return child;
}

GList *
g_list_remove_nth (GList * list, gint indx)
{
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
g_list_replace_nth (GList * list, gpointer item, gint indx)
{

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
widget_initialized (GtkWidget * w)
{
  gboolean initd = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (w),
                                                       "initialized"));

  return (initd != (gboolean) NULL && initd == true) ? true : false;
}

void
widget_initialize (GtkWidget * w, gboolean initd)
{
  g_object_set_data (G_OBJECT (w), "initialized", GINT_TO_POINTER (initd));
}

/*--------------------------------------------------------------------*/
/*--- Find a widget by name, starting from an enclosing container ----*/
/*--------------------------------------------------------------------*/

static gboolean
widget_name_p (GtkWidget * w, gchar * name)
{
  if (strcmp (gtk_widget_get_name (w), name) == 0) {
    return true;
  }
  else
    return false;
}

/*
 * parent must be a container: loop through its children, finding 
 * a widget with the name 'name'
*/
GtkWidget *
widget_find_by_name (GtkWidget * parent, gchar * name)
{
  GtkWidget *w, *namedw = NULL;
  GList *children, *l;

  if (widget_name_p (parent, name))
    namedw = parent;

  else {
    if (GTK_CONTAINER (parent)) {
      children = gtk_container_get_children (GTK_CONTAINER (parent));
      for (l = children; l; l = l->next) {
        if (GTK_IS_WIDGET (l->data)) {
          w = GTK_WIDGET (l->data);
          if (widget_name_p (w, name)) {
            namedw = w;
            break;
          }
          if (GTK_IS_CONTAINER (w)) {
            namedw = widget_find_by_name (w, name);
            if (namedw != NULL)
              break;
          }
        }
      }
    }
  }

  return namedw;
}

/*--------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                            Debugging                                  */
/*-----------------------------------------------------------------------*/

void
print_attachments (ggobid * gg)
{
  GList *l;
  GtkTableChild *child;

  g_printerr ("attachments:\n");
  for (l = (GTK_TABLE (gg->current_display->table))->children; l; l = l->next) {
    child = (GtkTableChild *) l->data;
    g_printerr (" %d %d, %d %d\n",
                child->left_attach, child->right_attach,
                child->top_attach, child->bottom_attach);
  }
}

/* ---------------------------------------------------------------------*/
/*     Used in deleting: figure out which elements to keep              */
/* ---------------------------------------------------------------------*/

guint *
find_keepers (gint ncols_current, GSList *cols, guint * nkeepers)
{
  gint j;
  guint nc = g_slist_length(cols);
  guint *keepers = g_new(guint, ncols_current - nc);

  j = *nkeepers = 0;
  for (j = 0; j < ncols_current; j++) {
    if (cols) {
      if (GPOINTER_TO_INT(cols->data) != j) {
        keepers[(*nkeepers)++] = j;
      }
      else {
        cols = cols->next;
      }
    }
    else {
      keepers[(*nkeepers)++] = j;
    }
  }

  g_return_val_if_fail(*nkeepers == ncols_current - nc, NULL);

  return keepers;
}

static gint
GGobiSleepTimer (gpointer data)
{
  gtk_main_quit ();             // Makes the innermost invocation of the main loop return when it regains control.
  return (0);
}

void
ggobi_sleep (guint interval)
{
  g_timeout_add (interval * 1000, GGobiSleepTimer, NULL);
  gtk_main ();
}

gboolean
in_vector (gint k, gint * vec, gint nels)
{
  gint j;
  gboolean in = false;
  for (j = 0; j < nels; j++) {
    if (k == vec[j]) {
      in = true;
      break;
    }
  }
  return in;
}

/* may be used eventually to list keys in hash tables */
/*private void
hash_table_keys_foreach(gpointer key, gpointer value, gpointer user_data)
{
  GSList **keys = user_data;
  *keys = g_slist_append(*keys, key);
}

public GSList *
hash_table_list_keys(GHashTable *hash)
{
  GSList *keys = NULL;
  g_hash_table_foreach(hash, self_hash_table_keys_foreach, &keys);
  return keys;
}*/
