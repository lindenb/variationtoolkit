


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

  * -f (ncbi-database-from) REQUIRED.
  * -t (ncbi-database-to) REQUIRED.
  * -d (delimiter) (default:tab)
  * -c (column) index of column containing gi identifier for database-from.


## Example ##



```

$ echo -e "#Genbank_GI\n256041817" | ncbielink  -c 1 -f protein -t nucleotide

#Genbank_GI	protein:nucleotide.linkName	protein:nucleotide.id
256041817	protein_nuccore	256041816
256041817	protein_nuccore_mrna	256041816
```





