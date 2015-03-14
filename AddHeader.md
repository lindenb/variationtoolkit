

Small linux utility inserting a header before a stream.

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
$ cd varkit/src
make ../bin/addheader
```


## Usage ##


```
addheader [options] (stdin)
```


## Options ##

  * **-d**(CHAR) delimiter default:tab(OPTIONAL).
  * **-L** (CHAR) begin head line with... (default:# )  (optional) .
  * **-c** (INTEGER) (STRING) custom column def  (optional) .
  * **-f** (FILENAME) (optional) insert this file before header.


## Example ##


```
$ echo -e "A\tG\tC\nA\tazd\tx" |\
addheader -c 2 HELLO

#$1	HELLO	$3
A	G	C
A	azd	x


```





