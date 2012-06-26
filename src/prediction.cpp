/*
 * prediction.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdint.h>
#include "mysqlapplication.h"
#include "zstreambuf.h"
#include "xfaidx.h"
#include "tokenizer.h"
#include "knowngene.h"
#define NOWHERE
#include "where.h"
#include "genomicsequence.h"
#include "mutatedsequence.h"
#include "stringsequence.h"
#include "proteincharsequence.h"
using namespace std;




/**
 * StringSequence
 */
class Consequence
	{
	public:
	    KnownGene* gene;
	    /** type of mutation (non-synonymous) */
	    set<std::string> type;
	    set<std::string> splicing;
	    std::string exonName;
	    std::string intronName;
	    int position_in_cdna;
	    int position_protein;
	    std::string* wildCodon;
	    std::string* mutCodon;
	    char wildAA;
	    char mutAA;
	    StringSequence* wildRNA;
	    ProteinCharSequence* wildProt;
	    ProteinCharSequence* mutProt;
	    MutedSequence* mutRNA;
	    Consequence()
		{
		gene=NULL;
		position_in_cdna=-1;
		position_protein=-1;
		wildAA='\0';
		mutAA='\0';
		wildProt=NULL;
		wildRNA=NULL;
		mutProt=NULL;
		mutRNA=NULL;
		wildCodon=NULL;
		mutCodon=NULL;
		}
	    ~Consequence()
		{
		if(wildCodon!=NULL) delete wildCodon;
		if(mutCodon!=NULL) delete mutCodon;
		if(wildRNA!=NULL) delete wildRNA;
		if(mutRNA!=NULL) delete mutRNA;
		if(wildProt!=NULL) delete wildProt;
		if(mutProt!=NULL) delete mutProt;
		}

	};

/**
 * Prediction
 */
class Prediction:public MysqlApplication
    {
    public:
	IndexedFasta* indexedFasta;
	const char* fasta;
	int chromColumn;
	int posColumn;
	int refColumn;
	int altColumn;
	bool print_sequences;

	Prediction():MysqlApplication(),
		indexedFasta(NULL),
		fasta(NULL),
		chromColumn(0),
		posColumn(1),
		refColumn(3),
		altColumn(4),
		print_sequences(true)
	    {
	    }
	virtual ~Prediction()
	    {
	    if(indexedFasta!=NULL) delete indexedFasta;
	    }
	
	std::vector<KnownGene*> getGenes(
	    std::string& chrom,
	    int32_t pos0
	    )
	    {
	    Tokenizer comma;
	    comma.delim=',';
	    vector<string> exonStarts;
	    vector<string> exonEnds;
	    std::vector<KnownGene*> genes;
	    ostringstream os;
	    os << "select "
		    "name,strand,txStart,txEnd,cdsStart,cdsEnd,exonCount,exonStarts,exonEnds "
		    " from knownGene where chrom=\"" << chrom
		    << "\" and txStart<="<< pos0
		    << " and txEnd >= "<< pos0
		    ;
	    string query=os.str();
	    WHERE(query);
	    MYSQL_ROW row;
	    if(mysql_real_query( mysql, query.c_str(),query.size())!=0)
		 {
		 THROW("Failure for "<< query << "\n" << mysql_error(mysql));
		 }
	    MYSQL_RES* res=mysql_use_result( mysql );
	    //int ncols=mysql_field_count(mysql);
	    while(( row = mysql_fetch_row( res ))!=NULL )
			{
	    	WHERE("reading row");
			KnownGene* g=new KnownGene;
			genes.push_back(g);
			g->chrom.assign(chrom);

			g->name.assign(row[0]);
			g->strand=row[1][0];
			g->txStart=atoi(row[2]);
			g->txEnd=atoi(row[3]);
			g->cdsStart=atoi(row[4]);
			g->cdsEnd=atoi(row[5]);
			int exonCount=atoi(row[6]);
			comma.split(row[7],exonStarts);
			comma.split(row[8],exonEnds);
			for(int i=0;i< exonCount;++i)
				{
				WHERE("exons");
				Exon exon;
				exon.index=i;
				exon.gene=g;
				exon.start=atoi(exonStarts[i].c_str());
				exon.end=atoi(exonEnds[i].c_str());
				g->exons.push_back(exon);
				}
			}
	    ::mysql_free_result( res );
	    WHERE("returning " << genes.size() << " genes");
	    return genes;
	    }


	char complement(char c)
	    {
	    switch(c)
		{
		case 'A': case 'a': return 'T';
		case 'G': case 'g': return 'C';
		case 'T': case 't': return 'A';
		case 'C': case 'c': return 'G';
		default: return 'N';
		}
	    }



	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    GenomicSeq* genomicSeq=NULL;
	    while(getline(in,line,'\n'))
		{
	    if(AbstractApplication::stopping()) break;
		if(line.empty()) continue;
		if(line[0]=='#')
		    {

		    if(line.size()>1 && line[1]=='#')
			{
			cout << line << endl;
			continue;
			}

		    cout << line << tokenizer.delim <<
			    "knownGene.name" << tokenizer.delim <<
			    "knownGene.strand" << tokenizer.delim <<
			    "knownGene.txStart" << tokenizer.delim <<
			    "knownGene.txEnd" << tokenizer.delim <<
			    "knownGene.cdsStart" << tokenizer.delim <<
			    "knownGene.cdsEnd" << tokenizer.delim <<
			    "prediction.type" << tokenizer.delim <<
			    "prediction.pos_in_cdna" << tokenizer.delim <<
			    "prediction.pos_in_protein" << tokenizer.delim <<
			    "prediction.exon" << tokenizer.delim <<
			    "prediction.intron" << tokenizer.delim <<
			    "prediction.wild.codon" << tokenizer.delim <<
			    "prediction.mut.codon" << tokenizer.delim <<
			    "prediction.wild.aa" << tokenizer.delim <<
			    "prediction.mut.aa";
		if(print_sequences)
			{	    
			     cout   << tokenizer.delim <<
			    "prediction.wild.prot" << tokenizer.delim <<
			    "prediction.mut.prot" << tokenizer.delim <<
			    "prediction.wild.rna" << tokenizer.delim <<
			    "prediction.mut.rna";
			    }
		    cout << tokenizer.delim <<
			    "prediction.splicing"
			    ;
		    cout << endl;
		    continue;
		    }
		WHERE(tokens.size());
		tokenizer.split(line,tokens);
		char *p2;
		WHERE(line);
		int pos1= strtol(tokens[posColumn].c_str(),&p2,10);
		int position0=pos1-1;
		WHERE("loading genes");
		vector<KnownGene*> genes=getGenes(tokens[chromColumn],position0);
		WHERE("done"<< genes.size());
		bool found=false;
		WHERE("");
		const GeneticCode* geneticCode=GeneticCode::standard();
		WHERE((geneticCode!=NULL));
		if(tokens[chromColumn].compare("chrM")==0)
		    {
		    geneticCode=GeneticCode::mitochondrial();
		    }
		std::string refBase=tokens[refColumn];
		std::string alt=tokens[altColumn];
		for(size_t i=0;i< refBase.size();++i) refBase[i]=(char)toupper(refBase[i]);
		for(size_t i=0;i< alt.size();++i) alt[i]=(char)toupper(alt[i]);

		/** loop over the genes */
		for(size_t i=0;i< genes.size();++i)
		    {
			WHERE("gene is "<< i);
		    KnownGene* gene=genes.at(i);

		    if(position0 >= gene->txEnd)
		    	{
		    	WHERE("");
		    	continue;
		    	}
		    if(position0 < gene->txStart)
		    	{
		    	WHERE("");
		    	continue;
		    	}
		    found=true;
		    Consequence* consequence=new Consequence;
		    consequence->gene=gene;
		    if(     (refBase.empty() || refBase.compare("A")==0 || refBase.compare("T")==0 || refBase.compare("G")==0 || refBase.compare("C")==0) &&
			    (alt.compare("A")==0 || alt.compare("T")==0 || alt.compare("G")==0 || alt.compare("C")==0)
		    )
			{
			if(genomicSeq==NULL ||
				!(genomicSeq->getChromStart()<=gene->txStart && gene->txEnd <= genomicSeq->getChromEnd())
			)
			    {
			    int start=std::max(gene->txStart-100,0);
			    if(genomicSeq!=NULL) delete genomicSeq;
			    WHERE("getseq");
			    genomicSeq=new GenomicSeq(indexedFasta,gene->chrom.c_str(),start,gene->txEnd+100);
			    }
			WHERE("");
			std::string genomicAtpos0;
			genomicAtpos0+=genomicSeq->at(position0);
			if(!refBase.empty() && genomicAtpos0.compare(refBase)!=0)
			    {
			    cerr << "WARNING REF!=GENOMIC SEQ!!! at "<< tokens[0]<< ":"<<(position0+1)<< "(+1):"<< genomicSeq->at(position0)<<"/"<<refBase << endl;
			    }

			if(gene->isForward())
			    {
			    WHERE("gene is forward");
			    if(position0 < gene->cdsStart)
				{
				WHERE(position0);
				consequence->type.insert("UTR5");
				}
			    else if( gene->cdsEnd <= position0 )
				{
				WHERE(gene->cdsStart << " - " << gene->cdsEnd << "<=" << position0);
				consequence->type.insert("UTR3");
				}
			    else
				{
				WHERE("");
				int32_t exon_index=0;
				while(exon_index< gene->countExons())
				    {
				    WHERE("");
				    const Exon* exon= gene->exon(exon_index);
				    for(int32_t i= exon->start;
					    i< exon->end;
					    ++i)
					{
					WHERE(i);
					if(i==position0)
					    {
					    WHERE(i);
					    consequence->exonName.assign(exon->name());
					    }
					WHERE(i);
					if(i< gene->cdsStart) continue;
					if(i>=gene->cdsEnd) break;
					WHERE(i);
					if(consequence->wildRNA==NULL)
					    {
					    consequence->wildRNA=new StringSequence;
					    consequence->mutRNA=new MutedSequence(consequence->wildRNA);
					    }
					WHERE(i);
					if(i==position0)
					    {
					    WHERE(i);
					    consequence->type.insert("EXON");
					    WHERE(i);
					    consequence->exonName.assign(exon->name());
					    WHERE(i);
					    assert(consequence->wildRNA!=NULL);
					    WHERE(consequence->wildRNA->size());
					    consequence->position_in_cdna=consequence->wildRNA->size();
					    WHERE(i);
					    //in splicing ?
					    if(exon->isSplicing(position0))
						{

						consequence->splicing.insert("SPLICING");

						if(exon->isSplicingAcceptor(position0))
						    {
						    consequence->splicing.insert("SPLICING_ACCEPTOR");
						    }
						else  if(exon->isSplicingDonor(position0))
						    {
						    consequence->splicing.insert( "SPLICING_DONOR");
						    }
						}
					    }
					WHERE(i);
					consequence->wildRNA->content+=(genomicSeq->at(i));

					if(i==position0)
					    {
					    WHERE(i);
					    consequence->mutRNA->mutations.insert(make_pair<int32_t,char>(
						    consequence->position_in_cdna,
						    alt.at(0)
					    ));
					    }

					if(	consequence->wildRNA->size()%3==0 &&
						consequence->wildRNA->size()>0 &&
						consequence->wildProt==NULL)
					    {
					    WHERE(i);
					    consequence->wildProt=new ProteinCharSequence(geneticCode,consequence->wildRNA);
					    consequence->mutProt=new ProteinCharSequence(geneticCode,consequence->mutRNA);
					    }
					}
				    WHERE("");
				    auto_ptr<Intron> intron= exon->getNextIntron();
				    if(intron.get()!=NULL && intron->contains(position0))
					{
					consequence->intronName=intron->name();
					consequence->type.insert("INTRON");

					if(intron->isSplicing(position0))
					    {
					    consequence->splicing.insert("INTRON_SPLICING");
					    if(intron->isSplicingAcceptor(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING_ACCEPTOR");
						}
					    else if(intron->isSplicingDonor(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING_DONOR");
						}
					    }
					}
				    ++exon_index;
				    }
				}
			    WHERE("");

			    }
			else // reverse orientation
			    {
				WHERE("gene is reverse");
			    if(position0 < gene->cdsStart)
				{
				consequence->type.insert("UTR3");
				}
			    else if( gene->cdsEnd <=position0 )
				{
				consequence->type.insert("UTR5");
				}
			    else
				{
				int32_t exon_index = gene->countExons()-1;
				while(exon_index >=0)
				    {
				    const Exon* exon= gene->exon(exon_index);
				    for(int i= exon->end-1;
					    i>= exon->start;
					    --i)
					{
					if(i==position0)
					    {
					    consequence->exonName.assign(exon->name());
					    }
					if(i>= gene->cdsEnd) continue;
					if(i<  gene->cdsStart) break;

					if(consequence->wildRNA==NULL)
					    {
					    consequence->wildRNA=new StringSequence;
					    consequence->mutRNA=new MutedSequence(consequence->wildRNA);
					    }

					if(i==position0)
					    {
					    consequence->type.insert("EXON");
					    consequence->position_in_cdna=consequence->wildRNA->size();

					    //in splicing ?
					    if(exon->isSplicing(position0))
						{
						consequence->splicing.insert( "INTRON_SPLICING");

						if(exon->isSplicingAcceptor(position0))
						    {
						    consequence->splicing.insert("INTRON_SPLICING_ACCEPTOR");
						    }
						else  if(exon->isSplicingDonor(position0))
						    {
						    consequence->splicing.insert("INTRON_SPLICING_DONOR");
						    }
						}


					    consequence->mutRNA->mutations.insert(make_pair<int32_t,char>(
						    consequence->position_in_cdna,
						    complement(alt.at(0))
					    ));
					    }

					consequence->wildRNA->content+=(complement(genomicSeq->at(i)));
					if( consequence->wildRNA->size()%3==0 &&
						consequence->wildRNA->size()>0 &&
						consequence->wildProt==NULL)
					    {
					    consequence->wildProt=new ProteinCharSequence(geneticCode,consequence->wildRNA);
					    consequence->mutProt=new ProteinCharSequence(geneticCode,consequence->mutRNA);
					    }

					}

				    auto_ptr<Intron> intron= exon->getPrevIntron();
				    if(intron.get()!=NULL &&
					    intron->contains(position0))
					{
					consequence->intronName=intron->name();
					consequence->type.insert("INTRON");

					if(intron->isSplicing(position0))
					    {
					    consequence->splicing.insert("INTRON_SPLICING");
					    if(intron->isSplicingAcceptor(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING_ACCEPTOR");
						}
					    else if(intron->isSplicingDonor(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING_DONOR");
						}
					    }
					}
				    --exon_index;
				    }
				}
			    WHERE("en of reverse");
			    }//end of if reverse
			WHERE("calc exon");
			if( consequence->wildProt!=NULL &&
				consequence->mutProt!=NULL &&
				consequence->position_in_cdna>=0)
			    {
				WHERE("end of consequence");
			    int pos_aa=consequence->position_in_cdna/3;
			    int mod= consequence->position_in_cdna%3;
			    assert(consequence->wildCodon==NULL);
			    assert(consequence->mutCodon==NULL);
			    consequence->wildCodon = new string;
			    consequence->mutCodon = new string;
			    *(consequence->wildCodon)+=consequence->wildRNA->at(consequence->position_in_cdna-mod+0);
			    *(consequence->wildCodon)+=consequence->wildRNA->at(consequence->position_in_cdna-mod+1);
			    *(consequence->wildCodon)+=consequence->wildRNA->at(consequence->position_in_cdna-mod+2);

			    *(consequence->mutCodon)+=consequence->mutRNA->at(consequence->position_in_cdna-mod+0);
			    *(consequence->mutCodon)+=consequence->mutRNA->at(consequence->position_in_cdna-mod+1);
			    *(consequence->mutCodon)+=consequence->mutRNA->at(consequence->position_in_cdna-mod+2);

			    consequence->position_protein=pos_aa+1;
			    consequence->wildAA=consequence->wildProt->at(pos_aa);
			    consequence->mutAA=consequence->mutProt->at(pos_aa);
			    if(geneticCode->isStop(consequence->wildProt->at(pos_aa)) &&
				    !geneticCode->isStop(consequence->mutProt->at(pos_aa)))
					{
					consequence->type.insert("EXON_STOP_LOST");
					}
			    else if( !geneticCode->isStop(consequence->wildProt->at(pos_aa)) &&
				    geneticCode->isStop(consequence->mutProt->at(pos_aa)))
					{
					consequence->type.insert( "EXON_STOP_GAINED");
					}
			    else if(consequence->wildProt->at(pos_aa)==consequence->mutProt->at(pos_aa))
					{
					consequence->type.insert("EXON_CODING_SYNONYMOUS");
					}
			    else
					{
					consequence->type.insert("EXON_CODING_NON_SYNONYMOUS");
					}
			    }
			else
				{
				WHERE("not in coding");
				}
			}//end of simpe ATCG
		    else
			{
		    	WHERE("not a simple ATGC "<< refBase << " "<< alt);
			int32_t wildrna=-1;

			if(gene->isForward())
			    {
			    if(position0 < gene->cdsStart)
				{
				consequence->type.insert("UTR5");
				}
			    else if( gene->cdsEnd <= position0 )
				{
				consequence->type.insert("UTR3");
				}
			    else
				{
				int32_t exon_index=0;
				while(exon_index< gene->countExons())
				    {
				    const Exon* exon= gene->exon(exon_index);
				    for(int i= exon->start;
					    i< exon->end;
					    ++i)
					{
					if(i==position0)
					    {
					    consequence->exonName.assign(exon->name());
					    }
					if(i< gene->cdsStart) continue;
					if(i>= gene->cdsEnd ) break;

					if(wildrna==-1)
					    {
					    wildrna=0;
					    }

					if(i==position0)
					    {
					    consequence->type.insert("EXON");
					    consequence->exonName.assign(exon->name());
					    consequence->position_in_cdna=wildrna;

					    //in splicing ?
					    if(exon->isSplicing(position0))
						{
						consequence->splicing.insert("SPLICING");

						if(exon->isSplicingAcceptor(position0))
						    {
						    consequence->splicing.insert("SPLICING_ACCEPTOR");
						    }
						else  if(exon->isSplicingDonor(position0))
						    {
						    consequence->splicing.insert("SPLICING_DONOR");
						    }
						}
					    }

					wildrna++;
					}
				    auto_ptr<Intron> intron= exon->getNextIntron();
				    if(intron.get()!=NULL && intron->contains(position0))
					{
					consequence->intronName=intron->name();
					consequence->type.insert("INTRON");

					if(intron->isSplicing(position0))
					    {
					    consequence->splicing.insert("INTRON_SPLICING");
					    if(intron->isSplicingAcceptor(position0))
						{
						consequence->splicing.insert( "INTRON_SPLICING_ACCEPTOR");
						}
					    else if(intron->isSplicingDonor(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING_DONOR");
						}
					    }
					}
				    ++exon_index;
				    }
				}
			    }
			else // reverse orientation
			    {

			    if(position0 < gene->cdsStart)
				{
				consequence->type.insert("UTR3");
				}
			    else if( gene->cdsEnd <=position0 )
				{
				consequence->type.insert("UTR5");
				}
			    else
				{

				int exon_index = gene->countExons()-1;
				while(exon_index >=0)
				    {
				    const Exon* exon= gene->exon(exon_index);
				    for(int32_t i= exon->end-1;
					    i>= exon->start;
					    --i)
					{
					if(i==position0)
					    {
					    consequence->exonName.assign(exon->name());
					    }
					if(i>= gene->cdsEnd) continue;
					if(i<  gene->cdsStart) break;

					if(wildrna!=-1)
					    {
					    wildrna=0;
					    }

					if(i==position0)
					    {
					    consequence->type.insert("EXON");
					    consequence->position_in_cdna=wildrna;

					    //in splicing ?
					    if(exon->isSplicing(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING");

						if(exon->isSplicingAcceptor(position0))
						    {
						    consequence->splicing.insert("INTRON_SPLICING_ACCEPTOR");
						    }
						else  if(exon->isSplicingDonor(position0))
						    {
						    consequence->splicing.insert("INTRON_SPLICING_DONOR");
						    }
						}
					    }

					wildrna++;
					}

				    auto_ptr<Intron> intron= exon->getPrevIntron();
				    if(intron.get()!=NULL &&
					    intron->contains(position0))
					{
					consequence->intronName.assign(intron->name());
					consequence->type.insert("INTRON");

					if(intron->isSplicing(position0))
					    {
					    consequence->splicing.insert("INTRON_SPLICING");
					    if(intron->isSplicingAcceptor(position0))
						{
						consequence->splicing.insert( "INTRON_SPLICING_ACCEPTOR");
						}
					    else if(intron->isSplicingDonor(position0))
						{
						consequence->splicing.insert("INTRON_SPLICING_DONOR");
						}
					    }
					}
				    --exon_index;
				    }
				}
			    }//end of if reverse
			if( wildrna!=-1 &&
				consequence->position_in_cdna>=0)
			    {
			    consequence->position_protein=consequence->position_in_cdna/3;
			    }
			}//end of not simple ATGC
		    WHERE("");
		    cout << line << tokenizer.delim
			    << gene->name << tokenizer.delim
			    << gene->strand << tokenizer.delim
			    << gene->txStart << tokenizer.delim
			    << gene->txEnd << tokenizer.delim
			    << gene->cdsStart << tokenizer.delim
			    << gene->cdsEnd << tokenizer.delim
			    ;
		    WHERE("");
		    if(consequence->type.empty())
			{
			cout << ".";
			}
		    else
			{
			for(set<string>::iterator r=consequence->type.begin();
			    r!= consequence->type.end();++r)
			    {
			    if(r!=consequence->type.begin()) cout << "|";
			    cout << (*r);
			    }
			}
		    cout << tokenizer.delim;
		    if(consequence->position_in_cdna!=-1)
			{
			cout << (consequence->position_in_cdna);
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;
		    if(consequence->position_protein!=-1)
			{
			cout << (consequence->position_protein);
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;
		    if(!consequence->exonName.empty())
			{
			cout << (consequence->exonName);
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;
		    if(!consequence->intronName.empty())
			{
			cout << (consequence->intronName);
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;
		    if(consequence->wildCodon!=NULL)
			{
			cout << *(consequence->wildCodon);
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;
		    if(consequence->mutCodon!=NULL)
			{
			cout << *(consequence->mutCodon);
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;
		    if(consequence->wildAA!=0)
			{
			cout << consequence->wildAA;
			}
		    else
			{
			cout << ".";
			}
		    cout << tokenizer.delim;

		    if(consequence->mutAA!=0)
			{
			cout << consequence->mutAA;
			}
		    else
			{
			cout << ".";
			}
		if(print_sequences)
			{
			cout << tokenizer.delim;
			if(consequence->wildProt!=NULL)
				{
				consequence->wildProt->print(cout);
				}
			else
				{
				cout << ".";
				}
			cout << tokenizer.delim;
			if(consequence->mutProt!=NULL)
				{
				consequence->mutProt->print(cout);
				}
			else
				{
				cout << ".";
				}
			cout << tokenizer.delim;
			if(consequence->wildRNA!=NULL)
				{
				consequence->wildRNA->print(cout);
				}
			else
				{
				cout << ".";
				}
			cout << tokenizer.delim;
			if(consequence->mutRNA!=NULL)
				{
				consequence->mutRNA->print(cout);
				}
			else
				{
				cout << ".";
				}
			}//end print_sequences
		    cout << tokenizer.delim;
		    if(consequence->splicing.empty())
			{
			cout << ".";
			}
		    else
			{
			for(set<string>::iterator r=consequence->splicing.begin();
			    r!= consequence->splicing.end();++r)
			    {
			    if(r!=consequence->splicing.begin()) cout << "|";
			    cout << (*r);
			    }
			}


		    cout << endl;
		    found=true;
		    delete consequence;
		    WHERE("");
		    }
		if(!found)
		    {
		    cout << line;
		    for(int i=0;i< 21;++i) cout << tokenizer.delim << ".";
		    cout << endl;
		    }

		while(!genes.empty())
		    {
		    delete genes.back();
		    genes.pop_back();
		    }
		}
	    if(genomicSeq!=NULL) delete genomicSeq;
	    }


	void usage(ostream& out,int argc,char** argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  -d <column-delimiter> (default:tab)" << endl;
		out << "  -f genome file indexed with samtools faidx." << endl;
		out << "  -c <CHROM col> (default:"<< chromColumn <<")" << endl;
		out << "  -p <POS col> (default:"<< posColumn <<")" << endl;
		out << "  -r <REF col> (default:"<< refColumn <<")" << endl;
		out << "  -a <ALT col> (default:"<< altColumn <<")" << endl;
		out << "  --noseq do not print sequences (RNA and prot)." << endl;
		MysqlApplication::printConnectOptions(out);
		}
    };
#define ARGVCOL(flag,var) else if(std::strcmp(argv[optind],flag)==0 && optind+1<argc)\
	{\
	char* p2;\
	app.var=(int)strtol(argv[++optind],&p2,10);\
	if(app.var<1 || *p2!=0)\
		{cerr << "Bad column for "<< flag << ".\n";app.usage(cerr,argc,argv);return EXIT_FAILURE;}\
	app.var--;\
	}


int main(int argc,char** argv)
    {
    Prediction app;
    int n_optind;
    int optind=1;
    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
			{
			app.usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		ARGVCOL("-c",chromColumn)
		ARGVCOL("-p",posColumn)
		ARGVCOL("-r",refColumn)
		ARGVCOL("-a",altColumn)
		else if(std::strcmp(argv[optind],"--noseq")==0)
			{
			app.print_sequences=false;
			}
		else if(std::strcmp(argv[optind],"--host")==0 && optind+1<argc)
			{
			app.host.assign(argv[++optind]);
			}
		else if(std::strcmp(argv[optind],"--user")==0 && optind+1<argc)
			{
			app.username.assign(argv[++optind]);
			}
		else if(std::strcmp(argv[optind],"--password")==0 && optind+1<argc)
			{
			app.password.assign(argv[++optind]);
			}
		else if(std::strcmp(argv[optind],"--database")==0 && optind+1<argc)
			{
			app.database.assign(argv[++optind]);
			}
		else if(std::strcmp(argv[optind],"--port")==0 && optind+1<argc)
			{
			app.port=atoi(argv[++optind]);
			}
		else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			{
			cerr<< "bad delimiter \"" << p << "\"\n";
			return (EXIT_FAILURE);
			}
			app.tokenizer.delim=p[0];
			}
		else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			app.fasta=argv[++optind];
			}
		else if((n_optind=app.argument(optind,argc,argv))!=-1)
			{
			if(n_optind==-2) return EXIT_FAILURE;
			optind=n_optind;
			}
		else if(strcmp(argv[optind],"--")==0)
			{
			++optind;
			break;
			}
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '" << argv[optind]<< "'" << endl;
			app.usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		else
			{
			break;
			}
		++optind;
		}

    if( app.fasta==NULL)
		{
		cerr << "genome fasta file undefined." << endl;
		 app.usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
    app.connect();
    WHERE("fasta");
    app.indexedFasta=new IndexedFasta(app.fasta);
    WHERE("reading");
    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		app.run(in);
		}
    else
		{
		while(optind< argc)
			{
			if(AbstractApplication::stopping()) break;
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			app.run(in);
			buf.close();
			++optind;
			}
		}
    return EXIT_SUCCESS;
    }
