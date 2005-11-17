/* read_mysql.h */
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

#ifndef READ_MYSQL_H
#define READ_MYSQL_H

#include <mysql.h>

#include "ggobi.h"
#include "externs.h"

#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>


typedef struct  {
  GtkWidget **textInput;
  int numInputs;
  ggobid *gg;
  GtkWidget *dialog;
} MySQLGUIInput;

typedef struct {
  char *host;
  char *user;
  char *password;
  char *dbname;
  unsigned int   port;
  char *socket;
  unsigned int   flags;

  char *dataQuery;
  char *segmentsQuery;
  char *appearanceQuery;
  char *colorQuery;
  
} MySQLLoginInfo;

/* Must correspond to the fieldNames in read_mysql.c */
typedef enum {HOST,USER, PASSWORD, DATABASE, PORT, SOCKET, FLAGS, MISS, 
              DATA_QUERY, SEGMENTS_QUERY, APPEARANCE_QUERY, COLOR_QUERY} 
           MySQLInfoElement;

#ifdef __cplusplus 
extern "C" {
#endif

  int read_mysql_data(MySQLLoginInfo *login, int init, ggobid *gg);
  MYSQL *GGOBI(mysql_connect)(MySQLLoginInfo *login, ggobid *gg);
  datad*  GGOBI(get_mysql_data)(MYSQL *conn, const char *query, ggobid *gg);
  void GGOBI(mysql_warning)(const char *msg, MYSQL *conn, ggobid *gg);
  int GGOBI(register_mysql_data)(MYSQL *conn, MYSQL_RES *res, int preFetched, ggobid *gg);
  MySQLGUIInput *GGOBI(get_mysql_login_info)(MySQLLoginInfo *info, ggobid *gg);
        /* This should go somewhere else. */
  void GGOBI(setDimensions)(int nrow, int ncol, ggobid *gg);

  int getDefaultValuesFromFile(char *fileName);
  int setMySQLLoginElement(MySQLInfoElement i, char *val, MySQLLoginInfo *info);
  MySQLInfoElement getMySQLLoginElementIndex(const char *name);

#ifdef __cplusplus 
}
#endif

#endif
