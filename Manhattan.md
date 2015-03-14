


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
  * -v (int) value column default:0
  * -r (int) COLOR column (optional)
  * -s (int) SAMPLE column (optional)
  * -m (double) optional user's min value
  * -M (double) optional user's max value



## Example ##

the following command lines creates a Manhattan plot for the QUALities of a VCF file.


```

$ gunzip -c data.vcf.gz | grep -v "##" | \
   normalizechrom | cut -d '     ' -f 1,2,6 |\
   manhattan > result.ps
$ evince result.ps

```



[http://variationtoolkit.googlecode.com/svn/trunk/doc/manhattan2eps](http://variationtoolkit.googlecode.com/svn/trunk/doc/manhattan2eps)


## Example 2 ##

plotting with Samples and Colors.


```

 cat sample2vcf.tsv |\
     scanvcf | awk '($3==".")' |grep NON_SYNO |\
     cut -d '     ' -f 1,2,6,11 |\
     awk '{printf("%s\trgb(10,%d,%d)\n",$0,255-(int($3)/100.0)*255.0,(int($3)/100.0)*255.0);}'|\
     manhattan -v 3 -r 5 -s 4 > result.ps
$ evince result.ps

```



[http://variationtoolkit.googlecode.com/svn/trunk/doc/manhattan2.eps](http://variationtoolkit.googlecode.com/svn/trunk/doc/manhattan2.eps)






