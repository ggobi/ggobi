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
 * The individual scatterplot itself
*/
 splotd splot;

/*
 * the state variables set at the interface
*/
 viewd view;

} scatterplotd;

#define SCATTERPLOT_H
