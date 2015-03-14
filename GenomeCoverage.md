

Computes the mean coverage for each chromosomes

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
make ../bin/genomecoverage
```


## Usage ##


```
genomecoverage -f reference.fasta [options] bam1 bam2...
```


## Options ##

  * **-f** (file) reference genome
  * **-m** (int) min mapping quality (OPTIONAL)


## Example ##

The output is: chrom, chromLength, mean-depth, mean-depth ignoring the unknown ('N') bases.

```
$ genomecoverage -f human.fasta  my.bam

#chrom    length    mean-depth(my.bam)    mean-depth-noN(my.bam)
1    249250621    28    30
2    243199373    30    31
3    198022430    30    31
4    191154276    31    31
5    180915260    30    31
6    171115067    30    31
7    159138663    30    30
8    146364022    31    31
9    141213431    26    30
10    135534747    30    31
11    135006516    29    30
12    133851895    30    30
13    115169878    26    31
14    107349540    25    31
15    102531392    24    30
16    90354753    28    31
17    81195210    27    29
18    78077248    30    31
19    59128983    25    27
20    63025520    28    29
21    48129895    24    33
22    51304566    18    27
X    155270560    31    32
Y    59373566    1    3 

```





