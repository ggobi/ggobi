<?xml version="1.0" ?>
<!-- 
  We can build a collection of XSL files that can be used to filter
  GGobi data files in XML to different targets.
  We can do this using xslt or alternatively use R, Python, Perl,
  etc. to work on the DOM directly. Different approaches for different
  situations.
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
		xmlns:r="http://www.r-project.org"
                version="1.0"
                exclude-result-prefixes="r"
                >


<xsl:template match="record">

</xsl:template>
</xsl:stylesheet>