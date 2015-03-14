


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

  * -u uniq
  * -i ignore case


## Example ##



```

$ fastaslice -e 1 -L 3 < nsp3.fasta |\
  fastasortuniq -i -u | head

>gi|77020788|gb|ABA60396.1| non-structural protein NSP3 [Human rotavirus B219]|slice:47-50
AAF
>gi|288187218|gb|ADC42131.1| translation enhancer NSP3 [Bovine rotavirus A]|slice:80-83
AAK
>gi|110558644|gb|ABG75781.1| NSP3 [Rotavirus A]|slice:20-23
AAL
>gi|284517165|gb|ADB92082.1| NSP3 [Human rotavirus A]|slice:48-51
AAR
>gi|256041817|gb|ACU64749.1| NSP3 protein [Rotavirus A AU32xUK reassortant (UKg9AU32)]|slice:24-27
AAT


```





