


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

  * -p (column) protein name column default:1
  * -s (column) protein start column (1-based) default:2
  * -e (column) protein end column (1-based)  default:3
  * -d (delimiter). Default:tab
  * --host (mysql host) default:genome-mysql.cse.ucsc.edu
  * --user (mysql user) default:genome
  * --password (mysql password) default:
  * --database (mysql database) default:hg19
  * --port (mysql password) default:0


## Example ##



```

$ echo -e "#Pep\tpepStart\tpepEnd\tDomain\nZC3H7B\t82\t115\tTPR\nNOTC2_HUMAN\t26\t63\tEGF_DOMAIN" |\
  protein2genome | verticalize 
  
  
>>>	2
$1	#Pep            	ZC3H7B
$2	pepStart        	82
$3	pepEnd          	115
$4	Domain          	TPR
$5	knownGene.name  	uc003azw.2
$6	knownGene.chrom 	chr22
$7	knownGene.strand	+
$8	knownGene.exon  	Exon 4
$9	domain.start    	41721879
$10	domain.end      	41721922
<<<	2

>>>	3
$1	#Pep            	ZC3H7B
$2	pepStart        	82
$3	pepEnd          	115
$4	Domain          	TPR
$5	knownGene.name  	uc003azw.2
$6	knownGene.chrom 	chr22
$7	knownGene.strand	+
$8	knownGene.exon  	Exon 5
$9	domain.start    	41723209
$10	domain.end      	41723268
<<<	3

>>>	4
$1	#Pep            	NOTC2_HUMAN
$2	pepStart        	26
$3	pepEnd          	63
$4	Domain          	EGF_DOMAIN
$5	knownGene.name  	uc001eik.2
$6	knownGene.chrom 	chr1
$7	knownGene.strand	-
$8	knownGene.exon  	Exon 2
$9	domain.start    	120572528
$10	domain.end      	120572609
<<<	4

>>>	5
$1	#Pep            	NOTC2_HUMAN
$2	pepStart        	26
$3	pepEnd          	63
$4	Domain          	EGF_DOMAIN
$5	knownGene.name  	uc001eik.2
$6	knownGene.chrom 	chr1
$7	knownGene.strand	-
$8	knownGene.exon  	Exon 3
$9	domain.start    	120548178
$10	domain.end      	120548211
<<<	5

```





