


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

  * -d or --delim (char) delimiter default:tab
  * -n first line is NOT the header.


## Example ##



```

$ gunzip -c file.vcf.gz | grep -v "##" |\
   verticalize  |\
   head -n 100
>>>	2
$1	#CHROM   	1
$2	POS      	880238
$3	ID       	rs3748592
$4	REF      	A
$5	ALT      	G
$6	QUAL     	48
$7	FILTER   	0
$8	INFO     	AC=2;DB=1;ST=0:0,3:4;DP=7;NC=-3.73;UM=3;CQ=INTRONIC;MQ=60;AN=2;PA=1^1:0.930&2^1:0.860&3^1:0.950;MZ=0;GN=NOC2L;PS=1
$9	FORMAT   	GT:GQ:DP:FLT
$10	162214_A1	1/1:48:7:0
<<<	2

>>>	3
$1	#CHROM   	1
(...)

```





