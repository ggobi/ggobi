<?xml version="1.0"?>

<!-- This provides rules to process the Docs/commandArgs.xml file in -->
<!-- GGobi into strings that can be used directly within C code -->
<!-- (help.c) to provide the input for the '-help' output -->
<!-- See share/R/commandArgs.S for a more powerful processing -->
<!-- mechanism that allows sorting by groups, etc. -->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:r="http://www.r-project.org"
		xmlns:xml="http://www.w3.org/XML"
                version="1.0"
                exclude-result-prefixes="r"
                >
<xsl:output omit-xml-declaration="yes" method="text"/>

<xsl:template match="arg[@alias]">
"<xsl:value-of select='@name'/>", "",
</xsl:template>

<xsl:template match="arg">
"<xsl:value-of select='@name'/>", 
"<xsl:value-of select='normalize-space(quickHelp)'/>",
</xsl:template>
<!-- <xsl:apply-templates select='quickHelp'/> -->
</xsl:stylesheet>