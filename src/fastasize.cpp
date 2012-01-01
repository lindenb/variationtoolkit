#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <cstdio>
#include "zstreambuf.h"
#include "throw.h"
#include "numeric_cast.h"
#include "fastareader.h"

using namespace std;

class FastaSize
	{
	private:
		int32_t* min_size;
		int32_t* max_size;
		FastaReader fastareader;
		bool print_size;
		bool inverse;
	public:
		FastaSize():min_size(0),max_size(0),print_size(false),inverse(false)
			{
			}
		
			
		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2011.\n";
			out << "Filters FASTA sequence on their sizes.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage:\n";
			out << "\t" << argv[0]<< " [options] (fasta|fasta.gz|stdin)\n";
			out << "Options:\n";
			out << "  -m <min-size>  default: none\n";
			out << "  -M <max-size>  default: none\n";
			out << "  -p append sequence size to FASTA header.\n";
			out << "  -v inverse.\n";
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
			    bool accept=true;

			    if(min_size!=NULL && seq->size()< (*min_size))
				{
				accept=false;
				}
			    if(max_size!=NULL && seq->size()> (*max_size))
				{
				accept=false;
				}
			    if(accept==inverse) continue;
			    cout << ">" << seq->name();
			    if(print_size) cout << "|length="<< seq->size();
			    for(int32_t i=0;i< seq->size();++i)
				    {
				    if(i%FastaSequence::DEFAULT_LINE_LENGTH==0) cout << std::endl;
				    cout << seq->at(i);
				    }
			    cout << std::endl;
			    }
			}
			
		int main(int argc,char** argv)
			{
			int optind=1;
			int32_t min_len=0;
			int32_t max_len=0;
			while(optind < argc)
			    {
			    if(strcmp(argv[optind],"-h")==0)
				    {
				    usage(cout,argc,argv);
				    return EXIT_SUCCESS;
				    }
			    else if(strcmp(argv[optind],"-p")==0)
				{
				this->print_size=true;
				}
			    else if(strcmp(argv[optind],"-v")==0)
				{
				this->inverse=true;
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
			    else if(strcmp(argv[optind],"-m")==0 && optind+1<argc)
				{
				if(!numeric_cast<int32_t>(argv[++optind],&min_len))
				    {
				    cerr << "Bad value for -m "<< argv[optind]<< endl;
				    usage(cerr,argc,argv);
				    return EXIT_FAILURE;
				    }
				this->min_size=&min_len;
				}
			    else if(strcmp(argv[optind],"-M")==0 && optind+1<argc)
				{
				if(!numeric_cast<int32_t>(argv[++optind],&max_len))
				    {
				    cerr << "Bad value for -M "<< argv[optind]<< endl;
				    usage(cerr,argc,argv);
				    return EXIT_FAILURE;
				    }
				this->max_size=&max_len;
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
			return EXIT_SUCCESS;
			}
			
	};

int main(int argc,char** argv)
	{
	FastaSize app;
	return app.main(argc,argv);
	}
