#include "PerlPlugin.h"

static gboolean readInput(InputDescription *desc, ggobid *gg, GGobiPluginInfo *);

InputDescription*
Perl_GetInputDescription(const char *const fileName, const char *const input, ggobid *gg, GGobiPluginInfo *plugin)
{
    InputDescription *desc;
    PerlPluginInstData *instData;

    desc = (InputDescription *) g_malloc(sizeof(InputDescription));
    desc->fileName = g_strdup("Perl random input plugin");
    instData = initializePerlPlugin(plugin->data, gg, NULL);
    createPlugin(plugin->data, instData, plugin);

    desc->userData = instData;
    desc->desc_read_input = readInput;

    return(desc);
}

datad *createDataset(PerlPluginInstData *data, gint nrow, ggobid *gg);
gint getNumRows(PerlPluginInstData *instData);
gint populateDataset(PerlPluginInstData *data, datad *gdata, gint nrow, ggobid *gg);
char *getDatasetName(PerlPluginInstData *instData);

/**
 This is used for the pull plugins. The push plugins will have already populated the data.
 We get the number of observations, number of variables and the variable names.
 Then we create the datad and then populate it
 */
gboolean
readInput(InputDescription *desc, ggobid *gg, GGobiPluginInfo *info)
{
    PerlPluginInstData *instData;
    gint nrow;
    datad *gdata;
    char *name;

    instData = (PerlPluginInstData *)desc->userData;

    nrow = getNumRows(instData);
    gdata = createDataset(instData, nrow, gg);
    populateDataset(instData, gdata, nrow, gg);

    name = getDatasetName(instData);
    if(!name)
	name = g_strdup("<Unnamed>");
    gdata->name = name;

    start_ggobi(gg, true, true);
    return(true);
}


datad *
createDataset(PerlPluginInstData *instData, gint nrow, ggobid *gg)
{
    datad *gdata;
    gint nvars, j;

    dSP;
    ENTER ;
    SAVETMPS;
    PUSHMARK(SP);   

    XPUSHs(instData->perlObj);
    PUTBACK;
    nvars = call_method("getVariableNames", G_KEEPERR | G_EVAL | G_ARRAY);

    SPAGAIN;
    gdata = datad_create(nrow, nvars, gg);
    for(j = nvars; j > 0; j--) {
      char *varName = POPp;
      GGOBI(setVariableName)(j-1, g_strdup(varName), false, gdata, gg);
    }

    PUTBACK;
    FREETMPS;
    LEAVE;
    return(gdata);
}

gint
populateDataset(PerlPluginInstData *instData, datad *gdata, gint nrow, ggobid *gg)
{
   dSP;
   int i, j, n;

   for(i = 0; i < nrow; i++) {
       ENTER ;
       SAVETMPS;
       PUSHMARK(SP);   

       XPUSHs(instData->perlObj);
       XPUSHs(sv_2mortal(newSViv(i)));
       PUTBACK;
       n = call_method("getRecord", G_KEEPERR | G_EVAL | G_ARRAY);
       if(SvTRUE(ERRSV)) {
	   fprintf(stderr, "Error in populating dataset: %s", SvPV(ERRSV, PL_na));fflush(stderr);
	   return(i);
       }
       SPAGAIN;
       for(j = 0; j < n; j++) {
	   double val = POPn;
#if 0
	   SV *el = POPs;
	   I32 type = SvTYPE(el);
	   val = SvNV(el);
#endif
	   gdata->raw.vals[i][j] = val;
       }

       PUTBACK;
       FREETMPS;
       LEAVE;
   }
    return(i);
}

gint 
getNumRows(PerlPluginInstData *instData)
{
    gint nrow, n;
    dSP;
    ENTER ;
    SAVETMPS;
    PUSHMARK(SP);   

    XPUSHs(instData->perlObj);
    PUTBACK;
    n = call_method("getNumRecords", G_KEEPERR | G_EVAL | G_SCALAR);

    SPAGAIN;
    nrow = POPi;

    PUTBACK;
    FREETMPS;
    LEAVE;
    return(nrow);
}


char *
getDatasetName(PerlPluginInstData *instData)
{
    char *name;
    int n;

    dSP;
    ENTER ;
    SAVETMPS;
    PUSHMARK(SP);   

    XPUSHs(instData->perlObj);
    XPUSHs(sv_2mortal(newSViv(1)));

    PUTBACK;
    n = call_method("getSourceDescription", G_KEEPERR | G_EVAL | G_ARRAY);
    if(n == 0 || SvTRUE(ERRSV)) {
	return(NULL);
    }

    SPAGAIN;
    name = POPp;
    name = g_strdup(name);
    PUTBACK;
    FREETMPS;
    LEAVE;
    return(name);
}
