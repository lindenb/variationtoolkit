/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Oct 2011
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	 Extract info field
 * Compilation:
 *	 g++ -o extractsnpeff -Wall -O3 extractsnpeff.cpp -lz
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <stdint.h>
#include "zstreambuf.h"
#include "tokenizer.h"

using namespace std;

class Mpileup2Pileup
	{
	public:
		Tokenizer tokenizer;
		
		Mpileup2Pileup()
			{
			}
		
		void run(std::istream& in)
			{
			string line;
			vector<string> tokens;
			vector<string> samples;
			while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				tokenizer.split(line,tokens);

				if(line[0]=='#')
					{
					if(line.size()>1 && line[1]=='#')
					    {
					    cout << line << endl;
					    continue;
					    }
					if(line.compare(0,6,"#CHROM")!=0) THROW("Bad VCF header");

					if(tokens.size()<10)
					    {
					    THROW("Illegal number of columns in header "<< line);
					    }

					for(size_t i=0;i< 9;++i)
					    {
					    cout << tokens[i] << "\t";
					    }
					cout << "SAMPLE\n";
					 samples.clear();
					for(size_t i=9;i< tokens.size();++i)
					    {

					    samples.push_back(tokens[i]);
					    }

					continue;
					}

				if(samples.size()+9!=tokens.size())
					{
					THROW("Bad number of columns (" << tokens.size()<< ") / " << samples.size()<< " samples / in " << line);
					}

				for(size_t i=0;i< samples.size();++i)
				    {
				    for(size_t j=0;j< 9;++j)
					{
					cout << tokens[j] << "\t";
					}
				    cout << tokens[9+i]<< "\t" << samples[i] << endl;
				    }
				}

			}
			
    void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Options:\n";
	    cerr << "  --delim <column-delimiter> (default:tab)" << endl;
	    cerr << endl;
	    }

    int main(int argc,char** argv)
	    {

	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
			{
			usage(argc,argv);
			return(EXIT_FAILURE);
			}
		else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
			    {
			    char* p=argv[++optind];
			    if(strlen(p)!=1)
				{
				fprintf(stderr,"Bad delimiter \"%s\"\n",p);
				return (EXIT_FAILURE);
				}
			    this->tokenizer.delim=p[0];
			    }
		else if(strcmp(argv[optind],"--")==0)
			    {
			    ++optind;
			    break;
			    }
		 else if(argv[optind][0]=='-')
			    {
			    cerr << "unknown option '" << argv[optind]<< "'" << endl;
			    return EXIT_FAILURE;
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
    Mpileup2Pileup app;
	return app.main(argc,argv);
	}
