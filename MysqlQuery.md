


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

  * -d (char) delimiter default:tab
  * --host mysql host ( genome-mysql.cse.ucsc.edu)
  * --user mysql user ( genome)
  * --password mysql password ( )
  * --database mysql db ( hg19)
  * --port (int) mysql port ( default)
  * -e or -q (SQL query)
  * -L (int) limit number or rows returned


## Example ##




```

$  echo -e "#Gene\nuc001aaa.3\nHello\nuc001aac.3" |\
      mysqlquery --host localhost --user anonymous --port 3316  \
             -q 'select mRNA,description from kgXref where kgId="$1"'  |\
      verticalize 
>>>	2
$1	#Gene      	uc001aaa.3
$2	mRNA       	BC032353
$3	description	Homo sapiens mRNA for DEAD/H box polypeptide 11 like 1 (DDX11L1 gene).
<<<	2

>>>	3
$1	#Gene      	Hello
$2	mRNA       	.
$3	description	.
<<<	3

>>>	4
$1	#Gene      	uc001aac.3
$2	mRNA       	BC063459
$3	description	Homo sapiens cDNA FLJ31670 fis, clone NT2RI2004984.
<<<	4

```





