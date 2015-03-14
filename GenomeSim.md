


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

  * -o (file.tar) output tar file. contains chromosomes and mutations.
  * -f (file) limit by genomic region (optional) read file:chrom(TAB)start(TAB)end
  * -i (file) no mutation in those genomic regions (optional) read file:chrom(TAB)start(TAB)end
  * -r (float) rate of mutations. default: 0.001
  * -R (float) fraction of indels default: 0.1
  * -X (float)  probability an indel is extended default: 0.1
  * -u (filename) read a file containing user-defined mutations (optional). Format: (CHROM)\\t(POS+1)\\t(BASE1)\\t(BASE2)
  * -m (chrom) (POS+1) (BASE1) (BASE2) insert user defined substitution. use dot('.') to not change the base.




```

$ genomesim -o chrom.tar -m chrM 10 A T -f regions.bed chrM.fa
$ tar tvf chrom.tar 
-rw-r--r-- 0/0           16795 2011-12-09 13:56 chrom/homologous1.fa
-rw-r--r-- 0/0           16793 2011-12-09 13:56 chrom/homologous2.fa
-rw-r--r-- 0/0             203 2011-12-09 13:56 chrom/mutations.txt

```





