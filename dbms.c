#include "dbms_ui.h"

#include <stdlib.h>
#include <string.h> /* memset() */

DBMSLoginInfo DefaultDBMSInfo;

void updateDBMSLoginInfo(DBMSLoginInfo *login, GHashTable *tbl);
DBMSLoginInfo *getDBMSCommandLineArgs(DBMSLoginInfo *login);

/*
  Optionally allocate and initialize the MySQLLoginInfo
  instance by copying the values from the DefaultDBMSInfo.
 */
DBMSLoginInfo *
initDBMSLoginInfo(DBMSLoginInfo *login, GHashTable *tbl)
{
  if(login == NULL)
    login = (DBMSLoginInfo*) g_malloc(sizeof(DBMSLoginInfo));


  memset(login, '\0', sizeof(DBMSLoginInfo));

  *login = DefaultDBMSInfo;

  if(tbl) {
      updateDBMSLoginInfo(login, tbl);
  }

  getDBMSCommandLineArgs(login);

  return(login);
}

DBMSLoginInfo *
getDBMSCommandLineArgs(DBMSLoginInfo *login)
{
    const char * tmp;
    tmp = getCommandLineArgValue("Host");
    if(tmp)
	setDBMSLoginElement(HOST, (char *)tmp, login);

    tmp = getCommandLineArgValue("User");
    if(tmp)
	setDBMSLoginElement(USER, (char *)tmp, login);

    tmp = getCommandLineArgValue("Database");
    if(tmp)
	setDBMSLoginElement(DATABASE, (char *)tmp, login);

    tmp = getCommandLineArgValue("DataQuery");
    if(tmp)
	setDBMSLoginElement(DATA_QUERY, (char *)tmp, login);

    return(login);
}


void
DBMSLoginInfoTableUpdate(gpointer key, gpointer value, gpointer userData)
{
    DBMSLoginInfo *login = (DBMSLoginInfo *)userData;
    DBMSInfoElement i;

    if(strcmp((char *)key, "DataQuery") == 0) {
	key = "Data query";
    }

    i = getDBMSLoginElementIndex((const char *)key);
    if(i != MISS) {
	setDBMSLoginElement(i, (char *)value, login);
    }
}

void
updateDBMSLoginInfo(DBMSLoginInfo *login, GHashTable *tbl)
{
    g_hash_table_foreach(tbl, DBMSLoginInfoTableUpdate, login);
}


const char * const DBMSFieldNames[] = {
                              "Host", 
                              "User",
                              "Password",
                              "Database",
                              "Port",
                              "Socket",
                              "Flags",
                              NULL,
                              "Data query",
                              "Segments query",
                              "Appearance query",
                              "Color query"
                             };


/*
  This maps the "field" name of the DBMSLoginInfo
 */
DBMSInfoElement
getDBMSLoginElementIndex(const char *name)
{
  unsigned int i;
  for(i = 0; i < NUM_DBMS_FIELDS; i++) {
    if(DBMSFieldNames[i] == NULL)
      continue;
    if(strcmp(DBMSFieldNames[i], name)==0) 
      return((DBMSInfoElement) i);
  }

  return(MISS);
}

int 
setDBMSLoginElement(DBMSInfoElement i, char * val, DBMSLoginInfo *info)
{
/* Arrange to have this done by the caller -- it causes a fatal error
   on Suns. */
/*
   if(val && !val[0])	
     val = NULL; 
*/

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
       if(val)
	   info->port = atoi(val);
       else
	   info->port = 0;
       break;
     case SOCKET:
       info->socket = val;
       break;
     case FLAGS:
       if(val)
	   info->flags = atoi(val);
       else
	   info->port = 0;
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
       break;
   }  

   return(i);
}

/*
  Retrieves the value of the element in the DBMSLoginInfo
  associated with the identifier (i).
 */
char *
getDBMSLoginElement(DBMSInfoElement i, int *isCopy, DBMSLoginInfo *info)
{ 
  char *val = NULL;
   switch(i) {
     case HOST:
       val = info->host;
       break;
     case USER:
       val = info->user;
       break;
     case PASSWORD:
       val = info->password;
       break;
     case DATABASE:
       val = info->dbname;
       break;
     case PORT:
       val = NULL;
       break;
     case SOCKET:
       val = NULL;
       break;
     case FLAGS:
       val = NULL;
       break;
     case DATA_QUERY:
       val = info->dataQuery;
       break;
     case COLOR_QUERY:
       val = info->colorQuery;
       break;
     case SEGMENTS_QUERY:
       val = info->segmentsQuery;
       break;
     default:
       break;
   }  

   return(val);
}

#ifdef USE_PROPERTIES
/*
   Read the DBMS defaults from the specified file
   and fill in the DefaultDBMSInfo with these values.

   This reads the specified file using the TrimmedProperties
   class and then iterates over the values to add them to the
   DefaultDBMSInfo object.
 */
int
getDefaultValuesFromFile(char *fileName)
{
  Properties *props = new TrimmedProperties(fileName);
  Property *prop;
  unsigned int i, ctr = 0;
  DBMSInfoElement id;

  for(i = 0; i < props->size() ; i++) {
    prop = props->element(i);
    id = getDBMSLoginElementIndex(prop->getName());
    if(id > -1) {
      setDBMSLoginElement(id, prop->getValue(), &DefaultDBMSInfo);
      ctr++;
    }
  }

  return(ctr);
}
#endif
