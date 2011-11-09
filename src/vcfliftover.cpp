/*

Author:
	Pierre Lindenbaum PhD
WWW:
	http://plindenbaum.blogspot.com
Contact:
	plindenbaum@yahoo.fr

*/
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <zlib.h>
#include <cerrno>
#include "application.h"
#include "xliftOver.h"
#include "zstreambuf.h"
#include "where.h"
using namespace std;


class VcfLiftOver:public AbstractApplication
    {
    public:
	LiftOver* liftOver;
	int chromCol;
	int posCol;
	bool oneBased;

	VcfLiftOver():liftOver(NULL)
	    {
	    chromCol=0;
	    posCol=1;
	    oneBased=true;
	    }
	~VcfLiftOver()
	    {
	    if(liftOver!=NULL) delete liftOver;
	    }


	void run(std::istream& in)
	    {
	    vector<string> tokens;
	    string line;
	    int shift=(oneBased?1:0);
	    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			tokenizer.split(line,tokens);
			if(line[0]=='#')
				{
				if(line.size()>1 && line[1]=='#')
					{
					cout << line << endl;
					continue;
					}
				cout << line ;
				cout << tokenizer.delim << "lift.chrom";
				cout << tokenizer.delim << "lift.start";
				cout << tokenizer.delim << "lift.end";
				cout << tokenizer.delim << "lift.error";
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
			int chromStart=(int)strtol(tokens[posCol].c_str(),&p2,10);
			if(*p2!=0)
				{
				cerr << "BAD POS  in " << line << endl;
				continue;
				}

			ChromStartEnd segment(tokens[chromCol],chromStart-shift,chromStart);
			std::auto_ptr<ChromStartEnd> seg = this->liftOver->convert(&segment);
			if(seg.get()==NULL)
				{
				const char* err=  this->liftOver->lastError();
				cout << line ;
				cout << tokenizer.delim << ".";
				cout << tokenizer.delim << ".";
				cout << tokenizer.delim << ".";
				cout << tokenizer.delim << (err==NULL?"#ERR":err);
				cout << endl;
				}
			else
				{
				cout << line ;
				cout << tokenizer.delim << seg->chrom;
				cout << tokenizer.delim << (seg->start+shift);
				cout << tokenizer.delim << (seg->end+shift);
				cout << tokenizer.delim << ".";
				cout << endl;
				}
			}
	    }
	void usage(std::ostream& out,int argc,char** argv)
		{
		out << endl;
		out << argv[0] <<" Author: Pierre Lindenbaum PHD. 2011.\n";
		out << "Last compilation:"<<__DATE__<<" " << __TIME__ << "\n";
		out << VARKIT_REVISION << endl;
		out << "Usage: "<<argv[0]<<" (options) {stdin|file|gzfiles}:\n";
		out << "  -d <char> column delimiter. default: TAB\n";
		out << "  -c <int> chromosome column ("<< (chromCol+1) << ").\n";
		out << "  -p <int> pos column ("<< (posCol+1) << ").\n";
		out << "  -1 data are NOT +1 based.\n";
		out << "Lift over parameters:\n";
		out << "  -f (path) liftOver map file (required).\n";
		out << "  -b (double) liftOver minblocks.\n";
		out << "  -m (double) liftOver minMatch.\n";
		out << endl;
		out << endl;
		}

	int main(int argc, char *argv[])
	    {
		char* minBlocks=NULL;
		char* minMatch=NULL;
	    char* mapFile=NULL;
	    int optind=1;
	    /* loop over the arguments */
	    while(optind<argc)
			{
			if(strcmp(argv[optind],"-h")==0)
				{
				usage(cerr,argc,argv);
				return EXIT_SUCCESS;
				}
			else if(strcmp(argv[optind],"-1")==0)
				{
				oneBased=false;
				}
			else if(strcmp(argv[optind],"-f")==0 && optind+1< argc)
				{
				mapFile=argv[++optind];
				}
			else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
				{
				if(strlen(argv[optind+1])!=1)
					{
					fprintf(stderr,"Expected only one char for the delimiter.\n");
					return EXIT_FAILURE;
					}
				tokenizer.delim=argv[++optind][0];
				}
			else if(strcmp(argv[optind],"-c")==0 && optind+1< argc)
				{
				char* p2=NULL;
				chromCol=strtol(argv[++optind],&p2,10);
				if(chromCol<1 ||  *p2!=0)
					{
					cerr << "Bad column for CHROM: "<< argv[optind] << endl;
					usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				chromCol--;
				}
			else if(strcmp(argv[optind],"-p")==0 && optind+1< argc)
				{
				char* p2=NULL;
				posCol=strtol(argv[++optind],&p2,10);
				if(posCol<1 ||  *p2!=0)
					{
					cerr << "Bad column for POS: "<< argv[optind] << endl;
					usage(cerr,argc,argv);
					return EXIT_FAILURE;
					}
				posCol--;
				}
			else if(strcmp(argv[optind],"-b")==0 && optind+1< argc)
				{
				minBlocks=argv[++optind];
				}
			else if(strcmp(argv[optind],"-m")==0 && optind+1< argc)
				{
				minMatch=argv[++optind];
				}
			else if(strcmp(argv[optind],"--")==0)
				{
				optind++;
				break;
				}
			else if(argv[optind][0]=='-')
				{
				cerr << "Unknown option: "<< argv[optind]<< "\n";
				usage(cerr,argc,argv);
				return EXIT_FAILURE;
				}
			else
				{
				break;
				}
			++optind;
			}
	    if(mapFile==NULL)
			{
			cerr << "Error: undefined map file file.\n" << endl;
			usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}



	    liftOver=new LiftOver(mapFile);

	    if(minMatch!=NULL)
	    	{
	    	char* p2;
	    	liftOver->minMatch(strtod(minMatch,&p2));
	    	if(*p2!=0)
	    		{
	    		cerr << "Error with param 'minMatch' "<< minMatch << endl;
	    		return EXIT_FAILURE;
	    		}
	    	}

	    if(minBlocks!=NULL)
			{
			char* p2;
			liftOver->minBlocks(strtod(minBlocks,&p2));
			if(*p2!=0)
				{
				cerr << "Error with param 'minBlocks' "<< minBlocks << endl;
				return EXIT_FAILURE;
				}
			}

	    if(optind==argc)
			{
			igzstreambuf buf;
			istream in(&buf);
			this->run(in);
			buf.close();
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
	    /* we're done */
	    return EXIT_SUCCESS;
	    }

    };

int main(int argc, char *argv[])
    {
    VcfLiftOver app;
    return app.main(argc,argv);
    }
