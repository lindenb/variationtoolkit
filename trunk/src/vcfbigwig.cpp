/*
 * vcfbigwig.cpp
 *
 *  Created on: Oct 13, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "kentBigWig.h"
#include "netstreambuf.h"
#include "tokenizer.h"
#include "throw.h"
#include "zstreambuf.h"
#include "where.h"
using namespace std;

class VCFBigWig
	{
	public:
		BigWig* bigwig;
		int chromCol;
		int posCol;
		Tokenizer tokenizer;
		int extend;
		VCFBigWig():bigwig(NULL),chromCol(0),posCol(1),extend(0)
			{
			}

		~VCFBigWig()
			{
			if(bigwig!=NULL) delete bigwig;
			}

		void run(istream& in)
			{
		    string line;
		    vector<string> tokens;
		    while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				if(line[0]=='#')
					{
					if(line.size()>1 && line[1]=='#')
						{
						cout << line << endl;
						continue;
						}
					cout    << line
							<< tokenizer.delim << "bigwig:min"
							<< tokenizer.delim << "bigwig:max"
							<< tokenizer.delim << "bigwig:mean"
							<< tokenizer.delim << "bigwig:coverage"
							<< tokenizer.delim << "bigwig:stddev"
							<< endl
							;
					continue;
					}
				tokenizer.split(line,tokens);
				if(chromCol>= (int)tokens.size())
					{
					cerr << "[error] CHROM column "<< (chromCol+1) << " for " << line << endl;
					continue;
					}
				if(posCol>= (int)tokens.size())
					{
					cerr << "[error] POS column "<< (posCol+1) << " for " << line << endl;
					continue;
					}
				char* p2;

				int32_t pos=(int32_t)strtod(tokens[posCol].c_str(),&p2);
				if(*p2!=0 )
					{
					cerr << "bad pos in " << line << endl;
					continue;
					}
				pos--;
				const char* chrom=tokens[chromCol].c_str();
				int chromStart=max(pos-extend,0);
				int chromEnd=(pos+extend+1);
				cout    << line
						<< tokenizer.delim << bigwig->minimum(chrom,chromStart,chromEnd)
						<< tokenizer.delim << bigwig->maximum(chrom,chromStart,chromEnd)
						<< tokenizer.delim << bigwig->mean(chrom,chromStart,chromEnd)
						<< tokenizer.delim << bigwig->coverage(chrom,chromStart,chromEnd)
						<< tokenizer.delim << bigwig->stdDev(chrom,chromStart,chromEnd)
						<< endl
						;
				}
		    }
		void usage(int argc,char** argv)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Usage" << endl
				<< "   "<< argv[0]<< " [options] -f file.bwig (file|stdin)"<< endl;
			cerr << "Options:\n";
			cerr << "  -f <bigwig file>" << endl;
			cerr << "  -d <delimiter> (default:tab)" << endl;
			cerr << "  -c <CHROM column=int> (default:"<< (chromCol+1)<< ")" << endl;
			cerr << "  -p <POS column=int> (default:"<< (posCol+1)<< ")" << endl;
			cerr << "  -x <extend=int> extends window size (default:"<< (extend)<< ")" << endl;
			cerr << endl;
			}
	};

#define SETINDEX(option,col) else if(std::strcmp(argv[optind],option)==0 && optind+1<argc) \
	{\
	char* p2;\
	int idx=strtol(argv[++optind],&p2,10);\
      	if(idx<1 || *p2!=0) \
		{\
		cerr << "Bad " option " column index in "<< argv[optind] << endl;\
		app.usage(argc,argv);\
		return EXIT_FAILURE;\
		}\
	app.col=idx-1;\
	}

int main(int argc,char** argv)
    {
	VCFBigWig app;
	char* bigWigFile=NULL;
    int optind=1;
    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			app.usage(argc,argv);
   			exit(EXIT_FAILURE);
   			}
   		SETINDEX("-c",chromCol)
   		SETINDEX("-p",posCol)
   		else if(std::strcmp(argv[optind],"-x")==0 && optind+1< argc)
			{
   			char* p2;
   			app.extend =(int)strtol(argv[++optind],&p2,10);
   			      	if(app.extend<0 || *p2!=0)
				{
				cerr << "Bad extend option in  "<< argv[optind] << endl;
				app.usage(argc,argv);
				return EXIT_FAILURE;
				}
			}
   		else if(std::strcmp(argv[optind],"-f")==0 && optind+1< argc)
   			{
   			bigWigFile=argv[++optind];
   			}
   		else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    cerr << "Bad delimiter \""<< p << "\"\n";
			    app.usage(argc,argv);
			    exit(EXIT_FAILURE);
			    }
			app.tokenizer.delim=p[0];
			}
   		else if(argv[optind][0]=='-')
   			{
   			fprintf(stderr,"unknown option '%s'\n",argv[optind]);
			app.usage(argc,argv);
   			exit(EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
                }

   if(bigWigFile==NULL)
   	   {
	   cerr << "Undefined bigWig file."<< endl;
	   app.usage(argc,argv);
	   return (EXIT_FAILURE);
   	   }
   app.bigwig=new BigWig(bigWigFile);
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
    return EXIT_SUCCESS;
    }
