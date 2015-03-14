

search for reads in one ot two (paired) FASTQ files."

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
$ cd varkit/src
make ../bin/fastqgrep
```


## Usage ##


```
fastqgrep -R file_of_names.txt [options] (stdin)|(fastq)|(fastq1 fastq2)
```


## Options ##

  * **-R** file\_of\_names.txt REQUIRED.
  * **-m** allow multiple hits per read  (optional) .
  * **-o1** (filename) file out1 OPTIONAL.
  * **-o2** (filename) file out2 OPTIONAL.


## Example ##


```
$ $ fastqgrep -R reads.txt  file2.fastq.gz  file1.fastq.gz | head
@HWI-1KL149:2:C0KR2ACXX:8:2104:4474:86720 2:N:0:TGACAA	@HWI-1KL149:2:C0KR2ACXX:8:2104:4474:86720 1:N:0:TGACAA
NNNNNTNNNNNNNNNANNNANNNNNNNTNNAANTTNNNNNNNNANNCNANTACNCGTANTNGCTTTNNTNANNACNNTTANATAGATTAGCAATCCTGNAC	AATCTCCTCCACCCCTACTGCCATTACTTTTTTAAAGTGCTAATCACACTATCATCTGTCCCCTTCCAACAAAAAAAAGAAAACCAGAAAAAAGAGTCCAG
+	+
#####################################################################################################	@<@DFFFFHDHGFIJGGGGJJJJJFIJJJJJCGGIEABFHCGBFHGAHIHCFHEGIEH7=FGHGEHGHEH;EFFDA?=@D299?@B@8>@ABBD<?::>CC
@HWI-1KL149:2:C0KR2ACXX:8:2104:4417:86728 2:N:0:GCCCAT	@HWI-1KL149:2:C0KR2ACXX:8:2104:4417:86728 1:N:0:GCCCAT
NNNNNCNNNNNNNNNCNNNANNNNNNNANNATNCANNNNNNNNCNNANANCCCNGCCCNCNACGCGNNTNTNNTANNCTGNACCTCCCCTATGAAGGTNCA	CTTAAGGCAGAGTCAAAAACACAAATAGCTTTCCCTCATTTTTAGTCTAGTAATTCAAACTGAGTTTCAGATTTAGCAGTGTAGATGGATCTATGATAGTA
+	+
#####################################################################################################	8+=?DBDDDD<ADEEA,AFFIIIF399?E>EEIIEIICDDEED?B9BDBBDD/??D?D)8=8B######################################
@HWI-1KL149:2:C0KR2ACXX:8:2104:4666:86501 2:N:0:TGACAA	@HWI-1KL149:2:C0KR2ACXX:8:2104:4666:86501 1:N:0:TGACAA
NNNNNANNNNNNNNNANNNCNNNNNNNCNNAGNAANNNNNNNNCNNANTNCACNAGACNANATTGANNTNANNGANNGGANAAATAGAAAAATCCAAANTT	GTTACTTGATAGCATTGTTCAGTTATTCTATATCTTTACTGACTTTCTGCTTATTTTTTCTATGAAATATTAAGAAATGAGTATAAAATCTCCAATGATAA

```





