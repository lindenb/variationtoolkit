

Note: I saw some regions where the UCSC API seems to be broken and exit.

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

  * -d (char) column delimiter. default: TAB
  * -c (int) chromosome column (1).
  * -p (int) pos column (2).
  * -1 data are NOT +1 based.
  * -f (path) liftOver map file (required).
  * -b (double) liftOver minblocks.
  * -m (double) liftOver minMatch.



## Example ##

Download the data of snp129 from UCSC hg18, remove some columns and convert to hg19.


```

$ curl  -s "http://hgdownload.cse.ucsc.edu/goldenPath/hg18/database/snp129.txt.gz" |\
  gunzip  -c |\
  cut -d '  ' -f 2,3,5 |\
  vcfliftover -1 -f /path/tp/hg18ToHg19.over.chain 
  
chr1	433	rs56289060	chr1	10433	10434	.
chr1	491	rs55998931	chr1	10491	10492	.
chr1	518	rs62636508	chr1	10518	10519	.
chr1	582	rs58108140	chr1	10582	10583	.
chr1	690	rs10218492	chr1	10827	10828	.
chr1	766	rs10218493	chr1	10903	10904	.
chr1	789	rs10218527	chr1	10926	10927	.
chr1	800	rs28853987	chr1	10937	10938	.
chr1	876	rs28484712	chr1	11013	11014	.
chr1	884	rs28775022	chr1	11021	11022	.
(...)
chr1	1609710	rs61776794	.	.	.	Deleted in new
chr1	1609743	rs61776795	.	.	.	Deleted in new
chr1	1609758	rs61776796	.	.	.	Deleted in new
chr1	1609849	rs7413891	.	.	.	Deleted in new
chr1	1610719	rs3737622	.	.	.	Deleted in new
chr1	1610719	rs45576038	.	.	.	Deleted in new
chr1	1610763	rs3737624	.	.	.	Deleted in new
chr1	2475133	rs3091278	.	.	.	Deleted in new
chr1	2475134	rs3091239	.	.	.	Deleted in new
(...)

```





