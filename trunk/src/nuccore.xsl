<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform' version='1.0'>
<xsl:output method="text"/>

<xsl:template match="/">
 <xsl:apply-templates select="TSeqSet/TSeq"/>
</xsl:template>

<xsl:template match="TSeq">
<xsl:value-of select="TSeq_seqtype/@value"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="TSeq_accver"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="TSeq_taxid"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="TSeq_orgname"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="TSeq_defline"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="TSeq_length"/>
<xsl:text>	</xsl:text>
<xsl:value-of select="concat(substring(normalize-space(TSeq_sequence),1,500),'...')"/>
</xsl:template>

</xsl:stylesheet>
