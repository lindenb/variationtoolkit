


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

  * **-z** (int) number of depth=0 accepted before starting a new WIG file
  * **-o** (filename-out) save as... (default:stdout).
  * **-t** print a ucsc custom track header.



```
$ bam2wig -t  file.bam

track name="__TRACK_NAME__" description="__TRACK_DESC__" type="wiggle_0"
fixedStep chrom=chrM start=23 step=1 span=1
2
2
3
4
5
6
6
4
(...)
```






