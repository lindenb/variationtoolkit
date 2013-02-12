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
#include <vector>
#include <limits>
#include <stdint.h>
#include <pthread.h>    /* POSIX Threads */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include "sam.h"
#include "bedindex.h"
#include "bam1sequence.h"
#include "throw.h"

using namespace std;

class BamStats
	{
	public: 
		struct Parallel
			{
			BamStats* owner;
			char* filebam;
			pthread_t thread;
			uint64_t total;
			uint64_t totalq30;
			uint64_t totalmappeddup;
			uint64_t totalmapped;
			uint64_t totalmappedproperpair;
			uint64_t totalmappedbed;
			uint64_t totalmappedbedq30;
			uint64_t totalmappedbeddup;
			};
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
		    out << " -t <number of threads> default: 1.\n";
		    }
		    
		    
		    
		static void* scan(void* ptr)
			{
			Parallel* p=(Parallel*)ptr;
			int ret;
			
			
        		//bam1_core_t *c=&b->core;
			bamFile fp=((p->filebam==0 || strcmp(p->filebam, "-")==0) ? ::bam_dopen(fileno(stdin), "r") : ::bam_open(p->filebam, "r")) ;
			
			if(fp==0)
				{
				THROW("cannot open "<<(p->filebam==0?"stdin":p->filebam));
				}
			bam_header_t *header= bam_header_read(fp);
			bam1_t *b=bam_init1();
        		while ((ret = bam_read1(fp, b)) >= 0)
				{
				Bam1Sequence bs(b);
				p->total++;
				if(bs.quality()>30) p->totalq30++;
				if(bs.is_mapped())
					{
					p->totalmapped++;
					if(bs.is_proper_pair())
						{
						p->totalmappedproperpair++;
						}
					if(bs.is_duplicate())
						{
						p->totalmappeddup++;
						}
						
					if(p->owner->bedindex.get()!=0 &&
						p->owner->bedindex->overlap(bs.chromosome(header),bs.pos(), bs.end())
						)
						{
						p->totalmappedbed++;
						if(bs.quality()>30) p->totalmappedbedq30++;
						if(bs.is_duplicate())
							{
							p->totalmappedbeddup++;
							}
						}
					}
				}
			bam_destroy1(b);
			::bam_header_destroy(header);
       			::bam_close(fp);
			return NULL;
			}
			
		void print(const Parallel* p)
			{
			cout << (p->filebam==0?"stdin":p->filebam) 
				<< "\t" << p->total
				<< "\t" << p->totalq30
				<< "\t" << p->totalmapped
				<< "\t" << p->totalmappedproperpair
				<< "\t" << p->totalmappeddup
				;
			if(bedindex.get()!=0 )
				{
				cout
					<< "\t" << p->totalmappedbed
					<< "\t" << p->totalmappedbedq30
					<< "\t" << p->totalmappedbeddup
					;
				}
			cout	<< endl;
			}	
			
		int main(int argc, char *argv[])
			{
			int nthreads=1;
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
				else if(strcmp(argv[optind],"-t")==0 && optind+1<argc)
					{
					nthreads= atoi(argv[++optind]);
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
			int count_files=argc-optind;
			if(nthreads>count_files) nthreads=count_files;
			if(nthreads<=0) nthreads=1;
			
			
			
			
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
			if(count_files==0)
				{
				Parallel p;
				memset((void*)&p,0,sizeof(Parallel));
				p.owner=this;
				p.filebam=0;
				scan(&p);
				print(&p);
				}
			else	{
				vector<Parallel*> threads;
				while(optind<argc)
					{
					while(optind<argc && (int)threads.size() < nthreads)
						{
						Parallel* p=new Parallel;
						memset((void*)p,0,sizeof(Parallel));
						p->owner=this;
						p->filebam=argv[optind++];
						threads.push_back(p);
						}
					if(threads.size()==1)
						{
						scan(threads[0]);
						}
					else
						{
						for(size_t i=0;i< threads.size();++i)
							{
							if(::pthread_create(
								&(threads[i]->thread),
								NULL,
								&BamStats::scan,
								(void *)threads[i]
								)!=0)
								{
								cerr << "bamstats01: Cannot create thread." << endl;
								exit(EXIT_FAILURE);
								}
							}
						for(size_t i=0;i< threads.size();++i)
							{
							 pthread_join(threads[i]->thread, NULL);
							}
						}
					for(size_t i=0;i< threads.size();++i)
						{
						print(threads[i]);
						delete threads[i];
						}
					threads.clear();
					}
				}
			return EXIT_SUCCESS;
			}

	
	};
	
int main(int argc, char *argv[])
    {
    BamStats app;
    return app.main(argc,argv);
    }

