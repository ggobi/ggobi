CXX=c:/MinGW/bin/g++

LIBS=-lole32 -loleaut32 -luuid


ifndef GGOBI_HOME
 GGOBI_HOME=../..
endif

include $(GGOBI_HOME)/local.config

DEFINES=-DNO_PYCOM_IPROVIDECLASSINFO
CXXFLAGS += -g $(DEFINES) -I$(GGOBI_HOME) $(INCLUDES:%=-I%) $(XML_INC_DIRS:%=-I%)

GGOBI_LIBS=-L${GGOBI_HOME} -lggobi $(LIB_DIRS:%=-L%) -lgtk -lgdk -lglib-2.0

OBJS=read.o error.o
ExcelReader.dll: $(OBJS)
	$(CXX) -shared -o $@ ExcelReader.def $(OBJS) ${GGOBI_LIBS} ${LIBS} ${XML_LIBS}

read: read.o error.o
	$(CXX) -o $@ read.o error.o $(GGOBI_LIBS) $(LIBS) $(LIBS)

OMEGA_XSL_DIR=d:/duncan/Projects/org/omegahat/Docs/XSL
# $(OMEGA_XSL_DIR)/Rstyle.xsl
plugin.html: plugin.xml $(GGOBI_HOME)/share/XSL/plugin.xsl
	xsltproc -o $@ -param rstyle "/cygdrive/d/duncan/Projects/org/omegahat/Docs/XSL/Rstyle.xsl" $(GGOBI_HOME)/share/XSL/plugin.xsl  $<

zip: ExcelReader.dll plugin.xml plugin.html
	zip ExcelReader.zip $^
