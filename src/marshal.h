
#ifndef __ggobi_marshal_MARSHAL_H__
#define __ggobi_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* NONE:OBJECT,POINTER,OBJECT (marshal.list:1) */
extern void ggobi_marshal_VOID__OBJECT_POINTER_OBJECT (GClosure     *closure,
                                                       GValue       *return_value,
                                                       guint         n_param_values,
                                                       const GValue *param_values,
                                                       gpointer      invocation_hint,
                                                       gpointer      marshal_data);
#define ggobi_marshal_NONE__OBJECT_POINTER_OBJECT	ggobi_marshal_VOID__OBJECT_POINTER_OBJECT

/* NONE:OBJECT,INT,OBJECT (marshal.list:2) */
extern void ggobi_marshal_VOID__OBJECT_INT_OBJECT (GClosure     *closure,
                                                   GValue       *return_value,
                                                   guint         n_param_values,
                                                   const GValue *param_values,
                                                   gpointer      invocation_hint,
                                                   gpointer      marshal_data);
#define ggobi_marshal_NONE__OBJECT_INT_OBJECT	ggobi_marshal_VOID__OBJECT_INT_OBJECT

/* NONE:POINTER,INT,OBJECT (marshal.list:3) */
extern void ggobi_marshal_VOID__POINTER_INT_OBJECT (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);
#define ggobi_marshal_NONE__POINTER_INT_OBJECT	ggobi_marshal_VOID__POINTER_INT_OBJECT

/* NONE:INT,INT,OBJECT (marshal.list:4) */
extern void ggobi_marshal_VOID__INT_INT_OBJECT (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);
#define ggobi_marshal_NONE__INT_INT_OBJECT	ggobi_marshal_VOID__INT_INT_OBJECT

G_END_DECLS

#endif /* __ggobi_marshal_MARSHAL_H__ */

