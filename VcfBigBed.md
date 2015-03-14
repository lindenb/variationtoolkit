


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

  * -f (BigBed file)
  * -d (delimiter) (default:tab)
  * -c (CHROM column=int) (default:1)
  * -p (POS column=int) (default:2)
  * -x (extend=int) extends window size (default:0)
  * -L (limit=int) limit to L records in bed (default:unbound)
  * -S (NOT-FOUND-String) default:!N/A.
  * -m  (int=mode) 0)=all 1:only-matching  2:only-non-matching default:0.


## Example ##

What's in the Big BED ?


```

$ cat test.bed 
chr7    115000000       116000000       100.0
chr7    115500000       116500000       200.0
chr7    116000000       117000000       100.0
chr8    1000000         2000000         1000
chr8    1100000         1200000         1000
chr8    1100000         1200000         1000
chr9    100     200     10
chr9    100     300     10
chr9    100     400     10
chr9    1000    2000    10
chr9    1200    2000    10
chr9    1300    2000    10

```



let's get the intersection of a VCF file with this BigBed file.



```

$ echo -e "#CHROM\tPOS\nchr9\t1250\nchrX\t1"  |\
  vcfbigbed  -f  test.bb 

#CHROM	POS	BigBed:chromStart	BigBed:chromEnd	BigBed:rest
chr9	1250	1000	2000	10
chr9	1250	1200	2000	10
chrX	1	!N/A	!N/A	!N/A

$ echo -e "#CHROM\tPOS\nchr9\t1250\nchrX\t1"  |\
  vcfbigbed  -x 1000 -f  test.bb 

#CHROM	POS	BigBed:chromStart	BigBed:chromEnd	BigBed:rest
chr9	1250	100	300	10
chr9	1250	100	400	10
chr9	1250	1000	2000	10
chr9	1250	1200	2000	10
chr9	1250	1300	2000	10
chrX	1	!N/A	!N/A	!N/A


```





