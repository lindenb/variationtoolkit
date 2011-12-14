/*
 * loessapp.cpp
 *
 *  Created on: Dec 14, 2011
 *      Author: lindenb
 */
#include <algorithm>
#include "numeric_cast.h"
#include "application.h"
#include "loess.h"
#include "zstreambuf.h"


using namespace std;


class LoessApp:public AbstractApplication
    {
    public:
	class Record
	    {
	    public:
		double x;
		double y;
		std::string line;
	    };

	class Sorter
	    {
	    public:
		bool operator()(const Record* r1,const Record* r2) const
		    {
		    return r1->x < r2->x;
		    }
	    };
	Sorter _sorter;
	Loess loessAlgo;
	int32_t groupCol;
	int32_t xCol;
	int32_t yCol;
	std::vector<Record*> records;

	LoessApp():groupCol(-1),xCol(-1),yCol(-1)
	    {

	    }

	virtual ~LoessApp()
	    {

	    }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -g (group col) (default:no group)" << endl;
	    out << "  -x (chromStart col) (default:none, use row index))" << endl;
	    out << "  -y (chromEnd col) required." << endl;
	    out << endl;
	    }

	void dump()
	    {
	    if(records.empty()) return;
	    bool is_sorted=true;
	    for(size_t i=1;i< records.size();++i)
		{
		if(records[i-1]->x > records[i]->x)
		    {
		    is_sorted=false;
		    break;
		    }
		}
	    if(!is_sorted)
		{
		std::sort(records.begin(),records.end(),_sorter);
		}
	    double* x=new double[records.size()];
	    double* y=new double[records.size()];
	    for(size_t i=0;i< records.size();++i)
		{
		x[i]=records[i]->x;
		y[i]=records[i]->y;
		}
	    auto_ptr<vector<double> > y2= loessAlgo.lowess(x,y,(int32_t)records.size());
	    delete[] x;
	    delete[] y;


	    for(size_t i=0;i< records.size();++i)
		{
		cout <<  records[i]->line << tokenizer.delim << y2->at(i) << endl;
		delete records[i];
		}
	    records.clear();
	    }

	void run(std::istream& in)
	    {
	    double nLine=0;
	    string line;
	    vector<string> tokens;
	    string curr_group("\0\0",2);
	    while(getline(in,line,'\n'))
		{
		++nLine;
		if(!line.empty() && line[0]=='#')
		    {
		    cout << line << tokenizer.delim << "Lowess"<< endl;
		    continue;
		    }
		tokenizer.split(line,tokens);
		if(groupCol!=-1)
		    {
		    if(groupCol>=(int32_t)tokens.size())
			{
			cerr << "column out of bound for GROUP in "<< line;
			continue;
			}
		    if(tokens[groupCol].compare(curr_group)!=0)
			{
			dump();
			curr_group.assign(tokens[groupCol]);
			}
		    }


		if(xCol!=-1 && xCol>=(int32_t)tokens.size())
		    {
		    cerr << "column out of bound for X in "<< line;
		    continue;
		    }
		if(yCol>=(int32_t)tokens.size())
		    {
		    cerr << "column out of bound for Y in "<< line;
		    continue;
		    }
		double x_value=nLine;
		if(xCol!=-1)
		    {
		    if(!numeric_cast<double>(tokens[xCol].c_str(),&x_value))
			{
			cerr << "Bad 'X' in "<< line << endl;
			continue;
			}
		    }
		double y_value=0;
		if(!numeric_cast<double>(tokens[yCol].c_str(),&y_value))
		    {
		    cerr << "Bad 'Y' in "<< line << endl;
		    continue;
		    }

		Record* rec=new Record;
		rec->line.assign(line);
		rec->x=x_value;
		rec->y=y_value;
		records.push_back(rec);
		}
	    dump();
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
		else if(strcmp(argv[optind],"-g")==0 && optind+1< argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&groupCol) || groupCol<1)
			{
			cerr << "Bad option '-g' " << argv[optind]<< endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    groupCol--;
		    }
		else if(strcmp(argv[optind],"-x")==0 && optind+1< argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&xCol) || xCol<1)
			{
			cerr << "Bad option '-x' " << argv[optind] << endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    xCol--;
		    }
		else if(strcmp(argv[optind],"-y")==0 && optind+1< argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&yCol) || yCol<1)
			{
			cerr << "Bad option '-y' " << argv[optind] << endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    yCol--;
		    }
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
	    if(yCol==-1)
		{
		cerr << "undefined y column:" << endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
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
		    }
		}
	    return EXIT_SUCCESS;
	    }

    };

int main(int argc,char** argv)
    {
    LoessApp app;
    return app.main(argc,argv);
    }
