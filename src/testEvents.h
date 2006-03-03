#ifndef GGOBI_TEST_EVENTS_H
#define GGOBI_TEST_EVENTS_H

#include "ggobi.h"

void test_variable_select(ggobid *gg, GGobiData *d, gint whichVar, splotd *sp, void *);
/*void test_variable_select(GtkWidget *w, gint whichVar, GGobiData *d, splotd *sp, ggobid *gg, char *val); */

/* void test_point_move_cb(void *userData, splotd *sp, GdkEventMotion *ev, ggobid *gg); */
void test_point_move_cb(void *userData, splotd *sp, gint which, GGobiData *d, ggobid *gg);

/*
void test_new_plot_cb(void *userData, GtkWidget *mainWin, splotd *sp, ggobid *gg);
 */
void test_new_plot_cb(void *userData, splotd *sp, ggobid *gg);

#if 1
void test_brush_motion_cb(void *userData, splotd *sp, GdkEventMotion *ev, GGobiData *d, ggobid *gg);
#else
void test_brush_motion_cb(char *userData, ggobid *gg, splotd *sp, GdkEventMotion *ev, GtkWidget *w);
#endif

void test_data_add_cb(ggobid *, GGobiData *d, gpointer data);


void test_sticky_points(ggobid *gg, int index, int state, GGobiData *d, gpointer data);
#endif

