#ifdef XGOBIINTERN
#define XGOBI_
#else
#define XGOBI_ extern
#endif

#ifndef DEFINES_H
#include "defines.h"
#endif

#ifndef XGOBI_TYPES_H
#include "types.h"
#endif

#ifndef XGOBI_H
#include "ggobi.h"
#endif

#ifndef DISPLAY_H
#include "display.h"
#endif

#ifndef __GTK_EXT_HRULER_H__
#include "gtkexthruler.h"
#endif
#ifndef __GTK_EXT_VRULER_H__
#include "gtkextvruler.h"
#endif
#ifndef __GTK_EXT_RULER_H__
#include "gtkextruler.h"
#endif

XGOBI_ GtkWidget *control_panel[NMODES];

XGOBI_ GdkGC *plot_GC;
XGOBI_ GdkGC *selvarfg_GC, *selvarbg_GC;     /* white background, thick lines */
XGOBI_ GdkGC *unselvarfg_GC, *unselvarbg_GC; /* grey background, thin lines */

XGOBI_ xgobid xg;
XGOBI_ GList *displays;
XGOBI_ displayd *current_display;

       /* The splot which has the mouse and keyboard focus */
XGOBI_ splotd *current_splot; 

XGOBI_ icoords mousepos, mousepos_o;

XGOBI_ gboolean mono_p;
