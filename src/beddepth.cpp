/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <limits>
#include <fstream>
#include <algorithm>
#include <set>
#include <stdint.h>
#include "tokenizer.h"
#include "xbam2.h"
#include "auto_vector.h"
#include "throw.h"
#include "numeric_cast.h"

using namespace std;

class BedDepth
    {
    public:
	class Shuttle
	    {
	    public:
		BedDepth* owner;
		int32_t chromStart;
		int32_t chromEnd;
		vector<int> depth;
	    };
	auto_vector<BamFile2> bamFiles;
	auto_ptr<uint32_t> max_qual;
	auto_ptr<uint32_t> min_qual;
	set<uint32_t> min_depth;
	bool discard_dup;

	BedDepth():max_qual(0),min_qual(0),discard_dup(true)
	    {
	    this->min_depth.insert(0);
	    }

	virtual ~BedDepth()
	    {

	    }



    // callback for bam_plbuf_init()
    static int  scan_bed_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	{
	Shuttle* shuttle=(Shuttle*)data;
	if((int32_t)pos< shuttle->chromStart || (int32_t)pos>=shuttle->chromEnd) return 0;
	int coverage=0;
	for(int d=0;d < depth;++d)
	    {
	   const bam_pileup1_t* curr=&pl[d];
	   const bam1_t* b =curr->b;
	   uint32_t qual=b->core.qual;

	   if(shuttle->owner->min_qual.get()!=0 && qual< *(shuttle->owner->min_qual) )
		{
		continue;
		}
	   if(shuttle->owner->max_qual.get()!=0 && qual> *(shuttle->owner->max_qual) )
		{
		continue;
		}
	    if(shuttle->owner->discard_dup)
	    	{
	    	if( ((curr->b->core.flag) & (BAM_FDUP))!=0) continue;
	    	}
	    coverage++;
	    }
	shuttle->depth.push_back(coverage);
	return 0;
	}


void cacl_depth(std::ostream& out,BamFile2* bf,int tid,int chromStart,int chromEnd)
    {
    Shuttle shuttle;
    shuttle.owner=this;
    shuttle.chromStart=chromStart;
    shuttle.chromEnd=chromEnd;
    shuttle.depth.reserve(1+chromEnd-chromStart);
    bam_plbuf_t *buf; buf = bam_plbuf_init( scan_bed_func,(void*)&shuttle); // initialize pileup
    bam_fetch(bf->bamPtr(), bf->bamIndex(), tid,chromStart,chromEnd, buf, BamFile2::fetch_func);
    bam_plbuf_push(0, buf); // finalize pileup
    while((int)shuttle.depth.size()<  (chromEnd-chromStart))
    	{
    	//cerr << "err" << shuttle.depth.size() << " "<< (chromEnd-chromStart) << endl;
    	shuttle.depth.push_back(0);
    	}
    	
    	
	std::sort(shuttle.depth.begin(),shuttle.depth.end());
	if(shuttle.depth.empty())
		{
		out << "0\t0\t0\t0\t0";
		for(set<uint32_t>::iterator r=min_depth.begin();r!=min_depth.end();++r) out << "\t0\t0";
		return;
		}
	double total=0;
	for(size_t i=0;i< shuttle.depth.size();++i)
		{
		total+=shuttle.depth[i];
		}
	
	out << shuttle.depth.size() << "\t";
	for(set<uint32_t>::iterator r=min_depth.begin();
		r!=min_depth.end();
		++r)
		{
		int covered=0;
		for(size_t i=0;i< shuttle.depth.size();++i)
			{
			if(shuttle.depth[i]>=(int)(*r)) covered++;
			}
		
		out << covered << "\t"
			<< covered/(double)shuttle.depth.size() << "\t"
			;
		}
 	out << shuttle.depth.front() << "\t"
		<< shuttle.depth.back() << "\t"
		<< shuttle.depth[shuttle.depth.size()/2] << "\t"
		<< total/shuttle.depth.size()
		;
	
    }

void run(std::istream& in)
     {
     Tokenizer tab;
     string line;
     vector<string> tokens;

    while(getline(in,line,'\n'))
	{
	if(line.empty()) continue;
	if(line[0]=='#') continue;
	tab.split(line,tokens);
	if(tokens.size()<3)
	    {
	    cerr << "Bad bed line "<< line << endl;
	    continue;
	    }
	cout << tokens[0] << "\t" << tokens[1] << "\t" << tokens[2];
	for(size_t i=0;i< this->bamFiles.size();++i)
	    {
	    cout << "\t";
	    BamFile2* bf= this->bamFiles.at(i);
	    int tid;
	    int chromStart;
	    int chromEnd;
	    if( ::numeric_cast<int>(tokens[1].c_str(),&chromStart) &&
		::numeric_cast<int>(tokens[2].c_str(),&chromEnd) &&
		 chromStart<=chromEnd &&
		(tid=bf->findTidByName(tokens[0].c_str()))>=0
		)
		{
		cacl_depth(cout,bf,tid,chromStart,chromEnd);
		}
	    else
		{
		cout << "ERROR\tERROR\tERROR\tERROR\tERROR\tERROR\tERROR";
		}
	    }

	cout << endl;
	}
     }

 void usage(ostream& out,int argc,char **argv)
	{
	out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	out << "Usage:\n\t"<< argv[0] << " [options]  (bed.file|stdin)\n";
	out << "Options:\n";
	out << " -f <bam-file> add this bam file. Can be called multiple times\n";
	out << " -m <min-qual uint32> (optional) min SAM record Quality.\n";
	out << " -M <max-qual uint32> (optional) max SAM record Quality.\n";
	out << " -D <min-depth> (optional) min depth.  Can be called multiple times\n";
	out << " -d do NOT discard duplicates" << endl;
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
		else if(strcmp(argv[optind],"-d")==0)
			{
			this->discard_dup=false;
			}
		else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			BamFile2* bf=new BamFile2(argv[++optind]);
			this->bamFiles.push_back(bf);
			}
		else if(strcmp(argv[optind],"-m")==0 && optind+1<argc)
			{
			uint32_t q;
			if(!numeric_cast<uint32_t>(argv[++optind],&q))
			    {
			    cerr << "Bad value for -m "<< argv[optind] << endl;
			    usage(cerr,argc,argv);
			    return EXIT_FAILURE;
			    }
			this->min_qual.reset(new uint32_t(q));
			}
		else if(strcmp(argv[optind],"-M")==0 && optind+1<argc)
		    {
		    uint32_t q;
		    if(!numeric_cast<uint32_t>(argv[++optind],&q))
			{
			cerr << "Bad value for -M "<< argv[optind] << endl;
			usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		    this->max_qual.reset(new uint32_t(q));
		    }
		else if(strcmp(argv[optind],"-D")==0 && optind+1<argc)
		    {
		    uint32_t q;
		    if(!numeric_cast<uint32_t>(argv[++optind],&q))
			{
			cerr << "Bad value for -D "<< argv[optind] << endl;
			usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		    this->min_depth.insert(q);
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
        if(this->bamFiles.empty())
            {
            cerr << "No bam defined.\n";
            return(EXIT_FAILURE);
            }
 	cout << "#chrom\tstart\tend";

		
#define HEADER(a) cout << "\t" << a << "(" << this->bamFiles.at(i)->path() << ")"
		for(size_t i=0;i< this->bamFiles.size();++i)
			{
			HEADER("size");
			for(set<uint32_t>::iterator r=min_depth.begin();
                	   r!=min_depth.end();
               		   ++r)
				{
				 HEADER("covered[depth:"<< (*r)<<"]");
                       		 HEADER("percent_covered[depth:"<< (*r)<<"]");
				}
			HEADER("min");
			HEADER("max");
			HEADER("median");
			HEADER("mean");
			
			BamFile2* bf= this->bamFiles.at(i);
			bf->open();
			if(!bf->is_open())
				{
				cerr << "cannot open '" << bf->path()<< endl;
				return(EXIT_FAILURE);
				}
			}
		
	cout << endl;
        if(optind==argc)
		{
		run(cin);
		}
        else
		{
		while(optind<argc)
		    {
		    fstream in(argv[optind],ios::in);
		    if(!in.is_open())
			{
			cerr << "Cannot open \"" << argv[optind]<< "\"\n";
			return EXIT_FAILURE;
			}
		    run(in);
		    in.close();
		    optind++;
		    }
		}
	return EXIT_SUCCESS;
	}
    };

int main(int argc, char *argv[])
    {
    BedDepth app;
    return app.main(argc,argv);
    }


