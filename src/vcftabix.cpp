/*
Motivation:
	Join a file containing some genomic positions and the
	content of a bgz file indexed with tabix
Author:
	Pierre Lindenbaum PhD
WWW:
	http://plindenbaum.blogspot.com
Contact:
	plindenbaum@yahoo.fr
Reference:
	http://plindenbaum.blogspot.com/2011/09/joining-genomic-annotations-files-with.html
Compilation:
	gcc -o jointabix -Wall -O2 -I${TABIXDIR} -L${TABIXDIR} jointabix.c  -ltabix -lz
API:
	http://samtools.sourceforge.net/tabix.shtml

*/
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <zlib.h>
#include <cerrno>
#include "tokenizer.h"
#include "xtabix.h"
#include "zstreambuf.h"

using namespace std;

enum {
	PRINT_ALL=0,
	PRINT_MATCHING=1,
	PRINT_UMATCHING=2
	};

class JoinTabix
    {
    public:
	Tokenizer tokenizer;
	Tabix* tabix;
	int chromCol;
	int posCol;
	int shift;
	string notFound;
	int mode;
	JoinTabix():tabix(NULL),notFound("!N/A"),mode(PRINT_ALL)
	    {
		chromCol=0;
		posCol=1;
		shift=0;
	    }
	~JoinTabix()
	    {
	    if(tabix!=NULL) delete tabix;
	    }


	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			tokenizer.split(line,tokens);
			if(line[0]=='#')
				{
				cout << line ;
				auto_ptr<string> head=tabix->header();
				if(head.get()!=NULL && mode!=PRINT_UMATCHING)
					{
					cout << tokenizer.delim << (*head);
					}
				cout << endl;
				continue;
				}
			if(chromCol>=(int)tokens.size())
				{
				cerr << "CHROM out of range in " << line << endl;
				continue;
				}
			if(posCol>=(int)tokens.size())
				{
				cerr << "POS out of range in " << line << endl;
				continue;
				}
			char* p2;
			bool found=false;
			int chromStart=(int)strtol(tokens[posCol].c_str(),&p2,10);
			if(*p2!=0)
				{
				cerr << "BAD POS  in " << line << endl;
				continue;
				}
			int chromEnd=chromStart+1;
			auto_ptr<Tabix::Cursor>c= tabix->cursor(tokens[chromCol].c_str(),chromStart,chromEnd);
			for(;;)
				{
				const char* s= c->next();
				if(s==NULL) break;
				found=true;
				if(mode!=PRINT_UMATCHING)
					{
					cout << line << tokenizer.delim << s << endl;
					}
				else
					{
					break;
					}
				}
			c.reset();

			if(!found && mode!=PRINT_MATCHING)
				{
				cout << line << tokenizer.delim << notFound << endl;
				}
			}
	    }
	void usage(int argc,char** argv)
		{
		cout << argv[0] <<" Author: Pierre Lindenbaum PHD. 2011.\n";
		cout << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
		cout << "Usage: "<<argv[0]<<" (options) {stdin|file|gzfiles}:\n";
		cout << "  -d <char> column delimiter. default: TAB\n";
		cout << "  -c <int> chromosome column ("<< (chromCol+1) << ").\n";
		cout << "  -p <int> pos column ("<< (posCol+1) << ").\n";
		cout << "  -f <filename> tabix file (required).\n";
		cout << "  -1 remove 1 to the VCF coodinates.\n";
		cout << "  -S <NOT-FOUND-String> default:"<< notFound << ".\n";
		cout << "  -m  <int=mode> 0=all 1:only-matching 2:only-non-matching default:"<< mode << ".\n";
		cout << endl;
		cout << endl;
		}
    };

int main(int argc, char *argv[])
  {

  char* tabixfile=NULL;
  JoinTabix app;
  int optind=1;
  /* loop over the arguments */
  while(optind<argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		    {
	    	app.usage(argc,argv);
		    return EXIT_SUCCESS;
		    }
	    else if(strcmp(argv[optind],"-f")==0 && optind+1< argc)
	    	{
	    	tabixfile=argv[++optind];
	    	}
	    else if(strcmp(argv[optind],"-1")==0)
	    	{
	    	app.shift=-1;
	    	}
	    else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    if(strlen(argv[optind+1])!=1)
		    	{
		    	fprintf(stderr,"Expected only one char for the delimiter.\n");
		    	return EXIT_FAILURE;
		    	}
		    app.tokenizer.delim=argv[++optind][0];
		    }
	    else if(strcmp(argv[optind],"-c")==0 && optind+1< argc)
	    	{
	    	char* p2=NULL;
	    	app.chromCol=strtol(argv[++optind],&p2,10)-1;
	    	if(app.chromCol<0 ||  *p2!=0)
	    		{
	    		cerr << "Bad column for CHROM: "<< argv[optind] << endl;
	    		return EXIT_FAILURE;
	    		}
	    	}
	    else if(strcmp(argv[optind],"-p")==0 && optind+1< argc)
	    	{
	    	char* p2=NULL;
			app.posCol=strtol(argv[++optind],&p2,10)-1;
			if(app.posCol<0 ||  *p2!=0)
				{
				cerr << "Bad column for POS: "<< argv[optind] << endl;
				return EXIT_FAILURE;
				}
	    	}
	    else if(strcmp(argv[optind],"-m")==0 && optind+1< argc)
			{
			char* p2=NULL;
			app.mode=strtol(argv[++optind],&p2,10)-1;
			if(app.mode<PRINT_ALL ||  app.mode>PRINT_UMATCHING)
				{
				cerr << "Bad mode: "<< argv[optind] << endl;
				return EXIT_FAILURE;
				}
			}
	    else if(strcmp(argv[optind],"-N")==0 && optind+1< argc)
	    	{
	    	app.notFound.assign(argv[++optind]);
	    	}
	    else if(strcmp(argv[optind],"--")==0)
		    {
		    optind++;
		    break;
		    }
	    else if(argv[optind][0]=='-')
		    {
		    cerr << "Unknown option: "<< argv[optind]<< "\n";
		    return EXIT_FAILURE;
		    }
	    else
		    {
		    break;
		    }
	    ++optind;
	    }
  if(tabixfile==NULL)
	{
	cerr << "Error: undefined tabix file.\n" << endl;
	app.usage(argc,argv);
	return EXIT_FAILURE;
	}
	    
  if(app.chromCol==app.posCol)
	{
	cerr << "Error: col(chrom)==col(pos)" << endl;
	app.usage(argc,argv);
	return EXIT_FAILURE;
	}

  app.tabix=new Tabix(tabixfile);
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
  /* we're done */
  return EXIT_SUCCESS;
  }
