

**xsltstream** is the C++ version of my [old java program](http://code.google.com/p/lindenb/wiki/Xsltstream) (see also: [http://plindenbaum.blogspot.com/2010/02/processing-large-xml-documents-with.html](http://plindenbaum.blogspot.com/2010/02/processing-large-xml-documents-with.html) ). It applies a XSLT stylesheet
to a fragment of a large XML document. It avoids to load the whole XML document in memory.

## Download ##
Download the sources from Google-Code using [subversion:...](http://subversion.apache.org/).
```
svn checkout http://variationtoolkit.googlecode.com/svn/trunk/ variationtoolkit-read-only
```
... or update the sources of an existing installation...
```
cd variationtoolkit
svn update
```
... and edit the `variationtoolkit/congig.mk` file.

## Dependencies ##

the libxml and libxslt C libraries.

## Compilation ##



```
$ cd variationtoolkit/src/
$ make ../bin/xsltstream

g++ -o ../bin/xsltstream xsltstream.cpp xstream.o application.o  -O3 -Wall `xml2-config --cflags --libs` `xslt-config --cflags --libs` -lz
```


## Usage ##


```
xsltstream [options] (files.xml|files.xml.gz|stdin)
```


## Options ##

  * **-f** (filename) xslt stylesheet.
  * **-n** (name) target element name (default:0).
  * **-d** (int) target element depth (default:none) root is '1'.


## Example ##

Say you want to transform a large XML [DAS](http://genome.ucsc.edu/FAQ/FAQdownloads.html#download23) file from the UCSC to a set of SQL statements.
The DAS looks like this ([http://genome.ucsc.edu/cgi-bin/das/hg19/features?segment=chr2:14504273,15722020;type=knownGene](http://genome.ucsc.edu/cgi-bin/das/hg19/features?segment=chr2:14504273,15722020;type=knownGene) ):

```
<?xml version="1.0" standalone="no"?>
<!DOCTYPE DASGFF SYSTEM "http://www.biodas.org/dtd/dasgff.dtd">
<DASGFF>
<GFF version="1.0" href="http://genome.ucsc.edu/cgi-bin/das/hg19/features">
<SEGMENT id="chr2" start="14504273" stop="14622020" version="1.00" label="chr2">
<FEATURE id="uc002rby.2.chr2.14368998.0" label="uc002rby.2">
 <TYPE id="knownGene" category="transcription" reference="no">knownGene</TYPE>
 <METHOD></METHOD>
 <START>14368999</START>
 <END>14372188</END>
 <SCORE>-</SCORE>
 <ORIENTATION>-</ORIENTATION>
 <PHASE>-</PHASE>
 <GROUP id="uc002rby.2.chr2.14368998">
  <LINK href="http://genome.ucsc.edu/cgi-bin/hgTracks?position=chr2:14368998-14541082&amp;db=hg19">Link to UCSC Browser</LINK>
 </GROUP>
</FEATURE>
<FEATURE id="uc002rby.2.chr2.14368998.1" label="uc002rby.2">
 <TYPE id="knownGene" category="transcription" reference="no">knownGene</TYPE>
 <METHOD></METHOD>
 <START>14376014</START>
 <END>14376074</END>
 <SCORE>-</SCORE>
 <ORIENTATION>-</ORIENTATION>
 <PHASE>-</PHASE>
 <GROUP id="uc002rby.2.chr2.14368998">
  <LINK href="http://genome.ucsc.edu/cgi-bin/hgTracks?position=chr2:14368998-14541082&amp;db=hg19">Link to UCSC Browser</LINK>
 </GROUP>
</FEATURE>
(...)
```

And this is the XSLT stylesheet we're going to use:

```
<?xml version='1.0'  encoding="UTF-8" ?>
<xsl:stylesheet
	xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
	version='1.0'
	>
<xsl:output method="text" encoding="UTF-8"/>

<xsl:template match="/">
<xsl:apply-templates select="DASGFF"/>
</xsl:template>

<xsl:template match="DASGFF">
<xsl:apply-templates select="GFF"/>
</xsl:template>

<xsl:template match="SEGMENT">
<xsl:apply-templates select="FEATURE"/>
</xsl:template>

<xsl:template match="FEATURE">
<xsl:text>insert into DasFeature(name,chrom,chromStart,chromEnd) values(&apos;</xsl:text>
<xsl:value-of select="@label"/>
<xsl:text>&apos;,&apos;</xsl:text>
<xsl:value-of select="../@label"/>
<xsl:text>&apos;,</xsl:text>
<xsl:value-of select="START"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="END"/>
<xsl:text>);</xsl:text>
</xsl:template>
</xsl:stylesheet>
```

Here is how we transform this large XML stream on the fly.

```
curl -s "http://genome.ucsc.edu/cgi-bin/das/hg19/features?segment=chr2:14504273,15722020;type=knownGene" |\
xsltstream -f stylesheet.xsl -n FEATURE


insert into DasFeature(name,chrom,chromStart,chromEnd) values('uc002rby.2','chr2',14368999,14372188);
insert into DasFeature(name,chrom,chromStart,chromEnd) values('uc002rby.2','chr2',14376014,14376074);
insert into DasFeature(name,chrom,chromStart,chromEnd) values('uc002rby.2','chr2',14457093,14457227);
insert into DasFeature(name,chrom,chromStart,chromEnd) values('uc002rby.2','chr2',14540967,14541082);
(...)
```






