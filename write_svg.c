/*-- write_svg.c: routines for saving a display as an svg file --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

gchar *hexcolor (GdkColor *color) {
  gulong val = 0;
  gchar buf[32];
  
  val = ((gulong) ((256 * color->red) / 65536)) << 16;
  val |= ((gulong) ((256 * color->green) / 65536)) << 8;
  val |= (gulong) ((256 * color->blue) / 65536);
   
  sprintf (buf, "#%6lx", val & 0xffffff);
  return g_strdup (buf);
}


void
svg_write_header (FILE *f) {
 fprintf (f,
   "<?xml version=\"1.0\" standalone=\"no\"?>\n");
 fprintf (f,
   "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20000303 Stylable//EN\"\n");
 fprintf (f,
   "\"http://www.w3.org/TR/2000/03/WD-SVG-20000303/DTD/svg-20000303-exchange.dtd\">\n");
 fprintf (f, "\n");
}

/*-- testing on xyplots only for the moment --*/
void
splot_write_svg (splotd *sp, ggobid *gg)
{
  gint k;
  gint i, m;
  gushort current_color;
  gint npoint_colors_used = 0;
  gushort point_colors_used[NCOLORS+2];
  GtkWidget *da = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gboolean draw_case;
  icoords minpix, maxpix;

  FILE *f = fopen ("foo.svg", "w");

  svg_write_header (f);

  fprintf (f, "<svg width=\"%dpx\" height=\"%dpx\">\n",
    da->allocation.width, da->allocation.height);

  /*
   * I want to know the minimum and maximum horizontal and
   * vertical pixel position for the plotted data.
  */
  maxpix.x = maxpix.y = 0;
  minpix.x = da->allocation.width;
  minpix.y = da->allocation.height;
  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];
    draw_case = splot_plot_case (m, d, sp, display, gg);
    if (draw_case) {
      if (sp->screen[m].x > 0 && sp->screen[m].x < da->allocation.width) {
        if (sp->screen[m].x < minpix.x)
          minpix.x = sp->screen[m].x;
        if (sp->screen[m].x > maxpix.x)
          maxpix.x = sp->screen[m].x;
      }
      if (sp->screen[m].y > 0 && sp->screen[m].y < da->allocation.height) {
        if (sp->screen[m].y < minpix.y)
          minpix.y = sp->screen[m].y;
        if (sp->screen[m].y > maxpix.y)
          maxpix.y = sp->screen[m].y;
      }
    }
  }

  /*
   * I'll shift enough to preserve about 20 pixels for
   * labelling on the left; and then scale enough to preserve
   * space on the bottom.  This could be really elaborate, but
   * this will get us started.
  */
  fprintf (f, "<g transform=\"translate(%d, %d) scale (%f)\">\n",
    MIN (20, minpix.x), 0,
    (gfloat) da->allocation.height / (gfloat) (da->allocation.height + 20));

  /*-- x axis --*/
  fprintf (f,
    "<path style=\"stroke: #000000\" d=\"M %d %d L %d %d z\"/>\n",
    minpix.x, maxpix.y, maxpix.x, maxpix.y);
/*
<path style="stroke: #000000" d="M 60 360 L 60 350 z"/>
<path style="stroke: #000000" d="M 60 350 L 340 350 z"/>
<path style="stroke: #000000" d="M 340 350 L 340 360 z"/>
<text x="50" y="375" > 122 </text>
<text x="330" y="375" > 242 </text>
*/

  /*-- y axis --*/
  fprintf (f,
    "<path style=\"stroke: #000000\" d=\"M %d %d L %d %d z\"/>\n",
    minpix.x, minpix.y, minpix.x, maxpix.y);
/*
<path style="stroke: #000000" d="M 40 60 L 50 60 z"/>
<path style="stroke: #000000" d="M 50 60 L 50 340 z"/>
<path style="stroke: #000000" d="M 50 340 L 40 340 z"/>
<text x="15" y="65" > 107 </text>
<text x="15" y="345" > 146 </text>
*/

  if (!gg->mono_p) {
    splot_point_colors_used_get (sp, &npoint_colors_used,
      point_colors_used, false, gg);

    /*
     * Now loop through point_colors_used[], plotting the points of each
     * color.  This avoids the need to reset the foreground so often.
     * On the other hand, it requires more looping.
    */
    for (k=0; k<npoint_colors_used; k++) {
      current_color = point_colors_used[k];


#ifdef _WIN32
#else
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        draw_case = splot_plot_case (m, d, sp, display, gg);

        if (draw_case && d->color_now[m] == current_color) {
          if (display->options.points_show_p) {
            gchar *cx = hexcolor (&gg->default_color_table[current_color]);
            fprintf (f, "<circle style=\"fill: %s; stroke: %s\"", cx, cx);
            /*-- write out sp->screen values --*/
            fprintf (f, " cx=\"%d\" cy=\"%d\" r=\"%d\"/>\n",
              sp->screen[m].x, sp->screen[m].y, 5);
          }
        }
      }
#endif
    }  /* deal with mono later */
  }

  fprintf (f, "</g>\n");
  fprintf (f, "</svg>\n");
  fclose (f);
  return;
}
void
display_write_svg (ggobid *gg) {
  /*-- for the moment, only deal with a single scatterplot --*/
  splot_write_svg (gg->current_splot, gg);
}
