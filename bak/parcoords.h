#ifndef DEFINES_H
#include "defines.h"
#endif

#ifndef VIEW_H
#include "view.h"
#endif

#ifndef SPLOT_H
#include "splot.h"
#endif

typedef struct {

 GtkWidget *window;
/*
 * Doubly linked list of splots, doing 1d plotting
 * Each plots knows its variables, so I'm not sure this structure
 * has to know them.
*/
 GList *pcplots;


 viewd view;

} parcoordsd;

#define PARCOORDS_H
