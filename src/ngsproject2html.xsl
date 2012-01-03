<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="xml" omit-xml-declaration="yes"/>

<xsl:key name="refindex" match="/projects/reference" use="@id" />
<xsl:key name="bamindex" match="/projects/bam" use="@id" />

<xsl:param name="projectid"></xsl:param>
<xsl:param name="scriptname">undefined</xsl:param>
<xsl:template match="/">
<div>
<xsl:apply-templates/>
</div>
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
<h1 style="text-align:center;">NGS Projects</h1>
<div>
<xsl:for-each select="project">
<div>
<form method="GET">
<xsl:attribute name="action"><xsl:value-of select="$scriptname"/></xsl:attribute>
<input type="hidden" name="action" value="project.show"/>
<input type="hidden" name="project.id">
<xsl:attribute name="value"><xsl:value-of select="@id"/></xsl:attribute>
</input>
<div>
<a href="#" title="go to this project" style="font-size:200%;" onclick="javascript:this.parentNode.parentNode.submit();"><xsl:value-of select="name"/></a><br/>
<p><xsl:value-of select="description"/></p>
<dl>
<dt>Reference</dt>
<dd><xsl:value-of select="key('refindex',reference/@ref)/name"/>: <xsl:value-of select="key('refindex',reference/@ref)/description"/></dd>
<dt>Number of BAMs:</dt>
<dd><xsl:value-of select="count(bam)"/> BAM(s)</dd>
</dl>
</div>
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
<form method="GET">
<xsl:attribute name="action"><xsl:value-of select="$scriptname"/></xsl:attribute>
<input type="hidden" name="action" value="bam.show"/>
<input type="hidden" name="project.id">
<xsl:attribute name="value"><xsl:value-of select="@id"/></xsl:attribute>
</input>
<div style="text-align:center;font-size:200%;">
<xsl:value-of select="name"/>
<br/>
<label for="query">Position: </label>
<input type="text" name="q" value="" id="query"/>
</div>
<p><xsl:value-of select="description"/></p>
<dl>
<dt>Reference</dt>
<dd><xsl:value-of select="key('refindex',reference/@ref)/name"/>: <xsl:value-of select="key('refindex',reference/@ref)/description"/></dd>
<dt>BAMs:</dt>
<xsl:for-each select="bam">
<dd>&quot;<xsl:value-of select="key('bamindex',@ref)/sample"/>&quot; <span><xsl:value-of select="key('bamindex',@ref)/path"/></span></dd>
</xsl:for-each>
</dl>

</form>
</xsl:template>

</xsl:stylesheet>
