/* tour1d_pp.h */
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
void center (array_f *data);
gint pca (array_f *pdata, void *param, gfloat *val);

gint alloc_subd_p (subd_param *sp, gint nrows, gint ncols);
gint free_subd_p (subd_param *sp);
int smallest (const void *left, const void *right);
void distance (array_f *pdata, gint i, gfloat *dist);
void mean_min_neighbour (array_f *pdata, gint *index, 
  int min_neighbour, gfloat *nmean);
void covariance (array_f *pdata, gint *index, int j, gfloat *mean, 
  gfloat *cov);
gfloat variance_explained (gfloat *ew, gint d, gint p);
void eigenvalues (gfloat *cov, int p, gfloat *ew,
                  gint matz, gfloat *ev, gfloat *fv1, gfloat *fv2);
gint subd (array_f *pdata, void *param, gfloat *val);

gint zero (gdouble *ptr, gint length);
gint compute_groups (gint *group, gint *ngroup, gint *groups, gint nrows, 
  gfloat *gdata);
gint alloc_discriminant_p (discriminant_param *dp, /* gfloat *gdata, */
  gint nrows, gint ncols);
gint free_discriminant_p (discriminant_param *dp);
gint discriminant (array_f *pdata, void *param, gfloat *val);

gint alloc_cartgini_p (cartgini_param *dp, gint nrows, gfloat *gdata);
gint free_cartgini_p (cartgini_param *dp);
gint cartgini (array_f *pdata, void *param, gfloat *val);

gint alloc_cartentropy_p (cartentropy_param *dp, gint nrows, gfloat *gdata);
gint free_cartentropy_p (cartentropy_param *dp);
gint cartentropy (array_f *pdata, void *param, gfloat *val);

gint alloc_cartvariance_p (cartvariance_param *dp, gint nrows, gfloat *gdata);
gint free_cartvariance_p (cartvariance_param *dp);
gint cartvariance (array_f *pdata, void *param, gfloat *val);

/*gfloat t1d_calc_indx(array_f, array_d, gint *, gint, gint,
                gint (*index) (array_f*, void*, gfloat*),
                void *param);*/
gfloat t1d_calc_indx(array_f, /* gint *, gint, gint,*/
                gint (*index) (array_f*, void*, gfloat*),
                void *param);
gboolean t1d_switch_index(gint, gint, ggobid *);
