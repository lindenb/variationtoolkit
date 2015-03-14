

A set of tools for GeneOntology:[http://geneontology.org](http://geneontology.org) based on the [sqlite3](http://www.sqlite.org/) library.

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

## Dependencies ##

[http://www.sqlite.org/](http://www.sqlite.org/) : libraries and headers for [sqlite3](http://www.sqlite.org/)

`libxml` [http://xmlsoft.org/](http://xmlsoft.org/) ( see also: [HowToInstall](HowToInstall.md) )

**libxslt**: the C library for xslt [http://xmlsoft.org/XSLT/](http://xmlsoft.org/XSLT/).

## Compilation ##

Define "`SQLITE_LIB`" and
"`SQLITE_CFLAGS`" in `config.mk` (see [HowToInstall](HowToInstall.md) )

```
$ cd variationtoolkit/src/
$ make ../bin/godbmgr 

if ! [ -z "-lsqlite3" ] ;then g++ -o ../bin/godbmgr godatabasemgr.cpp xsqlite.cpp application.o xstream.o xxml.o -g `xml2-config --cflags `  /usr/include/sqlite3.h   -lz -lsqlite3 `xml2-config  --libs` ; else g++ -o ../bin/godbmgr godatabasemgr.cpp  -DNOSQLITE -O3 -Wall  ; fi
```


## Usage ##


```
godbmgr (program-name) -f database.sqlite [options] (file1.vcf file2... | stdin )
```



## Program: loadrdf ##

Load the RDF/XML GO database ([http://archive.geneontology.org/latest-termdb/go\_daily-termdb.rdf-xml.gz](http://archive.geneontology.org/latest-termdb/go_daily-termdb.rdf-xml.gz)) into the [sqlite3](http://www.sqlite.org/) database.

### Usage ###


```
godbmgr loadrdf -f database.sqlite (stdin|file)
```


### Options ###

  * **-f** (filename) the [sqlite3](http://www.sqlite.org/) database


### Example ###


```
$ curl -s "http://archive.geneontology.org/latest-termdb/go_daily-termdb.rdf-xml.gz" |\
  gunzip -c |\
  godbmgr loadrdf -f database.sqlite
```

list the content of the database:

```
$ sqlite3 -separator '  ' -header  database.sqlite 'select * from TERM where acn="GO:0000007"'
acn	xml
GO:0000007	<go:term xmlns:go="http://www.geneontology.org/dtds/go.dtd#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" rdf:about="http://www.geneontology.org/go#GO:0000007">
            <go:accession xmlns:go="http://www.geneontology.org/dtds/go.dtd#">GO:0000007</go:accession>
            <go:name xmlns:go="http://www.geneontology.org/dtds/go.dtd#">low-affinity zinc ion transmembrane transporter activity</go:name>
            <go:definition xmlns:go="http://www.geneontology.org/dtds/go.dtd#">Catalysis of the transfer of a solute or solutes from one side of a membrane to the other according to the reaction: Zn2+ = Zn2+, probably powered by proton motive force. In low affinity transport the transporter is able to bind the solute only if it is present at very high concentrations.</go:definition>
            <go:is_a xmlns:go="http://www.geneontology.org/dtds/go.dtd#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" rdf:resource="http://www.geneontology.org/go#GO:0005385"/>
        </go:term>


$ sqlite3 -separator '  ' -header  database.sqlite 'select * from TERM2REL where acn="GO:0000007"'
acn	rel	target
GO:0000007	is_a	GO:0005385
```


## Program: loadgoa ##

inserts the database for GOA into a [sqlite3](http://www.sqlite.org/) database (e.g: [ftp://ftp.ebi.ac.uk/pub/databases/GO/goa/HUMAN/gene\_association.goa\_human.gz](ftp://ftp.ebi.ac.uk/pub/databases/GO/goa/HUMAN/gene_association.goa_human.gz))

### Usage ###


```
godbmgr loadgoa -f database.sqlite (stdin|file)
```


### Options ###

  * **-f** (filename) the [sqlite3](http://www.sqlite.org/) database


### Examples ###


```
$  curl -s "ftp://ftp.ebi.ac.uk/pub/databases/GO/goa/HUMAN/gene_association.goa_human.gz" |\
     gunzip -c |\
     godbmgr loadgoa -f database.sqlite
```

list the content of the database:

```


$ sqlite3 -line   database.sqlite 'select * from GOA where term="GO:0005385" limit 2' 
              DB = UniProtKB
    DB_Object_ID = B3KU87
DB_Object_Symbol = SLC30A6
            term = GO:0005385
  DB_Object_Name = cDNA FLJ45816 fis, clone NT2RP7019682, highly similar to Homo sapiens solute carrier family 30 (zinc transporter), member 6 (SLC30A6), mRNA
         Synonym = B3KU87_HUMAN|SLC30A6|hCG_23082|IPI01009565|B7WP49
  DB_Object_Type = protein

              DB = UniProtKB
    DB_Object_ID = B5MCR8
DB_Object_Symbol = SLC30A6
            term = GO:0005385
  DB_Object_Name = Solute carrier family 30 (Zinc transporter), member 6, isoform CRA_b
         Synonym = B5MCR8_HUMAN|SLC30A6|hCG_23082|IPI00894292
  DB_Object_Type = protein


```


## Program: desc ##

print the descendants (children) of a given GO node.

### Usage ###


```
godbmgr desc -f db.sqlite [options] term1 term2 ... termn
```


### Options ###

  * **-f** (filename) the [sqlite3](http://www.sqlite.org/) database
  * **-r** (rel) add a go relationship ([http://www.obofoundry.org/ro/](http://www.obofoundry.org/ro/)) (OPTIONAL, default: it adds "is\_a").
  * **-t** output: xml, goa ,tsv ,acn


### Examples ###


#### Default output ####


```
$ godbmgr desc -f database.sqlite "GO:0005385"
GO:0000006
GO:0000007
GO:0005385
GO:0015341
GO:0015633
GO:0016463
GO:0022883
```


#### xml/rdf output ####


```

$ godbmgr desc -f database.sqlite  -t xml "GO:0005385" | head

<go:go xmlns:go='http://www.geneontology.org/dtds/go.dtd#' xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>
 <rdf:RDF>
<go:term xmlns:go="http://www.geneontology.org/dtds/go.dtd#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" rdf:about="http://www.geneontology.org/go#GO:0000006">
            <go:accession xmlns:go="http://www.geneontology.org/dtds/go.dtd#">GO:0000006</go:accession>
            <go:name xmlns:go="http://www.geneontology.org/dtds/go.dtd#">high affinity zinc uptake transmembrane transporter activity</go:name>
            <go:definition xmlns:go="http://www.geneontology.org/dtds/go.dtd#">Catalysis of the transfer of a solute or solutes from one side of a membrane to the other according to the reaction: Zn2+(out) = Zn2+(in), probably powered by proton motive force. In high affinity transport the transporter is able to bind the solute even if it is only present at very low concentrations.</go:definition>
            <go:is_a xmlns:go="http://www.geneontology.org/dtds/go.dtd#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" rdf:resource="http://www.geneontology.org/go#GO:0005385"/>
        </go:term>
<go:term xmlns:go="http://www.geneontology.org/dtds/go.dtd#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" rdf:about="http://www.geneontology.org/go#GO:0000007">
            <go:accession xmlns:go="http://www.geneontology.org/dtds/go.dtd#">GO:0000007</go:accession>

```


#### GOA output ####


```
$godbmgr desc -f database.sqlite -t goa  "GO:0005385"

UniProtKB	B3KU87	SLC30A6	GO:0005385	cDNA FLJ45816 fis, clone NT2RP7019682, highly similar to Homo sapiens solute carrier family 30 (zinc transporter), member 6 (SLC30A6), mRNA	B3KU87_HUMAN|SLC30A6|hCG_23082|IPI01009565|B7WP49	protein
UniProtKB	B5MCR8	SLC30A6	GO:0005385	Solute carrier family 30 (Zinc transporter), member 6, isoform CRA_b	B5MCR8_HUMAN|SLC30A6|hCG_23082|IPI00894292	protein
(..)
UniProtKB	Q99726	SLC30A3	GO:0015633	Zinc transporter 3	ZNT3_HUMAN|ZNT3|SLC30A3|IPI00293793|Q8TC03protein

```


#### TSV output ####


```
$ godbmgr desc -f database.sqlite  -t tsv "GO:0022857" |\
    cut -c 1-100 |\
    head
#go:accession	go.name	go.def
GO:0000006	high affinity zinc uptake transmembrane transporter activity	Catalysis of the transfer of
GO:0000007	low-affinity zinc ion transmembrane transporter activity	Catalysis of the transfer of a s
GO:0000064	L-ornithine transmembrane transporter activity	Catalysis of the transfer of L-ornithine f
GO:0000095	S-adenosylmethionine transmembrane transporter activity	Catalysis of the transfer of S-ad
GO:0000099	sulfur amino acid transmembrane transporter activity	Catalysis of the transfer of sulfur 
GO:0000100	S-methylmethionine transmembrane transporter activity	Catalysis of the transfer of S-meth
GO:0000102	L-methionine secondary active transmembrane transporter activity	Catalysis of the transfe
GO:0000227	oxaloacetate secondary active transmembrane transporter activity	Catalysis of the transfe
GO:0000269	toxin export channel activity	Enables the energy independent passage of toxins, sized les
(...)

```



## Program: asc ##

prints the ascendants (parents) of a given node.

### Usage ###


```
godbmgr asc -f db.sqlite [options] term1 term2 ... termn
```


### Options ###

  * **-f** (filename) the [sqlite3](http://www.sqlite.org/) database
  * **-r** (rel) add a go relationship ([http://www.obofoundry.org/ro/](http://www.obofoundry.org/ro/)) (OPTIONAL, default: it adds "is\_a").
  * **-t** output: xml, goa ,tsv ,acn


### Examples ###


#### Default output ####


```
$ godbmgr asc -f database.sqlite "GO:0022857"
GO:0003674
GO:0005215
GO:0022857
all
```


#### xml/rdf output ####


```

$ godbmgr asc -f database.sqlite  -t xml "GO:0022857" | head

<go:go xmlns:go='http://www.geneontology.org/dtds/go.dtd#' xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>
 <rdf:RDF>
<go:term xmlns:go="http://www.geneontology.org/dtds/go.dtd#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" rdf:about="http://www.geneontology.org/go#GO:0003674">
            <go:accession xmlns:go="http://www.geneontology.org/dtds/go.dtd#">GO:0003674</go:accession>
            <go:name xmlns:go="http://www.geneontology.org/dtds/go.dtd#">molecular_function</go:name>
            <go:synonym xmlns:go="http://www.geneontology.org/dtds/go.dtd#">GO:0005554</go:synonym>
            <go:synonym xmlns:go="http://www.geneontology.org/dtds/go.dtd#">molecular function</go:synonym>
            <go:synonym xmlns:go="http://www.geneontology.org/dtds/go.dtd#">molecular function unknown</go:synonym>
            <go:definition xmlns:go="http://www.geneontology.org/dtds/go.dtd#">Elemental activities, such as catalysis or binding, describing the actions of a gene product at the molecular level. A given gene product may exhibit one or more molecular functions.</go:definition>
            <go:comment xmlns:go="http://www.geneontology.org/dtds/go.dtd#">Note that, in addition to forming the root of the molecular function ontology, this term is recommended for use for the annotation of gene products whose molecular function is unknown. Note that when this term is used for annotation, it indicates that no information was available about the molecular function of the gene product annotated as of the date the annotation was made; the evidence code ND, no data, is used to indicate this.</go:comment>

```


#### GOA output ####


```
$godbmgr desc -f database.sqlite -t goa  "GO:0005385"

UniProtKB	B3KU87	SLC30A6	GO:0005385	cDNA FLJ45816 fis, clone NT2RP7019682, highly similar to Homo sapiens solute carrier family 30 (zinc transporter), member 6 (SLC30A6), mRNA	B3KU87_HUMAN|SLC30A6|hCG_23082|IPI01009565|B7WP49	protein
UniProtKB	B5MCR8	SLC30A6	GO:0005385	Solute carrier family 30 (Zinc transporter), member 6, isoform CRA_b	B5MCR8_HUMAN|SLC30A6|hCG_23082|IPI00894292	protein
(..)
UniProtKB	Q99726	SLC30A3	GO:0015633	Zinc transporter 3	ZNT3_HUMAN|ZNT3|SLC30A3|IPI00293793|Q8TC03protein

```


#### TSV output ####


```
$ godbmgr asc -f database.sqlite  -t tsv "GO:0022857"   
#go:accession	go.name	go.def
GO:0003674	molecular_function	Elemental activities, such as catalysis or binding, describing the actions of a gene product at the molecular level. A given gene product may exhibit one or more molecular functions.
GO:0005215	transporter activity	Enables the directed movement of substances (such as macromolecules, small molecules, ions) into, out of or within a cell, or between cells.
GO:0022857	transmembrane transporter activity	Enables the transfer of a substance from one side of a membrane to the other.
all	all	.

```



## program goa ##

Annotate a TSV file with the GOA annotation.

### Usage ###


```
godbmgr goa -f db.sqlite [options] (stdin|files)
```


### Options ###

  * **-f** (filename) the [sqlite3](http://www.sqlite.org/) database
  * **-c** (column index) REQUIRED. The observed column.


### Example ###


```
$ echo -e "#MyGene\nHello\nNOTCH2" |\
   godbmgr goa -c 1 -f database.sqlite  |\
   head -n 4 |\
   verticalize

>>>	2
$1	#MyGene               	Hello
$2	DB                    	.
$3	DB_Object_ID          	.
$4	DB_Object_Symbol      	.
$5	term                  	.
$6	DB_Object_Name,Synonym	.
$7	DB_Object_Type        	.
$8  	???                   	.
<<<	2

>>>	3
$1	#MyGene               	NOTCH2
$2	DB                    	UniProtKB
$3	DB_Object_ID          	Q04721
$4	DB_Object_Symbol      	NOTCH2
$5	term                  	GO:0001709
$6	DB_Object_Name,Synonym	Neurogenic locus notch homolog protein 2
$7	DB_Object_Type        	NOTC2_HUMAN|NOTCH2|IPI00297655|Q5T3X7|Q99734|Q9H240
$8  	???                   	protein
<<<	3

>>>	4
$1	#MyGene               	NOTCH2
$2	DB                    	UniProtKB
$3	DB_Object_ID          	Q04721
$4	DB_Object_Symbol      	NOTCH2
$5	term                  	GO:0004872
$6	DB_Object_Name,Synonym	Neurogenic locus notch homolog protein 2
$7	DB_Object_Type        	NOTC2_HUMAN|NOTCH2|IPI00297655|Q5T3X7|Q99734|Q9H240
$8  	???                   	protein
```


## Program: grep ##

filters the line having an identifier (gene...) that is a children of a given GO term.

### Usage ###


```
godbmgr grep -f db.sqlite [options] (stdin|files)
```


### Options ###

  * **-f** (filename) the [sqlite3](http://www.sqlite.org/) database
  * **-c** (column index) REQUIRED. The column containing a GO:acn
  * **-v** inverse selection
  * **-t** (GO:acn) add a GO term in the filter (One is REQUIRED).
  * **-r** (rel) add a go relationship  ([http://www.obofoundry.org/ro/](http://www.obofoundry.org/ro/)) (OPTIONAL, default: it adds "is\_a").


### Example ###


```
$ 
$ echo -e "#MyACN\nGO:0003674\nGO:0001618\n" |\
  godbmgr grep -f database.sqlite -c 1 -t GO:0004872 -t GO:0004879 

#MyACN
GO:0001618
$ echo -e "#MyACN\nGO:0003674\nGO:0001618\n" |\
  godbmgr grep -f database.sqlite -c 1 -t GO:0004872 -t GO:0004879 -v

#MyACN
GO:0003674
```






