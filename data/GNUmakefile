XML_FILES=$(wildcard *.xml)
GGOBI_DTD=ggobi.dtd
XMLLINT=xmllint

xmlCheck: ggobi.dtd $(XML_FILES)
	for f in  $(XML_FILES) ; do \
	   echo $$f; $(XMLLINT) -dtdvalid $(GGOBI_DTD) -noout $$f ; \
	done
