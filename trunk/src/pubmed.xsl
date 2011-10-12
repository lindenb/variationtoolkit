<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="text"/>

<xsl:template match="/">
 <xsl:apply-templates select="PubmedArticleSet/PubmedArticle[1]"/>
</xsl:template>

<xsl:template match="PubmedArticle">
<xsl:value-of select="MedlineCitation/DateCreated/Year"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="MedlineCitation/Article/ArticleTitle"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="MedlineCitation/Article/Journal/Title"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="concat(substring(normalize-space(MedlineCitation/Article/Abstract),1,500),'...')"/>
</xsl:template>

</xsl:stylesheet>

