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

gint zero (gfloat *ptr, gint length);
gint compute_groups (gint *group, gint *ngroup, gint *groups, gint nrows, 
  gfloat *gdata);
gint alloc_discriminant_p (discriminant_param *dp, gfloat *gdata, gint nrows, 
  gint ncols);
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

gint alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols);
gint free_optimize0_p (optimize0_param *op);
gboolean iszero (array_f *data);

void initrandom(gfloat start);
gfloat uniformrandom(void);
gfloat normalrandom(void);
void normal_fill (array_f *data, gfloat delta, array_f *base);
void orthonormal (array_f *proj);
gint optimize0 (optimize0_param *op,
                gint (*index) (array_f*, void*, gfloat*),
                void *param);





















