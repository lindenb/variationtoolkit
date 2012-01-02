#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <algorithm>
#include <set>
#include <cstdio>
#include "zstreambuf.h"
#include "throw.h"
#include "fastareader.h"
#include "numeric_cast.h"
#include "where.h"
using namespace std;

class FastaSortUniq
	{
        private:
		FastaReader fastareader;
        public:
		bool use_title;
		bool ignore_case;
		bool uniq;
		struct CompareSeq
		    {
		    FastaSortUniq* owner;
		    CompareSeq(FastaSortUniq* owner):owner(owner)
			{
			}
		    bool operator() (const FastaSequence* seq1, const FastaSequence* seq2) const
		      {
		      int i=0;
		      if(owner->ignore_case)
			  {
			  if(owner->use_title)
			      {
			      i= strcasecmp(seq1->name(),seq2->name());
			      }
			  else
			      {
			      i= strcasecmp(seq1->c_str(),seq2->c_str());
			      }
			  }
		      else
			  {
			  if(owner->use_title)
			      {
			      i= strcmp(seq1->name(),seq2->name());
			      }
			  else
			      {
			      i= strcmp(seq1->c_str(),seq2->c_str());
			      }
			  }
		      if(i!=0) return i<0;
		      if(!(owner->uniq))
			  {
			  return seq1 < seq2;//just compare ptr address
			  }
		      return false;
		      }
		    };
		CompareSeq comparator;
		set<FastaSequence*,CompareSeq> sequences;

		FastaSortUniq():use_title(false),ignore_case(false),uniq(false),comparator(this),sequences(comparator)
			{
			}
		
			
		void usage(std::ostream& out,int argc,char** argv)
			{
			out << endl;
			out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2012.\n";
			out << "Sort/Uniq on FASTA sequences.\n";
			out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
			out << VARKIT_REVISION << endl;
			out << "Usage:\n";
			out << "\t" << argv[0]<< " [options] (fasta|fasta.gz|stdin)\n";
			out << "Options:\n";
			out << "  -u uniq.\n";
			out << "  -i ignore case.\n";
			out << "  -t sort on title rather than sequence.\n";
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
			    sequences.insert(seq.release());
			    }
			}
		void dump()
		    {
		    for(set<FastaSequence*,CompareSeq>::iterator r= sequences.begin();r!=sequences.end();++r)
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
			    else if(strcmp(argv[optind],"-u")==0)
				{
				uniq=true;
				}
			    else if(strcmp(argv[optind],"-i")==0)
				{
				ignore_case=true;
				}
			    else if(strcmp(argv[optind],"-t")==0)
				{
				use_title=true;
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
	FastaSortUniq app;
	return app.main(argc,argv);
	}
