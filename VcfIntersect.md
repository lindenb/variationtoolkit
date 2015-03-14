

Compute the intersection for an ordered VCF (ordered by CHROM/POS) with another source ordered by CHROM/POS.

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

  * -f (external url/file) [required](required.md)
  * -m (mode) 0:all 1:matching 2:unmatching default:0
  * -n (string) no-match string default: NO\_MATCH
  * -c1 (CHROM col) (default:1)
  * -s1 (START col) (default:2)
  * -e1 (END col) (default:2)
  * -d1 (delimiter) (default:tab)
  * -h1  toggle: input is half open (default:0)
  * -z1  toggle: input zero-based (default:0)
  * -c2 (CHROM col) (default:1)
  * -s2 (START col) (default:2)
  * -e2 (END col) (default:2)
  * -d2 (delimiter) (default:tab)
  * -h2  toggle: input is half open (default:1)
  * -z2  toggle: input zero-based (default:1)
  * --http  force database is a URL
  * --gunzip  force database is a gzipped stream_


## Example ##

annotate a VCF with the data from snp132 at the UCSC.


```

$ echo -e "#CHROM\tPOS\nchr1\t10519\nchr1\t10520\nchr1\t10828"|\
	vcfintersect -n NO\_MATCH -c2 2 -s2 3 -e2 4  \
		-f "http://hgdownload.cse.ucsc.edu/goldenPath/hg19/database/snp132.txt.gz" |\
	verticalize -n

>>>	1
$1  	#CHROM
$2  	POS
<<<	1

>>>	2
$1  	chr1
$2  	10519
$3  	585
$4  	chr1
$5  	10518
$6  	10519
$7  	rs62636508
$8  	0
$9  	+
$10 	G
$11 	G
$12 	C/G
$13 	genomic
$14 	single
$15 	by-1000genomes
$16 	0
$17 	0
$18 	unknown
$19 	exact
$20 	1
$21 	
$22 	2
$23 	1000GENOMES,BCMHGSC_JDW,
$24 	2
$25 	G,C,
$26 	112.000000,8.000000,
$27 	0.933333,0.066667,
$28 	
<<<	2

>>>	3
$1  	chr1
$2  	10520
$3  	NO_MATCH
<<<	3

>>>	4
$1  	chr1
$2  	10828
$3  	585
$4  	chr1
$5  	10827
$6  	10828
$7  	rs10218492
$8  	0
$9  	+
$10 	G
$11 	G
$12 	A/G
$13 	genomic
$14 	single
$15 	by-cluster
$16 	0
$17 	0
$18 	unknown
$19 	exact
$20 	1
$21 	
$22 	1
$23 	WUGSC_SSAHASNP,
$24 	0
$25 	
$26 	
$27 	
$28 	
<<<	4

```








