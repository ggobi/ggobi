Rplugin.so:

ifndef GGOBI_HOME
  GGOBI_HOME=../..
endif

-include $(GGOBI_HOME)/local.config
-include local.config


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

R_LIBS=-L$(R_HOME)/bin -lR

SRC=R
# REval
OBJS=$(SRC:%=%.o)

CFLAGS+= -g -Wall -I$(R_HOME)/include $(GGOBI_INCLUDE_DIR)  $(shell $(GTK_CONFIG) --cflags) -I$(OMEGA_HOME)/include/Corba -D_R_=1 -DUSE_R=1

Rplugin.so: $(OBJS)
	$(CC) -shared -o $@ $^ $(R_LIBS) $(GGOBI_LIB)  $(shell $(GTK_CONFIG) --libs) 

ifndef ADMIN
local.config: local.config.in configure
	./configure


configure: configure.in
	autoconf
endif
