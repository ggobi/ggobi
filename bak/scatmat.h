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

 gint nxvars, nyvars;
/*
 * The plots are laid out in a table, but how are they stored?
 * Probably using a doubly linked list of doubly linked lists.
 * Each plots knows its variables, so I'm not sure this structure
 * has to know them.
*/
 GList *splots;  /* doubly linked list of splots, doing 2d plotting */


 viewd view;

} scatmatd;

#define SCATMAT_H
