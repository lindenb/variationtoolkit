#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdio>
#include "zstreambuf.h"
#include "throw.h"
#include "fastareader.h"
#include "numeric_cast.h"
#include "where.h"
using namespace std;

class FastaTac
	{
        private:
		FastaReader fastareader;
		vector<FastaSequence*> sequences;
        public:
		FastaTac()
			{
			}
		
			
		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2012.\n";
			out << "Reverse order of FASTA sequences.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage:\n";
			out << "\t" << argv[0]<< " [options] (fasta|fasta.gz|stdin)\n";
			out << "Options:\n";
			//out << "Other options:\n";
			out << "  --reserve <buffer-size> . Reserve size for the fasta buffer (optional)\n";
			out << endl;
			}
	
		void run(std::istream& in)
			{
			for(;;)
			    {
			    auto_ptr<FastaSequence> seq=fastareader.next(in);
			    if(seq.get()==0) break;
			    sequences.push_back(seq.release());
			    }
			}
		void dump()
		    {
		    for(vector<FastaSequence*>::reverse_iterator r= sequences.rbegin();r!=sequences.rend();++r)
			{
			(*r)->printFasta(cout);
			delete *r;
			}
		    }
		int main(int argc,char** argv)
			{
			int optind=1;
			while(optind < argc)
			    {
			    if(strcmp(argv[optind],"-h")==0)
				    {
				    usage(cout,argc,argv);
				    return EXIT_SUCCESS;
				    }
			    else if(strcmp(argv[optind],"--reserve")==0 && optind+1<argc)
				{
				int32_t n=0;
				if(!numeric_cast<int32_t>(argv[++optind],&n))
				    {
				    cerr << "Bad value for -n "<< argv[optind]<< endl;
				    usage(cerr,argc,argv);
				    return EXIT_FAILURE;
				    }
				this->fastareader.reserve(n);
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
				return (EXIT_FAILURE);
				}
			    else
				{
				break;
				}
			    ++optind;
			    }


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
					run(in);
					buf.close();
					++optind;
					}
				}
			dump();
			return EXIT_SUCCESS;
			}
			
	};

int main(int argc,char** argv)
	{
	FastaTac app;
	return app.main(argc,argv);
	}
