#include "RSCommon.h"
#include "REval.h"

#include "plugin.h"

gboolean initR(GGobiPluginInfo *pluginfo);
SEXP Rg_evalCmd(const char *cmd);

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

    val = tryEval(e, &errorOccurred);
    if(errorOccurred) {
	fprintf(stderr, "Cannot parse the command\n");fflush(stderr);
        UNPROTECT(1);
	return(NULL_USER_OBJECT);
    }
    PROTECT(val);
    SETCAR(e, Rf_install("eval"));
    SETCAR(CDR(e), val);
    SET_TAG(CDR(e), NULL_USER_OBJECT);
    val = tryEval(e, &errorOccurred);
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
  int ok = 0;
  int errorOccurred = 0;

 if(canRead(name)) {
      SEXP tmp, val, e;
    /* These are static methods so cannot use them:
       R_LoadProfile(fp, R_NilValue);  
       R_ReplFile(fp, R_NilValue, 0, 0);
     */

  PROTECT(e = allocVector(LANGSXP, 2));
  PROTECT(val = Rf_findFun(Rf_install("source"), R_GlobalEnv));
  SETCAR(e, val);
  PROTECT(tmp = NEW_CHARACTER(1));
  SET_STRING_ELT(tmp, 0, COPY_TO_USER_STRING(name));
  SETCAR(CDR(e), tmp);
  val = tryEval(e, &errorOccurred);
    ok = TRUE;

  UNPROTECT(3);
 }

  return(ok);
}


gboolean
RLoadPlugin(gboolean initializing, GGobiPluginInfo *plugin)
{
    RPluginData *data = (RPluginData *) plugin->data;
    fprintf(stderr, "Loading R plugin %s %s\n", data->sourceFile, data->constructor);fflush(stderr);
    if(data->sourceFile)
	Rsource(data->sourceFile);
    return(true);
}


USER_OBJECT_
R_createRef(void *ptr, const char *className)
{
    USER_OBJECT_ val;
    USER_OBJECT_ klass;

    PROTECT(val = R_MakeExternalPtr(ptr, Rf_install((char *) className), NULL));
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
	USER_OBJECT_ e;
        /* Construct a call of the form constructor(gg, inst) */
        PROTECT(e = allocVector(LANGSXP, 3));
	SETCAR(e, Rf_install((char *) data->constructor));
        SETCAR(CDR(e), R_ggobiRef(gg));    
        SETCAR(CDR(CDR(e)), R_ggobiPluginInstRef(inst));    

	obj = tryEval(e, &wasError);

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

    obj = d->pluginObject;
    if(!isFunction(VECTOR_ELT(obj, 0))) {
	return(false);
    }

    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, VECTOR_ELT(obj, 0));
    tryEval(e, NULL);
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

    obj = d->pluginObject;
    if(Rf_length(obj) < 2 || !isFunction(VECTOR_ELT(obj, 1)))
	return(false);

    PROTECT(e = allocVector(LANGSXP, 1));
    SETCAR(e, VECTOR_ELT(obj, 1));
    tryEval(e, NULL);
    UNPROTECT(1);

    return(true);
}


gboolean
RUnloadPlugin(gboolean quitting, GGobiPluginInfo *plugin)
{
    return(true);
}

