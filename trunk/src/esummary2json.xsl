<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="text"/>

<xsl:template match="/">
<xsl:apply-templates select="eSummaryResult"/>
</xsl:template>

<xsl:template match="eSummaryResult[ERROR]">
<xsl:text>{"error":"</xsl:text>
<xsl:value-of select="ERROR"/>
<xsl:text>"}</xsl:text>
</xsl:template>

<xsl:template match="eSummaryResult[DocSum]">
<xsl:for-each select="//Item[@Name='description']">
<xsl:value-of select="."/>
<xsl:text> </xsl:text>
</xsl:for-each>
</xsl:template>


</xsl:stylesheet>

