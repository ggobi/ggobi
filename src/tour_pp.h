#ifndef TOUR_PP_H
#define TOUR_PP_H


/* tour_pp.h */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be contacted at the following email addresses:
    dicook@iastate.edu    sigbert@wiwi.hu-berlin.de
*/


gint alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols, gint ndim);
gint free_optimize0_p (optimize0_param *op);
gboolean iszero (array_f *data);

void normal_fill (array_f *data, gfloat delta, array_f *base);
void orthonormal (array_f *proj);
gint optimize0 (optimize0_param *op,
                Tour_PPIndex_f fun,
                void *param);

/* Utility routines */
gdouble ludcmp(gdouble *a, gint n, gint *Pivot); 
gdouble tour_pp_solve(gdouble *a, gdouble *b, gint n, gint *Pivot); 
void inverse(gdouble *a, gint n);

/* Arbitrary dimensional indices */
gint holes_raw(array_f *pdata, void *param, gfloat *val, gpointer unused);
gint central_mass_raw(array_f *pdata, void *param, gfloat *val, gpointer unused);
void zero (gdouble *ptr, gint length);
void zero_int (gint *mem, gint size);
gint compute_groups (vector_i group, vector_i ngroup, gint *groups,  gint nrows, gfloat *gdata);
gint discriminant (array_f *pdata, void *param, gfloat *val, gpointer unused);
gint cartgini (array_f *pdata, void *param, gfloat *val, gpointer unused);
gint cartentropy (array_f *pdata, void *param, gfloat *val, gpointer unused);



#endif 
