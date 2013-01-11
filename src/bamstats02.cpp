/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com

 * Motivation:
 *	stats for BAM: number of indels per reads

 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <memory>
#include <limits>
#include <vector>
#include <stdint.h>
#include "sam.h"
#include "bam1sequence.h"
#include "throw.h"
#include "numeric_cast.h"

using namespace std;

class BamStats02
	{
	public:
		int min_qual;
		struct Base
			{
			uint64_t match;
			uint64_t mismatch;
			};
		
		BamStats02():min_qual(0)
			{
			}
 		void usage(ostream& out,int argc,char **argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2013.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    out << "Usage:\n\t"<< argv[0] << " [options] (stdin | file1.bam file2.bam ... fileN.bam)\n";
		    out << "\t-q (int) min qual default:"<< min_qual<< endl;
		    }
		    
		void scan(const char* filebam)
			{
			int ret;
			uint64_t total=0UL;
			uint64_t totalinsertion=0UL;
			uint64_t suminsertion=0UL;
			uint64_t totalmismatches=0UL;
			vector<Base> pos2count;
			const Base zero={0UL,0UL};
        		//bam1_core_t *c=&b->core;
			bamFile fp=((filebam==0 || strcmp(filebam, "-")==0) ? ::bam_dopen(fileno(stdin), "r") : ::bam_open(filebam, "r")) ;
			
			if(fp==0)
				{
				THROW("cannot open "<<(filebam==0?"stdin":filebam));
				}
			bam_header_t *header= bam_header_read(fp);
			bam1_t *b=bam_init1();
        		while ((ret = bam_read1(fp, b)) >= 0)
				{
				Bam1Sequence bs(b);
				++total;
				if(bs.quality()<min_qual) continue;
				if(!bs.is_mapped()) continue;
				const char* md=bs.get_aux_MD();
				if(md==0)
					{
					cerr << "#No aux MD:Z:* for " << bs.name() << endl;
					continue;
					}
				char* p=(char*)md;
				uint32_t readpos=0;
				//see also http://davetang.org/muse/2011/01/28/perl-and-sam/
				while(*p!=0)
					{
					if(isdigit(*p))
						{
						int32_t count=0;
						while(isdigit(*p))
							{
							count=count*10 + (*p -'0');
							++p;
							}
						for(int i=0;i< count;++i)
							{
							while(pos2count.size()<= readpos) pos2count.push_back(zero);
							pos2count[readpos].match++;
							++readpos;
							}
						}
					else if(isalpha(*p))
						{
						while(pos2count.size()<= readpos) pos2count.push_back(zero);
						pos2count[readpos].mismatch++;
						++totalmismatches;
						++readpos;
						++p;
						}
					else if(*p=='^')//deletion in read
						{
						++p;
						totalinsertion++;
						if(!isalpha(*p))
							{
							cerr << "Illegal character in MD after ^" << md << endl;
							break;
							}
						while(isalpha(*p))
							{
							suminsertion++;
							++p;
							}
						}
					else
						{
						cerr << "Illegal character in MD " << md << endl;
						break;
						}
					}

				}
			bam_destroy1(b);
			::bam_header_destroy(header);
       			::bam_close(fp);
       			for(size_t i=0;i< pos2count.size();++i)
       				{
       				cout << (i+1) << "\t"
       					<< pos2count[i].match << "\t"
       					<< pos2count[i].mismatch 
       					//<< "\t" << (pos2count[i].mismatch/((double)(pos2count[i].match+pos2count[i].mismatch)))
       					<< endl;
       				}
       			

			}
			
		int main(int argc, char *argv[])
			{
			int optind=1;
			while(optind < argc)
				{
				if(strcmp(argv[optind],"-h")==0)
					{
					usage(cout,argc,argv);
					return EXIT_FAILURE;
					}
				else if(strcmp(argv[optind],"-q")==0 && optind+1<argc)
					{
					if(!numeric_cast<int>(argv[++optind],&(min_qual)) || min_qual < 0) \
						{cerr << "Bad column for -q:" << argv[optind]<< ".\n";return EXIT_FAILURE;}
					}
				else if(strcmp(argv[optind],"--")==0)
					{
					++optind;
					break;
					}
				else if(argv[optind][0]=='-')
					{
					cerr << "unknown option '" << argv[optind]<< endl;
					usage(cerr,argc,argv);
					return(EXIT_FAILURE);
					}
				else
					{
					break;
					}
				++optind;
				}
			
			if(optind==argc)
				{
				scan(0);
				}
			else while(optind<argc)
				{
				scan(argv[optind++]);
				}
			return EXIT_SUCCESS;
			}

	
	};
	
int main(int argc, char *argv[])
    {
    BamStats02 app;
    return app.main(argc,argv);
    }

