<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:r="http://www.r-project.org"
		xmlns:xml="http://www.w3.org/XML/1998/namespace"
                version="1.0"
                exclude-result-prefixes="r"
                >

<xsl:template match="arguments">
<head>
<body>
<title>Command line arguments for GGobi</title>
<h1>Command line arguments for GGobi</h1>
<table border="1">
 <tr>
  <th>Argument name</th>
  <th>Description</th>
 </tr>
  <xsl:apply-templates />
</table>
</body>
</head>
</xsl:template>

<xsl:template match="arg">
 <tr>
  <td align="left">
  <xsl:element name="a"> 
   <xsl:attribute name="name">
    <xsl:value-of select="@name"/>
   </xsl:attribute>
   <xsl:value-of select="@name"/>
  </xsl:element>
  </td>
  <td align="left"><xsl:apply-templates /></td>
</tr>
</xsl:template>

<xsl:template match="argRef">
 <xsl:element name="a">
  <xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute>
  <xsl:value-of select="@name"/>
 </xsl:element>
</xsl:template>

</xsl:stylesheet>
