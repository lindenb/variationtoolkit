

Prints the regions uncovered, with a low coverage, in a BAM file.

## Compilation ##


```
$ cd varkit/src
make ../bin/bamnocoverage
```


## Usage ##


```
bamnocoverage [options] (stdin|bam1 bam2....)
```


## Options ##

  * **-m**(int) min depth (OPTIONAL).


## Example ##


```
$ bamnocoverage -m 10 testchr222.bam | head

chr22	0	14438296	14438296
chr22	14438297	14512511	74214
chr22	14512518	14521021	8503
chr22	14521025	14558541	37516
chr22	14558544	14558549	5
chr22	14558566	14558989	423
chr22	14558995	14558998	3
chr22	14559007	14747713	188706
chr22	14747717	15032707	284990
chr22	15032711	15392372	359661


```





