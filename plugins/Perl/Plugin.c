#include <EXTERN.h>
#include <perl.h>


#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif


#ifdef NORMAL
#undef NORMAL
#endif

#include "plugin.h"


SV *Perl_getUnnamedArguments(GSList *args);
SV *Perl_getNamedArguments(GHashTable *table);

SV * createPerlReferenceObject(gpointer ptr, const char *className);


typedef struct {
    gchar *moduleName; /* Will load this when needed */
    gchar *className; /* will call new() for that class. */
}  PerlPluginData;

typedef struct {
    SV *pluginInstanceRef;
    SV *ggobiRef;
    SV *perlObj;
} PerlPluginInstData;


void xs_init _((void));

/**
 This gets called when the Perl language plugin is loaded.
 It initializes a perl interpreter and gets on with things.

 It should look for a perl class name or a piece of code to run
 and potentially pass the command line arguments to it.
 We'll determine a syntax for that soon.

 */
gboolean
Perl_onLoad(gboolean initializing, GGobiPluginInfo *plugin)
{
 int argc;
 char **argv;
 void (*lxs_init)(void);
 PerlInterpreter  *interpreter;

 lxs_init = NULL; // xs_init;

   argc = 3;
   argv = (char **) g_malloc(sizeof(char *) * argc);
   argv[0] = "GGobiPerlPlugin";
   argv[1] = "-e";
   argv[2]= "0";

#if 0
   if(pluginfo->details->namedArgs) {
      init = (char *) g_hash_table_lookup(pluginfo->details->namedArgs, "init");
   }
#endif

   interpreter = perl_alloc();
   perl_construct(interpreter);

   perl_parse(interpreter, lxs_init, argc, argv, (char **)NULL);
   perl_run(interpreter);

   return(true);
}


gboolean
Perl_processPlugin(xmlNodePtr node, GGobiPluginInfo *plugin, GGobiPluginType type,
                    GGobiPluginInfo *langPlugin, GGobiInitInfo *info)
{
    GGobiPluginDetails *details;
    PerlPluginData *data = (PerlPluginData *)g_malloc(sizeof(PerlPluginData));

    memset(data, '\0',sizeof(PerlPluginData));
    details = plugin->details;
    plugin->data = data;

    data->moduleName = details->onLoad;

    if(type == GENERAL_PLUGIN) {
	data->className = plugin->info.g->onCreate;
         /* We should be able to specify the pointers directly. This is stupid! */
	plugin->info.g->onCreate = g_strdup("PerlCreatePlugin");
	plugin->info.g->onClose = g_strdup("PerlDestroyPlugin");
	plugin->info.g->onUpdateDisplay = g_strdup("PerlUpdateDisplayMenu");
    } else {
	data->className = plugin->info.i->read_symbol_name; /* getDescription; */
        plugin->info.i->getDescription = g_strdup("Perl_GetInputDescription");
    }

    details->onLoad = g_strdup("PerlLoadPlugin");
    details->onUnload = g_strdup("PerlUnloadPlugin");

    setLanguagePluginInfo(details, "Perl", info);

    return(true);
}

PerlPluginInstData *
initializePerlPlugin(PerlPluginData *details, ggobid *gg, PluginInstance *inst)
{
    PerlPluginInstData *instData;
    char *cmd;
    char *moduleName = details->moduleName;

    instData = (PerlPluginInstData *) malloc(sizeof(PerlPluginInstData));

    /* load the Perl module, create the ggobi and plugin instance references
       as Perl objects. */

    cmd = g_malloc( sizeof(char *) * (strlen(moduleName) + 3 + strlen("require")));
    sprintf(cmd, "require %s;", moduleName);
    (void) eval_pv(cmd, TRUE);
    if(SvTRUE(ERRSV)) {
	fprintf(stderr, "Failed to load the perl module %s\n", moduleName);
    }
    g_free(cmd);

       /* create the references */
    instData->ggobiRef = createPerlReferenceObject(gg, "GGobi::GGobiRef");
    instData->pluginInstanceRef = createPerlReferenceObject(inst, "GGobi::PluginInstRef");

    return(instData);
}

SV *
createPerlReferenceObject(gpointer ptr, const char *className)
{
    SV *ref = NULL;

    dSP;
    ENTER ;
    SAVETMPS;
    PUSHMARK(SP);   

    XPUSHs(sv_2mortal(newSVpv(className, 0)));

    PUTBACK;
    call_method("new", G_SCALAR | G_EVAL | G_KEEPERR);
    if(SvTRUE(ERRSV)) {
	fprintf(stderr, "Can't create Perl reference object of class %s\n", className);fflush(stderr);
	return(NULL);
    }
    SPAGAIN;
    ref = POPs;
    PUTBACK;
    FREETMPS;
    LEAVE;

    return(ref);
}

/**

 */
gboolean
PerlCreatePlugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    PerlPluginData *data = (PerlPluginData *) plugin->data;
    PerlPluginInstData *instData;
    gboolean status = false;
    char *className = inst->info->info.g->onCreate;  

    inst->data = initializePerlPlugin(data, gg, inst);

    if(data->className) {
	SV *args, *namedArgs;
	int n;

	dSP;

        args = Perl_getUnnamedArguments(plugin->details->args);
        namedArgs = Perl_getNamedArguments(plugin->details->namedArgs);

	ENTER ;
	SAVETMPS;
	PUSHMARK(SP);

  	/* Pop the different arguments onto the stack.
           reference to the ggobi instance,
           reference to the plugin instance
           named arguments
           un-named arguments
         */
        XPUSHs(sv_2mortal(newSVpv(data->className, 0)));
        XPUSHs(instData->ggobiRef);
        XPUSHs(instData->pluginInstanceRef);
	XPUSHs(sv_2mortal(namedArgs));
	XPUSHs(sv_2mortal(args));
	PUTBACK;
        /* */
        n = call_method("new", G_SCALAR | G_EVAL | G_KEEPERR);
        if(SvTRUE(ERRSV)) {
	    return(false);
	}

	SPAGAIN;
        if(n > 0) {
	    instData->perlObj = POPs;
	} else
	    status = true;

	PUTBACK;
	FREETMPS;
	LEAVE;
    }

    return(status);
}

SV *
Perl_getUnnamedArguments(GSList *args)
{
    int n, i;
    GSList *tmp;
    AV *ans;
    SV *el;

    if(args == NULL || (n = g_slist_length(args)) == 0)
	return(&sv_undef);

    tmp = args;
    ans = newAV();
    av_extend(ans, n);

    for(i = 0; i < n; i++) {
	if(tmp->data) {
	    el = newSVpv((char *) tmp->data, 0);
	    SvREFCNT_inc(el);
	    av_push(ans, el);
	}
	tmp = tmp->next;
    }

    return((SV *) ans);
}

typedef struct {
    SV *table;
} Perl_HashTableConverter;


void
collectHashElement(gpointer gkey, gpointer gvalue, HV *table)
{
    char *key;
    SV *el;
    key = g_strdup((char *) gkey); /* Who frees this? */
    el = newSVpv((char *) gvalue, 0);
    SvREFCNT_inc(el);

    hv_store(table, key, strlen(key), el, 0);
}

SV *
Perl_getNamedArguments(GHashTable *table)
{
    int n;
    HV *ans;

    if(!table || (n = g_hash_table_size(table)) < 1)
	return(&sv_undef);

    ans = newHV();

    g_hash_table_foreach(table, (GHFunc) collectHashElement, (gpointer) ans);

    return((SV *) ans);
}


gboolean
PerlDestroyPlugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    gboolean status = true;
    return(status);
}

gboolean
PerlUpdateDisplayMenu(ggobid *gg, PluginInstance *inst)
{
    gboolean status = true;
    return(status);
}

gboolean
PerlLoadPlugin(gboolean initializing, GGobiPluginInfo *plugin)
{
    gboolean status = true;
    return(status);
}

gboolean
PerlUnloadPlugin(gboolean quitting, GGobiPluginInfo *plugin)
{
    gboolean status = true;
    return(status);
}
