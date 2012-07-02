/*
 * redonstructvar.cpp
 *
 *      Author: lindenb
 */
#include <string>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <vector>
#include <cmath>
#include "where.h"
#include "lzw.h"
#include "bam.h"
#include "sam.h"
#include "throw.h"
using namespace std;

#define IS_FLAG_SET(b,flg) (((b)->core.flag & (flg))!=0)

class BamLZW1
    {
    public:
	class Count
		{
		public:
			std::string example;
			uint32_t mapped;
			uint32_t unmapped;
			Count():mapped(0UL),unmapped(0UL) {}
			Count(const Count& cp):example(cp.example),mapped(cp.mapped),unmapped(cp.unmapped) {}
			~Count() {}
			Count& operator=(const Count& cp)
				{
				if(this!=&cp)
					{
					example.assign(cp.example);
					mapped=cp.mapped;
					unmapped=cp.unmapped;
					}
				return *this;
				}
			
		};
	std::vector<Count> complexity2count;
	LZWComplexity lzw;
	samfile_t* in; 


	BamLZW1():in(0)
	    {
	    }

	~BamLZW1()
	    {
	    }

	void dump()
		{
		cout << "#complexity\tmapped\tunmapped\tsample" << endl;
		for(size_t i=0;i< complexity2count.size();++i)
			{
			Count& count=complexity2count.at(i);
			if(count.example.empty()) continue;
			
			cout	<< i
				<< "\t"
				<< count.mapped
				<< "\t"
				<< count.unmapped
				<< "\t"
				<< count.example
				<< endl
				;
			}
		}


	/** main loop */
	void run(const char* filename)
	    	{
	    	Count zero;
	    	int prev_tid=-1;
		bam1_t *b=NULL;
		this->in = samopen(filename==0?"-":filename, "rb", 0);
 		if(this->in==0)
			{
			THROW("Cannot open " << filename);
			}
 		b = bam_init1();
 		std::string seq;
 		seq.reserve(1000);
 		size_t n_reads=0;
		while(samread(this->in, b) > 0) /* loop over the records */
			{
			const bam1_core_t *c = &b->core;
			if(c->l_qseq<=0) continue;
			uint8_t *s = bam1_seq(b);
			seq.clear();
			
			for (int i = 0; i < c->l_qseq; ++i)
			 	{
			 	seq+= bam_nt16_rev_table[bam1_seqi(s, i)];
			 	}
			size_t complexity= lzw.complexity(seq.data(),seq.size());
			if(complexity>=complexity2count.size())
				{
				complexity2count.resize(complexity+1,zero);
				}
			Count& count=complexity2count.at(complexity);
			
			
			if(count.example.empty())
				{
				clog << "[LOG] complexity:" << complexity <<"="<< seq  << endl;
				count.example.assign(seq);
				}
			if(IS_FLAG_SET(b,BAM_FUNMAP))
				{
				count.unmapped++;
				if(count.unmapped % 1000000 == 0)
					{
					clog << "[LOG] unmapped [" << complexity <<"]="<< count.unmapped  << endl;
					}
				}
			else
				{
				if(prev_tid!=c->tid)
					{
					clog << "[LOG] chromosome: " << in->header->target_name[c->tid] << " "  << endl;
					prev_tid=c->tid;
					}
				count.mapped++;
				if(count.mapped % 1000000 == 0)
					{
					clog << "[LOG] mapped [" << complexity <<"]="<<count.mapped  << endl;
					}
				}
			if(++n_reads % 1000000 ==0)
				{
				clog << "[LOG] nReads= " << n_reads << endl;
				}
			}
		bam_destroy1(b);
		samclose(this->in);
	    }


	void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] bam1.bam bam2.bam bam3.bam ..."<< endl;
	    out << "\nOptions:\n";
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return(EXIT_FAILURE);
		    }
		else if(strcmp(argv[optind],"--")==0)
		    {
		    ++optind;
		    break;
		    }
		else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '" << argv[optind]<< "'" << endl;
		    this->usage(cerr,argc,argv);
		    return EXIT_FAILURE;
		    }
		else
		    {
		    break;
		    }
		++optind;
		}


	    if(optind==argc)
		{
		run(NULL);
		}
	    else
		{
		while(optind< argc)
		    {
		    char* bamfile=argv[optind++];
		    run(bamfile);
		    }
		}

	    dump();
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    BamLZW1 app;
    return app.main(argc,argv);
    }
