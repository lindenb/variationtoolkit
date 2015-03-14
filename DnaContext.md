


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

  * -c (chrom Column) (1)
  * -p|-s (pos or start Column) (2)
  * -d (column-delimiter) (default:tab)
  * -x (segment-size) (default:10)
  * -e (end-column) . exclusive of -x . Assumes BED data (first base=0)
  * -f (genome file indexed with tabix
  * --no-gc don't print gc percent
  * --no-seq don't print DNA sequence


## Example ##




```

$ gunzip -c data.vcf.gz |\
  grep -v "##" | normalizechrom |\
  dnacontext -f hg19.fa  |\
  awk '($8=".")'

#CHROM POS ID REF ALT QUAL FILTER . FORMAT CALL LEFT(DNA) CONTEXT(DNA) RIGHT(DNA)
chr1 879317 rs7523549 C T 71 0 . GT:GQ:DP:FLT 0/1:34:8:0 GAGTTTTCTA C GTGGCCAGCT
chr1 880238 rs3748592 A G 51 0 . GT:GQ:DP:FLT 1/1:51:8:0 AGCCAGCCTT A GAGGTTACTC
chr1 880390 rs3748593 C A 99 0 . GT:GQ:DP:FLT 1/0:99:30:0 TGCCCTCCCG C CAGATGGGCT
chr1 881627 rs2272757 G A 99 0 . GT:GQ:DP:FLT 1/0:59:20:0 TACAAGGTCA G GGGTGTCCCC
chr1 883625 rs4970378 A G 39 0 . GT:GQ:DP:FLT 1/1:39:4:0 GAAGAGCAGG A GAGAGGGCCG
chr1 887560 rs3748595 A C 99 0 . GT:GQ:DP:FLT 1/1:99:40:0 CCAGGCTGAC A AGTCAGGCTG
(...)

```





