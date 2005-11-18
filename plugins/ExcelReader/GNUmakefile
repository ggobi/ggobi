CXX=c:/MinGW/bin/g++

PLUGIN_LIBS=-lole32 -loleaut32 -luuid

SRC=$(wildcard *.cpp)

DEFINES=-DNO_PYCOM_IPROVIDECLASSINFO
CXXFLAGS += -g $(DEFINES) -I$(GGOBI_HOME)/src $(INCLUDES:%=-I%) $(XML_INC_DIRS:%=-I%)

ExcelReader.dll: $(OBJS)
	$(CXX) -shared -o $@ ExcelReader.def $(OBJS) ${GGOBI_LIBS} ${PLUGIN_LIBS} ${XML_LIBS}

read: read.o error.o
	$(CXX) -o $@ read.o error.o $(GGOBI_LIBS) $(PLUGIN_LIBS)

OMEGA_XSL_DIR=d:/duncan/Projects/org/omegahat/Docs/XSL
# $(OMEGA_XSL_DIR)/Rstyle.xsl
plugin.html: plugin.xml $(GGOBI_HOME)/share/XSL/plugin.xsl
	xsltproc -o $@ -param rstyle "/cygdrive/d/duncan/Projects/org/omegahat/Docs/XSL/Rstyle.xsl" $(GGOBI_HOME)/share/XSL/plugin.xsl  $<

zip: ExcelReader.dll plugin.xml plugin.html
	zip ExcelReader.zip $^
