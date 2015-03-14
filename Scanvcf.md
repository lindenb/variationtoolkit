

Reads a strean with two columns: a **Sample-Name** and the **file path to a VCF**(.gz).

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

```
#VCF	SAMPLE
/path/to/file1a.vcf.gz	Sample1
/path/to/file1b.vcf.gz	Sample1
/path/to/file2.vcf.gz	Sample2
```

it then prints all the VCF, adding an extra column with the sample name to the output.


## Options: ##

  * **--sample or -S** (column-index) column for the path to **SAMPLE**
  * **--vcf or -V** (column-index) column for the path to **VCF(.gz)**



## Example: ##


```
$ head -n3 input.txt

#Sample	VCF
Sample1	data/sample1.vcf.gz
Sample2	data/sample1.vcf.gz
Sample2	data/sample1.vcf.gz


$ cat input.txt |scanvcf 

#CHROM POS ID REF ALT QUAL FILTER . FORMAT Call SAMPLE
1 879317 rs7523549 C T 71 0 . GT:GQ:DP:FLT 0/1:34:8:0 Sample1
1 880238 rs3748592 A G 51 0 . GT:GQ:DP:FLT 1/1:51:8:0 Sample1
1 880390 rs3748593 C A 99 0 . GT:GQ:DP:FLT 1/0:99:30:0 Sample1
1 881627 rs2272757 G A 99 0 . GT:GQ:DP:FLT 1/0:59:20:0 Sample1
(...)
Y 13524507 . C T 99 0 . GT:GQ:DP:FLT 1/1:99:233:0 Sample20
Y 21154323 rs10465459 G A 99 0 . GT:GQ:DP:FLT 1/1:99:215:0 Sample20
Y 21154426 rs52812045 G A 99 0 . GT:GQ:DP:FLT 1/0:99:143:0 Sample20
Y 21154466 rs10465460 T A 99 0 . GT:GQ:DP:FLT 1/1:99:134:0 Sample20
Y 21154529 . G A 51 0 . GT:GQ:DP:FLT 1/1:51:8:0 Sample20

```





