#include "ggobi.h"
#include "display.h"

const gchar * const
gtk_display_tree_label(displayd *dpy)
{
  GtkGGobiExtendedDisplayClass *klass;
  gchar * const label = "?";
  klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(dpy)->klass);
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
  klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(dpy)->klass);
  if(klass->titleLabel)
    return(klass->treeLabel);
  if(klass->title_label)
    return(klass->title_label(dpy));
  
  edpy = GTK_GGOBI_EXTENDED_DISPLAY(dpy);
  if(edpy->titleLabel) {
    return(edpy->titleLabel);
  }
  
  return(label);
}



/********************************/


/*
 Add by type and deduce the class from that.

 Can add be specifying pointers to routines to get the GtkType.
 Also can do this with dlsym() to resolve a routine by name.
 And can also lookup the name of a previously instantiated GtkType.
*/

GSList *ExtendedDisplayTypes;

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


