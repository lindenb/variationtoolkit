


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

  * -p show pmids.
  * -c (column) index of column containing gi identifier for database-from.


## Example ##



```

$ echo -e "#query\nNOTCH2"  |\
  ncbiesearch  -q 'Caignec $1' | \
  ncbicited -p -c 2

#query	pubmed.id	count.citing	citing.pmid
NOTCH2	21793104	.	.
NOTCH2	21718643	.	.
NOTCH2	21378989	2	21984761
NOTCH2	21378989	2	22046109
```




