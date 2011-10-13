<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="text"/>

<xsl:template match="/">
 <xsl:apply-templates select="Entrezgene-Set/Entrezgene[1]"/>
</xsl:template>

<xsl:template match="Entrezgene">
<xsl:value-of select="Entrezgene_gene/Gene-ref/Gene-ref_locus[1]"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="Entrezgene_gene/Gene-ref/Gene-ref_desc[1]"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="Entrezgene_gene/Gene-ref/maploc[1]"/>
<xsl:text>	</xsl:text>
<xsl:for-each select="Entrezgene_gene/Gene-ref/Gene-ref_db/Dbtag">
<xsl:if test='position()!=1'><xsl:text>|</xsl:text></xsl:if>
<xsl:value-of select="concat(Dbtag_db,'=')"/>
<xsl:for-each select="Dbtag_tag">
<xsl:if test='position()!=1'><xsl:text> </xsl:text></xsl:if>
<xsl:choose>
  <xsl:when test="Object-id/Object-id_str">
	<xsl:value-of select="Object-id/Object-id_str"/>
  </xsl:when>
  <xsl:otherwise>
	<xsl:value-of select="Object-id/Object-id_id"/>
  </xsl:otherwise>
</xsl:choose>
</xsl:for-each>
</xsl:for-each>
<xsl:text>	</xsl:text>
<xsl:value-of select="normalize-space(Entrezgene_summary)"/>


</xsl:template>

</xsl:stylesheet>

