


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
cd variationtoolkit/src/
make ../bin/uniprot
```


## Options ##

  * -d (char) delimiter default:tab
  * -p (column-index) column containing the amino acid index.
  * -s (spId-index) column containing the swissprot-acn  (e.g.: Q04721 or  NOTC2\_HUMAN)._


## Example ##




```

$ echo -e "#POS\tID\n54\tQ04721\n1\tHELLO\n166\tP03536" |\
    uniprot -p 1 -s 2 |\
    verticalize 
#warning: Cannot find record for HELLO
>>>	2
$1	#POS            	54
$2	ID              	Q04721
$3	uniprot.beg     	26
$4	uniprot.end     	2471
$5	uniprot.type    	chain
$6	uniprot.status  	.
$7	uniprot.desc    	Neurogenic locus notch homolog protein 2
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	2

>>>	3
$1	#POS            	54
$2	ID              	Q04721
$3	uniprot.beg     	26
$4	uniprot.end     	1677
$5	uniprot.type    	topological domain
$6	uniprot.status  	potential
$7	uniprot.desc    	Extracellular
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	3

>>>	4
$1	#POS            	54
$2	ID              	Q04721
$3	uniprot.beg     	26
$4	uniprot.end     	63
$5	uniprot.type    	domain
$6	uniprot.status  	.
$7	uniprot.desc    	EGF-like 1
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	4

>>>	5
$1	#POS            	54
$2	ID              	Q04721
$3	uniprot.beg     	53
$4	uniprot.end     	62
$5	uniprot.type    	disulfide bond
$6	uniprot.status  	by similarity
$7	uniprot.desc    	.
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	5

>>>	6
$1	#POS            	1
$2	ID              	HELLO
$3	uniprot.beg     	.
$4	uniprot.end     	.
$5	uniprot.type    	.
$6	uniprot.status  	.
$7	uniprot.desc    	.
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	6

>>>	7
$1	#POS            	166
$2	ID              	P03536
$3	uniprot.beg     	1
$4	uniprot.end     	315
$5	uniprot.type    	chain
$6	uniprot.status  	.
$7	uniprot.desc    	Non-structural protein 3
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	7

>>>	8
$1	#POS            	166
$2	ID              	P03536
$3	uniprot.beg     	150
$4	uniprot.end     	206
$5	uniprot.type    	region of interest
$6	uniprot.status  	.
$7	uniprot.desc    	Dimerization
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	8

>>>	9
$1	#POS            	166
$2	ID              	P03536
$3	uniprot.beg     	166
$4	uniprot.end     	237
$5	uniprot.type    	coiled-coil region
$6	uniprot.status  	potential
$7	uniprot.desc    	.
$8	uniprot.evidence	.
$9	uniprot.ref     	.
<<<	9

```





