#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "GGobiAPI.h"

#include "ggobi.h"
#include "types.h"
#include "vars.h"
#include "externs.h"

void displays_release(xgobid *xg);
void display_release(displayd *display, xgobid *xg);
void splot_release(splotd *sp, displayd *display, xgobid *xg);
void data_release(xgobid *xg);
void vardata_free(xgobid *xg);
void vardatum_free(vardatad *var, xgobid *xg);

gchar *
getFileName ()
{
  return(xg.filename);
}


DataMode
getDataMode ()
{
  return(xg.data_mode);
}


gchar **
getVariableNames(int transformed)
{
  gchar **names;
  int nc = xg.ncols, i;
  vardatad *form;

  names = (gchar**) g_malloc(sizeof(gchar*)*nc);

    for(i = 0; i < nc; i++) {
      form = xg.vardata + i;
      names[i] = transformed ? form->collab_tform : form->collab;
    }

  return(names);
}

void
setVariableName(gint jvar, gchar *name)
{
 gchar *old = xg.vardata[jvar].collab;

 if(old)
   g_free(old);

 xg.vardata[jvar].collab = g_strdup(name);
}



void 
destroyCurrentDisplay ()
{
  display_free (xg.current_display, false);
}

/*

  An initial attempt to allow new data to be introduced
  to the Ggobi session, replacing the existing contents.
 
  There are still a few details remaining regarding the scaling
  on the axes, etc. (See ruler_ranges_set in scatterplot.c)
  The reverse pipeline data has not been established correctly
  and the computation is incorrect.
  Specifically, the routine splot_screen_to_tform() is not
  

  When this works, we will take the calls for the different stages 
  and put them in separate routines.
 */
void
XGOBI(setData)(double *values, gchar **rownames, gchar **colnames, int nr, int nc)
{
extern void rowlabels_alloc(void);
 int i, j;

  displays_release(&xg);
  data_release(&xg);

  xg.ncols = nc;
  xg.nrows = nr;

  xg.nrows_in_plot = xg.nrows;  /*-- for now --*/
  xg.nlinkable = xg.nrows;      /*-- for now --*/
  xg.nrgroups = 0;              /*-- for now --*/

  xg.rows_in_plot = NULL;

  arrayf_alloc(&xg.raw, nr, nc);

  rowlabels_alloc();
  /*  xg.rowlab = (gchar **) g_malloc(nr * sizeof(gchar*)); */

  vardata_alloc();
  vardata_init();

  for(j = 0; j < nc ; j++) {
   xg.vardata[j].collab = g_strdup(colnames[j]);
   xg.vardata[j].collab_tform = g_strdup(colnames[j]);
   for(i = 0; i < nr ; i++) {
     if(j == 0) {
       xg.rowlab[i] = g_strdup(rownames[i]);
     }

     xg.raw.data[i][j] = values[i + j*nr];
   }
  }

   /* Now recompute and display the top plot. */
  dataset_init(&xg);

  /* Have to patch up the displays list since we removed
     every entry and that makes for meaningless entries.
   */
  xg.displays->next = NULL;
}


/* These are all for freeing the currently held data. */

void
displays_release(xgobid *xg)
{
 GList *dlist;
 displayd *display;

 /* We have to be careful here as we are removing all the elements
    of the singly-linked list. When we remove the last one,
    the ->next value of the dlist becomes non-NULL. Hence
    we are getting garbage. Accordingly, we count down from the total
    number to remove using num and when this is 0, we exit.
    This should leave the slist allocated, but empty.

    We have to patch the list up afterwards.
  */
 int num = g_list_length(xg->displays);

 for(dlist = xg->displays; dlist != NULL; dlist = dlist->next, num--) {
  if(num == 0)
   break;
  display = (displayd *) dlist->data;
  /*  display_release(display, xg); */
  display_free(display, true);
 }
}

void
display_release(displayd *display, xgobid *xg)
{
   display_free(display, true);
}


void
splot_release(splotd *sp, displayd *display, xgobid *xg)
{
 splot_free(sp, display);
}

/* Not in the API for the moment. A "protected" routine. */
void
data_release(xgobid *xg)
{
 extern void rowlabels_free(void);
 if(xg->rowlab) {
    rowlabels_free();
    xg->rowlab = NULL;
 }

  vardata_free(xg);
}

void
vardata_free(xgobid *xg)
{
 int i;

  for(i = 0; i < xg->ncols ; i++) {
    vardatum_free(xg->vardata+i, xg);
    /*    g_free(xg->vardata +i ); */
  }
  g_free(xg->vardata);
  xg->vardata = NULL;
}

void 
vardatum_free(vardatad *var, xgobid *xg)
{
 if(var->collab)
  g_free(var->collab);
 if(var->collab_tform)
  g_free(var->collab_tform);
}
