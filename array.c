/*-- array.c --*/

#include <gtk/gtk.h>
#include <vars.h>


/*-------------------------------------------------------------------------*/
/*                     floating point array management                     */
/*-------------------------------------------------------------------------*/

void
arrayf_init (array_f *arrp)
{
  arrp->nrows = arrp->ncols = 0;
  arrp->data = (gfloat **) NULL;
}

void
arrayf_free (array_f *arrp, gint nr, gint nc)
{
  gint i;

  /*-- if nr != 0, free only the last nrows-nr rows --*/

  for (i=nr; i<arrp->nrows; i++)
    g_free (arrp->data[i]);

  if (nr == 0) {
    g_free (arrp->data);
    arrp->data = (gfloat **) NULL;
  }

  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* Zero a floating point array. */
void
arrayf_zero (array_f *arrp)
{
  int i, j;
  for (i=0; i<arrp->nrows; i++) {
    for (j=0; j<arrp->ncols; j++) {
      arrp->data[i][j] = 0.0;
    }
  }
}

/* allocate a floating point array */
void
arrayf_alloc (array_f *arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0)||(arrp->ncols != 0))
    arrayf_free (arrp, 0, 0);

  arrp->data = (gfloat **) g_malloc (nr * sizeof (gfloat *));
  for (i = 0; i < nr; i++)
    arrp->data[i] = (gfloat *) g_malloc (nc * sizeof (gfloat));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* allocate a floating point array populated with 0 */
void
arrayf_alloc_zero (array_f *arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0)||(arrp->ncols != 0))
    arrayf_free (arrp, 0, 0);

  arrp->data = (gfloat **) g_malloc (nr * sizeof (gfloat *));
  for (i = 0; i < nr; i++)
    arrp->data[i] = (gfloat *) g_malloc0 (nc * sizeof (gfloat));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* increase the number of rows in a floating point array */
void
arrayf_add_rows (array_f *arrp, gint nr)
{
  int i;

  if (nr > arrp->nrows) {

    arrp->data = (gfloat **) g_realloc (arrp->data, nr * sizeof (gfloat *));
    for (i = arrp->nrows; i < nr; i++)
      arrp->data[i] = (gfloat *) g_malloc (arrp->ncols * sizeof (gfloat));

    arrp->nrows = nr;
  }
}

/* increase the number of columns in a floating point array */
void
arrayf_add_cols (array_f *arrp, gint nc)
{
  int i;

  if (nc > arrp->ncols) {
    for (i=0; i<arrp->nrows; i++)
      arrp->data[i] = (gfloat *) g_realloc (arrp->data[i],
                                            nc * sizeof (gfloat));
    arrp->ncols = nc;
  }
}

void
arrayf_copy (array_f *arrp_from, array_f *arrp_to)
{
  gint i, j;

  if (arrp_from->ncols == arrp_to->ncols &&
      arrp_from->nrows == arrp_to->nrows)
  {
    for (i=0; i<arrp_from->nrows; i++)
      for (j=0; j<arrp_from->ncols; j++)
        arrp_to->data[i][j] = arrp_from->data[i][j];
  }
}

/*-------------------------------------------------------------------------*/
/*                      short integer array management                     */
/*-------------------------------------------------------------------------*/

void
arrays_init (array_s *arrp)
{
  arrp->nrows = arrp->ncols = 0;
  arrp->data = (gshort **) NULL;
}

void
arrays_free (array_s *arrp, gint nr, gint nc)
{
  gint i;

  /*-- if nr != 0, free only the last nrows-nr rows --*/

  for (i=nr; i<arrp->nrows; i++)
    g_free (arrp->data[i]);

  if (nr == 0) {
    g_free (arrp->data);
    arrp->data = (gshort **) NULL;
  }

  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* Zero a short array. */
void
arrays_zero (array_s *arrp)
{
  int i, j;
  for (i=0; i<arrp->nrows; i++) {
    for (j=0; j<arrp->ncols; j++) {
      arrp->data[i][j] = 0.0;
    }
  }
}

/* allocate a short integer array */
void
arrays_alloc (array_s *arrp, gint nr, gint nc)
{
  int i;

  /*-- if the dimensions match, then the allocated size is surely ok --*/
  if (arrp->nrows != nr || arrp->ncols != nc) {
    
    if (arrp->nrows != 0 || arrp->ncols != 0)
      arrays_free (arrp, 0, 0);

    arrp->data = (gshort **) g_malloc (nr * sizeof (gshort *));
    for (i = 0; i < nr; i++)
      arrp->data[i] = (gshort *) g_malloc (nc * sizeof (gshort));
    arrp->nrows = nr;
    arrp->ncols = nc;
  }
}

/* allocate a short integer array populated with 0 */
void
arrays_alloc_zero (array_s *arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0)||(arrp->ncols != 0))
    arrays_free (arrp, 0, 0);

  arrp->data = (gshort **) g_malloc (nr * sizeof (gshort *));
  for (i = 0; i < nr; i++)
    arrp->data[i] = (gshort *) g_malloc0 (nc * sizeof (gshort));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* increase the number of rows in a short integer array */
void
arrays_add_rows (array_s *arrp, gint nr)
{
  int i;

  if (nr > arrp->nrows) {

    arrp->data = (gshort **) g_realloc (arrp->data, nr * sizeof (gshort *));
    for (i = arrp->nrows; i < nr; i++)
      arrp->data[i] = (gshort *) g_malloc (arrp->ncols * sizeof (gshort));

    arrp->nrows = nr;
  }
}

/* increase the number of columns in a short integer array */
void
arrays_add_cols (array_s *arrp, gint nc)
{
  int i;

  if (nc > arrp->ncols) {
    for (i=0; i<arrp->nrows; i++)
      arrp->data[i] = (gshort *) g_realloc (arrp->data[i],
                                            nc * sizeof (gshort));
    arrp->ncols = nc;
  }
}

/*-------------------------------------------------------------------------*/
/*                      long array management                              */
/*-------------------------------------------------------------------------*/

void
arrayl_init (array_l *arrp)
{
  arrp->nrows = arrp->ncols = 0;
  arrp->data = (glong **) NULL;
}

void
arrayl_free (array_l *arrp, gint nr, gint nc)
{
  gint i;

  /*-- if nr != 0, free only the last nrows-nr rows --*/

  for (i=nr; i<arrp->nrows; i++)
    g_free (arrp->data[i]);

  if (nr == 0) {
    g_free (arrp->data);
    arrp->data = (glong **) NULL;
  }

  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* Zero a long array. */
void
arrayl_zero (array_l *arrp)
{
  int i, j;
  for (i=0; i<arrp->nrows; i++) {
    for (j=0; j<arrp->ncols; j++) {
      arrp->data[i][j] = 0.0;
    }
  }
}

/* allocate a long array */
void
arrayl_alloc (array_l *arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0)||(arrp->ncols != 0))
    arrayl_free (arrp, 0, 0);

  arrp->data = (glong **) g_malloc (nr * sizeof (glong *));
  for (i = 0; i < nr; i++)
    arrp->data[i] = (glong *) g_malloc (nc * sizeof (glong));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* allocate a long array populated with 0 */
void
arrayl_alloc_zero (array_l *arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0)||(arrp->ncols != 0))
    arrayl_free (arrp, 0, 0);

  arrp->data = (glong **) g_malloc (nr * sizeof (glong *));
  for (i = 0; i < nr; i++)
    arrp->data[i] = (glong *) g_malloc0 (nc * sizeof (glong));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* increase the number of rows in a long integer array */
void
arrayl_add_rows (array_l *arrp, gint nr)
{
  int i;

  if (nr > arrp->nrows) {

    arrp->data = (glong **) g_realloc (arrp->data, nr * sizeof (glong *));
    for (i = arrp->nrows; i < nr; i++)
      arrp->data[i] = (glong *) g_malloc (arrp->ncols * sizeof (glong));

    arrp->nrows = nr;
  }
}

/* increase the number of columns in a long array */
void
arrayl_add_cols (array_l *arrp, gint nc)
{
  int i;

  if (nc > arrp->ncols) {
    for (i=0; i<arrp->nrows; i++)
      arrp->data[i] = (glong *)
        g_realloc (arrp->data[i], nc * sizeof (glong));
    arrp->ncols = nc;
  }
}
