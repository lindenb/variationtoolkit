#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <cstdio>
#include "zstreambuf.h"
#include "throw.h"

using namespace std;

class FastaToTsv
	{
	public:
		bool to_upper;
		std::string delim;
		FastaToTsv():to_upper(false),delim("\t")
			{
			}
		
			
		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2011.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage: "<<argv[0]<<" (options) {stdin|*.fa|*.fa.gz}:\n";
			out << "  -d <char> delimiter. default: tab\n";
			out << "  -u convert sequence to upper case.\n";
			out << endl;
			}
	
		void run(std::istream& in)
			{
			bool found(false);
			int c;
			while((c=in.get())!=EOF)
				{
				if(c=='>')
					{
					if(found) cout << endl;
					found=true;
					while((c=in.get())!=EOF && c!='\n')
						{
						if(c=='\r') continue;
						cout << (char)c;
						}
					cout << delim;
					continue;
					}
				if(isspace(c) || isdigit(c)) continue;
				cout << (char)(to_upper?toupper(c):c);
				}
			if(found) cout << endl;
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
			    else if(strcmp(argv[optind],"-u")==0)
				{
				to_upper=true;
				}
			   else if(strcmp(argv[optind],"-d")==0 && optind+1<argc)
				{
				delim.assign(argv[++optind]);
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
	FastaToTsv app;
	return app.main(argc,argv);
	}
