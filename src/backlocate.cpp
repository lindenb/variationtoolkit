/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Nov 2011
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:

 */
#include <set>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <iostream>
#include "zstreambuf.h"
#include "knowngene.h"
#include "xfaidx.h"
#include "mysqlapplication.h"
#include "where.h"
#include "genomicsequence.h"
#include "mutatedsequence.h"
#include "stringsequence.h"
#include "proteincharsequence.h"


using namespace std;

char complement(char c)
		{
		switch(c)
			{
			case 'A':case 'a': return 'T';
			case 'T':case 't': return 'A';
			case 'G':case 'g': return 'C';
			case 'C':case 'c': return 'G';
			default: THROW("bad base "<< c);
			}
		}

 class RNASequence:public AbstractCharSequence
		{
 	 	 public:
			vector<int32_t> genomicPositions;
			const GenomicSeq* genomic;
			char strand;
			RNASequence(const GenomicSeq* genomic,char strand):genomic(genomic),strand(strand)
				{
				}
			virtual char at(int32_t i) const
				{
				char c=genomic->at(this->genomicPositions.at(i));
				return (strand=='+'?c:complement(c));
				}
			virtual  int32_t size() const
				{
				return (int32_t)genomicPositions.size();
				}
		};

class BackLocate:public MysqlApplication
	    {
	    public:
			int geneCol;
			int mutCol;
			IndexedFasta* indexedFasta;
			bool printSequences;

		    BackLocate()
		    	{
		    	geneCol=0;
		    	mutCol=1;
		    	indexedFasta=NULL;
		    	printSequences=false;
		    	}

		    virtual ~BackLocate()
				{
				if(indexedFasta!=NULL) delete indexedFasta;

				}


	     void backLocate(
	    		const KnownGene* gene,
	    		const string& geneName,
	    		char aa1,
	    		char aa2,
	    		int peptidePos1
	    		)
	    		{
	    		const int extra=1000;
	    		const GeneticCode* geneticCode=GeneticCode::standard();
	    		auto_ptr<RNASequence> wildRNA;
	    		auto_ptr<ProteinCharSequence> wildProt;
	    		std::auto_ptr<GenomicSeq> genomicSeq=std::auto_ptr<GenomicSeq>(
	    			new 	GenomicSeq(
	    				this->indexedFasta,
	    				gene->chrom.c_str(),
	    				std::max(gene->txStart-extra,0),
	    				gene->txEnd+extra
	    				)
	    			);



	    	     if(gene->isForward())
	        		{
	        		int exon_index=0;
	        		while(exon_index< gene->countExons())
	        			{
	        			for(int i= gene->getExonStart(exon_index);
	        					i< gene->getExonEnd(exon_index);
	        					++i)
	        				{
	        				if(i< gene->cdsStart) continue;
	        				if(i>=gene->cdsEnd) break;

	    					if(wildRNA.get()==NULL)
	    						{
	    						wildRNA.reset(new RNASequence(genomicSeq.get(),'+'));
	    						}

	        				wildRNA->genomicPositions.push_back(i);

	        				if(wildRNA->size()%3==0 && wildRNA->size()>0 && wildProt.get()==NULL)
	            				{
	            				wildProt.reset(new ProteinCharSequence(geneticCode,wildRNA.get()));
	            				}
	        				}
	        			++exon_index;
	        			}
	        		}
	    	   else // reverse orientation
	        		{
	        		int exon_index = gene->countExons()-1;
	        		while(exon_index >=0)
	        			{
	        			for(int i= gene->getExonEnd(exon_index)-1;
	        				    i>= gene->getExonStart(exon_index);
	        				--i)
	        				{
	        				if(i>= gene->cdsEnd) continue;
	        				if(i<  gene->cdsStart) break;

	        				if(wildRNA.get()==NULL)
	    						{
	    						wildRNA.reset(new RNASequence(genomicSeq.get(),'-'));
	    						}



	        				wildRNA->genomicPositions.push_back(i);
	        				if( wildRNA->size()%3==0 &&
	        					wildRNA->size()>0 &&
	        					wildProt.get()==0)
	            				{
	            				wildProt.reset(new ProteinCharSequence(geneticCode,wildRNA.get()));
	            				}

	        				}
	        			--exon_index;
	        			}

	        		}//end of if reverse

	    	     if(wildProt.get())
	    	    	 {
	    	    	 cerr << "#no protein found for transcript:"<< gene->name << endl;
	    	    	 return;
	    	    	 }
	    	    int peptideIndex0= peptidePos1-1;
	            if(peptideIndex0 >=wildProt->size())
	            	{
	            	cerr << "#index out of range for :"<< gene->name << " peptide length="<< wildProt->size() << endl;
	    	    	return;
	            	}

	            if(wildProt->at(peptideIndex0)!=aa1)
	            	{
	            	cout << "##Warning ref aminod acid for "
	            			<< gene->name << "  ["<< peptidePos1<< "] is not the same ("
	            			<< wildProt->at(peptideIndex0)<< "/"<<aa1<<")"
	            			<< endl;
	            	}
	            else
	            	{
	            	cout << "##"<<gene->name << endl;
	            	}
	            int indexesInRNA[]={
	            	0+ peptideIndex0*3,
	            	1+ peptideIndex0*3,
	            	2+ peptideIndex0*3
	            	};
	            string codon;
	            codon += wildRNA->at(indexesInRNA[0]);
	            codon += wildRNA->at(indexesInRNA[1]);
	            codon += wildRNA->at(indexesInRNA[2]);


	            for(int idx=0;idx<3;++idx)
	            	{
	            	int indexInRna=indexesInRNA[idx];
	            	cout << geneName;
	            	cout << '\t';
	            	cout << aa1;
	            	cout << '\t';
	            	cout << peptidePos1;
	            	cout << '\t';
	            	cout << aa2;
	            	cout << '\t';
	            	cout << gene->name;
	            	cout << '\t';
	            	cout << gene->strand;
	            	cout << '\t';
	            	cout << wildProt->at(peptideIndex0);
	            	cout << '\t';
	            	cout << indexInRna;
	            	cout << '\t';
	            	cout << codon;
	            	cout << '\t';
	            	cout << wildRNA->at(indexInRna);
	            	cout << '\t';
	            	cout << gene->chrom ;
	            	cout << '\t';
	            	cout << wildRNA->genomicPositions.at(indexInRna);
	            	cout << '\t';
	            	cout << gene->getExonNameFromGenomicIndex(wildRNA->genomicPositions.at(indexInRna))->c_str();
	            	if(this->printSequences)
	            		{
	            		std::string s=*(wildRNA->toString());
	            		cout << '\t';
	                	cout << s.substr(0,indexInRna) << "["
	                		<< s.at(indexInRna)+"]"
	                		<< (indexInRna+1<(int32_t)s.size()?s.substr(indexInRna+1):"");
	                	s.assign(*(wildProt->toString()));
	                	cout << '\t';
	                	cout << s.substr(0,peptideIndex0)
	                		<< "["+aa1<<"/"<<aa2<<"/"
	                		<< wildProt->at(peptideIndex0) << "]"
	                		<< (peptideIndex0+1<(int32_t)s.size()?s.substr(peptideIndex0+1):"");
	            		}
	            	cout << endl;
	            	}
	    		}





		  std::vector<KnownGene*> getGenes(const char* geneName)
		  	    {
		  	    Tokenizer comma;
		  	    comma.delim=',';
		  	    vector<string> exonStarts;
		  	    vector<string> exonEnds;
		  	    std::vector<KnownGene*> genes;
		  	    ostringstream os;
		  	    os << "select "
		  		    "K.chrom,K.name,K.strand,K.txStart,K.txEnd,K.cdsStart,K.cdsEnd,K.exonCount,K.exonStarts,K.exonEnds "
		  		    " from knownGene as K, kgXref  as X "
		  	    	" where X.kgID=K.name and X.geneSymbol=\"" << geneName
		  		    << "\""
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
		  			g->chrom.assign(row[0]);

		  			g->name.assign(row[1]);
		  			g->strand=row[2][0];
		  			g->txStart=atoi(row[3]);
		  			g->txEnd=atoi(row[4]);
		  			g->cdsStart=atoi(row[5]);
		  			g->cdsEnd=atoi(row[6]);
		  			int exonCount=atoi(row[7]);
		  			comma.split(row[8],exonStarts);
		  			comma.split(row[9],exonEnds);
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



		  void run(std::istream& in)
		  	{
			string line;
			vector<string> tokens;
			vector<string> hash;
			while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				if(line[0]=='#')
					{
					continue;
					}
				tokenizer.split(line,tokens);
				if(this->geneCol>=(int)tokens.size())
					{
					cerr << "GENE: Column out of range in " << line << endl;
					continue;
					}
				if(this->mutCol>=(int)tokens.size())
					{
					cerr << "MUTATION: Column out of range in " << line << endl;
					continue;
					}


				std::string geneName(tokens[geneCol]);
				if(geneName.empty())
					{
					cerr << "Bad line. No gene in "+line << endl;
					continue;
					}
				std::string mut(tokens[mutCol]);
				if(mut.size()<3 ||
					!isalpha(mut[0]) ||
					!isdigit(mut[1]) ||
					!isalpha(mut[mut.size()-1]) ||
					!isdigit(mut[mut.size()-2])
					)
					{
					cerr << "BAD MUTATION in " << line << endl;
					continue;
					}
				char aa1= std::toupper(mut.at(0));
				char aa2= std::toupper(mut.at(mut.size()-1));
				char* p2=0;
				int position1=strtol(&(mut.c_str())[1],&p2,10);
				if(position1==0 || p2+1!=0)
					{
					cerr << "Bad position in " << line << endl;
					continue;
					}
				std::vector<KnownGene*> genes=getGenes(geneName.c_str());
				for(size_t i=0;i< genes.size();++i)
					{
					KnownGene* gene=genes.at(i);
					backLocate(gene, geneName, aa1, aa2, position1);
					delete gene;
					}
				}
		  	  }



		  void usage(ostream& out,int argc,char **argv)
		  		{
		  		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		  		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		  		out << "Options:\n";
		  		out << "  -g (column) gene name default:"<< (geneCol+1)<< "\n";
		  		out << "  -m (column) mutation in protein default:"<< (mutCol+1)<< "\n";
		  		out << "  -f (pasta to fasta reference indexed with faidx).\n";
		  		out << "Other options:\n";
		  		out << "  -d delimiter. Default:tab\n";
		  		out << "(stdin|vcf|vcf.gz)\n";
		  		MysqlApplication::printConnectOptions(out);
		  		}


		  int main(int argc,char** argv)
			{
			char* fasta=NULL;
			int n_optind=0;
			int optind=1;
			while(optind < argc)
				{
				if(strcmp(argv[optind],"-h")==0)
					{
					usage(cerr,argc,argv);
					return EXIT_SUCCESS;
					}
				else if(strcmp(argv[optind],"-f")==0 && optind+1< argc)
					{
					fasta=argv[++optind];
					}
				else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
					{
					char* p=argv[++optind];
					if(strlen(p)!=1)
						{
						cerr << "Bad delimiter \""<< p << "\"\n";
						usage(cerr,argc,argv);
						return (EXIT_FAILURE);
						}
					this->tokenizer.delim=p[0];
					}
				else if(strcmp(argv[optind],"-g")==0 && optind+1<argc)
					{
					char* p2;
					geneCol=(int)strtol(argv[++optind],&p2,10);
					if(*p2!=0 || geneCol<1)
						{
						cerr << "Bad gene column:" << argv[optind] << endl;
						return EXIT_FAILURE;
						}
					geneCol--;/* to 0-based */
					}
				else if(strcmp(argv[optind],"-m")==0 && optind+1<argc)
					{
					char* p2;
					mutCol=(int)strtol(argv[++optind],&p2,10);
					if(*p2!=0 || mutCol<1)
						{
						cerr << "Bad mutation column:" << argv[optind] << endl;
						usage(cerr,argc,argv);
						return EXIT_FAILURE;
						}
					mutCol--;/* to 0-based */
					}
				else if((n_optind= this->argument(optind,argc,argv))!=-1)
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
					usage(cerr,argc,argv);
					exit(EXIT_FAILURE);
					}
				else
					{
					break;
					}
				++optind;
				}
		   this->connect();
		   if(fasta==NULL)
			   {
			   cerr << "undefined fasta reference" << endl;
			   usage(cerr,argc,argv);
			   exit(EXIT_FAILURE);
			   }
		   if(mutCol==-1)
		   	   {
			   cerr << "undefined mutation column." << endl;
			   usage(cerr,argc,argv);
			   exit(EXIT_FAILURE);
		   	   }

		   if(geneCol==-1)
			   {
			   cerr << "undefined gene column." << endl;
			   usage(cerr,argc,argv);
			   exit(EXIT_FAILURE);
			   }
		   this->indexedFasta=new IndexedFasta(fasta);
		   this->connect();


		   cout << "#User.Gene";
		           	cout << '\t';
		           	cout << "AA1";
		           	cout << '\t';
		           	cout << "petide.pos.1";
		           	cout << '\t';
		           	cout << "AA2";
		           	cout << '\t';
		           	cout << "knownGene.name";
		           	cout << '\t';
		           	cout << "knownGene.strand";
		           	cout << '\t';
		           	cout << "knownGene.AA";
		           	cout << '\t';
		           	cout << "index0.in.rna";
		           	cout << '\t';
		           	cout << "codon";
		           	cout << '\t';
		           	cout << "base.in.rna";
		           	cout << '\t';
		           	cout << "chromosome";
		           	cout << '\t';
		           	cout << "index0.in.genomic";
		           	cout << '\t';
		           	cout << "exon";
		           	if(printSequences)
		           		{
		           		cout << '\t';
		               	cout << "mRNA";
		               	cout << '\t';
		               	cout << "protein";
		           		}
		           cout << endl;
		   if(optind==argc)
				{
				igzstreambuf buf;
				istream in(&buf);
				run(in);
				}
			else
				{
				while(optind< argc)
					{
					igzstreambuf buf(argv[optind++]);
					istream in(&buf);
					this->run(in);
					buf.close();
					++optind;
					}
				}

			return EXIT_SUCCESS;
			}

	    };


int main(int argc,char** argv)
	{
	BackLocate app;
	return app.main(argc,argv);
	}
