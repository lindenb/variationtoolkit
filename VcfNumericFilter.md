

Filters a VCF-like file on a numeric value.
Prints all the data for a given CHROM/POS/REF/ALT if **ANY** of the observed numeric data falls in the given range.

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
make ../bin/vcfnumericfilter
```


## Option ##

  * **--delim** (char) or -d  (char) (delimiter) default:tab
  * **--norefalt** : don't look at REF and ALT
  * **--sample** SAMPLE column index
  * **--chrom** CHROM column index: default 1
  * **--pos** POS position column index: default 2
  * **--ref** REF reference allele column index: default 4
  * **--alt** ALT alternate allele column index: default 5
  * **-c** observed column index. REQUIRED.
  * **-m** (min-inclusive value) OPTIONAL.
  * **-M** (max-inclusive value) OPTIONAL.


## Example ##


```

echo -e "1\t100\t.\tA\tT\t1\tSAMPLE1\n1\t100\t.\tA\tT\t50\tSAMPLE2\n1\t100\t.\tA\tT\t50\tSAMPLE3\n1\t200\t.\tG\tT\t1\tSAMPLE1\n1\t200\t.\tG\tT\t5\tSAMPLE2\n1\t200\t.\tG\tT\t5\tSAMPLE3" |\
vcfnumericfilter  -c 6 

1	100	.	A	T	1	SAMPLE1
1	100	.	A	T	50	SAMPLE2
1	100	.	A	T	50	SAMPLE3
1	200	.	G	T	1	SAMPLE1
1	200	.	G	T	5	SAMPLE2
1	200	.	G	T	5	SAMPLE3

$ echo -e "1\t100\t.\tA\tT\t1\tSAMPLE1\n1\t100\t.\tA\tT\t50\tSAMPLE2\n1\t100\t.\tA\tT\t50\tSAMPLE3\n1\t200\t.\tG\tT\t1\tSAMPLE1\n1\t200\t.\tG\tT\t5\tSAMPLE2\n1\t200\t.\tG\tT\t5\tSAMPLE3" |\
vcfnumericfilter  -c 6 -m 20
1	100	.	A	T	1	SAMPLE1
1	100	.	A	T	50	SAMPLE2
1	100	.	A	T	50	SAMPLE3
```





