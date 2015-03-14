

Inserts a Variant Call Format document ([VCF](http://www.1000genomes.org/wiki/Analysis/Variant+Call+Format/vcf-variant-call-format-version-40)) into a [sqlite3](http://www.sqlite.org/) database.

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

## Dependencies ##

[http://www.sqlite.org/](http://www.sqlite.org/) : libraries and headers for sqlite3.

## Compilation ##

Define "`SQLITE_LIB`" and
"`SQLITE_CFLAGS`" in `config.mk` (see [HowToInstall](HowToInstall.md) )

```
$ cd variationtoolkit/src/
$ make ../bin/vcf2sqlite 

if ! [ -z "$(SQLITE_LIB)" ] ;then g++ -o ../bin/vcf2sqlite vcf2sqlite.cpp xsqlite.cpp application.o -O3 -Wall -lz   ; else g++ -o ../bin/vcf2sqlite vcf2sqlite.cpp  -DNOSQLITE -O3 -Wall  ; fi
```


## Usage ##


```
vcf2sqlite -f database.sqlite (file1.vcf file2... | stdin )
```


## Options ##

  * -f (file) [sqlite3](http://www.sqlite.org/) database (**REQUIRED**).


## Schema ##


![http://variationtoolkit.googlecode.com/svn/wiki/schema01.png](http://variationtoolkit.googlecode.com/svn/wiki/schema01.png)


## Example: ##


```
$ vcf2sqlite -f db.sqlite file.vcf
$ sqlite3 -line db.sqlite  "select * from VCFCALL LIMIT 4"

       id = 1
   nIndex = 0
vcfrow_id = 1
sample_id = 1
     prop = GT
    value = 1/1

       id = 2
   nIndex = 1
vcfrow_id = 1
sample_id = 1
     prop = PL
    value = 46,6,0

       id = 3
   nIndex = 2
vcfrow_id = 1
sample_id = 1
     prop = GQ
    value = 10

       id = 4
   nIndex = 0
vcfrow_id = 2
sample_id = 1
     prop = GT
    value = 1/1


```




```
$ sqlite3 -column -header  db.sqlite \
   "select SAMPLE.name,VCFCALL.value,count(*) from VCFCALL,SAMPLE where SAMPLE.id=VCFCALL.sample_id and prop='GT' group by SAMPLE.id,VCFCALL.value"

name         value       count(*)  
-----------  ----------  ----------
rmdup_1.bam  0/1         545       
rmdup_1.bam  1/1         429       
rmdup_2.bam  0/1         625       
rmdup_2.bam  1/1         349       
rmdup_3.bam  0/1         595       
rmdup_3.bam  1/1         379       
rmdup_4.bam  0/1         548       
rmdup_4.bam  1/1         426       
rmdup_5.bam  0/1         564       
rmdup_5.bam  1/1         410       
rmdup_6.bam  0/1         724       
rmdup_6.bam  1/1         250
```







