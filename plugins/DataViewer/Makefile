PKG=DataViewer

$(PKG).so:

ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

# -rpath or --rpath?  The compiler on fry accepts -rpath.

include local.config
-include $(GGOBI_HOME)/local.config


ifdef GTK_EXTRA_CONFIG
 GTKEXTRA_INCLUDES=$(shell $(GTK_EXTRA_CONFIG) --cflags)
 EXTRA_LIBS=$(shell $(GTK_EXTRA_CONFIG) --libs)
else
 EXTRA_LIBS=$(GTKEXTRA_LIB_DIR:%=-L%) $(GTKEXTRA_LIB_DIR:%=-Xlinker -rpath -Xlinker %) -lgtkextra 
endif

CFLAGS+=-g $(GTKEXTRA_INCLUDES) $(GNOME_SUPPORT_INCLUDE_DIR:%=-I%) -I$(GGOBI_HOME) $(shell $(GTK_CONFIG) --cflags)

GGOBI_LIB=-L$(GGOBI_HOME) -lggobi

SRC=worksheet
OBJS=$(SRC:%=%.o)

$(PKG).so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@ $(GGOBI_LIB)  $(EXTRA_LIBS)  $(GTK_CONFIG --libs)

clean:
	rm -f *.o 

distClean: clean
	rm config.cache config.log $(PKG).so

local.config: configure local.config.in
	./configure

configure: configure.in
	autoconf
