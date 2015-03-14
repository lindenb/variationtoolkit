


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

```

$ curl -s "ftp://ftp-trace.ncbi.nih.gov/1000genomes/ftp/release/20100804/ALL.2of4intersection.20100804.sites.vcf.gz" |\
   gunzip -c | grep -v "##" |\
   vcfcut -e '2:10kb+500bp;1:10000-20000'
   
#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO
1	10327	rs112750067	T	C	.	PASS	DP=65;AF=0.208;CB=BC,NCBI
1	10469	rs117577454	C	G	.	PASS	DP=2055;AF=0.020;CB=UM,BC,NCBI
1	10492	rs55998931	C	T	.	PASS	DP=231;AF=0.167;CB=BC,NCBI
(...)
1	16841	.	G	T	.	PASS	DP=2906;AF=0.004;CB=UM,BI;EUR_R2=0.248
2	10038	.	C	A	.	PASS	DP=73;AF=0.409;CB=BC,NCBI
2	10075	.	C	A	.	PASS	DP=31;AF=0.150;CB=BC,NCBI
2	10144	.	C	A	.	PASS	DP=33;AF=0.562;CB=BC,NCBI
2	10159	.	C	A	.	PASS	DP=32;AF=0.222;CB=BC,NCBI
2	10205	.	T	G	.	PASS	DP=582;AF=0.107;CB=UM,BC
2	10297	.	G	T	.	PASS	DP=500;AF=0.246;CB=UM,BC
2	10363	.	G	A	.	PASS	DP=788;AF=0.016;CB=UM,BI;EUR_R2=0.273;AFR_R2=0.034
2	10437	rs71337607	C	T	.	PASS	DP=324;AF=0.267;CB=UM,BC

```







