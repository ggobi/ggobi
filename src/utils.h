#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

gboolean   is_numeric (const gchar * str);
gdouble as_number (const char *sval);
gboolean as_logical (const gchar * sval);

#ifdef __cplusplus
}
#endif

#endif
