#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "ggobiClass.h"

#include <stdio.h>

#include "plugin.h"
#include "dspdesc.h"
#include "macros.h"

#define MAX_PER_ROW 100

/*
 Jittering:  d->jitdata is added directly to world data, so the
   appropriate values of jitdata should be divided by PRECISION1
   and then added to tform where we're using that; where we're using
   planar, we're ok, because planar already includes jittering.
 Shift and scale:  see splot_plane_to_screen in splot.c
   Take care of shifting by describing only those points that
     are within the viewing area.
   Scaling:  sp->scale.{x,y}
      screen=(planar - pmid) * (max*(scale/2)) / PRECISION1
      screen += max/2
*/

/* In sp_plot_edges.c */
gboolean splot_plot_edge (gint m, GGobiData *d, GGobiData *e,
		 splotd *sp, displayd *display, ggobid *gg);
gboolean splot_hidden_edge (gint m, GGobiData *d, GGobiData *e,
		   splotd *sp, displayd *display, ggobid *gg);


/* c(color.red, color.green, color.blue) */
void
describe_color (FILE *fp, GdkColor color)
{
  fprintf (fp, "%.3f", color.red / 65535.0);
  ADD_COMMA(fp);
  fprintf (fp, "%.3f", color.green / 65535.0);
  ADD_COMMA(fp);
  fprintf (fp, "%.3f", color.blue / 65535.0);
}

/*
colormap=list (
 name="",
 ncolors=,
 type="",
 system="rgb", 
 backgroundColor=c(r,g,b),
 hiddenColor=c(r,g,b),
 accentColor=c(r,g,b),
 foregroundColors=list ( c(r,g,b), ...)
),
*/
void
describe_colorscheme (FILE *fp, ggobid *gg)
{
  gint i;
  colorschemed *scheme = gg->activeColorScheme;
 
  OPEN_NAMED_LIST(fp, "colormap");

  fprintf (fp, "name='%s',\n", scheme->name);  
  fprintf (fp, "ncolors=%d,\n", scheme->n);
  fprintf (fp, "type=%d,\n", scheme->type);
  /* the only scheme we currently support */
  fprintf (fp, "system='rgb',\n");

  OPEN_NAMED_C(fp, "backgroundColor");
  describe_color (fp, scheme->rgb_bg);
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  OPEN_NAMED_C (fp, "hiddenColor");
  describe_color (fp, scheme->rgb_hidden);
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  OPEN_NAMED_C (fp, "accentColor");
  describe_color (fp, scheme->rgb_accent);
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  OPEN_NAMED_LIST(fp, "foregroundColors");
  for (i=0; i<scheme->n; i++) {
    OPEN_C(fp);  /* one foreground color */
    describe_color (fp, scheme->rgb[i]);
    CLOSE_C(fp);  /* one foreground color */
    if (i < scheme->n-1)
      ADD_COMMA(fp);
  }
  CLOSE_LIST(fp); /* foregroundColors */

  CLOSE_LIST(fp); /* colormap */
  ADD_COMMA(fp);
  ADD_CR(fp);
}


/*
stickylabels = list(list(index=, label=""),
                    list(index=, label=""), ...)),
*/
void
describe_sticky_labels (FILE *fp, GGobiData *d, cpaneld *cpanel)
{
  gint j;

  if (d->sticky_ids && g_slist_length (d->sticky_ids) > 0) {
    GSList *l;
    ADD_COMMA(fp);
    OPEN_NAMED_LIST(fp, "stickylabels");
    for (l = d->sticky_ids; l; l = l->next) {
      OPEN_LIST(fp);  /* one sticky label */
      j = GPOINTER_TO_INT (l->data);
      fprintf (fp, "index=%d", j);
      ADD_COMMA(fp);
      fprintf (fp, "label=");

      if (cpanel->id_display_type == ID_RECORD_LABEL)
        fprintf (fp, "%s", (gchar *) g_array_index (d->rowlab, gchar *, j));
      else if (cpanel->id_display_type == ID_RECORD_NO) {
        fprintf (fp, "%d", j);
      }  else if (cpanel->id_display_type == ID_RECORD_ID) {
        if (d->rowIds && d->rowIds[j]) {
          fprintf (fp, "%s", d->rowIds[j]);
        } else {
          fprintf (fp, "%s", "");
        }
      }
      CLOSE_LIST(fp);  /* one sticky label */
      if (l->next)
        ADD_COMMA(fp); 
    }
    CLOSE_LIST(fp);  /* stickylabels */
  }
}

/* 
  iscr.x -= (sp->max.x / 2);
  gtmp =  (iscr.x * PRECISION1) / (sp->iscale.x);
  planarx = gtmp + sp->pmid.x
*/
static greal
scale_convert (splotd *sp, gint ival, gint max, greal mid, gint scale)
{
  gint itmp;

  ival -= (max / 2);
  itmp = (ival * PRECISION1) / (scale);
  return ((greal) itmp + mid);
}

/*
    list(
      projection = "",
      scale = c(x= , y= ),  # a float between 0 and 1
      # limits are derived by converting the screen size into tform
      # coords and also into planar coordinates
      # Update: these limits are not used in the R scripts
      # that generate the plots, but we'll leave them in for
      # a while just in case.  dfs, March 2006
      tformLims = c(xmin=, xmax=, ymin=, ymax=),
      planarLims = c(xmin=, xmax=, ymin=, ymax=),
      points = list(
        # x and y are tform for 2d plots, planar for tours;
        # for 1dplots, one is the data (tform) and the other is
        #     the 'spreadData'
        x = c(), y = c(), 
        hidden = c(), missing = c(),
        color = c(), glyphtype = c(), glyphsize = c(), 
      ),
      edges = list(
        # src and dest are integers which index into points${x,y}
        src = c(), dest = c(), 
        hidden = c(), missing = c(),
        color = c(), linetype = c(), linewidth = c(),
      ),
      stickylabels = list(list(index=, label=""),
                          list(index=, label=""), ...)),
      # the parameters corresponding to the current projection
      params = list(label="", orientation=""),
      params = list(xlabel="", ylabel="",),
      params = list(F, labels, ranges),
      params = list(F, labels, ranges),
      params = list(F, labels, ranges),
      dparams = list(xF, xlabels, xranges, yF, ylabels, yranges),
*/
void
describe_scatterplot_plot (FILE *fp, ggobid *gg, displayd *display,
   splotd *sp, dspdescd *desc, ProjectionMode projection)
{
  vartabled *vt; 
  cpaneld *cpanel = &display->cpanel;
  gint i, j, m;
  gint k = -1;
  const gchar *const *gnames = GGOBI(getPModeNames)(&k);
  GGobiData *d = display->d;
  GGobiData *e = display->e;
  /*gboolean missing;*/
  icoords scr;
  fcoords tfmin, tfmax;
  fcoords pmin, pmax;
  float ftmp;
  gint counter;

  OPEN_LIST(fp);  /* plot; unlabelled */

  /* A scatterplot is both a display type and a plot type */
  fprintf (fp, "type='scatterplot'"); ADD_COMMA(fp);
  fprintf (fp, "projection='%s'", gnames[(gint) projection]); ADD_COMMA(fp);

  /*-- scale parameters ------------------------------------------*/

  fprintf (fp, "scale=c(%.3f, %.3f)", sp->scale.x, sp->scale.y);
  ADD_COMMA(fp); ADD_CR(fp);

  /* Find the tform values of the screen min and max */
  tfmin.x = tfmin.y = tfmax.x = tfmax.y = 0.0;

  scr.x = scr.y = 0;
  /*splot_screen_to_tform (cpanel, sp, &scr, &tfmin, gg);*/
  if(sp && GGOBI_IS_EXTENDED_SPLOT(sp)) {
     GGobiExtendedSPlotClass *klass;
     klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
     if(klass->screen_to_tform)
       klass->screen_to_tform(cpanel, sp, &scr, &tfmin, gg);
     else
       g_printerr ("screen_to_tform routine needed\n");
  }

  scr.x = sp->max.x;
  scr.y = sp->max.y;
  /*splot_screen_to_tform (cpanel, sp, &scr, &tfmax, gg);*/
  if(sp && GGOBI_IS_EXTENDED_SPLOT(sp)) {
     GGobiExtendedSPlotClass *klass;
     klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
     if(klass->screen_to_tform)
       klass->screen_to_tform(cpanel, sp, &scr, &tfmax, gg);
     else
       g_printerr ("screen_to_tform routine needed\n");
  }

  // Swap ymin and ymax
  ftmp = tfmin.y;
  tfmin.y = tfmax.y;
  tfmax.y = ftmp;

  /* Update: none of these limits are now used in the R scripts that
   * generate the plots, but we'll leave them in for a while just in
   * case.  dfs, March 2006
   */
  fprintf (fp,
    "tformLims=c(xmin=%.3f, xmax=%.3f, ymin=%.3f, ymax=%.3f),",
    tfmin.x, tfmax.x, tfmin.y, tfmax.y);
  ADD_CR(fp);

   /* Now how do I find the planar coordinates for the same points? */
   /*   For those four values of iscr.{x,y},  do this:
    iscr.x -= (sp->max.x / 2);
    gtmp = (iscr.x * PRECISION1) / (sp->iscale.x);
    planarx = gtmp + sp->pmid.x
    */

  // Swap y min and max up front.
  pmin.x = scale_convert (sp, 0, sp->max.x, sp->pmid.x, sp->iscale.x);
  pmax.y = scale_convert (sp, 0, sp->max.y, sp->pmid.y, sp->iscale.y);
  pmax.x = scale_convert (sp, sp->max.x, sp->max.x, sp->pmid.x, sp->iscale.x);
  pmin.y = scale_convert (sp, sp->max.y, sp->max.y, sp->pmid.x, sp->iscale.y);

  fprintf (fp,
    "planarLims=c(xmin=%.3f, xmax=%.3f, ymin=%.3f, ymax=%.3f),",
    pmin.x, pmax.x, pmin.y, pmax.y);
  ADD_CR(fp);

  /*-- row-wise data ----------------------------------------*/

  OPEN_NAMED_LIST(fp, "points");

  /* raw row number -- the edges use these  */
  OPEN_NAMED_C(fp, "index");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
    fprintf (fp, "%d", i);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* x coordinates */
  OPEN_NAMED_C(fp, "x");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    if (projection == P1PLOT) {
      fprintf (fp, "%g", d->tform.vals[i][sp->p1dvar]);
    } else if (projection == XYPLOT) {
      fprintf (fp, "%g", d->tform.vals[i][sp->xyvars.x]);
      /*  Is this how to add jittering to tform?
      fprintf (fp, "%g", d->tform.vals[i][sp->xyvars.x] +
	       d->jitdata.vals[i][sp->xyvars.x] / precis);
      */
    } else {  /* planar already includes jitdata! */
      fprintf (fp, "%g", sp->planar[i].x);
    }
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* y coordinates */
  OPEN_NAMED_C(fp, "y");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    if (projection == P1PLOT) {
      /* I <think> spread_data is in tform coordinates, but it's hard
         to tell.  And maybe it doesn't matter because the spread axis
         isn't going to be labelled.
      */
      fprintf (fp, "%g", sp->p1d.spread_data.els[m]);
    } else if (projection == XYPLOT) {
      fprintf (fp, "%g", d->tform.vals[i][sp->xyvars.y]);
    } else {
      fprintf (fp, "%g", sp->planar[i].y);
    }
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* color */
  OPEN_NAMED_C(fp, "color");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%d", d->color_now.els[i]);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* glyphtype */
  OPEN_NAMED_C(fp, "glyphtype");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%d", d->glyph_now.els[i].type);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* glyphsize */
  OPEN_NAMED_C(fp, "glyphsize");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%d", d->glyph_now.els[i].size);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* hiddenness */
  OPEN_NAMED_C(fp, "hidden");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%d", d->hidden_now.els[i]);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); /*ADD_COMMA(fp);*/ ADD_CR(fp);

  /* missingness */
  /*
  if (d->nmissing > 0) {
    OPEN_NAMED_C(fp, "missing");
    for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
      missing = false;
      if(GGOBI_IS_EXTENDED_SPLOT(sp)) {
        GGobiExtendedSPlotClass *klass;
        klass = GGOBI_EXTENDED_SPLOT_GET_CLASS(sp);
        if (klass->draw_case_p)
          missing = klass->draw_case_p (sp, i, d, gg);
      }
      fprintf (fp, "%d,", missing);
    }
    CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);
  }
  */
  CLOSE_LIST(fp);  /* points */
  ADD_COMMA(fp);
  ADD_CR(fp);

  /*-- parameters for each projection ---------------------------------*/

  if (projection == P1PLOT) {
    OPEN_NAMED_LIST(fp, "params");
    vt = vartable_element_get (sp->p1dvar, d);
    fprintf (fp, "label='%s',", vt->collab_tform);
    /* type: TEXTURE=0, ASH=1 */
    fprintf (fp, "type=%d,", cpanel->p1d.type);
    /* plot orientation: HORIZONTAL=0, VERTICAL=1 */
    fprintf (fp, "orientation=%d,", display->p1d_orientation);
    /* whether we're drawing line segments */
    fprintf (fp, "lines=%d", cpanel->p1d.ASH_add_lines_p);
    CLOSE_LIST(fp); /* p1plot params */
  } else if (projection == XYPLOT) {
    OPEN_NAMED_LIST(fp, "params");
    vt = vartable_element_get (sp->xyvars.x, d);
    fprintf (fp, "xlabel='%s',", vt->collab_tform);
    vt = vartable_element_get (sp->xyvars.y, d);
    fprintf (fp, "ylabel='%s'", vt->collab_tform);
    CLOSE_LIST(fp);  /* xyplot params */
  } else if (projection == TOUR1D) {
    /* F, variable labels, variable lims */
    OPEN_NAMED_LIST(fp, "params");
    /* F */
    OPEN_NAMED_C(fp, "F");
    for (k=0; k<display->t1d.nsubset; k++) {
      j = display->t1d.subset_vars.els[k];
      fprintf (fp, "%.3f", display->t1d.F.vals[0][j]);
      if (k < display->t1d.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* variable labels */
    OPEN_NAMED_C(fp, "labels");
    for (k=0; k<display->t1d.nsubset; k++) {
      j = display->t1d.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      fprintf (fp, "'%s'", vt->collab_tform);
      if (k < display->t1d.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* variable ranges */
    OPEN_NAMED_LIST(fp, "ranges");
    for (k=0; k<display->t1d.nsubset; k++) {
      j = display->t1d.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      OPEN_C(fp);
      fprintf (fp, "%.3f, %.3f", vt->lim.min, vt->lim.max);
      CLOSE_C(fp);
      if (k < display->t1d.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_LIST(fp);  /* tour1d ranges */
    CLOSE_LIST(fp);  /* tour1d params */
  } else if (projection == TOUR2D3) {
    /* F, variable labels, variable lims */
    OPEN_NAMED_LIST(fp, "params");
    /* F */
    OPEN_NAMED_C(fp, "F");
    for (k=0; k<display->t2d3.nsubset; k++) {
      j = display->t2d3.subset_vars.els[k];
      fprintf (fp, "%.3f", display->t2d3.F.vals[0][j]);
      if (k < display->t2d3.nsubset-1)
        ADD_COMMA(fp);
    }
    for (k=0; k<display->t2d3.nsubset; k++) {
      j = display->t2d3.subset_vars.els[k];
      fprintf (fp, "%.3f", display->t2d3.F.vals[1][j]);
      if (k < display->t2d3.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* variable labels */
    OPEN_NAMED_C(fp, "labels");
    for (k=0; k<display->t2d3.nsubset; k++) {
      j = display->t2d3.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      fprintf (fp, "'%s'", vt->collab_tform);
      if (k < display->t2d3.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* variable ranges */
    OPEN_NAMED_LIST(fp, "ranges");
    for (k=0; k<display->t2d3.nsubset; k++) {
      j = display->t2d3.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      OPEN_C(fp);
      fprintf (fp, "%.3f, %.3f", vt->lim.min, vt->lim.max);
      CLOSE_C(fp);
      if (k < display->t2d3.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_LIST(fp);  /* tour2d3 ranges */
    CLOSE_LIST(fp);  /* tour2d3 params */
  } else if (projection == TOUR2D) {
    /* F, variable labels, variable lims */
    OPEN_NAMED_LIST(fp, "params");
    /* F */
    OPEN_NAMED_C(fp, "F");
    for (k=0; k<display->t2d.nsubset; k++) {
      j = display->t2d.subset_vars.els[k];
      fprintf (fp, "%.3f", display->t2d.F.vals[0][j]);
      if (k < display->t2d.nsubset-1)
        ADD_COMMA(fp);
    }
    ADD_COMMA(fp);
    for (k=0; k<display->t2d.nsubset; k++) {
      j = display->t2d.subset_vars.els[k];
      fprintf (fp, "%.3f", display->t2d.F.vals[1][j]);
      if (k < display->t2d.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);  /* F */
    ADD_COMMA(fp);
    /* variable labels */
    OPEN_NAMED_C(fp, "labels");
    for (k=0; k<display->t2d.nsubset; k++) {
      j = display->t2d.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      fprintf (fp, "'%s'", vt->collab_tform);
      if (k < display->t2d.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp); /* labels */
    ADD_COMMA(fp);
    /* variable ranges */
    OPEN_NAMED_LIST(fp, "ranges");
    for (k=0; k<display->t2d.nsubset; k++) {
      j = display->t2d.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      OPEN_C(fp);
      fprintf (fp, "%.3f, %.3f", vt->lim.min, vt->lim.max);
      CLOSE_C(fp);
      if (k < display->t2d.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_LIST(fp);   /* tour2d ranges */
    CLOSE_LIST(fp);  /* tour2d params */
  } else if (projection == COTOUR) {
    /* xF, x variable labels, x variable lims; ditto for y */
    OPEN_NAMED_LIST(fp, "params");
    /* xF */
    OPEN_NAMED_C(fp, "xF");
    for (k=0; k<display->tcorr1.nsubset; k++) {
      j = display->tcorr1.subset_vars.els[k];
      fprintf (fp, "%.3f", display->tcorr1.F.vals[0][j]);
      if (k < display->tcorr1.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* variable labels */
    OPEN_NAMED_C(fp, "xlabels");
    for (k=0; k<display->tcorr1.nsubset; k++) {
      j = display->tcorr1.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      fprintf (fp, "'%s'", vt->collab_tform);
      if (k < display->tcorr1.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* x variable ranges */
    OPEN_NAMED_LIST(fp, "xranges");
    for (k=0; k<display->tcorr1.nsubset; k++) {
      j = display->tcorr1.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      OPEN_C(fp);
      fprintf (fp, "%.3f, %.3f", vt->lim.min, vt->lim.max);
      CLOSE_C(fp);
      if (k < display->tcorr1.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_LIST(fp);  /* tour2x1d, x ranges */

    ADD_COMMA(fp);  ADD_CR(fp);

    /* yF */
    OPEN_NAMED_C(fp, "yF");
    for (k=0; k<display->tcorr2.nsubset; k++) {
      j = display->tcorr2.subset_vars.els[k];
      fprintf (fp, "%.3f", display->tcorr2.F.vals[0][j]);
      if (k < display->tcorr2.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* y variable labels */
    OPEN_NAMED_C(fp, "ylabels");
    for (k=0; k<display->tcorr2.nsubset; k++) {
      j = display->tcorr2.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      fprintf (fp, "'%s'", vt->collab_tform);
      if (k < display->tcorr2.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_C(fp);
    ADD_COMMA(fp);
    /* y variable ranges */
    OPEN_NAMED_LIST(fp, "yranges");
    for (k=0; k<display->tcorr2.nsubset; k++) {
      j = display->tcorr2.subset_vars.els[k];
      vt = vartable_element_get (j, d);
      OPEN_C(fp);
      fprintf (fp, "%.3f, %.3f", vt->lim.min, vt->lim.max);
      CLOSE_C(fp);
      if (k < display->tcorr2.nsubset-1)
        ADD_COMMA(fp);
    }
    CLOSE_LIST(fp);  /* tour2x1d, y ranges */

    CLOSE_LIST(fp);  /* tour2x1d params */
  }
  /*  ADD_COMMA(fp); */ 

  /* sticky labels */
  describe_sticky_labels (fp, d, cpanel);

  /* edges: source, dest, color, glyph type and size, hidden,
     sticky labels (not adding missing at the moment)
  */
  if (d->idTable != NULL && e != (GGobiData *) NULL && e->edge.n > 0) {
    endpointsd *endpoints;
    gint a, b;

    endpoints = resolveEdgePoints(e, d);

    /* if edges are not displayed, no need to pass them along */
    if (endpoints &&
        (display->options.edges_undirected_show_p ||
         display->options.edges_arrowheads_show_p ||
	 display->options.edges_directed_show_p))
    {
      ADD_COMMA(fp);
      OPEN_NAMED_LIST(fp, "edges");

      for (i=0; i<e->edge.n; i++) {
        /*
         * This hides missings if missings_show_p is turned off;
         * I don't think we're handling missings the same way for
         * nodes and edges, even in ggobi.
         */
        if (!splot_plot_edge (i, d, e, sp, display, gg))
          continue;

        OPEN_LIST(fp);  /* one edge */

        /* source and destination */
        edge_endpoints_get (i, &a, &b, d, endpoints, e);
        fprintf (fp, "src=%d,", a);
        fprintf (fp, "dest=%d,", b);

        /* color, glyph type, glyph size */
        fprintf (fp, "color=%d,", e->color_now.els[i]);
        fprintf (fp, "ltype=%d,", e->glyph_now.els[i].type);
        fprintf (fp, "lwd=%d,", e->glyph_now.els[i].size);

        /* hiddenness */
        fprintf (fp, "hidden=%d",
          splot_hidden_edge (i, d, e, sp, display, gg));

        CLOSE_LIST(fp);  /* one edge */
        if (i < e->edge.n-1)
          ADD_COMMA(fp);
      }
      /* sticky labels */
      describe_sticky_labels (fp, e, cpanel);
    }
    CLOSE_LIST(fp);  /* edges */
    /*ADD_COMMA(fp); */ ADD_CR(fp);
  }

  CLOSE_LIST(fp); /* plot */
}

void
describe_scatterplot_display (FILE *fp, ggobid *gg, displayd *display, 
			      dspdescd *desc)
{
  splotd *sp = (splotd *) display->splots->data;

  fprintf (fp, "nplots=1");
  ADD_COMMA(fp); ADD_CR(fp);
  OPEN_NAMED_LIST(fp, "plots");

  describe_scatterplot_plot (fp, gg, display, sp, desc, 
    display->cpanel.pmode);

  CLOSE_LIST(fp);  /* plots */
}


void
describe_scatmat_display (FILE *fp, ggobid *gg, displayd *display, 
		      dspdescd *desc)
{
  GList *l;
  splotd *sp;
  gint ncols, *cols;
  ProjectionMode projection;
  GGobiData *d = display->d;
  gint i, j, nplotted_vars, *plotted_vars;
  GtkTableChild *child;
  GtkWidget *da;

  cols = (gint *) g_malloc(d->ncols * sizeof(gint));
  ncols = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display)->plotted_vars_get(display, cols, d, gg);

  fprintf (fp, "nplots=%d", ncols * ncols);
  ADD_COMMA(fp); ADD_CR(fp);
  OPEN_NAMED_LIST(fp, "plots");

  /* This output relies on the idea that we'll use 
     par(mfcol=, mfrow=) to lay out the plots in composite displays,
     rather than using splom or any other utility. */

  plotted_vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  nplotted_vars =
    GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->plotted_vars_get (display,
      plotted_vars, d, gg);

  for (i=0; i<nplotted_vars; i++) {
    for (j=0; j<nplotted_vars; j++) {

      for (l = (GTK_TABLE (display->table))->children; l; l = l->next) {
        child = (GtkTableChild *) l->data;
        if (child->top_attach==i && child->left_attach==j) {
          da = child->widget;
          sp = (splotd *) g_object_get_data (G_OBJECT (da), "splotd");
          projection = (sp->p1dvar != -1) ? P1PLOT : XYPLOT;
          describe_scatterplot_plot (fp, gg, display, sp, desc, projection);
          ADD_COMMA(fp);
          break;
        }
      }
    }
  }

  /*
  for (l = display->splots; l; l = l->next) {
    sp = (splotd *) l->data;
    projection = (sp->p1dvar != -1) ? P1PLOT : XYPLOT;
    describe_scatterplot_plot (fp, gg, display, sp, desc, projection);
    if (l->next)
      ADD_COMMA(fp);
  }
  */

  CLOSE_LIST(fp);  /* plots */
  g_free(cols);
}


void
describe_parcoords_display (FILE *fp, ggobid *gg, displayd *display, 
		      dspdescd *desc)
{
  GList *l;
  splotd *sp;
  gint ncols;

  ncols = g_list_length (display->splots);
  fprintf (fp, "nplots=%d", ncols);
  ADD_COMMA(fp); ADD_CR(fp);
  OPEN_NAMED_LIST(fp, "plots");

  /* We seem to be working through the plots row-wise, but I don't
  think that's something we can count on.  I hope we can make use of
  the axis labels to figure out which plot belongs where.  Otherwise,
  I'll have to add a position indicator to each plot. */

  for (l = display->splots; l; l = l->next) {
    sp = (splotd *) l->data;
    describe_scatterplot_plot (fp, gg, display, sp, desc, P1PLOT);
    ADD_COMMA(fp);
  }

  CLOSE_LIST(fp);  /* plots */
}


void
describe_time_series_display (FILE *fp, ggobid *gg, displayd *display, 
		      dspdescd *desc)
{
  GList *l;
  splotd *sp;
  gint ncols;

  ncols = g_list_length (display->splots);

  fprintf (fp, "nplots=%d", ncols);
  ADD_COMMA(fp); ADD_CR(fp);
  OPEN_NAMED_LIST(fp, "plots");

  for (l = display->splots; l; l = l->next) {
    sp = (splotd *) l->data;
    describe_scatterplot_plot (fp, gg, display, sp, desc, XYPLOT);
    if (l->next)
      ADD_COMMA(fp);
  }

  CLOSE_LIST(fp);  /* plots */
}

void
describe_barchart_plot (FILE *fp, ggobid *gg, displayd *display,
   splotd *sp, dspdescd *desc)
{
  GGobiData *d = display->d;
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);
  vartabled *vtx = vartable_element_get (sp->p1dvar, d);
  gint i, m, level, counter;

  OPEN_LIST(fp);  /* plot; unlabelled */
  if (vtx->vartype == categorical) {
    if (bsp->bar->is_spine)
      fprintf (fp, "type='spineplot'");
    else
      fprintf (fp, "type='barplot'");
  } else {
    fprintf (fp, "type='histogram'");
  }
  ADD_COMMA(fp);


  OPEN_NAMED_LIST(fp, "points");
  /* x coordinates */
  OPEN_NAMED_C(fp, "x");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%g", d->tform.vals[i][sp->p1dvar]);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* color */
  OPEN_NAMED_C(fp, "color");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%d", d->color_now.els[i]);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  /* hiddenness */
  OPEN_NAMED_C(fp, "hidden");
  for (m=0, counter=1; m<d->nrows_in_plot; m++, counter++) {
    i = d->rows_in_plot.els[m];
    fprintf (fp, "%d", d->hidden_now.els[i]);
    if (m < d->nrows_in_plot-1)
      ADD_COMMA(fp);
    if (counter % MAX_PER_ROW == 0) ADD_CR(fp);
  }
  CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

  CLOSE_LIST(fp);  /* points */
  ADD_COMMA(fp);
  ADD_CR(fp);

  /* parameters */
  OPEN_NAMED_LIST(fp, "params");
  /* Variable name */
  fprintf (fp, "label='%s',", vtx->collab_tform);


  if (vtx->vartype == categorical) {
    gchar *catname;

    /* level names */
    OPEN_NAMED_C(fp, "levelnames");
    for (i = 0; i < bsp->bar->nbins; i++) {
      level = checkLevelValue (vtx, (gdouble) bsp->bar->bins[i].value);
      catname = g_strdup_printf ("%s",
                                 (level ==
                                  -1) ? "missing" : vtx->level_names[level]);
      fprintf (fp, "'%s'", catname);
      if (i < bsp->bar->nbins-1) 
        ADD_COMMA(fp);
      if (i % MAX_PER_ROW == 0) ADD_CR(fp);
    }
    CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);

    /* level values */
    OPEN_NAMED_C(fp, "levelvalues");
    for (i = 0; i < bsp->bar->nbins; i++) {
      level = checkLevelValue (vtx, (gdouble) bsp->bar->bins[i].value);
      fprintf (fp, "%d", level);
      if (i < bsp->bar->nbins-1) 
        ADD_COMMA(fp);
      if (i % MAX_PER_ROW == 0) ADD_CR(fp);
    }
    CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);
  } else {
    /* breaks */
    OPEN_NAMED_C(fp, "breaks");
    for (i = 0; i < bsp->bar->nbins; i++) {
      if (i < bsp->bar->nbins-1) 
        ADD_COMMA(fp);
      fprintf (fp, "%.3f", bsp->bar->breaks[i]);
    }
    CLOSE_C(fp); ADD_COMMA(fp); ADD_CR(fp);
  }

  CLOSE_LIST(fp);  /* params */
  ADD_COMMA(fp);
  ADD_CR(fp);

  CLOSE_LIST(fp); /* plot */
}

void describe_barchart_display (FILE *fp, ggobid *gg, displayd *display, 
		      dspdescd *desc)
{
  splotd *sp = (splotd *) display->splots->data;

  fprintf (fp, "nplots=1");
  ADD_COMMA(fp); ADD_CR(fp);
  OPEN_NAMED_LIST(fp, "plots");

  describe_barchart_plot (fp, gg, display, sp, desc);

  CLOSE_LIST(fp);  /* plots */
}


void
desc_setup (dspdescd *desc)
{
  GtkWidget *entry;

  entry = (GtkWidget *)
    g_object_get_data(G_OBJECT(desc->window), "TITLE");
  if (desc->title) g_free(desc->title);
  desc->title = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
/*
  entry = (GtkWidget *)
    g_object_get_data(G_OBJECT(desc->window), "FILENAME");*/
  if (desc->filename) g_free(desc->filename);
  //desc->filename = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
  desc->filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(desc->window));
} 

void
desc_write (PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  dspdescd *desc = dspdescFromInst (inst);
  FILE *fp;
  displayd *display = gg->current_display;

  if (display == (displayd *) NULL) {
    quick_message ("There is no current display", false);
    return;
  }
  
  desc_setup (desc);

  if ((fp = fopen(desc->filename, "w")) == NULL) {
    gchar *message = g_strdup_printf ("'%s' can not be opened for writing",
				      desc->filename);
    quick_message (message, false);
    g_free (message);
    return;
  }

  /* Just handle a single display, the current one */
  OPEN_NAMED_LIST (fp, "display");

  /* describe the colorscheme here, once for all (potential) displays */
  describe_colorscheme (fp, gg);

  fprintf (fp, "title='%s',", desc->title);

  /* A display could report its own name, but I don't know if it can */
  if (GGOBI_IS_SCATTERPLOT_DISPLAY(display)) {
    fprintf (fp, "type='scatterplot',");
    describe_scatterplot_display (fp, gg, display, desc);
  } else if (GGOBI_IS_SCATMAT_DISPLAY(display)) {
    gint ncols, *cols;
    GGobiData *d = display->d;
    fprintf (fp, "type='scatmat',");
    /* ncols: display is symmetric */
    cols = (gint *) g_malloc(d->ncols * sizeof(gint));
    ncols = GGOBI_EXTENDED_DISPLAY_GET_CLASS(display)->plotted_vars_get(display, cols, d, gg);
    fprintf (fp, "ncols = %d,", ncols);
    g_free(cols);
    describe_scatmat_display (fp, gg, display, desc);
  } else if (GGOBI_IS_PAR_COORDS_DISPLAY(display)) {
    fprintf (fp, "type='parcoords',");
    fprintf (fp, "ncols = %d,", g_list_length (display->splots));
    describe_parcoords_display (fp, gg, display, desc);
  } else if (GGOBI_IS_TIME_SERIES_DISPLAY(display)) {
    fprintf (fp, "type='timeseries',");
    fprintf (fp, "ncols = %d,", g_list_length (display->splots));
    describe_time_series_display (fp, gg, display, desc);
  } else if (GGOBI_IS_BARCHART_DISPLAY(display)) {
    fprintf (fp, "type='barchart',");  /* barchart or histogram */
    describe_barchart_display (fp, gg, display, desc);
  /*
    -- is_histogram and is_spine are attributes of the plot, not the
       display : sp->bar->is_histogram, etc.
    -- this will call describe_barchart_plot
    -- other useful attributes: nbins, breaks
  */
  }

  ADD_COMMA(fp);
  ADD_CR(fp);

  /* Add a few booleans describing the display */
  fprintf (fp, "showMissing=%d,", display->d->missings_show_p);
  fprintf (fp, "showPoints=%d,", display->options.points_show_p);
  fprintf (fp, "showDirectedEdges=%d,",
    display->options.edges_directed_show_p);
  fprintf (fp, "showUndirectedEdges=%d,",
    display->options.edges_undirected_show_p);
  fprintf (fp, "showArrowheads=%d",
    display->options.edges_arrowheads_show_p);

  CLOSE_LIST(fp);
  ADD_CR(fp);

  fclose(fp);

  //gtk_widget_hide(desc->window);
  /* Put a message in the status bar saying it was done. */
}
