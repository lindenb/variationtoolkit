


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

  * -d (column-delimiter) (default:tab)
  * -f genome file indexed with samtools faidx.
  * -c (CHROM col) (default:0)
  * -p (POS col) (default:1)
  * -r (REF col) (default:3)
  * -a (ALT col) (default:4)
  * --host (mysql host) default:genome-mysql.cse.ucsc.edu
  * --user (mysql user) default:genome
  * --password (mysql password) default:
  * --database (mysql database) default:hg19
  * --port (mysql password) default:0


## Example ##



```

$ mysql -e 'select chrom as "#CHROM",cdsStart+1 as "POS","." as "ID","A" as "REF" ,"G" as "ALT" from knownGene where strand="+" and cdsStart<cdsEnd limit 10' -D hg19 |\
   prediction -f hg19.fa  | \
   verticalize | cut -c 1-80
   
>>>	2
$1	#CHROM                   	chr1
$2	POS                      	12190
$3	REF                      	A
$4	ALT                      	G
$5	knownGene.name           	uc001aaa.3
$6	knownGene.strand         	+
$7	knownGene.txStart        	11873
$8	knownGene.txEnd          	14409
$9	knownGene.cdsStart       	11873
$10	knownGene.cdsEnd         	11873
$11	prediction.type          	UTR3
$12	prediction.pos_in_cdna   	.
$13	prediction.pos_in_protein	.
$14	prediction.exon          	.
$15	prediction.intron        	.
$16	prediction.wild.codon    	.
$17	prediction.mut.codon     	.
$18	prediction.wild.aa       	.
$19	prediction.mut.aa        	.
$20	prediction.wild.prot     	.
$21	prediction.mut.prot      	.
$22	prediction.wild.rna      	.
$23	prediction.mut.rna       	.
$24	prediction.splicing      	.
<<<	2

>>>	3
$1	#CHROM                   	chr1
$2	POS                      	12190
$3	REF                      	A
$4	ALT                      	G
$5	knownGene.name           	uc010nxq.1
$6	knownGene.strand         	+
$7	knownGene.txStart        	11873
$8	knownGene.txEnd          	14409
$9	knownGene.cdsStart       	12189
$10	knownGene.cdsEnd         	13639
$11	prediction.type          	EXON|EXON_CODING_NON_SYNONYMOUS
$12	prediction.pos_in_cdna   	0
$13	prediction.pos_in_protein	1
$14	prediction.exon          	Exon 1
$15	prediction.intron        	.
$16	prediction.wild.codon    	ATG
$17	prediction.mut.codon     	GTG
$18	prediction.wild.aa       	M
$19	prediction.mut.aa        	V
$20	prediction.wild.prot     	MSESINFSHNLGQLLSPPRCVVMPGMPFPSIRSPELQKTTADLDHTLVSV
$21	prediction.mut.prot      	VSESINFSHNLGQLLSPPRCVVMPGMPFPSIRSPELQKTTADLDHTLVSV
$22	prediction.wild.rna      	ATGAGTGAGAGCATCAACTTCTCTCACAACCTAGGCCAGCTCCTGTCTCC
$23	prediction.mut.rna       	GTGAGTGAGAGCATCAACTTCTCTCACAACCTAGGCCAGCTCCTGTCTCC
$24	prediction.splicing      	.
<<<	3

>>>	4
$1	#CHROM                   	chr1
$2	POS                      	12190
$3	REF                      	A
$4	ALT                      	G
$5	knownGene.name           	uc010nxr.1
$6	knownGene.strand         	+
$7	knownGene.txStart        	11873
$8	knownGene.txEnd          	14409
$9	knownGene.cdsStart       	11873
$10	knownGene.cdsEnd         	11873
$11	prediction.type          	UTR3
$12	prediction.pos_in_cdna   	.
$13	prediction.pos_in_protein	.
$14	prediction.exon          	.
$15	prediction.intron        	.
$16	prediction.wild.codon    	.
$17	prediction.mut.codon     	.
$18	prediction.wild.aa       	.
$19	prediction.mut.aa        	.
$20	prediction.wild.prot     	.
$21	prediction.mut.prot      	.
$22	prediction.wild.rna      	.
$23	prediction.mut.rna       	.
$24	prediction.splicing      	.
<<<	4

>>>	5
$1	#CHROM                   	chr1
$2	POS                      	69091
$3	REF                      	A
$4	ALT                      	G
$5	knownGene.name           	uc001aal.1
$6	knownGene.strand         	+
$7	knownGene.txStart        	69090
$8	knownGene.txEnd          	70008
$9	knownGene.cdsStart       	69090
$10	knownGene.cdsEnd         	70008
$11	prediction.type          	EXON|EXON_CODING_NON_SYNONYMOUS
$12	prediction.pos_in_cdna   	0
$13	prediction.pos_in_protein	1
$14	prediction.exon          	Exon 1
$15	prediction.intron        	.
$16	prediction.wild.codon    	ATG
$17	prediction.mut.codon     	GTG
$18	prediction.wild.aa       	M
(...)

```





