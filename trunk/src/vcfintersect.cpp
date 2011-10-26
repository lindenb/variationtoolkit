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



class VcfIntersection:public AbstractApplication
    {
    public:
	int chromColumn;
	int posColumn;
	bool inverse;

	class Record
		{
		public:
			ChromStartEnd pos;
			vector<string> tokens;
		};


	VcfIntersection():
		chromColumn(0),
		posColumn(1),
		inverse(false)
	    {
	    }
	virtual ~VcfIntersection()
	    {
	    }

#define CHECKCOL(a) if(a>=(int)tokens.size()){\
		cerr << "Column "<< #a << " out of range in "<< line << endl;\
		continue;}

	Record* parseRecord1(std::istream& in)
		{
		vector<string> tokens;
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
		}

	Record* parseRecord2(std::istream& in)
		{
		return NULL;
		}

	void echo(Record* rec1,Record* rec2)
		{

		}

	void run(std::istream& in,std::istream& in2)
	    {
		vector<Record*> buffer;
	    string line;
	    Record* rec=NULL;
	    while((rec=parseRecord1(line))!=NULL)
		    {
		    if(AbstractApplication::stopping()) break;

		    bool foundMatch=false;
		    bool refill=true;
		    size_t i=0;
		    while(i<buffer.size())
		    	{
		    	Record* rec2=buffer.at(i);
		    	int d=rec2->pos.chrom.compare(rec->pos.chrom);
		    	if(d<0)
		    		{
		    		delete rec2;
		    		buffer.erase(buffer.begin()+i);
		    		continue;
		    		}
		    	else if(d>0)
		    		{
		    		refill=false;
		    		break;
		    		}
		    	if(rec2->pos.end<=rec->pos.start)
		    		{
		    		delete rec2;
		    		buffer.erase(buffer.begin()+i);
		    		continue;
		    		}
		    	else if(rec2->pos.start>=rec->pos.end)
		    		{
		    		refill=false;
		    		break;
		    		}
		    	echo(rec,rec2);
		    	foundMatch=true;
		    	++i;
		    	}

		    if(refill)
		    	{
		    	Record* rec2;
		    	while((rec2=parseRecord1(line))!=NULL)
		    		{
		    		int d=rec2->pos.chrom.compare(rec->pos.chrom);
					if(d<0)
						{
						delete rec2;
						continue;
						}
					else if(d>0)
						{
						buffer.push_back(rec2);
						break;
						}
					if(rec2->pos.end<=rec->pos.start)
						{
						delete rec2;
						continue;
						}
					else if(rec2->pos.start>=rec->pos.end)
						{
						buffer.push_back(rec2);
						break;
						}
					echo(rec,rec2);
					foundMatch=true;
		    		}
		    	}

		    delete rec;
		    }
	    }


	virtual void usage(ostream& out,int argc,char** argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  -e <ranges> (chr:start-end)*" << endl;
		out << "  -c <CHROM col> (default:"<< chromColumn <<")" << endl;
		out << "  -p <POS col> (default:"<< posColumn <<")" << endl;
		out << "  -d delimiter, default:tab" << endl;
		out << "  -v inverse" << endl;
		}

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
		char* dbName=NULL;
		VcfIntersection app;
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

		if(dbName==NULL)
			{
			cerr << "Undefined database name.\n";
			this->usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}


		igzstreambuf buf2(argv[optind++]);
		istream in2(&buf2);
		if(optind==argc)
			{
			igzstreambuf buf;
			istream in(&buf);
			this->run(in,in2);
			}
		else if(optind+1==argc)
			{
			if(AbstractApplication::stopping()) break;
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			this->run(in,in2);
			buf.close();
			++optind;
			}
		else
			{
			cerr << "Illegal number of arguments.\n";
			return EXIT_FAILURE;
			}
		buf2.close();
		return EXIT_SUCCESS;
		}
    };


int main(int argc,char** argv)
	{
	VcfIntersection app;
	return app.main(argc,argv);
	}
