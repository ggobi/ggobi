#ifndef GGOBI_EVENTS_H
#define GGOBI_EVENTS_H

typedef void (*ggev_datad_added_f)(ggobid *, datad *, void *);
typedef void (*ggev_brush_motion_f)(ggobid *, splotd *, GdkEventMotion *ev, datad *, void*);
typedef void (*ggev_move_point_f)(ggobid *, splotd *, gint, datad *, void*);
typedef void (*ggev_identify_point_f)(ggobid *, splotd *, gint, datad *, void*);
typedef void (*ggev_select_variable_f)(ggobid *, datad *, gint, splotd *, void*);
typedef void (*ggev_splot_new_f)(ggobid *, splotd *, void*);
typedef void (*ggev_variable_added_f)(ggobid *, vartabled*, gint, datad *, void*);
typedef void (*ggev_variable_list_changed_f)(ggobid *, datad *, void*);

typedef void (*ggev_sticky_point_added_f)(ggobid *, gint, gint, datad *, void*);
typedef void (*ggev_sticky_point_removed_f)(ggobid *, gint, gint, datad *, void*);

typedef void (*ggev_clusters_changed_f)(ggobid *, datad *, void *);

#ifdef CHECK_EVENT_SIGNATURES
#define CHECK_EVENT_SIGNATURE(x,y)  static ggev_#y __check##x##y = &x;
#else
#define CHECK_EVENT_SIGNATURE(x,y) 
#endif


#endif
