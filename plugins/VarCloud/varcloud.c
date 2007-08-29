#include <gtk/gtk.h>
#include "session.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "varcloud.h"

#include "GGStructSizes.c"

void    vcl_window_closed(GtkWidget *w, PluginInstance *inst);
void    close_vcl_window_cb(GtkWidget *w, PluginInstance *inst);
void    create_vcl_window(vcld *vcl, PluginInstance *inst);
void    show_vcl_window (GtkAction *action, PluginInstance *inst);

gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  static GtkActionEntry entry = {
	"VarCloud", NULL, "Variogram _Cloud", NULL, "Spatial data analysis tool", 
		G_CALLBACK (show_vcl_window)
  };
  GGOBI(addToolAction)(&entry, (gpointer)inst, gg);
  return(true);
}

void
show_vcl_window (GtkAction *action, PluginInstance *inst)
{
  if (g_slist_length(inst->gg->d) < 1) {
    g_printerr ("No datasets to show\n");
    return;
  }

  if (inst->data == NULL) {
    vcld *vcl = (vcld *) g_malloc (sizeof (vcld));

    vcl_init (vcl, inst->gg);
    create_vcl_window (vcl, inst);

  } else {
    gtk_widget_show_now ((GtkWidget*) inst->data);  /* ie, window */
  }
}

vcld *
vclFromInst (PluginInstance *inst)
{
  GtkWidget *window = (GtkWidget *) inst->data;
  vcld *vcl = NULL;

  if (window)
    vcl = (vcld *) g_object_get_data(G_OBJECT(window), "vcld");

  return vcl;
}

static void
vcl_datad_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  GGobiData *d, *dprev;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (!gtk_tree_selection_get_selected(tree_sel, &model, &iter))
	  return;
 
  dprev = vcl->dsrc;
  gtk_tree_model_get(model, &iter, 1, &d, -1);
  vcl->dsrc = d;

  /* Rebuild the clists ... or should the clists respond to these events? */
  if (vcl->dsrc != dprev) {
    GtkWidget *tree_view;
    GtkWidget *window = (GtkWidget *) inst->data;
    gchar *names[] = {"XCOORD", "YCOORD", "VAR1"};
    vartabled *vt;
    gint j, k;
    GtkTreeModel *model;
	GtkTreeIter iter;

    for (k=0; k<3; k++) {
      tree_view = widget_find_by_name(window, names[k]);
	  model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
	  gtk_list_store_clear(GTK_LIST_STORE(model));
      for (j=0; j<vcl->dsrc->ncols; j++) {
        vt = vartable_element_get (j, vcl->dsrc);
        if (vt) {
          gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, vt->collab, -1);
		}
      }
    }
  }
}

static void 
vcl_tree_view_datad_added_cb (ggobid *gg, GGobiData *d, GtkWidget *tree_view)
{
  GtkWidget *swin;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (tree_view == NULL)
    return;

  swin = (GtkWidget *)
    g_object_get_data(G_OBJECT (tree_view), "datad_swin");
  
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view));
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, -1);
	
  gtk_widget_show_all (swin);
}

static void
vcl_xcoord_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->xcoord = tree_selection_get_selected_row(tree_sel);
}
static void
vcl_ycoord_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->ycoord = tree_selection_get_selected_row(tree_sel);
}
static void
vcl_variable1_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->var1 = tree_selection_get_selected_row(tree_sel);
}
static void
vcl_variable2_set_cb (GtkTreeSelection *tree_sel, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->var2 = tree_selection_get_selected_row(tree_sel);
}


void
create_vcl_window(vcld *vcl, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox;
  GtkWidget *tree_view;
  GtkWidget *frame, *btn, *hbox, *vb;
  GtkWidget *swin;
  gint j, nd;
  GSList *l;
  GGobiData *d;
  ggobid *gg = inst->gg;
  vartabled *vt;
  GtkListStore *model;
  GtkTreeIter iter;

  vcl->tips = gtk_tooltips_new ();

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_object_set_data(G_OBJECT (window), "vcld", vcl);
  inst->data = window; 

  gtk_window_set_title(GTK_WINDOW(window),
    "VarCloud");
  g_signal_connect (G_OBJECT (window), "destroy",
    G_CALLBACK (vcl_window_closed), inst);

  main_vbox = gtk_vbox_new (false, 1);
  gtk_container_set_border_width (GTK_CONTAINER(main_vbox), 5); 
  gtk_container_add (GTK_CONTAINER(window), main_vbox);

  /*-- if there's more than one datad, start with a list of datads --*/
  nd = g_slist_length (gg->d);
  if (nd > 1) {
    frame = gtk_frame_new("Dataset");
    gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
    vb = gtk_vbox_new (false, 2);
    gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
    gtk_container_add (GTK_CONTAINER(frame), vb);

    /* Create a scrolled window to pack the CList widget into */
    swin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	  model = gtk_list_store_new(2, G_TYPE_STRING, GGOBI_TYPE_DATA);
	  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	  populate_tree_view(tree_view, NULL, 1, false, GTK_SELECTION_SINGLE, 
	  	G_CALLBACK(vcl_datad_set_cb), inst);
	  g_object_set_data(G_OBJECT(tree_view), "datad_swin", swin);
    g_signal_connect (G_OBJECT (gg), "datad_added",
      G_CALLBACK(vcl_tree_view_datad_added_cb), tree_view);
    /*-- --*/

    for (l = gg->d; l; l = l->next) {
      d = (GGobiData *) l->data;
      gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	  gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, d->name, 1, d, -1);
    }
	
    select_tree_view_row (tree_view, 0);
    gtk_container_add (GTK_CONTAINER (swin), tree_view);
    gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);
    gtk_box_pack_start (GTK_BOX (main_vbox), frame, true, true, 2);
  }


/*-- Coordinates --*/

  hbox = gtk_hbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

  /*-- X Coordinate */
  frame = gtk_frame_new("X Coordinate");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 2);
  vb = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 
  model = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, NULL, 1, false, GTK_SELECTION_SINGLE, 
  	G_CALLBACK(vcl_xcoord_set_cb), inst);
  gtk_widget_set_name(tree_view, "XCOORD");
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, vt->collab, -1);
  }
  select_tree_view_row (tree_view, 0);
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);

  frame = gtk_frame_new("Y Coordinate");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 2);
  vb = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 
  model = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, NULL, 1, false, GTK_SELECTION_SINGLE, 
  	G_CALLBACK(vcl_ycoord_set_cb), inst);
  gtk_widget_set_name(tree_view, "YCOORD");
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, vt->collab, -1);
  }
  select_tree_view_row (tree_view, 0);
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);

  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, true, true, 2);

/*-- Covariates --*/

  hbox = gtk_hbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

  frame = gtk_frame_new("Variable 1");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 2);
  vb = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 
  model = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, NULL, 1, false, GTK_SELECTION_SINGLE, 
  	G_CALLBACK(vcl_variable1_set_cb), inst);
  gtk_widget_set_name(tree_view, "VAR1");
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, vt->collab, -1);
  }
  select_tree_view_row (tree_view, 2);
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);


  frame = gtk_frame_new("Variable 2");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 2);
  vb = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
 
  model = gtk_list_store_new(1, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
  populate_tree_view(tree_view, NULL, 1, false, GTK_SELECTION_SINGLE, 
  	G_CALLBACK(vcl_variable2_set_cb), inst);
  gtk_widget_set_name(tree_view, "VAR2");
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, vt->collab, -1);
  }
  select_tree_view_row (tree_view, 2);
  gtk_container_add (GTK_CONTAINER (swin), tree_view);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);

  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, true, true, 2);


/*-- Run controls --*/

  hbox = gtk_hbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, false, false, 2);

  /*-- run --*/
  btn = gtk_button_new_with_mnemonic ("_Var cloud");
  gtk_widget_set_name (btn, "VarCloud");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (vcl->tips), btn,
    "Launch variogram cloud plot, using Variable 1", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, false, 2);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (launch_varcloud_cb), inst);

  btn = gtk_button_new_with_mnemonic ("_Cross-var cloud");
  gtk_widget_set_name (btn, "Cross");
  gtk_tooltips_set_tip (GTK_TOOLTIPS (vcl->tips), btn,
    "Launch cross-variogram cloud plot, using Variables 1 and 2", NULL);
  gtk_box_pack_start (GTK_BOX (hbox), btn, true, false, 2);
  g_signal_connect (G_OBJECT (btn), "clicked",
    G_CALLBACK (launch_varcloud_cb), inst);


  btn = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (vcl->tips), btn,
    "Close this window", NULL);
  g_signal_connect (G_OBJECT (btn), "clicked",
		      G_CALLBACK (close_vcl_window_cb), inst); 
  gtk_box_pack_start (GTK_BOX (main_vbox), btn, false, false, 2);


  gtk_widget_show_all (window);
}

void freePlugin(ggobid *gg, PluginInstance *inst)
{
  if (inst->data) {
    GtkWidget *window = (GtkWidget *) inst->data;
    gtk_widget_destroy (window);
    inst->data = NULL;
  }
}

void vcl_window_closed(GtkWidget *w, PluginInstance *inst)
{
  freePlugin (inst->gg, inst);
}

void
vcl_free (vcld *vcl) {

  g_free (vcl);
}

void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  freePlugin (gg, inst);
}

