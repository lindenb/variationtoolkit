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


#include "zstreambuf.h"
#include "tokenizer.h"
#include "application.h"
//#define NOWHERE
#include "netstreambuf.h"
#include "where.h"
#include "segments.h"
#include "auto_vector.h"

using namespace std;

enum JoinType
    {
    PRINT_ALL=0,
    PRINT_MATCHING=1,
    PRINT_UMATCHING=2
    };


class VcfIntersection:public AbstractApplication
    {
    public:

	class Config
	    {
	    public:
		int chromColumn;
		int startColumn;
		int endColumn;
		char delim;
		bool halfOpen;
		bool zeroBased;
		Config():chromColumn(0),
			startColumn(1),
			endColumn(1),
			delim('\t'),
			halfOpen(true),
			zeroBased(true)
		    {
		    }
	    };

	JoinType joinType;
	Config config1;
	Config config2;
	class Record
		{
		public:
			ChromStartEnd pos;
			vector<string> tokens;

			Record(const string& chrom,int S,int E):pos(chrom,S,E)
			    {

			    }
		};


	VcfIntersection():
		joinType(PRINT_ALL)
	    {
	    config1.halfOpen=false;
	    config1.zeroBased=false;
	    }
	virtual ~VcfIntersection()
	    {
	    }


#define CHECKCOL(a)

	Record* parseRecord(
		const std::string& line,
		const vector<string>& tokens,
		const Config* cfg
		)
	    {
	    if(cfg->chromColumn >= (int)tokens.size())
		{
		cerr << "Column CHROM out of range in "<< line << endl;\
		return NULL;
		}

	    char* p2;
	    int chromStart=(int)strtol(tokens[cfg->startColumn].c_str(),&p2,10);
	    if(chromStart <(cfg->zeroBased?0:1) || *p2!=0)
		{
		cerr << "Bad START "<< tokens[cfg->startColumn] << " in "<<line << endl;
		return NULL;
		}
	    int chromEnd=chromStart;
	    if(cfg->startColumn!=cfg->endColumn)
		{
		chromEnd=(int)strtol(tokens[cfg->endColumn].c_str(),&p2,10);
		if(chromStart>chromEnd || chromEnd <(cfg->zeroBased?0:1) || *p2!=0)
		    {
		    cerr << "Bad END "<< tokens[cfg->endColumn] << " in "<<line << endl;
		    return NULL;
		    }
		}
	    if(!cfg->halfOpen)
		{
		chromEnd++;
		}
	    if(!cfg->zeroBased)
		{
		chromStart--;
		chromEnd--;
		}
	    Record* rec=new Record(tokens[cfg->chromColumn],chromStart,chromEnd);
	    rec->tokens=tokens;
	    return rec;
	    }


	Record* parseRecord(std::istream& in,const Config* cfg)
	    {
	    Tokenizer tokenizer(cfg->delim);
	    Record* rec=NULL;
	    string line;
	    vector<string> tokens;
	    for(;;)
		{
		if(AbstractApplication::stopping()) return NULL;
		if(!getline(in,line,'\n')) return NULL;
		tokenizer.split(line,tokens);
		rec=parseRecord(line,tokens,cfg);
		if(rec!=NULL) break;
		}
	    return rec;
	    }

	Record* parseRecord1(std::istream& in)
	    {
	    return parseRecord(in,&config1);
	    }

	Record* parseRecord2(std::istream& in)
	    {
	    return parseRecord(in,&config2);
	    }

	void echo(Record* rec1,Record* rec2)
		{
		if(joinType==PRINT_UMATCHING) return;

		for(size_t i=0;i< rec1->tokens.size();++i)
		    {
		    if(i>0) cout << tokenizer.delim;
		    cout << rec1->tokens[i];
		    }
		for(size_t i=0;i< rec2->tokens.size();++i)
		    {
		    cout << rec2->tokens[i];
		    }
		cout << endl;
		}

	void run(std::istream& in,std::istream& in2)
	    {
	    Record* rec=NULL;
	    auto_vector<Record> buffer;
	    while((rec=parseRecord1(in))!=NULL)
		{
		if(AbstractApplication::stopping()) break;

		/* did we find a match for 'rec'  ? */
		bool foundMatch=false;
		/* should we refill the buffer ? */
		bool refill=true;
		size_t i=0;
		while(i<buffer.size())
		    {
		    Record* rec2=buffer.at(i);
		    int d=rec2->pos.chrom.compare(rec->pos.chrom);
		    if(d<0)
			    {
			    buffer.erase(i);
			    continue;
			    }
		    else if(d>0)
			    {
			    refill=false;
			    break;
			    }
		    if(rec2->pos.end<=rec->pos.start)
			    {
			    buffer.erase(i);
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
		    while((rec2=parseRecord2(in2))!=NULL)
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
		if(!foundMatch && joinType!=PRINT_MATCHING)
		    {
		    for(size_t i=0;i< rec->tokens.size();++i)
			{
			if(i>0) cout << tokenizer.delim;
			cout << rec->tokens[i];
			}
		    cout << endl;
		    }
		delete rec;
		}
	    }


	virtual void usage(ostream& out,int argc,char** argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << VARKIT_REVISION << endl;
		out << "Options:\n";
		out << "   -f <external url/file> [required]" << endl;
		out << " Input Stream/File\n";
		out << "   -c1 <CHROM col> (default:"<< (config1.chromColumn+1) <<")" << endl;
		out << "   -s1 <START col> (default:"<<  (config1.startColumn+1) <<")" << endl;
		out << "   -e1 <END col> (default:"<<  (config1.endColumn+1) <<")" << endl;
		out << "   -d1 <delimiter> (default:tab)" << endl;
		out << "   -h1  toggle: input is half open (default:"<< config1.halfOpen<<")" << endl;
		out << "   -z1  toggle: input zero-based (default:"<< config1.zeroBased<<")" << endl;
		out << " Remove/External Stream/File\n";
		out << "   -c2 <CHROM col> (default:"<< (config2.chromColumn+1) <<")" << endl;
		out << "   -s2 <START col> (default:"<<  (config2.startColumn+1) <<")" << endl;
		out << "   -e2 <END col> (default:"<<  (config2.endColumn+1) <<")" << endl;
		out << "   -d2 <delimiter> (default:tab)" << endl;
		out << "   -h2  toggle: input is half open (default:"<< config2.halfOpen<<")" << endl;
		out << "   -z2  toggle: input zero-based (default:"<< config2.zeroBased<<")" << endl;
		out << "\n\n";
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
			    this->usage(cerr,argc,argv);
			    return(EXIT_FAILURE);
			    }
			else if(strcmp(argv[optind],"-f")==0 && optind+1< argc)
			    {
			    dbName=argv[++optind];
			    }
			ARGVCOL("-c1",config1.chromColumn)
			ARGVCOL("-c2",config2.chromColumn)
			ARGVCOL("-s1",config1.startColumn)
			ARGVCOL("-s2",config2.startColumn)
			ARGVCOL("-e1",config1.endColumn)
			ARGVCOL("-e2",config2.endColumn)
			else if(strcmp(argv[optind],"-z1")==0)
			    {
			    config1.zeroBased=!config1.zeroBased;
			    }
			else if(strcmp(argv[optind],"-z2")==0)
			    {
			    config2.zeroBased=!config2.zeroBased;
			    }
			else if(strcmp(argv[optind],"-h1")==0)
			    {
			    config1.halfOpen=!config1.halfOpen;
			    }
			else if(strcmp(argv[optind],"-h2")==0)
			    {
			    config2.halfOpen=!config2.halfOpen;
			    }
			else if(strcmp(argv[optind],"-d1")==0 && optind+1< argc)
			    {
			    char* p=argv[++optind];
			    if(strlen(p)!=1)
				    {
				    cerr<< "bad delimiter \"" << p << "\"\n";
				    return (EXIT_FAILURE);
				    }
			    app.config1.delim=p[0];
			    }
			else if(strcmp(argv[optind],"-d2")==0 && optind+1< argc)
			    {
			    char* p=argv[++optind];
			    if(strlen(p)!=1)
				    {
				    cerr<< "bad delimiter \"" << p << "\"\n";
				    return (EXIT_FAILURE);
				    }
			    app.config2.delim=p[0];
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

		if(dbName==NULL)
			{
			cerr << "Undefined database name.\n";
			this->usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		istream* in2;
		netstreambuf* buf1=0;

		if(	strncmp(dbName,"http://",7)==0 ||
			strncmp(dbName,"ftp://",6)==0 ||
			strncmp(dbName,"https://",8)==0)
		    {
		    buf1=new netstreambuf;
		    buf1->open(dbName);
		    in2=new istream(buf1);
		    }
		else
		    {
		    fstream* f=new fstream(dbName);
		    if(f->is_open()) THROW("Cannot open "<< dbName);
		    in2=f;
		    }

		if(strlen(dbName)>2 && strncmp(&dbName[strlen(dbName)-3],".gz",3)==0)
		    {
		    deflatestreambuf* buf2=new deflatestreambuf(in2);
		    in2=new istream(buf2);
		    }


		if(optind==argc)
			{
			igzstreambuf buf;
			istream in(&buf);
			this->run(in,*in2);
			}
		else if(optind+1==argc)
			{
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			this->run(in,*in2);
			buf.close();
			++optind;
			}
		else
			{
			cerr << "Illegal number of arguments.\n";
			return EXIT_FAILURE;
			}
		return EXIT_SUCCESS;
		}
    };


int main(int argc,char** argv)
	{
	VcfIntersection app;
	return app.main(argc,argv);
	}
