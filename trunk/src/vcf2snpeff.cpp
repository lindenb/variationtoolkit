/*
 * md5cols.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: lindenb
 */
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define NOWHERE
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "bin.h"
#include "where.h"
#include "numeric_cast.h"

using namespace std;

class Vcf2SnpEff
    {
    public:
	Tokenizer tokenizer;
	vector<int> columns;
	static void print_as_hex (const unsigned char *digest, int len)
	    {
	    for(int i = 0; i < len; i++)
		{
		fprintf(stdout,"%02x", digest[i]);
		}
	    }

	 void run(std::istream& in)
	     {
	     string line;
	     vector<int> binList;
	     vector<string> tokens;
	     ostringstream os;
	     while(getline(in,line,'\n'))
		 {
		 if(line.empty()) continue;
		 if(line[0]=='#')
		     {
		     cout << "#Chromo\tPosition\tReference\tChange\t" << line << endl;
		     continue;
		     }
		 tokenizer.split(line,tokens);
		 string chrom=tokens[0];
		 if(chrom.compare(0,3,"chr")==0) chrom=chrom.substr(3);
		 int pos;
		 numeric_cast<int>(tokens[1].c_str(),&pos);
		 string ref=tokens[3];
		 string alt=tokens[4];
		 if(ref.size()<alt.size()) /* AC/A = DELETION */
		     {
		     assert(ref[0]==alt[0]);
		     ref.assign("*");
		     alt[0]='+';
		     ++pos;
		     }
		 else if(ref.size()>alt.size())
		     {
		     assert(ref[0]==alt[0]);
		     alt.assign(ref);
		     alt[0]='-';
		     ref.assign("*");
		     ++pos;
		     }
		 else
		     {
		     //single SNP
		     }
		 cout 	<< chrom
			<< "\t"<< pos
			<< "\t" << ref
			<< "\t" << alt
			<< "\t" << line
			<< endl;
		 }
	     }

	int main(int argc,char** argv)
	    {
	    int optind=1;
	    while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			    {
			    cerr << argv[0] << "Pierre Lindenbaum PHD. 2012.\n";
			    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			    cerr << "Usage:\n\t"<< argv[0]<< " [options] (files|stdin)*\n\n";
			    cerr << "Options:\n";
			    cerr << "(stdin|files)\n\n";
			    exit(EXIT_FAILURE);
			    }
		    else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			int c;
			if(!numeric_cast(argv[++optind],&c) || c<1)
			    {
			    cerr << "Bad column " << argv[optind]<< endl;
			    return EXIT_FAILURE;
			    }
			this->columns.push_back(c-1);
			}
		    else if(argv[optind][0]=='-')
			{
			cerr<<"unknown option '"<<argv[optind]<<"'"<< endl;
			exit(EXIT_FAILURE);
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
		    this->run(in);
		    }
	    else
		    {
		    while(optind< argc)
				{
				char* filename=argv[optind++];
				WHERE(filename);
				igzstreambuf buf(filename);
				istream in(&buf);
				this->run(in);
				buf.close();
				}
		    }
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    Vcf2SnpEff app;
    return app.main(argc,argv);
    }
