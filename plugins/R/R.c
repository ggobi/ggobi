#include "RSCommon.h"
#include "REval.h"

#include "plugin.h"

gboolean initR(GGobiPluginInfo *pluginfo);
SEXP Rg_evalCmd(const char *cmd);

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

  init = (char *) g_hash_table_lookup(pluginfo->details->namedArgs, "init");
  if(init && init[0]) {
     (void) Rg_evalCmd(init);
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
        UNPROTECT(1);
	return(NULL_USER_OBJECT);
    }
    PROTECT(val);
    SETCAR(e, Rf_install("eval"));
    SETCAR(CDR(e), val);
    SET_TAG(CDR(e), NULL_USER_OBJECT);
    val = tryEval(e, &errorOccurred);
 
    UNPROTECT(2);
    return(val);  
}

Rboolean
Rsource(const char *name)
{
  int ok = 0;

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
  val = eval(e, R_NilValue);
    ok = TRUE;

  UNPROTECT(3);
 }

  return(ok);
}
