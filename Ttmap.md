


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

## Options: ##

  * **-c** (int) chrom column
  * **-s** (int) start column
  * **-e** (int) end column default: start column
  * **-o** (int) strand column default (optional)
  * **-n** (int) name column default (optional)
  * **-d** (char) delimiter default:tab
  * **-C** (int) fix the number of output columns.



## Example: ##


```
$ curl -s "http://hgdownload.cse.ucsc.edu/goldenPath/hg19/database/knownGene.txt.gz" |\
  gunzip -c |  grep chrM |\
  ttmap -c 2 -s 4 -e 5 -o 3 -n 1 -C 50

>chrM:236-15998
<              .              .              .     uc004coq.3
    >          .              .              .     uc004cor.1
    >          .              .              .     uc004cos.3
               . >            .              .     uc011mfh.1
               .       >      .              .     uc004cou.3
               .          >   .              .     uc011mfi.1
               .              . >            .     uc004cov.3
               .              .      >       .     uc004cow.1
               .              .       >      .     uc004cox.3
               .              .              .>    uc004coy.2
```






