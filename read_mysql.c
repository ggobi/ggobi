#include "read_mysql.h"
#include <stdlib.h>
#include <stdio.h>

static int initialized = 0;


MySQLLoginInfo DefaultMySQLInfo = {
  "tuolomne",
  "duncan",
  NULL,
  "ggobi",
  0,
  NULL,
  0,
  NULL,
  NULL,
  NULL
};

const char *MySQLDefaultUIInfo[10] = {"tuolomne", "duncan",NULL, "ggobi", NULL, NULL, NULL, "SELECT * from flea;", NULL, NULL};


MySQLLoginInfo *initMySQLLoginInfo(MySQLLoginInfo *login);


void GGOBI(cancelMySQLGUI)(GtkButton *button, MySQLGUIInput *guiInput);
void GGOBI(getMySQLGUIInfo)(GtkButton *button, MySQLGUIInput *guiInput);
void GGOBI(getMySQLGUIHelp)(GtkButton *button, MySQLGUIInput *guiInput);


int
read_mysql_data(MySQLLoginInfo *login, ggobid *gg)
{
  MYSQL *conn;
  conn = GGOBI(mysql_connect)(login, gg);
  if(conn == NULL) {
    free(login);
    return(0);
  }

  if(GGOBI(get_mysql_data)(conn, login->dataQuery, gg) < 1) {
    return(-1);
  }


  vgroups_sort(gg);
  { int j;
  for (j=0; j<gg->ncols; j++)
    gg->vardata[j].groupid = gg->vardata[j].groupid_ori;
  }

  segments_alloc(gg->nsegments, gg);

  if(gg->nsegments < 1)
   segments_create(gg);


  dataset_init(gg, true);
  return(1); /* everything was ok*/
}


MYSQL *
GGOBI(mysql_connect)(MySQLLoginInfo *login, ggobid *gg)
{
  MYSQL *conn;
  if(initialized == 0) {

  }

  conn = mysql_init(NULL);
  if(conn == NULL) {
    GGOBI(mysql_warning)("Can't initialize mysql!", conn, gg);
    return(NULL);
  }

  conn = mysql_real_connect(conn, login->host, login->user, login->password,
			      login->dbname, login->port, login->socket, login->flags
                             );

  if(conn == NULL) {
    GGOBI(mysql_warning)("Can't connect to mysql!", conn, gg);   
    return(NULL);
  }

  return(conn);
}


int
GGOBI(get_mysql_data)(MYSQL *conn, const char *query, ggobid *gg)
{
  MYSQL_RES *res;
  int status;

  if(query == NULL || query[0] == '\0')
    return(-1);

    status =  mysql_query(conn, query);

       /* Call mysql_use_result() to get the entries row at a time. */
    if( (res = mysql_store_result(conn)) == NULL ) {
      GGOBI(mysql_warning)("Error from query", conn, gg);
      return(-1);
    }

    gg->filename = g_strdup(query);
    GGOBI(register_mysql_data)(conn, res, 1, gg);

 return(gg->nrows);
}

/**
   Takes the result set and read each row.
 */
int
GGOBI(register_mysql_data)(MYSQL *conn, MYSQL_RES *res, int preFetched, ggobid *gg)
{
  int i, rownum = 0;
  unsigned long nrows, ncols;
  MYSQL_ROW row;


   nrows =  mysql_num_rows(res);
   ncols = mysql_num_fields(res);

   GGOBI(setDimensions)(nrows, ncols, gg);

   for(i = 0; i < ncols; i++) {
    MYSQL_FIELD *field = mysql_fetch_field(res);
    gg->vardata[i].collab = g_strdup(field->name);
    gg->vardata[i].collab_tform = g_strdup(field->name);
    gg->vardata[i].groupid = gg->vardata[i].groupid_ori = i;
   }

    while((row = mysql_fetch_row(res)) != NULL) { 
      for(i = 0; i < ncols; i++) {
        gg->raw.data[rownum][i] = atof(row[i]);
      }
      rownum++;
    }

 return(i);
}

/**
  Displays a warning/error from MySQL's error message using a dialog.
  Uses quick_message.
 */
void
GGOBI(mysql_warning)(const char *msg, MYSQL *conn, ggobid *gg)
{
 char *errmsg = NULL; 
 char *buf;
  if(conn)
   errmsg = mysql_error(conn);

  buf = g_malloc(sizeof(char) * (strlen(errmsg) + strlen(msg) + 2));
  sprintf(buf, "%s %s", msg, errmsg);

  quick_message(buf,true);
  free(buf);
}

void
GGOBI(setDimensions)(gint nrow, gint ncol, ggobid *gg)
{
  gg->nrows = nrow;
  gg->nrows_in_plot = gg->nrows;  /*-- for now --*/
  gg->nrgroups = 0;              /*-- for now --*/

  rowlabels_alloc (gg);
  br_glyph_ids_alloc (gg);
  br_glyph_ids_init (gg);

  br_color_ids_alloc (gg);
  br_color_ids_init (gg);


  gg->ncols = ncol;

  arrayf_alloc (&gg->raw, gg->nrows, gg->ncols);

  vardata_alloc (gg);
  vardata_init (gg);

  hidden_alloc (gg);
}


  const char *fieldNames[] = {"Host", 
                              "User",
                              "Password",
                              "Database",
                              "Port",
                              "Socket",
                              "Flags",
                              NULL,
                              "Data query",
                              "Segments query",
                              "Color query"
                             };

enum {HOST,USER, PASSWORD, DATABASE, PORT, SOCKET, FLAGS, MISS, DATA_QUERY, SEGMENTS_QUERY, COLOR_QUERY};

MySQLGUIInput *
GGOBI(get_mysql_login_info)(ggobid *gg)
{
  int i, ctr;
  GtkWidget *dialog,*lab, *input, *table;
  GtkWidget *okay_button, *cancel_button, *help_button;
  MySQLGUIInput *guiInputs;

  int n = sizeof(fieldNames)/sizeof(fieldNames[0]);

  guiInputs  = (MySQLGUIInput*) g_malloc(sizeof(MySQLGUIInput));

  dialog = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog), "MySQL Login & Query Settings");

  guiInputs->gg = gg;
  guiInputs->dialog = dialog;
  guiInputs->textInput = (GtkWidget**) g_malloc(sizeof(GtkWidget*) * n);
  guiInputs->numInputs = n;

  table = gtk_table_new(n, 2, 0);
  for(i = 0, ctr=0; i < n; i++) {
    if(fieldNames[i] == NULL) {
      guiInputs->textInput[i] = NULL;
      continue;
    }
    lab = gtk_label_new(fieldNames[i]);
    gtk_label_set_justify(GTK_LABEL(lab), GTK_JUSTIFY_LEFT);
    input = gtk_entry_new();
    if(i == PASSWORD)
      gtk_entry_set_visibility(GTK_ENTRY(input), FALSE);
    guiInputs->textInput[i] = input;

    if(MySQLDefaultUIInfo[ctr])
      gtk_entry_set_text(GTK_ENTRY(input), MySQLDefaultUIInfo[ctr]);

    gtk_table_attach_defaults(GTK_TABLE(table), lab, 0, 1, ctr,ctr+1);
    gtk_table_attach_defaults(GTK_TABLE(table), input, 1, 2, ctr, ctr+1);
    ctr++;
  }

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), table, TRUE, TRUE, 0);

  okay_button = gtk_button_new_with_label("Okay");
  cancel_button = gtk_button_new_with_label("Cancel");
  help_button = gtk_button_new_with_label("Help");
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), okay_button);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), cancel_button);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), help_button);

  gtk_widget_show_all(dialog);
  
  gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
                               GTK_SIGNAL_FUNC (GGOBI(cancelMySQLGUI)), guiInputs);

  gtk_signal_connect (GTK_OBJECT (okay_button), "clicked",
                               GTK_SIGNAL_FUNC (GGOBI(getMySQLGUIInfo)), guiInputs);
  gtk_signal_connect (GTK_OBJECT (help_button), "clicked",
                               GTK_SIGNAL_FUNC (GGOBI(getMySQLGUIHelp)), guiInputs);


  return(NULL);
}

MySQLLoginInfo *
initMySQLLoginInfo(MySQLLoginInfo *login)
{
  if(login == NULL)
    login = (MySQLLoginInfo*) g_malloc(sizeof(MySQLLoginInfo));


  memset(login, '\0', sizeof(MySQLLoginInfo));

  *login = DefaultMySQLInfo;

  return(login);
}


void
GGOBI(getMySQLGUIInfo)(GtkButton *button, MySQLGUIInput *guiInput)
{
 ggobid *gg = guiInput->gg;
 int i;
 char *val;
 MySQLLoginInfo* info = initMySQLLoginInfo(NULL);

 for(i = 0; i < guiInput->numInputs; i++) {
   if(guiInput->textInput[i] == NULL)
     continue;
   val = gtk_entry_get_text(GTK_ENTRY(guiInput->textInput[i]));
   if(val)
     val = g_strdup(val);
   else
     continue;
   switch(i) {
     case HOST:
       info->host = val;
       break;
     case USER:
       info->user = val;
       break;
     case PASSWORD:
       info->password = val;
       break;
     case DATABASE:
       info->dbname = val;
       break;
     case PORT:
       info->port = atoi(val);
       break;
     case SOCKET:
       info->socket = val;
       break;
     case FLAGS:
       info->flags = atoi(val);
       break;
     case DATA_QUERY:
       info->dataQuery = val;
       break;
     case COLOR_QUERY:
       info->colorQuery = val;
       break;
     case SEGMENTS_QUERY:
       info->segmentsQuery = val;
       break;

     default:

   }  
 }

  /* Only cancel if we read something. Otherwise,
     leave the display for the user to edit.
   */
 if(read_mysql_data(info, gg) > 0) {
   GGOBI(cancelMySQLGUI)(button, guiInput);
 }
}

/*
  Close the specified dialog and free up the associated GUI info.
 */

void
GGOBI(cancelMySQLGUI)(GtkButton *button, MySQLGUIInput *guiInput)
{
  gtk_widget_destroy (guiInput->dialog);
  g_free (guiInput);
}

void
GGOBI(getMySQLGUIHelp)(GtkButton *button, MySQLGUIInput *guiInput)
{
  quick_message("GGobi/MySQL help not implemented yet!", false);

  /*
  GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(win), "GGobi/MySQL login help");

  gtk_container_add(GTK_CONTAINER(win), );
  */
}
