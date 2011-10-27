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
//#define NOWHERE
#include "where.h"
#include "segments.h"

using namespace std;

extern std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s);

class VcfCut:public AbstractApplication
    {
    public:
	int chromColumn;
	int posColumn;
	map<string,vector<StartEnd> > ranges;
	bool inverse;
	VcfCut():
		chromColumn(0),
		posColumn(1),
		inverse(false)
	    {
	    }
	virtual ~VcfCut()
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
		    if(line.empty()) continue;
		    if(line[0]=='#')
			{
			cout << line << endl;
			continue;
			}

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
		    bool keep=ranges.empty();

		    map<string,vector<StartEnd> >::iterator r=ranges.find(tokens[chromColumn]);
		    if(r==ranges.end())
			{

			keep=false;
			}
		    else
			{
			keep=false;
			//QUICK'n dirty scan
			for(size_t i=0;i< r->second.size();++i)
			    {

			    if(pos < r->second[i].start || pos > r->second[i].end) continue;

			    keep=true;

			    break;
			    }
			}
		    if(keep==!inverse)
			{
			cout << line <<endl;
			}
		    }
	    }


	virtual void usage(ostream& out,int argc,char** argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  -e <ranges> (chr:start-end)*" << endl;
		out << "  -c <CHROM col> (default:"<< (1+chromColumn) <<")" << endl;
		out << "  -p <POS col> (default:"<< (1+posColumn) <<")" << endl;
		out << "  -d delimiter, default:tab" << endl;
		out << "  -v inverse" << endl;
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
    VcfCut app;
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

		else if(std::strcmp(argv[optind],"-e")==0 && optind+1<argc)
			{
			std::auto_ptr<std::vector<ChromStartEnd> > m=parseSegments(argv[++optind]);
			if(m.get()!=NULL)
			    {
			    for(size_t i=0;i< m->size();++i)
				{
				const ChromStartEnd& seg=m->at(i);
				app.ranges[seg.chrom].push_back(StartEnd(seg.start,seg.end));
				}
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
		else if(strcmp(argv[optind],"-v")==0 )
		    {
		    app.inverse=true;
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
