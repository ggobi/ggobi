/* brush_init.c */
/*
 * Allocation and initialization routines for brushing.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* external variables */

/*-------------------------------------------------------------------------*/
/*                      glyphs                                             */
/*-------------------------------------------------------------------------*/

void
br_glyph_ids_free ()
{
  g_free (gg.glyph_ids);
  g_free (gg.glyph_now);
  g_free (gg.glyph_prev);
}

void
br_glyph_ids_alloc ()
{
  gg.glyph_ids = (glyphv *) g_realloc (gg.glyph_ids,
                                       gg.nrows * sizeof (glyphv));
  gg.glyph_now = (glyphv *) g_realloc (gg.glyph_now,
                                       gg.nrows * sizeof (glyphv));
  gg.glyph_prev = (glyphv *) g_realloc (gg.glyph_prev,
                                       gg.nrows * sizeof (glyphv));
}

void
br_glyph_ids_init ()
{
  gint j;

  for (j=0; j<gg.nrows; j++) {
    gg.glyph_ids[j].type = gg.glyph_now[j].type =
      gg.glyph_prev[j].type = gg.glyph_0.type;
    gg.glyph_ids[j].size = gg.glyph_now[j].size =
      gg.glyph_prev[j].size = gg.glyph_0.size;
  }
}

/*-------------------------------------------------------------------------*/
/*                       color                                             */
/*-------------------------------------------------------------------------*/

void
br_color_ids_free ()
{
  g_free (gg.color_ids);
  g_free (gg.color_now);
  g_free (gg.color_prev);
}

void
br_color_ids_alloc ()
{
  gint i;

  gg.color_ids = (gushort *)  g_realloc (gg.color_ids,
                                         gg.nrows * sizeof (gushort));
  gg.color_now = (gushort *)  g_realloc (gg.color_now,
                                         gg.nrows * sizeof (gushort));
  gg.color_prev = (gushort *) g_realloc (gg.color_prev,
                                         gg.nrows * sizeof (gushort));
  for (i=0; i<gg.nrows; i++)
    gg.color_ids[i] = gg.color_now[i] = gg.color_prev[i] = gg.color_0;
}

void
br_color_ids_init ()
{
  gint i;

  gg.color_id = gg.color_0;
  for (i=0; i<gg.nrows; i++)
    gg.color_ids[i] = gg.color_now[i] = gg.color_prev[i] = gg.color_0;
}

/*-------------------------------------------------------------------------*/
/*                             erasing                                     */
/*-------------------------------------------------------------------------*/

void
hidden_alloc (void)
{
  if (gg.hidden != NULL) g_free (gg.hidden);
  if (gg.hidden_now != NULL) g_free (gg.hidden_now);
  if (gg.hidden_prev != NULL) g_free (gg.hidden_prev);

  gg.hidden = (gboolean *) g_malloc (gg.nrows * sizeof (gboolean));
  gg.hidden_now = (gboolean *) g_malloc (gg.nrows * sizeof (gboolean));
  gg.hidden_prev = (gboolean *) g_malloc (gg.nrows * sizeof (gboolean));
}
void
hidden_init (void)
{
  gint i;

  for (i=0; i<gg.nrows; i++)
    gg.hidden[i] = gg.hidden_now[i] = gg.hidden_prev[i] = false;
}


/*-------------------------------------------------------------------------*/
/*                           line color                                    */
/*-------------------------------------------------------------------------*/


void
br_line_color_ids_alloc ()
{
  gint ns = gg.nsegments; 

  gg.line_color_ids = (gushort *) g_realloc ((gpointer) gg.line_color_ids,
    ns * sizeof (gushort));
  gg.line_color_now = (gushort *) g_realloc ((gpointer) gg.line_color_now,
    ns * sizeof (gushort));
  gg.line_color_prev = (gushort *) g_realloc ((gpointer) gg.line_color_prev,
    ns * sizeof (gushort));
  gg.xed_by_new_brush = (gushort *) g_realloc ((gpointer) gg.xed_by_new_brush,
    ns * sizeof (gushort));
}

void
br_line_color_ids_free ()
{
  g_free ((gpointer) gg.line_color_ids);
  g_free ((gpointer) gg.line_color_now);
  g_free ((gpointer) gg.line_color_prev);
  g_free ((gpointer) gg.xed_by_new_brush);
}

void
br_line_color_ids_init ()
{
  gint j;

  for (j=0; j<gg.nsegments; j++) {
    gg.line_color_ids[j] = gg.line_color_now[j] = gg.line_color_prev[j] =
      gg.color_0;
  }
}

/*-------------------------------------------------------------------------*/
/*                           the brush itself                              */
/*-------------------------------------------------------------------------*/

void
brush_pos_init ()
{
  gg.app.brush_pos.x1 = gg.app.brush_pos.y1 = 20;
  gg.app.brush_pos.x2 = gg.app.brush_pos.y2 = 40;
}

/*----------------------------------------------------------------------*/
/*                          general                                     */
/*----------------------------------------------------------------------*/

void
brush_alloc ()
/*
 * Dynamically allocate arrays.
*/
{
  guint nr = (guint) gg.nrows;
  gint i, iv, ih;
  gboolean initd = false;

  gg.br_nbins = BRUSH_NBINS;

  gg.included = (gboolean *) g_realloc (gg.included, nr * sizeof (gboolean));
  gg.under_new_brush = (gboolean *) g_realloc (gg.under_new_brush,
                                               nr * sizeof (gboolean));

  for (i=0; i<nr; i++) {
    gg.included[i] = true;
    gg.under_new_brush[i] = false;
  }

  /*
   * color_ids and glyph_ids and their kin were allocated when
   * the data was read in.
  */

  if (!initd) {
    /* binning the plot window; no need to realloc these */
    gg.br_binarray = (bin_struct **)
      g_malloc (gg.br_nbins * sizeof (bin_struct *));
    for (ih=0; ih<gg.br_nbins; ih++) {
      gg.br_binarray[ih] = (bin_struct *)
        g_malloc (gg.br_nbins * sizeof (bin_struct));

      for (iv=0; iv<gg.br_nbins; iv++) {
        gg.br_binarray[ih][iv].nels = 0;
        gg.br_binarray[ih][iv].nblocks = 1;
        gg.br_binarray[ih][iv].els = (gulong *)
          g_malloc (BINBLOCKSIZE * sizeof (gulong));
      }
    }
    initd = true;
  }
}

void
brush_free ()
/*
 * Dynamically free arrays.
*/
{
  int j,k;

  br_glyph_ids_free ();
  br_color_ids_free ();

  g_free ((gpointer) gg.under_new_brush);

  for (k=0; k<gg.br_nbins; k++) {
    for (j=0; j<gg.br_nbins; j++)
      g_free ((gpointer) gg.br_binarray[k][j].els);
    g_free ((gpointer) gg.br_binarray[k]);
  }
  g_free ((gpointer) gg.br_binarray);
}

void
brush_init ()
{
  static gboolean firsttime = true;

  gg.glyph_id.type = gg.glyph_0.type = FILLED_CIRCLE;
  gg.glyph_id.size = gg.glyph_0.size = 3;
  gg.color_id = gg.color_0 = 0;

  if (firsttime) {
    brush_pos_init ();

    /*
     * Used in binning the plot window
    */
    gg.br_nbins = BRUSH_NBINS;

    /*
     * These are initialized so that the first merge_brushbins()
     * call will behave reasonably.
    */
    gg.bin0.x = gg.bin1.x = BRUSH_NBINS;
    gg.bin0.y = gg.bin1.y = BRUSH_NBINS;

    brush_alloc ();

    firsttime = false;

  } else {
  }
}

void
brush_activate (gboolean state)
{
  if (state)
    assign_points_to_bins ();
}
