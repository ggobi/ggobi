/* tour2d.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <math.h>

#include "vars.h"
#include "externs.h"

void 
set_tour2dvar(ggobid *gg, gint jvar)
{
  gint j, jtmp, k;
  gboolean selected=false;
  displayd *dsp = gg->current_display;
  extern void zero_tau(displayd *, ggobid *);
  extern void zero_tinc(displayd *, ggobid *);
  extern void init_basis(displayd *, ggobid *);

  for (j=0; j<dsp->ntour_vars; j++)
    if (jvar == dsp->tour_vars[j])
      selected = true;

  /* deselect var if ntour_vars > 2 */
  if (selected) {
    if (dsp->ntour_vars > 2) {
      for (j=0; j<dsp->ntour_vars; j++) {
        if (jvar == dsp->tour_vars[j]) 
          break;
      }
      if (j<dsp->ntour_vars-1) {
        for (k=j; k<dsp->ntour_vars-1; k++){
          dsp->tour_vars[k] = dsp->tour_vars[k+1];
	}
      }
      dsp->ntour_vars--;
    }
  }
  else { /* not selected, so add the variable */
    if (jvar > dsp->tour_vars[dsp->ntour_vars-1]) {
      dsp->tour_vars[dsp->ntour_vars] = jvar;
    }
    else if (jvar < dsp->tour_vars[0]) {
      for (j=dsp->ntour_vars; j>0; j--) {
          dsp->tour_vars[j] = dsp->tour_vars[j-1];
      }
      dsp->tour_vars[0] = jvar;
    }
    else {
      for (j=0; j<dsp->ntour_vars-1; j++) {
        if (jvar > dsp->tour_vars[j] && jvar < dsp->tour_vars[j+1]) {
          jtmp = j+1;
          break;
	}
      }
      for (j=dsp->ntour_vars-1;j>=jtmp; j--) 
          dsp->tour_vars[j+1] = dsp->tour_vars[j];
      dsp->tour_vars[jtmp] = jvar;
    }
    dsp->ntour_vars++;
  }

  dsp->tour_get_new_target = true;
}

void
tour2d_varsel (ggobid *gg, gint jvar, gint button)
{
  if (button == 1 || button == 2) {
    set_tour2dvar(gg, jvar);
  }
}

