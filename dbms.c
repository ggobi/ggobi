#include "dbms_ui.h"

#include <stdlib.h>

DBMSLoginInfo DefaultDBMSInfo;

/*
  Optionally allocate and initialize the MySQLLoginInfo
  instance by copying the values from the DefaultDBMSInfo.
 */
DBMSLoginInfo *
initDBMSLoginInfo(DBMSLoginInfo *login)
{
  if(login == NULL)
    login = (DBMSLoginInfo*) g_malloc(sizeof(DBMSLoginInfo));


  memset(login, '\0', sizeof(DBMSLoginInfo));

  *login = DefaultDBMSInfo;

  return(login);
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
setDBMSLoginElement(DBMSInfoElement i, char *val, DBMSLoginInfo *info)
{
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
