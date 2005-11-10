
/*----------------------------------------------------------------------*/
/*     forward pipeline for scatterplot: for one point only             */
/*----------------------------------------------------------------------*/

/*
 * this is the complement to screen_to_tform.
*/
    
void
splot_tform_to_screen (cpaneld *cpanel, splotd *sp, icoords *scr,
  fcoords *tfd, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;

  gfloat xmin = GTK_RULER (display->hrule)->lower;
  gfloat xmax = GTK_RULER (display->hrule)->upper;
  gfloat ymin = GTK_RULER (display->vrule)->upper;
  gfloat ymax = GTK_RULER (display->vrule)->lower;

  scr.x = (gint)
    ((gfloat) sp->max.x * (xmax - tfd->x) / (xmax - xmin));
  scr.y = (gint)
    ((gfloat) sp->max.y * (xmax - tfd->y) / (ymax - ymin));
}
