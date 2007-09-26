/* tour_pp.c */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke and Eun-Kyung Lee

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
#include "tour_pp_ui.h"
#include "display.h"

/* reset pp variables */
void reset_pp(GGobiStage *d, GGobiSession *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->t1d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t1d_window)) {
      free_optimize0_p(&dsp->t1d_pp_op);
      alloc_optimize0_p(&dsp->t1d_pp_op, d->n_rows, dsp->t1d.nactive, 1);
      t1d_pp_reinit(dsp, gg);
    }
    if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
      free_optimize0_p(&dsp->t2d_pp_op);
      alloc_optimize0_p(&dsp->t2d_pp_op, d->n_rows, dsp->t2d.nactive, 
        2);
      t2d_pp_reinit(dsp, gg);
    }
  }
}









