/* A ggobi drive */

#include <math.h>
#include <gtk/gtk.h>
#include "ggobi.h"
#include "GGobiAPI.h"
#include <stdio.h>
#include <stdlib.h>

float **my_matrix = NULL;
int nrows, ncols;

void create_matrix()
{
  /* we will use a 10 (c) by 100 (r) matrix of float to demonstrate the procedure */
  int i, j;
  ncols = 10;
  nrows = 100;
  my_matrix = malloc(sizeof(float*) * nrows);
  my_matrix[0] = malloc(sizeof(float) * nrows*ncols);
  for (i=1; i<nrows; i++)
    my_matrix[i] = my_matrix[i-1] + ncols;
  for (i=0; i<nrows; i++)
    for (j=0; j<ncols; j++)
      my_matrix[i][j] = sqrt(i*j)+i;
}

void free_matrix()
{
  if (!my_matrix) return;
  free(my_matrix[0]);
  free(my_matrix);
}

ggobid * create_mygobi()
{
  InputDescription *input = NULL;
  ggobid *gg;

  /* create a matrix using the above function create_matrix() */
  create_matrix();

  /* then load this matrix to ggobi       ********************** TODO ****************** */
  /* the following is just guessing ... */

  gg = create_ggobi(input);

  return gg;
}

void on_bt_clicked(GtkWidget *widget, gpointer data)
{
  create_mygobi();                        /* ******************* TODO ****************** */
  g_print ("Open ggobi\n");
}

/* *******************************************************************************
 * Next step:
 *
 * After successfully displaying the matrix, how to communicate between ggobi
 * and this program (mygobi)? More specifically, when one uses identity
 * tool in ggobi, how can this program know and capture the activity when
 * a point is highlighted and/or clicked?
 *
 * *************************************************************************** */

void destroy( GtkWidget *widget, gpointer   data)
{
  free_matrix();
  gtk_main_quit();
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *button;
    
  ggobiInit(&argc, &argv);
    
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    
  button = gtk_button_new_with_label ("Open ggobi");
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (on_bt_clicked), NULL);
  g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (gtk_widget_destroy),
			     G_OBJECT (window));
  gtk_container_add (GTK_CONTAINER (window), button);
    
  gtk_widget_show (button);
  gtk_widget_show (window);
    
  gtk_main ();
    
  return(0);
}
