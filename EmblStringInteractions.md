

Calls the service: EMBL String interactions ( [http://string-db.org/help/index.jsp?topic=/org.string-db.docs/api.html](http://string-db.org/help/index.jsp?topic=/org.string-db.docs/api.html) ).

## Options ##

  * -d (char) delimiter default:tab
  * -c column identifier


## Example ##



```

$ echo -e "#Gene\nNOTCH2\nEIF4G1\nPABPC1" | \
   emblstringresolve -c 1 | \
   emblstringinteractions -c 2 | \
   verticalize 
   
>>>	2
$1	#Gene          	NOTCH2
$2	stringId       	9606.ENSP00000256646
$3	preferredName  	NOTCH2
$4	annotation     	Notch homolog 2 (Drosophila); Functions as a receptor for membrane-bound ligands Jagged1, Jagged2 and Delta1 to regulate cell-fate determination. Upon ligand activation through the released notch intracellular domain (NICD) it forms a transcriptional activator complex with RBP-J kappa and activates genes of the enhancer of split locus. Affects the implementation of differentiation, proliferation and apoptotic programs (By similarity)
$5	interactorA    	string:9606.ENSP00000355718
$6	interactorB    	string:9606.ENSP00000326366
$7	labelA         	DLL1
$8	labelB         	PSEN1
$9	aliasesA       	-
$10	aliasesB       	-
$11	method         	-
$12	firstAuthor    	-
$13	publication    	-
$14	taxonA         	taxid:9606
$15	taxonB         	taxid:9606
$16	types          	-
$17	sources        	-
$18	interaction.ids	-
$19	score          	score:0.999|escore:0.639|dscore:0.9|tscore:0.984
<<<	2

>>>	3
$1	#Gene          	NOTCH2
$2	stringId       	9606.ENSP00000256646
$3	preferredName  	NOTCH2
$4	annotation     	Notch homolog 2 (Drosophila); Functions as a receptor for membrane-bound ligands Jagged1, Jagged2 and Delta1 to regulate cell-fate determination. Upon ligand activation through the released notch intracellular domain (NICD) it forms a transcriptional activator complex with RBP-J kappa and activates genes of the enhancer of split locus. Affects the implementation of differentiation, proliferation and apoptotic programs (By similarity)
$5	interactorA    	string:9606.ENSP00000345206
$6	interactorB    	string:9606.ENSP00000292599
$7	labelA         	RBPJ


```





