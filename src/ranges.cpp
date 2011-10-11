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
 *	stupid split data for a given numeric value
 */
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <limits>
#include <map>
#include <vector>
#include <iostream>
#include <cmath>
#include <stdint.h>
#include "smartcmp.h"
#include "zstreambuf.h"
#include "tokenizer.h"
using namespace std;

class Ranges
	{
	public:
		struct Segment
		    {
		    int32_t chromStart;
		    int32_t chromEnd;
		    uint32_t count;
		    double total;
		    bool below;
		    };
		typedef map<string,vector<Segment>*,SmartComparator> chrom2segments_map;


		Tokenizer tokenizer;
		int32_t chromColumn;
		int32_t posColumn;
		int32_t valColumn;
		double limit;
		Ranges():chromColumn(0),
			posColumn(1),
			valColumn(-1),
			limit(0.0)
			{
			tokenizer.delim='\t';
			}

		void usage(int argc,char** argv)
		    {
		    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    cerr << "Usage" << endl
			    << "   "<< argv[0]<< " [options] -v val (file|stdin)"<< endl;
		    cerr << "Options:\n";
		    cerr << "  -c <chrom Column> ("<<  (chromColumn+1) << ")" << endl;
		    cerr << "  -p <pos Column> ("<<  (posColumn+1) << ")" << endl;
		    cerr << "  -v <value Column> ("<<  (valColumn+1) << ")" << endl;
		    cerr << "  -d <column-delimiter> (default:tab)" << endl;
		    cerr << "  -L <double: treshold> (default:"<< limit<<")" << endl;
		    cerr << endl;
		    }

		void run(std::istream& in)
		    {
		    chrom2segments_map chrom2segment;
		    string line;
		    vector<string> tokens;
		    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			if(line[0]=='#')
				{
				continue;
				}
			tokenizer.split(line,tokens);
			if(chromColumn>= (int)tokens.size())
			    {
			    cerr << "[error] CHROM column "<< (chromColumn+1) << " for " << line << endl;
			    continue;
			    }
			if(posColumn>= (int)tokens.size())
			    {
			    cerr << "[error] POS column "<< (posColumn+1) << " for " << line << endl;
			    continue;
			    }
			if(valColumn>= (int)tokens.size())
			    {
			    cerr << "[error] VAL column "<< (valColumn+1) << " for " << line << endl;
			    continue;
			    }
			char* p2;

			int32_t pos=(int32_t)strtod(tokens[posColumn].c_str(),&p2);
			if(*p2!=0 )
				{
				cerr << "bad pos in " << line << endl;
				continue;
				}

			double v=strtod(tokens[valColumn].c_str(),&p2);
			if(*p2!=0 || isnan(v))
				{
				cerr << "bad number in " << line << endl;
				continue;
				}
			chrom2segments_map::iterator r= chrom2segment.find(tokens[chromColumn]);
			vector<Segment>* data=NULL;
			if(r==chrom2segment.end())
			    {
			    data=new vector<Segment>;
			    chrom2segment.insert(make_pair(tokens[chromColumn],data));
			    }
			else
			    {
			    data= r->second;
			    }
			bool below(v < this->limit);
			if(!data->empty() && data->back().chromEnd> pos)
			    {
			    THROW("Data are not sorted on CHROM/POS (was "<< line << ")");
			    }
			if(data->empty() || data->back().below!=below)
			    {
			    Segment range;
			    range.below=below;
			    range.chromStart=pos;
			    range.chromEnd=pos;
			    range.total=v;
			    range.count=1;
			    data->push_back(range);
			    }
			else
			    {
			    Segment& last=data->back();
			    last.chromEnd=pos;
			    last.total+=v;
			    last.count++;
			    }
			}
		    cout << "#CHROM" << tokenizer.delim
			 << "chromStart" << tokenizer.delim
			 << "chromEnd" << tokenizer.delim
			 << "length" << tokenizer.delim
			 << "In/Out" << tokenizer.delim
			 << "Mean" << tokenizer.delim
			 << "Count" << endl;
		    for(chrom2segments_map::iterator r= chrom2segment.begin();
			    r!=chrom2segment.end();
			    ++r)
			{
			vector<Segment>* data=r->second;
			for(size_t i=0;i< data->size();++i)
			    {
			    Segment& seg=data->at(i);
			    int32_t B,E;
			    if(i==0)
				{
				B=0;
				}
			    else
				{
				B=data->at(i-1).chromEnd+1;
				}
			    if(i+1==data->size())
				{
				E= std::numeric_limits<int32_t>::max()-10;//avoid futur side effect
				}
			    else
				{
				E=data->at(i+1).chromStart-1;
				}

			    cout << r->first  << tokenizer.delim
				  << B << tokenizer.delim
				  << E << tokenizer.delim
				  << (E-B+1) << tokenizer.delim
				  << (seg.below?'-':'+')<< tokenizer.delim
				  << (seg.total/seg.count)<< tokenizer.delim
				  << seg.count << endl;
			    }
			delete data;
			}
		    }
	};

int main(int argc,char** argv)
    {
    Ranges app;
    int optind=1;
    while(optind < argc)
	    {
	    if(std::strcmp(argv[optind],"-h")==0)
		    {
		    app.usage(argc,argv);
		    return (EXIT_FAILURE);
		    }

	    else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
		{
		char* p2;
		app.chromColumn=(int)strtol(argv[++optind],&p2,10)-1;
		if(app.chromColumn<0 || *p2!=0)
		    {
		    cerr << "Illegal number for CHROM" << endl;
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"-p")==0 && optind+1<argc)
		{
		char* p2;
		app.posColumn=(int)strtol(argv[++optind],&p2,10)-1;
		if(app.posColumn<0 || *p2!=0)
		    {
		    cerr << "Illegal number for POS" << endl;
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"-v")==0 && optind+1<argc)
		{
		char* p2;
		app.valColumn=(int)strtol(argv[++optind],&p2,10)-1;
		if(app.valColumn<0 || *p2!=0)
		    {
		    cerr << "Illegal column number for VAL" << endl;
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"-L")==0 && optind+1<argc)
		{
		char* p2;
		app.limit=(double)strtod(argv[++optind],&p2);
		if(*p2!=0 || isnan(app.limit))
		    {
		    cerr << "Illegal number for treshold" << endl;
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    char* p=argv[++optind];
		    if(strlen(p)!=1)
			{
			cerr << "Bad delimiter \""<< p << "\"\n";
			app.usage(argc,argv);
			return(EXIT_FAILURE);
			}
		    app.tokenizer.delim=p[0];
		    }
	    else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
		    app.usage(argc,argv);
		    return (EXIT_FAILURE);
		    }
	    else
		    {
		    break;
		    }
	    ++optind;
	    }
    if(app.valColumn<0)
	{
	cerr << "Undefined VAL column."<< endl;
	app.usage(argc,argv);
	return (EXIT_FAILURE);
	}

    if(optind==argc)
	    {
	    igzstreambuf buf;
	    istream in(&buf);
	    app.run(in);
	    }
    else if(optind+1==argc)
	    {
	    char* filename=argv[optind++];
	    igzstreambuf buf(filename);
	    istream in(&buf);
	    app.run(in);
	    buf.close();
	    }
    else
	    {
	    cerr << "Illegal number of arguments" << endl;
	    app.usage(argc,argv);
	    }
    return EXIT_SUCCESS;
    }
