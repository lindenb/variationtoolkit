.PHONY: all wikis
all:wikis

wikis: varkit2wiki.xsl varkit.xml schema01.png
	xsltproc --novalid varkit2wiki.xsl varkit.xml
	
%.png:%.svg
	inkscape --without-gui --export-png=$@ $<
