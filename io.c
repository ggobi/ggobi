/* io.c -- routines that are used in both reading and writing */

#include <strings.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include "writedata.h"

#define READ_FILESET   0
#define EXTEND_FILESET 1
#define WRITE_FILESET  2

void
filesel_ok (GtkWidget *w, datad *d, GtkFileSelection *fs)
{
  extern const gchar* const key_get (void);
  ggobid *gg = (ggobid *) gtk_object_get_data (GTK_OBJECT (fs), key_get());
  gchar *fname = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  guint action = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (fs),
                 "action"));
  guint len = strlen (fname);
  gchar *filename;


  switch (action) {
    case READ_FILESET:
      /*-- deal with suffixes? --*/
      if (fileset_read_init (fname, gg)) 
        ;
      break;
    case EXTEND_FILESET:  /*-- not yet enabled --*/
      break;
    case WRITE_FILESET:
      switch (gg->save.format) {
        case XMLDATA:
#ifdef USE_XML
          /*-- if fname already contains ".xml", then don't add it --*/
          if (g_strncasecmp (&fname[len-4], ".xml", 4) == 0)
            filename = g_strdup (fname);
          else
            filename = g_strdup_printf ("%s.xml", fname);

/*        write_xml ((const gchar *) g_strdup_printf ("%s.xml", fname), gg);*/
          write_xml ((const gchar *) filename, gg);
          g_free (filename);
#endif
          break;
        case ASCIIDATA:
          /*-- if fname already contains ".dat", then strip it off --*/
          if (g_strncasecmp (&fname[len-4], ".dat", 4) == 0)
            filename = g_strndup (fname, len-4);
          else
            filename = g_strdup (fname);
          g_printerr ("filename=%s\n", filename);
          ggobi_file_set_create (filename, gg);
          g_free (filename);
          break;
        case BINARYDATA:  /*-- not implemented --*/
          break;
        case MYSQL_DATA:  /*-- not implemented --*/
          break;
      }
      break;
  }
}

static void
filename_get_configure (GtkWidget *fs, guint type, ggobid *gg) {
  extern const gchar* const key_get (void);

  gtk_object_set_data (GTK_OBJECT (fs), "action", GINT_TO_POINTER (type));
  gtk_object_set_data (GTK_OBJECT (fs), key_get(), gg);

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (filesel_ok), (gpointer) fs);
                            
  /*-- Ensure that the dialog box is destroyed. --*/
    
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) fs);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(fs)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (gpointer) fs);
}

/*--------------------------------------------------------------------------*/
/*                    reading files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_r (ggobid *gg, guint action, GtkWidget *w) 
{
  GtkWidget *fs = gtk_file_selection_new ("read ggobi data");
  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs));

  filename_get_configure (fs, READ_FILESET, gg);

  gtk_widget_show (fs);
}

/*
 * Open a file for reading
*/
FILE *open_file_r (gchar *f, gchar *suffix)
{
  FILE *fp = NULL;
  gchar *fname;

  fname = g_strdup_printf ("%s%s", f, suffix);

#ifndef _WIN32
  if (access (fname, R_OK) == 0)
#endif
    fp = fopen (fname, "r");

  if (fp != NULL) {
    /*
     * Make sure it isn't an empty file -- get a single character
    */
    gint ch = getc (fp);
    if (ch == EOF) {
      g_printerr ("%s is an empty file!\n", fname);
      fclose (fp);
      fp = NULL;
    } else ungetc (ch, fp);
  }

  g_free (fname);
  return fp;
}

FILE *open_ggobi_file_r (gchar *fname, gint nsuffixes, gchar **suffixes,
  gboolean optional)
{
  FILE *fp = NULL;
  gint n;

  if (nsuffixes == 0)
    fp = open_file_r (fname, "");

  else {
    for (n=0; n<nsuffixes; n++) {
      if ((fp = open_file_r (fname, suffixes[n])) != NULL)
        break;
    }
  }

  if (fp == NULL && !optional) {
    GString *gstr = g_string_new ("Unable to open ");
    if (nsuffixes > 0) {
      for (n=0; n<nsuffixes; n++) {
        if (n < nsuffixes-1)
          g_string_sprintfa (gstr, " %s%s or", fname, suffixes[n]);
        else
          g_string_sprintfa (gstr, " %s%s", fname, suffixes[n]);
      }

    } else 
      g_string_sprintfa (gstr, "%s", fname);

    g_printerr ("%s\n", gstr->str);
    g_string_free (gstr, true);
  }

  return fp;
}

/*--------------------------------------------------------------------------*/
/*                    writing files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_w (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *fs;

  if (gg->save.format == XMLDATA)
    fs = gtk_file_selection_new ("Specify base name for new xml file");
  else
    fs = gtk_file_selection_new ("Specify base name for new file set");
  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs));

  filename_get_configure (fs, WRITE_FILESET, gg);

  gtk_widget_show (fs);
}
