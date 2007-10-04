/* tour2d_pp.c */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke and Eun-Kuung Lee

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be contacted at the following email addresses:
    dicook@iastate.edu    sigbert@wiwi.hu-berlin.de
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vars.h"
#include "externs.h"

#include "tour2d_pp.h"
#include "tour_pp_ui.h"




void t2d_pptemp_set(gdouble slidepos, displayd *dsp, GGobiSession *gg) {
  dsp->t2d_pp_op.temp_start = slidepos;
}

void t2d_ppcool_set(gdouble slidepos, displayd *dsp, GGobiSession *gg) {
  dsp->t2d_pp_op.cooling = slidepos;
}


/* This function interacts with control  buttons in ggobi */
void t2d_optimz(gint optimz_on, gboolean *nt, gint *bm, displayd *dsp) {
  gboolean new_target = *nt;
  gint bas_meth = *bm;
  gint i, j;

  if (optimz_on) 
  {
    for (i=0; i<2; i++)
      for (j=0; j<dsp->t2d.nactive; j++)
        dsp->t2d_pp_op.proj_best.vals[i][j] = 
          dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];
    /*    dsp->t2d.ppval = dsp->t2d_indx_min;*/
    dsp->t2d_pp_op.index_best = dsp->t2d.ppval;
    bas_meth = 1;
  }
  else
    bas_meth = 0;

  new_target = true;

  *nt = new_target;
  *bm = bas_meth;
}

void t2d_clear_pppixmap(displayd *dsp, GGobiSession *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (dsp->t2d_pp_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_line (dsp->t2d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (dsp->t2d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);
}

void t2d_clear_ppda(displayd *dsp, GGobiSession *gg)
{
  gint i;

  /* clear the ppindx matrix */
  dsp->t2d_ppindx_count = 0;
  dsp->t2d_indx_min=1000.;
  dsp->t2d_indx_max=-1000.;
  for (i=0; i<100; i++) 
  {
    dsp->t2d_ppindx_mat[i] = 0.0;
  }

  t2d_clear_pppixmap(dsp, gg);
}

void t2d_ppdraw_all(gint wid, gint hgt, gint margin, displayd *dsp, GGobiSession *gg)
{
  GdkPoint pptrace[100];
  gint i;

  t2d_clear_pppixmap(dsp, gg);

  for (i=0; i<dsp->t2d_ppindx_count; i++) 
  {
    pptrace[i].x = margin+i*2;
    pptrace[i].y = hgt-margin-(gint)((gdouble)((dsp->t2d_ppindx_mat[i]-
      dsp->t2d_indx_min)/(gdouble) (dsp->t2d_indx_max-dsp->t2d_indx_min)) * 
      (gdouble) (hgt - 2*margin));
  }
  gdk_draw_lines (dsp->t2d_pp_pixmap, gg->plot_GC,
    pptrace, dsp->t2d_ppindx_count);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);

}

/* This is writes text to the pp window to inform the
user that optimize is finding a new maximum */ 
void t2d_ppdraw_think(displayd *dsp, GGobiSession *gg)
{
  splotd *sp = (splotd *) g_list_nth_data (dsp->splots, 0);
  colorschemed *scheme = gg->activeColorScheme;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;
  PangoLayout *layout = gtk_widget_create_pango_layout(sp->da, "Thinking...");
  
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_layout(dsp->t2d_pp_pixmap, gg->plot_GC, 10, 10, layout);
  g_object_unref(G_OBJECT(layout));
  /*gdk_text_extents (
    gtk_style_get_font (style),
    varlab, strlen (varlab),
    &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (dsp->t2d_pp_pixmap,
    gtk_style_get_font (style),
     gg->plot_GC, 10, 10, varlab);*/
  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);
}

/* This is the pp index plot drawing routine */ 
void t2d_ppdraw(gdouble pp_indx_val, displayd *dsp, GGobiSession *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;
  gint j;
  static gboolean init = true;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  if (init) {
    t2d_clear_ppda(dsp, gg);
    init = false;
  }

  dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count] = pp_indx_val;

  if (dsp->t2d_indx_min > pp_indx_val)
      dsp->t2d_indx_min = pp_indx_val;
  if (dsp->t2d_indx_max < pp_indx_val)
    dsp->t2d_indx_max = pp_indx_val;

  if (dsp->t2d_indx_min == dsp->t2d_indx_max) dsp->t2d_indx_min *= 0.9999;

  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
    dsp->t2d_indx_min, dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count], dsp->t2d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  if (dsp->t2d_ppindx_count == 0) 
  {
    dsp->t2d_ppindx_count++;
  }
  else if (dsp->t2d_ppindx_count > 0 && dsp->t2d_ppindx_count < 80) {
    t2d_ppdraw_all(wid, hgt, margin, dsp, gg);
    dsp->t2d_ppindx_count++;
  }
  else if (dsp->t2d_ppindx_count >= 80) 
  {
    /* cycle values back into array */
    for (j=0; j<=dsp->t2d_ppindx_count; j++)
      dsp->t2d_ppindx_mat[j] = dsp->t2d_ppindx_mat[j+1];
    t2d_ppdraw_all(wid, hgt, margin, dsp, gg);
  }

  g_free (label);
}

void t2d_pp_reinit(displayd *dsp, GGobiSession *gg)
{
  gint i, j;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  for (i=0; i<dsp->t2d_pp_op.proj_best.nrows; i++)
    for (j=0; j<dsp->t2d_pp_op.proj_best.ncols; j++)
      dsp->t2d_pp_op.proj_best.vals[i][j] = 
	  dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];
  dsp->t2d.ppval = 0.0;
  dsp->t2d.oppval = -1.0;
  dsp->t2d_pp_op.index_best = 0.0;
  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
			   dsp->t2d_indx_min, dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count], 
			   dsp->t2d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  t2d_clear_ppda(dsp, gg);
  g_free (label);
}


/* 
 We could call this with the tour t2d object rather than the index type
 since we no longer need the indxtype 
*/
gboolean
t2d_switch_index(Tour2DCPanel controls, gint basismeth, displayd *dsp,
  GGobiSession *gg)
{
  GGobiStage *d = dsp->d;
  gint nrows = d->n_rows;
  gint i, j, k;

  if (d->n_rows == 1)  /* can't do pp on no data! */
    return(false);

  /* Copy data into pp opt'n data */
  for (i=0; i<d->n_rows; i++)
    for (j=0; j<dsp->t2d.nactive; j++)
      dsp->t2d_pp_op.data.vals[i][j] = 
        ggobi_stage_get_raw_value(d, i, dsp->t2d.active_vars.els[j]);

  /* Copy current projection into opt'n projection */
  for (i=0; i<2; i++)
    for (j=0; j<dsp->t2d.nactive; j++) 
      dsp->t2d_pp_op.proj_best.vals[i][j] = 
        dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];

  for (k=0; k<2; k++) 
    for (i=0; i<d->n_rows; i++) {
      dsp->t2d_pp_op.pdata.vals[i][k] = 
          (ggobi_stage_get_raw_value(d, i, dsp->t2d.active_vars.els[0])*
          dsp->t2d_pp_op.proj_best.vals[k][0]);
      for (j=1; j<dsp->t2d.nactive; j++)
        dsp->t2d_pp_op.pdata.vals[i][k] += 
          (ggobi_stage_get_raw_value(d, i, dsp->t2d.active_vars.els[j])*
          dsp->t2d_pp_op.proj_best.vals[k][j]);
    }

  GGOBI_STAGE_ATTR_INIT(d, cluster);
  vector_d groups;
  vectord_init_null(&groups);
  vectord_alloc_zero(&groups, nrows);
  for (i=0; i<nrows; i++) { 
    groups.els[i] = GGOBI_STAGE_GET_ATTR_CLUSTER(d, i);
  }

  if(controls.ppindex.index_f) {
    dsp->t2d.ppval = controls.ppindex.index_f(dsp->t2d_pp_op.pdata, groups);
    if(basismeth == 1) 
      dsp->t2d.ppval = optimize0 (&dsp->t2d_pp_op, controls.ppindex.index_f, groups);
  }
  g_debug("PPI: %f", dsp->t2d.ppval);
  
  vectord_free(&groups);
  return(true);
}
