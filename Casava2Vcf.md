

Transforms Casava SNPs and INDEL to VCF

## Warning ##

This tool has been poorly designed and it only worked for an old version of Casava. You should really look for another software.

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
make ../bin/casava2vcf
```


## Usage ##


```
casava2vcf *.txt > result.vcf 
```


## Example ##


```
$ casava2vcf *_indels.txt *_snps.txt

##FORMAT=<ID=DP,Number=1,Type=Integer,Description="Basecalls mapped to the site but filtered out before genotype calling">
#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO	FORMAT	__SAMPLE__
chr22.fa	16052167	.	A	AAAAC	365	.	INSERTION=4;DP=5;alt_reads=5;indel_reads=7;other_reads=14;repeat_unit=AAAC;ref_repeat_count=8;indel_repeat_count=9	GT:GQ:DP	0/1:203:5
(...)
chr22.fa	51243297	.	A	T	10	.	DP=1;bcalls_used=1;Q_snp=10;max_gt=AT;Q_max_gt=3;max_gt_poly_site=AT;Q_max_gt_poly_site=AT;A_used=0;C_used=0;G_used=0;T_used=1	GT:GQ:DP	0/1:3:0

```





