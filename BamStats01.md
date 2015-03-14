

Computes some statistics about the reads in a Bam.

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
make ../bin/bamstats01
```


## Usage ##


```
bamstats01 [options] (stdin|file1.bam file2.bam ... fileN.bam)
```


## Options ##

  * -b (bedfile) optional.
  * -g (groupid) optional.


## Example ##

  * all: all reads
  * q30: reads having a quality > 30
  * mapped: reads mapped on the reference
  * mappedproperpair: reads in proper pair
  * mappeddup: reads mapped on the reference and marked as duplicate
  * mappedbed: reads mapped in target
  * mappedbed30: reads mapped in target having a quality > 30
  * mappedbeddup: reads mapped int target and marked as duplicate


```
$ bamstats01 -b capture.bed  file1.bam file2.bam file3.bam file4.bam

#BAM	all	q30	mapped	mappedproperpair	mappeddup	mappedbed	mappedbedq30	mappedbeddup
file1.bam	118802226	113879690	118083439	115271815	69951710	81979191	80349586	49112045
file2.bam	129023584	123865417	128474569	126954981	81926541	87948491	86093108	56540705
file3.bam	192392854	183316380	190793205	187733660	133590003	124523963	122117063	87770525
file4.bam	142838162	137089152	142180885	141085425	85571616	98484535	96465031	59782134



```



## Creating a graph with R ##


```
pdf("hist.pdf",paper="A4r")
T<-read.delim("output-of-bamstats01.tsv",header=T,sep="\t",row.names=1)
barplot(as.matrix(t(T)),beside=TRUE,col=rainbow(8),las=2,cex.names=0.8,legend=colnames(T))
dev.off()'
```


![http://variationtoolkit.googlecode.com/svn/wiki/bamstats01.jpg](http://variationtoolkit.googlecode.com/svn/wiki/bamstats01.jpg)





