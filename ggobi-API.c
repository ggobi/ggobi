#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "GGobiAPI.h"


#include "types.h"
#include "vars.h"
#include "externs.h"

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
  display_free (xg.current_display);
}
