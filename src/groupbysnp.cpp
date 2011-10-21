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
 *	group VCF data by variation
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

using namespace std;


typedef int column_t;


class GroupBySnp:public AbstractApplication
    {
    public:
	column_t chromcol;
	column_t poscol;
	column_t genecol;
	column_t refcol;
	column_t altcol;
	column_t samplecol;
	set<column_t> left_columns;
	set<column_t> top_columns;
	bool use_ref_alt;
	vector<vector<string> > buffer;
	vector<string> sampleNames;


	GroupBySnp()
	    {
	    chromcol=0;
	    poscol=1;
	    refcol=3;
	    altcol=4;
	    samplecol=-1;
	    use_ref_alt=true;
	    }
	~GroupBySnp()
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

	void samplesInBuffer(set<string>& set)
		{
		set.clear();
		for(size_t i=0;i< buffer.size();++i)
			{
			if(buffer[i][samplecol].empty())
				{
				cerr << "Warning empty sample name in "<< buffer[i][samplecol]<< endl;
				}
			set.insert(buffer[i][samplecol]);
			}
		}



	void dumpBuffer()
		{
		if(buffer.empty()) return;
		set<string> samples;
		samplesInBuffer(samples);
		bool first;
		const vector<string>& front=buffer.front();

		for(set<int>::iterator r=left_columns.begin();r!=left_columns.end();++r)
			{
			if(first) cout << tokenizer.delim;
			first=false;
			cout << front[*r];
			}
		for(vector<string>::iterator r2=sampleNames.begin();r2!=sampleNames.end();++r2)
			{
			bool sample_found=false;
			for(size_t i=0;i< buffer.size();++i)
				{
				if(buffer[i][samplecol].compare(*r2)!=0) continue;
				sample_found=true;
				if(first) cout << tokenizer.delim;
				first=false;
				cout << (*r2);
				for(set<int>::iterator r=top_columns.begin();r!=top_columns.end();++r)
					{
					cout << tokenizer.delim;
					cout << buffer[i][*r];
					}
				break;
				}
			if(!sample_found)
				{
				for(size_t i=0;i < 1+top_columns.size();++i)
					{
					if(first) cout << tokenizer.delim;
					cout << ".";
					first=false;
					}
				}
			}
		cout << tokenizer.delim << samples.size() << endl;

		buffer.clear();
		}


	void run(std::istream& in)
	    {
		vector<string> tokens;
	    string line;
	    int max_index=chromcol;
	    max_index=max(max_index,samplecol);
	    max_index=max(max_index,poscol);
	    max_index=max(max_index,refcol);
	    max_index=max(max_index,altcol);
	    for(set<int>::iterator r=top_columns.begin();r!=top_columns.end();++r)
	    	{
	    	max_index=max(max_index,*r);
	    	}
	    for(set<int>::iterator r=left_columns.begin();r!=left_columns.end();++r)
			{
			max_index=max(max_index,*r);
			}


	    while(getline(in,line,'\n'))
			{
	    	bool first=true;
			if(line.empty()) continue;
			tokenizer.split(line,tokens);
			if(line[0]=='#' && line.size()>1 && line[1]=='#') continue;
			if(max_index>=(int)tokens.size())
				{
				cerr << "Column out of range in " << line << endl;
				continue;
				}
			if(line[0]=='#')
				{
				for(set<int>::iterator r=left_columns.begin();r!=left_columns.end();++r)
					{
					cout << (first?'#':tokenizer.delim);
					first=false;
					cout << tokens[*r];
					}
				for(vector<string>::iterator r2=sampleNames.begin();r2!=sampleNames.end();++r2)
					{
					cout << tokenizer.delim;
					cout << (*r2);
					first=false;
					for(set<int>::iterator r=top_columns.begin();r!=top_columns.end();++r)
							{
							cout << tokenizer.delim  << (*r2) <<":"<< tokens[*r];
							}
					}
				cout << tokenizer.delim << "count.samples" << endl;
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
		out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Options:\n";
		out << "  --delim (char) or -d  (char) <delimiter> default:tab\n";
		out << "  --norefalt : don't look at REF and ALT\n";
		out << "  --sample SAMPLE column index\n";
		out << "  --chrom CHROM column index: default "<< (chromcol+1) << "\n";
		out << "  --pos POS position column index: default "<< (poscol+1) << "\n";
		out << "  --ref REF reference allele column index: default "<< (refcol+1) << "\n";
		out << "  --alt ALT alternate allele column index: default "<< (altcol+1) << "\n";
		out << "  -T 1,2,3,4 columns indexes on top.\n";
		out << "  -L 5,6,7 columns indexes on left.\n";
		out << "  -n <name1,name2,name3> add this sample name.\n";
		out << "(stdin|vcf|vcf.gz)\n";
		}


#define SETINDEX(option,col) else if(std::strcmp(argv[optind],option)==0 && optind+1<argc) \
	{\
	char* p2;\
	int idx=strtol(argv[++optind],&p2,10);\
      	if(idx<1)  { cerr << "Bad " option " index in "<< argv[optind] << endl; return EXIT_FAILURE;}\
	this->col=idx-1;\
	}


int main(int argc,char** argv)
    {
	Tokenizer comma;
	comma.delim=',';
    int optind=1;
    set<string,SmartComparator> sNames;
    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			usage(cerr,argc,argv);
   			exit(EXIT_FAILURE);
   			}
   		SETINDEX("--sample",samplecol)
   		SETINDEX("--chrom",chromcol)
   		SETINDEX("--pos",poscol)
   		SETINDEX("--ref",refcol)
   		SETINDEX("--alt",altcol)
   		else if(std::strcmp(argv[optind],"-L")==0 && optind+1 < argc)
			{
			vector<string> tokens;
			comma.split(argv[++optind],tokens);
			for(size_t i=0;i< tokens.size();++i)
				{
				if(tokens[i].empty()) continue;
				char* p2;
				int idx=strtol(tokens[i].c_str(),&p2,10);\
				if(idx<1)
					{
					cerr << "Bad column index in "<< argv[optind] << endl;
					return EXIT_FAILURE;
					}
				idx-=1;
				this->left_columns.insert(idx);
				}
			}
   		else if(std::strcmp(argv[optind],"-T")==0 && optind+1 < argc)
			{
			vector<string> tokens;
			comma.split(argv[++optind],tokens);
			for(size_t i=0;i< tokens.size();++i)
				{
				if(tokens[i].empty()) continue;
				char* p2;
				int idx=strtol(tokens[i].c_str(),&p2,10);\
				if(idx<1)
					{
					cerr << "Bad column index in "<< argv[optind] << endl;
					return EXIT_FAILURE;
					}
				idx-=1;
				this->top_columns.insert(idx);
				}
			}
   		else if(std::strcmp(argv[optind],"-n")==0 && optind+1 < argc)
			{
			vector<string> tokens;
			comma.split(argv[++optind],tokens);
			for(size_t i=0;i< tokens.size();++i)
				{
				if(tokens[i].empty()) continue;
				sNames.insert(tokens[i]);
				}
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

    for(set<string,SmartComparator>::iterator r=sNames.begin();r!=sNames.end();++r)
    	{
    	this->sampleNames.push_back(*r);
    	}

    if(this->sampleNames.empty())
    	{
    	cerr << "No sample name defined."<< endl;
    	usage(cerr,argc,argv);
    	return (EXIT_FAILURE);
    	}

    if(samplecol==-1)
    	{
    	cerr << "Undefined sample column."<< endl;
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
	GroupBySnp app;
	return app.main(argc,argv);
	}

