/* GGobiEvents.h */
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

#ifndef GGOBI_EVENTS_H
#define GGOBI_EVENTS_H

/*
 The CHECK_EVENT_SIGNATURE is for testing routines used with g_signal_connect.
 The CHECK_R_EVENT_SIGNATURE is for testing routines used with g_signal_connect_object.

 Similarly, the ggev_r_... are for the routines used with g_signal_connect_object.
*/

typedef void (*ggev_datad_added_f)(ggobid *, GGobiStage *, void *);
typedef void (*ggev_brush_motion_f)(ggobid *, splotd *, GdkEventMotion *ev, GGobiStage *, void*);
typedef void (*ggev_r_brush_motion_f)(void *,  splotd *, GdkEventMotion *ev, GGobiStage *, ggobid *);

typedef void (*ggev_move_point_f)(ggobid *, splotd *, gint, GGobiStage *, void*);
typedef void (*ggev_r_move_point_f)(void*, splotd *, gint, GGobiStage *, ggobid *);

typedef void (*ggev_identify_point_f)(ggobid *, splotd *, gint, GGobiStage *, void*);
typedef void (*ggev_select_variable_f)(ggobid *, GGobiStage *, gint, splotd *, void*);

typedef void (*ggev_splot_new_f)(ggobid *, splotd *, void*);
typedef void (*ggev_r_splot_new_f)(void *, splotd *, ggobid *);

typedef void (*ggev_variable_added_f)(ggobid *, gint, GGobiStage *, void*);
typedef void (*ggev_variable_list_changed_f)(ggobid *, GGobiStage *, void*);

typedef void (*ggev_sticky_point_added_f)(ggobid *, gint, gint, GGobiStage *, void*);
typedef void (*ggev_sticky_point_removed_f)(ggobid *, gint, gint, GGobiStage *, void*);

typedef void (*ggev_clusters_changed_f)(ggobid *, GGobiStage *, void *);

#ifdef CHECK_EVENT_SIGNATURES
#ifdef __GNUC__
#define UNUSED_ATTR  __attribute__ ((unused))
#else
#define UNUSED_ATTR
#endif

#define CHECK_EVENT_SIGNATURE(x,y)  static ggev_##y __check ## x ## y UNUSED_ATTR = & x;
#define CHECK_R_EVENT_SIGNATURE(x,y)  static ggev_r_##y __check ## x ## y UNUSED_ATTR = & x;
#else
#define CHECK_EVENT_SIGNATURE(x,y) 
#endif


#endif
