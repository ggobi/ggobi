#include "PerlPlugin.h"

SV *
Perl_getUnnamedArguments(GSList *args)
{
    int n, i;
    GSList *tmp;
    AV *arr;
    SV *el, *ans;

    if(args == NULL || (n = g_slist_length(args)) == 0)
	return(&sv_undef);

    tmp = args;

    arr = newAV();
    av_extend(arr, n); 

    for(i = 0; i < n; i++) {
	if(tmp->data) {
	    el = newSVpv((char *) tmp->data, 0);
	    SvREFCNT_inc(el);
	    av_push(arr, el);
	}
	tmp = tmp->next;
    }
    ans = newRV_noinc((SV *)arr);

    return((SV *) ans);
}

typedef struct {
    SV *table;
} Perl_HashTableConverter;

void
collectHashElement(gpointer gkey, gpointer gvalue, HV *table)
{
    char *key;
    SV *el;
    key = g_strdup((char *) gkey); /* Who frees this? */
    el = newSVpv((char *) gvalue, 0);
    SvREFCNT_inc(el);

    hv_store(table, key, strlen(key), el, 0);
}

SV *
Perl_getNamedArguments(GHashTable *table)
{
    int n;
    HV *hash;
    SV *ans;
    if(!table || (n = g_hash_table_size(table)) < 1)
	return(&sv_undef);

    hash = newHV();

    g_hash_table_foreach(table, (GHFunc) collectHashElement, (gpointer) hash);

    ans = newRV_noinc((SV*) hash);

    return((SV *) ans);
}
