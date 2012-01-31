<?xml version='1.0' encoding="UTF-8"?>
<xsl:stylesheet
	xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	version='1.0'
	>
<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>


<xsl:template match="/">
<config>
<xsl:apply-templates select="resultset[row]"/>
</config>
</xsl:template>

<xsl:template match="resultset">
<xsl:variable name="table" select="normalize-space(substring-after(@statement,'desc '))"/>
<table>
<xsl:attribute name="id"><xsl:value-of select="concat('type',$table)"/></xsl:attribute>
<xsl:attribute name="label"><xsl:value-of select="$table"/></xsl:attribute>
<xsl:attribute name="desc"><xsl:value-of select="$table"/></xsl:attribute>
<xsl:for-each select="row">
<xsl:variable name="type" select="field[@name='Type']"/>
<column>
<xsl:attribute name="name"><xsl:value-of select="field[@name='Field']"/></xsl:attribute>
<xsl:attribute name="dataType">
	<xsl:choose>
		<xsl:when test="starts-with($type ,'int(')">xsd:int</xsl:when>
		<xsl:when test="starts-with($type ,'double(')">xsd:double</xsl:when>
		<xsl:otherwise>xsd:string</xsl:otherwise>
	</xsl:choose>
</xsl:attribute>
</column>
</xsl:for-each>
</table>

<instance>
<xsl:attribute name="id"><xsl:value-of select="$table"/></xsl:attribute>
<xsl:attribute name="table-ref"><xsl:value-of select="concat('type',$table)"/></xsl:attribute>
<xsl:attribute name="label"><xsl:value-of select="$table"/></xsl:attribute>
<xsl:attribute name="desc"><xsl:value-of select="$table"/></xsl:attribute>
<xsl:attribute name="path"><xsl:value-of select="concat('/var/www/cgi-bin/',$table,'.txt.gz')"/></xsl:attribute>
</instance>
</xsl:template>





</xsl:stylesheet>

