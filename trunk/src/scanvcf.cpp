/**
* Author:
* 	Pierre Lindenbaum PhD
* Contact:
* 	plindenbaum@yahoo.fr
* Date:
* 	Oct 2011
* WWW:
* 	http://plindenbaum.blogspot.com
* Motivation:
* 	List VCFs
*/
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <fstream>
#include <cstdio>
#include "zstreambuf.h"
#include "tokenizer.h"
using namespace std;


class ScanVCF
	{	
	private:
		bool header_printed;
	public:
		char delim;
		int sample_column;
		int vcf_column;
		
		ScanVCF():header_printed(false),delim('\t'),sample_column(0),vcf_column(1)
			{
			}
		~ScanVCF()
			{
			}
		
		void readvcf(const string& sample,const string& path)
			{
			igzstreambuf buf(path.c_str());
			istream in(&buf);
			string line;
			vector<string> tokens;
			while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				if(line.size()>1 && line[0]=='#' && line[1]=='#') continue;
				if(line[0]=='#' )
					{
					if(line.compare(0,7,"#CHROM\t")!=0)
					 	{
						THROW("In " << path << " Expected line starting with #CHROM but got " << line);
						}
					if(!header_printed)
						{
						cout << line<< "\tSAMPLE" << endl;
						}
					header_printed=true;
					continue;
					}
				cout << line << "\t" << sample << endl;
				}
			}
			
		void run(istream& in)
			{
			string line;
			Tokenizer tokenizer;
			tokenizer.delim=delim;
			vector<string> tokens;
			while(getline(in,line,'\n'))
				{
				if(line.empty() || line[0]=='#') continue;
				if(tokenizer.split(line,tokens)<2)
					{
					THROW("Expected two columns in "<< line << endl);
					}
				if(sample_column>=(int)tokens.size())
					{
					THROW("Cannot find SAMPLE column in  "<< line << endl);
					}
				if(vcf_column>=(int)tokens.size())
					{
					THROW("Cannot find VCF column in  "<< line << endl);
					}
				readvcf(tokens[sample_column],tokens[vcf_column]);
				}
			}
		void usage(int argc,char** argv)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Options:\n";
			cerr << "  --sample or -S <column-index> column for the path to SAMPLE ("<<  sample_column << ")" << endl;
			cerr << "  --vcf or -V <column-index> column for the path to VCF(.gz) ("<<  vcf_column << ")" << endl;
			}
	};
	
int main(int argc,char** argv)
	{
	ScanVCF app;
	int optind=1;
	while(optind < argc)
   		{
   		if(strcmp(argv[optind],"-h")==0)
   			{
   			app.usage(argc,argv);
   			return (EXIT_SUCCESS);
   			}
   		else if((strcmp(argv[optind],"--sample")==0 || strcmp(argv[optind],"-S")==0) && optind+1< argc)
			{
			char* p2;
			app.sample_column=(int)strtol(argv[++optind],&p2,10)-1;
			if(app.sample_column<0 || *p2!=0)
				{
				cerr << "Illegal number for Sample column" << endl;
				return EXIT_FAILURE;
				}
			}
		else if((strcmp(argv[optind],"--vcf")==0 || strcmp(argv[optind],"-V")==0) && optind+1< argc)
			{
			char* p2;
			app.vcf_column=(int)strtol(argv[++optind],&p2,10)-1;
			if(app.vcf_column<0 || *p2!=0)
				{
				cerr << "Illegal number for VCF column" << endl;
				return EXIT_FAILURE;
				}
			}
   		else if(strcmp(argv[optind],"--delim")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    cerr << "Bad delimiter \""<< p << "\"\n";
			    app.usage(argc,argv);
			    return(EXIT_FAILURE);
			    }
			app.delim=p[0];
			}
   		else if(argv[optind][0]=='-')
   			{
   			cerr << "unknown option '" << argv[optind] << "'"<< endl;
   			app.usage(argc,argv);
   			return (EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
        }
    if(app.vcf_column==app.sample_column)
    	{
    	cerr << "VCF column==SAMPLE column !" << endl;
    	return EXIT_FAILURE;
    	}    
       
	if(optind==argc)
	    {
	    app.run(cin);
	    }
    else
	    {
	    while(optind< argc)
			{
			char* filename=argv[optind++];
			fstream in(filename,ios::in);
			if(!in.is_open())
			    {
			    cerr << "Cannot open "<< filename << " " << strerror(errno) << endl;
			    return EXIT_FAILURE;
			    }
			app.run(in);
			in.close();
			}
	    }
	return EXIT_SUCCESS;
	}
