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

AM_CPPFLAGS = @DEBUG_CFLAGS@ @GTK_CFLAGS@ @LIBXML_CFLAGS@ @LIBGSF_CFLAGS@
AM_CFLAGS   = -std=gnu99
AM_LDFLAGS  = @GTK_LIBS@ @LIBXML_LIBS@ @LIBGSF_LIBS@
LDADD       = ../src/libggobi.la

if HAS_VALA
%.stamp: %.vala
	@VALAC@ -C --vapidir=../vapi --vapidir=../src \
    --pkg=gtk+-2.0 --pkg=libgsf-1 --pkg=ggobi-compat --pkg=ggobi --pkg=libxml-2.0 $<
	@touch $@
else
%.stamp: %.vala
	@echo "Either install the Vala compiler or download a GGobi distribution tarball"
	@exit 2
endif

