

Annotates a VCF with the data from the [Exome Variant Server](http://evs.gs.washington.edu/EVS/) stored in a [sqlite3](http://www.sqlite.org/) database.

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

## Compilation ##


```
$ cd varkit/src
make ../bin/vcfevs
```


## Creating the SQLITE database ##

The following java program dump all the data as XML from the EVS server:
GIST : https://gist.github.com/2604781


Compile and run:


```
javac DumpExomeVariantServerData
java DumpExomeVariantServerData > input.evs.tsv
```


Create the  [sqlite3](http://www.sqlite.org/) database and insert the data:


```
$ sqlite3 evs.sqlite
sqlite> create table evsData(chrom TEXT NOT NULL,pos INT NOT NULL,xml TEXT NOT NULL);
sqlite> create index chrompos on evsData(chrom,pos);
sqlite> .separator "\t";
sqlite> .import "input.evs.tsv" evsData

```


We can now query this sql database :


```
$
$ sqlite3 evs.sqlite 'select xml from evsData where chrom="1" and pos=69552' | xmllint  --format -

<?xml version="1.0"?>
<snpList>
  <positionString>1:69552</positionString>
  <chrPosition>69552</chrPosition>
  <alleles>C/G</alleles>
  <uaAlleleCounts>C=4/G=4644</uaAlleleCounts>
  <aaAlleleCounts>C=0/G=2944</aaAlleleCounts>
  <totalAlleleCounts>C=4/G=7588</totalAlleleCounts>
  <uaMAF>0.0861</uaMAF>
  <aaMAF>0.0</aaMAF>
  <totalMAF>0.0527</totalMAF>
  <avgSampleReadDepth>143</avgSampleReadDepth>
  <geneList>OR4F5</geneList>
  <snpFunction>
    <chromosome>1</chromosome>
    <position>69552</position>
    <conservationScore>1.0</conservationScore>
    <conservationScoreGERP>-0.1</conservationScoreGERP>
    <snpFxnList>
      <mrnaAccession>NM_001005484.1</mrnaAccession>
      <fxnClassGVS>coding-synonymous</fxnClassGVS>
      <aminoAcids>none</aminoAcids>
      <proteinPos>154/306</proteinPos>
      <cdnaPos>462</cdnaPos>
      <pphPrediction>unknown</pphPrediction>
      <granthamScore>NA</granthamScore>
    </snpFxnList>
    <refAllele>G</refAllele>
    <ancestralAllele>C</ancestralAllele>
    <firstRsId>55874132</firstRsId>
    <secondRsId>0</secondRsId>
    <filters>SVM</filters>
    <clinicalLink>unknown</clinicalLink>
  </snpFunction>
  <conservationScore>1.0</conservationScore>
  <conservationScoreGERP>-0.1</conservationScoreGERP>
  <refAllele>G</refAllele>
  <altAlleles>C</altAlleles>
  <ancestralAllele>C</ancestralAllele>
  <chromosome>1</chromosome>
  <hasAtLeastOneAccession>true</hasAtLeastOneAccession>
  <rsIds>rs55874132</rsIds>
  <filters>SVM</filters>
  <clinicalLink>unknown</clinicalLink>
  <dbsnpVersion>dbSNP_129</dbsnpVersion>
  <uaGenotypeCounts>CC=0/CG=4/GG=2320</uaGenotypeCounts>
  <aaGenotypeCounts>CC=0/CG=0/GG=1472</aaGenotypeCounts>
  <totalGenotypeCounts>CC=0/CG=4/GG=3792</totalGenotypeCounts>
  <onExomeChip>false</onExomeChip>
  <gwasPubmedIds>unknown</gwasPubmedIds>
</snpList>
```




## Options ##

  * **-f**(file) sqlite filename. (REQUIRED).
  * **-c** (column-name) limit header to this column name (can be used multiple times)
  * **-a** (column-index) column for alternal allele (optional)
  * **-x** (column-name) print xml


## Examples ##


```
$
export LD_LIBRARY_PATH=/path/to/sqlite3/lib #if needed

$ echo -e "#CHROM\tPOS\n1\t69511\n1\t69512\n1\t69552" |\
vcfevs -f evs.sqlite 

#CHROM	POS	evs.positionString	evs.chrPosition	evs.alleles	evs.uaAlleleCounts	evs.aaAlleleCounts	evs.totalAlleleCounts	evs.uaMAF	evs.aaMAF	evs.totalMAF	evs.avgSampleReadDepth	evs.geneList	evs.conservationScore	evs.conservationScoreGERP	evs.refAllele	evs.altAllelesevs.ancestralAllele	evs.chromosome	evs.hasAtLeastOneAccession	evs.rsIds	evs.filters	evs.clinicalLink	evs.dbsnpVersion	evs.uaGenotypeCounts	evs.aaGenotypeCounts	evs.totalGenotypeCounts	evs.onExomeChip	evs.gwasPubmedIds
1	69511	1:69511	69511	G/A	G=4235/A=483	G=1707/A=1297	G=5942/A=1780	10.2374	43.1758	23.051	74	OR4F5	1.0	1.1	A	G	G1	true	rs75062661	PASS	unknown	dbSNP_131	GG=1964/GA=307/AA=88	GG=703/GA=301/AA=498	GG=2667/GA=608/AA=586	false	unknown
1	69512	.	.	.	.	.	.	.	.	.	.	.	.	.	.	.	.	.	.	..	.	.	.	.	.	.	.
1	69552	1:69552	69552	C/G	C=4/G=4644	C=0/G=2944	C=4/G=7588	0.0861	0.0	0.0527	143	OR4F5	1.0	-0.1	G	C	C1	true	rs55874132	SVM	unknown	dbSNP_129	CC=0/CG=4/GG=2320	CC=0/CG=0/GG=1472	CC=0/CG=4/GG=3792	false	unknown


$ echo -e "#CHROM\tPOS\n1\t69511\n1\t69512\n1\t69552" |\
vcfevs -f ~/WORK/20120506.evs.download/evs.sqlite -c uaMAF 
#CHROM	POS	evs.uaMAF
1	69511	10.2374
1	69512	.
1	69552	0.0861


$ echo -e "#CHROM\tPOS\n1\t69511\n1\t69512\n1\t69552" |\
vcfevs -f evs.sqlite  -x -c _ | cut -c 1-200

#CHROM	POS	evs.xml
1	69511	<snpList><positionString>1:69511</positionString><chrPosition>69511</chrPosition><alleles>G/A</alleles><uaAlleleCounts>G=4235/A=483</uaAlleleCounts><aaAlleleCounts>G=1707/A=1297</aaAlleleCount
1	69512	.
1	69552	<snpList><positionString>1:69552</positionString><chrPosition>69552</chrPosition><alleles>C/G</alleles><uaAlleleCounts>C=4/G=4644</uaAlleleCounts><aaAlleleCounts>C=0/G=2944</aaAlleleCounts><to

```





