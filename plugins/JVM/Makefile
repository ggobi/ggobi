PKG=JVM

All:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

include local.config
-include $(GGOBI_HOME)/local.config

ifdef ADMIN
 include Install/GNUmakefile.admin
endif

ifeq (${NATIVE_ACCESS},yes)
 SRC=ggobiAccess
 OTHERS=libJavaGGobi.so
endif

All: $(PKG).so classes $(OTHERS)

classes:
	$(MAKE) -C ggobi classes

$(PKG).so:



ifdef GGOBI_CONFIG
 GGOBI_INCLUDE_DIR=$(shell $(GGOBI_CONFIG) --cflags)
 GGOBI_LIB=$(shell $(GGOBI_CONFIG) --libs)
else
 GGOBI_INCLUDE_DIR=-I$(GGOBI_HOME)
 GGOBI_LIB=-L$(GGOBI_HOME) -lggobi
endif

GTK_CONFIG=gtk-config
EXTRA_LIBS=${JNI_LIBS:%=-L%} -ljvm -lhpi

ifeq (${DEBUG},yes)
 DEBUG="-DJAVA_DEBUG=1"
endif

GGOBI_CFLAGS=$(shell cd $(GGOBI_HOME) ; make cflags)
CFLAGS+=-g $(GGOBI_CFLAGS) $(JNI_INCLUDES:%=-I%)  $(DEBUG) $(GGOBI_INCLUDE_DIR)


SRC+=jvm


OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(GGOBI_LIB)  $(EXTRA_LIBS)  $(shell $(GTK_CONFIG) --libs)

distClean:: 
	rm -f ggobi.class 



ggobiAccess.o: ggobi_ggobi.h ggobi_datad.h

ggobi_%.h: ggobi/%.class
	$(JAVAH) -classpath . $(subst .class,,$(subst /,., $<))

ggobi/%.class: ggobi/%.java
	$(MAKE) -C ggobi $(@F)

libJavaGGobi.so: ggobiAccess.o
	$(CC) -shared -o $@ ggobiAccess.o $(GGOBI_LIB) $(EXTRA_LIBS)  $(shell $(GTK_CONFIG) --libs)

