/*-- display_ui.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#ifdef SUPPORT_PLUGINS  
#include "plugin.h"
#endif

void buildExtendedDisplayMenu(ggobid *gg, int nd, datad *d0);

void
display_set_position (windowDisplayd *display, ggobid *gg)
{
  gint x, y, width, height;
  gint posx, posy;

  /*-- get the size and position of the gg->main_window) --*/
  gdk_window_get_root_origin (gg->main_window->window, &x, &y);
  gdk_window_get_size (gg->main_window->window, &width, &height);

  gtk_widget_realize (display->window);
  if (x==0 && y==0) {  /*-- can't get any info for the first display --*/
    posx = gdk_screen_width()/4;
    posy = gdk_screen_height()/4;
  } else {
    posx = x+(3*width)/4;
    posy = y+(3*height)/4;
  }

  gtk_widget_set_uposition (display->window, posx, posy);
}


void
display_menu_build (ggobid *gg)
{
  gint nd;
  datad *d0;

  if(gg == NULL || gg->d == NULL)
      return;

  nd = ndatad_with_vars_get (gg);

  d0 = (datad *) gg->d->data;
  if (gg->display_menu != NULL)
    gtk_widget_destroy (gg->display_menu);

  if (nd > 0) {
    gg->display_menu = gtk_menu_new ();

    if (g_slist_length(ExtendedDisplayTypes)) {
       buildExtendedDisplayMenu(gg, nd, d0);
    }
  }

#ifdef SUPPORT_PLUGINS  
  if (sessionOptions->info != NULL) {
    pluginsUpdateDisplayMenu(gg, gg->pluginInstances);
  }
#endif

  /*-- these two lines replace gtk_menu_popup --*/
  if (nd) {
    gtk_widget_show_all (gg->display_menu);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->display_menu_item),
                               gg->display_menu);
  }
}

void
display_menu_init (ggobid *gg)
{
  gg->display_menu_item = submenu_make ("_Display", 'D',
    gg->main_accel_group);

  gtk_widget_show (gg->display_menu_item);

  submenu_insert (gg->display_menu_item, gg->main_menubar, 1);
}


typedef struct {
  GtkGGobiExtendedDisplayClass *klass;
  datad *d;
} ExtendedDisplayCreateData;

static void
extended_display_open_cb (GtkWidget *w, ExtendedDisplayCreateData *data)
{
  ggobid *gg = data->d->gg;
  displayd *dpy;

  if(data->d->nrows == 0)
    return;

  splot_set_current (gg->current_splot, off, gg);   
  if(data->klass->create) {
    dpy = data->klass->create(false, NULL, data->d, gg);
  } else if(data->klass->createWithVars) {
    gint *selected_vars, nselected_vars = 0;

     selected_vars = (gint *) g_malloc (data->d->ncols * sizeof (gint));
     nselected_vars = selected_cols_get (selected_vars, data->d, gg);
#if 0
     if(nselected_vars < 1) {
        nselected_vars = 1;
	selected_vars[0] = 0;
     }
#endif
     dpy = data->klass->createWithVars(false, nselected_vars, selected_vars, data->d, gg);
     g_free(selected_vars);
  } else {
       /* How to get the name of the class from the class! GTK_OBJECT_CLASS(gtk_type_name(data->klass)->type) */
       g_printerr("Real problems! An extended display (%s) without a create routine!\n",  "?");
       return;
  }

  if(!dpy) {
     g_printerr("Failed to create display of type %s\n", data->klass->titleLabel);
     return;
  }

  display_add(dpy, gg);
  varpanel_refresh(dpy, gg);
}

void
buildExtendedDisplayMenu(ggobid *gg, gint nd, datad *d0)
{
  gchar label[200], *lbl;
  GtkGGobiExtendedDisplayClass *klass;
  GSList *el = ExtendedDisplayTypes;
  const gchar * desc;
  GtkWidget *item, *submenu, *anchor;
  gint k;
  ExtendedDisplayCreateData *cbdata;

  while (el) {
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS((GtkObjectClass *) el->data);
    desc = klass->titleLabel;
    sprintf(label, "New %s", desc);

    if(nd == 1) {
      cbdata = (ExtendedDisplayCreateData *) g_malloc(sizeof(ExtendedDisplayCreateData));
      cbdata->d = d0;
      cbdata->klass = klass;

      item = CreateMenuItem (gg->display_menu, label,
             NULL, NULL, gg->main_menubar, gg->main_accel_group,
             GTK_SIGNAL_FUNC (extended_display_open_cb), (gpointer) cbdata, gg);
      gtk_object_set_data (GTK_OBJECT (item),
        "missing_p", GINT_TO_POINTER (0));
    } else {
      submenu = gtk_menu_new ();
      anchor = CreateMenuItem (gg->display_menu, label,
         NULL, NULL, gg->main_menubar, NULL, NULL, NULL, NULL);

      k = 0;
      for (k=0; k < g_slist_length(gg->d); k++) {
        datad *d = (datad*) g_slist_nth_data (gg->d, k);

        /*-- add an item for each datad with variables --*/
        if (g_slist_length (d->vartable) > 0) {
          lbl = datasetName (d, gg);
          cbdata = (ExtendedDisplayCreateData *)
            g_malloc(sizeof(ExtendedDisplayCreateData));
          cbdata->d = d;
          cbdata->klass = klass;
          item = CreateMenuItem (submenu, lbl,
               NULL, NULL, gg->display_menu, gg->main_accel_group,
               GTK_SIGNAL_FUNC (extended_display_open_cb),
               cbdata, gg);

          gtk_object_set_data (GTK_OBJECT (item),
             "displaytype", (gpointer) klass);
          gtk_object_set_data (GTK_OBJECT (item),
             "missing_p", GINT_TO_POINTER (0));
          g_free (lbl);
        }
      }
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (anchor), submenu);
    }

    el = el->next;
  }
}
