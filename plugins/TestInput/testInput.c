#include "GGobiAPI.h"
#include "ggobi.h"

#include <stdlib.h>

gboolean generate_data(InputDescription *desc, ggobid *gg);

InputDescription *
test_input_description(const char * const fileName, const char * const modeName, 
                             ggobid *gg, GGobiInputPluginInfo *info)
{
  InputDescription *desc;
  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  desc->fileName = g_strdup("Test input plugin data");
  desc->mode = unknown_data;
  desc->read_input = generate_data;

  return(desc);
}

gboolean 
generate_data(InputDescription *desc, ggobid *gg)
{

  int nr = 10, nc = 3;
  int i, j;
  datad *d;
  d = datad_create(nr, nc, gg);

  for(i = 0; i < nr; i++) {
      char *tmp;
      for(j = 0; j < nc; j++) {
	  if(i == 0) {
	      vartabled *vt;
	      tmp = g_strdup("abc");
	      sprintf(tmp, "%c", 'A' + i);
	      vt = vartable_element_get (j, d);
	      vt->collab = g_strdup(tmp);
	      vt->collab_tform = g_strdup(tmp);
	  }
	  d->raw.vals[i][j] = 100.*((float)rand()/(float)RAND_MAX);
      }
  }
//  start_ggobi(gg, true, true);

  return(true);
}
