/*-- write_svg.c: routines for saving a display as an svg file --*/

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
#ifndef _WIN32
  gint i, m;
#endif
  gushort current_color;
  static gint npoint_colors_used = 0;
  static gushort point_colors_used[NCOLORS+2];
  GtkWidget *da = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gboolean draw_case;

  FILE *f = fopen ("foo.svg", "w");

  svg_write_header (f);
  fprintf (f, "<svg width=\"%dpx\" height=\"%dpx\">\n",
    da->allocation.width, da->allocation.height);
  fprintf (f, "<g>\n");

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

        /*-- determine whether case m should be plotted --*/
        draw_case = true;
        if (d->hidden_now[m])
          draw_case = false;

        /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
        else if (!display->options.missings_show_p && d->nmissing > 0) {
          switch (display->displaytype) {
            case parcoords:
              if (d->missing.vals[m][sp->p1dvar])
                draw_case = false;
              break;

            case scatmat:
              if (sp->p1dvar != -1) {
                if (d->missing.vals[m][sp->p1dvar])
                  draw_case = false;
              } else {
                if (d->missing.vals[m][sp->xyvars.x] ||
                    d->missing.vals[m][sp->xyvars.y])
                {
                  draw_case = false;
                }
              }
              break;

            case scatterplot:
              break;
          }
        }

        if (draw_case && d->color_now[m] == current_color) {
          if (display->options.points_show_p) {
            gchar *cx = hexcolor (&gg->default_color_table[current_color]);
            fprintf (f, "<circle style=\"fill: %s; stroke: %s\"\n", cx, cx);
            /*-- write out sp->screen values --*/
            fprintf (f, "cx=\"%d\" cy=\"%d\" r=\"%d\"/>\n",
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
