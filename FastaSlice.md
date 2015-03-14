


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

## Options: ##

  * **-e** (every)  default:1
  * **-L** (fragment size)  default: (same as -e)


## Example: ##



```
$ fastaslice -e 10 -L 20 < nsp3.fasta | head

>gi|256041817|gb|ACU64749.1| NSP3 protein [Rotavirus A AU32xUK reassortant (UKg9AU32)]|slice:0-20
MLKMESTQQMASSIINTSFE
>gi|256041817|gb|ACU64749.1| NSP3 protein [Rotavirus A AU32xUK reassortant (UKg9AU32)]|slice:10-30
ASSIINTSFEAAVVAATSTL
>gi|256041817|gb|ACU64749.1| NSP3 protein [Rotavirus A AU32xUK reassortant (UKg9AU32)]|slice:20-40
AAVVAATSTLELMGIQYDYN
>gi|256041817|gb|ACU64749.1| NSP3 protein [Rotavirus A AU32xUK reassortant (UKg9AU32)]|slice:30-50
ELMGIQYDYNEIYTRVKSKF
>gi|256041817|gb|ACU64749.1| NSP3 protein [Rotavirus A AU32xUK reassortant (UKg9AU32)]|slice:40-60
EIYTRVKSKFDYVMDDSGVK
```






