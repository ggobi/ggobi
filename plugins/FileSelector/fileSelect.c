#include "plugin.h"
#include <gtkextra/gtkiconfilesel.h>
#include "icon.xpm"

#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#endif

typedef struct {
    InputDescription *desc;
    GtkWidget *w;
    ggobid *gg;
    char *cwd;
} DialogInput;


gboolean show_fileselector(InputDescription *desc, ggobid *gg, GGobiPluginInfo *);

InputDescription *
get_description(const char *const fileName, const char *const modeName,
                  ggobid *gg, GGobiPluginInfo *info)
{

  InputDescription *desc;

  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  desc->fileName = g_strdup("File selection");
  desc->mode = unknown_data;
  desc->desc_read_input = show_fileselector;

  return(desc);
}

static void
ok_clicked(GtkWidget *widget, DialogInput *data)
{
  GtkIconFileSel *filesel;
  gchar *path;
  gchar *file;
  char *buf;

  filesel = GTK_ICON_FILESEL(data->w);
  path = gtk_file_list_get_path(GTK_FILE_LIST(filesel->file_list));
  file = gtk_file_list_get_filename(GTK_FILE_LIST(filesel->file_list));

 data->desc->fileName = g_malloc(sizeof(gchar) *(strlen(path) + strlen(file) + 1));
  sprintf(data->desc->fileName, "%s%s", path, file);


  data->desc->baseName = g_malloc(sizeof(gchar) *(strlen(file) + 1));
  sprintf(data->desc->baseName, "%s", file);


  data->desc->dirName = g_malloc(sizeof(gchar) *(strlen(path) + 1));
  sprintf(data->desc->dirName,  "%s", path);

  data->desc->mode =  xml_data; /* Will end up in recursive calls if use unknow_data */

  if(read_input(data->desc, data->gg)) {
      start_ggobi(data->gg, true, true);
  }

  gtk_widget_destroy(data->w);
  free(data->cwd);
  g_free(data);
}

static void
cancel_clicked(GtkWidget *widget, DialogInput *data)
{
    gtk_widget_destroy(data->w);
    g_free(data->desc->fileName);
    g_free(data->desc);
    g_free(data);
}

gboolean
show_fileselector(InputDescription *desc, ggobid *gg, GGobiPluginInfo *info)
{
    GtkWidget *w, *dlg;
    gint type;
    DialogInput *data;
    char buf[1000];
    fprintf(stderr, "Building file selector\n");fflush(stderr);
    data = (DialogInput *) g_malloc(sizeof(DialogInput));

    w = gtk_icon_file_selection_new("Select GGobi input file");

    data->desc = desc;
    data->w = w;
    data->gg = gg;
    getcwd(buf, sizeof(buf));
    data->cwd = g_strdup(buf);

    gtk_icon_file_selection_show_tree(GTK_ICON_FILESEL(w), TRUE); 

    type = gtk_file_list_add_type
                        (GTK_FILE_LIST(GTK_ICON_FILESEL(w)->file_list),
                         (const gchar **)icon);
    gtk_file_list_add_type_filter
                        (GTK_FILE_LIST(GTK_ICON_FILESEL(w)->file_list),
                         type,
                         "*.xml");
    gtk_icon_file_selection_open_dir(GTK_ICON_FILESEL(w), data->cwd);

    gtk_icon_file_selection_set_filter(GTK_ICON_FILESEL(w), "*.xml");


    gtk_signal_connect (GTK_OBJECT (GTK_ICON_FILESEL(w)->ok_button), 
                     "clicked",
		      GTK_SIGNAL_FUNC (ok_clicked), data);

    gtk_signal_connect (GTK_OBJECT (GTK_ICON_FILESEL(w)->cancel_button), 
                     "clicked",
		      GTK_SIGNAL_FUNC (cancel_clicked), data);

    gtk_widget_show(w);

    return(true);
}
