/* example-start notebook notebook.c */

#define false 0
#define true 1

#include <gtk/gtk.h>
#include <strings.h>

/* for testing */
#define VAR_CIRCLE_DIAM 40
gint ncols=14;
gchar *collab[] =
  {"Climate", "HousingCost", "HlthCare", "Crime",    "Transp", "Educ",
   "Arts",    "Recreat",     "Econ",     "CaseNum ", "Long",   "Lat",
   "Pop",     "StNum"
  };
/* */

#define 1DPLOT  0
#define XYPLOT  1
#define ROTATE  2
#define GRTOUR  3
#define COTOUR  4
#define SCALE   5
#define BRUSH   6
#define IDENT   7
#define LINEED  8
#define MOVEPTS 9

#define NVIEWTYPES 10
static char *view_types_fullname[] = {
  "1DPlot",
  "XYPlot",
  "Rotation",
  "Grand Tour",
  "Correlation Tour",
  "Scale",
  "Brush",
  "Identify",
  "Edit Lines",
  "Move Points",
};
static char *view_types_nickname[] = {
  "1DPlot",
  "XYPlot",
  "Rotate",
  "GrTour",
  "CoTour",
  "Scale",
  "Brush",
  "Ident",
  "LineEd",
  "MovePts",
};

/*
 * The view type panels will be labelled frames
*/
GtkWidget *view_panel[NVIEWTYPES];

make_view_panels() {

  view_panel[1DPLOT] = gtk_frame_new(view_types_fullname[1DPLOT]);
  gtk_frame_set_shadow_type (GTK_FRAME (view_panel[1DPLOT]), GTK_SHADOW_IN);
  gtk_widget_set_usize (view_panel[1DPLOT], 100, 75);

}


gint delete( GtkWidget *widget,
             GtkWidget *event,
             gpointer   data )
{
  gtk_main_quit();
  return(FALSE);
}

static void
set_view_mode(gpointer   data,
              guint      callback_action,
              GtkWidget  *widget)
{
  int i;
  int viewtype = -1;

  for (i=0; i<NVIEWTYPES; i++) {
    if (strstr(widget->name, view_types_fullname[i])) {
      printf("new view type: %s \n", view_types_fullname[i]);
      viewtype = i;
      break;
    }
  }
}

/* This is the GtkItemFactoryEntry structure used to generate new menus.
   Item 1: The menu path. The letter after the underscore indicates an
           accelerator key once the menu is open.
   Item 2: The accelerator key for the entry
   Item 3: The callback function.
   Item 4: The callback action.  This changes the parameters with
           which the function is called.  The default is 0.
   Item 5: The item type, used to define what kind of an item it is.
           Here are the possible values:

           NULL               -> "<Item>"
           ""                 -> "<Item>"
           "<Title>"          -> create a title item
           "<Item>"           -> create a simple item
           "<CheckItem>"      -> create a check item
           "<ToggleItem>"     -> create a toggle item
           "<RadioItem>"      -> create a radio item
           <path>             -> path of a radio item to link against
           "<Separator>"      -> create a separator
           "<Branch>"         -> create an item to hold sub items (optional)
           "<LastBranch>"     -> create a right justified branch
*/
static GtkItemFactoryEntry menu_items[] = {
  { "/_File",         NULL,     NULL,          0, "<Branch>" },
  { "/File/_Read",    NULL,     NULL,          0 },
  { "/File/Save (_extend file set)",   
                      NULL,     NULL,          0 },
  { "/File/Save (_new file set)",   
                      NULL,     NULL,          0 },
  { "/File/_Print",   NULL,     NULL,          0 },
  { "/File/sep1",     NULL,     NULL,          0, "<Separator>" },
  { "/File/_Quit",    "<alt>Q",     gtk_main_quit, 0 },

  { "/_View",                  NULL,       NULL,          0, "<Branch>" },
  { "/View/1_DPlot",            "d",       set_view_mode, 0 },
  { "/View/_XYPlot",            "x",       set_view_mode, 0 },
  { "/View/_Rotation",          "r",       set_view_mode, 0 },
  { "/View/_Grand Tour",        "g",       set_view_mode, 0 },
  { "/View/_Correlation Tour",  "c",       set_view_mode, 0 },
  { "/View/sep1",              NULL,       NULL,          0, "<Separator>" },
  { "/View/_Scale",             "s",       set_view_mode, 0 },
  { "/View/_Brush",             "b",       set_view_mode, 0 },
  { "/View/_Identify",    "i",       set_view_mode, 0 },
  { "/View/Edit _Lines",      "l",       set_view_mode, 0 },
  { "/View/_Move Points",     "m",       set_view_mode, 0 },

  { "/_Tools",        NULL,         NULL, 0, "<Branch>" },
  { "/Tools/Hide or exclude ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Sample or subset ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Smooth ...", 
                      NULL,         NULL, 0, NULL },
  { "/Tools/Jitter ...", 
                      NULL,         NULL, 0, NULL },

  { "/_Display",      NULL,         NULL, 0, "<Branch>" },
  { "/_Display/_Add axes to plot",  
                      "<a>",         NULL, 0, "<ToggleItem>" },
  { "/_Display/_Center axes in 3D+ modes",  
                      NULL,         NULL, 0, "<ToggleItem>" },
  { "/_Display/Plot the points",  
                      NULL,         NULL, 0, "<ToggleItem>" },
  { "/_Display/Show undirected lines",  
                      NULL,         NULL, 0, "<ToggleItem>" },

  { "/_Help",         NULL,         NULL, 0, "<LastBranch>" },
  { "/_Help/About XGobi",  
                      NULL,         NULL, 0, NULL },
  { "/_Help/About help ...",  
                      NULL,         NULL, 0, NULL },
};

void get_main_menu( GtkWidget  *window,
                    GtkWidget **menubar )
{
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;
  gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);

  accel_group = gtk_accel_group_new ();

  /* This function initializes the item factory.
     Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
              or GTK_TYPE_OPTION_MENU.
     Param 2: The path of the menu.
     Param 3: A pointer to a gtk_accel_group.  The item factory sets up
              the accelerator table while generating menus.
  */

  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
                       accel_group);

  /* This function generates the menu items. Pass the item factory,
     the number of items in the array, the array itself, and any
     callback data for the the menu items. */
  gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

  /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

  if (menubar)
    /* Finally, return the actual menu bar created by the item factory. */
    *menubar = gtk_item_factory_get_widget (item_factory, "<main>");
}

gboolean
da_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  gdk_window_clear_area (widget->window,
                         event->area.x, event->area.y,
                         event->area.width, event->area.height);
  gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
                             &event->area);
  gdk_draw_arc (widget->window,
                widget->style->fg_gc[widget->state],
                false,
                0, 0,
                VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM,
                0, 64 * 360);
  gdk_gc_set_clip_rectangle (widget->style->fg_gc[widget->state],
                             NULL);

  return TRUE;
}

gint main(gint argc,
          char *argv[] )
{
  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *table;
  GtkWidget *notebook;
  GtkWidget *frame;
  GtkWidget *label;
  GtkWidget *checkbutton;
  GtkWidget *frame1, *frame2;
  GtkWidget *bin, *vb, *da;
  GtkTooltips *var_label_tips;
  gint i, j, k;
  gint nr, nc;
  gchar bufferf[32];
  gchar bufferl[32];

/* from itemfactory.c */
  GtkWidget *main_vbox;
  GtkWidget *menubar;
/* */

/* from scrolledwin.c */
  GtkWidget *scrolled_window;
  GtkWidget *var_panel;
  gchar buffer[32];
/* */

/* from paned.c */
  GtkWidget *hpaned;
/* */

  
  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (delete), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 10);

/*
 * How about a menu bar?
*/
  main_vbox = gtk_vbox_new (FALSE, 1);
  gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  gtk_widget_show (main_vbox);

  get_main_menu (window, &menubar);
  gtk_box_pack_start (GTK_BOX (main_vbox), menubar, FALSE, TRUE, 0);
  gtk_widget_show (menubar);
/* */

/*
 * A paned widget to separate the view mode panels from the
 * variable panel
*/
  hpaned = gtk_hpaned_new ();
  gtk_container_add (GTK_CONTAINER(main_vbox), hpaned);
  gtk_paned_set_handle_size (GTK_PANED(hpaned), 10);
  gtk_paned_set_gutter_size (GTK_PANED(hpaned), 20);
  gtk_widget_show (hpaned);
/* */

  frame1 = gtk_frame_new(NULL);
  gtk_widget_show(frame1);
  gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_IN);
  table = gtk_table_new(6, ncols/6, FALSE);
  gtk_widget_show(table);
  
  /* Create a new notebook, place the position of the tabs */
  notebook = gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);
  gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0,6,0,1);
  gtk_widget_show(notebook);
  
  /* Let's append a bunch of pages to the notebook */
  for (i=0; i < NVIEWTYPES; i++) {
    sprintf(bufferf, "%s", view_types_fullname[i]);
    sprintf(bufferl, "%s", view_types_nickname[i]);

    frame = gtk_frame_new (bufferf);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
    gtk_widget_set_usize (frame, 100, 75);
    gtk_widget_show (frame);

/*
    panel = make_view_panel(i);
*/

/*
    label = gtk_label_new (bufferf);
    gtk_container_add (GTK_CONTAINER (frame), label);
    gtk_widget_show (label);
*/

    label = gtk_label_new (bufferl);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
  }
    
  /* Set the initial page */
  gtk_notebook_set_page (GTK_NOTEBOOK(notebook), 1);

  gtk_container_add (GTK_CONTAINER (frame1), table);
  gtk_paned_pack1 (GTK_PANED(hpaned), frame1, false, true);

/*
 * Now add the second table, the one that will hold the variable
 * circles.  Assume that the variables ncols and *collab
 * already exist.
*/

  frame2 = gtk_frame_new(NULL);
  gtk_widget_show(frame2);
  gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_IN);
  /* create a new scrolled window. */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (frame2), scrolled_window);
  gtk_paned_pack2 (GTK_PANED(hpaned), frame2, false, true);


  /* the policy is one of GTK_POLICY AUTOMATIC, or GTK_POLICY_ALWAYS.
   * GTK_POLICY_AUTOMATIC will automatically decide whether you need
   * scrollbars, whereas GTK_POLICY_ALWAYS will always leave the scrollbars
   * there.  The first one is the horizontal scrollbar, the second,
   * the vertical. */
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  gtk_widget_show (scrolled_window);

  var_label_tips = gtk_tooltips_new();

  /* create a table of nr by nc squares. */
  nr = MIN(ncols, 6); 
  nc = (int) (ncols/6) + (ncols % 6 > 0 ? 1 : 0);
  var_panel = gtk_table_new (nr, nc, true);

  /* pack the table into the scrolled window */
  gtk_scrolled_window_add_with_viewport (
    GTK_SCROLLED_WINDOW (scrolled_window), var_panel);
  gtk_widget_show (var_panel);

  /*
   * create a grid of buttons in the table
  */
  k = 0;
  for (i=0; i<nc; i++) {
   for (j=0; j<nr; j++) {

      vb = gtk_vbox_new (false, 0);
      gtk_container_border_width (GTK_CONTAINER (vb), 1);
      gtk_widget_show (vb);

      sprintf (buffer, collab[k++]);
      button = gtk_toggle_button_new_with_label (buffer);
      gtk_tooltips_set_tip(GTK_TOOLTIPS (var_label_tips),
        button, buffer, NULL);
      gtk_container_add (GTK_CONTAINER (vb), button);

      da = gtk_drawing_area_new();
      gtk_drawing_area_size(GTK_DRAWING_AREA (da),
        VAR_CIRCLE_DIAM, VAR_CIRCLE_DIAM);
      gtk_signal_connect (GTK_OBJECT (da), "expose_event",
        GTK_SIGNAL_FUNC(da_expose), NULL);
      gtk_widget_show (da);
      gtk_container_add (GTK_CONTAINER (vb), da);

      gtk_table_attach (GTK_TABLE (var_panel), vb, i, i+1, j, j+1,
        GTK_FILL, GTK_FILL, 6, 6);
      gtk_widget_show (button);
      if (k==ncols) break;
   }
 }

/* */
  
  gtk_widget_show(window);
  
  gtk_main ();
  
  return(0);
}
