-include local.config

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



CFLAGS+= -g -Wall $(PERL_CFLAGS) $(GGOBI_INCLUDE_DIR) $(shell $(GTK_CONFIG) --cflags)

SRC=Plugin
OBJS=$(SRC:%=%.o)

PerlPlugin.so: $(OBJS)
	$(CC) -shared -o $@ $(OBJS) $(shell $(GTK_CONFIG) --libs) $(PERL_LDFLAGS)