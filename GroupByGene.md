


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

## Options ##

  * --delim (char) delimiter default:tab
  * --norefalt : don't look at REF and ALT
  * --sample SAMPLE column index
  * --gene GENE column index
  * --chrom CHROM column index: default 1
  * --pos POS position column index: default 2
  * -ref REF reference allele column index: default 4
  * --alt ALT alternate allele column index: default 5


## Example ##

The following command line extracts the name of the GENE, sort the data on CHROM/POS/REF/ALT/SAMPLE and group the data by gene.


```

$  cat list.tsv | scanvcf  |\
   extractinfo -t GN | awk '($12!="N/A")' |\
   sort -t '       ' -k1,1 -k2,2n -k4,4 -k5,5 -k11,11 |\
   groupbygene --gene 12 --sample 11


GENE	CHROM	START	END	count(SAMPLES)	count(distinct_MUTATION)	count(Sample1)	count(Sample2)	count(Sample3)	count(Sample4)	count(Sample5)
A1	19	58862835	58864479	5	2	2	2	2	2	2
A1CF	10	52569637	52576068	5	3	1	3	1	1	1
A2M	12	9230038	9264946	5	8	2	2	2	7	3
A2ML1	12	8990937	9020912	5	17	12	7	12	13	10
A4GALT	22	43088971	43089849	5	3	3	3	3	3	1
A4GNT	3	137843106	137850003	4	3	3	3	3	3	0
(...)

```





