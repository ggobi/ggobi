#ifndef GGOBI_READ_INIT_H
#define GGOBI_READ_INIT_H

#include "ggobi.h"
#include <gtk/gtk.h>
#include "fileio.h"

typedef struct _GGobiInitInfo {
    int numInputs;
    InputDescription *inputs;    
    char *filename;
} GGobiInitInfo;

GGobiInitInfo *read_init_file(const char *filename);

#endif /* end of GGOBI_READ_INIT_H */
