/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Jan 2012
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	http://biostar.stackexchange.com/questions/15925
 */
#include <set>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <iostream>
#include "zstreambuf.h"
#include "segments.h"
#include "knowngene.h"
#include "xfaidx.h"
#include "mysqlapplication.h"
#define NOWHERE
#include "where.h"
#include "numeric_cast.h"


using namespace std;



class ProteinToGenome:public MysqlApplication
	    {
	    private:


		    int32_t pepNameCol;
		    int32_t pepStartCol;
		    int32_t pepEndCol;
	    public:
		ProteinToGenome():
			pepNameCol(0),
			pepStartCol(1),
			pepEndCol(2)
		    {
		    }

		virtual ~ProteinToGenome()
		    {
		    }

	     void backLocate(
			const string& line,
	    		const KnownGene* gene,
	    		const string& pepName,
	    		int32_t pepStart0,
	    		int32_t pepEnd0
	    		)
	    		{
			vector<TidStartEnd> exonStartEnd;
			int32_t mRNA_size=0;
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
					    mRNA_size++;
					    int32_t pep_index=mRNA_size/3;
					    if(pepStart0<= pep_index && pep_index<=pepEnd0)
						{
						if(exonStartEnd.empty() || exonStartEnd.back().chrom!=exon_index)
						    {
						    TidStartEnd range(exon_index,i,i+1);
						    exonStartEnd.push_back(range);
						    }
						else
						    {
						    exonStartEnd.back().end++;
						    }
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
					    mRNA_size++;
					    int32_t pep_index=mRNA_size/3;
					    if(pepStart0<= pep_index && pep_index<=pepEnd0)
						{
						if(exonStartEnd.empty() || exonStartEnd.back().chrom!=exon_index)
						    {
						    TidStartEnd range(exon_index,i,i+1);
						    exonStartEnd.push_back(range);
						    }
						else
						    {
						    exonStartEnd.back().start--;
						    }
						}

					    }
	        			--exon_index;
	        			}

	        		}//end of if reverse

	    	     if(exonStartEnd.empty())
	    	    	 {
	    		cout << line
			      << tokenizer.delim << "."
			      << tokenizer.delim << "."
			      << tokenizer.delim << "."
			      << tokenizer.delim << "."
			      << tokenizer.delim << "."
			      << tokenizer.delim << "."
			      << endl;
	    	    	 }

	    	     for(vector<TidStartEnd>::iterator r=exonStartEnd.begin();r!=exonStartEnd.end();++r)
	    		 {
	    		 cout << line
	    		      << tokenizer.delim << gene->name
	    		      << tokenizer.delim << gene->chrom
	    		      << tokenizer.delim << gene->strand
	    		      << tokenizer.delim << "Exon "<< (gene->isForward()?r->chrom+1:(gene->countExons()-r->chrom))
	    		      << tokenizer.delim << r->start
	    		      << tokenizer.delim << r->end
	    		      << endl;
	    		 }
	    	     }





		  auto_ptr<std::vector<KnownGene*> > getGenes(const char* pepName)
		  	    {
		  	    Tokenizer comma;
		  	    comma.delim=',';
		  	    vector<string> exonStarts;
		  	    vector<string> exonEnds;
		  	    std::vector<KnownGene*>* genes=new std::vector<KnownGene*>;
		  	    ostringstream os;
		  	    os << "select "
		  		    "K.chrom,K.name,K.strand,K.txStart,K.txEnd,K.cdsStart,K.cdsEnd,K.exonCount,K.exonStarts,K.exonEnds "
		  		    " from knownGene as K, kgXref  as X "
		  	    	" where X.kgID=K.name and (X.spID=\"" << pepName
		  		    << "\" or spDisplayID=\""<< pepName << "\" "
		  		   <<  " or geneSymbol=\""<< pepName << "\")"
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

		  			KnownGene* g=new KnownGene;
		  			genes->push_back(g);
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
		  			WHERE(g->name);
		  			}
		  	    ::mysql_free_result( res );
		  	    return auto_ptr<std::vector<KnownGene*> >(genes);
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
					cout << line
					      << tokenizer.delim << "knownGene.name"
					      << tokenizer.delim << "knownGene.chrom"
					      << tokenizer.delim << "knownGene.strand"
					      << tokenizer.delim << "knownGene.exon"
					      << tokenizer.delim << "domain.start"
					      << tokenizer.delim << "domain.end"
					      << endl;
					continue;
					}
				tokenizer.split(line,tokens);
				if(this->pepNameCol>=(int)tokens.size())
					{
					cerr << "PEPTIDE NAME: Column out of range in " << line << endl;
					continue;
					}
				if(this->pepStartCol>=(int)tokens.size())
					{
					cerr << "PEPTIDE START: Column out of range in " << line << endl;
					continue;
					}
				if(this->pepEndCol>=(int)tokens.size())
				    {
				    cerr << "PEPTIDE End: Column out of range in " << line << endl;
				    continue;
				    }

				int32_t pepStart1=0;
				int32_t pepEnd1=0;
				if(!numeric_cast<int32_t>(tokens[pepStartCol].c_str(),&pepStart1) || pepStart1<0)
				    {
				    cerr << "Bad peptide START position in "<< line << endl;
				    continue;
				    }
				if(!numeric_cast<int32_t>(tokens[pepEndCol].c_str(),&pepEnd1) || pepEnd1<0)
				    {
				    cerr << "Bad peptide END position in "<< line << endl;
				    continue;
				    }
				if(pepStart1>pepEnd1)
				    {
				    cerr << "Bad peptide START>END in "<< line << endl;
				    continue;
				    }


				std::string pepName(tokens[pepNameCol]);
				if(pepName.empty())
				    {
				    cerr << "Bad line. No protein name in "+line << endl;
				    continue;
				    }


				std::auto_ptr<std::vector<KnownGene*> > genes=getGenes(pepName.c_str());
				if(genes->empty())
				    {
				    cout << "#no knownGene found for "<< pepName << endl;
				    continue;
				    }
				for(size_t i=0;i< genes->size();++i)
				    {
				    KnownGene* gene=genes->at(i);
				    backLocate(line,gene, pepName, pepStart1-1,pepEnd1-1);
				    delete gene;
				    }
				}
		  	  }



		  void usage(ostream& out,int argc,char **argv)
			{
			out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
			out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			out << "Maps a proteic region back to the genome."<< endl;
			out << "Options:\n";
			out << "  -p (column) protein name column default:"<< (pepNameCol+1)<< "\n";
			out << "  -s (column) protein start column (1-based) default:"<< (pepStartCol+1)<< "\n";
			out << "  -e (column) protein end column (1-based)  default:"<< (pepEndCol+1)<< "\n";
			out << "Other options:\n";
			out << "  -d delimiter. Default:tab\n";
			out << "(stdin|tsv|tsv.gz)\n";
			MysqlApplication::printConnectOptions(out);
			}


		  int main(int argc,char** argv)
			{
			int n_optind=0;
			int optind=1;
			while(optind < argc)
			    {
			    if(strcmp(argv[optind],"-h")==0)
				{
				usage(cerr,argc,argv);
				return EXIT_SUCCESS;
				}
			    else if(strcmp(argv[optind],"-p")==0)
				{
				if(!numeric_cast(argv[++optind],&pepNameCol) || pepNameCol<1)
				    {
				    cerr << "Bad value for option -p :" << argv[optind] << endl;
				    usage(cerr,argc,argv);
				    return (EXIT_FAILURE);
				    }
				pepNameCol--;
				}
			    else if(strcmp(argv[optind],"-s")==0 && optind+1< argc)
				{
				if(!numeric_cast(argv[++optind],&pepStartCol) || pepStartCol<1)
				    {
				    cerr << "Bad value for option -s :" << argv[optind] << endl;
				    usage(cerr,argc,argv);
				    return (EXIT_FAILURE);
				    }
				pepStartCol--;
				}
			    else if(strcmp(argv[optind],"-e")==0 && optind+1< argc)
				{
				if(!numeric_cast(argv[++optind],&pepEndCol) || pepEndCol<1)
				    {
				    cerr << "Bad value for option -e :" << argv[optind] << endl;
				    usage(cerr,argc,argv);
				    return (EXIT_FAILURE);
				    }
				pepEndCol--;
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
				    return EXIT_FAILURE;
				    }
			    else
				    {
				    break;
				    }
			    ++optind;
			    }

		   this->connect();

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
	ProteinToGenome app;
	return app.main(argc,argv);
	}
