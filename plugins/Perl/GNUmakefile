-include local.config

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

PERL_CFLAGS=$(shell perl -MExtUtils::Embed -e ccopts) -DPERL_POLLUTE
PERL_LDFLAGS=$(shell perl -MExtUtils::Embed -e ldopts)
GTK_CONFIG=gtk-config

ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I../..
 GGOBI_LIB=-L../.. -lggobi
endif

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

ifndef PERL
 PERL=perl
endif

GGOBI_CFLAGS=$(shell cd $(GGOBI_HOME) ; $(MAKE) cflags)

CFLAGS+= -g -Wall $(PERL_CFLAGS) $(GGOBI_INCLUDE_DIR) $(GGOBI_CFLAGS)

SRC=Plugin XSInit InputPlugin ConvertUtils
OBJS=$(SRC:%=%.o)

GGOBI_XS_SO=$(shell pwd)/blib/arch/auto/GGobi/GGobi.so

PerlPlugin.so:

$(GGOBI_XS_SO): GGobi.xs Makefile.perl
	$(MAKE) -f Makefile.perl

PerlPlugin.so: $(OBJS) $(GGOBI_XS_SO)
	$(CC) -shared -o $@ $(OBJS) $(GGOBI_XS_SO) $(shell $(GTK_CONFIG) --libs) $(PERL_LDFLAGS)

Makefile.perl: Makefile.PL
	$(PERL) Makefile.PL

cflags:
	(cd ../.. ; make cflags)
