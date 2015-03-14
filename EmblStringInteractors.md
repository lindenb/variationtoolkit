

Calls the service: EMBL String interactors ( [http://string-db.org/help/index.jsp?topic=/org.string-db.docs/api.html](http://string-db.org/help/index.jsp?topic=/org.string-db.docs/api.html) ).

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


## Example ##




```

$ echo -e "#Gene\nNOTCH2\nEIF4G1\nPABPC1" |\
  emblstringresolve -c 1 | \
  emblstringinteractors -c 2 | \
  emblstringresolve -c 5 | \
  verticalize
  
>>>	2
$1	#Gene        	NOTCH2
$2	stringId     	9606.ENSP00000256646
$3	preferredName	NOTCH2
$4	annotation   	Notch homolog 2 (Drosophila); Functions as a receptor for membr
$5	interactor   	9606.ENSP00000256646
$6	stringId     	9606.ENSP00000256646
$7	preferredName	NOTCH2
$8	annotation   	Notch homolog 2 (Drosophila); Functions as a receptor for membr
<<<	2

>>>	3
$1	#Gene        	NOTCH2
$2	stringId     	9606.ENSP00000256646
$3	preferredName	NOTCH2
$4	annotation   	Notch homolog 2 (Drosophila); Functions as a receptor for membr
$5	interactor   	9606.ENSP00000345206
$6	stringId     	9606.ENSP00000345206
$7	preferredName	RBPJ
$8	annotation   	recombination signal binding protein for immunoglobulin kappa J
<<<	3

>>>	4
$1	#Gene        	NOTCH2
$2	stringId     	9606.ENSP00000256646
$3	preferredName	NOTCH2
$4	annotation   	Notch homolog 2 (Drosophila); Functions as a receptor for membr
$5	interactor   	9606.ENSP00000355718
$6	stringId     	9606.ENSP00000355718
$7	preferredName	DLL1
  
  

```







