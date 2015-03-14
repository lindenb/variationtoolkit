


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

  * --delim (char) delimiter default:tab
  * --host mysql host ( genome-mysql.cse.ucsc.edu)
  * --user mysql user ( genome)
  * --password mysql password ( )
  * --database mysql db ( hg19)
  * --port (int) mysql port ( default)
  * --table or -T (string)
  * -C (int) chromosome column (first is 1).
  * -S (int)start column (first is 1).
  * -E (int) end column (first is 1).
  * -f first column is not header.
  * -1 data are +1 based.
  * --limit (int) limit number or rows returned
  * --field (string) set custom field. Can be used several times
  * --type (int) type of selection: 0 any (default), 1 user data IN mysql data,2 user data embrace mysql data. (stdin|files)


## Example ##

Compute the intersection of our data with ucsc.snp132 keep the lines containing the word 'syn'.


```

$ gunzip -c data.vcf.gz |\
   grep -v "##"  |  normalizechrom |\
   mysqlucsc --host myhost --user mypassword -C 1 -S 2 -E 2 --table snp132    |\
   awk '($8=".")' | grep -i syn |\
   head
   
chr1 16375063 rs45612832 C G 67 0 . GT:GQ:DP:FLT 0/1:67:53:0 709 chr1 16375063 16375064 rs1889790 0 + A A A/C genomic single by-cluster,by-frequency 0.4488 0.151587 coding-synon,near-gene-5 exact 1 NonIntegerChromCount 9 BCMHGSC_JDW,BCM_SSAHASNP,BGI,HGSV,SC_SNP,SEATTLESEQ,SSAHASNP,TSC-CSHL,UCSF_HG, 2 A,C, 15.980000,31.020000, 0.340000,0.660000, maf-5-some-pop,maf-5-all-pops
chr1 16375063 rs45612832 C G 67 0 . GT:GQ:DP:FLT 0/1:67:53:0 709 chr1 16375063 16375064 rs45575235 0 + A A A/C genomic single by-cluster,by-frequency 0 0 coding-synon,near-gene-5 exact 2 MultipleAlignments 3 ENSEMBL,GMI,PHARMGKB_PCE, 1 C, 2.000000, 1.000000, maf-5-some-pop,maf-5-all-pops
chr1 16890671 rs55951643 T C 99 0 . GT:GQ:DP:FLT 1/0:99:1177:0 713 chr1 16890671 16890672 rs2419525 0 - G G C/T genomic single by-cluster,by-2hit-2allele,by-hapmap 0.18 0.24 coding-synon exact 1 10 BCMHGSC_JDW,BGI,CSHL-HAPMAP,ENSEMBL,GMI,HGSV,SC_JCM,SC_SNP,TSC-CSHL,WI_SSAHASNP, 2 T,C, 9.000000,1.000000, 0.900000,0.100000, maf-5-some-pop,maf-5-all-pops
chr1 16890671 rs55951643 T C 99 0 . GT:GQ:DP:FLT 1/0:99:1177:0 713 chr1 16890671 16890672 rs17409315 0 - G G C/T cDNA single unknown 0 0 coding-synon exact 3 MultipleAlignments 1 SEQUENOM, 0
chr1 22176831 rs2290500 C T 87 0 . GT:GQ:DP:FLT 0/1:47:9:0 754 chr1 22176683 22176855 rs2229485 0 - TGCTGGGGACAGAGGGCAAAGGGTCAATAGCCGGCTAGGAGGTGAGATGAGATGGGGCTCCTGGTCTCAAGGCAGGTGCAGTCTGCGGCTTGGCCTCCTGATCCTGCCGTTGCAAGAGTGGGGGGCCTCCCACCCTGGGTCCCCAGCCCTGCCCTCCCTGAGAGCTACTCAC TGCTGGGGACAGAGGGCAAAGGGTCAATAGCCGGCTAGGAGGTGAGATGAGATGGGGCTCCTGGTCTCAAGGCAGGTGCAGTCTGCGGCTTGGCCTCCTGATCCTGCCGTTGCAAGAGTGGGGGGCCTCCCACCCTGGGTCCCCAGCCCTGCCCTCCCTGAGAGCTACTCAC A/T cDNA single by-frequency 0.120708 0.213971 coding-synon rangeInsertion 1 FlankMismatchGenomeLonger,SingleClassLongerSpan,ObservedMismatch 2 CORNELL,WICVAR, 2 T,A, 77.000000,59.000000, 0.566176,0.433824, maf-5-some-pop,maf-5-all-pops,observed-mismatch
chr1 26361669 rs61742342 C A 99 0 . GT:GQ:DP:FLT 1/0:99:34:0 786 chr1 26361669 26361670 rs61739493 0 + G G G/T genomic single unknown 0 0 coding-synon exact 1 1 CORNELL, 2 G,T, 77.000000,1.000000, 0.987179,0.012820,
chr1 26608828 rs17838088 G A 36 0 . GT:GQ:DP:FLT 1/1:28:4:0 788 chr1 26608828 26608829 rs61775085 0 + G G A/G genomic single unknown 0.5 0 coding-synon exact 1 2 BCMHGSC_JDW,ENSEMBL, 2 G,A, 1.000000,1.000000, 0.500000,0.500000, maf-5-some-pop,maf-5-all-pops
chr1 27210721 rs3170660 T C 99 0 . GT:GQ:DP:FLT 1/1:99:63:0 792 chr1 27210721 27210722 rs78109142 0 + G G A/G genomic single by-cluster,by-frequency,by-1000genomes 0.165289 0.235211 coding-synon exact 1 1 1000GENOMES, 2 G,A, 195.000000,13.000000, 0.937500,0.062500, maf-5-some-pop,maf-5-all-pops
chr1 64643277 rs7527017 C T 99 0 . GT:GQ:DP:FLT 0/1:99:117:0 1078 chr1 64643277 64643278 rs80063252 0 + G G A/G genomic single by-cluster,by-frequency,by-1000genomes 0.0768 0.180282 coding-synon exact 1 1 1000GENOMES, 2 G,A, 158.000000,10.000000, 0.940476,0.059524, maf-5-some-pop
chr1 110709719 rs7527375 T C 99 0 . GT:GQ:DP:FLT 1/0:99:31:0 1429 chr1 110709719 110709720 rs12737742 0 + G G A/C/G genomic single by-cluster,by-1000genomes 0.375 0.216506 coding-synon,missense exact 1 SingleClassTriAllelic 9 1000GENOMES,BCMHGSC_JDW,BUSHMAN,CORNELL,ENSEMBL,HGSV,ILLUMINA,SEATTLESEQ,SSAHASNP, 3 G,A,C, 57.000000,21.000000,1.000000, 0.721519,0.265823,0.012658, maf-5-some-pop,maf-5-all-pops

```





