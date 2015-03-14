The Variation Toolkit

  * <a href='http://code.google.com/p/variationtoolkit/source/browse/trunk/'>Sources</a>
| **Title** | **Description** |
|:----------|:----------------|
| [HowToInstall](HowToInstall.md) | How to Install the Variation toolkit |
| [NgsProject](NgsProject.md) | CGI interface to Samtools tview. Visualize short reads alignments |
| [Vcf2Sqlite](Vcf2Sqlite.md) | Inserts a VCF in a sqlite3 database |
| [Vcf2xml](Vcf2xml.md) | Transforms a VCF to xml |
| [FastaSlice](FastaSlice.md) | Slice a FASTA input |
| [Scanvcf](Scanvcf.md) | reads some VCFs, appends a column with the sample name |
| [ExtractInfo](ExtractInfo.md) | Extract a field from the INFO column of a VCF file |
| [ExtractFormat](ExtractFormat.md) | Extract a field from the FORMAT column of a VCF file. |
| [NcbiEFetch](NcbiEFetch.md) | Fetch a record from the NCBI database |
| [SamplesPerSnp](SamplesPerSnp.md) | Appends a column with the number of Samples per Variation |
| [GroupBySnp](GroupBySnp.md) | Creates a pivot table with the data(samples)=f(SNP) |
| [NumericSplit](NumericSplit.md) | A simple numeric splitter |
| [GroupByGene](GroupByGene.md) | transposes a VCF table with a 'GENE' and a 'SAMPLE' column and ouput a new table: count(Gene)=f(SAMPLE) |
| [NormalizeChrom](NormalizeChrom.md) | Normalizes the name of a chromosome to/from UCSC/ENSEMBL |
| [DnaContext](DnaContext.md) | Prints the DNA context of a variation using a genome indexed with samtools faidx and its GC percent |
| [Prediction](Prediction.md) | basic variation predictor |
| [Manhattan](Manhattan.md) | draws a manhattan plot as a postscript file |
| [NcbiESearch](NcbiESearch.md) | Search NCBI/Entrez |
| [VcfTTView](VcfTTView.md) | Prints the BAM alignments around variations. |
| [VCFTabix](VCFTabix.md) | Intersection VCF/Tabix |
| [MysqlQuery](MysqlQuery.md) | Sends a mysql query for each row |
| [MysqlUcsc](MysqlUcsc.md) | Intersection VCF/UCSC mysql data. |
| [VcfBigWig](VcfBigWig.md) | Intersection VCF/BigWig |
| [VcfBigBed](VcfBigBed.md) | Intersection VCF/BigBed |
| [Verticalize](Verticalize.md) | Verticalize a table |
| [UniProt](UniProt.md) | Take as input a position on a protein and an uniprot ACN, connect to uniprot.org and answers wether a amino acid is contained in a 'feature' |
| [PfamScan](PfamScan.md) | Take as input a position on a protein and an uniprot ACN, connect to  pfam.sanger.ac.uk and answers wether a amino acid is contained in a 'match'. |
| [Vcf2bed](Vcf2bed.md) | Generates a BED file from a VCF |
| [EmblStringSesolve](EmblStringSesolve.md) | Calls the service: EMBL String resolve |
| [EmblStringInteractors](EmblStringInteractors.md) | Calls the service: EMBL String interactors |
| [EmblStringInteractions](EmblStringInteractions.md) | Calls the service: EMBL String interactions |
| [VcfCut](VcfCut.md) | (very) simple 'cut this region' |
| [UcscGenesPs](UcscGenesPs.md) | Visualization of the mutations in the UCSC genes |
| [VcfIntersect](VcfIntersect.md) | Compute the intersection for an ordered VCF/BED with another source |
| [Bam2wig](Bam2wig.md) | Creates a WIG file for the coverage of a BAM file |
| [Ttmap](Ttmap.md) | prints an ASCII genomic map |
| [VcfLiftOver](VcfLiftOver.md) | Use the UCSC C API to process the data with 'liftOver' |
| [BackLocate](BackLocate.md) | convert a protein variation to a genomic position |
| [GenomeSim](GenomeSim.md) | Generates two mutated homologous sequences from a fasta file |
| [FastaSortUniq](FastaSortUniq.md) | sort/uniq on FASTA sequences |
| [FastaRevComp](FastaRevComp.md) | reverse complement FASTA sequences |
| [FastaTail](FastaTail.md) | Prints the last sequences of a list of FASTA sequences |
| [Fasta2Term](Fasta2Term.md) | colorizes some fasta sequences when printing to stdout. |
| [Fasta2Tsv](Fasta2Tsv.md) | prints Fasta Sequences as Tab delimited values |
| [FastaTac](FastaTac.md) | Reverse the order of some fasta sequences |
| [NcbiELink](NcbiELink.md) | Retrieves related NCBI records using NCBI-ELink |
| [NcbiCited](NcbiCited.md) | Retrieves  NCBI/Pubmed records citing a pmid using NCBI-ELink |
| [Protein2Genome](Protein2Genome.md) | Maps a protein start/end back to the genome |
| [XsltStream](XsltStream.md) | applies a XSLT stylesheet to some fragments of a SAX/XML stream. |
| [GeneOntologyDbManager](GeneOntologyDbManager.md) | Utilities for Gene Ontology |
| [ShowProgress](ShowProgress.md) | Displays a progress bar on the console |
| [Casava2Vcf](Casava2Vcf.md) | Transforms Casava SNPs and INDEL to VCF |
| [VcfId](VcfId.md) | fills the ID columns of a VCF using UCSC/snpX |
| [VcfEvs](VcfEvs.md) | Annotates a VCF with the data from exome variant server |
| [VcfNumericFilter](VcfNumericFilter.md) | Filters a VCF-like file on a numeric value |
| [TeeC](TeeC.md) | Tee for counting the rows |
| [BamGrepReads](BamGrepReads.md) | Search for short-read names in a BAM |
| [BamNoCoverage](BamNoCoverage.md) | prints the uncovered region in a BAM file |
| [AddHeader](AddHeader.md) | inserts a header before a stream |
| [FastQGrep](FastQGrep.md) | search for reads in one ot two (paired) FASTQ files. |
| [GenomeCoverage](GenomeCoverage.md) | Computes the mean coverage for each chromosomes |
| [BedDepth](BedDepth.md) | Computes the mean depth for each Bam from a BED |
| [BamStats04](BamStats04.md) | Computes a statistics about the fraction of covered bases in a Bam. |
| [BamStats01](BamStats01.md) | Computes some statistics about the reads in a Bam. |
| [BamStats05](BamStats05.md) |  Depth of coverage for each bases for one or more BAM  |