

Depth of coverage for each bases for one or more BAM. It's slow and takes some memory, but it works fine.

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
make ../bin/bamstats05
```


## Usage ##


```
bamstats05 [options] (stdin|file1.bam file2.bam ... fileN.bam)
```


## Options ##

  * -m (min-qual uint32) (optional) min SAM record Quality.
  * -M (max-qual uint32) (optional) max SAM record Quality.
  * -b (bedfile) required.
  * -g split by group (optional).
  * -d do NOT dicard reads marked as duplicates.


## Example ##



```
$ bamstats05 -g -b target.bed *.bam
#CHROM	POS	file1.bam : idp11184	file1..bam : idp8320	file2.bam : idp653008	file2.bam : idp655904	file3.bam : idp816976	file3.bam : idp819872	file4.bam : idp15056	file4.bam : idp17920	file5.bam : idp35264	file5.bam : idp38128
chrX	480532	206	221	2	1	287	280	92	104	2	0
chrX	480533	206	221	2	1	287	283	92	104	2	0
chrX	480534	206	221	2	1	287	284	92	104	2	0
chrX	480535	206	221	2	1	287	284	92	104	2	0
chrX	480536	206	221	2	1	287	284	92	104	2	0
chrX	480537	206	221	2	1	287	284	92	104	2	0
chrX	480538	206	221	2	1	287	284	92	104	2	0
chrX	480539	206	221	2	1	287	284	92	104	2	0
chrX	480540	206	221	2	1	287	284	92	104	2	0
chrX	480541	206	221	2	1	287	284	92	104	2	0
chrX	480542	206	221	2	1	287	284	92	104	2	0
chrX	480543	206	222	2	1	287	284	92	104	2	0
chrX	480544	206	222	2	1	287	284	92	104	2	0
chrX	480545	206	222	2	1	287	284	92	104	2	0
chrX	480546	206	222	2	1	287	284	92	104	2	0
chrX	480547	206	222	2	1	287	284	92	104	2	0
chrX	480548	206	222	2	1	287	284	92	104	2	0
chrX	480549	206	222	2	1	287	284	92	104	2	0
chrX	480550	206	222	2	1	287	284	92	104	2	0
chrX	480551	206	222	2	1	287	284	92	104	2	0
chrX	480552	206	222	2	1	287	284	92	104	2	0
chrX	480553	206	222	2	1	287	284	92	104	2	0
chrX	480554	206	222	2	1	287	284	92	104	2	0
chrX	480555	206	222	2	1	287	284	92	104	2	0
chrX	480556	206	222	2	1	287	284	92	104	2	0
chrX	480557	206	222	2	1	287	284	92	104	2	0
chrX	480558	206	222	2	1	287	284	92	104	2	0
chrX	480559	206	222	2	1	287	284	92	104	2	0
chrX	480560	206	222	2	1	287	284	92	104	2	0
chrX	480561	206	222	2	1	287	284	92	104	2	0
chrX	480562	206	222	2	1	287	284	92	104	2	0
chrX	480563	206	222	2	1	287	284	92	104	2	0
chrX	480564	206	222	2	1	287	284	92	104	2	0
chrX	480565	206	222	2	1	287	284	92	104	2	0
chrX	480566	206	222	2	1	287	284	92	104	2	0
chrX	480567	206	222	2	1	287	284	92	104	2	0
chrX	480568	206	222	2	1	287	284	92	104	2	0
chrX	480569	206	222	2	1	287	284	92	104	2	0
chrX	480570	206	222	2	1	287	284	92	104	2	0
chrX	480571	206	222	2	1	287	284	92	104	2	0
chrX	480572	206	222	2	1	287	284	92	104	2	0
chrX	480573	206	222	2	1	287	284	92	104	2	0
chrX	480574	206	222	2	1	287	285	92	104	2	0
chrX	480575	206	222	2	1	287	285	92	104	2	0
chrX	480576	206	222	2	1	287	285	92	104	2	0
chrX	480577	206	222	2	1	287	285	92	104	2	0
chrX	480578	206	222	2	1	287	285	92	104	2	0
chrX	480579	206	222	2	1	287	285	92	104	2	0
chrX	480580	206	222	2	1	287	285	92	104	2	0
chrX	480581	206	222	2	1	287	285	92	104	2	0
chrX	480582	206	222	2	1	287	285	92	104	2	0
chrX	480583	206	222	2	1	287	285	92	104	2	0
chrX	480584	206	222	2	1	287	285	92	104	2	0
chrX	480585	206	222	2	1	287	285	92	104	2	0
chrX	480586	206	222	2	1	287	285	92	104	2	0
(...)
```



### Example: inserting and querying with sqlite3 and awk ###
.
The following awk script was made for a file created using the **paste** command on multiple output of bamstats05.

```
BEGIN	{
	FS="[	]";
	printf("begin transaction;");
	printf("create table GID(id int unique,bam varchar(150),grp varchar(50));\n");
	printf("create index GIDIDX1 on GID(bam);\n");
	printf("create index GIDIDX2 on GID(grp);\n");
	printf("create table BASES(chrom varchar(20),pos int,gid int,total int );\n");
	printf("create index BASESIDX1 on BASES(gid);\n");
	}

NR==1	{
	ncols=split($0,header);
	for(i=1;i<= ncols;++i)
		{
		if(header[i]=="POS" || header[i]=="#CHROM") continue;
		split(header[i],tokens,"[ ]");
		printf("insert into GID(id,bam,grp) values (%d,'%s','%s');\n",i,tokens[1],tokens[3]);
		}
	next;
	}

	{
	for(i=1;i<= NF;++i)
		{
		if(header[i]=="POS" || header[i]=="#CHROM") continue;
		printf("insert into BASES(chrom,pos,gid,total) values ('%s',%s,%d,%s);\n",$1,$2,i,$i);
		}
	}
	
END	{
	printf("end;\n");
	}


```

insert into sqlite3:

```
cat bamstats05.output.tsv |awk -f haloplex.awk | sqlite3 file.sql
```

and query:

```
$ sqlite3 file.sql 'select sum(BASES.total) from BASES,GID where BASES.gid=GID.id and GID.bam="file001.bam";'
```






