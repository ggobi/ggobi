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
  OutputDescription *file;

  /* Add more fields here to store other settings. */  
/*
Here's what we had in xgobi:
  Postscript printer:
Really the postscript printer command, eg lpr -Pserif

  Background color: (a string)
  Foreground color: (a string)
  Color for white glyphs: (a string)
The purpose of this was to let a plot with a dark background be
printed with a light background, and then to allow white glyphs
to be reset as dark glyphs.  I don't know how to do this now.
Color names don't really make sense any more, since ggobi doesn't
use color names, and they aren't portable to Windows, are they?
Should people specify r,g,b?  Or should we bring up a color wheel
to let them set the color?

  Pointsize:

Then we had two buttons: write to file, or send to printer
*/

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
typedef gboolean (*PrintDialogHandler)(PrintOptions *options, struct _PrintInfo *data,  void *userData);

struct _GGobiPrintHandler;
typedef struct _GGobiPrintHandler   GGobiPrintHandler;
typedef PrintOptions *(*PrintCallbackHandler)(PrintOptions *options, displayd *dpy, ggobid *gg, GGobiPrintHandler *);

struct _GGobiPrintHandler {
  PrintCallbackHandler callback;
  PrintDialogHandler dialog;  
  void *userData;
};

extern GGobiPrintHandler DefaultPrintHandler;

typedef struct _PrintInfo {
  PrintOptions *options;
  displayd *dpy;
  ggobid *ggobi;

  PrintDialogHandler handler;
  void *userData;

  GtkWidget *dialog;
} PrintInfo;


/*
  This presents a dialog which allows the user to edit the setting options.

  The 

  The PrintHandler allows the caller to specify a routine that will be invoked
  when the user clicks on the Ok button of the dialog.
 */

PrintOptions *showPrintDialog(PrintOptions *options, displayd *dpy, ggobid *gg, GGobiPrintHandler*);


GtkWidget *
createPrintDialog(PrintOptions *options, displayd *dpy, ggobid *gg, PrintDialogHandler print, void *userData);

void setStandardPrintHandlers();
PrintOptions *getDefaultPrintOptions();

gboolean PrintAsSVG(PrintOptions *options, PrintInfo *info, void *userData);
#endif
