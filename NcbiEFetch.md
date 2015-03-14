

Currently supported databases: pubmed , nucleotide, protein, snp , gene and taxonomy.

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
  * -d (delimiter) (default:tab)
  * -c (column=int)
  * -f (sqlite-database-file) (OPTIONAL) store the XML records in a [sqlite3](http://www.sqlite.org/) database. Make things faster next time this record will be asked.


## Example ##

The following example generates a sequences of 6 pubmed ID and we call ncbiefetch to download the records.


```

$  (echo "#GI"; seq 1000 2 1010)   |\
      ncbiefetch -c 1 |\
      cut -c 1-100

#GI	pubmed.year	pubmed.title	pubmed.journal	pubmed.abstract
1000	1976	The amino acid sequence of Neurospora NADP-specific glutamate dehydrogenase. The tryptic p
1002	1976	The amino acid sequence of Neurospora NADP-specific glutamate dehydrogenase. Peptic and ch
1004	1976	Properties of 5-aminolaevulinate synthetase and its relationship to microsomal mixed-funct
1006	1976	The attachment of glutamine synthetase to brain membranes.	Biochemical medicine	...
1008	1976	Nature and possible origin of human serum ribonuclease.	Biochemical and biophysical resear
1010	1976	Formation of non-amidine products in the chemical modification of horse liver alcohol dehy

```



The following example creates a sequence of 3 gi, we fetch each record (the gi is in the 1st column) and we cut the result down to 80 characters.



```

$ (echo "#GI"; seq 5 2 10)  | ncbiefetch -c 1 -D nucleotide | cut -c 1-80
#GI	nucleotide.type	nucleotide.accver	nucleotide.taxid	nucleotide.orgname	nucleo
5	nucleotide	X60065.1	9913	Bos taurus	B.bovis beta-2-gpI mRNA for beta-2-glycopr
7	nucleotide	X51700.1	9913	Bos taurus	Bos taurus mRNA for bone Gla protein	437	G
9	nucleotide	X68321.1	9913	Bos taurus	B.taurus mRNA for cyclin A	1512	GAATTCCAGG

```



Let's download some data for 3 rs\#\# from dbsnp.


```

echo -e "#RS\nrs25\nrs26\nrs27"  | ncbiefetch -c 1 -D snp

#RS	snp.het	snp.bitField	snp.seq5	snp.obs	snp.seq3	snp.map
rs25	0	050100080001030500120101	AGTAAGAGGAATCAATGTCATAGGCTTTAGATAGCATTTATGACTGTGTGCTCGTGTGTGTGAAAACT..
rs26	0	050100080011000100000700	AAATGTGTGACCAAGAAAATGACtttttttttttccgactgtgtctcgctctgttgccaggctggagt..
rs27	0	050100080001030100100100	TCTATGTCCAGAACTATGGATATATATTGACCTTAACTGTCAAGTATATACAAAAGAGCCAAACTGCA..

```



Taxonomy


```

$ echo -e "#Taxon-id\n9606\n9605"  | ncbiefetch -c 1 -D taxonomy

#Taxon-id	taxon.name	taxon.lineage
9606	Homo sapiens	cellular organisms; Eukaryota; Opisthokonta; Metazoa; Eumetazo...
9605	Homo	cellular organisms; Eukaryota; Opisthokonta; Metazoa; Eumetazoa; Bilat...

```



Gene:


```

$ (echo "#Gene-Id"; seq 105 2 110)  | ncbiefetch -c 1 -D gene

#Gene-Id	gene.locus	gene.desc	gene.maploc	gene.ids	gene.summary
105	ADARB2	adenosine deaminase, RNA-specific, B2		HGNC=227|Ensembl=ENSG000001857
107	ADCY1	adenylate cyclase 1 (brain)		HGNC=232|Ensembl=ENSG00000164742|HPRD=000
109	ADCY3	adenylate cyclase 3		HGNC=234|Ensembl=ENSG00000138031|HPRD=02620|MIM=6

```


Using [sqlite3](http://www.sqlite.org/) :

```
$echo -e "25\n26" | ncbiefetch -D snp -c 1 -f entrez.sqlite
(...)
$ echo "select * from entrez;" | sqlite3 -header -line entrez.sqlite  | cut -c 1-300
id = 25
db = snp
xml = <?xml version="1.0"?>
<ExchangeSet xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.ncbi.nlm.nih.gov/SNP/docsum" xsi:schemaLocation="http://www.ncbi.nlm.nih.gov/SNP/docsum ftp://ftp.ncbi.nlm.nih.gov/snp/specs/docsum_3.3.xsd"><Rs rsId="25" snpClass="snp" snpType="notwithdrawn" molType="genomic" geno
</ExchangeSet>


id = 26
db = snp
xml = <?xml version="1.0"?>
<ExchangeSet xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://www.ncbi.nlm.nih.gov/SNP/docsum" xsi:schemaLocation="http://www.ncbi.nlm.nih.gov/SNP/docsum ftp://ftp.ncbi.nlm.nih.gov/snp/specs/docsum_3.3.xsd"><Rs rsId="26" snpClass="mixed" snpType="notwithdrawn" molType="genomic" bi
</ExchangeSet>
```






