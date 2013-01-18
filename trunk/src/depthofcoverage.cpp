/*
 *      Author: lindenb
 */
#include <algorithm>
#include <iostream>
#include <cmath>
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

class DepthOfCoverage
    {
    private:
	uint32_t currStart;
	uint32_t currEnd;
	vector<uint32_t> coverage;
    public:
	auto_vector<BamFile2> bamFiles;
	uint32_t min_qual;
        bool  discard_duplicates;

	DepthOfCoverage():min_qual(0),discard_duplicates(true)
	    {
	    }

	virtual ~DepthOfCoverage()
	    {
	    }


	static int  pileup_callback(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    DepthOfCoverage* stat=(DepthOfCoverage*)data;
	    if(pos < stat->currStart) return 0;
	    if(pos >= stat->currEnd) return 0;
	    for(int i=0;i< depth;++i)
		{
		const bam_pileup1_t* curr=&pl[i];
		if(curr->b->core.qual < stat->min_qual) continue;
		if(stat->discard_duplicates)
			{
			if( ((curr->b->core.flag) & (BAM_FDUP))!=0) continue;
			}
		stat->coverage[pos-stat->currStart]++;
		}
	    return 0;
	    }



	void run(BamFile2* bam,int tid,int32_t chromStart,int32_t chromEnd)
	    {
	    this->currStart=chromStart;
	    this->currEnd=chromEnd;
	    coverage.clear();
	    coverage.resize(chromEnd-chromStart,0);
	    ::bam_plbuf_t *buf= ::bam_plbuf_init( DepthOfCoverage::pileup_callback,this);
	    if(buf==NULL) THROW("bam_plbuf_init failed");
	    ::bam_fetch(
		    bam->bamPtr(),
		    bam->bamIndex(),
		    tid,
		    chromStart,
		    chromEnd,
		    buf,
		    BamFile2::fetch_func
		    );

	    ::bam_plbuf_push(0, buf); // finalize pileup
	    ::bam_plbuf_destroy(buf);
	    double total=0;
	    for(size_t i=0;i< coverage.size();++i) total+=coverage[i];
	    if(!coverage.empty())
		{
		cout << ceil(total/(double)coverage.size());
		}
	    else
		{
		cout << ".";
		}
	    }

	void init_run()
	    {
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->open();
		}
	    }

	void finish_run()
	    {
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}
	    }

	void run(const char* bed_filename)
	    {
	    bool header_printed=false;
	    init_run();
	    vector<string> tokens;
	    string line;
	    Tokenizer tab('\t');
	    igzstreambuf buf(bed_filename);
	    istream in(&buf);
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		tab.split(line,tokens);
		if(tokens.size()<3)
		    {
		    cerr << "Illegal number of tokens in "<< line << endl;
		    continue;
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

		if(!header_printed)
		    {
		    header_printed=true;
		   for(size_t i=0;i< tokens.size();++i)
		       {
		       cout <<(i>0?'\t':'#') << "$"<<(i+1);
		       }
		    for(size_t i=0;i< bamFiles.size();++i)
			{
			cout << "\t" << bamFiles.at(i)->path();
			}
		    cout << endl;
		    }
		cout << line;
		for(size_t i=0;i< bamFiles.size();++i)
		    {
		    int32_t tid=bamFiles.at(i)->findTidByName(seg.chrom.c_str());
		    if(tid==-1)
			{
			cerr << "No chromosome for " << tokens[0]  << " in " << line << endl;
			return;
			}
		    cout << "\t";
		    run(bamFiles.at(i),tid,seg.start,seg.end);
		    }
		cout << endl;
		}
	    buf.close();
	    finish_run();
	    }



	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] (bams)"<< endl;
	    out << "\nOptions:\n";
	    out << " -B (file) limit to that bed file (required)\n";
	    out << " -m (int) min mapping quality default:0\n";
	    out << " -d do NOT dicard reads marked as duplicates." << endl;
            out << endl;
	    out << endl;
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
                else if(strcmp(argv[optind],"-d")==0)
			{
                        this->discard_duplicates=false;
		        }
		else if( (strcmp(argv[optind],"-B")==0 || strcmp(argv[optind],"--bed")==0) && optind+1<argc)
		    {
		    befFile=argv[++optind];
		    }
		else if( (strcmp(argv[optind],"-m")==0) && optind+1<argc)
		    {
		    if(!numeric_cast<uint32_t>(argv[++optind],&min_qual))
			{
			cerr << "bad QUAL in option '" << argv[optind]<< "'" << endl;
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

	    while(optind< argc)
		{
		this->bamFiles.push_back(new BamFile2(argv[optind++]));
		}

	    if(befFile==0)
		{
		cerr << "no Bed file defined"<< endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	    run(befFile);
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    DepthOfCoverage app;
    return app.main(argc,argv);
    }
