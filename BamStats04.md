

Computes a statistics about the fraction of covered bases in a Bam.

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
$ cd variationtoolkit/src
make ../bin/bamstats04
```


## Usage ##


```
bamstats04 [options]  -b file.bed file1.bam file2.bam ... fileN.bam
```


## Options ##

  * -m (min-qual int) (optional) min SAM record Quality.
  * -M (max-qual int) (optional) max SAM record Quality.
  * -b (bedfile) required.
  * -g (groupid) optional.
  * -d do NOT dicard reads marked as duplicates.
  * --bin-max (int) max observed depth default:200
  * --bin-step (int) category/depth step:10
  * --cumulative (if coverage=100, then increase each bin from 0 to 100)
  * -p print percentage instead of count


## Example ##



```
$ bamstats04 -g mygroupid -p -b  my.bed my.bam |\
  verticalize 

>>>	2
$1	#filename  	my.bam
$2	total-bases	1338720
$3	group-id   	mygroupid
$4	[0-10[     	21.6465
$5	[10-20[    	8.33647
$6	[20-30[    	6.69251
$7	[30-40[    	5.83699
$8	[40-50[    	5.07828
$9	[50-60[    	4.48645
$10	[60-70[    	4.14956
$11	[70-80[    	3.6009
$12	[80-90[    	3.21912
$13	[90-100[   	3.14248
$14	[100-110[  	2.68988
$15	[110-120[  	2.49074
$16	[120-130[  	2.42142
$17	[130-140[  	2.04322
$18	[140-150[  	2.04554
$19	[150-160[  	1.75115
$20	[160-170[  	1.69154
$21	[170-180[  	1.52168
$22	[180-190[  	1.46409
$23	[190-200[  	1.32769
$24	[200-all[  	14.3638
<<<	2


```





