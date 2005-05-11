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
DBMSGUIInput *
GGOBI(get_dbms_login_info)(DBMSLoginInfo *info, ggobid *gg)
{
  int i, ctr;
  GtkWidget *dialog,*lab, *input, *table;
  GtkWidget *okay_button, *cancel_button, *help_button;
  DBMSGUIInput *guiInputs;
  char *tmpVal;
  int isCopy;
  int n = NUM_DBMS_FIELDS; /* sizeof(fieldNames)/sizeof(fieldNames[0]); */

  if(info == NULL)
    info = &DefaultDBMSInfo;

  guiInputs  = (DBMSGUIInput*) g_malloc(sizeof(DBMSGUIInput));

    /* Create the GUI and its components. */
  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "DBMS Login & Query Settings");

  guiInputs->gg = gg;
  guiInputs->dialog = dialog;
  guiInputs->textInput = (GtkWidget**) g_malloc(sizeof(GtkWidget*) * n);
  guiInputs->numInputs = n;

  guiInputs->info = info;

  table = gtk_table_new(n, 2, 0);
     /* Now run through all the entries of interest and generate  
        the label, text entry pair. Store the entry widget
        in the guiInputs array of textInput elements.
        Then they can be queried in the handler of the Ok button click.
      */
  for(i = 0, ctr=0; i < n; i++) {
    if(DBMSFieldNames[i] == NULL) {
      guiInputs->textInput[i] = NULL;
      continue;
    }
    lab = gtk_label_new(DBMSFieldNames[i]);
    gtk_label_set_justify(GTK_LABEL(lab), GTK_JUSTIFY_LEFT);
    input = gtk_entry_new();
    if(i == PASSWORD)
      gtk_entry_set_visibility(GTK_ENTRY(input), FALSE);
    guiInputs->textInput[i] = input;
    
    tmpVal = getDBMSLoginElement((DBMSInfoElement) i, &isCopy, info);
    if(tmpVal)
      gtk_entry_set_text(GTK_ENTRY(input), tmpVal);

    gtk_table_attach_defaults(GTK_TABLE(table), lab, 0, 1, ctr,ctr+1);
    gtk_table_attach_defaults(GTK_TABLE(table), input, 1, 2, ctr, ctr+1);
    ctr++;
  }

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, TRUE, TRUE, 0);


      /* Now add the buttons at the bottom of the dialog. */
  okay_button = gtk_button_new_with_label("Okay");
  cancel_button = gtk_button_new_with_label("Cancel");
  help_button = gtk_button_new_with_label("Help");
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), okay_button);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), cancel_button);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), help_button);

  gtk_widget_show_all(dialog);

      /* Now setup the action/signal handlers. */  
  gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (GGOBI(cancelDBMSGUI)), guiInputs);

  gtk_signal_connect (GTK_OBJECT (okay_button), "clicked",
                      GTK_SIGNAL_FUNC (GGOBI(getDBMSGUIInfo)), guiInputs);
  gtk_signal_connect (GTK_OBJECT (help_button), "clicked",
                      GTK_SIGNAL_FUNC (GGOBI(getDBMSGUIHelp)), guiInputs);


  return(NULL);
}


/*
   Callback for the Ok button which processes the user's
   entries for all of the fields and packages them up into
   an DBMSLoginInfo object. Then it calls the read_mysql_data
   with this information.
   The guiInput argument contains the ggobid object reference
   and the array of input/entry widgets.
 */
void
GGOBI(getDBMSGUIInfo)(GtkButton *button, DBMSGUIInput *guiInput)
{
 ggobid *gg = guiInput->gg;
 gint i;
 gchar *val = NULL;
 DBMSLoginInfo* info;  

 info = guiInput->info;

 for(i = 0; i < guiInput->numInputs; i++) {
   if(guiInput->textInput[i] == NULL)
     continue;

   val = gtk_editable_get_chars(GTK_EDITABLE(guiInput->textInput[i]), 0, -1);

#if 0
   if(val && val[0])
     /*val = g_strdup(val);*/ 
     /* Is this necessary with gtk_editable_get_chars? I bet not.  dfs */
     ;
   else 
     continue;
#endif

   setDBMSLoginElement((DBMSInfoElement) i, val, info);
   val = NULL;
 }

  /* Only cancel if we read something. Otherwise,
     leave the display for the user to edit.
   */
 if(info->dbms_read_input == NULL)
     return;

  if (info->dbms_read_input(info, TRUE, gg) > 0) {
   GGOBI(cancelDBMSGUI)(button, guiInput);
     /* Can we free the info here. */
  }
}

/*
  Close the specified dialog and free up the associated GUI info.
 */

void
GGOBI(cancelDBMSGUI)(GtkButton *button, DBMSGUIInput *guiInput)
{
  gtk_widget_destroy (guiInput->dialog);
  g_free (guiInput);
}

void
GGOBI(getDBMSGUIHelp)(GtkButton *button, DBMSGUIInput *guiInput)
{
  quick_message("GGobi/DBMS help not implemented yet!", false);
}
