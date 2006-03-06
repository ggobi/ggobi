/* print.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include "print.h"

#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>

#include <string.h>
#include <stdio.h>

//static void addDialogButtons(GtkWidget *dialog, PrintInfo *data);
static void handlePrintOptions (PrintInfo * info);

GGobiPrintHandler DefaultPrintHandler;

PrintInfo *
createPrintInfo (GtkWidget * dialog, PrintOptions * options, displayd * dpy,
                 ggobid * gg, PrintDialogHandler print, void *userData)
{
  PrintInfo *data;
  data = (PrintInfo *) g_malloc (1 * sizeof (PrintInfo));
  data->options = options;
  data->dpy = dpy;
  data->ggobi = gg;
  data->dialog = dialog;
  data->handler = print;
  data->userData = userData;
  return (data);
}

PrintOptions *
showPrintDialog (PrintOptions * options, displayd * dpy, ggobid * gg,
                 GGobiPrintHandler * printHandler)
{
  GtkWidget *dlg;
  PrintInfo *info;

  dlg = createPrintDialog (dpy);
  info =
    createPrintInfo (dlg, options, dpy, gg, printHandler->dialog,
                     printHandler->userData);

  if (gtk_dialog_run (GTK_DIALOG (dlg)) == GTK_RESPONSE_ACCEPT)
    handlePrintOptions (info);

  gtk_widget_destroy (dlg);
  g_free (info);

  //gdk_window_show(dlg->window);
  //gdk_window_raise(dlg->window);

  return (options);
}



GtkWidget *
createPrintDialog (displayd * dpy)
{
  gchar *title;
  GtkWidget *dialog;

  title = g_malloc ((strlen ("Print Options") +
                     strlen ((dpy ? " for display" : "")) +
                     1) * sizeof (gchar));
  sprintf (title, "%s%s", "Print Options", (dpy ? " for display" : ""));

  dialog = gtk_dialog_new_with_buttons (title, NULL, 0,
                                        GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                        NULL);
  //gtk_window_set_title(GTK_WINDOW(dialog), title);



  //addDialogButtons(dialog, data);
  //gtk_widget_show_all(dialog);
  return (dialog);
}

/*
  Local handlers for the printing dialog action buttons (Okay, Cancel).
 */

static void
handlePrintOptions (PrintInfo * info)
{
  gboolean ok = true;
  PrintOptions localOptions;
  PrintOptions *opts;

  opts = (info->handler == NULL) ? &localOptions : info->ggobi->printOptions;

  /* Get the settings from the dialog elements. 
     For the moment, just grab them from the defaults.
   */
  getDefaultPrintOptions (opts);

  if (info->handler) {
    ok = info->handler (opts, info, info->userData);
  }
  else {
    /* We already have set the options globally when we got them. */
  }
}

/*
 This is the unfinished version of the default print handler
 for the stand-alone ggobi. 
 It is the value to which DefaultHandler is set.
 It simply takes the display object from the PrintInfo
 that was established when the print button was selected.

 We return true to indicate that the print was successful. 
 */

void
setStandardPrintHandlers ()
{
  if (DefaultPrintHandler.callback == NULL
      && DefaultPrintHandler.dialog == NULL) {
    DefaultPrintHandler.userData = NULL;
  }
  if (DefaultPrintHandler.callback == NULL)
    DefaultPrintHandler.callback = &showPrintDialog;
  //if(DefaultPrintHandler.dialog == NULL)
  //  DefaultPrintHandler.dialog = PrintAsSVG;
}


PrintOptions *
getDefaultPrintOptions (PrintOptions * opts)
{
  GdkColor black, white;

  if (opts == NULL)
    opts = (PrintOptions *) g_malloc (sizeof (PrintOptions));

  opts->width = 480;
  opts->height = 400;
  opts->file = (OutputDescription *) g_malloc (sizeof (OutputDescription));
  //opts->file->fileName = g_strdup("foo.svg");


  gdk_color_white (gdk_colormap_get_system (), &white);
  gdk_color_black (gdk_colormap_get_system (), &black);
  opts->background = white;
  opts->foreground = black;
  return (opts);
}
