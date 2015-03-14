

Displays a progress bar on the console.

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
make ../bin/showprogress
```


## Option ##

  * **-e** (x:int) print every 'x' lines (optional)
  * **-p** (string) optional prefix.


## Example ##


```
$ tr "\0" "\n" < /dev/zero |\
head -n 100000 |\awk '{for(i=0;i< 10;++i) {print $0,i;}}' |\
showprogress | grep 8 > /dev/null
1000000 lines.

```





