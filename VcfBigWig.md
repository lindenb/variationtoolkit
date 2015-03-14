


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

  * -f (bigwig file)
  * -d (delimiter) (default:tab)
  * -c (CHROM column=int) (default:1)
  * -p (POS column=int) (default:2)
  * -x (extend=int) extends window size (default:0)


## Example ##

What's in the Big wig ?


```

$ kent/src/hg/encode/validateFiles/tests/test4.bw file.wig
$ cat file.wig

#bedGraph section chr1:1-1099
chr1    1       1000    54
chr1    1000    1099    53

```



let's get the intersection of a VCF file with this BIGWIG file.



```

$ echo -e "#CHROM\tPOS\nchr1\t500\nchr1\t1001"  |\
  vcfbigwig -f  kent/src/hg/encode/validateFiles/tests/test4.bw
  
#CHROM	POS	bigwig:min	bigwig:max	bigwig:mean	bigwig:coverage	bigwig:stddev
chr1	500	54	54	54	1	0
chr1	1001	53	53	53	1	0

$ echo -e "#CHROM\tPOS\nchr1\t500\nchr1\t1001"  |\
  vcfbigwig -x 100 -f  kent/src/hg/encode/validateFiles/tests/test4.bw
  
#CHROM	POS	bigwig:min	bigwig:max	bigwig:mean	bigwig:coverage	bigwig:stddev
chr1	500	54	54	54	1	0
chr1	1001	53	54	53.5025	0.99005	0.501255

$ echo -e "#CHROM\tPOS\nchrX\t500\nchrX\t1001"  |\
  vcfbigwig  -f  kent/src/hg/encode/validateFiles/tests/test4.bw
  
#CHROM	POS	bigwig:min	bigwig:max	bigwig:mean	bigwig:coverage	bigwig:stddev
chrX	500	nan	nan	nan	nan	nan
chrX	1001	nan	nan	nan	nan	nan

```







