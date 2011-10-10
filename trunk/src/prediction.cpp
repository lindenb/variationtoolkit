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
#include <mysql.h>

#include "geneticcode.h"
#include "zstreambuf.h"
#include "xfaidx.h"
#include "tokenizer.h"
#include "knowngene.h"

using namespace std;

class AbstractCharSequence
	{
	public:
	    AbstractCharSequence()
		{
		}
	    virtual ~AbstractCharSequence()
		{
		}
	    virtual char at(int32_t index) const=0;
	    virtual int32_t size() const=0;
	    char operator[](int32_t index) const
		{
		return at(index);
		}
	};


class GenomicSeq:public AbstractCharSequence
    {
    private:
	std::auto_ptr<std::string> array;
	int chromStart;
    public:
     GenomicSeq(IndexedFasta* indexedFasta,
	     const char* chrom,
	     int32_t chromStart0,
	     int32_t chromEnd0
	     )
	    {
	    this->array = indexedFasta->fetch(chrom,chromStart0,chromEnd0);
	    this->chromStart=chromStart0;
	    }

     virtual ~GenomicSeq()
	{
	}

    int getChromStart() const
	    {
	    return chromStart;
	    }
    int getChromEnd() const
	    {
	    return getChromStart()+ (int32_t)array->size();
	    }

    virtual char at(int32_t index) const
	    {
	    if(index < getChromStart() || index>= getChromEnd() ) return '?';
	    return std::toupper(array->at(index-getChromStart()));
	    }
    virtual int32_t size() const
	    {
	    return getChromEnd();
	    }
    };

class ProteinCharSequence:public AbstractCharSequence
    {
    private:
	const GeneticCode* code;
	const AbstractCharSequence* delegate;
    public:
	ProteinCharSequence(
		const GeneticCode* code,
		const AbstractCharSequence* delegate):
		    code(code),delegate(delegate)
		{
		}
	virtual ~ProteinCharSequence()
	    {
	    }

	virtual char at(int32_t index) const
		{
		return code->translate(
			delegate->at(index*3+0),
			delegate->at(index*3+1),
			delegate->at(index*3+2)
			);
		}

	virtual int32_t size() const
		{
		return delegate->size()/3;
		}
	};

class MutedSequence:public AbstractCharSequence
    {
    private:
	    const AbstractCharSequence* delegate;
    public:
	    std::map<int32_t,char> mutations;
	    MutedSequence(const AbstractCharSequence* delegate):delegate(delegate)
		{
		}
	    virtual ~MutedSequence()
		{
		}
	    virtual char at(int32_t index) const
		{
		std::map<int32_t,char>::const_iterator r= mutations.find(index);
		return r==mutations.end()?delegate->at(index):r->second;
		}
	    virtual int32_t size() const
		{
		return delegate->size();
		}
	};

class StringSequence:public AbstractCharSequence
    {
    public:
	std::string content;
	StringSequence()
	    {
	    }
	virtual ~StringSequence()
	    {
	    }
	virtual char at(int32_t index) const
	    {
	    return content.at(index);
	    }
	virtual int32_t size() const
	    {
	    return (int32_t)content.size();
	    }
    };

class Consequence
	{
	public:
	    KnownGene* gene;
	    set<std::string> type;
	    set<std::string> splicing;
	    std::string exonName;
	    std::string intronName;
	    int position_in_cdna;
	    int position_protein;
	    std::string wildCodon;
	    std::string mutCodon;
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
		mutProt=NULL;
		mutRNA=NULL;
		}
	    ~Consequence()
		{
		if(wildRNA!=NULL) delete wildRNA;
		if(mutRNA!=NULL) delete mutRNA;
		if(wildProt!=NULL) delete wildProt;
		if(mutProt!=NULL) delete mutProt;
		}
	};


class Prediction
    {
    public:
	Tokenizer tokenizer;
	IndexedFasta* indexedFasta;
	const char* fasta;
	MYSQL* mysql;
	Prediction():indexedFasta(NULL),fasta(NULL),mysql(NULL)
	    {
	    if((mysql=::mysql_init(NULL))==NULL) THROW("Cannot init mysql");
	    }
	~Prediction()
	    {
	    if(indexedFasta!=NULL) delete indexedFasta;
	    ::mysql_close(mysql);
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
	    MYSQL_ROW row;
	    if(mysql_real_query( mysql, query.c_str(),query.size())!=0)
		 {
		 THROW("Failure for "<< query << "\n" << mysql_error(mysql));
		 }
	    MYSQL_RES* res=mysql_use_result( mysql );
	    //int ncols=mysql_field_count(mysql);
	    while(( row = mysql_fetch_row( res ))!=NULL )
		{
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
		    Exon exon;
		    exon.index=i;
		    exon.gene=g;
		    exon.start=atoi(exonStarts[i].c_str());
		    exon.end=atoi(exonEnds[i].c_str());
		    g->exons.push_back(exon);
		    }
		}
	    ::mysql_free_result( res );
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
	bool isStop(char c)
	    {
	    return c=='*';
	    }

	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    GenomicSeq* genomicSeq=NULL;
	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(line[0]=='#')
		    {
		    cout << line;

		    cout << endl;
		    continue;
		    }
		tokenizer.split(line,tokens);
		char *p2;

		int pos1= strtol(tokens[1].c_str(),&p2,10);
		int position0=pos1-1;
		vector<KnownGene*> genes=getGenes(tokens[0],position0);
		bool found=false;
		const GeneticCode* geneticCode=GeneticCode::standard();
		if(tokens[0].compare("chrM")==0)
		    {
		    geneticCode=GeneticCode::mitochondrial();
		    }
		std::string refBase=tokens[3];
		std::string alt=tokens[4];
		for(size_t i=0;i< genes.size();++i)
			{
			KnownGene* gene=genes.at(i);

			if(position0 >= gene->txEnd) continue;
			if(position0 < gene->txStart) continue;
			found=true;
			Consequence* consequence=new Consequence;
			consequence->gene=gene;
			if( (refBase.empty() || refBase.compare("A")==0 || refBase.compare("T")==0 || refBase.compare("G")==0 || refBase.compare("C")==0) &&
				(alt.compare("A")==0 || alt.compare("T")==0 || alt.compare("G")==0 || alt.compare("C")==0)
			)
			    {
			    if(genomicSeq==NULL ||
			      !(genomicSeq->getChromStart()<=gene->txStart && gene->txEnd <= genomicSeq->getChromEnd())
				)
				{
				int start=std::max(gene->txStart-100,0);
				if(genomicSeq!=NULL) delete genomicSeq;
				genomicSeq=new GenomicSeq(indexedFasta,gene->chrom.c_str(),start,gene->txEnd+100);
				}
			    std::string genomicAtpos0;
			    genomicAtpos0+=genomicSeq->at(position0);
			    if(!refBase.empty() && genomicAtpos0.compare(refBase)!=0)
				{
				cerr << "WARNING REF!=GENOMIC SEQ!!! at "<< tokens[0]<< ":"<<(position0+1)<< ":"<< genomicSeq->at(position0)<<"/"<<refBase << endl;
				}

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
					for(int32_t i= exon->start;
						i< exon->end;
						++i)
					    {
					    if(i==position0)
						{
						consequence->exonName.assign(exon->name());
						}
					    if(i< gene->cdsStart) continue;
					    if(i>=gene->cdsEnd) break;

					    if(consequence->wildRNA==NULL)
						{
						consequence->wildRNA=new StringSequence;
						consequence->mutRNA=new MutedSequence(consequence->wildRNA);
						}

					    if(i==position0)
						{
						consequence->type.insert("EXON");
						consequence->exonName.assign(exon->name());
						consequence->position_in_cdna=consequence->wildRNA->size();

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

					    consequence->wildRNA->content+=(genomicSeq->at(i));

					    if(i==position0)
						{
						consequence->mutRNA->mutations.insert(make_pair<int32_t,char>(
							consequence->position_in_cdna,
							alt.at(0)
						    ));
						}

					    if(	consequence->wildRNA->size()%3==0 &&
						    consequence->wildRNA->size()>0 &&
						    consequence->wildProt==NULL)
						{
						consequence->wildProt=new ProteinCharSequence(geneticCode,consequence->wildRNA);
						consequence->mutProt=new ProteinCharSequence(geneticCode,consequence->mutRNA);
						}
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
						consequence->exonName= exon->name();
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
				}//end of if reverse
			    if( consequence->wildProt!=NULL &&
				    consequence->mutProt!=NULL &&
				    consequence->position_in_cdna>=0)
				{
				int pos_aa=consequence->position_in_cdna/3;
				int mod= consequence->position_in_cdna%3;
				consequence->wildCodon=""+
					consequence->wildRNA->at(consequence->position_in_cdna-mod+0)+
					consequence->wildRNA->at(consequence->position_in_cdna-mod+1)+
					consequence->wildRNA->at(consequence->position_in_cdna-mod+2)
					;
				consequence->mutCodon=""+
					consequence->mutRNA->at(consequence->position_in_cdna-mod+0)+
					consequence->mutRNA->at(consequence->position_in_cdna-mod+1)+
					consequence->mutRNA->at(consequence->position_in_cdna-mod+2)
					;
				consequence->position_protein=pos_aa+1;
				consequence->wildAA=consequence->wildProt->at(pos_aa);
				consequence->mutAA=consequence->mutProt->at(pos_aa);
				if(isStop(consequence->wildProt->at(pos_aa)) &&
					!isStop(consequence->mutProt->at(pos_aa)))
				    {
				    consequence->type.insert("EXON_STOP_LOST");
				    }
				else if( !isStop(consequence->wildProt->at(pos_aa)) &&
					isStop(consequence->mutProt->at(pos_aa)))
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
			    }//end of simpe ATCG
			else
			    {
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
						consequence->exonName=exon->name();
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
						consequence->exonName=exon->name();
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
						consequence->exonName= exon->name();
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

			found=true;
			delete consequence;
			}


		    while(!genes.empty())
			{
			delete genes.back();
			genes.pop_back();
			}
		    }
		if(genomicSeq!=NULL) delete genomicSeq;
		}


	void usage(int argc,char** argv)
		{
		cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		cerr << "Options:\n";
		cerr << "  -d <column-delimiter> (default:tab)" << endl;
		cerr << "  -f genome file indexed with samtools faidx." << endl;
		}
    };

int main(int argc,char** argv)
    {
    Prediction app;
    string host("genome-mysql.cse.ucsc.edu");
    string username("genome");
    string password;
    string database("hg19");

    int port=0;
    int optind=1;
    while(optind < argc)
	{
	if(strcmp(argv[optind],"-h")==0)
	    {
	    app.usage(argc,argv);
	    return(EXIT_FAILURE);
	    }
	else if(std::strcmp(argv[optind],"--host")==0 && optind+1<argc)
	    {
	    host.assign(argv[++optind]);
	    }
	else if(std::strcmp(argv[optind],"--user")==0 && optind+1<argc)
	    {
	    username.assign(argv[++optind]);
	    }
	else if(std::strcmp(argv[optind],"--password")==0 && optind+1<argc)
	    {
	    password.assign(argv[++optind]);
	    }
	else if(std::strcmp(argv[optind],"--port")==0 && optind+1<argc)
	    {
	    port=atoi(argv[++optind]);
	    }
	else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
	    {
	    char* p=argv[++optind];
	    if(strlen(p)!=1)
		{
		fprintf(stderr,"Bad delimiter \"%s\"\n",p);
		return (EXIT_FAILURE);
		}
	    app.tokenizer.delim=p[0];
	    }
	else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
	    {
	    app.fasta=argv[++optind];
	    }
	else if(strcmp(argv[optind],"--")==0)
	    {
	    ++optind;
	    break;
	    }
	else if(argv[optind][0]=='-')
	    {
	    cerr << "unknown option '" << argv[optind]<< "'" << endl;
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
	return EXIT_FAILURE;
	}

    if(::mysql_real_connect(
    	    app. mysql,
    	    host.c_str(),
    	    username.c_str(),
    	    password.c_str(),
    	    database.c_str(), port,NULL, 0 )==NULL)
    	{
    	THROW("mysql_real_connect failed "<< mysql_error(app.mysql));
    	}


    app.indexedFasta=new IndexedFasta(app.fasta);

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
	    igzstreambuf buf(argv[optind++]);
	    istream in(&buf);
	    app.run(in);
	    buf.close();
	    ++optind;
	    }
	}
    return EXIT_SUCCESS;
    }
