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
 *	normlize chrom column to/from UCSC/ENSEMBL
 * Compilation:
 *	 g++ -o normalizechrom -Wall -O3 normalizechrom.cpp -lz
 */
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <iostream>
#include <zlib.h>
using namespace std;

class FastaHead
	    {
	    public:
		  int32_t count;

		  FastaHead():count(10)
		    {
		    
		    }
		    	


		  void usage(ostream& out,int argc,char** argv)
			{
			  out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			  out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			  out << "Usage" << endl
				    << "   "<< argv[0]<< " [options] -f genome.fa (files|stdin)"<< endl;
			  out << "Options:\n";
			  out << "  -n <count> ("<<  (count) << ")" << endl;
			  out << endl;
			}

		  void run(gzFile in)
		      {
		      int c;
		      int32_t index=0;
		      while((c=gzgetc(in))!=EOF)
			  {
			  if(c=='>')
			      {
			      index++;
			      if(index>=this.count) break;
			      while((c=gzgetc(in))!=EOF )
				  {
				  fputc(c,stdout);
				  if(c=='\n')break;
				  }
			      continue;
			      }
			  fputc(c,stdout);
			  }
		      }


		 int main(int argc,char** argv)
		    {
		    int optind=1;
		    while(optind < argc)
			{
			if(strcmp(argv[optind],"-h")==0)
				{
				usage(cout,argc,argv);
				exit(EXIT_FAILURE);
				}
			else if(strcmp(argv[optind],"-n")==0 && optind+1<argc)
				{
				char* p2;
				this->count=(int)strtol(argv[++optind],&p2,10);
				}
			else if(strcmp(argv[optind],"--")==0)
				{
				++optind;
				break;
				}
			 else if(argv[optind][0]=='-')
				{
				cerr << "unknown option '" << argv[optind]<< "'" << endl;
				exit(EXIT_FAILURE);
				}
			else
			    {
			    break;
			    }
			++optind;
			}
		    if(optind==argc)
			    {
			    gzFile in=gzdopen(fileno(stdin),"r");
			    run(in);
			    gzclose(in);
			    }
		    else
			    {
			    while(optind< argc)
				    {
				    gzFile in=gzdopen(argv[optind++],"r");
				    run(in);
				    gzclose(in);
				    ++optind;
				    }
			    }
		    return EXIT_SUCCESS;
		    }
	    };

int main(int argc,char** argv)
    {
    FastaHead app;
    return app.main(argc,argv);
    }
