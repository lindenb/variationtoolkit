

Computes the mean depth of a set of BAMs in the regions of a BED.

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
make ../bin/beddepth
```


## Usage ##


```
beddepth -f bam [options] (stdin|bam)..
```


## Options ##

  * -f (bam-file) add this bam file. Can be called multiple times
  * -m (min-qual uint32) (optional) min SAM record Quality.
  * -M (max-qual uint32) (optional) max SAM record Quality.
  * -D (min-depth) (optional) min depth.  Can be called multiple times
  * -d do NOT discard duplicates


## Example ##

The output is: chrom, chromLength, mean-depth, mean-depth ignoring the unknown ('N') bases.

```
$ echo -e "seq2\t100\t200\nseq1\t0\t10\nseq2\t1500\t1550" |\
beddepth -D 5 -D 100 -D 200 -f  sorted.bam -f ex1.bam |\
verticalize 


>>>	2
$1	#chrom                                	seq2
$2	start                                 	100
$3	end                                   	200
$4	size(sorted.bam)                      	100
$5	covered[depth:0](sorted.bam)          	100
$6	percent_covered[depth:0](sorted.bam)  	1
$7	covered[depth:5](sorted.bam)          	100
$8	percent_covered[depth:5](sorted.bam)  	1
$9	covered[depth:100](sorted.bam)        	0
$10	percent_covered[depth:100](sorted.bam)	0
$11	covered[depth:200](sorted.bam)        	0
$12	percent_covered[depth:200](sorted.bam)	0
$13	min(sorted.bam)                       	18
$14	max(sorted.bam)                       	93
$15	median(sorted.bam)                    	57
$16	mean(sorted.bam)                      	57.27
$17	size(ex1.bam)                         	100
$18	covered[depth:0](ex1.bam)             	100
$19	percent_covered[depth:0](ex1.bam)     	1
$20	covered[depth:5](ex1.bam)             	100
$21	percent_covered[depth:5](ex1.bam)     	1
$22	covered[depth:100](ex1.bam)           	0
$23	percent_covered[depth:100](ex1.bam)   	0
$24	covered[depth:200](ex1.bam)           	0
$25	percent_covered[depth:200](ex1.bam)   	0
$26	min(ex1.bam)                          	6
$27	max(ex1.bam)                          	31
$28	median(ex1.bam)                       	19
$29	mean(ex1.bam)                         	19.09
<<<	2

>>>	3
$1	#chrom                                	seq1
$2	start                                 	0
$3	end                                   	10
$4	size(sorted.bam)                      	10
$5	covered[depth:0](sorted.bam)          	10
$6	percent_covered[depth:0](sorted.bam)  	1
$7	covered[depth:5](sorted.bam)          	8
$8	percent_covered[depth:5](sorted.bam)  	0.8
$9	covered[depth:100](sorted.bam)        	0
$10	percent_covered[depth:100](sorted.bam)	0
$11	covered[depth:200](sorted.bam)        	0
$12	percent_covered[depth:200](sorted.bam)	0
$13	min(sorted.bam)                       	3
$14	max(sorted.bam)                       	15
$15	median(sorted.bam)                    	12
$16	mean(sorted.bam)                      	9.3
$17	size(ex1.bam)                         	10
$18	covered[depth:0](ex1.bam)             	10
$19	percent_covered[depth:0](ex1.bam)     	1
$20	covered[depth:5](ex1.bam)             	2
$21	percent_covered[depth:5](ex1.bam)     	0.2
$22	covered[depth:100](ex1.bam)           	0
$23	percent_covered[depth:100](ex1.bam)   	0
$24	covered[depth:200](ex1.bam)           	0
$25	percent_covered[depth:200](ex1.bam)   	0
$26	min(ex1.bam)                          	1
$27	max(ex1.bam)                          	5
$28	median(ex1.bam)                       	4
$29	mean(ex1.bam)                         	3.1
<<<	3

>>>	4
$1	#chrom                                	seq2
$2	start                                 	1500
$3	end                                   	1550
$4	size(sorted.bam)                      	50
$5	covered[depth:0](sorted.bam)          	50
$6	percent_covered[depth:0](sorted.bam)  	1
$7	covered[depth:5](sorted.bam)          	50
$8	percent_covered[depth:5](sorted.bam)  	1
$9	covered[depth:100](sorted.bam)        	33
$10	percent_covered[depth:100](sorted.bam)	0.66
$11	covered[depth:200](sorted.bam)        	0
$12	percent_covered[depth:200](sorted.bam)	0
$13	min(sorted.bam)                       	45
$14	max(sorted.bam)                       	129
$15	median(sorted.bam)                    	114
$16	mean(sorted.bam)                      	97.02
$17	size(ex1.bam)                         	50
$18	covered[depth:0](ex1.bam)             	50
$19	percent_covered[depth:0](ex1.bam)     	1
$20	covered[depth:5](ex1.bam)             	50
$21	percent_covered[depth:5](ex1.bam)     	1
$22	covered[depth:100](ex1.bam)           	0
$23	percent_covered[depth:100](ex1.bam)   	0
$24	covered[depth:200](ex1.bam)           	0
$25	percent_covered[depth:200](ex1.bam)   	0
$26	min(ex1.bam)                          	15
$27	max(ex1.bam)                          	43
$28	median(ex1.bam)                       	38
$29	mean(ex1.bam)                         	32.34
<<<	4



```





