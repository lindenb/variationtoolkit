#include <cstring>
#include <list>
#include "zstreambuf.h"
#include "tokenizer.h"
#include "fastareader.h"
#include "numeric_cast.h"

using namespace std;

class FastaTail
	{
	public:
		std::list<FastaSequence*> sequences;
		std::size_t count_sequences;
		std::size_t limit;
		FastaReader reader;
		
		FastaTail():count_sequences(0),limit(10)
			{
			}
		~FastaTail()
			{
			
			}
			
		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2011.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage: "<<argv[0]<<" (options) {stdin|*.fa|*.fa.gz}:\n";
			out << "  -n <int> number of sequences. default: " << limit << "\n";
			out << "  --reserve <int> reserve n-bytes in memory for reading the sequence (optional).\n";
			out << endl;
			}
	
		void run(std::istream& in)
			{
			auto_ptr<FastaSequence> ret;
			for(;;)
				{
				ret=reader.next(in);
				if(ret.get()==0) break;
				sequences.push_back( ret.release() );
				count_sequences++;
				if(count_sequences>limit)
					{
					delete sequences.front();
					sequences.pop_front();
					--count_sequences;
					}
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
			    else if(strcmp(argv[optind],"-n")==0 && optind+1<argc)
				{
				if(!numeric_cast<size_t>(argv[++optind],&limit))
					{
					cerr << "Bad number: " << argv[optind] << endl; 
					usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				}
			   else if(strcmp(argv[optind],"--reserve")==0 && optind+1<argc)
				{
				int32_t reserve=0;
				if(!numeric_cast<int32_t>(argv[++optind],&reserve))
					{
					cerr << "Bad number: " << argv[optind] << endl; 
					usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				reader.reserve(reserve);
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
			for(list<FastaSequence*>::iterator r=sequences.begin();
				r!=sequences.end();
				++r)
				{
				(*r)->printFasta(cout);
				delete *r;
				}
			return EXIT_SUCCESS;
			}
			
	};

int main(int argc,char** argv)
	{
	FastaTail app;
	return app.main(argc,argv);
	}
