/*
 * cmpdepth.cpp
 *
 *  Created on: Jan 6, 2012
 *      Author: lindenb
 */
#include <algorithm>
#include <iostream>
#include <memory>
#include "xbam2.h"
#include "auto_vector.h"
#include "throw.h"

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}
using namespace std;

class CompareDepth
    {
    private:
	vector<int64_t> depths;
	double total_depth;
	size_t current_bam_index;

    public:

	auto_vector<BamFile2> bamFiles;
	int32_t window_size;
	int32_t window_shift;
	int32_t minimum_mean_depth;
	double ratio;

	CompareDepth():window_size(50),window_shift(25),minimum_mean_depth(20),ratio(2)
	    {
	    }

	virtual ~CompareDepth()
	    {
	    }



	static int  pileup_callback(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    CompareDepth* app=(CompareDepth*)data;
	    app->depths[app->current_bam_index]+=depth;
	    app->total_depth+=depth;
	    return 0;
	    }

	void run()
	    {
	    cout << "#CHROM\tSTART\tEND";
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		cout << "\t" << bamFiles[i]->path();
		bamFiles[i]->open();
		}
	    cout << "\tmean.depth\tflag" << endl;

	    depths.resize(bamFiles.size(),0);
	    BamFile2* first=bamFiles.front();
	    for(int32_t tid=0; tid< first->count_targets();++tid)
		{
		const char* chrom_name= first->target_name(tid);
		if(strcmp("X",chrom_name)!=0) continue;//TODO change this
		for(int32_t n=0;
			n + this->window_size < first->target_length(tid);n+=this->window_shift)
		    {
		    if(n<154182200) continue;//TODO change this
		    total_depth=0.0;
		    fill(depths.begin(),depths.end(),0);
		    for(current_bam_index=0;current_bam_index < bamFiles.size();++current_bam_index)
			{
			::bam_plbuf_t *buf= ::bam_plbuf_init( CompareDepth::pileup_callback,this);
			if(buf==NULL) THROW("boum");
			::bam_fetch(bamFiles[current_bam_index]->bamPtr(), bamFiles[current_bam_index]->bamIndex(), tid, n, n+this->window_shift, buf, BamFile2::fetch_func);
			::bam_plbuf_push(0, buf); // finalize pileup
			::bam_plbuf_destroy(buf);
			}

		    double mean=((total_depth/this->window_size)/bamFiles.size());
		    if(mean< minimum_mean_depth) continue;

		    cout << chrom_name << "\t" << n << "\t" << (n+this->window_size);
		    for(current_bam_index=0;current_bam_index < bamFiles.size();++current_bam_index)
			    {
			    cout << "\t" <<  (int32_t)(depths[current_bam_index]/(double)this->window_size);
			    }

		    cout << "\t"<< (int32_t)mean << "\t";
		    bool found=false;
		    for(size_t i=0;
			    i < bamFiles.size();
			    ++i
			    )
			{
			double d1= depths[i]/(double)this->window_size;
			double d2=0;
			for(size_t j=0;
			    j < bamFiles.size();
			    ++j
			    )
			    {
			    if(i==j) continue;
			    d2+= depths[j]/(double)this->window_size;
			    }
			d2/=(double)(depths.size()-1);
			if(d1< d2/ratio ||d1>d2*ratio)
			    {
			    if(found) cout << ";";
			    cout << "$" << (i+1);
			    found=true;
			    }
			}
		    if(!found)
			{
			cout << ".";
			}
		    cout << endl;
		    }
		}

	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}
	    }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] (bams)"<< endl;
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


		while(optind< argc)
		    {
		    this->bamFiles.push_back(new BamFile2(argv[optind++]));
		    }

	    if(bamFiles.size()<2)
		{
		cerr << "expected at least 2 BAMs. buy got " << (argc-optind) << endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	    run();
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    CompareDepth app;
    return app.main(argc,argv);
    }
