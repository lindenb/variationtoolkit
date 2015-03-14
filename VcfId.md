


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

  * -t (string) table name default:snp135
  * -f limit to first snp.
  * --host (mysql host) default:genome-mysql.cse.ucsc.edu
  * --user (mysql user) default:genome
  * --password (mysql password) default:
  * --database (mysql database) default:hg19
  * --port (mysql password) default:0


## Example ##


```
$ cat casava_chr1_snps.txt |\
casava2vcf |\
vcfid  --host localhost --user anonymous --port 3316  |\
grep -v "##" |\
verticalize

(...)
>	172
$1	#CHROM    	chr1
$2	POS       	601635
$3	ID        	rs2531322
$4	REF       	C
$5	ALT       	A
$6	QUAL      	16
$7	FILTER    	.
$8	INFO      	DP=6;bcalls_used=6;Q_snp=16;max_gt=AC;Q_max_gt=16;max_gt_poly_site=AC;Q_max_gt_poly_site=AC;A_used=2;C_used=4;G_used=0;T_used=0
$9	FORMAT    	GT:GQ:DP
$10	__SAMPLE__	0/1:16:6
<<<	172

>>>	173
$1	#CHROM    	chr1
$2	POS       	601841
$3	ID        	rs148361140,rs406120
$4	REF       	G
$5	ALT       	C
$6	QUAL      	18
$7	FILTER    	.
$8	INFO      	DP=4;bcalls_used=4;Q_snp=18;max_gt=CG;Q_max_gt=18;max_gt_poly_site=CG;Q_max_gt_poly_site=CG;A_used=0;C_used=2;G_used=2;T_used=0
$9	FORMAT    	GT:GQ:DP
$10	__SAMPLE__	0/1:18:4
<<<	173

(...)

```





