/* tour2d_pp.h */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke and Eun-Kyung Lee

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
void alloc_holes_p (holes_param *hp, gint nrows);
void free_holes_p (holes_param *hp);

gint skewness_raw2(array_f *pdata, void *param, gfloat *val);
gint skewness_raw1( array_f *pdata, void *param, gfloat *val);
gint skewness( array_f *pdata, void *param, gfloat *val);
gint central_mass(array_f *pdata, void *param, gfloat *val);
gint central_mass_raw1(array_f *pdata, void *param, gfloat *val);
gint central_mass_raw2(array_f *pdata, void *param, gfloat *val);
gfloat mean_fn2(gfloat *x1, gfloat *x2, gint n);
/*void inverse(double *a, int n);*/
gint holes( array_f *pdata,void *param, gfloat *val);
gint holes_raw1(array_f *pdata,void *param, gfloat *val);
gint holes_raw2(array_f *pdata,void *param, gfloat *val);

/*gfloat t2d_calc_indx(array_f, array_d, gint *, gint, gint,
                gint (*index) (array_f*, void*, gfloat*),
                void *param);*/
gfloat t2d_calc_indx(array_f, gint (*index) (array_f*, void*, gfloat*),
                void *param);
gboolean t2d_switch_index(gint indxtype, gint basismeth, displayd *, ggobid *);
