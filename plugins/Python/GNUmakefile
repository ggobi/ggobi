GGOBI_LIB=-L.
PKG=Python
GGOBI_HOME=../..

All: $(PKG).so

SRC=Python

-include local.config
-include $(GGOBI_HOME)/local.config

ifdef ADMIN
include Install/GNUmakefile.admin
else
include $(GGOBI_HOME)/plugins/GNUmakefile.plugin
endif



