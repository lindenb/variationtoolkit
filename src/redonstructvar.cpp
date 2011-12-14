/*
 * redonstructvar.cpp
 *
 *  Created on: Dec 14, 2011
 *      Author: lindenb
 */
#include <string>
#include <memory>
#include "sam.h"
#include "auto_vector.h"
#include "xfaidx.h"
#include "knowngene.h"

class RedonStructVar
    {
    public:
	// callback for bam_fetch()
	static int fetch_func(const bam1_t *b, void *data)
	    {
		bam_plbuf_t *buf = (bam_plbuf_t*)data;
		bam_plbuf_push(b, buf);
		return 0;
	    }

	static int  scan_all_genome_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    return 0;
	    }

	struct Param
	    {
	    RedonStructVar* owner;
	    Exon* exon;
	    int* coverage;
	    };

	struct GCPercent
	    {
	    double gc;
	    double total;
	    };




	class BamFile
	    {
	    public:
		std::string filename;
		samfile_t *in;
		bam_header_t* header;
		bam_index_t *idx;

		BamFile():in(NULL),header(NULL),idx(NULL)
		    {
		    }


		void close()
		    {
		    if(in!=NULL) ::bamclose(in);
		    in=NULL;
		    if(header!=NULL) ::bam_header_destroy(header);
		    header=NULL;
		    if(idx!=NULL) ::bam_index_destroy(idx);
		    idx=NULL;
		    }

		~BamFile()
		    {
		    close();
		    }

		void open()
		    {
		    close();
		    in=::samopen(filename.c_str(), "rb", 0);
		    if(in==NULL)
			{
			THROW("Cannot open BAM file \"" << filename << "\". "<< strerror(errno));
			}

		    header= ::bam_header_read(in);
		        if(header==NULL)
		    	{
		    	THROW("Cannot read header for "<< filename);
		    	}
		     ::bam_init_header_hash(header);

		    idx = bam_index_load(filename.c_str());
		    if(idx==NULL)
			{
			THROW("Cannot open INDEX for BAM  \"" << filename << "\". "<< strerror(errno));
			}

		    }
		int32_t getTidByName(const char* seq_name)
		    {
		    return ::bam_get_tid(header, seq_name);
		    }

		void getCoverage(Param* param)
		    {
		    const char* chrom=param->exon->gene->chrom.c_str();
		    int32_t tid=getTidByName(chrom);
		    if(tid<0) THROW("Cannot find chromosome \""<< chrom << "\" in "<< filename);

		    ::bam_plbuf_t *buf= ::bam_plbuf_init( scan_all_genome_func,param);
		    ::bam_fetch(parameter.in->x.bam, idx, tid, position, position, buf, fetch_func);
		    ::bam_plbuf_push(0, buf); // finalize pileup

		    }

	    };

	std::string genomePath;
	auto_vector<BamFile> bamFiles;
	std::auto_ptr<IndexedFasta> faidx;

	RedonStructVar()
	    {
	    }


	void getCoverage(const Exon* exon)
	    {
	    Param param;
	    param.coverage=new int[exon->end - exon->start ];
	    param.owner=this;
	    param.exon=(Exon*)exon;
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		BamFile* bf=bamFiles.at(i);
		bf->getCoverage(&param);
		}
	    delete [] param.coverage;
	    }

	GCPercent getGC(const Exon* exon)
	    {
	    GCPercent g;
	    memset((void*)&gc,0,sizeof(GCPercent));
	    std::auto_ptr<string> seq=faidx->fetch(exon->gene->chrom.c_str(),exon->start,exon->end);
	    if(seq.get()!=0)
		{
		for(size_t i=0;i<seq->size();++i)
		    {
		    switch(toupper(seq->at(i)))
			{
			case 'A':case 'T': case 'W': g.total++;break;
			case 'G':case 'C': case 'S': g.gc++;g.total++;break;
			}
		    }
		}
	    return g;
	    }

	void run(const Exon* exon)
	    {

	    }

	void run(const KnownGene* gene)
	    {
	    if(gene->countExons()==0) return;
	    for(int32_t i=0;i< gene->countExons();++i)
		{
		run(gene->exon(i));
		}
	    }

    };
