


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

  * -D (database) (default pubmed)
  * -q (query) [required](required.md)
  * -d (delimiter) (default:tab)
  * -L (limit=int) (default:10)


## Example ##

The following example creates a sequence of 3 names, we search the NCBi for each name and the word "Rotavirus" in the title, limit to 2 record, we fetch each record (the PMID is in the 2nd column) and we cut the result down to 80 characters.



```

$ echo -e "#subject\nPiron\nLindenbaum\nPoncet" |\
   ncbiesearch -q '$1 "Rotavirus"[TITL]' -L 2  |\
   ncbiefetch -c 2 |\
   cut -c 1-80
#subject	pubmed.id	pubmed.year	pubmed.title	pubmed.journal	pubmed.abstract
Piron	10888646	2000	Efficient translation of rotavirus mRNA requires simultaneou
Piron	10364288	1999	Identification of the RNA-binding, dimerization, and eIF4GI-
Lindenbaum	15047801	2004	RoXaN, a novel cellular protein containing TPR, LD, and
Lindenbaum	8985320	1997	In vivo and in vitro phosphorylation of rotavirus NSP5 c
Poncet	21864538	2011	Structural Organisation of the Rotavirus Nonstructural Prot
Poncet	20935207	2010	Rapid generation of rotavirus-specific human monoclonal ant

```





