/*
 * prediction.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
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
#include <memory>
#include <stdint.h>

#include "geneticcode.h"
#include "zstreambuf.h"
#include "xfaidx.h"
#include "tokenizer.h"
#include "knowngene.h"
#include "application.h"
#define NOWHERE
#include "where.h"

using namespace std;

class VcfToBed:public AbstractApplication
    {
    public:
	int chromColumn;
	int posColumn;
	int scoreColumn;
	set<int> name_columns;
	char delim_name;
	VcfToBed():
		chromColumn(0),
		posColumn(1),
		scoreColumn(-1),
		delim_name('|')
	    {
	    }
	virtual ~VcfToBed()
	    {
	    }

#define CHECKCOL(a) if(a>=(int)tokens.size()){\
		cerr << "Column "<< #a << " out of range in "<< line << endl;\
		continue;}



	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;

	    while(getline(in,line,'\n'))
		    {
		    if(AbstractApplication::stopping()) break;
		    if(line.empty() || line[0]=='#') continue;
		    tokenizer.split(line,tokens);
		    CHECKCOL(chromColumn);
		    CHECKCOL(posColumn);
		    char* p2;
		    int pos=(int)strtol(tokens[posColumn].c_str(),&p2,10);
		    if(pos <0 || *p2!=0)
			    {
			    cerr << "Bad POS "<< tokens[posColumn] << " in "<<line << endl;
			    continue;
			    }
		    cout << tokens[chromColumn] << "\t"
			     << (pos-1) << "\t"
			     << pos  << "\t"
			     ;
		    for(set<int>::iterator r=name_columns.begin();r!=name_columns.end();++r)
			    {
			    if(r!=name_columns.begin()) cout << delim_name;
			    if((*r)>=(int)tokens.size()) continue;
			    cout << tokens[(*r)];
			    }
		    cout << "\t";
		    if(scoreColumn!=-1 && scoreColumn< (int)tokens.size())
			    {
			    cout << tokens[scoreColumn];
			    }
		    else
			    {
			    cout << "0";
			    }
		    cout << "\t";
		    cout << "+";
		    cout << endl;
		    }
	    }


	virtual void usage(ostream& out,int argc,char** argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  -d <column-delimiter> (default:tab)" << endl;

		out << "  -c <CHROM col> (default:"<< chromColumn <<")" << endl;
		out << "  -p <POS col> (default:"<< posColumn <<")" << endl;
		out << "  -S <bed score col> (default:"<< scoreColumn <<")" << endl;
		out << "  -N <col> adds this column for the 'name'" << endl;
		out << "  -D <char> name separator." << endl;
		out << "  -t print ucsc custom track header." << endl;
		}
    };

#define ARGVCOL(flag,var) else if(std::strcmp(argv[optind],flag)==0 && optind+1<argc)\
	{\
	char* p2;\
	app.var=(int)strtol(argv[++optind],&p2,10);\
	if(app.var<1 || *p2!=0)\
		{cerr << "Bad column for "<< flag << ".\n";app.usage(cerr,argc,argv);return EXIT_FAILURE;}\
	app.var--;\
	}


int main(int argc,char** argv)
    {
	VcfToBed app;
	bool custom_track_header=false;
    int optind=1;
    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
			{
			app.usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		ARGVCOL("-c",chromColumn)
		ARGVCOL("-p",posColumn)
		ARGVCOL("-S",scoreColumn)
		else if(std::strcmp(argv[optind],"-N")==0 && optind+1<argc)
			{
			Tokenizer t;
			t.delim=',';
			vector<string> comma;
			t.split(argv[++optind],comma);
			for(size_t i=0;i< comma.size();++i)
				{
				if(comma[i].empty()) continue;
				char* p2;
				int x=(int)strtol(comma[i].c_str(),&p2,10);
				if(x<1 || *p2!=0)
					{cerr << "Bad column for -N.\n";app.usage(cerr,argc,argv);return EXIT_FAILURE;}
				app.name_columns.insert(x-1);
				}
			}

		else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
				{
				cerr<< "bad delimiter \"" << p << "\"\n";
				return (EXIT_FAILURE);
				}
			app.tokenizer.delim=p[0];
			}
		else if(strcmp(argv[optind],"-D")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
				{
				cerr<< "bad option -D \"" << p << "\"";
				return (EXIT_FAILURE);
				}
			app.delim_name=p[0];
			}
		else if(strcmp(argv[optind],"-t")==0 )
			{
			custom_track_header=true;
			}
		else if(strcmp(argv[optind],"--")==0)
			{
			++optind;
			break;
			}
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '" << argv[optind]<< "'" << endl;
			app.usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		else
			{
			break;
			}
		++optind;
		}


    if(custom_track_header)
    	{
    	cout << "track name=\"__TRACK_NAME__\" description=\"__TRACK_DESC__\" " << endl;
    	}


    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		app.run(in);
		}
    else
		{
		while(optind< argc)
			{
			if(AbstractApplication::stopping()) break;
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			app.run(in);
			buf.close();
			++optind;
			}
		}
    return EXIT_SUCCESS;
    }
