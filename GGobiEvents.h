#ifndef GGOBI_EVENTS_H
#define GGOBI_EVENTS_H

typedef void (*datad_added_f)(ggobid *, datad *, void *);
typedef void (*brush_motion_f)(ggobid *, splotd *, GdkEventMotion *ev, datad *, void*);
typedef void (*move_point_f)(ggobid *, splotd *, gint, datad *, void*);
typedef void (*identify_point_f)(ggobid *, splotd *, gint, datad *, void*);
typedef void (*select_variable_f)(ggobid *, datad *, gint, splotd *, void*);
typedef void (*splot_new_f)(ggobid *, splotd *, void*);
typedef void (*variable_added_f)(ggobid *, vartabled*, gint, datad *, void*);
typedef void (*variable_list_changed_f)(ggobid *, datad *, void*);

typedef void (*sticky_point_added_f)(ggobid *, gint, gint, datad *, void*);
typedef void (*sticky_point_removed_f)(ggobid *, gint, gint, datad *, void*);

#define CHECK_EVENT_SIGNATURE(x,y)  static y __check##x##y = &x;

#endif
