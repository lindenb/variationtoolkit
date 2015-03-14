


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

  * -c (column-infex) (-1)
  * --delim (column-delimiter) (default:tab)
  * -m (min-value)
  * -M (max-value)
  * -v Inverse


## Example ##

The following command line extracts the number of samples/variation and only keep the variation carried by 5 to 9 samples.


```

$ cat list.tsv | scanvcf  |\
 sort -t'  ' -k1,1 -k2,2n -k4,4 -k5,5 -k11,11 |\
 samplespersnp --sample 11 |\
 numericsplit -c 12 -m 5 -M 9 | awk '($8=".")' | head

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
1 900285 rs4970435 C T 39 0 . GT:GQ:DP:FLT 1/1:39:4:0 Sample11 9
1 900285 rs4970435 C T 42 0 . GT:GQ:DP:FLT 1/1:32:6:0 Sample12 9
1 900285 rs4970435 C T 66 0 . GT:GQ:DP:FLT 1/1:66:13:0 Sample13 9
1 900285 rs4970435 C T 42 0 . GT:GQ:DP:FLT 1/1:42:5:0 Sample14 9
1 900285 rs4970435 C T 48 0 . GT:GQ:DP:FLT 1/1:48:7:0 Sample15 9
1 900285 rs4970435 C T 66 0 . GT:GQ:DP:FLT 1/1:66:13:0 Sample16 9
1 900285 rs4970435 C T 51 0 . GT:GQ:DP:FLT 1/1:51:9:0 Sample17 9
```






