


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

```
cd src
 make ../bin/extractformat
```


## Options ##

  * -f (format-column-infex) (8)
  * -c (call-column-infex) (9)
  * --delim (column-delimiter) (default:tab)
  * -t (tag) (required)
  * -N (string) symbol for NOT-FOUND. default:N/A


## Example ##

The following command line extract the field **'GT'** from the VCF and we count the occurence of the values.


```

$ gunzip -c data.vcf.gz |\
   extractformat -t GT |\
   cut -d '        ' -f 11 |\
   sort |\
    uniq -c

     29 
  10729 0/1
  10800 1/0
  13518 1/1
     11 1/2

```





