/*
 * ttmap.cpp
 *
 *  Created on: Dec 17, 2011
 *      Author: lindenb
 */
#include <limits>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "numeric_cast.h"
#include "application.h"
#include "zstreambuf.h"
#include "auto_vector.h"

using namespace std;

class GObject
    {
    public:
	int32_t start;
	int32_t end;
	int8_t strand;
	std::string name;

	bool operator<(const GObject& cp) const
	    {
	    if(start!=cp.start) return start<cp.start;
	    if(end!=cp.end) return end<cp.end;
	    return name< cp.name;
	    }

    };

struct Sorter
    {
    bool    operator()(const GObject* o1 , const GObject* o2)
	    {
	    return *o1 < *o2;
	    }
    };

class TTMap:public AbstractApplication
    {
    public:
	int columns;
	int chromCol;
	int startCol;
	int endCol;
	int32_t nameCol;
	int32_t strandCol;

	TTMap():chromCol(0),
		startCol(1),
		endCol(2),
		nameCol(3),
		strandCol(-1)

	    {

	    }
#define CHECK_COL(c) if((int32_t)tokens.size()<=c){\
	    cerr << "Expected at least "<< (c+1)<<  " columns in "<< line \
	    << "but got " << tokens.size() << " " << #c << endl;\
	    continue;\
	    }
#define CONVERT_X(x) (((double)((x)-chromStart)/(double)(chromEnd-chromStart))*(double)this->columns)

	void dump(
		const std::string& chromosome,
		auto_vector<GObject>& data
		)
	    {
	    if(data.empty()) return;
	    int32_t chromStart=numeric_limits<int32_t>::max();
	    int32_t chromEnd=numeric_limits<int32_t>::min();
	    vector<GObject*> v;
	    v.reserve(data.size());
	    for(size_t i=0;i< data.size();++i)
		{
		chromStart = min(chromStart,data[i]->start);
		chromEnd = max(chromEnd,data[i]->end);
		v.push_back(data[i]);
		}

	    Sorter sorter;
	    sort(v.begin(),v.end(),sorter);

	    cout << ">" << chromosome << ":" << chromStart << "-" << chromEnd << endl;
	    for(size_t i=0;i< v.size();++i)
		{
		GObject* object=v.at(i);
		int x0=CONVERT_X(object->start);
		int x1=CONVERT_X(object->end);
		if(x1==x0) x1++;
		for(int x=0;x< this->columns;++x)
		    {
		    char pixel=' ';
		    if(x>0 && x%15==0) pixel='.';
		    if(x>=x0 && x< x1)
			{
			pixel='=';
			if(object->strand==1 && x+1==x1) pixel='>';
			if(object->strand==-1 && x==x0) pixel='<';
			}
		    cout << pixel;
		    }
		if(!object->name.empty())
		    {
		    cout << " " << object->name;
		    }
		cout << endl;
		}
	    cout << endl;
	    data.clear();
	    }

	void run(std::istream& in)
	    {
	    std::string prev_chrom("\1\1",2);
	    std::string line;
	    std::vector<std::string> tokens;
	    auto_vector<GObject> data;
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		tokenizer.split(line,tokens);
		CHECK_COL(chromCol);
		if(tokens[chromCol].compare(prev_chrom)!=0)
		    {
		    dump(prev_chrom,data);
		    data.clear();
		    }
		prev_chrom.assign(tokens[chromCol]);
		CHECK_COL(startCol);
		int32_t start;
		if(!numeric_cast<int32_t>(tokens[startCol].c_str(),&start))
		    {
		    cerr << "Bad START in " << line << endl;
		    continue;
		    }
		int32_t end=start+1;
		if(endCol!=-1)
		    {
		    CHECK_COL(endCol);
		    if(!numeric_cast<int32_t>(tokens[startCol].c_str(),&end) || end < start)
			{
			cerr << "Bad END in " << line << endl;
			continue;
			}
		    }
		int8_t strand=0;
		if(strandCol!=-1)
		    {
		    CHECK_COL(strandCol);
		    if(tokens[strandCol].compare("+")==0)
			{
			strand=1;
			}
		    else if(tokens[strandCol].compare("-")==0)
			{
			strand=-1;
			}
		    }
		 string name;
		 if(nameCol!=-1)
		     {
		     CHECK_COL(nameCol);
		     name.assign(tokens[nameCol]);
		     }
		 GObject* object=new GObject;
		 object->start=start;
		 object->end=end;
		 object->strand=strand;
		 object->name.assign(name);
		 data.push_back(object);
		 }
	    dump(prev_chrom,data);
	    }


virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<"." << endl;
	    out << "Options:\n";
	    out << "  -c (int) chrom column default:(" << (chromCol+1)<< endl;
	    out << "  -s (int) start column default:(" << (startCol+1)<< endl;
	    out << "  -e (int) end column default: start column"<< endl;
	    out << "  -o (int) strand column default (optional)"<<endl;
	    out << "  -n (int) name column default (optional)"<<endl;
	    out << "  -d (char) delimiter default:tab\n";
	    out << "  -C (int) fix the number of output columns."<< endl;
	    out << "(stdin|bed|bed.gz)\n\n";
	    }




#define ASKCOL(opt,var) if(strcmp(argv[optind],opt)==0 && optind+1<argc) \
	{\
	if(!numeric_cast<int32_t>(argv[++optind],&(var)) || var < 1) \
	    {\
	    cerr << "Bad column index for "<< #var << endl;\
	    usage(cerr,argc,argv);\
	    return EXIT_FAILURE;\
	    }\
	var--;\
	}

	int main(int argc,char** argv)
	    {
		int optind=1;
		while(optind < argc)
			{
			if(std::strcmp(argv[optind],"-h")==0)
				{
				this->usage(cerr,argc,argv);
				return (EXIT_FAILURE);
				}
			else ASKCOL("-c",chromCol)
			else ASKCOL("-s",startCol)
			else ASKCOL("-e",endCol)
			else ASKCOL("-o",strandCol)
			else ASKCOL("-n",nameCol)
			else if(std::strcmp(argv[optind],"-C")==0 && optind+1< argc)
			    {
			    if(!numeric_cast<int32_t>(argv[++optind],&columns))
				{
				cerr << "Bad column index for COLS." << endl;
				usage(cerr,argc,argv);
				return EXIT_FAILURE;
				}
			    }
			else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
				{
				char* p=argv[++optind];
				if(strlen(p)!=1)
				    {
				    cerr << "Bad delimiter \""<< p << "\"\n";
				    this->usage(cerr,argc,argv);
				    return(EXIT_FAILURE);
				    }
				this->tokenizer.delim=p[0];
				}
			else if(argv[optind][0]=='-')
				{
				cerr << "unknown option '"<< argv[optind]<<"'\n";
				this->usage(cerr,argc,argv);
				return (EXIT_FAILURE);
				}
			else
				{
				break;
				}
			++optind;
			}
		if(columns==-1)
		    {
		    columns=80;
		    char* p=getenv("COLUMNS");
		    if(	p!=NULL &&
			!numeric_cast<int32_t>(p,&columns))
			{
			columns=80;
			}
		    columns-=15;
		    }
		if(columns<10)
		    {
		    columns=10;
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
			    if(AbstractApplication::stopping()) break;
			    char* filename=argv[optind++];
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
    TTMap app;
    return app.main(argc,argv);
    }

