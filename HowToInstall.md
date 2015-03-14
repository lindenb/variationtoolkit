



## Dependencies ##

  * **Samtools**: Utilities for post-processing alignments in the SAM format. [pmid:19505943](http://www.ncbi.nlm.nih.gov/pubmed/19505943)
  * **Tabix**: fast retrieval of sequence features from generic TAB-delimited files. [pmid:21208982](http://www.ncbi.nlm.nih.gov/pubmed/21208982)
  * **mysql-dev**: the files and libraries for the mysql database.
  * **libxml2**: the C library for xml [http://xmlsoft.org/](http://xmlsoft.org/).
  * **libxslt**: the C library for xslt [http://xmlsoft.org/XSLT/](http://xmlsoft.org/XSLT/).
  * **libcurl**: the C library for downloading URLs.
  * **jkentsrc**: (optional) Jim Kent's C library / Ucsc parsing bigbed, bigwig...
  * **sqlite3**: (optional) SQL database engine ([http://www.sqlite.org](http://www.sqlite.org)).


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

## Building ##

Download the latest version of varkit using `subversion`:


```
svn checkout http://variationtoolkit.googlecode.com/svn/trunk/ variationtoolkit-read-only
```


or update your current version by calling `svn update` in the  variationtoolkit folder.

```
svn update
```


## Edit the Config file. ##

Edit the file variationtoolkit/config.mk . You'll have to set the path to the sources of tabix (>=0.2.5), samtools (>=-0.1.18), etc..

```
##path to SAMTOOLS
SAMDIR=${HOME}/samtools-0.1.18
##path to TABIX
TABIXDIR=/home/lindenb/tabix-0.2.5
##optional path to UCSC kent's src
KENTDIR=${HOME}/src/kent/kent
##optional path to google leveldb
LEVELDBDIR=${HOME}/tmp/leveldb-read-only
SQLITE_LIB=-lsqlite3
SQLITE_CFLAGS=
```


then type:

```

make
```

the binaries will be generated in the **bin/** folder.

## Compiling one program without dependencies ##

If you don't want to install all the programs and just beed one program you can go in the `src` folder and ask for compiling the software. e.g:

```
cd src
make ../bin/fastatail
$ make ../bin/fasta2tsv
g++ -o ../bin/fasta2tsv fasta2tsv.cpp -O3 -Wall -lz
```





