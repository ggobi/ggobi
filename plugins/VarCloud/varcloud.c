#include <gtk/gtk.h>
#include "ggobi.h"
#include "externs.h"
#include "GGobiAPI.h"

#include <stdio.h>

#include "plugin.h"
#include "varcloud.h"

#include "GGStructSizes.c"

void       close_vcl_window(GtkWidget *w, PluginInstance *inst);
void       create_vcl_window(vcld *vcl, PluginInstance *inst);
void       show_vcl_window (GtkWidget *widget, PluginInstance *inst);

gboolean
addToToolsMenu(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  GtkWidget *entry;
  const gchar *lbl = "VarCloud ...";

  inst->data = NULL;
  inst->info = plugin;
  inst->gg = gg;

  entry = GGobi_addToolsMenuItem ((gchar *)lbl, gg);
  gtk_signal_connect (GTK_OBJECT(entry), "activate",
                      GTK_SIGNAL_FUNC (show_vcl_window), inst);
  return(true);
}

void
show_vcl_window (GtkWidget *widget, PluginInstance *inst)
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
    vcl = (vcld *) gtk_object_get_data (GTK_OBJECT(window), "vcld");

  return vcl;
}

static void
vcl_datad_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  ggobid *gg = inst->gg;
  vcld *vcl = vclFromInst (inst);
  gchar *dname;
  datad *d, *dprev;
  GSList *l;

  dprev = vcl->dsrc;
  gtk_clist_get_text (GTK_CLIST (cl), row, 0, &dname);
  for (l = gg->d; l; l = l->next) {
    d = l->data;
    if (strcmp (d->name, dname) == 0) {
      vcl->dsrc = d;
      break;
    }
  }
  /* Don't free the string; it's just a pointer */

  /* Rebuild the clists ... or should the clists respond to these events? */
  if (vcl->dsrc != dprev) {
    GtkWidget *clist;
    GtkWidget *window = (GtkWidget *) inst->data;
    gchar *names[] = {"XCOORD", "YCOORD", "VAR1"};
    vartabled *vt;
    gint j, k;
    gchar *row[1];

    for (k=0; k<3; k++) {
      clist = widget_find_by_name(window, names[k]);
      gtk_clist_freeze (GTK_CLIST(clist));
      gtk_clist_clear (GTK_CLIST (clist));
      for (j=0; j<vcl->dsrc->ncols; j++) {
        vt = vartable_element_get (j, vcl->dsrc);
        if (vt) {
          row[0] = g_strdup_printf (vt->collab);
          gtk_clist_append (GTK_CLIST (clist), row);
        }
      }
      gtk_clist_thaw (GTK_CLIST(clist));
    }
  }
}

static void 
vcl_clist_datad_added_cb (ggobid *gg, datad *d, void *clist)
{
  gchar *row[1];
  GtkWidget *swin;
  gchar *clname;

  if (clist == NULL)
    return;

  swin = (GtkWidget *)
    gtk_object_get_data (GTK_OBJECT (clist), "datad_swin");
  clname = gtk_widget_get_name (GTK_WIDGET(clist));

  row[0] = g_strdup (d->name);
  gtk_clist_append (GTK_CLIST (GTK_OBJECT(clist)), row);
  g_free (row[0]);

  gtk_widget_show_all (swin);
}

static void
vcl_xcoord_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->xcoord = row;
}
static void
vcl_ycoord_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->ycoord = row;
}
static void
vcl_variable1_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->var1 = row;
}
static void
vcl_variable2_set_cb (GtkWidget *cl, gint row, gint column,
  GdkEventButton *event, PluginInstance *inst)
{
  vcld *vcl = vclFromInst (inst);
  vcl->var2 = row;
}


void
create_vcl_window(vcld *vcl, PluginInstance *inst)
{
  GtkWidget *window, *main_vbox;
  GtkWidget *clist;
  GtkWidget *frame, *btn, *hbox, *vb;
  GtkWidget *swin;
  gint j, nd;
  GSList *l;
  datad *d;
  gchar *row[1];
  ggobid *gg = inst->gg;
  vartabled *vt;

  vcl->tips = gtk_tooltips_new ();

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (window), "vcld", vcl);
  inst->data = window; 

  gtk_window_set_title(GTK_WINDOW(window),
    "VarCloud");
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
    GTK_SIGNAL_FUNC (close_vcl_window), inst);

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
      GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

    clist = gtk_clist_new (1);
    gtk_clist_set_selection_mode (GTK_CLIST (clist),
      GTK_SELECTION_SINGLE);
    gtk_signal_connect (GTK_OBJECT (clist), "select_row",
      (GtkSignalFunc) vcl_datad_set_cb, inst);
    gtk_signal_connect (GTK_OBJECT (gg), "datad_added",
      (GtkSignalFunc) vcl_clist_datad_added_cb, GTK_OBJECT (clist));
    /*-- --*/

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      row[0] = g_strdup (d->name);
      gtk_clist_append (GTK_CLIST (clist), row);
      g_free (row[0]);
    }
    gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
    gtk_container_add (GTK_CONTAINER (swin), clist);
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
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 
  clist = gtk_clist_new(1);
  gtk_widget_set_name(clist, "XCOORD");
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
		      GTK_SIGNAL_FUNC (vcl_xcoord_set_cb), inst);
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    row[0] = g_strdup (vt->collab);
    gtk_clist_append (GTK_CLIST (clist), row);
    g_free (row[0]);
  }
  gtk_clist_select_row (GTK_CLIST(clist), 0, 0);
  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);

  frame = gtk_frame_new("Y Coordinate");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 2);
  vb = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 
  clist = gtk_clist_new(1);
  gtk_widget_set_name(clist, "YCOORD");
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
		      GTK_SIGNAL_FUNC (vcl_ycoord_set_cb), inst);
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    row[0] = g_strdup (vt->collab);
    gtk_clist_append (GTK_CLIST (clist), row);
    g_free (row[0]);
  }
  gtk_clist_select_row (GTK_CLIST(clist), 1, 0);
  gtk_container_add (GTK_CONTAINER (swin), clist);
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
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 
  clist = gtk_clist_new(1);
  gtk_widget_set_name(clist, "VAR1");
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
		      GTK_SIGNAL_FUNC (vcl_variable1_set_cb), inst);
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    row[0] = g_strdup (vt->collab);
    gtk_clist_append (GTK_CLIST (clist), row);
    g_free (row[0]);
  }
  gtk_clist_select_row (GTK_CLIST(clist), 2, 0);  /* assumes 3
						     variables */
  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);


  frame = gtk_frame_new("Variable 2");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start (GTK_BOX (hbox), frame, true, true, 2);
  vb = gtk_vbox_new (false, 2);
  gtk_container_set_border_width (GTK_CONTAINER (vb), 5);
  gtk_container_add (GTK_CONTAINER(frame), vb);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swin),
    GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
 
  clist = gtk_clist_new(1);
  gtk_widget_set_name(clist, "VAR2");
  gtk_clist_set_selection_mode(GTK_CLIST(clist), GTK_SELECTION_SINGLE);
  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
		      GTK_SIGNAL_FUNC (vcl_variable2_set_cb), inst);
  for (j=0; j<vcl->dsrc->ncols; j++) {
    vt = vartable_element_get (j, vcl->dsrc);
    row[0] = g_strdup (vt->collab);
    gtk_clist_append (GTK_CLIST (clist), row);
    g_free (row[0]);
  }
  gtk_clist_select_row (GTK_CLIST(clist), 2, 0);  /* assumes 3
						     variables */
  gtk_container_add (GTK_CONTAINER (swin), clist);
  gtk_box_pack_start (GTK_BOX (vb), swin, true, true, 2);

  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, true, true, 2);


/*-- Run controls --*/

  /*-- run --*/
  btn = gtk_button_new_with_label ("Launch variogram cloud plot");
  gtk_widget_set_name (btn, "Launch");

  gtk_box_pack_start (GTK_BOX (main_vbox), btn, false, false, 2);
  gtk_signal_connect (GTK_OBJECT (btn), "clicked",
    GTK_SIGNAL_FUNC (launch_varcloud_cb), inst);

  gtk_widget_show_all (window);
}


void close_vcl_window(GtkWidget *w, PluginInstance *inst)
{
}

/*
 * This one is called when ggobi shuts down; it doesn't
 * need to do much of anything, I think.  
 */
void closeWindow(ggobid *gg, GGobiPluginInfo *plugin, PluginInstance *inst)
{
  if (inst->data) {
    inst->data = NULL;
  }
}

void
vcl_free (vcld *vcl) {

  g_free (vcl);
}
