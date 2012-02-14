<?xml version='1.0' ?>
<xsl:stylesheet
	xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
	xmlns="http://www.w3.org/1999/xhtml"
	version='1.0'
	>
<xsl:output method="text" />

<xsl:param name="method">echo</xsl:param>
<xsl:template match="/">
<xsl:text>
#include &lt;stdio.h&gt;
#include &lt;stdlib.h&gt;
#include &lt;libxml/encoding.h&gt;
#include &lt;libxml/xmlwriter.h&gt;


void </xsl:text>
<xsl:value-of select="$method"/>
<xsl:text>(xmlTextWriterPtr w)
	{
	</xsl:text>
<!-- ignore first element -->
<xsl:apply-templates select="*" mode="ignore-first"/>
<xsl:text>
	}
</xsl:text>
</xsl:template>

<xsl:template match="*" mode="ignore-first">
<xsl:apply-templates select="*" />
</xsl:template>

<xsl:template match="*">
<xsl:variable name="ns">
      <xsl:value-of select="namespace-uri(.)"/>
</xsl:variable>


   <xsl:choose>
   	<xsl:when test="string-length($ns)&gt;0">
   	  <xsl:text>xmlTextWriterStartElementNS(w,BAD_CAST &quot;</xsl:text>
   	  <xsl:value-of select="substring-before(name(.),':')"/>
   	  <xsl:text>",BAD_CAST "</xsl:text>
   	  <xsl:value-of select="local-name(.)"/>
   	  <xsl:text>",BAD_CAST &quot;</xsl:text>
   	  <xsl:value-of select="$ns"/>
   	  <xsl:text>&quot;);</xsl:text>
   	</xsl:when>
   	<xsl:otherwise>
   	  <xsl:text>xmlTextWriterStartElement(w, BAD_CAST &quot;</xsl:text>
   	  <xsl:value-of select="name(.)"/>
   	  <xsl:text>&quot;)</xsl:text>
   	</xsl:otherwise>
   </xsl:choose>

<xsl:text>
</xsl:text>

   <xsl:for-each select="@*">
		<xsl:variable name="v">
			<xsl:call-template name="escape">
			   <xsl:with-param name="s">
			      <xsl:value-of select="."/>
			   </xsl:with-param>
			</xsl:call-template>
		</xsl:variable>
		<xsl:choose>
		   <xsl:when test="namespace-uri(.)=''">
		      <xsl:text>xmlTextWriterWriteAttribute(w, BAD_CAST &quot;</xsl:text>
		      <xsl:value-of select="name(.)"/>
		      <xsl:text>&quot;,BAD_CAST &quot;</xsl:text>
		      <xsl:value-of select="$v"/>
		      <xsl:text>&quot;);</xsl:text>
		   </xsl:when>
		   <xsl:otherwise>
		      <xsl:text>xmlTextWriterWriteAttributeNS(w, BAD_CAST &quot;</xsl:text>
		      <xsl:value-of select="substring-before(name(.),':')"/>
		      <xsl:text>&quot;,BAD_CAST &quot;</xsl:text>
		      <xsl:value-of select="local-name(.)"/>
	              <xsl:text>&quot;,BAD_CAST &quot;</xsl:text>    
		      <xsl:value-of select="namespace-uri(.)"/>
                      <xsl:text>&quot;,BAD_CAST &quot;</xsl:text>
                      <xsl:value-of select="$v"/>
		      <xsl:text>&quot;);</xsl:text>
                     </xsl:otherwise>
		</xsl:choose>
<xsl:text>
</xsl:text>
</xsl:for-each>



<xsl:choose>
 <xsl:when test="count(*)&gt;0">
	 <xsl:apply-templates select="*"/>
 </xsl:when>
 <xsl:when test="string-length(normalize-space(.))&gt;0">
	<xsl:apply-templates select="text()"/>
 </xsl:when>
</xsl:choose>
   
<xsl:text>xmlTextWriterEndElement(w); /* </xsl:text>
<xsl:value-of select="name(.)"/>
<xsl:text> */

</xsl:text>
</xsl:template>


<xsl:template match="text()">

<xsl:if test="string-length(normalize-space(.))&gt;0">
<xsl:text>xmlTextWriterWriteString(w,BAD_CAST &quot;</xsl:text>
<xsl:call-template name="escape">
   <xsl:with-param name="s">
      <xsl:value-of select="."/>
   </xsl:with-param>
</xsl:call-template>
<xsl:text>&quot;);
</xsl:text>
</xsl:if>
</xsl:template>




<xsl:template name="escape">
<xsl:param name="s"/><xsl:variable name="c"><xsl:value-of select="substring($s,1,1)"/></xsl:variable>
<xsl:choose>
 <xsl:when test="$c='&#xA;'">\n</xsl:when>
 <xsl:when test='$c="&#39;"'>\'</xsl:when>
 <xsl:when test="$c='&#34;'">\"</xsl:when>
 <xsl:when test="$c='\'">\\</xsl:when>
 <xsl:otherwise><xsl:value-of select="$c"/></xsl:otherwise>
</xsl:choose><xsl:if test="string-length($s) &gt;1"><xsl:call-template name="escape">
<xsl:with-param name="s"><xsl:value-of select="substring($s,2,string-length($s)-1)"/></xsl:with-param>
</xsl:call-template></xsl:if>
</xsl:template>


</xsl:stylesheet>

