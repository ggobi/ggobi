#ifndef GGOBI_TEST_EVENTS_H
#define GGOBI_TEST_EVENTS_H

#include "ggobi.h"

void test_variable_select(GtkWidget *w, gint whichVar, datad *d, splotd *sp, ggobid *gg, char *val);

/*
void test_new_plot_cb(void *userData, GtkWidget *mainWin, splotd *sp, ggobid *gg);
 */
void test_new_plot_cb(void *userData, splotd *sp, ggobid *gg);
void test_brush_motion_cb(char *userData, ggobid *gg, splotd *sp, GdkEventMotion *ev);

void test_data_add_cb(GtkWidget *w, datad *d,  ggobid *gg, gpointer data);
#endif

