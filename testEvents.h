#ifndef GGOBI_TEST_EVENTS_H
#define GGOBI_TEST_EVENTS_H

#include "ggobi.h"

void test_variable_select(GtkWidget *w, gint whichVar, datad *d, splotd *sp, ggobid *gg, char *val);

void test_point_move_cb(char *userData, splotd *sp, GdkEventMotion *ev, ggobid *gg, GtkWidget *w);

/*
void test_new_plot_cb(void *userData, GtkWidget *mainWin, splotd *sp, ggobid *gg);
 */
void test_new_plot_cb(void *userData, splotd *sp, ggobid *gg, GtkWidget *);
void test_brush_motion_cb(char *userData, ggobid *gg, splotd *sp, GdkEventMotion *ev, GtkWidget *w);

void test_data_add_cb(GtkWidget *w, datad *d,  ggobid *gg, gpointer data);


void test_sticky_points(GtkWidget *w, int index, int state, datad *d, gpointer data);
#endif

