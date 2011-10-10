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

class JoinTabix
    {
    public:
	Tokenizer tokenizer;
	Tabix* tabix;
	JoinTabix():tabix(NULL)
	    {

	    }
	~JoinTabix()
	    {
	    if(tabix!=NULL) delete tabix;
	    }


	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    while(getlin(in,line,'\n'))
		{
		if(line.empty()) continue;
		tokenizer.split(line,tokens);
		if(line[0]=='#')
		    {
		    continue;
		    }

		bool found=false;
		int chromStart=0;
		int chromEnd=0;
		auto_ptr<Tabix::Cursor>c= tabix->cursor(tokens[0].c_str(),chromStart,chromEnd);
		for(;;)
		    {
		    const char* s= c->next();
		    if(s==NULL) break;
		    found=true;
		    cout << line << tokenizer.delim << s << endl;
		    }
		c.reset();





		if(!found)
		    {
		    cout << line << tokenizer.delim << "N/A" << endl;
		    }
		}
	    }
    };

int main(int argc, char *argv[])
  {
  JoinTabix app;
  int optind=1;
  /* loop over the arguments */
  while(optind<argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		    {
		    fprintf(stdout, "Author: Pierre Lindenbaum PHD. 2011.\n");
		    fprintf(stdout, "Last compilation:%s %s\n",__DATE__,__TIME__);
		    fprintf(stdout, "Usage: %s (options) {stdin|file|gzfiles}:\n",argv[0]);
		    fprintf(stdout, "  -d <char> column delimiter. default: TAB\n");
		    fprintf(stdout, "  -c <int> chromosome column (%d).\n",param.chromCol+1);
		    fprintf(stdout, "  -s <int> start column (%d).\n",param.startCol+1);
		    fprintf(stdout, "  -e <int> end column (%d).\n",param.endCol+1);
		    fprintf(stdout, "  -i <char> ignore lines starting with (\'%c\').\n",param.ignore);
		    fprintf(stdout, "  -t <filename> tabix file (required).\n");
		    fprintf(stdout, "  +1 add 1 to the genomic coodinates.\n");
		    fprintf(stdout, "  -1 remove 1 to the genomic coodinates.\n");
		    return EXIT_SUCCESS;
		    }
	    else if(strcmp(argv[optind],"-f")==0 && optind+1< argc)
	    	{
	    	tabixfile=argv[++optind];
	    	}
	    else if(strcmp(argv[optind],"-1")==0)
	    	{
	    	param.shift=-1;
	    	}
	    else if(strcmp(argv[optind],"+1")==0)
	    	{
	    	param.shift=1;
	    	}
	    else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
		    {
		    if(strlen(argv[optind+1])!=1)
		    	{
		    	fprintf(stderr,"Expected only one char for the delimiter.\n");
		    	return EXIT_FAILURE;
		    	}
		    param.delim=argv[++optind][0];
		    }
	    else if(strcmp(argv[optind],"-c")==0 && optind+1< argc)
	    	{
	    	param.chromCol=parseInt1(argv[++optind]);
	    	}
	    else if(strcmp(argv[optind],"-s")==0 && optind+1< argc)
	    	{
	    	param.startCol=parseInt1(argv[++optind]);
	    	}
	    else if(strcmp(argv[optind],"-e")==0 && optind+1< argc)
	    	{
	    	param.endCol=parseInt1(argv[++optind]);
	    	}
	    else if(strcmp(argv[optind],"-i")==0 && optind+1< argc)
	    	{
	    	if(strlen(argv[optind+1])!=1)
		    	{
		    	fprintf(stderr,"Expected only one char for the delimiter.\n");
		    	return EXIT_FAILURE;
		    	}
	    	param.ignore=argv[++optind][0];
	    	}
	    else if(strcmp(argv[optind],"--")==0)
		    {
		    optind++;
		    break;
		    }
	    else if(argv[optind][0]=='-')
		    {
		    fprintf(stderr,"Unnown option: %s\n",argv[optind]);
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
	fprintf(stderr,"Error: undefined tabix file.\n");
	return EXIT_FAILURE;
	}
	    
  if(param.chromCol==param.startCol)
	{
	fprintf(stderr,"Error: col(chrom)==col(start).\n");
	return EXIT_FAILURE;
	}

  if(param.chromCol==param.endCol)
	{
	fprintf(stderr,"Error: col(chrom)==col(end).\n");
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
