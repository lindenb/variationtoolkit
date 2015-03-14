


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
make ../bin/samplespersnp
```


## Options ##

  * --delim (char) or -d  (char) (delimiter) default:tab
  * --norefalt : don't look at REF and ALT
  * --sample SAMPLE column index
  * --chrom CHROM column index: default 1
  * --pos POS position column index: default 2
  * --ref REF reference allele column index: default 4
  * --alt ALT alternate allele column index: default 5
  * -e (query) (optional) filters by samples using boolean request eg. '((S1 && S2) | (!(S3) || "S4"))'.|
|:-------|


## Example ##

The following command line scans the VCF, sort the variations by CHROM/POS/REF/ALT/SAMPLE, counts the number of samples/variation



```

$ cat list.tsv | scanvcf  |\
  sort -t'  ' -k1,1 -k2,2n -k4,4 -k5,5 -k11,11 |\
  samplespersnp --sample 11 | awk '($8=".")'

1 753269 rs61770172 C G 99 0 . GT:GQ:DP:FLT 1/1:99:116:0 Sample16 1
1 753405 rs61770173 C A 99 0 . GT:GQ:DP:FLT 1/1:63:31:0 Sample10 7
1 753405 rs61770173 C A 81 0 . GT:GQ:DP:FLT 1/1:51:19:0 Sample12 7
1 753405 rs61770173 C A 35 0 . GT:GQ:DP:FLT 1/0:35:66:0 Sample19 7
1 753405 rs61770173 C A 99 0 . GT:GQ:DP:FLT 1/1:99:35:0 Sample3 7
1 753405 rs61770173 C A 90 0 . GT:GQ:DP:FLT 1/1:90:21:0 Sample5 7
1 753405 rs61770173 C A 99 0 . GT:GQ:DP:FLT 1/1:99:36:0 Sample6 7
1 753405 rs61770173 C A 90 0 . GT:GQ:DP:FLT 1/1:90:21:0 Sample9 7
1 876499 rs4372192 A G 39 0 . GT:GQ:DP:FLT 1/1:39:4:0 Sample12 6
1 876499 rs4372192 A G 42 0 . GT:GQ:DP:FLT 1/1:42:5:0 Sample16 6
1 876499 rs4372192 A G 39 0 . GT:GQ:DP:FLT 1/1:39:4:0 Sample17 6
1 876499 rs4372192 A G 45 0 . GT:GQ:DP:FLT 1/1:45:6:0 Sample18 6
1 876499 rs4372192 A G 45 0 . GT:GQ:DP:FLT 1/1:45:6:0 Sample4 6
1 876499 rs4372192 A G 42 0 . GT:GQ:DP:FLT 1/1:42:5:0 Sample6 6
1 877831 rs6672356 T C 42 0 . GT:GQ:DP:FLT 1/1:42:5:0 Sample14 2
1 877831 rs6672356 T C 39 0 . GT:GQ:DP:FLT 1/1:39:4:0 Sample4 2
1 878601 . C T 98 0 . GT:GQ:DP:FLT 0/1:50:11:0 Sample14 1

```



This tool also takes an option '-e' for a query over the samples. e.g: "variation must contains Sample11, Sample12 BUT NOT Sample1 to Sample5:"


```

-e '(Sample11  && Sample12 && (!(Sample1 || Sample2 || Sample3 || Sample4 || Sample5)))'

```





