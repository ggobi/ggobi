/* io.c -- routines that are used in both reading and writing */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <unistd.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include "writedata.h"
#include "write_xml.h"

#define READ_FILESET   0
#define EXTEND_FILESET 1
#define WRITE_FILESET  2

void
filesel_ok (GtkWidget *w, GtkFileSelection *fs)
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
      if (fileset_read_init (fname, unknown_data, gg)) 
        /*-- destroy and rebuild the menu every time data is read in --*/
        display_menu_build (gg);
      break;
    case EXTEND_FILESET:  /*-- not yet enabled --*/
      break;
    case WRITE_FILESET:
      switch (gg->save.format) {
        case XMLDATA:
#ifdef USE_XML
         {
           XmlWriteInfo info;

          /*-- if fname already contains ".xml", then don't add it --*/
          if (len >= 4 && g_strncasecmp (&fname[len-4], ".xml", 4) == 0)
            filename = g_strdup (fname);
          else
            filename = g_strdup_printf ("%s.xml", fname);

          memset(&info, '0',sizeof(XmlWriteInfo));
          info.useDefault = true;
          write_xml ((const gchar *) filename, gg, &info);
          g_free (filename);
	 }
#endif
          break;
        case ASCIIDATA:
        {
          datad *d = NULL;
          GSList *l = gg->d;
          gchar *name;
          gint k;
          gint nd = g_slist_length (gg->d);
          
          /*-- if fname already contains ".dat", then strip it off --*/
          if (len >= 4 && g_strncasecmp (&fname[len-4], ".dat", 4) == 0)
            filename = g_strndup (fname, len-4);
          else
            filename = g_strdup (fname);

          k = 0;
          while (l) {
            d = (datad *) l->data;
            name = (nd > 1) ?
              g_strdup_printf ("%s%d", filename, k) : g_strdup (filename);
            ggobi_file_set_create (name, d, gg);
            l = l->next;
            k++;
            g_free (name);
          }


          g_free (filename);
          break;
        }
        case BINARYDATA:  /*-- not yet implemented --*/
          break;
        case MYSQL_DATA:  /*-- never will be implemented --*/
          break;
      }
      break;
  }
}

static void
filename_get_configure (GtkWidget *fs, guint type, ggobid *gg) {
  extern const gchar* const key_get (void);

  const gchar *key = key_get();
  gtk_object_set_data (GTK_OBJECT (fs), "action", GINT_TO_POINTER (type));
  gtk_object_set_data (GTK_OBJECT (fs), key, gg);

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (filesel_ok), (gpointer) fs);
                            
  /*-- Ensure that the dialog box is destroyed. --*/
    
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (GtkObject*) fs);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(fs)->cancel_button),
                             "clicked", (GtkSignalFunc) GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (GtkObject*) fs);
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
    gint ch;
    ch = getc (fp);
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
