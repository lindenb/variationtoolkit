/*
 * vcf2snp
 *
 *  Created on: Feb 22, 2012
 *      Author: lindenb
 *  Motivation:
 *  	join the output of snpEff with VCF
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
#include <algorithm>


#define NOWHERE
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "bin.h"
#include "where.h"
#include "numeric_cast.h"

using namespace std;

static char myupper(char c)
    {
    return (char)::toupper(c);
    }

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
		     if(line.size()>1 && line[1]=='#') continue;
		     cout << "# Chromo\tPosition\tReference\tChange\t" << line << endl;
		     continue;
		     }
		 tokenizer.split(line,tokens);
		 string chrom=tokens[0];
		 int pos0;
		 numeric_cast<int>(tokens[1].c_str(),&pos0);

		 vector<string> alts;
		 Tokenizer comma(',');
		 comma.split(tokens[4],alts);
		for(size_t i=0;i< alts.size();++i)
		    {
		    string ref=tokens[3];
		    string alt(alts[i]);
		    int pos=pos0;

		    transform(ref.begin(),ref.end(),ref.begin(),myupper);
		    transform(alt.begin(),alt.end(),alt.begin(),myupper);

		    while(!alt.empty() &&
			  !ref.empty() &&
			  alt[alt.size()-1]==ref[ref.size()-1]
			 )
			{
			alt.erase(alt.size()-1,1);
			ref.erase(ref.size()-1,1);
			}


		    while(!alt.empty() &&
			  !ref.empty() &&
			  alt[0]==ref[0]
			 )
			{
			alt.erase(0,1);
			ref.erase(0,1);
			++pos;
			}



		    if(ref.size()<alt.size()) /* A/AC = INSERTION */
			 {
			 size_t diff=ref.size();
			 alt.erase(0,ref.size());
			 ref.assign("*");
			 alt.insert(0,1,'+');
			 pos+=diff;
			 }
		     else if(ref.size()>alt.size()) //deletion
			 {
			 size_t diff=alt.size();
			 alt.assign(ref);
			 alt.erase(0,diff);
			 alt.insert(0,1,'-');
			 ref.assign("*");
			 pos+=diff;
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
