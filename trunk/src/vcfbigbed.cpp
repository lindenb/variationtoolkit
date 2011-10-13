/*
 * vcfBigBed.cpp
 *
 *  Created on: Oct 13, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "kentBigBed.h"
#include "netstreambuf.h"
#include "tokenizer.h"
#include "throw.h"
#include "zstreambuf.h"
#include "where.h"
using namespace std;

enum {
	PRINT_ALL=0,
	PRINT_MATCHING=1,
	PRINT_UMATCHING=2
	};


class VCFBigBed
	{
	public:
		BigBed* bigBed;
		int chromCol;
		int posCol;
		Tokenizer tokenizer;
		int extend;
		int mode;
		int limit;
		std::string notFound;
		VCFBigBed():bigBed(NULL),chromCol(0),posCol(1),
				extend(0),mode(PRINT_ALL),limit(0),notFound("!N/A")
			{
			}

		~VCFBigBed()
			{
			if(bigBed!=NULL) delete bigBed;
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
							<< tokenizer.delim << "BigBed:chromStart"
							<< tokenizer.delim << "BigBed:chromEnd"
							<< tokenizer.delim << "BigBed:rest"
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
				bool found=false;
				auto_ptr<vector<BigBed::Interval> > v= bigBed->query(
						chrom,
						chromStart,
						chromEnd,
						limit
						);
				if(v.get()!=NULL)
					{
					for(size_t i=0;i< v->size();i++)
						{
						found=true;
						const BigBed::Interval& bed=v->at(i);
						if(mode!=PRINT_UMATCHING)
							{
							cout << line
										<< tokenizer.delim << bed.start
										<< tokenizer.delim << bed.end
										<< tokenizer.delim << bed.line
										<< endl;
							}
						else
							{
							break;
							}
						}
					}

				if(!found && mode!=PRINT_MATCHING)
					{
					cout << line << tokenizer.delim << notFound
								<< tokenizer.delim << notFound
								<< tokenizer.delim << notFound
								<< endl;
					}
				}
		    }
		void usage(int argc,char** argv)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Usage" << endl
				<< "   "<< argv[0]<< " [options] -f file.bwig (file|stdin)"<< endl;
			cerr << "Options:\n";
			cerr << "  -f <BigBed file>" << endl;
			cerr << "  -d <delimiter> (default:tab)" << endl;
			cerr << "  -c <CHROM column=int> (default:"<< (chromCol+1)<< ")" << endl;
			cerr << "  -p <POS column=int> (default:"<< (posCol+1)<< ")" << endl;
			cerr << "  -x <extend=int> extends window size (default:"<< (extend)<< ")" << endl;
			cerr << "  -L <limit=int> limit to L records in bed (default:unbound)" << endl;
			cout << "  -S <NOT-FOUND-String> default:"<< notFound << ".\n";
			cout << "  -m  <int=mode> "<< PRINT_ALL <<">=all "<< PRINT_MATCHING<<
								":only-matching  "<< PRINT_UMATCHING <<":only-non-matching default:"<< mode << ".\n";

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
	VCFBigBed app;
	char* BigBedFile=NULL;
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
   			BigBedFile=argv[++optind];
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
   		else if(strcmp(argv[optind],"-L")==0 && optind+1< argc)
			{
			char* p2=NULL;
			app.limit=(int)strtol(argv[++optind],&p2,10);
			if(app.limit<0)
				{
				cerr << "Bad limit: "<< argv[optind] << endl;
				app.usage(argc,argv);
				return EXIT_FAILURE;
				}
			}
   		else if(strcmp(argv[optind],"-m")==0 && optind+1< argc)
			{
			char* p2=NULL;
			app.mode=(int)strtol(argv[++optind],&p2,10);
			if(app.mode<0 ||  app.mode>2)
				{
				cerr << "Bad mode: "<< argv[optind] << endl;
				app.usage(argc,argv);
				return EXIT_FAILURE;
				}
			}
		else if(strcmp(argv[optind],"-S")==0 && optind+1< argc)
			{
			app.notFound.assign(argv[++optind]);
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

   if(BigBedFile==NULL)
   	   {
	   cerr << "Undefined BigBed file."<< endl;
	   app.usage(argc,argv);
	   return (EXIT_FAILURE);
   	   }
   app.bigBed=new BigBed(BigBedFile);
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
