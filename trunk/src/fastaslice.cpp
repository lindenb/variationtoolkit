#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include "zstreambuf.h"
#include "throw.h"
#include "numeric_cast.h"
#include "fastareader.h"

using namespace std;

class FastaSlice
	{
	private:
		int32_t* every;
		int32_t* slice_size;
		FastaReader fastareader;
	public:
		FastaSlice():every(0),slice_size(0)
			{
			}
		
			
		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2012.\n";
			out << "Slice FASTA sequences.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage:\n";
			out << "\t" << argv[0]<< " [options] (fasta|fasta.gz|stdin)\n";
			out << "Options:\n";
			out << "  -e <every>  default:"<< every <<"\n";
			out << "  -L <fragment size>  default: (same as -e)\n";
			out << "Other options:\n";
			out << "  --reserve <buffer-size> . Reserve size for the fasta buffer (optional)\n";
			out << endl;
			}
	
		void run(std::istream& in)
			{
			for(;;)
			    {
			    auto_ptr<FastaSequence> seq=fastareader.next(in);
			    if(seq.get()==0) break;
			    int32_t start=0;
			    while(start< seq->size())
				{
				int32_t end=std::min(seq->size(),start+(*slice_size));
				cout << ">" << seq->name() << "|slice:"<<start<<"-"<<end ;
				for(int32_t i=start;i< end;++i)
				    {
				    if((i-start)%FastaSequence::DEFAULT_LINE_LENGTH==0) cout << endl;
				    cout << seq->at(i);
				    }
				cout << endl;
				start+=(*every);
				}
			    }
			}
			
		int main(int argc,char** argv)
			{
			int optind=1;
			int32_t param_every=0;
			int32_t param_length=0;
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
			    else if(strcmp(argv[optind],"-e")==0 && optind+1<argc)
				{
				if(!numeric_cast<int32_t>(argv[++optind],&param_every) || param_every<1)
				    {
				    cerr << "Bad value for -e : "<< argv[optind]<< endl;
				    usage(cerr,argc,argv);
				    return EXIT_FAILURE;
				    }
				this->every=&param_every;
				}
			    else if(strcmp(argv[optind],"-L")==0 && optind+1<argc)
				{
				if(!numeric_cast<int32_t>(argv[++optind],&param_length) || param_length<1)
				    {
				    cerr << "Bad value for -L "<< argv[optind]<< endl;
				    usage(cerr,argc,argv);
				    return EXIT_FAILURE;
				    }
				this->slice_size=&param_length;
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
			if(this->every==0)
			    {
			    cerr << "undefined param -e\n";
			    usage(cerr,argc,argv);
			    return (EXIT_FAILURE);
			    }

			if(this->slice_size==0)
			    {
			    param_length=*(this->every);
			    this->slice_size=&param_length;
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
			return EXIT_SUCCESS;
			}
			
	};

int main(int argc,char** argv)
	{
	FastaSlice app;
	return app.main(argc,argv);
	}
