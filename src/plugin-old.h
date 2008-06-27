/* plugin.h */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#ifndef GGOBI_PLUGIN_H
#define GGOBI_PLUGIN_H

#include <libxml/tree.h>

#include "session.h"
#include "plugin.h"

gboolean registerPlugins(GGobiSession *gg, GList *plugins);
GtkWidget * showPluginInfo (GList * plugins, GGobiSession * gg);
void closePlugins(GGobiSession *gg);

void registerDefaultPlugins(GGobiInitInfo *info);

#endif
