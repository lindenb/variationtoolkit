

Reads an ordered VCF (ordered by CHROM/POS and optionaly by SAMPLE) , connect to an UCSC database a print the
variation in postscript.

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

  * -c (int) CHROM column default:1
  * -p (int) POS column default:2
  * -s (int) SAMPLE column (optional)
  * -r (int) COLOR column (optional)
  * --host (mysql host) default:genome-mysql.cse.ucsc.edu
  * --user (mysql user) default:genome
  * --password (mysql password) default:
  * --database (mysql database) default:hg19
  * --port (mysql password) default:0


## Example ##


![http://variationtoolkit.googlecode.com/svn/wiki/ucscgenesps.jpg](http://variationtoolkit.googlecode.com/svn/wiki/ucscgenesps.jpg)




```

$ cat sample2vcf.tsv | tr -d ' ' |\
  scanvcf |\
  awk -F '      ' '($3==".")' |\
  normalizechrom |\
  sort -t '  ' -k1,1 -k2,2n -k11,11 |\
  head -n 10000 |\
  ucscgenesps --host localhost --user username --port 3316 -s 11 > result.ps 

```





