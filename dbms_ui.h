/* dbms_ui.h */

#ifndef DBMS_UI_H
#define DBMS_UI_H

#include <gtk/gtk.h>
#include <gtk/gtkdialog.h>

#include "ggobi.h"

#include "dbms.h"

typedef struct  {
  GtkWidget **textInput;
  int numInputs;
  ggobid *gg;
  GtkWidget *dialog;

  DBMSLoginInfo *info;
} DBMSGUIInput;


#ifdef __cplusplus 
extern "C" {
#endif

#if 0
  int read_mysql_data(MySQLLoginInfo *login, int init, ggobid *gg);
  MYSQL *GGOBI(mysql_connect)(MySQLLoginInfo *login, ggobid *gg);
  datad*  GGOBI(get_mysql_data)(MYSQL *conn, const char *query, ggobid *gg);
  void GGOBI(mysql_warning)(const char *msg, MYSQL *conn, ggobid *gg);
  int GGOBI(register_mysql_data)(MYSQL *conn, MYSQL_RES *res, int preFetched, ggobid *gg);
#endif

  DBMSGUIInput *GGOBI(get_dbms_login_info)(DBMSLoginInfo *info, ggobid *gg);
        /* This should go somewhere else. */
  void GGOBI(setDimensions)(int nrow, int ncol, ggobid *gg);

  int getDefaultValuesFromFile(char *fileName);
  DBMSInfoElement getDBMSLoginElementIndex(const char *name);


    void GGOBI(cancelDBMSGUI)(GtkButton *button, DBMSGUIInput *guiInput);
    void GGOBI(getDBMSGUIHelp)(GtkButton *button, DBMSGUIInput *guiInput);
    void GGOBI(getDBMSGUIInfo)(GtkButton *button, DBMSGUIInput *guiInput);

#ifdef __cplusplus 
}
#endif

#endif
