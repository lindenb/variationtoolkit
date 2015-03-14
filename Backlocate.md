## Options: ##

  * **-g** (column) gene name default:1
  * **-m** (column) mutation in protein default:2
  * **-f** (pasta to fasta reference indexed with faidx).
  * **-p** print sequences.
  * **-d** delimiter. Default:tab
  * **--host** (mysql host) default:genome-mysql.cse.ucsc.edu
  * **--user** (mysql user) default:genome
  * **--password** (mysql password) default:
  * **--database** (mysql database) default:hg19
  * **--port** (mysql password) default:0



## Example: ##


```
echo -e  "NOTCH2\tM1T\nEIF4G1\tD240Y" |\
    backlocate -f /path/to/hg19.fa 

#User.Gene	AA1	petide.pos.1	AA2	knownGene.name	knownGene.strand	knownGene.AA	index0.in.rna	codon	base.in.rna	chromosome	index0.in.genomic	exon
##uc001eik.2
NOTCH2	M	1	T	uc001eik.2	-	M	0	ATG	A	chr1	120612019	Exon 1
NOTCH2	M	1	T	uc001eik.2	-	M	1	ATG	T	chr1	120612018	Exon 1
NOTCH2	M	1	T	uc001eik.2	-	M	2	ATG	G	chr1	120612017	Exon 1
##uc001eil.2
NOTCH2	M	1	T	uc001eil.2	-	M	0	ATG	A	chr1	120612019	Exon 1
NOTCH2	M	1	T	uc001eil.2	-	M	1	ATG	T	chr1	120612018	Exon 1
NOTCH2	M	1	T	uc001eil.2	-	M	2	ATG	G	chr1	120612017	Exon 1
##uc001eim.3
NOTCH2	M	1	T	uc001eim.3	-	M	0	ATG	A	chr1	120548116	Exon 2
NOTCH2	M	1	T	uc001eim.3	-	M	1	ATG	T	chr1	120548115	Exon 2
NOTCH2	M	1	T	uc001eim.3	-	M	2	ATG	G	chr1	120548114	Exon 2
##Warning ref aminod acid for uc003fnp.2  [240] is not the same (I/D)
EIF4G1	D	240	Y	uc003fnp.2	+	I	717	ATC	A	chr3	184039089	Exon 10
EIF4G1	D	240	Y	uc003fnp.2	+	I	718	ATC	T	chr3	184039090	Exon 10
EIF4G1	D	240	Y	uc003fnp.2	+	I	719	ATC	C	chr3	184039091	Exon 10
##Warning ref aminod acid for uc003fnu.3  [240] is not the same (I/D)
EIF4G1	D	240	Y	uc003fnu.3	+	I	717	ATC	A	chr3	184039089	Exon 9
EIF4G1	D	240	Y	uc003fnu.3	+	I	718	ATC	T	chr3	184039090	Exon 9
EIF4G1	D	240	Y	uc003fnu.3	+	I	719	ATC	C	chr3	184039091	Exon 9
##Warning ref aminod acid for uc003fnq.2  [240] is not the same (V/D)
EIF4G1	D	240	Y	uc003fnq.2	+	V	717	GTA	G	chr3	184039350	Exon 7
EIF4G1	D	240	Y	uc003fnq.2	+	V	718	GTA	T	chr3	184039351	Exon 7
EIF4G1	D	240	Y	uc003fnq.2	+	V	719	GTA	A	chr3	184039352	Exon 7
##Warning ref aminod acid for uc003fnr.2  [240] is not the same (L/D)
EIF4G1	D	240	Y	uc003fnr.2	+	L	717	CTC	C	chr3	184039581	Exon 6
EIF4G1	D	240	Y	uc003fnr.2	+	L	718	CTC	T	chr3	184039582	Exon 6
EIF4G1	D	240	Y	uc003fnr.2	+	L	719	CTC	C	chr3	184039583	Exon 6
##Warning ref aminod acid for uc003fny.3  [240] is not the same (T/D)
EIF4G1	D	240	Y	uc003fny.3	+	T	717	ACC	A	chr3	184039677	Exon 3
EIF4G1	D	240	Y	uc003fny.3	+	T	718	ACC	C	chr3	184039678	Exon 3
EIF4G1	D	240	Y	uc003fny.3	+	T	719	ACC	C	chr3	184039679	Exon 3
##uc010hxx.2
EIF4G1	D	240	Y	uc010hxx.2	+	D	717	GAT	G	chr3	184038780	Exon 10
EIF4G1	D	240	Y	uc010hxx.2	+	D	718	GAT	A	chr3	184039069	Exon 11
EIF4G1	D	240	Y	uc010hxx.2	+	D	719	GAT	T	chr3	184039070	Exon 11
##Warning ref aminod acid for uc003fns.2  [240] is not the same (L/D)
EIF4G1	D	240	Y	uc003fns.2	+	L	717	CTC	C	chr3	184039209	Exon 10
EIF4G1	D	240	Y	uc003fns.2	+	L	718	CTC	T	chr3	184039210	Exon 10
EIF4G1	D	240	Y	uc003fns.2	+	L	719	CTC	C	chr3	184039211	Exon 10
(...)
```

