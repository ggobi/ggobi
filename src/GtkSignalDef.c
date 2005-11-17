/* GtkSignalDef.c */

#define gtk_object_class_user_signal_new g_signal_new
/* see ggobiClass.c for the new signatures that need to be replicated here. */

#if 0

#error "These need to be updated. Please contact ggobi-core@ggobi.org"

#else
       /*-- If this signal has not been initialized yet, do it now --*/
  if (gtk_signal_lookup ("datad_added", GTK_TYPE_GGOBI) == 0) {
      /* This signal is  f(ggobid *, datad *d, gpointer userData) */
    GGobiSignals[DATAD_ADDED_SIGNAL] =
	g_signal_new ("datad_added", 
        GTK_TYPE_GGOBI, 
        GTK_RUN_LAST | GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_VOID__POINTER,
        GTK_TYPE_NONE, 
        1,
        GTK_TYPE_POINTER);
  }
  if (gtk_signal_lookup ("brush_motion", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[BRUSH_MOTION_SIGNAL] = 
	g_signal_new("brush_motion",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST|GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_VOID__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("move_point", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[POINT_MOVE_SIGNAL] = 
      g_signal_new(	  "move_point",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST|GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_NONE__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("identify_point", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[IDENTIFY_POINT_SIGNAL] = 
      g_signal_new(
        "identify_point",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST|GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_NONE__POINTER_POINTER_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

    /* This should be for a ggobi datad rather than a widget. Make that a
       GtkObject and give it a type. */
  if (gtk_signal_lookup ("select_variable", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_SELECTION_SIGNAL] = 
      g_signal_new(
        "select_variable",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST|GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_VOID__POINTER_INT_POINTER, GTK_TYPE_NONE, 3,
        GTK_TYPE_POINTER, GTK_TYPE_INT, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("splot_new", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[SPLOT_NEW_SIGNAL] = 
      g_signal_new(
        "splot_new",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST|GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
        GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("variable_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_ADDED_SIGNAL] =
      g_signal_new (
        "variable_added",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST | GTK_RUN_ACTION,
	0, NULL, NULL,
        gtk_marshal_NONE__POINTER_POINTER,
        GTK_TYPE_NONE, 2,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }
  if (gtk_signal_lookup ("variable_list_changed", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[VARIABLE_LIST_CHANGED_SIGNAL] =
      g_signal_new ("variable_list_changed",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST | GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_NONE__POINTER_POINTER,
        GTK_TYPE_NONE, 2,
        GTK_TYPE_POINTER, GTK_TYPE_POINTER);
  }

  if (gtk_signal_lookup ("sticky_point_added", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_ADDED_SIGNAL] =
        g_signal_new ("sticky_point_added",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST | GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_NONE__INT_INT_POINTER,
        GTK_TYPE_NONE, 3,
        GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_POINTER);  /* record index and datad pointer **/
  }

  if (gtk_signal_lookup ("sticky_point_removed", GTK_TYPE_GGOBI) == 0) {
    GGobiSignals[STICKY_POINT_REMOVED_SIGNAL] =
      g_signal_new ("sticky_point_removed",
        GTK_TYPE_GGOBI,
        GTK_RUN_LAST | GTK_RUN_ACTION,
        0, NULL, NULL,
        gtk_marshal_NONE__INT_INT_POINTER,
        GTK_TYPE_NONE, 3,
        GTK_TYPE_INT, GTK_TYPE_INT, GTK_TYPE_POINTER);  /* record index and datad pointer **/
  }

#endif
