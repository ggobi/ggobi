<?xml version="1.0"?>
<!-- Customization layer -->


<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:r="http://www.r-project.org"
		xmlns:s="http://cm.bell-labs.com/stat/S4"
		xmlns:c="http://www.c.org"
                xmlns:python="http://www.python.org"
                xmlns:perl="http://www.perl.org"
		xmlns:vb="http://www.visualbasic.com"
		xmlns:omegahat="http://www.omegahat.org"
                xmlns:bioc="http://www.bioconductor.org"
                version="1.0">

<xsl:import href="d:/duncan/Projects/org/omegahat/Docs/XSL/Rstyle.xsl"/>

<xsl:template match="modeName">
 <h2>Mode</h2>
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="bugs">
 <h2>Bugs</h2>
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="details">
 <h2>Details</h2>
 <xsl:apply-templates/>
</xsl:template>


<xsl:template match="dll" />

<xsl:template match="description">
 <h2>Description</h2>
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="usage">
 <h2>Usage</h2>
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="documentation">
 <h2>Documentation</h2>
 <xsl:apply-templates/>
</xsl:template>

<xsl:template match="plugin|inputPlugin">
 <xsl:apply-templates/>
</xsl:template>


<xsl:template match="r:package|s:package|s:library">
 <i>
  <xsl:element name="a">
   <xsl:attribute name="href">http://www.omegahat.org/<xsl:value-of select="."/></xsl:attribute>
   <xsl:apply-templates/>
  </xsl:element>
  </i>
</xsl:template>


</xsl:stylesheet>
