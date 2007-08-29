#include "session.h"
#include "plugin.h"

#include <gdk_imlib.h>

typedef struct {
    PluginInstance *inst;
    GtkWidget *fileSelector;
} FileData;

static void saveAsImage(PluginInstance *inst, GtkWidget *widget);
static void SaveImage(FileData *data, GtkWidget *widget);


gboolean
init_image_plugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    GtkWidget *entry;

    /* Want to put it on any newly created display. */
    entry = GGobi_addToolsMenuItem("Save", gg);
    g_signal_connect_swapped(G_OBJECT(entry), "activate",
                               G_CALLBACK(saveAsImage), (gpointer) inst);
}

void
saveAsImage(PluginInstance *inst, GtkWidget *widget)
{

 GtkWidget *file_selector;
 FileData *data;

 data = (FileData *) g_malloc(sizeof(FileData));

 file_selector = gtk_file_selection_new("Image file name");
 data->fileSelector = file_selector;
 data->inst = inst;

 g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION(file_selector)->ok_button),
                                           "clicked", G_CALLBACK (SaveImage),
                                           (gpointer) data);

    g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION(file_selector)->cancel_button),
                                           "clicked", G_CALLBACK (gtk_widget_destroy),
                                           (gpointer) file_selector);

    gtk_widget_show(file_selector);
}

static void
SaveImage(FileData *data, GtkWidget *widget)
{
    displayd *dpy;
    GtkWidget *win;
    splotd *sp;
    GdkImlibImage *image;
    GdkImlibSaveInfo info;
    PluginInstance *inst = data->inst;
    char *fileName = "/tmp/foo.jpg";
    dpy = inst->gg->current_display;


    win = GGOBI_WINDOW_DISPLAY(dpy)->window;
    sp  = dpy->current_splot;
    gdk_imlib_init();

    fileName = gtk_file_selection_get_filename (GTK_FILE_SELECTION(data->fileSelector));
    fprintf(stderr, "Saving image %s\n", fileName);
    image = gdk_imlib_create_image_from_drawable(sp->da->window, NULL, 0, 0, sp->max.x, sp->max.y);

#if 0
    gdk_imlib_save_image_to_eim(image, "/tmp/foo.png");     
#else
    info.quality = 256;
    info.page_size = PAGE_SIZE_LETTER;
    info.color = RT_DITHER_TRUECOL;
    info.scaling = 1024;
    gdk_imlib_save_image(image, fileName, &info);     
#endif

    gdk_imlib_destroy_image(image);
    g_free(data);
}
