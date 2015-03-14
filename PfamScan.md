


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
  * -p (column-index) column containing the amino acid index.
  * -a (acn) column containing the protein-acn (e.g.: Q04721 or  IF4G1\_HUMAN)._


## Example ##



```

$ echo -e "#ACN\tPOS\nQ04721\t1650\nIF4G1_HUMAN\t1540\nQ04721\t300\nHello\t9980\n.\t." |\
	pfamscan -a 1 -p 2 |\
	verticalize


>>>	2
$1	#ACN      	Q04721
$2	POS       	1650
$3	pfam.beg  	1617
$4	pfam.end  	1677
$5	pfam.acn  	PF07684
$6	pfam.id   	NODP
$7	pfam.type 	Pfam-A
$8	pfam.class	.
<<<	2

>>>	3
$1	#ACN      	IF4G1_HUMAN
$2	POS       	1540
$3	pfam.beg  	1518
$4	pfam.end  	1599
$5	pfam.acn  	PF02020
$6	pfam.id   	W2
$7	pfam.type 	Pfam-A
$8	pfam.class	.
<<<	3

>>>	4
$1	#ACN      	Q04721
$2	POS       	300
$3	pfam.beg  	298
$4	pfam.end  	338
$5	pfam.acn  	PF07645
$6	pfam.id   	EGF_CA
$7	pfam.type 	Pfam-A
$8	pfam.class	.
<<<	4

>>>	5
$1	#ACN      	Hello
$2	POS       	9980
$3	pfam.beg  	.
$4	pfam.end  	.
$5	pfam.acn  	.
$6	pfam.id   	.
$7	pfam.type 	.
$8	pfam.class	.
<<<	5

>>>	6
$1	#ACN      	.
$2	POS       	.
$3	pfam.beg  	.
$4	pfam.end  	.
$5	pfam.acn  	.
$6	pfam.id   	.
$7	pfam.type 	.
$8	pfam.class	.
<<<	6

```





