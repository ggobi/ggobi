#include "print.h"

#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>



#if 0
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-uidefs.h>
#include <libgnomeui/gnome-stock.h>
#endif

gboolean PrintAsSVG(PrintOptions *options, PrintInfo *info, void *userData);

static void addDialogButtons(GtkWidget *dialog, PrintInfo *data);
static void cancelPrint(GtkButton *button, PrintInfo *info);
static void handlePrintOptions(GtkButton *button, PrintInfo *info);

GGobiPrintHandler DefaultPrintHandler;

PrintOptions *
showPrintDialog(PrintOptions *options, displayd *dpy, ggobid *gg, GGobiPrintHandler *printHandler)
{
 GtkWidget *dlg;

 dlg = createPrintDialog(options, dpy, gg, printHandler->dialog, printHandler->userData);
 gdk_window_show(dlg->window);
 gdk_window_raise(dlg->window);

 return(options);
}


GtkWidget *
createPrintDialog(PrintOptions *options, displayd *dpy, ggobid *gg, PrintDialogHandler print, void *userData)
{
 char *title;
 GtkWidget *dialog;
 PrintInfo *data;
  title = g_malloc((strlen("Print Options") + strlen((dpy ? " for display" : "")) + 1 )* sizeof(char));
  sprintf(title, "%s%s", "Print Options", (dpy ? " for display" : ""));

#ifdef USE_GNOME_DIALOGS
 dlg = gnome_dialog_new(title, GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL);
#else
  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), title);

#endif

  data = (PrintInfo *)g_malloc( 1 * sizeof(PrintInfo));
  data->options = options;
  data->dpy = dpy;
  data->ggobi = gg;
  data->dialog = dialog;
  data->handler = print;
  data->userData = userData;

  addDialogButtons(dialog, data);
  gtk_widget_show_all(dialog);
  return(dialog);
}

/*
  Local handlers for the printing dialog action buttons (Okay, Cancel).
 */

static void
handlePrintOptions(GtkButton *button, PrintInfo *info)
{
  gboolean ok = true;
  PrintOptions localOptions;
  PrintOptions *opts;

  opts =  (info->handler == NULL) ? &localOptions : info->ggobi->printOptions ;

    /* Get the settings from the dialog elements. 
       For the moment, just grab them from the defaults.
     */    
  getDefaultPrintOptions(opts);

  if(info->handler) {
    ok = info->handler(opts, info, info->userData);
  } else {
    /* We already have set the options globally when we got them. */
  }

  if(ok) {
    cancelPrint(button, info);
  }

}

static void
cancelPrint(GtkButton *button, PrintInfo *info)
{
  gtk_widget_destroy (info->dialog);
  g_free(info);
}

/*
 Adds the Okay and Cancel buttons to the specified dialog
 and establishes local handlers for the click actions.
 */
static void
addDialogButtons(GtkWidget *dialog, PrintInfo *data)
{
  GtkWidget *okay_button, *cancel_button, *help_button;
  okay_button = gtk_button_new_with_label("Okay");
  cancel_button = gtk_button_new_with_label("Cancel");
  help_button = gtk_button_new_with_label("Help");
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), okay_button);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), cancel_button);


      /* Now setup the action/signal handlers. */  
  gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (cancelPrint), data);


  gtk_signal_connect (GTK_OBJECT (okay_button), "clicked",
                      GTK_SIGNAL_FUNC (handlePrintOptions), data);

  /*  gnome_dialog_set_default(GNOME_DIALOG(dlg), GNOME_OK); */
}

/*
 This is the unfinished version of the default print handler
 for the stand-alone ggobi. 
 It is the value to which DefaultHandler is set.
 It simply takes the display object from the PrintInfo
 that was established when the print button was selected.

 We return true to indicate that the print was successful. 
 */

gboolean
PrintAsSVG(PrintOptions *options, PrintInfo *info, void *userData)
{
 extern void display_write_svg (ggobid *gg);

  display_write_svg(info->ggobi);

 return(true);
}


void 
setStandardPrintHandlers()
{
  if(DefaultPrintHandler.callback == NULL && DefaultPrintHandler.dialog == NULL) {
   DefaultPrintHandler.userData = NULL;
  }
  if(DefaultPrintHandler.callback == NULL)
   DefaultPrintHandler.callback = &showPrintDialog;
  if(DefaultPrintHandler.dialog == NULL)
    DefaultPrintHandler.dialog = PrintAsSVG;
}


PrintOptions*
getDefaultPrintOptions(PrintOptions *opts)
{
 GdkColor black, white;

 if(opts == NULL)
   opts = (PrintOptions *) g_malloc(sizeof(PrintOptions));

 opts->width = 480;
 opts->height = 400;
 opts->file = (OutputDescription *) g_malloc(sizeof(OutputDescription));
  opts->file->fileName = g_strdup("foo.svg");

 
  gdk_color_white(gdk_colormap_get_system (), &white);
  gdk_color_black(gdk_colormap_get_system (), &black);
 opts->background = white;
 opts->foreground = black;
  return(opts);
}
