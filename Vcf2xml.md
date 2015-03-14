


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

## Motivation ##

**vcf2xml** a "Variant Call Format" ([VCF](http://www.1000genomes.org/wiki/Analysis/Variant+Call+Format/vcf-variant-call-format-version-40)) document to XML so it can be later processed with [xslt](http://www.w3.org/TR/xslt), [xquery](http://www.w3.org/TR/xquery/), etc...

## Dependencies ##

`libxml` [http://xmlsoft.org/](http://xmlsoft.org/) ( see also: [HowToInstall](HowToInstall.md) )

## Compiling ##


```
$ cd variationtoolkit/src/
$ make ../bin/vcf2xml

g++ -o ../bin/vcf2xml vcf2xml.cpp application.o -O3 -Wall `xml2-config --cflags --libs` -lz
```


## Usage: ##


```
vcf2xml (file.vcf | stdin)
```


## Example: ##


```
$ vcf2xml input.vcf | xmllint --format -

<?xml version="1.0" encoding="UTF-8"?>
<vcf>
  <head>
    <meta key="fileformat">VCFv4.1</meta>
    <meta key="samtoolsVersion">0.1.17 (r973:277)</meta>
    <infos>
      <info>
        <id>DP</id>
        <number>1</number>
        <type>Integer</type>
        <description>Raw read depth</description>
      </info>
      <info>
        <id>DP4</id>
        <number>4</number>
        <type>Integer</type>
        <description># high-quality ref-forward bases</description>
      </info>
      <info>
        <id>MQ</id>
(...)
      </calls>
    </variation>
    <variation>
      <chrom>chr1</chrom>
      <pos>112697</pos>
      <ref>T</ref>
      <alt>G</alt>
      <qual>10.4</qual>
      <infos>
        <info key="DP">1</info>
        <info key="AF1">1</info>
        <info key="AC1">2</info>
        <info key="DP4">0,0,0,1</info>
        <info key="MQ">60</info>
        <info key="FQ">-30</info>
      </infos>
      <calls>
        <call sample="input.bam">
          <prop key="GT">1/1</prop>
          <prop key="PL">40,3,0</prop>
          <prop key="GQ">5</prop>
        </call>
      </calls>
    </variation>
  </body>
</vcf>
```





