#include "ggobi.h"
#include "display.h"

gchar * const
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
