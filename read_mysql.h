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

  const char *dataQuery;
  const char *segmentsQuery;
  const char *colorQuery;
  
} MySQLLoginInfo;

int read_mysql_data(MySQLLoginInfo *login, ggobid *gg);
MYSQL *GGOBI(mysql_connect)(MySQLLoginInfo *login, ggobid *gg);
int  GGOBI(get_mysql_data)(MYSQL *conn, const char *query, ggobid *gg);
void GGOBI(mysql_warning)(const char *msg, MYSQL *conn, ggobid *gg);
int GGOBI(register_mysql_data)(MYSQL *conn, MYSQL_RES *res, int preFetched, ggobid *gg);
MySQLGUIInput *GGOBI(get_mysql_login_info)(ggobid *gg);
/* This should go somewhere else. */
void GGOBI(setDimensions)(int nrow, int ncol, ggobid *gg);
#endif
