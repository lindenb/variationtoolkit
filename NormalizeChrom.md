


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

  * -i (string) ignore lines starting with this string.
  * -d (string) column delimiter (default:tab).
  * -c (int) column index (+1) (default:1).
  * -E convert to ENSEMBL syntax (default is UCSC).


## Example ##



```

$ echo -e "1\t10\nX\t20\nMT\t30" | normalizechrom 

chr1	10
chrY	20
chrM	30

$ echo -e "chr1\t10\nX\t20\nchrM\t30" | normalizechrom -E

1	10
X	20
MT	30

```





