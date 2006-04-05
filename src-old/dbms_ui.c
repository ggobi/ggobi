/* dbms_ui.c */
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

#include "dbms_ui.h"

#ifdef USE_PROPERTIES
#include "properties.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "externs.h"

/*
   This creates the interactive dialog with which the user can specify the
   different parameters for the SQL connection, including the host, user,
   password, etc. and the query to get the data, color table, etc.
   The default values to display are taken from the DBMSLoginInfo
   object passed to this routine. If it is null, the default info
   is used. This allows the values read from an input file to be used
   as partial specification.
 */
DBMSGUIInput *GGOBI (get_dbms_login_info) (DBMSLoginInfo * info, ggobid * gg)
{
  int i, ctr;
  GtkWidget *dialog, *lab, *input, *table;
  GtkWidget *okay_button, *cancel_button, *help_button;
  DBMSGUIInput *guiInputs;
  char *tmpVal;
  int isCopy;
  int n = NUM_DBMS_FIELDS;      /* sizeof(fieldNames)/sizeof(fieldNames[0]); */
  gint response;

  if (info == NULL)
    info = &DefaultDBMSInfo;

  guiInputs = (DBMSGUIInput *) g_malloc (sizeof (DBMSGUIInput));

  /* Create the GUI and its components. */
  dialog =
    gtk_dialog_new_with_buttons ("DBMS Login & Query Settings", NULL, 0,
                                 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                 GTK_STOCK_HELP, GTK_RESPONSE_HELP, NULL);

  guiInputs->gg = gg;
  guiInputs->dialog = dialog;
  guiInputs->textInput = (GtkWidget **) g_malloc (sizeof (GtkWidget *) * n);
  guiInputs->numInputs = n;

  guiInputs->info = info;

  table = gtk_table_new (n, 2, 0);
  /* Now run through all the entries of interest and generate  
     the label, text entry pair. Store the entry widget
     in the guiInputs array of textInput elements.
     Then they can be queried in the handler of the Ok button click.
   */
  for (i = 0, ctr = 0; i < n; i++) {
    if (DBMSFieldNames[i] == NULL) {
      guiInputs->textInput[i] = NULL;
      continue;
    }
    lab = gtk_label_new (DBMSFieldNames[i]);
    gtk_label_set_justify (GTK_LABEL (lab), GTK_JUSTIFY_LEFT);
    input = gtk_entry_new ();
    if (i == PASSWORD)
      gtk_entry_set_visibility (GTK_ENTRY (input), FALSE);
    guiInputs->textInput[i] = input;

    tmpVal = getDBMSLoginElement ((DBMSInfoElement) i, &isCopy, info);
    if (tmpVal)
      gtk_entry_set_text (GTK_ENTRY (input), tmpVal);

    gtk_table_attach_defaults (GTK_TABLE (table), lab, 0, 1, ctr, ctr + 1);
    gtk_table_attach_defaults (GTK_TABLE (table), input, 1, 2, ctr, ctr + 1);
    ctr++;
  }

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), table, TRUE, TRUE,
                      0);

  while (true) {
    response = gtk_dialog_run (GTK_DIALOG (dialog));
    if (response == GTK_RESPONSE_HELP)
      GGOBI (getDBMSGUIHelp) (guiInputs);
    else if (response == GTK_RESPONSE_CANCEL
             || GGOBI (getDBMSGUIInfo) (guiInputs))
      break;
  }

  gtk_widget_destroy (dialog);
  g_free (guiInputs);

  /* Now add the buttons at the bottom of the dialog. */
  /*okay_button = gtk_button_new_with_label("Okay");
     cancel_button = gtk_button_new_with_label("Cancel");
     help_button = gtk_button_new_with_label("Help");
     gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), okay_button);
     gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), cancel_button);
     gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), help_button);

     gtk_widget_show_all(dialog);

     g_signal_connect (G_OBJECT (cancel_button), "clicked",
     G_CALLBACK (GGOBI(cancelDBMSGUI)), guiInputs);

     g_signal_connect (G_OBJECT (okay_button), "clicked",
     G_CALLBACK (GGOBI(getDBMSGUIInfo)), guiInputs);
     g_signal_connect (G_OBJECT (help_button), "clicked",
     G_CALLBACK (GGOBI(getDBMSGUIHelp)), guiInputs);
   */

  return (NULL);
}


/*
   Callback for the Ok button which processes the user's
   entries for all of the fields and packages them up into
   an DBMSLoginInfo object. Then it calls the read_mysql_data
   with this information.
   The guiInput argument contains the ggobid object reference
   and the array of input/entry widgets.
 */
gboolean GGOBI (getDBMSGUIInfo) (DBMSGUIInput * guiInput)
{
  ggobid *gg = guiInput->gg;
  gint i;
  gchar *val = NULL;
  DBMSLoginInfo *info;

  info = guiInput->info;

  for (i = 0; i < guiInput->numInputs; i++) {
    if (guiInput->textInput[i] == NULL)
      continue;
    val =
      gtk_editable_get_chars (GTK_EDITABLE (guiInput->textInput[i]), 0, -1);
    setDBMSLoginElement ((DBMSInfoElement) i, val, info);
    val = NULL;
  }

  /* Only cancel if we read something. Otherwise,
     leave the display for the user to edit.
   */
  if (info->dbms_read_input != NULL
      && info->dbms_read_input (info, TRUE, gg) > 0) {
    return (true);
  }
  return (false);
}

/*
  Close the specified dialog and free up the associated GUI info.
 */

void GGOBI (getDBMSGUIHelp) (DBMSGUIInput * guiInput)
{
  quick_message ("GGobi/DBMS help not implemented yet!", false);
}
