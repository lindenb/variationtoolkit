

Search for short-read names in a BAM file. Answer to http://www.biostars.org/post/show/48211.

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
make ../bin/bamgrepreads
```


## Usage ##


```
bamgrepreads -R file.txt [options] (stdin|bam1 bam2 ...)
```


## Options ##

  * **-R**(file) list of reads names (REQUIRED).
  * **-f** (int) required flag (OPTIONAL)
  * **-F** (int) filtering flag (OPTIONAL)
  * **-e** one match per read (goes faster)


## Example ##


```
$echo "HTKZQN1_353:6:1103:6755:52161" > list.txt
$ bamgrepreads -R list.txt -F 4 -e input.bam

HTKZQN1_353:6:1103:6755:52161   163     chr2    10012   254     100M    =      10184    272     AAGACAACCACCCTCACCCTCACCCTCACCCTCACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCTAACCCAAA    CCBFFFFFHGHHHJJJJJJJJIJJJIJIJJJGIJJIIGGIJJIJIIJGIIIJJJJJHHHHFFFFFEDDCDDDDDDCDDDDDDDDDDDCDBDDDDDDDDDB    AM:i:0  BC:Z:0 XD:Z:CTA2CCTA5A5A5A5A66C SM:i:0  AS:i:425

```





