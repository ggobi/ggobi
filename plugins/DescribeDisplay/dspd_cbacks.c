#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include "ggobiClass.h"

#include <stdio.h>

#include "plugin.h"
#include "dspdesc.h"

void
desc_write_scatterplot (FILE *fp, ggobid *gg, displayd *display, 
  dspdescd *desc)
{
  splotd *sp = display->splots->data;  /* or current_splot */
  datad *d = display->d;
  PipelineMode projection = projection_get (gg);
  gchar *indent1 = " ";
  gchar *indent2 = "  ";
  gchar *indent3 = "    ";
  gint i;
  gint n = -1;
  const gchar *const *gnames = GGOBI(getOpModeNames)(&n);
  vartabled *vt;

  /* Start with some of the following */
  /*
<scatterplot projection=""> # projection: 2d, 1d, 2x1dtour, etc.

 <points count="">               # nrows_in_plot
   <x> ... </x>                  # planar.x
   <y> ... </y>                  # planar.y
   <color> ... </color>          # color_now
   <glyphtype> ... </glyphtype>  # glyph_now.type
   <glyphsize> ... </glyphsize>  # glyph_now.size
   <ids> ... </ids>              # rowIds; optional
   <labels> ... </labels>        # rowlab; optional
 </points>

 <stickylabels n="">             # sticky_ids + row labels
   <label name="" index="" />
   <label name="" index="" />
 </stickylabels>


 <xyplotparams>                  # vartable->collab_tform
   <xlab> ... </xlab>
   <ylab> ... </ylab>
 </xyplotparams>
  */

  fprintf (fp, "%s<scatterplot projection='%s' title='%s'>\n", indent1,
    gnames[(gint) projection], desc->title);

  /*-- row-wise data ----------------------------------------*/
  fprintf (fp, "%s<points count='%d'>\n", indent2,
    d->nrows_in_plot);

  /* horizontal coordinates */
  fprintf (fp, "%s<x> ", indent3);
  for (i=0; i<d->nrows_in_plot; i++) {
    if (i && i%8 == 0) fprintf (fp, "\n%s", indent3);
    fprintf (fp, "%g ", sp->planar[ d->rows_in_plot.els[i] ].x);
  }
  fprintf (fp, "</x>\n");
  /* vertical coordinates */
  fprintf (fp, "%s<y> ", indent3);
  for (i=0; i<d->nrows_in_plot; i++) {
    if (i && i%8 == 0) fprintf (fp, "\n%s", indent3);
    fprintf (fp, "%g ", sp->planar[ d->rows_in_plot.els[i] ].y);
  }
  fprintf (fp, "</y>\n");

  fprintf (fp, "%s</points>\n", indent2);

  /*-- parameters for each projection ---------------------------------*/
  if (projection == P1PLOT) {
  } else if (projection == XYPLOT) {
    fprintf (fp, "%s<xyplotparams>\n", indent2);
    vt = vartable_element_get (sp->xyvars.x, d);
    fprintf (fp, "%s<xlab>%s</xlab>\n", indent3, vt->collab_tform); 
    vt = vartable_element_get (sp->xyvars.y, d);
    fprintf (fp, "%s<ylab>%s</ylab>\n", indent3, vt->collab_tform); 
    fprintf (fp, "%s</xyplotparams>\n", indent2);

  } else if (projection == TOUR1D) {
  } else if (projection == TOUR2D3) {
  } else if (projection == TOUR2D) {
  } else if (projection == COTOUR) {
  }

  fprintf (fp, "%s</scatterplot>\n", indent1);
}

void
desc_setup (dspdescd *desc)
{
  GtkWidget *entry;

  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(desc->window), "TITLE");
  if (desc->title) g_free(desc->title);
  desc->title = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);

  entry = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT(desc->window), "FILENAME");
  if (desc->filename) g_free(desc->filename);
  desc->filename = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
} 

void
desc_write_cb (GtkWidget *btn, PluginInstance *inst)
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

  fprintf (fp, "<ggdisplay>\n");

  if (GTK_IS_GGOBI_SCATTERPLOT_DISPLAY(display))
    desc_write_scatterplot (fp, gg, display, desc);

  fprintf (fp, "</ggdisplay>\n");
  fclose(fp);
}
