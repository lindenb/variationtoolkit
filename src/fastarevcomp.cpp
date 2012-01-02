#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <algorithm>

#include <cstdio>
#include "zstreambuf.h"
#include "throw.h"
#include "fastareader.h"
#include "numeric_cast.h"
#include "where.h"
using namespace std;

class FastaRevComp
	{
        private:
		FastaReader fastareader;
		bool print_original;
		bool do_reverse;
		bool do_complement;
        public:
		FastaRevComp():print_original(false),do_reverse(true),do_complement(true)
			{
			}
		
		char complement(char c) const
		    {
		    switch(c)
			{
			case 'A':return 'T';break;
			case 'T':return 'A';break;
			case 'U':return 'A';break;
			case 'G':return 'C';break;
			case 'C':return 'G';break;
			case 'Y':return 'R';break;
			case 'R':return 'Y';break;
			case 'S':return 'S';break;
			case 'W':return 'W';break;
			case 'K':return 'M';break;
			case 'M':return 'K';break;
			case 'B':return 'V';break;
			case 'D':return 'H';break;
			case 'H':return 'D';break;
			case 'V':return 'B';break;
			case 'N':return 'N';break;

			case 'a':return 't';break;
			case 't':return 'a';break;
			case 'u':return 'a';break;
			case 'g':return 'c';break;
			case 'c':return 'g';break;
			case 'y':return 'r';break;
			case 'r':return 'y';break;
			case 's':return 's';break;
			case 'w':return 'w';break;
			case 'k':return 'm';break;
			case 'm':return 'k';break;
			case 'b':return 'v';break;
			case 'd':return 'h';break;
			case 'h':return 'd';break;
			case 'v':return 'b';break;
			case 'n':return 'n';break;

			default: break;
			}
		    cerr << "Illegal base: "<< c << endl;
		    return c;
		    }


		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2012.\n";
			out << "Reverse-Complement FASTA sequences.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage:\n";
			out << "\t" << argv[0]<< " [options] (fasta|fasta.gz|stdin)\n";
			out << "Options:\n";
			out << " -p print original sequence.\n";
			out << " -c DISABLE complement.\n";
			out << " -r DISABLE reverse.\n";
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
			    if(print_original || (!do_complement && !do_reverse))
				{
				seq->printFasta(cout);
				}

			    if(do_complement && !do_reverse)
				{
				cout << ">"<< seq->name() << "|complement";
				for(int32_t i=0;i< seq->size();++i)
				    {
				    if((i%FastaSequence::DEFAULT_LINE_LENGTH)==0) cout << endl;
				    cout << complement(seq->at(i));
				    }
				cout << endl;
				}
			    else if(!do_complement && do_reverse)
				{
				cout << ">"<< seq->name() << "|reverse";
				for(int32_t i=0;i< seq->size();++i)
				    {
				    if((i%FastaSequence::DEFAULT_LINE_LENGTH)==0) cout << endl;
				    cout << (seq->at((seq->size()-1)-i));
				    }
				cout << endl;
				}
			    else if(do_complement && do_reverse)
				{
				cout << ">"<< seq->name() << "|reverse-complement";
				for(int32_t i=0;i< seq->size();++i)
				    {
				    if((i%FastaSequence::DEFAULT_LINE_LENGTH)==0) cout << endl;
				    cout << complement(seq->at((seq->size()-1)-i));
				    }
				cout << endl;
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
			    else if(strcmp(argv[optind],"-p")==0 )
				{
				print_original=true;
				}
			    else if(strcmp(argv[optind],"-c")==0 )
				{
				do_complement=false;
				}
			    else if(strcmp(argv[optind],"-r")==0 )
				{
				do_reverse=false;
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
			return EXIT_SUCCESS;
			}
			
	};

int main(int argc,char** argv)
	{
	FastaRevComp app;
	return app.main(argc,argv);
	}
