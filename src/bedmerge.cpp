/*
 * bedmerge.cpp
 *
 *  Created on: Dec 5, 2011
 *      Author: lindenb
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <iostream>
#include "zstreambuf.h"
#include "application.h"
using namespace std;

class BedMerge:public AbstractApplication
    {
    public:
	    int32_t chrom_col;
	    int32_t start_col;
	    int32_t end_col;
	    string prev_chrom;
	    vector<bool> mapped;
	    BedMerge():chrom_col(0),start_col(1),end_col(2)
		{
		}
	    virtual ~BedMerge()
		{

		}

	    void dump()
		{
		size_t i=0;
		while(i< mapped.size())
		    {
		    if(!mapped[i])
			{
			++i;
			continue;
			}
		    size_t j=i;
		    while(mapped[j] && j < mapped.size())
			{
			++j;
			}
		    cout << prev_chrom
			<< tokenizer.delim
			<< i
			<< tokenizer.delim
			<< j
			<< endl;
		    i=j;
		    }
		prev_chrom.assign("");
		mapped.clear();
		}

	    void run(std::istream& in)
		{
		string line;
		vector<string> tokens;

		while(getline(in,line,'\n'))
		    {
		    if(!line.empty() && line[0]=='#')
			{
			cout << line << endl;
			continue;
			}
		    tokenizer.split(line,tokens);
		    if(chrom_col>=(int32_t)tokens.size())
			{
			cerr << "column out of bound for CHROM in "<< line;
			continue;
			}
		    if(chrom_col>=(int32_t)tokens.size())
			{
			cerr << "column out of bound for CHROM in "<< line;
			continue;
			}
		    if(start_col>=(int32_t)tokens.size())
			{
			cerr << "column out of bound for CHROMSTART in "<< line;
			continue;
			}
		    if(end_col>=(int32_t)tokens.size())
			{
			cerr << "column out of bound for CHROMEND in "<< line;
			continue;
			}
		    if(prev_chrom.compare(tokens[chrom_col])!=0 && !mapped.empty())
			{
			dump();
			}
		    prev_chrom.assign(tokens[chrom_col]);
		     int32_t chromStart=(int32_t)atoi(tokens[start_col].c_str());
		     int32_t chromEnd=(int32_t)atoi(tokens[end_col].c_str());
		     if((int32_t)mapped.size()<=chromEnd)
			 {

			 mapped.resize(chromEnd,false);
			 }
		     while(chromStart< chromEnd)
			 {
			 mapped[chromStart++]=true;
			 }
		     }
		dump();
		}
	virtual void usage(ostream& out,int argc,char** argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    out << "Options:\n";
		    out << "  -c <chrom col> (default:"<< (1+chrom_col) <<")" << endl;
		    out << "  -s <chromStart col> (default:"<< (1+start_col) <<")" << endl;
		    out << "  -e <chromEnd col> (default:"<< (1+end_col) <<")" << endl;
		    out << "  -d delimiter, default:tab" << endl;
		    }


	#define ARGVCOL(flag,var) else if(std::strcmp(argv[optind],flag)==0 && optind+1<argc)\
		{\
		char* p2;\
		this->var=(int)strtol(argv[++optind],&p2,10);\
		if(this->var<1 || *p2!=0)\
			{cerr << "Bad column for "<< flag << ".\n";this->usage(cerr,argc,argv);return EXIT_FAILURE;}\
		this->var--;\
		}


	int main(int argc,char** argv)
	    {
	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
			{
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		ARGVCOL("-c",chrom_col)
		ARGVCOL("-s",start_col)
		ARGVCOL("-e",end_col)
		else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
				{
				cerr<< "bad delimiter \"" << p << "\"\n";
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
			this->usage(cerr,argc,argv);
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
	    else if(optind+1==argc)
		{
		igzstreambuf buf(argv[optind++]);
		istream in(&buf);
		this->run(in);
		buf.close();
		++optind;
		}
	    else
		{
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    BedMerge app;
    return app.main(argc,argv);
    }

