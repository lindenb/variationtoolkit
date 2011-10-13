<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet
	version='1.0'
	xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
	xmlns:s="http://www.ncbi.nlm.nih.gov/SNP/docsum">
<xsl:output method="text"/>

<xsl:template match="/">
 <xsl:apply-templates select="s:ExchangeSet/s:Rs[1]"/>
</xsl:template>

<xsl:template match="s:Rs">
<xsl:value-of select="s:Het/@value"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="@bitField"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="s:Sequence[1]/s:Seq5"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="s:Sequence[1]/s:Observed"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="s:Sequence[1]/s:Seq5"/>
<xsl:text>	</xsl:text>
<xsl:for-each select="s:Assembly[@groupLabel]">
<xsl:variable name="groupLabel" select="@groupLabel"/>
<xsl:for-each select="s:Component[@chromosome]">
<xsl:variable name="component" select="@chromosome"/>
<xsl:for-each select="s:MapLoc[@physMapInt]">
<xsl:value-of 
select="concat($groupLabel,':',$component,':',@physMapInt,'|')"/>
</xsl:for-each>
</xsl:for-each>
</xsl:for-each>
</xsl:template>


</xsl:stylesheet>

