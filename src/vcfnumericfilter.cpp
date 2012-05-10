/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	May 2012
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	numeric filter for VCF, filters only if all CHROM/POS/REF/ALT match criteria
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
#include "application.h"
#include "smartcmp.h"
#include "numeric_cast.h"

using namespace std;


typedef int column_t;


class VcfNumericFilter:public AbstractApplication
    {
    public:
	column_t chromcol;
	column_t poscol;
	column_t genecol;
	column_t refcol;
	column_t altcol;
	column_t observedCol;
	bool use_ref_alt;
	auto_ptr<double> min_value;
	auto_ptr<double> max_value;
	vector<vector<string> > buffer;

	VcfNumericFilter():observedCol(-1),min_value(0),max_value(0)
	    {
	    chromcol=0;
	    poscol=1;
	    refcol=3;
	    altcol=4;
	    use_ref_alt=true;
	    }
	~VcfNumericFilter()
	    {

	    }

	
	bool same(const string& line1,const string& line2)
	    {
	    return strcasecmp(line1.c_str(),line2.c_str())==0;
	    }
	bool equals(const vector<string>& tokens1,const vector<string>& tokens2)
	    {
	    if(!same(tokens1[chromcol],tokens2[chromcol])) return false;
	    if(!same(tokens1[poscol],tokens2[poscol])) return false;
	    if(use_ref_alt)
			{
			if(!same(tokens1[refcol],tokens2[refcol]))
				{
				cerr << "?not same ref beween\n\t"
						<< tokens1[chromcol]<< ":"<< tokens1[poscol]<<":"<< tokens1[refcol] << " and "
						<< tokens2[chromcol]<< ":"<< tokens2[poscol]<<":"<< tokens2[refcol] << endl;
				return false;
				}
			if(!same(tokens1[altcol],tokens2[altcol]))
				{
				return false;
				}
			}
	    return true;
	    }




	void dumpBuffer()
		{
		if(buffer.empty()) return;
		vector<string> tokens;
		
		size_t count_ok=0;
		for(size_t i=0;i< buffer.size();++i)
			{
			double v;
			if(!numeric_cast<double>(buffer[i][observedCol].c_str(),&v))
				{
				cerr << "Bad numeric column in " << buffer[i][observedCol] << endl;
				continue;
				}
			
			if(min_value.get()!=0 && v < (*min_value))
				{
				continue;
				}
			if(max_value.get()!=0 && v > (*max_value))
				{
				continue;
				}
			++count_ok;
			}
		
		if(count_ok!=0)
			{
			for(size_t i=0;i< buffer.size();++i)
				{
				for(size_t j=0;j< buffer[i].size();++j)
					{
					if(j>0) cout << "\t";
					cout << buffer[i][j];
					}
				cout << endl;
				}
			}

		
		
		buffer.clear();
		}


	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    int max_index=chromcol;
	    max_index=max(max_index,observedCol);
	    max_index=max(max_index,poscol);
	    max_index=max(max_index,refcol);
	    max_index=max(max_index,altcol);

	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			
			if(line[0]=='#')
				{
				cout << line << endl;
				continue;
				}
			tokenizer.split(line,tokens);
			if(max_index>=(int)tokens.size())
				{
				cerr << "Column out of range in " << line << endl;
				continue;
				}

			if(buffer.empty() || equals(buffer.front(),tokens))
				{
				buffer.push_back(tokens);
				}
			else
				{
				dumpBuffer();
				buffer.clear();
				buffer.push_back(tokens);
				}
			}
	    dumpBuffer();
	    }
	void usage(ostream& out,int argc,char **argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  --delim (char) or -d  (char) <delimiter> default:tab\n";
		out << "  --norefalt : don't look at REF and ALT\n";
		out << "  --chrom CHROM column index: default "<< (chromcol+1) << "\n";
		out << "  --pos POS position column index: default "<< (poscol+1) << "\n";
		out << "  --ref REF reference allele column index: default "<< (refcol+1) << "\n";
		out << "  --alt ALT alternate allele column index: default "<< (altcol+1) << "\n";
		out << "  -c observed column index: REQUIRED\n";
		out << "  -m (min-inclusive value) OPTIONAL.\n";
		out << "  -M (max-inclusive value) OPTIONAL.\n";
		out << "(stdin|vcf|vcf.gz)\n";
		}


#define SETINDEX(option,col) else if(std::strcmp(argv[optind],option)==0 && optind+1<argc) \
	{\
	if(!numeric_cast<int>(argv[++optind],&(this->col)) || this->col<1) \
      		{ cerr << "Bad " option " index in "<< argv[optind] << endl; return EXIT_FAILURE;}\
	this->col--;\
	}


int main(int argc,char** argv)
    {
	Tokenizer comma;
	comma.delim=',';
    int optind=1;

    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			usage(cerr,argc,argv);
   			exit(EXIT_FAILURE);
   			}
   		SETINDEX("--chrom",chromcol)
   		SETINDEX("--pos",poscol)
   		SETINDEX("--ref",refcol)
   		SETINDEX("--alt",altcol)
		SETINDEX("-c",observedCol)
   		else if(std::strcmp(argv[optind],"-m")==0 && optind+1 < argc)
   			{
			double v;
			if(!numeric_cast<double>(argv[++optind],&v))
				{
				cerr << "Bad min value :" << argv[optind] << endl;
				return EXIT_FAILURE;
				}
			this->min_value.reset(new double(v));
   			}
		else if(std::strcmp(argv[optind],"-M")==0 && optind+1< argc)
			{
			double v;
			if(!numeric_cast<double>(argv[++optind],&v))
				{
				cerr << "Bad max value :" << argv[optind] << endl;
				return EXIT_FAILURE;
				}
			this->max_value.reset(new double(v));
			} 
		else if(std::strcmp(argv[optind],"--norefalt")==0)
   			{
   			use_ref_alt=false;
   			}
   		else if(
   				(std::strcmp(argv[optind],"-d")==0 ||
   				 std::strcmp(argv[optind],"--delim")==0) && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    cerr << "Bad delimiter \""<< p << "\"\n";
			    usage(cerr,argc,argv);
			    return(EXIT_FAILURE);
			    }
			tokenizer.delim=p[0];
			}
   		else if(argv[optind][0]=='-')
   			{
   			cerr << "unknown option '"<< argv[optind] << "'\n";
			usage(cerr,argc,argv);
			return(EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
        }

    if(this->min_value.get()!=0 && this->max_value.get()!=0 && *(this->min_value) > *(this->max_value))
    	{
    	cerr << "Bad min/max."<< endl;
    	usage(cerr,argc,argv);
    	return (EXIT_FAILURE);
    	}

    if(observedCol==-1)
    	{
    	cerr << "Undefined observed column."<< endl;
    	usage(cerr,argc,argv);
    	return (EXIT_FAILURE);
    	}
   if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		run(in);
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
	VcfNumericFilter app;
	return app.main(argc,argv);
	}

