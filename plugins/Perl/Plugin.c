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


/**
  This is called when reading the definition of a plugin that uses
  this Perl language plugin. This must extract the information 
  for the plugin that is particular to Perl and substitute the
  methods for the plugin with C-level routines which GGobi can 
  call to communicate with the Perl object that is the plugin.
  So we extract the name of the Perl module to load when the plugin 
  is first loaded; the name of the class from which to instantiate the plugin;
  the 
 */
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


gboolean
PerlDestroyPlugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    gboolean status = true;
    PerlPluginInstData *instData;
    instData = (PerlPluginInstData *) inst->data;

    fprintf(stderr, "Reference counts: gg = %ld, plugin = %ld, obj = %ld\n",
                    SvREFCNT(instData->ggobiRef),
                    SvREFCNT(instData->pluginInstanceRef),
                    SvREFCNT(instData->perlObj));

    SvREFCNT_dec(instData->ggobiRef);    
    SvREFCNT_dec(instData->pluginInstanceRef);    
    SvREFCNT_dec(instData->perlObj);    

    return(status);
}

/**
 */
gboolean
PerlUpdateDisplayMenu(ggobid *gg, PluginInstance *inst)
{
    gboolean status = true;
    fprintf(stderr, "update display menu\n");
    return(status);
}

/**
  load the Perl module associated with this plugin 
*/
gboolean
PerlLoadPlugin(gboolean initializing, GGobiPluginInfo *plugin)
{
    gboolean status = true;
    PerlPluginData *data = (PerlPluginData *) plugin->data;
    char *cmd;
    char *moduleName;

    moduleName = data->moduleName;

#if 0
    require_pv(moduleName);
#else
    cmd = g_malloc( sizeof(char *) * (strlen(moduleName) + 3 + strlen("require")));
    sprintf(cmd, "require %s;", moduleName);
    (void) eval_pv(cmd, TRUE);
    g_free(cmd);
#endif
    if(SvTRUE(ERRSV)) {
	fprintf(stderr, "Failed to load the perl module %s\n", moduleName);
    }
    return(status);
}

gboolean
PerlUnloadPlugin(gboolean quitting, GGobiPluginInfo *plugin)
{
    gboolean status = true;
    return(status);
}


/**
 This is called when we create an instance of a GGobi plugin implemented via a Perl class
 using the generic Perl language plugin.
 We load the module associated with the plugin.
 Then we create reference objects in Perl to the C-level data structures for
 the ggobid and plugin instances.
 These are held around for the duration of the plugin.
 */
PerlPluginInstData *
initializePerlPlugin(PerlPluginData *details, ggobid *gg, PluginInstance *inst)
{
    PerlPluginInstData *instData;

    instData = (PerlPluginInstData *) malloc(sizeof(PerlPluginInstData));

       /* create the references */
    instData->ggobiRef = createPerlReferenceObject(gg, "GGobi::GGobiRef");
    instData->pluginInstanceRef = createPerlReferenceObject(inst, "GGobi::PluginInstRef");
    return(instData);
}

SV *
createPerlReferenceObject(gpointer ptr, const char *className)
{
    SV *ref = NULL;
    float f = (float) ((long)ptr);
    int n ;
    dSP;

    ENTER ;
    SAVETMPS;
    PUSHMARK(SP);   

      /* Pop the class name of the object we are trying to create
         and the actual address of the C-level object that we are 
         exporting as a reference. */
    XPUSHs(sv_2mortal(newSVpv(className, 0)));
    XPUSHs(sv_2mortal(newSVnv(f)));

    PUTBACK;

    n = call_method("new", G_SCALAR | G_EVAL | G_KEEPERR);
    if(SvTRUE(ERRSV) || n < 1) {
	fprintf(stderr, "Can't create Perl reference object of class %s\n", className);fflush(stderr);
	return(NULL);
    }

    SPAGAIN;
    ref = POPs;

    SvREFCNT_inc(ref);

    PUTBACK;
    FREETMPS;
    LEAVE;

    return(ref);
}

/**
 This creates an instance of the specified Perl plugin.
 It does this by call the new() method for the class specified
 in the onCreate element of the plugin description (in the XML).
 It passes 5 arguments to that method:
  1) the class name
  2) a reference to the C-levle GGobi data structure which the plugin instance serves
  3) a reference to the C-level which acts as a proxy for the plugin instance
  4) a hash of the named arguments for the plugin (given in the XML) 
  5) an array of the unnamed arguments.
 */
gboolean
PerlCreatePlugin(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    PerlPluginData *data = (PerlPluginData *) plugin->data;
    PerlPluginInstData *instData;
    gboolean status = false;

    inst->data = instData = initializePerlPlugin(data, gg, inst);

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
        XPUSHs((newSVpv(data->className, 0)));

        XPUSHs(instData->ggobiRef);
        XPUSHs(instData->pluginInstanceRef);

	XPUSHs(sv_2mortal(namedArgs)); 
	XPUSHs(sv_2mortal(args));

	PUTBACK;

        /* */
        n = call_method("new", G_SCALAR | G_EVAL | G_KEEPERR);
        if(SvTRUE(ERRSV)) {
	    fprintf(stderr, "Error in creating plugin: %s\n", SvPV(ERRSV, PL_na));fflush(stderr);
	    return(false);
	}

	SPAGAIN;
        if(n > 0) {
	    instData->perlObj = POPs;
	    status = true;
	} else
	    status = false;

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
    AV *arr;
    SV *el, *ans;

    if(args == NULL || (n = g_slist_length(args)) == 0)
	return(&sv_undef);

    tmp = args;

    arr = newAV();
    av_extend(arr, n); 

    for(i = 0; i < n; i++) {
	if(tmp->data) {
	    el = newSVpv((char *) tmp->data, 0);
	    SvREFCNT_inc(el);
	    av_push(arr, el);
	}
	tmp = tmp->next;
    }
    ans = newRV_noinc((SV *)arr);

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
    HV *hash;
    SV *ans;
    if(!table || (n = g_hash_table_size(table)) < 1)
	return(&sv_undef);

    hash = newHV();

    g_hash_table_foreach(table, (GHFunc) collectHashElement, (gpointer) hash);

    ans = newRV_noinc((SV*) hash);

    return((SV *) ans);
}

