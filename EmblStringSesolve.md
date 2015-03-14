

Calls the service: EMBL String resolve ([http://string-db.org/help/index.jsp?topic=/org.string-db.docs/api.html](http://string-db.org/help/index.jsp?topic=/org.string-db.docs/api.html) ).

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

  * -d (char) delimiter default:tab
  * -c column identifier
  * -t (int) taxon id


## Example ##





```

$ echo -e "#Gene\nNOTCH2\nEIF4G1\nPABPC1" |\
  emblstringresolve -c 1 |verticalize 
>>>	2
$1	#Gene        	NOTCH2
$2	stringId     	9606.ENSP00000256646
$3	preferredName	NOTCH2
$4	annotation   	Notch homolog 2 (Drosophila); Functions as a receptor...
<<<	2

>>>	3
$1	#Gene        	EIF4G1
$2	stringId     	9606.ENSP00000316879
$3	preferredName	EIF4G1
$4	annotation   	eukaryotic translation initiation factor 4 gamma, 1; ...
<<<	3

>>>	4
$1	#Gene        	PABPC1
$2	stringId     	9606.ENSP00000313007
$3	preferredName	PABPC1
$4	annotation   	poly(A) binding protein, cytoplasmic 1; Binds the...
<<<	4

```





