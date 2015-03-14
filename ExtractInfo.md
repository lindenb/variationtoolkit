


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
cd src
 make ../bin/extractinfo
```


## Options: ##

  * **-c** (info-column-infex) (7)
  * **--delim** (column-delimiter) (default:tab)
  * **-t** (tag) (required)
  * **-N** (string) symbol for NOT-FOUND. default:N/A
  * **-i** ignore line if tag was not found



## Example: ##

The following script extract the **GN**(gene name) field from the column **INFO**. We keep the lines for the gene NOTCH2 and we display the associated SNP.

```

$ gunzip -c data.vcf.gz |\
  extractinfo -t GN -i | \
  awk -F '       ' '($11 =="NOTCH2")' |\
  cut -d ' ' -f 3 | grep rs

rs6685892
rs2493392
rs2493420
rs7534585
rs7534586
rs2493409
rs2453040
rs2124109

```





