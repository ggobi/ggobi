# ADD NEW TESTS HERE (root name of vala file)
check_PROGRAMS = dummy

VALAS = $(check_PROGRAMS:%=%.vala)
VALASOURCES = $(VALAS:%.vala=%.c)
VALAHEADERS = $(VALAS:%.vala=%.h)
VALASTAMPS = $(VALAS:%.vala=%.stamp)

BUILT_SOURCES = $(VALASTAMPS) $(VALASOURCES) $(VALAHEADERS)
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = $(VALAS) $(VALAHEADERS) $(VALASTAMPS)

TESTS = $(check_PROGRAMS)

AM_CPPFLAGS = @DEBUG_CFLAGS@ @DEP_CFLAGS@
AM_CFLAGS   = -std=gnu99
AM_LDFLAGS  = @DEP_LIBS@
LDADD       = ../src/libggobi.la

if USE_VALAC
%.stamp: %.vala
	@VALAC@ -C --vapidir=../vapi --vapidir=../src \
          --pkg=gtk+-2.0 --pkg=libgsf-1 --pkg=ggobi-compat --pkg=ggobi \
          --pkg=libxml-2.0 --pkg=gmodule-2.0 --pkg=clutter-0.8 --pkg=cogl-0.8 \
          --pkg=gl $<
	@touch $@
else
%.stamp: %.vala
	@echo "Either install the Vala compiler or download a GGobi distribution tarball"
	@exit 2
endif

