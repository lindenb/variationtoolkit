

Original code from samtools ttview : Heng Li, Bob Handsaker, Jue Ruan, Colin Hercus, Petr Danecek

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
make ../bin/vcfttview
```


## Options ##

  * -c (chrom Column) (1)
  * -p (pos Column) (2)
  * -s (sample Column) (0) [optional](optional.md)
  * -d (column-delimiter) (default:tab)
  * -B (bam-file) [one main bam for all data](defines.md)
  * -f (file) loads a file tab delimited with SAMPLE-NAME\\tPATH-TO-BAM
  * -F (SAMPLE) (FILE)  push a SAMPLE-NAME/PATH-TO-BAM in the current list
  * -a for one position, print all BAM
  * -x (int) shift x bases to the right: default10
  * -w (int) screen width default:80
  * -R (fasta) reference file indexed with samtools faidx


## Example ##



```


$ echo -e "ref\t3\nref2\t2" |\
  vcfttview -x 3 -B toy.bam -R toy.fa

>ref:3

1         11              21        31         41        51        61           
AGCATGTTAGATAA****GATA**GCTGTGCTAGTAGGCAG*TCAGCGCCATNNNNNNNNNNNNNNNNNNNNNNNNNNNN
      ........    ....  ......K.K......K. ..........                            
      ........AGAG....***...      ,,,,,    ,,,,,,,,,                            
        ......GG**....AA                                                        
        ..C...**** ...**...>>>>>>>>>>>>>>T.....                                 



>ref2:2

1         11            21        31        41        51        61              
aggttttataaaac****aattaagtctacagagcaactacgcgNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
.............Y    ..W...................                                        
..............****..A...                                                        
 .............****..A...T.                                                      
     .........AAAT.............                                                 
         C...T****....................                                          
           ..T****.....................                                         
             T****......................                                        
                                                               

```





