
#include "testEvents.h"

#include <stdio.h>

/*
  Used to test the selection of variable events.
 */
void
test_variable_select(GtkWidget *w, gint whichVar, datad *d, splotd *sp, ggobid *gg, char *val)
{
  vartabled *vartab = g_slist_nth_data(d->vartable, whichVar);
  fprintf(stderr,
    "Selected variable: %d %s in %s.  User value %s. # plots in display %d\n", 
    whichVar, vartab->collab, d->name, val,
    g_list_length(sp->displayptr->splots));
}

void
test_point_move_cb(char *userData, splotd *sp, GdkEventMotion *ev, ggobid *gg, GtkWidget *w)
{
  fprintf(stderr, "Moving a point\n");fflush(stderr);
}

void
test_brush_motion_cb(char *userData, ggobid *gg, splotd *sp, GdkEventMotion *ev, GtkWidget *w)
{
    fprintf(stderr, "brush motion callback (gg) %p (sp) %p (ev) %p, (userData) %s\n", gg, sp, ev, userData);fflush(stderr);
}

/*
  use gtk_signal_connect_object()
 */
void 
test_new_plot_cb(void *userData, splotd *sp, ggobid *gg, GtkWidget *srcWidget)
{
  fprintf(stderr, "New plot: %s\n", (char *)userData);fflush(stderr);
  gtk_signal_connect_object(GTK_OBJECT(sp->da), "brush_motion", test_brush_motion_cb, (gpointer)"My brushing");
  gtk_signal_connect_object(GTK_OBJECT(sp->da), "move_point", test_point_move_cb, (gpointer)"My moving");
}

/*
  use gtk_signal_connect() rather than ...._object().
 */
void
test_data_add_cb(GtkWidget *w, datad *d,  ggobid *gg, gpointer data)
{
  g_printerr ("(test_data_add_cb) adding datad\n");
  gtk_signal_connect(GTK_OBJECT(gg->main_window), "select_variable", test_variable_select, "My String");
}


void
test_sticky_points(GtkWidget *w, gint index, gint state, datad *d, gpointer data)
{
    fprintf(stderr, "[Sticky point identification] %d %s in %s\n",
	    index, state == STICKY ? "sticky" : "unsticky", d->name);
    fflush(stderr);
}
