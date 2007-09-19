#ifndef GGOBI_PIPELINE_H
#define GGOBI_PIPELINE_H

#include "vars.h"
#include "externs.h"


void       tform_to_world (GGobiStage *);
void       tform_to_world_by_var (GGobiStage *, guint j);


void
pt_world_to_raw_by_var (gint j, greal * world, greal * raw, GGobiStage * d);

void
pt_screen_to_raw (icoords * screen, gint id, gboolean horiz, gboolean vert,
                  greal * raw, gcoords * eps, GGobiStage * d, splotd * sp,
                  GGobiSession * gg);
void
pt_plane_to_world (splotd * sp, gcoords * planar, gcoords * eps,
                   greal * world);
void
pt_screen_to_plane (icoords * screen, gint id, gboolean horiz, gboolean vert,
                   gcoords * eps, gcoords * planar, splotd * sp);


#endif
