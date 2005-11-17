/*      tvpanel.h    */

typedef struct varrectd {
  gint jvar;              /*-- don't know if this is useful or not --*/
  GtkWidget *box, *da, *label;
  GdkPixmap *pix;
};
typedef struct varcircled {
  gint jvar;
  GtkWidget *box, *table, *da_x, *da_y, *da_circ, *label;
  GdkPixmap *pix_x, *pix_y, *pix_circ;
};

  GdkCursor *tour_cursor;
  gint jtour_cursor;

  struct _Varpanel_tour1d {
    /* GtkWidget *vbox, *swin, *vb, *button_box, *manip_btn; */
    gint nvars, *vars;
    GSList *varrects;  /*-- freed when mode changes --*/
  } vp_tour1d;

  struct _Varpanel_tour2d {
    /* GtkWidget *vbox, *swin, *vb, *button_box, *manip_btn; */
    gint nvars, *vars;
    GSList *varcircles;  /*-- freed when mode changes --*/
  } vp_tour2d;

  struct _Varpanel_tourcorr {
    /* GtkWidget *vpane; */
    /* GtkWidget  *xvbox, *xswin, *xvb, *xbutton_box, *xmanip_btn; */
    /* GtkWidget  *yvbox, *yswin, *yvb, *ybutton_box, *ymanip_btn; */
    gint nxvars, *xvars_p;
    GSList *xvarrects;
    gint nyvars, *yvars_p;
    GSList *yvarrects;
  } vp_tourcorr;

  struct _Varpanel_rotate {
    GtkWidget *vbox;
    gint nvars /* = 3 */, *vars, *vars_p;
    GSList *varcircles;
  } vp_rotate;

/* end of tvpanel.h */

/*----------------- functions -------------------------------------------*/

/*-------------------- tour1d -------------------------------------------*/

/*-- reset the variables associated with each varrect --*/
tour1d_varrect_retune (display, gg) {
  gint j;
  GList *l, *children;
  varrectd *r;
  GtkWidget *w, *rectholder;  /*-- the container of the varrects; a vbox --*/
  
  rectholder = widget_find_by_name (gg->varpanel_ui.window,
    "TOUR1D:rectholder");

  /*--- Reset the jvar element of each varrectd struct  --*/
  children = (GSList *) gtk_container_get_children (GTK_CONTAINER (rectholder));
  for (l = children, j = 0; l; l=l->next, j++) {
    w = (GtkWidget *) l->data;
    r = (rectdatad *) g_object_get_data(G_OBJECT (w), "rectdata");
    r->jvar = display->vp_tour1d.vars[j];
  }
}

tour1d_varrect_add (display, gg) {
  varrectd *varrect;
  GtkWidget *rectholder;  /*-- the container of the varrects; a vbox --*/
  
  rectholder = widget_find_by_name (gg->varpanel_ui.window,
    "TOUR1D:rectholder");

  varbox = varrect_create ();

  /*-- add the varrect structure to the list of boxes --*/
  /*-- in the display structure? --*/
  display->vp_tour1d.varrects = g_slist_add (display->vp_tour1d.varrects,
    varrect);

  /*-- add the rectangle's enclosing container to the parent --*/
  gtk_box_pack_start (GTK_BOX (rectholder), varrect->box, false, false, 1);

  tour1d_varrect_retune (display, gg);
}

varrect_destroy (varrectd *r) {
  gdk_pixmap_unref (r->pix);
  gtk_widget_destroy (r->box);
}

tour1d_varrect_remove (jvar) {
  gint j;
  GList *l, *children;
  varrectd *r;
  GtkWidget *w, *rectholder;  /*-- the container of the varrects; a vbox --*/
  
  rectholder = widget_find_by_name (gg->varpanel_ui.window,
    "TOUR1D:rectholder");

  /*--- Find the varrectd struct associated with jvar --*/
  children = (GList *) gtk_container_get_children (GTK_CONTAINER (rectholder));
  for (l = children, j = 0; l; l=l->next, j++) {
    w = (GtkWidget *) l->data;
    r = (rectdatad *) g_object_get_data(G_OBJECT (w), "rectdata");
    if (r->jvar == jvar) {
      g_list_remove (r);
      varrect_destroy (r);
    }
  }
  tour1d_varrect_retune (display, gg);
}


/*
 * rename the current tour1d_varsel as tour1d_varrect_select?
*/

/*-- add a variable to the set of variables available for touring --*/
/*-- or remove a variable from the set of variables available for touring --*/
tour1d_varsel (gint jvar) {
  gint *nvars = &display->vp_tour1d.nvars;
  gint *vars = &display->vp_tour1d.vars;

  if (variable_in_set (jvar, *nvars, *vars)) {
    remove_from_variable_set (jvar, *nvars, *vars);
    remove_from_active_vars (jvar, );
    tour1d_varrect_remove (jvar);
  } else {
    add_to_variable_set (jvar, display->vp_tour1d.vars);
    add_to_active_vars (jvar, );
    tour1d_varrect_add ();
  }
}




/*-----------------------------------------------------------------------*/

tourcorr_varbox_add () {
}
tourcorr_varbox_remove () {
}

tour2d_varcircle_add () {
}
tour2d_varcircle_remove () {
}

