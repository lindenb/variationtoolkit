<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="xml" omit-xml-declaration="yes"/>

<xsl:param name="projectid"></xsl:param>
<xsl:template match="/">
<html><body>
<xsl:apply-templates/>
</body></html>
</xsl:template>


<xsl:template match="projects">
<xsl:choose>
	<xsl:when test="$projectid=''">
	  <xsl:apply-templates select="." mode="list"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:apply-templates select="." mode="form"/>	
	</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="projects" mode="list">
<div>
<xsl:for-each select="project">
<div>
<form method="POST" action="__CGI__">
<input type="hidden" name="action" value="project.show"/>
<input type="hidden" name="project.id">
<xsl:attribute name="value"><xsl:value-of select="@id"/></xsl:attribute>
</input>
<a onclick="javascript:this.parentNode.submit();" href="#"><xsl:value-of select="name"/></a><br/>
<p><xsl:value-of select="description"/></p>

</form>
</div>
</xsl:for-each>
</div>
</xsl:template>


<xsl:template match="projects" mode="form">
<div>
<xsl:apply-templates select="project[@id=$projectid]" mode="form"/>	
</div>
</xsl:template>

<xsl:template match="project" mode="form">
<form method="POST" action="__CGI__">
<input type="hidden" name="action" value="bam.show"/>
<input type="hidden" name="project.id">
<xsl:attribute name="value"><xsl:value-of select="@id"/></xsl:attribute>
</input>
<input type="text" name="q" value=""/>
<p><xsl:value-of select="description"/></p>

</form>
</xsl:template>

</xsl:stylesheet>
