#include <ggobi.h>

#include <display.h>
#include <GGobiAPI.h>

#include <scatmatClass.h>
#include <scatterplotClass.h>
#include <parcoordsClass.h>
#include <tsdisplay.h>
#include <barchartDisplay.h>

#include <externs.h>

displayd *createWindowlessDisplay(GtkType type, datad *d, ggobid *gg);

/**
 This example is intended to illustrate how we can create our own
 layouts of displayd's and splotd's in GGobi using Gtk
 to manage the widgets in a different way.
 This is also used to test the facilities for creating 
 "stand-alone" displayd's and splotd's

 This arranges a collection of different GGobi displays 
 in a single window.   These displays would ordinarily 
 be displayed in separate windows in the regular GGobi.

 We start by creating a scatterplot matrix and
 putting that in its own window.

*/

/*
 Sort out active display "Bug?  I see no active display"
 Which displayd is active. Which splotd?

 Check setting the viewmodes by clicking on plots, etc.

 For the barchart, one of the rulersis too small.
 ** CRITICAL **: file gtkextruler.c: line 422 (gtk_ext_ruler_calc_scale): assertion `dx > 0' failed.
 Why does this appear. Doesn't in stand-alone ggobi.
*/

/*
 Callback when the user presses the Reset button at the top.
*/
void
set_active_display(GtkWidget *btn, displayd *dpy)
{
/*	display_tree_display_child_select(NULL, dpy); 
    almost does what we want but there is an unecessary 
    check that the tree is active.
*/

      splotd *sp;
      display_set_current(dpy, dpy->ggobi); 
      sp = (splotd *) g_list_nth_data (dpy->splots, 0);
      GGOBI(splot_set_current_full) (dpy, sp, dpy->ggobi);
}

int
main(int argc, char *argv[])
{
 ggobid *gg;
 GtkWidget *win, *box, *box2, *pane;
 GtkWidget *btn;

 displayd *dpy;
 datad *data;
 gint indices[] = {0, 2, 1, 3};
 gint *rows, *cols;

 GGOBI(main(argc, argv, false));

 gg = GGobi_ggobi_get(0);

 if(g_slist_length(gg->d) == 0) {
    g_printerr("Need a dataset!\n");
    return(1);
 }

 data = (datad *) gg->d->data;

   /* Create a window for our customized display */
 win = gtk_window_new(GTK_WINDOW_TOPLEVEL);

   /* And a box to contain it and any other widgets such as a label. */
 box = gtk_vbox_new(false, 1); 
 btn = gtk_button_new_with_label("Reset");

 box2 = gtk_hpaned_new();

   /* Now create our scatterplot matrix. We create a 3 x 2 matrix of plots. */
 rows = cols = &indices[0];
 dpy = createWindowlessDisplay(GTK_TYPE_GGOBI_SCATMAT_DISPLAY, data, gg);

   /* Now fill in the display.*/
 scatmat_new(dpy, false, 3, rows, 2, cols,  data, gg);
 gtk_widget_set_usize(GTK_WIDGET(dpy), 200, 150); 

 /* Now put the GUI together. */
 gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(dpy)); 


/********* Put a regular scatterplot *********/


 dpy = createWindowlessDisplay(GTK_TYPE_GGOBI_SCATTERPLOT_DISPLAY, data, gg);
 createScatterplot(dpy, false, NULL, 4, rows, data, gg);
 gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(dpy)); 


 gtk_container_add(GTK_CONTAINER(box), btn);
 gtk_signal_connect(GTK_OBJECT(btn), "clicked", set_active_display, dpy);


 gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(box2)); 
 gtk_widget_show_all(box2);


 /* Now the lower row consisting of a paracoords plot and 
    and a time series plot. */

 pane = gtk_vpaned_new();
 box2 = gtk_hpaned_new();

 dpy = createWindowlessDisplay(GTK_TYPE_GGOBI_TIME_SERIES_DISPLAY, data, gg);
 tsplot_new(dpy, false, 4, rows, data, gg);
 gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(dpy)); 
 gtk_widget_show_all(GTK_WIDGET(dpy));


 dpy = createWindowlessDisplay(GTK_TYPE_GGOBI_BARCHART_DISPLAY, data, gg);
 createBarchart(dpy, false, NULL, 3, data, gg);
 gtk_container_add(GTK_CONTAINER(box2), GTK_WIDGET(dpy));
 gtk_widget_show(GTK_WIDGET(dpy));

 gtk_container_add(GTK_CONTAINER(pane), GTK_WIDGET(box2));


/* Last row. */
 dpy = createWindowlessDisplay(GTK_TYPE_GGOBI_PARCOORDS_DISPLAY, data, gg);
 parcoords_new(dpy, false, 3, rows, data, gg);
 gtk_widget_set_usize(GTK_WIDGET(dpy), 200, 150); 

 gtk_widget_show(GTK_WIDGET(dpy));
#if 1
 gtk_container_add(GTK_CONTAINER(pane), GTK_WIDGET(dpy)); 
 gtk_widget_show_all(GTK_WIDGET(dpy));
#else
 btn = gtk_frame_new("Hi");
 gtk_container_add(GTK_CONTAINER(btn), GTK_WIDGET(dpy));
 gtk_container_add(GTK_CONTAINER(pane), GTK_WIDGET(btn)); 
 gtk_widget_show_all(GTK_WIDGET(btn));
#endif


 gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(pane)); 
 gtk_container_add(GTK_CONTAINER(win), box);

 gtk_widget_set_usize(win, 720, 600);


 gtk_widget_show(pane);
 gtk_widget_show(box2);
 gtk_widget_show(box);
 gtk_widget_show(win);

/* gtk_widget_show_all(win); Bad news as it shows the hidden ruler in the bar chart! */
 gtk_main();

 return(0);
}

displayd *
createWindowlessDisplay(GtkType type, datad *data, ggobid *gg)
{
 displayd *dpy;
 dpy = gtk_type_new(type);
 GTK_GGOBI_WINDOW_DISPLAY(dpy)->useWindow = false;
 if(data && gg)
   display_set_values(dpy, data, gg);

 display_add(dpy, gg);

 return(dpy);
}
