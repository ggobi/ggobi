#ifndef GGVIS_H

#include "defines.h"
#include "plugin.h"

typedef enum {deflt, within, between, anchorscales, anchorfixed} MDSGroupInd;

typedef struct {

  array_d dist_orig;
  array_d dist;
  array_d pos_orig;
  array_d pos;

} ggvisd;


/*----------------------------------------------------------------------*/
/*                          functions                                   */
/*----------------------------------------------------------------------*/

void ggvis_init (ggvisd *);
ggvisd* ggvisFromInst (PluginInstance *inst);

#define GGVIS_H
#endif
