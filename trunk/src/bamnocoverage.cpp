/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	July 2012
 * WWW:
 *	http://plindenbaum.blogspot.com
 */
#include <string>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cassert>
#include "bam.h"
#include "sam.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory>
#include <cmath>
#include "segments.h"
#include "auto_vector.h"
#include "throw.h"
//#define NOWHERE
#include "where.h"
#include "numeric_cast.h"
#include "zstreambuf.h"
#include "tokenizer.h"

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}
using namespace std;

class BamNoCoverage
    {
    public:
	samfile_t *fp_in;
	int32_t min_depth;
	int32_t zero_start;
	int32_t prev_pos;
	int prev_tid;
	bool in_low_coverage;

	BamNoCoverage():fp_in(0),min_depth(0),zero_start(0),prev_pos(-1),prev_tid(-1),in_low_coverage(true)
	    {
	    }

	virtual ~BamNoCoverage()
	    {
	    }

	void echo(int tid,int chromStart,int chromEnd)
		{
		cout	<< fp_in->header->target_name[tid]
			<< "\t"		
			<< chromStart
			<< "\t"		
			<< chromEnd
			<< "\t"
			<< (chromEnd-chromStart)
			<< endl;
		}	
	
	 void end_of_chromosome()
		{
		if(this->prev_tid==-1 ) return;
		int32_t max_len=1+this->fp_in->header->target_len[this->prev_tid];
		if(this->zero_start<max_len)
		    {
		    echo(this->prev_tid,this->zero_start,max_len);
		    }
		this->zero_start=0;
		this->prev_pos=0;
		}

	 void  callback(int32_t tid, int32_t pos,  int32_t depth)
	    {
	    if(this->prev_tid!=tid)
		{
		if(this->prev_tid!=-1)
			{
			this->end_of_chromosome();
			}
		this->prev_tid=tid;
		this->zero_start=0;
		this->prev_pos=-1;
		}

	    if((this->prev_pos+1!=pos ||  depth <= this->min_depth ) && !this->in_low_coverage)
	    	{
		this->in_low_coverage=true;
		this->zero_start=this->prev_pos+1;
	    	}
	    else if(this->in_low_coverage && depth > this->min_depth)
		{
		echo(tid,this->zero_start,pos);
		this->in_low_coverage=false;
		}
	    this->prev_pos=pos;
	    }


	static int  pileup_callback(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    BamNoCoverage* stat=(BamNoCoverage*)data;
	    stat->callback((int32_t)tid,(int32_t)pos,depth);
	    return 0;
	    }


	void run()
	    {
	      this->prev_tid=-1;
	      this->in_low_coverage=true;
	      this->zero_start=0;
	      this->prev_pos=-1;
	     sampileup(this->fp_in, -1, BamNoCoverage::pileup_callback, this);  
	     end_of_chromosome();
	     }


	


	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] (bams)"<< endl;
	    out << "\nOptions:\n";
	    out << " -m (int) min depth default:0\n";
	    out << endl;
	    out << endl;
	    }

	void runbam(const char* f)
		{
 		this->fp_in= samopen(f==NULL?"-":f,"rb",0);
	   	 if(NULL == fp_in)
		      {
		      THROW("Could not open "<<(f==NULL?"<STDIN>":f) << ":" << strerror(errno));
		      }
	         run();
	        samclose(fp_in);
	        fp_in=0;
		}

	int main(int argc,char** argv)
	    {
	    char* befFile=0;
	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return(EXIT_SUCCESS);
		    }
		else if( (strcmp(argv[optind],"-B")==0 || strcmp(argv[optind],"--bed")==0) && optind+1<argc)
		    {
		    befFile=argv[++optind];
		    }
		else if( (strcmp(argv[optind],"-m")==0) && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&min_depth)  || min_depth<0)
			{
			cerr << "bad MIN DEPTH in option '" << argv[optind]<< "'" << endl;
			this->usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
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
			runbam(NULL);
			}
	    else
			{
			while(optind<argc)
				{
				runbam(argv[optind++]);
				}
			}

	   
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    BamNoCoverage app;
    return app.main(argc,argv);
    }
