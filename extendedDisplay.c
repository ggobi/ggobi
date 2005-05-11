/* extendedDisplay.c */
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

#include "ggobi.h"
#include "display.h"

const gchar * const
gtk_display_tree_label(displayd *dpy)
{
  GtkGGobiExtendedDisplayClass *klass;
  gchar * const label = "?";
  klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(dpy));
  if(klass->treeLabel)
    return(klass->treeLabel);
  if(klass->tree_label)
    return(klass->tree_label(dpy));
  
  return(label);
}

const gchar * const
gtk_display_title_label(displayd *dpy)
{
	GtkGGobiExtendedDisplayClass *klass;
	extendedDisplayd *edpy;
	gchar * const label = "?";

	edpy = GTK_GGOBI_EXTENDED_DISPLAY(dpy);
	if(edpy->titleLabel) {
		return(edpy->titleLabel);
	}

	klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(dpy));
	if(klass->titleLabel)
		return(klass->treeLabel);
	if(klass->title_label)
		return(klass->title_label(dpy));


	return(label);
}



/********************************/


/*
 Add by type and deduce the class from that.

 Can add be specifying pointers to routines to get the GtkType.
 Also can do this with dlsym() to resolve a routine by name.
 And can also lookup the name of a previously instantiated GtkType.
*/

GSList *ExtendedDisplayTypes = NULL;

int
addDisplayType(GtkType type)
{
    GtkObjectClass *klass;
    if(!gtk_type_is_a(type, GTK_TYPE_GGOBI_EXTENDED_DISPLAY)) {
      g_printerr("%s is not a Gtk type that extends GtkGGobiExtendedDisplay", gtk_type_name(type));
    }
    klass = gtk_type_class(type);
    ExtendedDisplayTypes = g_slist_append(ExtendedDisplayTypes, klass);
    return(g_slist_length(ExtendedDisplayTypes));
}


void
registerDisplayTypes(const GtkTypeLoad * const loaders, int n)
{
   int i;
   GtkType type;

   for(i = 0; i < n; i++) {
     type = loaders[i]();
     addDisplayType(type);
   }
}


