#include <mysql.h>

#include "ggobi.h"
#include "GGobiAPI.h"

#include "dbms_ui.h"

#include <stdlib.h>
#include <string.h>

gboolean mysql_read(InputDescription *desc, ggobid *gg, GGobiInputPluginInfo *);
int read_mysql_data(DBMSLoginInfo *info, gboolean init, ggobid *gg);


void mysql_warning(const char *msg, MYSQL *conn, ggobid *gg);
MYSQL* makeConnection(DBMSLoginInfo *login, ggobid *gg);
MYSQL_RES *query(const char * const query, MYSQL *conn, ggobid *gg);
int processResult(MYSQL_RES *result, MYSQL *conn, ggobid *gg);

/**
  This creates and populates an InputDescription object
  which will cause GGobi to use the routines in this file
  to read data. Specifically, GGobi will call postgres_read()
  below which will open a GUI to get the DBMS inputs from the user,
  including the login information, query for the data, etc.

  @see postgres_read.
 */
InputDescription *
mysql_input_description(const char * const fileName, const char * const modeName, 
                             ggobid *gg, GGobiInputPluginInfo *info)
{
  InputDescription *desc;
  desc = (InputDescription*) g_malloc(sizeof(InputDescription));
  memset(desc, '\0', sizeof(InputDescription));

  desc->fileName = g_strdup("MySQL table");
  desc->mode = unknown_data;
  desc->desc_read_input = mysql_read;

  return(desc);
}

/**
 This is the initial entry point for reading data from 
 Postgres. This only performs the initial setup, specifically
 requesting the inputs from the user for parameterizing
 the connection and the SQL queries for the different
 data information. It does this by using the DBMS GUI
 input routines in GGobi.
 */
gboolean 
mysql_read(InputDescription *desc, ggobid *gg, GGobiInputPluginInfo *plugin)
{
    DBMSLoginInfo *info ;
    info = initDBMSLoginInfo(NULL);
     /* We would read these values from a file. */

    info->desc = desc;
    info->dbms_read_input = read_mysql_data;

    GGOBI(get_dbms_login_info)(info, gg);

    return(false);
}


/******************************************************************/

/**
 This is the routine that actually reads the data, using the inputs
 from the user gathered by the GUI into the `info' structure.
 */
int
read_mysql_data(DBMSLoginInfo *info, gboolean init, ggobid *gg)
{
    MYSQL *conn;
    MYSQL_RES *result;

    conn = makeConnection(info, gg);    
    if(!conn) {
	return(-1);
    }

    result = query(info->dataQuery, conn, gg);
    processResult(result, conn, gg);
    mysql_free_result(result);
    mysql_close(conn);
    
    start_ggobi(gg, true, init);
 
    return(1);
}


MYSQL*
makeConnection(DBMSLoginInfo *login, ggobid *gg)
{
  MYSQL *conn;

  conn = mysql_init(NULL);

  if(conn == NULL) {
    mysql_warning("Can't initialize mysql!", conn, gg);
    return(NULL);
  }

  conn = mysql_real_connect(conn, login->host, login->user, login->password,
			     login->dbname, login->port, login->socket, login->flags
                           );

  if(conn == NULL) {
    mysql_warning("Can't connect to mysql!", conn, gg);   
    return(NULL);
  }

  return(conn);
}

MYSQL_RES *
query(const char * const query, MYSQL *conn, ggobid *gg)
{
  MYSQL_RES *res;
  int status;
  char *msg;

  status =  mysql_query(conn, query);

  if( status || (res = mysql_store_result(conn)) == NULL ) {
      mysql_warning(query, conn, gg);
  }

  return(res);
}

void
mysql_warning(const char *msg, MYSQL *conn, ggobid *gg)
{
 char *errmsg = NULL; 
 char *buf;
 if(conn) {
   errmsg = mysql_error(conn);
   if(errmsg == NULL)
     errmsg = ""; 
 } else 
   errmsg = "";

  buf = (char *) g_malloc(sizeof(char) * (strlen(errmsg) + strlen(msg) + 2));
  sprintf(buf, "%s %s", msg, errmsg);

  quick_message(buf,true);
  free(buf);
}


int
processResult(MYSQL_RES *result, MYSQL *conn, ggobid *gg)
{
  int i, j;
  int nr, nc;
  datad *d;

  nr =  mysql_num_rows(result);
  nc = mysql_num_fields(result);
 
  d = datad_create(nr, nc, gg);

  for(i = 0; i < nr; i++) {
      MYSQL_ROW row;
      float f;
      char *tmp;
      char *l;
      char buf[10];
      sprintf(buf, "%d", 1 + i);
      l = g_strdup(buf);
      g_array_append_val (d->rowlab, l);

      row = mysql_fetch_row(result);
      if(row == NULL)
	  break;
 
     for(j = 0; j < nc; j++) {
	  if(i == 0) {
	      MYSQL_FIELD *field = mysql_fetch_field(result);
	      GGOBI(setVariableName)(j, g_strdup(field->name), false, d, gg);
	  }

	  tmp = row[j];
          if(tmp)
	      f = atof(tmp);
	  else
	      f = 0.;
	  d->raw.vals[i][j] = f;
      }
  }

  return(1);
}

#ifdef STANDALONE
int
main(int argc, char *argv[])
{
  PGconn *conn;
  PGresult *result;
  float *data;
  int nr, nc;
  DataFrame frame;
  DBMSLoginInfo info;
  memset(&info, '\0', sizeof(DBMSLoginInfo));

    conn = makeConnection(&info);
    result = query(conn);
    processResult(result, &frame);

    fprintf(stderr, "Result: %d, %d\n", frame.nr, frame.nc);
    return(0); 
}
#endif
