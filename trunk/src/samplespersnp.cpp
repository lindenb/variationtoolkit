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
 *	group VCF data by sample/SNP
 * Compilation:
 *	 g++ -o samplepersnp -Wall -O3 samplepersnp.cpp -lz
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

using namespace std;


typedef int column_t;

#define CHECK_COL_INDEX(idx,tokens) do { if(((size_t)idx)>=tokens.size()) \
	{\
	THROW("Column index " <<  #idx << " out of range ("<< idx << ")");\
	} } while(0)

#define CHECK_COL_INDEXES(list) \
	    CHECK_COL_INDEX(chromcol,list);\
	    CHECK_COL_INDEX(poscol,list);\
	    if(use_ref_alt) CHECK_COL_INDEX(refcol,list);\
	    if(use_ref_alt) CHECK_COL_INDEX(altcol,list);\
	    CHECK_COL_INDEX(samplecol,list);

class SamplePerSnp:public AbstractApplication
    {
    public:
	column_t chromcol;
	column_t poscol;
	column_t genecol;
	column_t refcol;
	column_t altcol;
	column_t samplecol;

	bool use_ref_alt;
	vector<string> buffer;
	void* selectSetOpaque;
	string matching_flag;///output all results but add extra column with this flag

	SamplePerSnp()
	    {
	    chromcol=0;
	    poscol=1;
	    refcol=3;
	    altcol=4;
	    samplecol=-1;
	    use_ref_alt=true;
	    selectSetOpaque=NULL;
	    }
	~SamplePerSnp()
	    {

	    }

	
	bool same(const string& line1,const string& line2)
	    {
	    return strcasecmp(line1.c_str(),line2.c_str())==0;
	    }
	bool equals(const string& line1,const string& line2)
	    {
	    vector<string> tokens1;
	    tokenizer.split(line1,tokens1);
	    CHECK_COL_INDEXES(tokens1);
	    vector<string> tokens2;
	    tokenizer.split(line2,tokens2);
	    CHECK_COL_INDEXES(tokens2);
	    if(!same(tokens1[chromcol],tokens2[chromcol])) return false;
	    if(!same(tokens1[poscol],tokens2[poscol])) return false;
	    if(use_ref_alt)
		{
		if(!same(tokens1[refcol],tokens2[refcol]))
		    {
		    cerr << "?not same ref beween\n\t"
			<< line1 << "\n\t"<< line2 << endl;
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
			vector<string> tokens;
			tokenizer.split(buffer[i],tokens);
			CHECK_COL_INDEX(samplecol,tokens);
			if(tokens[samplecol].empty())
				{
				cerr << "Warning empty sample name in "<< buffer[i]<< endl;
				}
			set.insert(tokens[samplecol]);
			}
		}



	void dumpBuffer()
		{
		if(buffer.empty()) return;
		set<string> samples;
		samplesInBuffer(samples);
		bool print=true;

		if(selectSetOpaque!=NULL)
			{
			extern bool selectSetEval(void*,const set<string>&);
			print=selectSetEval(selectSetOpaque,samples);
			}

		if(selectSetOpaque!=NULL && !matching_flag.empty())
			{
			for(size_t i=0;i< buffer.size();++i)
				{
				cout << buffer[i] << tokenizer.delim <<samples.size() <<  tokenizer.delim;
				if(print)
					{
					cout << matching_flag;
					}
				else
					{
					cout << ".";
					}
				cout << endl;
				}
			}
		else if(print)
			{
			for(size_t i=0;i< buffer.size();++i)
				{
				cout << buffer[i] << tokenizer.delim <<samples.size()<< endl;
				}
			}
		buffer.clear();
		}


	void run(std::istream& in)
	    {
	    string line;

	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			if(line[0]=='#')
				{
				cout << line << tokenizer.delim << "Sample/SNP";
				if(selectSetOpaque!=NULL && !matching_flag.empty())
					{
					cout << tokenizer.delim << "FLAG(Sample/SNP)";
					}
				cout << endl;
				continue;
				}

			if(buffer.empty() || equals(buffer.front(),line))
				{
				buffer.push_back(line);
				}
			else
				{
				dumpBuffer();
				buffer.clear();
				buffer.push_back(line);
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
		out << "  -e <query> (optional) filters by samples using boolean request eg. '((S1 && S2) || (!(S3) || \"S4\"))'."<< "\n";
		out << "  -m <STRING> (optional) with option \'-e\', prints ALL the line but append this flags at the end for the matching rows."<< "\n";
		out << "(stdin|vcf|vcf.gz)\n";
		}
    };

#define SETINDEX(option,col) else if(std::strcmp(argv[optind],option)==0 && optind+1<argc) \
	{\
	char* p2;\
	int idx=strtol(argv[++optind],&p2,10);\
      	if(idx<1) THROW("Bad " option " index in "<< argv[optind]);\
	app.col=idx-1;\
	}


int main(int argc,char** argv)
    {
    SamplePerSnp app;
    int optind=1;
    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			app.usage(cerr,argc,argv);
   			exit(EXIT_FAILURE);
   			}
   		else if(std::strcmp(argv[optind],"-e")==0 && optind+1 < argc)
			{
   			extern void* selectSetParse(const char* s);
			app.selectSetOpaque=selectSetParse(argv[++optind]);
			}
		else if(std::strcmp(argv[optind],"-m")==0 && optind+1 < argc)
			{
			app.matching_flag.assign(argv[++optind]);
			}
   		SETINDEX("--sample",samplecol)
   		SETINDEX("--chrom",chromcol)
   		SETINDEX("--pos",poscol)
   		SETINDEX("--ref",refcol)
   		SETINDEX("--alt",altcol)
   		else if(std::strcmp(argv[optind],"--norefalt")==0)
   			{
   			app.use_ref_alt=false;
   			}
   		else if(
   				(std::strcmp(argv[optind],"-d")==0 ||
   				 std::strcmp(argv[optind],"--delim")==0) && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    cerr << "Bad delimiter \""<< p << "\"\n";
			    app.usage(cerr,argc,argv);
			    exit(EXIT_FAILURE);
			    }
			app.tokenizer.delim=p[0];
			}
   		else if(argv[optind][0]=='-')
   			{
   			cerr << "unknown option '"<< argv[optind] << "'\n";
			app.usage(cerr,argc,argv);
   			exit(EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
                }
    if(app.samplecol==-1)
    	{
    	cerr << "Undefined sample column."<< endl;
    	app.usage(cerr,argc,argv);
    	return (EXIT_FAILURE);
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
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			app.run(in);
			buf.close();
			++optind;
			}
		}
   if(app.selectSetOpaque!=NULL)
   	   {
	   extern void selectSetFree(void* ptr);
	   selectSetFree(app.selectSetOpaque);
   	   }
    return EXIT_SUCCESS;
    }
