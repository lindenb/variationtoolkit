/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com

 * Motivation:
 *	stats for BAM

 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <memory>
#include <limits>
#include <stdint.h>
#include "sam.h"
#include "bedindex.h"
#include "bam1sequence.h"
#include "throw.h"

using namespace std;

class BamStats
	{
	public:
		std::auto_ptr<BedIndex> bedindex;
		
		BamStats():bedindex(0)
			{
			}
 		void usage(ostream& out,int argc,char **argv)
		    {
		    out << argv[0] << " Pierre Lindenbaum PHD. 2013.\n";
		    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    out << "Usage:\n\t"<< argv[0] << " [options] (stdin | file1.bam file2.bam ... fileN.bam)\n";
		    out << "Options:\n";
		    out << " -b <bedfile> optional.\n";
		    }
		void scan(const char* filebam)
			{
			int ret;
			uint64_t total=0UL;
			uint64_t totalq30=0UL;
			uint64_t totalmappeddup=0UL;
			uint64_t totalmapped=0UL;
			uint64_t totalmappedproperpair=0UL;
			uint64_t totalmappedbed=0UL;
			uint64_t totalmappedbedq30=0UL;
			uint64_t totalmappedbeddup=0UL;
			
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
				if(bs.quality()>30) totalq30++;
				if(bs.is_mapped())
					{
					++totalmapped;
					if(bs.is_proper_pair())
						{
						++totalmappedproperpair;
						}
					if(bs.is_duplicate())
						{
						++totalmappeddup;
						}
						
					if(bedindex.get()!=0 &&
						bedindex->overlap(bs.chromosome(header),bs.pos()-1, bs.pos())
						)
						{
						++totalmappedbed;
						if(bs.quality()>30) totalmappedbedq30++;
						if(bs.is_duplicate())
							{
							++totalmappedbeddup;
							}
						}
					}
				}
			bam_destroy1(b);
			::bam_header_destroy(header);
       			::bam_close(fp);
			cout << (filebam==0?"stdin":filebam) 
				<< "\t" << total
				<< "\t" << totalq30
				<< "\t" << totalmapped
				<< "\t" << totalmappedproperpair
				<< "\t" << totalmappeddup
				;
			if(bedindex.get()!=0 )
				{
				cout
					<< "\t" << totalmappedbed
					<< "\t" << totalmappedbedq30
					<< "\t" << totalmappedbeddup
					;
				}
			cout	<< endl;
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
				else if(strcmp(argv[optind],"-b")==0 && optind+1<argc)
					{
					this->bedindex= BedIndex::read(argv[++optind]);
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
			cout << "#BAM"
				<< "\t" << "all"
				<< "\t" << "q30"
				<< "\t" << "mapped"
				<< "\t" << "mappedproperpair"
				<< "\t" << "mappeddup"
				;
			if(bedindex.get()!=0 )
				{
				cout
					<< "\t" << "mappedbed"
					<< "\t" << "mappedbedq30"
					<< "\t" << "mappedbeddup"
					;
				}
			cout << endl;
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
    BamStats app;
    return app.main(argc,argv);
    }

