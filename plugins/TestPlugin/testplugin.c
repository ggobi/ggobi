
#include <stdio.h>
#include <gtk/gtk.h>
#include "plugin.h"

gboolean
init(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    fprintf(stderr, "initializing test plugin instance\n");fflush(stderr);
    return(true);
}

gboolean
close(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
    fprintf(stderr, "closing the test plugin instance\n");fflush(stderr);
    return(true);
}

gboolean
load(gboolean initializing, GGobiPluginInfo *plugin)
{
    fprintf(stderr, "loading the test plugin\n");fflush(stderr);
    return(true);
}

gboolean
unload(gboolean initializing, GGobiPluginInfo *plugin)
{
    fprintf(stderr, "unloading the test plugin\n");fflush(stderr);
    return(true);
}


gboolean
updateDisplay(ggobid *gg, PluginInstance *plugin)
{
    fprintf(stderr, "updating displays for the test plugin\n");fflush(stderr);
    return(true);
}
