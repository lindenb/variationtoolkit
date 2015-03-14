


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

  * -d (column-delimiter) (default:tab)
  * -c (CHROM col) (default:0)
  * -p (POS col) (default:1)
  * -S (bed score col) (default:-1)
  * -N (col) adds this column for the 'name'
  * -D (char) name separator.
  * -t print ucsc custom track header.


## Example ##




```

$ gunzip -c file.vcf.gz |\
  normalizechrom |\
  vcf2bed -t -S 6 -N 3,4,5 -D _

track name="__TRACK_NAME__" description="__TRACK_DESC__" 
chr1	879316	879317	rs7523549_C_T	71	+
chr1	880237	880238	rs3748592_A_G	51	+
chr1	880389	880390	rs3748593_C_A	99	+
chr1	881626	881627	rs2272757_G_A	99	+
chr1	883624	883625	rs4970378_A_G	39	+
chr1	887559	887560	rs3748595_A_C	99	+
chr1	887800	887801	rs3828047_A_G	99	+
chr1	888638	888639	rs3748596_T_C	99	+
chr1	888658	888659	rs3748597_T_C	99	+
chr1	889157	889158	rs56262069_G_C	51	+
(...)

```





