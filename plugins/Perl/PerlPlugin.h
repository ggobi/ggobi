#ifndef GGOBI_PERL_PLUGIN_H
#define GGOBI_PERL_PLUGIN_H


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

#include "GGobiAPI.h"
#include "plugin.h"


typedef struct {
    gchar *moduleName; /* Will load this when needed */
    gchar *className; /* will call new() for that class. */
}  PerlPluginData;

typedef struct {
    SV *pluginInstanceRef;
    SV *ggobiRef;
    SV *perlObj;
} PerlPluginInstData;

gboolean createPlugin(PerlPluginData *data, PerlPluginInstData *instData, GGobiPluginInfo *plugin);
PerlPluginInstData *initializePerlPlugin(PerlPluginData *details, ggobid *gg, PluginInstance *inst);


InputDescription* Perl_GetInputDescription(const char *const fileName, const char *const input, ggobid *gg, GGobiPluginInfo *plugin);

/* ConvertUtils */
SV *Perl_getUnnamedArguments(GSList *args);
SV *Perl_getNamedArguments(GHashTable *table);

#endif

