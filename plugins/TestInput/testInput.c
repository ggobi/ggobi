#include "GGobiAPI.h"
#include "ggobi.h"

#include <stdlib.h>

#include "GGStructSizes.c"

gboolean generate_data(InputDescription *desc, ggobid *gg, GGobiPluginInfo *info);

InputDescription *
test_input_description(const char * const fileName, const char * const modeName, 
                             ggobid *gg, GGobiPluginInfo *info)
{
  InputDescription *desc;
  g_printerr("[test_input_description  started]\n");
  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  desc->fileName = g_strdup("Test input plugin data");
  desc->mode = unknown_data;
  desc->desc_read_input = generate_data;
  g_printerr("[test_input_description  finished]\n");
  return(desc);
}

gboolean 
generate_data(InputDescription *desc, ggobid *gg, GGobiPluginInfo *plugin)
{
  int nr = 10, nc = 3;
  int i, j;
  datad *d;
  g_printerr("[generate_data %d %d]\n", nr, nc);
  d = datad_create(nr, nc, gg);

  for(i = 0; i < nr; i++) {
      char *tmp;
      for(j = 0; j < nc; j++) {
	  if(i == 0) {
	      vartabled *vt;
	      tmp = g_strdup("abc");
	      sprintf(tmp, "%c", 'A' + j);
#if 1
	      GGOBI(setVariableName)(j, g_strdup(tmp), true, d, gg);
#else
	      vt = vartable_element_get (j, d);
	      vt->collab = g_strdup(tmp);
	      vt->collab_tform = g_strdup(tmp);
#endif
	  }
	  d->raw.vals[i][j] = 100.*((float)rand()/(float)RAND_MAX);
      }
  }

  datad_init(d, gg, false);

  g_printerr("[starting ggobi %d %d]\n", nr, nc);
  start_ggobi(gg, true, true);

  return(true);
}
