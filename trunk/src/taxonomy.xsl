<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="text"/>

<xsl:template match="/">
 <xsl:apply-templates select="TaxaSet/Taxon[1]"/>
</xsl:template>

<xsl:template match="Taxon">
<xsl:value-of select="ScientificName"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="normalize-space(Lineage)"/>
</xsl:template>

</xsl:stylesheet>

