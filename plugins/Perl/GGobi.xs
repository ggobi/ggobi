#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "ggobi.h"
#include "GGobiAPI.h"

extern ggobid *all_ggobis[];

MODULE = plugin PACKAGE = GGobi PREFIX = PERL_GGobi_

PROTOTYPES: ENABLE

SV *
numDataSets(pgg)
	SV *pgg
	PREINIT:
	ggobid *gg;
	int n;
        unsigned long p;
	PPCODE:
	p = ((unsigned long) SvUV(SvRV(pgg)));
        gg = (ggobid *)	p;
	n = g_slist_length(gg->d);
	XPUSHs(sv_2mortal(newSVuv(n)));
