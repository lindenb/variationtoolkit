


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

  * -d (char) column delimiter. default: TAB
  * -c (int) chromosome column (1).
  * -p (int) pos column (2).
  * -f (filename) tabix file (required).
  * -1 remove 1 to the VCF coodinates.
  * -S (NOT-FOUND-String) default:!N/A.
  * -m  (int=mode) 0)=all 1:only-matching  2:only-non-matching default:0.


## Example ##

> download some 1000G data:


```

 curl  -s "ftp://ftp-trace.ncbi.nih.gov/1000genomes/ftp/release/20100804/ALL.2of4intersection.20100804.sites.vcf.gz" |\
            gunzip -c | grep -v "##" |\
            head -n 10000 > ~/1000G.vcf

```


Index this file with tabix.


```

$ bgzip 1000G.vcf
$ tabix -p vcf 1000G.vcf.gz
<pre><![CDATA[

```


Compute the intersection of this file with our VCF. Only retain the matching line (option -m 1)


```

$ gunzip -c data.vcf.gz |\
  grep -v "##"  |  normalizechrom -E | vcftabix  -f 1000G.vcf.gz -m 1 |\
  awk '($8=".")' |  awk '($18=".")'
#CHROM POS ID REF ALT QUAL FILTER . FORMAT Call #CHROM POS ID REF ALT QUAL FILTER .
1 879317 rs7523549 C T 71 0 . GT:GQ:DP:FLT 0/1:34:8:0 1 879317 rs7523549 C T . PASS .
1 880238 rs3748592 A G 51 0 . GT:GQ:DP:FLT 1/1:51:8:0 1 880238 rs3748592 A G . PASS .
1 880390 rs3748593 C A 99 0 . GT:GQ:DP:FLT 1/0:99:30:0 1 880390 rs3748593 C A . PASS .
1 881627 rs2272757 G A 99 0 . GT:GQ:DP:FLT 1/0:59:20:0 1 881627 rs2272757 G A . PASS .
1 883625 rs4970378 A G 39 0 . GT:GQ:DP:FLT 1/1:39:4:0 1 883625 rs4970378 A G . PASS .
1 887560 rs3748595 A C 99 0 . GT:GQ:DP:FLT 1/1:99:40:0 1 887560 rs3748595 A C . PASS .
1 887801 rs3828047 A G 99 0 . GT:GQ:DP:FLT 1/1:99:32:0 1 887801 rs3828047 A G . PASS .
1 888639 rs3748596 T C 99 0 . GT:GQ:DP:FLT 1/1:99:32:0 1 888639 rs3748596 T C . PASS .
1 888659 rs3748597 T C 99 0 . GT:GQ:DP:FLT 1/1:99:26:0 1 888659 rs3748597 T C . PASS .
(...)

```





