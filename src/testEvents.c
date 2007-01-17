
#include "testEvents.h"

#include <stdio.h>

/*
  Used to test the selection of variable events.
 */
void
test_variable_select (GGobiSession * gg, GGobiStage * d, gint whichVar, splotd * sp,
                      void *val)
{
  GGobiVariable *vartab = g_slist_nth_data (d->vartable, whichVar);
  fprintf (stderr,
           "Selected variable: %d %s in %s.  User value %s. # plots in display %d\n",
           whichVar, ggobi_stage_get_col_name(d, whichVar), d->name, (char *) val,
           g_list_length (sp->displayptr->splots));
}

void
test_point_move_cb (void *userData, splotd * sp, gint which, GGobiStage * d,
                    GGobiSession * gg)
{
  fprintf (stderr, "Moving a point\n");
  fflush (stderr);
}

void
test_brush_motion_cb (void *userData, splotd * sp, GdkEventMotion * ev,
                      GGobiStage * d, GGobiSession * gg)
{
  fprintf (stderr,
           "brush motion callback (gg) %p (sp) %p (ev) %p, (userData) %s\n",
           (void *) gg, (void *) sp, (void *) ev, (char *) userData);
  fflush (stderr);
}


/*
  use g_signal_connect_swapped()
 */
void
test_new_plot_cb (void *userData, splotd * sp, GGobiSession * gg)
{
  fprintf (stderr, "New plot: %s\n", (char *) userData);
  fflush (stderr);
#if 1
  g_signal_connect_swapped (G_OBJECT (gg), "brush_motion",
                            G_CALLBACK (test_brush_motion_cb),
                            (gpointer) "My brushing");
  g_signal_connect_swapped (G_OBJECT (gg), "move_point",
                            G_CALLBACK (test_point_move_cb),
                            (gpointer) "My moving");
#else
  g_signal_connect_swapped (G_OBJECT (sp->da), "brush_motion",
                            G_CALLBACK (test_brush_motion_cb),
                            (gpointer) "My brushing");
  g_signal_connect_swapped (G_OBJECT (sp->da), "move_point",
                            test_point_move_cb, (gpointer) "My moving");
#endif
}

/* Raises warning because of the char * for the last parameter not being a void *. */
CHECK_EVENT_SIGNATURE (test_variable_select, select_variable_f)

/*
  use g_signal_connect() rather than ...._object().
 */
     void test_data_add_cb (GGobiSession * gg, GGobiStage * d, gpointer data)
{
  g_printerr ("(test_data_add_cb) adding datad\n");
  g_signal_connect (G_OBJECT (gg), "select_variable",
                    G_CALLBACK (test_variable_select), "My String");
}


void
test_sticky_points (GGobiSession * gg, gint index, gint state, GGobiStage * d,
                    gpointer data)
{
  fprintf (stderr, "[Sticky point identification] %d %s in %s\n",
           index, state == STICKY ? "sticky" : "unsticky", d->name);
  fflush (stderr);
}

CHECK_EVENT_SIGNATURE (test_data_add_cb, datad_added_f)

/* Warning is because we connect this with the order of the first and last arguments reversed. */
  CHECK_R_EVENT_SIGNATURE (test_brush_motion_cb, brush_motion_f)
  CHECK_R_EVENT_SIGNATURE (test_point_move_cb, move_point_f)

  CHECK_R_EVENT_SIGNATURE (test_new_plot_cb, splot_new_f)
  CHECK_EVENT_SIGNATURE (test_sticky_points, sticky_point_added_f)
  CHECK_EVENT_SIGNATURE (test_sticky_points, sticky_point_removed_f)
