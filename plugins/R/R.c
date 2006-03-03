#include "RSCommon.h"
#include "plugin.h"
#include "GGobiAPI.h"


void R_PreserveObject(SEXP);
void R_ReleaseObject(SEXP);

gboolean initR(GGobiPluginInfo *pluginfo);
SEXP Rg_evalCmd(const char *cmd);


SEXP mapHashTable(GHashTable *table);
USER_OBJECT_ argsToCharacter(GSList *args);

typedef struct {
   const char *sourceFile;
   const char *constructor;
} RPluginData;

typedef struct _RRunTimeData RRunTimeData;

struct _RRunTimeData {
    USER_OBJECT_ pluginObject;
};

/**
  This is the initial entry point for the plugin that starts the
  virtual machine.
  @see initJVM()
 */
gboolean 
loadR(gboolean initializing, GGobiPluginInfo *pluginInfo)
{
    return(initR(pluginInfo));
} 


gboolean 
initR(GGobiPluginInfo *pluginfo)
{
  extern int Rf_initEmbeddedR(int argc, char *argv[]);
  char **argv;
  int argc = 2;
  char *init;

  argv = (char **) g_malloc(argc* sizeof(gchar*));
  argv[0] = g_strdup("RGGobiPlugin");
  argv[1] = g_strdup("--slave");
    Rf_initEmbeddedR(argc, argv);

  if(pluginfo->details->namedArgs) {
      init = (char *) g_hash_table_lookup(pluginfo->details->namedArgs, "init");
      if(init && init[0]) {
	  (void) Rg_evalCmd(init);
      }
  }

  fprintf(stderr, "Started R\n");fflush(stderr);

  return(true);
}


USER_OBJECT_
Rg_evalCmd(const char *cmd)
{
    USER_OBJECT_ e, tmp, val = NULL_USER_OBJECT;
    int errorOccurred = 0;

    PROTECT(e = allocVector(LANGSXP, 2));

    SETCAR(e, Rf_install("parse"));
    SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
    SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(cmd));
    SET_TAG(CDR(e), Rf_install("text"));

    val = R_tryEval(e, R_GlobalEnv, &errorOccurred);
    if(errorOccurred) {
	fprintf(stderr, "Cannot parse the command\n");fflush(stderr);
        UNPROTECT(1);
	return(NULL_USER_OBJECT);
    }
    PROTECT(val);
    SETCAR(e, Rf_install("eval"));
    SETCAR(CDR(e), val);
    SET_TAG(CDR(e), NULL_USER_OBJECT);
    val = R_tryEval(e, R_GlobalEnv, &errorOccurred);
    if(errorOccurred) {
	fprintf(stderr, "There was a problem\n");fflush(stderr);
	return(NULL_USER_OBJECT);
    }
    UNPROTECT(2);
    return(val);  
}

Rboolean
Rsource(const char *name)
{
  int errorOccurred = 0;

  if(canRead(name)) {
     SEXP tmp, val, e;

     PROTECT(e = allocVector(LANGSXP, 2));
     SETCAR(e, Rf_install("source"));
     SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
     SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(name));

     val = R_tryEval(e, R_GlobalEnv, &errorOccurred);
     if(errorOccurred) {
	 fprintf(stderr, "Failed to source %s\n", name);fflush(stderr);
     }

     UNPROTECT(1);
  } else {
    fprintf(stderr,"Cannot locate file %s for source()ing it.\n", name);fflush(stderr);
  }

  return(errorOccurred == 0);
}


gboolean
RLoadPlugin(gboolean initializing, GGobiPluginInfo *plugin)
{
    RPluginData *data = (RPluginData *) plugin->data;
    fprintf(stderr, "Loading R plugin from file `%s'\n", data->sourceFile);fflush(stderr);
    if(data->sourceFile)
	Rsource(data->sourceFile);
    return(true);
}


USER_OBJECT_
R_createRef(void *ptr, const char *className)
{
    USER_OBJECT_ val;
    USER_OBJECT_ klass;

    PROTECT(val = R_MakeExternalPtr(ptr, Rf_install((char *) className), R_NilValue));
    PROTECT(klass = NEW_CHARACTER(1));
    SET_STRING_ELT(klass, 0, COPY_TO_USER_STRING(className));
    SET_CLASS(val, klass);
    UNPROTECT(2);

    return(val);
}


USER_OBJECT_
R_ggobiPluginInstRef(PluginInstance *inst)
{
    return(R_createRef(inst, "ggobiPluginInstance"));
}

USER_OBJECT_
R_ggobiRef(ggobid *gg)
{
    return(R_createRef(gg, "ggobi"));
}

gboolean
RCreatePlugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    RPluginData *data = (RPluginData *) plugin->data;
    USER_OBJECT_ obj;
    int wasError;

    inst->data = NULL;

    fprintf(stderr, "Creating R plugin %s\n", data->constructor);fflush(stderr);
    if(data->constructor) {
	USER_OBJECT_ e, tmp;
        /* Construct a call of the form constructor(gg, inst) */
        PROTECT(e = allocVector(LANGSXP, 5));
	SETCAR(e, Rf_install((char *) data->constructor));
        tmp = CDR(e);
        SETCAR(tmp, R_ggobiRef(gg));    
	tmp = CDR(tmp);
        SETCAR(tmp, R_ggobiPluginInstRef(inst));    
	tmp = CDR(tmp);
        SETCAR(tmp, mapHashTable(plugin->details->namedArgs));
	tmp = CDR(tmp);
        SETCAR(tmp, argsToCharacter(plugin->details->args));

	obj = R_tryEval(e, R_GlobalEnv, &wasError);

	if(!wasError && obj && obj != NULL_USER_OBJECT) {
            RRunTimeData *runTime;
	    R_PreserveObject(obj);
	    runTime = (RRunTimeData *) g_malloc(sizeof(RRunTimeData));
	    runTime->pluginObject = obj;
            inst->data = runTime;
	} 
    }
    fprintf(stderr, "Created plugin\n");fflush(stderr);
    return(true);
}

gboolean
RDestroyPlugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    USER_OBJECT_ obj, e;
    RRunTimeData *d = (RRunTimeData *) inst->data;

    if(!d) 
       return(true);

    obj = d->pluginObject;
    if(!isFunction(VECTOR_ELT(obj, 0))) {
	return(false);
    }

    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, VECTOR_ELT(obj, 0));
    R_tryEval(e, R_GlobalEnv, NULL);
    UNPROTECT(1);
    R_ReleaseObject(obj);
    g_free(d);
    inst->data = NULL;

    return(true);
}

/** 
  Call the R plugin's onUpdateDisplay method.
  Currently assume this is the second element
  in the list that is the R plugin object.
 */
gboolean
RUpdateDisplayMenu(ggobid *gg, PluginInstance *inst)
{
    USER_OBJECT_ obj, e;
    RRunTimeData *d = (RRunTimeData *) inst->data;

    if(d == NULL)
	return(false);

    obj = d->pluginObject;
    if(Rf_length(obj) < 2 || !isFunction(VECTOR_ELT(obj, 1)))
	return(false);

    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, VECTOR_ELT(obj, 1));
    R_tryEval(e, R_GlobalEnv, NULL);
    UNPROTECT(1);

    return(true);
}

gboolean
RUnloadPlugin(gboolean quitting, GGobiPluginInfo *plugin)
{
    return(true);
}


enum {GET_DIMS, GET_VARIABLE_NAMES,
      GET_RECORD, GET_SOURCE_DESCRIPTION, 
      GET_NUM_RECORDS, GET_NUM_VARIABLES};


gboolean
getDims(int *nrow, int *ncol, USER_OBJECT_ obj)
{
    USER_OBJECT_ e, val;
    int wasError;
    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, VECTOR_ELT(obj, GET_DIMS));
    val = R_tryEval(e, R_GlobalEnv, &wasError);
    if(!wasError) {
	*nrow = INTEGER_DATA(val)[0];
	*ncol = INTEGER_DATA(val)[1];
    }
    UNPROTECT(1);
    return(wasError == 0);
}

USER_OBJECT_
getVariableNames(USER_OBJECT_ obj)
{
    USER_OBJECT_ e, val;
    int wasError;
    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, VECTOR_ELT(obj, GET_VARIABLE_NAMES));
    val = R_tryEval(e, R_GlobalEnv, &wasError);
    UNPROTECT(1);

    return(val);
}


gboolean
R_readInputFromDescription(InputDescription *desc, ggobid *gg, GGobiPluginInfo *info)
{
    USER_OBJECT_ obj = (USER_OBJECT_) desc->userData;
    USER_OBJECT_ varNames, e, tmp;
    int nrow, ncol, i, j;
    datad *gdata;
    int wasError;
gboolean init = true;

    if(getDims(&nrow, &ncol, obj) == false) {
	return(false);
    }
    varNames = getVariableNames(obj);
    if(varNames==NULL || varNames == NULL_USER_OBJECT)
	return(false);
    gdata = ggobi_data_new(nrow, ncol);
    gdata->name = g_strdup("rrandom");
    for(j = 0; j < ncol; j++) {
	const char *tmp;
	tmp = CHAR_DEREF(STRING_ELT(varNames, j));
	GGOBI(setVariableName)(j, g_strdup(tmp), false, gdata, gg);
    }


    PROTECT(e = allocVector(LANGSXP,2));
    SETCAR(e, VECTOR_ELT(obj, GET_RECORD));
    SETCAR(CDR(e), tmp = NEW_INTEGER(1));
    for(i = 0 ; i < nrow; i++) {
	USER_OBJECT_ els;
	INTEGER_DATA(tmp)[0] = i+1;
	els = R_tryEval(e, R_GlobalEnv, &wasError);
        for(j = 0; j < ncol; j++)
	    gdata->raw.vals[i][j] = NUMERIC_DATA(els)[j];
    }

    start_ggobi(gg, true, init);

    return(true);
}

/**
  This is called with a filename which may be null and a mode name.
  We then go to R and call the function specified in the getDescription
  value of the plugin's XML definition.  
 */
InputDescription *
R_GetInputDescription(const char * const fileName, const char * const modeName, 
                       ggobid *gg, GGobiPluginInfo *info)
{
    int wasError;
    USER_OBJECT_ obj, e, tmp;
    InputDescription *desc = NULL;

    RPluginData *data = (RPluginData *) info->data;

    PROTECT(e = allocVector(LANGSXP, 5));
    SETCAR(e, Rf_install(data->constructor));
    SETCAR(CDR(e), tmp = NEW_CHARACTER(1));
    if(fileName)
	SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(fileName));
    SETCAR(CDR(CDR(e)), tmp = NEW_CHARACTER(1));
    if(tmp)
	SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(fileName));

    if(info->details->namedArgs) {
        SETCAR(CDR(CDR(CDR(e))), mapHashTable(info->details->namedArgs));
    } else
        SETCAR(CDR(CDR(CDR(e))), R_NilValue);

   SETCAR(CDR(CDR(CDR(CDR(e)))), argsToCharacter(info->details->args));

    obj = R_tryEval(e, R_GlobalEnv, &wasError);
    if(!wasError) {
	desc = g_malloc(sizeof(InputDescription));
	memset(desc, '\0', sizeof(InputDescription));

	R_PreserveObject(obj);
	desc->fileName = g_strdup(data->constructor);
	desc->mode = unknown_data;
        desc->userData = obj;
	desc->desc_read_input = R_readInputFromDescription;
    }
    UNPROTECT(1);

    return(desc);
}


/**
  This routine interprets the XML input (already read) for a plugin that uses this
  R meta-plugin. It patches the DLL name, etc. into the new plugins information so
  that we can ensure that it is loaded, tell whether it the onLoad routine has been 
  run, etc.
  Most importantly, it stores the identifiers for the R code that implements
  the plugin in a special structure and replaces the onCreate routine and 
  onLoad symbol names with the names of special C routines that will be called
  to do this. 
 */
gboolean
R_processPlugin(xmlNodePtr node, GGobiPluginInfo *plugin, GGobiPluginType type, 
                GGobiPluginInfo *langPlugin, GGobiInitInfo *info)
{
    GGobiPluginDetails *details;
    RPluginData *data = (RPluginData *)g_malloc(sizeof(RPluginData));

    memset(data, '\0',sizeof(RPluginData));
    details = plugin->details;
    plugin->data = data;

    data->sourceFile = details->onLoad;


    if(type == GENERAL_PLUGIN) {
	data->constructor = plugin->info.g->onCreate;
	plugin->info.g->onCreate = g_strdup("RCreatePlugin");
	plugin->info.g->onClose = g_strdup("RDestroyPlugin");
	plugin->info.g->onUpdateDisplay = g_strdup("RUpdateDisplayMenu");
    } else {
	data->constructor = plugin->info.i->read_symbol_name; /* getDescription; */
        plugin->info.i->getDescription = g_strdup("R_GetInputDescription");
    }

    details->onLoad = g_strdup("RLoadPlugin");
    details->onUnload = g_strdup("RUnloadPlugin");

    setLanguagePluginInfo(details, "R", info);

    return(true);
}



typedef struct {
    USER_OBJECT_ els;
    USER_OBJECT_ names;
    int  current;
    int max;
} R_HashTableConverter;

static gboolean
collectHashElement(gpointer key, gpointer value, R_HashTableConverter *data)
{
    SET_STRING_ELT(data->names, data->current, COPY_TO_USER_STRING((char *)key));  
    SET_STRING_ELT(data->els, data->current, COPY_TO_USER_STRING((char *)value));  
    data->current++;

    return(true);
}

SEXP
mapHashTable(GHashTable *table)
{
    R_HashTableConverter data;

    if(table == NULL)
	return(R_NilValue); 

    data.max = g_hash_table_size(table);
    if(data.max < 1)
	return(R_NilValue);

    data.current = 0;
    PROTECT(data.els = NEW_CHARACTER(data.max));
    PROTECT(data.names = NEW_CHARACTER(data.max));

    g_hash_table_foreach(table,  (GHFunc) collectHashElement, (gpointer) &data);

    SET_NAMES(data.els, data.names);
    UNPROTECT(2);
    return(data.els);
}


USER_OBJECT_
argsToCharacter(GSList *args)
{
    USER_OBJECT_ ans;
    int i, n;
    GSList *tmp;

    if(args == NULL)
	return(NULL_USER_OBJECT);

    n = g_slist_length(args);
    PROTECT(ans = NEW_CHARACTER(n));
    tmp = args;
    for(i = 0; i < n ; i++) {
	if(tmp->data)
	    SET_STRING_ELT(ans, i, COPY_TO_USER_STRING((char *)tmp->data));
	tmp = tmp->next;
    }

    return(ans);
}


/********************************************************************************

 These functions provide an interface between Rggobi, Rgtk and ggobi itself.
 They are temporarily located here until we figure out the relationship between
 the Rggobi and RGtk packages.

********************************************************************************/

/**
  Get a reference to the Help menu in the menubar.
 */
USER_OBJECT_
RGGobi_getHelpMenu(USER_OBJECT_ sgg)
{
 ggobid *gg = (ggobid *) R_ExternalPtrAddr(sgg);
 GtkItemFactory *factory;
 GtkWidget *help_menu = NULL;

 if(!gg) {
     PROBLEM "Invalid ggobid object"
     ERROR;
 }

  factory = gtk_item_factory_from_path ("<main>");
  help_menu = gtk_item_factory_get_widget (factory, "<main>/Help");   

  return(R_createRef(help_menu, "GtkObject"));
}

/**
  Get the window object (GtkWindow) associated with the given displayd
  object.
 */
USER_OBJECT_
RGGobi_getDisplayWindow(USER_OBJECT_ sdpy)
{
 displayd *dpy;
 dpy = (displayd *) (long) NUMERIC_POINTER(VECTOR_ELT(sdpy, 1))[0];
 if(GGOBI_IS_WINDOW_DISPLAY(dpy)) {
   return(R_createRef(GGOBI_WINDOW_DISPLAY(dpy)->window, "GtkWidget"));
 } else
   return (R_NilValue);
}
