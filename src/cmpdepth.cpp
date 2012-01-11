/*
 * cmpdepth.cpp
 *
 *  Created on: Jan 6, 2012
 *      Author: lindenb
 */
#include <algorithm>
#include <iostream>
#include <memory>
#include <cmath>
#include "xbam2.h"
#include "segments.h"
#include "auto_vector.h"
#include "throw.h"
#define NOWHERE
#include "where.h"
#include "numeric_cast.h"
#include "zstreambuf.h"
#include "tokenizer.h"

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}
using namespace std;

extern std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s);

class CompareDepth
    {
    private:
	vector<int64_t>* depths_array;

	size_t current_bam_index;
	int32_t buffer_tid;
	int32_t buffer_size;
	int32_t buffer_start;
    public:

	auto_vector<BamFile2> bamFiles;
	int32_t window_size;
	int32_t window_shift;
	int32_t minimum_max_depth;
	double ratio;

	CompareDepth():depths_array(0),
		buffer_tid(-1),
		buffer_size(1000000),
		buffer_start(-1),
		window_size(50),
		window_shift(25),
		minimum_max_depth(0),
		ratio(2)
	    {
	    }

	virtual ~CompareDepth()
	    {
	    }



	static int  pileup_callback(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {

	    CompareDepth* app=(CompareDepth*)data;
	    int32_t index=(int32_t)pos-app->buffer_start;
	    if(index<0 || index>= (int32_t)app->depths_array[app->current_bam_index].size())
		{
		return 0;
		}
	    app->depths_array[app->current_bam_index][index]+=depth;

	    return 0;
	    }

	void refill_buffer(int32_t tid,int32_t start)
	    {

	    buffer_tid=tid;
	    buffer_start=start;
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		depths_array[i].resize(buffer_size,0);
		fill(depths_array[i].begin(),depths_array[i].end(),0);
		}
	    for(    this->current_bam_index=0;
		    this->current_bam_index < bamFiles.size();
		    this->current_bam_index++
		    )
		{
		BamFile2* bam=bamFiles[current_bam_index];
		::bam_plbuf_t *buf= ::bam_plbuf_init( CompareDepth::pileup_callback,this);
		if(buf==NULL) THROW("bam_plbuf_init failed");
		::bam_fetch(
			bam->bamPtr(),
			bam->bamIndex(),
			tid,
			start,
			start+buffer_size,
			buf,
			BamFile2::fetch_func
			);

		::bam_plbuf_push(0, buf); // finalize pileup
		::bam_plbuf_destroy(buf);

		}
	    }


	void run(const char* chrom_name,int tid,int32_t chromStart,int32_t chromEnd)
	    {
	    vector<double> depths(bamFiles.size(),0);
	    for(int32_t n=chromStart;n+window_size<=chromEnd;n+=window_shift)
		{

		if(buffer_tid!=tid ||
		    !(buffer_start<=n && n+window_size <= buffer_start+buffer_size)
		    )
		    {
		    refill_buffer(tid,n);
		    }

		double biggest_shift=0.0;
		double total_depth=0.0;
		double max_depth=0.0;
		fill(depths.begin(),depths.end(),0.0);
		for(size_t i=0;i < bamFiles.size();++i)
		    {
		    double bam_depth=0;
		    for(int32_t x=0;x < this->window_size;++x )
			{
			bam_depth+=depths_array[i].at(n-buffer_start+x);
			}
		    depths[i]=bam_depth;
		    depths[i]/=this->window_size;
		    max_depth=std::max(max_depth,depths[i]);
		    total_depth+=depths[i];
		    }



		if(max_depth< minimum_max_depth)
		    {
		    WHERE(max_depth<< "<"<<minimum_max_depth );
		    continue;
		    }

		cout << chrom_name << "\t" << n << "\t" << (n+this->window_size);
		for(size_t i=0;i < depths.size();++i)
			{
			cout << "\t" <<  (int32_t)(depths[i]);
			}

		double mean=total_depth/bamFiles.size();
		cout << "\t"<< (int32_t)mean << "\t";
		bool found=false;
		for(size_t i=0;
			i < bamFiles.size();
			++i
			)
		    {
		    double d1= depths[i];
		    double d2=0;
		    for(size_t j=0;
			j < bamFiles.size();
			++j
			)
			{
			if(i==j) continue;
			d2+= depths[j];
			biggest_shift=std::max(biggest_shift,abs(depths[j]-d1));
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
		cout << "\t" ;
		cout << (int)biggest_shift ;
		cout << endl;

		}
	    }

	void init_run()
	    {
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->open();
		}
	    depths_array=new vector<int64_t>[bamFiles.size()];
	    }

	void finish_run()
	    {
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}
	    delete [] depths_array;
	    }

	void runBed(const char* fname)
	    {
	    init_run();
	    vector<string> tokens;
	    string line;
	    Tokenizer tab('\t');
	    igzstreambuf buf(fname);
	    istream in(&buf);
	    BamFile2* first=bamFiles.front();
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		tab.split(line,tokens);
		if(tokens.size()<3)
		    {
		    cerr << "Illegal number of tokens in "<< line << endl;
		    continue;
		    }
		int32_t tid=first->findTidByName(tokens[0].c_str());
		if(tid==-1)
		    {
		    cerr << "No chromosome for " << tokens[0]  << " in " << line << endl;
		    return;
		    }
		ChromStartEnd seg(tokens[0].c_str(),0,0);
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&seg.start))
		    {
		    cerr << "Bad start in "<< line << endl;
		    continue;
		    }
		if(!numeric_cast<int32_t>(tokens[2].c_str(),&seg.end))
		    {
		    cerr << "Bad end in "<< line << endl;
		    continue;
		    }

		run(seg.chrom.c_str(),tid,seg.start,seg.end);
		}
	    buf.close();
	    finish_run();
	    }

	void run(const ChromStartEnd* seg)
	    {
	    init_run();
	    BamFile2* first=bamFiles.front();
	    int32_t tid=first->findTidByName(seg->chrom.c_str());
	    if(tid==-1)
		{
		cerr << "No chromosome for " << *(seg) << " in bam header\n";
		return;
		}
	   run(seg->chrom.c_str(),tid,seg->start,seg->end);
	   finish_run();
	   }


	void run()
	    {
	    init_run();
	    BamFile2* first=bamFiles.front();
	    for(int32_t tid=0; tid< first->count_targets();++tid)
		{
		const char* chrom_name= first->target_name(tid);
		run(chrom_name,tid,0, first->target_length(tid));
		}

	    finish_run();
	    }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] (bams)"<< endl;
	    out << "\nOptions:\n";
	    out << " -w (every-bases) window-size default:"<< window_size << "\n";
	    out << " -s (shift-bases) window-shift default:"<< window_shift << "\n";
	    out << " -b (size) buffer-size for each BAM default:"<< buffer_size << "\n";
	    out << " -m (size) minimal maximum depth between samples default:"<< minimum_max_depth << "\n";
	    out << " -r (value) depth treshold default:"<< ratio << "\n";
	    out << " -p (chrom:start-end) limit to that position (optional)\n";
	    out << " -B (file) limit to that bed file (optional)\n";
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    char* befFile=0;
	    char* positionStr=0;
	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return(EXIT_SUCCESS);
		    }
		else if(strcmp(argv[optind],"-B")==0 && optind+1<argc)
		    {
		    befFile=argv[++optind];
		    }
		else if(strcmp(argv[optind],"-p")==0 && optind+1<argc)
		    {
		    positionStr=argv[++optind];
		    }
		else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
		    {
		    befFile=argv[++optind];
		    }
		else if(strcmp(argv[optind],"-r")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<double>(argv[++optind],&ratio) || ratio<=0.0)
			{
			cerr << "Bad option -r "<< argv[optind]<<endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    }
		else if(strcmp(argv[optind],"-m")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&minimum_max_depth) || minimum_max_depth<0)
			{
			cerr << "Bad option -m "<< argv[optind]<<endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    }
		else if(strcmp(argv[optind],"-b")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&buffer_size) || buffer_size<1)
			{
			cerr << "Bad option -b "<< argv[optind]<<endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    }
		else if(strcmp(argv[optind],"-s")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&window_shift) || window_shift<1)
			{
			cerr << "Bad option -s "<< argv[optind]<<endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
			}
		    }
		else if(strcmp(argv[optind],"-w")==0 && optind+1<argc)
		    {
		    if(!numeric_cast<int32_t>(argv[++optind],&window_size) || window_size<1)
			{
			cerr << "Bad option -w "<< argv[optind]<<endl;
			this->usage(cerr,argc,argv);
			return(EXIT_FAILURE);
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
	    if(buffer_size<=window_size)
		{
		cerr << "buffer_size < window_size"<< endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
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
	    cout << "#CHROM\tSTART\tEND";
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		cout << "\t" << bamFiles[i]->path();
		}
	    cout << "\tmean.depth\tflag\tlargest.diff" << endl;
	    if(befFile!=0)
		{
		runBed(befFile);
		}
	    else if(positionStr!=0)
		{
		std::auto_ptr<std::vector<ChromStartEnd> > segs= parseSegments(positionStr);
		if(segs.get()==0)
		    {
		    cerr << "Cannot parse segments in "<< positionStr << endl;
		    this->usage(cerr,argc,argv);
		    return EXIT_FAILURE;
		    }
		for(std::vector<ChromStartEnd>::iterator r=segs->begin();
			r!=segs->end();
			++r)
		    {
		    run(&(*r));
		    }
		}
	    else
		{
		run();
		}
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    CompareDepth app;
    return app.main(argc,argv);
    }
