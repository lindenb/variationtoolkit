


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

  * --delim (char) or -d  (char) (delimiter) default:tab
  * --norefalt : don't look at REF and ALT
  * --sample SAMPLE column index
  * --chrom CHROM column index: default 1
  * --pos POS position column index: default 2
  * --ref REF reference allele column index: default 4
  * --alt ALT alternate allele column index: default 5
  * -T 1,2,3,4,... columns indexes on top.
  * -L 5,6,7,... columns indexes on left.
  * -n (name1,name2,name3,...) add this sample name.
  * -c print a count by pair. This option loop over each pair of samples adds a column ('0' or '1') telling if a variation was found in both samples in the pair.


### Examples ###

Read the VCF data and generate a pivot table.


```

$  cat sample2vcf.tsv | scanvcf | grep -v "##"  |\
   sed 's/^#CHROM/#/' |\
   sort -t '       ' -k1,1 -k2,2n -k4,4 -k5,5 -k11,11 |\
   sed 's/^#/#CHROM/' |\
   groupbysnp -L 1,2,3,4,5 -T 6,7,8,9,10 --sample 11  -n Sample1,Sample2,Sample3,Sample4  |\
   verticalize

>>>	2
$1	#CHROM       	1
$2	POS          	753405
$3	ID           	rs61770173
$4	REF          	C
$5	ALT          	A
$6	Sample1         	.
$7	Sample1:QUAL    	.
$8	Sample1:FILTER  	.
$9	Sample1:INFO    	.
$10	Sample1:FORMAT  	.
$11	Sample1:CALL    	.
$12	Sample2         	.
$13	Sample2:QUAL    	.
$14	Sample2:FILTER  	.
$15	Sample2:INFO    	.
$16	Sample2:FORMAT  	.
$17	Sample2:CALL    	.
$18	Sample3         	Sample3
$19	Sample3:QUAL    	99
$20	Sample3:FILTER  	0
$21	Sample3:INFO    	AC=2;DB=3;ST=0:0,3:32;DP=35;NC=-0.76;UM=3;CQ=...
$22	Sample3:FORMAT  	GT:GQ:DP:FLT
$23	Sample3:CALL    	1/1:99:35:0
$24	Sample4         	.
$25	Sample4:QUAL    	.
$26	Sample4:FILTER  	.
$27	Sample4:INFO    	.
$28	Sample4:FORMAT  	.
$29	Sample4:CALL    	.
$30	count.samples	1
<<<	2

>>>	3
$1	#CHROM       	1
$2	POS          	876499
$3	ID           	rs4372192
$4	REF          	A
$5	ALT          	G
$6	Sample1         	.
$7	Sample1:QUAL    	.
$8	Sample1:FILTER  	.
$9	Sample1:INFO    	.
$10	Sample1:FORMAT  	.
$11	Sample1:CALL    	.
$12	Sample2         	.
$13	Sample2:QUAL    	.
$14	Sample2:FILTER  	.
$15	Sample2:INFO    	.
$16	Sample2:FORMAT  	.
$17	Sample2:CALL    	.
$18	Sample3         	.
$19	Sample3:QUAL    	.
$20	Sample3:FILTER  	.
$21	Sample3:INFO    	.
$22	Sample3:FORMAT  	.
$23	Sample3:CALL    	.
$24	Sample4         	Sample4
$25	Sample4:QUAL    	45
$26	Sample4:FILTER  	0
$27	Sample4:INFO    	AC=2;DB=1;ST=0:0,6:0;DP=6;NC=-3.05;UM=3;CQ=...
$28	Sample4:FORMAT  	GT:GQ:DP:FLT
$29	Sample4:CALL    	1/1:45:6:0
$30	count.samples	1
<<<	3

>>>	4
$1	#CHROM       	1
$2	POS          	877831
$3	ID           	rs6672356
$4	REF          	T
$5	ALT          	C
$6	Sample1         	.
$7	Sample1:QUAL    	.
$8	Sample1:FILTER  	.
$9	Sample1:INFO    	.
$10	Sample1:FORMAT  	.
$11	Sample1:CALL    	.
$12	Sample2         	.
$13	Sample2:QUAL    	.
$14	Sample2:FILTER  	.
$15	Sample2:INFO    	.
$16	Sample2:FORMAT  	.
$17	Sample2:CALL    	.
$18	Sample3         	.
$19	Sample3:QUAL    	.
$20	Sample3:FILTER  	.
$21	Sample3:INFO    	.
$22	Sample3:FORMAT  	.
$23	Sample3:CALL    	.
$24	Sample4         	Sample4
$25	Sample4:QUAL    	39
$26	Sample4:FILTER  	0
$27	Sample4:INFO    	AC=2;DB=1;ST=0:0,2:2;DP=4;NC=0.40;UM=3;CQ=...
$28	Sample4:FORMAT  	GT:GQ:DP:FLT
$29	Sample4:CALL    	1/1:39:4:0
$30	count.samples	1
<<<	4

>>>	5
$1	#CHROM       	1
$2	POS          	879317
$3	ID           	rs7523549
$4	REF          	C
$5	ALT          	T
$6	Sample1         	CALL
$7	Sample1:QUAL    	71
$8	Sample1:FILTER  	0
$9	Sample1:INFO    	AC=1;DB=1;ST=2:1,3:2;DP=8;NC=2.16;UM=3;CQ=...
$10	Sample1:FORMAT  	GT:GQ:DP:FLT
$11	Sample1:CALL    	0/1:34:8:0
$12	Sample2         	.
$13	Sample2:QUAL    	.
$14	Sample2:FILTER  	.
$15	Sample2:INFO    	.
$16	Sample2:FORMAT  	.
$17	Sample2:CALL    	.
$18	Sample3         	.
$19	Sample3:QUAL    	.
$20	Sample3:FILTER  	.
$21	Sample3:INFO    	.
$22	Sample3:FORMAT  	.
$23	Sample3:CALL    	.
$24	Sample4         	.
$25	Sample4:QUAL    	.
$26	Sample4:FILTER  	.
$27	Sample4:INFO    	.
$28	Sample4:FORMAT  	.
$29	Sample4:CALL    	.
$30	count.samples	1
<<<	5

>>>	6
$1	#CHROM       	1
$2	POS          	880238
$3	ID           	rs3748592
$4	REF          	A
$5	ALT          	G
$6	Sample1         	CALL
$7	Sample1:QUAL    	51
$8	Sample1:FILTER  	0
$9	Sample1:INFO    	AC=2;DB=1;ST=0:0,4:4;DP=8;NC=-3.73;UM=3;CQ=...
$10	Sample1:FORMAT  	GT:GQ:DP:FLT
$11	Sample1:CALL    	1/1:51:8:0
$12	Sample2         	Sample2
$13	Sample2:QUAL    	54
$14	Sample2:FILTER  	0
$15	Sample2:INFO    	AC=2;DB=1;ST=0:0,3:6;DP=9;NC=-3.73;UM=3;CQ=..
$16	Sample2:FORMAT  	GT:GQ:DP:FLT
$17	Sample2:CALL    	1/1:54:9:0
$18	Sample3         	Sample3
$19	Sample3:QUAL    	54
$20	Sample3:FILTER  	0
$21	Sample3:INFO    	AC=2;DB=1;ST=0:0,4:5;DP=9;NC=-3.73;UM=3;CQ=...
$22	Sample3:FORMAT  	GT:GQ:DP:FLT
$23	Sample3:CALL    	1/1:54:9:0
$24	Sample4         	Sample4
$25	Sample4:QUAL    	72
$26	Sample4:FILTER  	0
$27	Sample4:INFO    	AC=2;DB=1;ST=0:0,5:10;DP=15;NC=-3.73;UM=3;CQ=...
$28	Sample4:FORMAT  	GT:GQ:DP:FLT
$29	Sample4:CALL    	1/1:72:15:0
$30	count.samples	4
<<<	6
(...)

```





