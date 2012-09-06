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

using namespace std;

class GenomeCoverage
    {
    private:
	vector<uint32_t> coverage;
    public:
	auto_vector<BamFile2> bamFiles;
	uint32_t min_qual;
	

	GenomeCoverage():min_qual(0)
	    {
	    }

	virtual ~GenomeCoverage()
	    {
	    }


	static int  pileup_callback(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    GenomeCoverage* stat=(GenomeCoverage*)data;
	    if(pos >= stat->coverage.size()) return 0;
	    for(int i=0;i< depth;++i)
		{
		const bam_pileup1_t* curr=&pl[i];
		if(curr->b->core.qual < stat->min_qual) continue;
		stat->coverage[pos]++;
		}
	    return 0;
	    }

	class ChromInfo
		{
		public:
			std::string name;
			uint32_t length;
			uint32_t count_N;
			ChromInfo():length(0),count_N(0) {}
		};

	void run(BamFile2* bam,int tid,ChromInfo* chromInfo)
	    {
	    coverage.clear();
	    coverage.resize(chromInfo->length,0);
	    ::bam_plbuf_t *buf= ::bam_plbuf_init( GenomeCoverage::pileup_callback,this);
	    if(buf==NULL) THROW("bam_plbuf_init failed");
	    ::bam_fetch(
		    bam->bamPtr(),
		    bam->bamIndex(),
		    tid,
		    0,
		    (int32_t)chromInfo->length,
		    buf,
		    BamFile2::fetch_func
		    );

	    ::bam_plbuf_push(0, buf); // finalize pileup
	    ::bam_plbuf_destroy(buf);
	    double total=0;
	    for(size_t i=0;i< coverage.size();++i) total+=coverage[i];
	    cout << "\t";
	    cout << ceil(total/(double)chromInfo->length);
	    cout << "\t";
	    cout << ceil(total/(double)(chromInfo->length - chromInfo->count_N));
	    }

	void init_run()
	    {
	    cout << "#chrom\tlength";
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		cout << "\tmean-depth(" << bamFiles[i]->path() << ")\tmean-depth-noN(" << bamFiles[i]->path() << ")";
		bamFiles[i]->open();
		}
	    cout << endl;
	    }

	void finish_run()
	    {
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}
	    }
	
	
	
	void runChrom(ChromInfo* chromInfo)
		{
		cout << chromInfo->name << "\t";
		cout << chromInfo->length;
		for(size_t i=0;i< bamFiles.size();++i)
		    {
		    int32_t tid=bamFiles.at(i)->findTidByName(chromInfo->name.c_str());
		    if(tid==-1)
			{
			cerr << "No chromosome for " << chromInfo->name  << " in " << bamFiles.at(i)->path() << endl;
			exit(EXIT_FAILURE);
			}
		   
		    run(bamFiles.at(i),tid,chromInfo);
		    }
		cout << endl;
		}

	void run(const char* referencefilename)
	    {
	  
	    FILE* in=fopen( referencefilename,"r");
	    if(in==0)
			{
			cerr << "Cannot open "<<referencefilename << " " <<strerror(errno) << endl;
			exit(EXIT_FAILURE);
			}
	    else
			{
			ChromInfo* currinfo=0;
			int c;
			while((c=fgetc(in))!=EOF)
				{
				if(c=='>')
					{
					if(currinfo!=NULL)
						{
						runChrom(currinfo);
						delete 	currinfo;					
						}
					currinfo=new ChromInfo;
					bool wsp=false;
					while((c=fgetc(in))!=EOF && c!='\n')
						{
						if(wsp) continue;
						if(isspace(c)) {wsp=true; continue;}
						currinfo->name+=(char)c;
						}
	
					}
				else if(currinfo!=0 && isalpha(c))
					{
					currinfo->length++;
					switch(c)
						{
						case 'a':case 'A': case 't': case 'T':
						case 'g':case 'G': case 'c': case 'C':
							break;
						default: currinfo->count_N++; break;
						}
					}
				}
			if(currinfo!=NULL)
				{
				runChrom(currinfo);
				delete 	currinfo;					
				}
			fclose(in);
			}


	    }



	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] (bams)"<< endl;
	    out << "\nOptions:\n";
	    out << " -f (fasta) genome reference (required)\n";
	    out << " -m (int) min mapping quality default:0\n";
	    out << endl;
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    char* fastaFile=0;
	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return(EXIT_SUCCESS);
		    }
		else if( (strcmp(argv[optind],"-f")==0 || strcmp(argv[optind],"--ref")==0) && optind+1<argc)
		    {
		    fastaFile=argv[++optind];
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
		
	    if(fastaFile==0)
		{
		cerr << "no reference fasta defined"<< endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	    while(optind< argc)
		{
		this->bamFiles.push_back(new BamFile2(argv[optind++]));
		}
	    init_run();
	    run(fastaFile);
	    finish_run();
	    return EXIT_SUCCESS;
	    }

    };


int main(int argc,char** argv)
    {
    GenomeCoverage app;
    return app.main(argc,argv);
    }
