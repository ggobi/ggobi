#ifndef GGOBI_TEST_EVENTS_H
#define GGOBI_TEST_EVENTS_H

#include "ggobi.h"

void test_variable_select(GGobiSession *gg, GGobiStage *d, gint whichVar, splotd *sp, void *);
/*void test_variable_select(GtkWidget *w, gint whichVar, GGobiStage *d, splotd *sp, GGobiSession *gg, char *val); */

/* void test_point_move_cb(void *userData, splotd *sp, GdkEventMotion *ev, GGobiSession *gg); */
void test_point_move_cb(void *userData, splotd *sp, gint which, GGobiStage *d, GGobiSession *gg);

/*
void test_new_plot_cb(void *userData, GtkWidget *mainWin, splotd *sp, GGobiSession *gg);
 */
void test_new_plot_cb(void *userData, splotd *sp, GGobiSession *gg);

#if 1
void test_brush_motion_cb(void *userData, splotd *sp, GdkEventMotion *ev, GGobiStage *d, GGobiSession *gg);
#else
void test_brush_motion_cb(char *userData, GGobiSession *gg, splotd *sp, GdkEventMotion *ev, GtkWidget *w);
#endif

void test_data_add_cb(GGobiSession *, GGobiStage *d, gpointer data);


void test_sticky_points(GGobiSession *gg, int index, int state, GGobiStage *d, gpointer data);
#endif

