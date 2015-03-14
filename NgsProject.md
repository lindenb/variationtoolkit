


This C++ program named ngsproject.cgi uses the samtools api, it allows any user to visualize all the alignments in a given NGS project. The projects and their BAMS are defined on the server side using a simple XML document.

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

# How to install on apache/httpd #

add the path to the cgi directory in the configuration file variationtoolkit/config.mk


```
(...)
CGI_BIN_DIR=/var/www/cgi-bin
```

Create the cgi-bin folder is needed

```
sudo mkdir -p /var/www/cgi-bin
sudo chmod 755 /var/www/cgi-bin
```

Create a XML file describing your project
```
cat /var/www/cgi-bin/ngsproject.xml  


<?xml version="1.0"?>
<!DOCTYPE projects [
<!ENTITY samdir "/tmp">
]>
<projects>
  <reference id="ref1">
    <name>Samtools1</name>
    <description>Samtools example 1</description>
    <path>&samdir;/examples/ex1.fa</path>
  </reference>
  <bam id="b1">
    <sample>Huey</sample>
    <path>&samdir;/examples/ex1.bam</path>
  </bam>
  <bam id="b2">
    <sample>Dewey</sample>
    <path>&samdir;/examples/ex1.bam</path>
  </bam>
  <bam id="b3">
    <sample>Louie</sample>
    <path>&samdir;/examples/ex1.bam</path>
  </bam>
  <project id="p1">
    <name>Project P1</name>
    <description>This is my 1st project</description>
    <bam ref="b1"/>
    <bam ref="b2"/>
    <bam ref="b3"/>
    <reference ref="ref1"/>
  </project>
  <project id="p2">
    <name>Project P2</name>
    <description>This is my 2nd project</description>
    <bam ref="b1"/>
    <bam ref="b3"/>
    <reference ref="ref1"/>
  </project>
</projects>
```

Edit the apache2/httpd config `/etc/apache2/apache2.conf` or `/etc/httpd/conf/httpd.conf`:

```
sudo nano /etc/apache2/apache2.conf
```

and add the following lines, specify **NGS\_PROJECT\_PATH**, the path to the XML file

```
<VirtualHost *:80>
ServerName localhost
DocumentRoot /var/www/
AddHandler cgi-script .cgi .pl 
SetEnv NGS_PROJECT_PATH /var/www/cgi-bin/ngsproject.xml    
<Directory /cgi-bin/>
        AllowOverride None
        Options ExecCGI -MultiViews +SymLinksIfOwnerMatch
        Order allow,deny
        Allow from all    
</Directory>

</VirtualHost>
```

and restart apache:

```
sudo /etc/init.d/apache2 restart
```


## Screenshot ##


![http://variationtoolkit.googlecode.com/svn/trunk/doc/ngsproject.jpg](http://variationtoolkit.googlecode.com/svn/trunk/doc/ngsproject.jpg)





