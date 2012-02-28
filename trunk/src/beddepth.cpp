/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com
 *	http://samtools.sourceforge.net/
 *	http://samtools.sourceforge.net/sam-exam.shtml
 * Reference:
 *	http://genome.ucsc.edu/goldenPath/help/wiggle.html
 * Motivation:
 *	creates a WIGGLE file from a BAM file.
 * Usage:
 *	bam2wig <bam-file> (<region>)
 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <limits>
#include <fstream>
#include <algorithm>
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

	auto_vector<BamFile2> bamFiles;

	BedDepth()
	    {

	    }

	virtual ~BedDepth()
	    {

	    }

    // callback for bam_fetch()
    static int fetch_func(const bam1_t *b, void *data)
	{
	bam_plbuf_t *buf = (bam_plbuf_t*)data;
	bam_plbuf_push(b, buf);
	return 0;
	}

    // callback for bam_plbuf_init()
    static int  scan_bed_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	{
	vector<int>* param = (vector<int>*)data;
	param->push_back(depth);
	return 0;
	}


void cacl_depth(std::ostream& out,BamFile2* bf,int tid,int chromStart,int chromEnd)
    {
    vector<int> param;
    param.reserve(1+chromEnd-chromStart);
    bam_plbuf_t *buf; buf = bam_plbuf_init( scan_bed_func,(void*)&param); // initialize pileup
    bam_fetch(bf->bamPtr(), bf->bamIndex(), tid,chromStart,chromEnd, buf, fetch_func);
    bam_plbuf_push(0, buf); // finalize pileup
    std::sort(param.begin(),param.end());
    if(param.empty())
	{
	out << "0\t0\t0\t0\t0";
	return;
	}
    double total=0;
    for(size_t i=0;i< param.size();++i) total+=param[i];

    out << param.size() << "\t"
	<< param.front() << "\t"
	<< param.back() << "\t"
	<< param[param.size()/2] << "\t"
	<< total/param.size()
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
	cout << line;
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
		cout << "ERROR\tERROR\tERROR\tERROR\tERROR";
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
		else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			BamFile2* bf=new BamFile2(argv[++optind]);
			this->bamFiles.push_back(bf);
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
        for(size_t i=0;i< this->bamFiles.size();++i)
	    {
	    BamFile2* bf= this->bamFiles.at(i);
	    bf->open();
	    if(!bf->is_open())
		{
		cerr << "cannot open '" << bf->path()<< endl;
		return(EXIT_FAILURE);
		}
	    }
        if(optind==argc)
		{
		run(cin);
		return EXIT_FAILURE;
		}
        else
		{
		while(optind<argc)
		    {
		    fstream in(argv[optind++],ios::in);
		    if(!in.is_open())
			{
			cerr << "Cannot open \"" << argv[optind]<< "\"\n";
			return EXIT_FAILURE;
			}
		    run(in);
		    in.close();
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


