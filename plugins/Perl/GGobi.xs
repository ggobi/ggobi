#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "ggobi.h"
#include "GGobiAPI.h"

ggobid *
asGGobi(SV *pgg)
{
        unsigned long p;
        ggobid *gg;
	p = ((unsigned long) SvUV(SvRV(pgg)));
        gg = (ggobid *)	p;
        return(gg);
}

MODULE = plugin PACKAGE = GGobi PREFIX = PERL_GGobi_

PROTOTYPES: ENABLE

SV *
numDataSets(pgg)
	SV *pgg
	PREINIT:
	ggobid *gg;
	int n;
	PPCODE:
        gg = asGGobi(pgg);
	n = g_slist_length(gg->d);
	XPUSHs(sv_2mortal(newSVuv(n)));
