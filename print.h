#ifndef GGOBI_PRINT_H
#define GGOBI_PRINT_H

/*
 This file defines structures that store the
 settings that control how printing is done.
 */

#include "ggobi.h"
#include "types.h"



typedef struct _OutputDescription {
  DataMode format;
  char *fileName;
  gboolean toPrinter;
} OutputDescription;

struct _PrintOptions {

  int width;
  int height;
  OutputDescription *fileName;

  /* Add more fields here to store other settings. */  

  GdkColor background;
  GdkColor foreground;
};


/*
  This is a typedef for a class of routines that can be registered
  for performing the actual printing.
  For example, we might register a C routine in R which would call
  a function to do the printing. In the stand-alone ggobi, we would
  have a version that does the printing via SVG.
 */
struct _PrintInfo;
typedef gboolean (*PrintHandler)(PrintOptions *options, struct _PrintInfo *data,  void *userData);
extern PrintHandler DefaultPrintHandler;

typedef struct _PrintInfo {
  PrintOptions *options;
  displayd *dpy;
  ggobid *ggobi;

  PrintHandler handler;
  void *userData;

  GtkWidget *dialog;
} PrintInfo;


/*
  This presents a dialog which allows the user to edit the setting options.

  The 

  The PrintHandler allows the caller to specify a routine that will be invoked
  when the user clicks on the Ok button of the dialog.
 */

PrintOptions *showPrintDialog(PrintOptions *options, displayd *dpy, ggobid *gg, PrintHandler print, void *userData);


GtkWidget *
createPrintDialog(PrintOptions *options, displayd *dpy, ggobid *gg, PrintHandler print, void *userData);


gboolean PrintAsSVG(PrintOptions *options, PrintInfo *info, void *userData);
#endif
