/* cpanel.c : control panel */

#include <gtk/gtk.h>

#include "vars.h"

/* external functions */
void init_rotation_cpanel (cpaneld *);
void init_scatmat_cpanel (cpaneld *);
void cpanel_parcoords_init (cpaneld *);
void init_p1d_cpanel (displayd *, cpaneld *);
void show_rotation_cpanel (cpaneld *);
void set_mode (gint);
/*                    */

void
init_control_panel (displayd *display, cpaneld *cpanel, gint initial_mode) {

  cpanel->mode = initial_mode;
  cpanel->projection = initial_mode;


  init_p1d_cpanel (display, cpanel);

  /*
   * rotation
  */
  init_rotation_cpanel (cpanel);


  /*
   * brushing
  */
  cpanel->show_points_p = true;

  /*
   * line editing
  */
  cpanel->show_lines_p = true;


  init_scatmat_cpanel (cpanel);

  cpanel_parcoords_init (cpanel);
}

void show_control_panel (cpaneld *cpanel) {

  /*
   * rotation
  */
  show_rotation_cpanel (cpanel);

  /*
   * brushing
  to->show_points_p = from->show_points_p;
  */

  /*
   * line editing
  to->show_lines_p = from->show_lines_p;
  */

  set_mode (cpanel->mode);
}
