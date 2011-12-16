/*
 * redonstructvar.cpp
 *
 *  Created on: Dec 14, 2011
 *      Author: lindenb
 */
#include <string>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <set>
#include <map>
#include "bam.h"
#include "throw.h"
#include "auto_vector.h"
#include "xfaidx.h"
#include "knowngene.h"
#include "application.h"
#include "zstreambuf.h"
#include "loess.h"
#include "where.h"
using namespace std;

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}

class RedonStructVar:public AbstractApplication
    {
    public:


	class BamFile;

	struct DepthShuttle
	    {
	    RedonStructVar* owner;
	    BamFile* bamFile;
	    const char* chrom;
	    int32_t start;
	    int32_t end;
	    double coverage;
	    };

	struct GCPercent
	    {
	    double gc;
	    double total;
	    };

	struct CoverageAndGc
	    {
	    double gc;
	    double coverage;
	    bool operator< (const CoverageAndGc& cp) const
		{
		return gc < cp.gc;
		}
	    };


	// callback for bam_fetch()
	static int fetch_func(const bam1_t *b, void *data)
	    {
		bam_plbuf_t *buf = (bam_plbuf_t*)data;
		bam_plbuf_push(b, buf);
		return 0;
	    }



	/**
	static int  _deprecated_scan_all_genome_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    DepthShuttle* param=( DepthShuttle*)data;
	    if(pos<param->position) return 0;
	    if(pos>=pos) return 0;
	    cerr << "seq:"<< param->bamFile->header->target_name[tid] << " pos:" << pos << " dep:"<< depth << endl;
	    for( int i=0;i<depth ;++i)
		{

		const bam_pileup1_t *p=&pl[i];
		bam1_t* aln=p->b;
		const bam1_core_t core=aln->core;
		uint32_t *cigar = bam1_cigar(aln);
		const uint8_t* seq=bam1_seq(aln);
		uint8_t *qualities = bam1_qual(aln);

		for(int k= pl[0].b->core.pos;k<core.pos;++k)
		    {
		    cerr << " ";
		    }

		int read_seq_index=0;
		int read_genomic_pos=core.pos;

		for (int cigaridx = 0; cigaridx < core.n_cigar; ++cigaridx)
		    {
		    int op = cigar[cigaridx] & BAM_CIGAR_MASK;
		    int n_op= cigar[cigaridx] >> BAM_CIGAR_SHIFT;
		    for(int k=0;k< n_op;++k)
			{
			switch(op)
			    {
			    case BAM_CMATCH:
				{
				char base=bam_nt16_rev_table[bam1_seqi(seq,read_seq_index)];
				uint8_t qual=qualities[read_seq_index];

				if(read_genomic_pos==pos)
				    {
				    if(param->owner->min_qual>=(double)qual)
					{
					param->coverage->at(pos-param->exon->start)++;
					}
				    //cerr << "[";
				    }
				cerr << base;
				if(read_genomic_pos==pos)
				    {
				    //cerr << "]";
				    }
				++read_seq_index;
				++read_genomic_pos;
				break;
				}
			    //deletion from the reference
			    case BAM_CDEL:
				{
				if(read_genomic_pos==pos)
				    {
				    //cerr << "[";
				    }
				cerr << "*";
				if(read_genomic_pos==pos)
				    {
				    //cerr << "]";
				    }
				++read_genomic_pos;
				break;
				}
			    //insertion to the reference
			    case BAM_CINS:
				{
				if(read_genomic_pos==pos)
				    {
				    //cerr << "[ ]";
				    }

				++read_seq_index;
				++read_genomic_pos;
				break;
				}
			    default:
				{
				cerr << "####" << op ;
				break;
				}
			    }
			}
		    }

		 cerr << " "<<  core.pos << " ";
		 for(int n=0;n< core.l_qseq;++n)
		    {
		    cerr << (char)(qualities[n]+33);
		    }

		for(int n=0;n< core.l_qseq;++n)
		    {
		    cerr << bam_nt16_rev_table[bam1_seqi(seq,n)];
		    }
		cerr << " ";

		cerr << " qpos:" <<  p->qpos;
		cerr << " indel:" <<  p->indel;
		cerr << " is_del :" <<  p->is_del;
		cerr << " strand:" <<  bam1_strand(aln);

		cerr << " cigar:" << core.n_cigar;


		cerr << endl;
		}
	    cerr << endl;
	    return 0;
	    }*/


	static int  scan_all_genome_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    DepthShuttle* param=( DepthShuttle*)data;
	    //WHERE("ici!"<< pos << " " << param->chrom << ":" << param->position);
	    if((int32_t)pos<param->start) return 0;
	    if((int32_t)pos>=param->end) return 0;
	    //WHERE("ici!"<< pos << " vs " << param->chrom << ":" << param->position << " d="<< depth);
	    for( int i=0;i<depth ;++i)
		{
		const bam_pileup1_t *p=&pl[i];
		bam1_t* aln=p->b;
		const bam1_core_t& core=aln->core;
		uint32_t *cigar = bam1_cigar(aln);
		//const uint8_t* seq=bam1_seq(aln);
		uint8_t *qualities = bam1_qual(aln);
		int read_seq_index=0;
		uint32_t genomic_pos=core.pos;

		for (uint32_t cigaridx = 0; cigaridx < core.n_cigar; ++cigaridx)
		    {
		    int op = cigar[cigaridx] & BAM_CIGAR_MASK;
		    int n_op= cigar[cigaridx] >> BAM_CIGAR_SHIFT;
		    for(int k=0;k< n_op;++k)
			{
			switch(op)
			    {
			    case BAM_CMATCH:
				{
				if(genomic_pos==pos)
				    {
				    uint8_t qual=qualities[read_seq_index];
				    if(param->owner->min_qual<=(double)qual)
					{
					param->coverage++;
					}
				    }
				++read_seq_index;
				++genomic_pos;
				break;
				}
			    //deletion from the reference
			    case BAM_CDEL:
				{
				++genomic_pos;
				break;
				}
			    //insertion to the reference
			    case BAM_CSOFT_CLIP:
			    case BAM_CINS:
				{
				++read_seq_index;
				break;
				}
			    default:
				{
				cerr << "####cigar::op not handled:" << op << endl ;
				break;
				}
			    }
			}
		    }
		}
	    return 0;
	    }


	class BamFile
	    {
	    public:
		std::string filename;
		bamFile in;
		bam_header_t* header;
		bam_index_t *idx;

		BamFile():in(NULL),header(NULL),idx(NULL)
		    {
		    }


		void close()
		    {
		    if(in!=NULL) ::bam_close(in);
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
		    in=::bam_open(filename.c_str(), "rb");
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

		bool pileup(DepthShuttle* param)
		    {
		    int32_t tid=getTidByName(param->chrom);

		    if(tid<0)
			{
			cerr << "Cannot find chromosome \""<< param->chrom << "\". Check you're using the ucsc nomenclature.\n";
			return false;
			}
		    if(param->end >= (int32_t)header->target_len[tid])
			{
			return false;
			}

		    ::bam_plbuf_t *buf= ::bam_plbuf_init( scan_all_genome_func,param);
		    if(buf==NULL) THROW("boum");
		    ::bam_fetch(in, idx, tid, param->start, param->end, buf, fetch_func);
		    ::bam_plbuf_push(0, buf); // finalize pileup
		    ::bam_plbuf_destroy(buf);
		    return true;
		    }

	    };
	Loess loessAlgo;
	auto_vector<BamFile> bamFiles;
	std::auto_ptr<IndexedFasta> faidx;
	auto_vector<KnownGene> knownGenes;
	double min_qual;

	RedonStructVar():min_qual(0.0)
	    {
	    }


	bool coverageAt(const char* chrom,int32_t exon_start,int32_t exon_end,double* coverage)
	    {
	    DepthShuttle param;
	    memset((void*)&param,0,sizeof(DepthShuttle));
	    param.chrom=chrom;
	    param.owner=this;
	    param.start=exon_start;
	    param.end=exon_end;
	    for(size_t b=0;b < bamFiles.size();++b)
		{
		BamFile* bf=bamFiles.at(b);
		param.bamFile=bf;
		if(!bf->pileup(&param)) return false;
		}
	    param.coverage/=(double)(exon_end-exon_start);
	    param.coverage/=(double)bamFiles.size();
	    *coverage=param.coverage;
	    return true;
	    }

	bool gcPercentAt(const char* chrom,int32_t exon_start,int32_t exon_end,double* gcpercent)
	    {
	    GCPercent g;
	    std::memset((void*)&g,0,sizeof(GCPercent));
	    std::auto_ptr<string> seq=faidx->fetch(chrom,exon_start,exon_end);
	    if(seq.get()==0) return false;
	    //if(seq->size()!=1) cerr << "???GC%:"<< chrom<<":" << pos << endl;
	    for(size_t i=0;i<seq->size();++i)
		{
		switch(toupper(seq->at(i)))
		    {
		    case 'A':case 'T': case 'W': g.total++;break;
		    case 'G':case 'C': case 'S': g.gc++;g.total++;break;
		    default:break;
		    }
		}

	    if(g.total==0) return false;
	    *gcpercent=g.gc/g.total;
	    return true;
	    }

	void run()
	    {
	    const int32_t window_size=100;
	    vector<CoverageAndGc> coverageandgc;
	    set<string> chromosomes;
	    //get distinct chromosome
	    for(size_t i=0;i< knownGenes.size();++i)
		{
		chromosomes.insert(knownGenes.at(i)->chrom);
		}
	    for(set<string>::iterator r=chromosomes.begin(); r!=chromosomes.end();++r)
		{
		WHERE("chromosome"<< (*r));
		vector<bool> exon_bases;
		exon_bases.reserve(50000000);
		for(size_t i=0;i< knownGenes.size();++i)
		    {
		    KnownGene* gene=knownGenes.at(i);
		    if((*r).compare(gene->chrom)!=0) continue;
		    if((int32_t)exon_bases.size()< gene->txEnd)
			{
			exon_bases.resize(gene->txEnd+window_size+1,false);
			}
		    for(int32_t e=0;e < gene->countExons();++e)
			{
			const Exon* exon= gene->exon(e);
			for(int32_t pos=max(0,exon->start-window_size);pos<exon->end+window_size;++pos)
			    {
			    exon_bases[pos]=true;
			    }
			}
		    }
		int32_t exon_beg=0;
		while(exon_beg<(int32_t)exon_bases.size())
		    {
		    if(!exon_bases[exon_beg])
			{
			exon_beg++;
			continue;
			}
		    int32_t exon_end=1+exon_beg;
		    while(exon_end< (int32_t)exon_bases.size() && exon_bases[exon_end])
			{
			exon_end++;
			}
		    CoverageAndGc cov;
		    if(  coverageAt(r->c_str(),exon_beg,exon_end,&(cov.coverage)) &&
			gcPercentAt(r->c_str(),exon_beg,exon_end,&(cov.gc)))
			{
			WHERE((*r)<<":"<< exon_beg << "-" << exon_end);
			coverageandgc.push_back(cov);
			}
		    exon_beg=exon_end;
		    }
		}
	    WHERE("sorting");
	    //sort on gc
	    sort(coverageandgc.begin(),coverageandgc.end());

	    vector<double> x;
	    vector<double> y;
	    x.reserve(coverageandgc.size());
	    y.reserve(coverageandgc.size());
	    for(vector<CoverageAndGc>::iterator r=coverageandgc.begin();
		r!=coverageandgc.end();
		++r)
		{
		x.push_back(r->gc);
		y.push_back(r->coverage);
		}
	    /*
	     * echo -e "set yrange [0.0:150.0]\nset terminal postscript\nset output 'jeter.ps'\nplot 'jeter.dat' using 1:2 title 'f1',  'jeter.dat' using 1:3 title 'f2' " | gnuplot  > jeter.ps; ps2pdf jeter.ps
	    */
	    auto_ptr<vector<double> > y_prime=loessAlgo.lowess(&(x.front()),&(y.front()),x.size());
	    WHERE("saving");
	    FILE* out=fopen("jeter.dat","w");
	    fprintf(out,"GC\tDEPTH\tLOESS_DEPTH\n");
	    for(size_t i=0;i< coverageandgc.size();++i)
		{
		fprintf(out,"%f\t%f\t%f\n",coverageandgc[i].gc,coverageandgc[i].coverage,y_prime->at(i));
		}
	    fflush(out);
	    fclose(out);

	    fstream os("jeter.exons.tsv",ios::out);
	    if(!os.is_open()) THROW("boum");

	    os << "chrom" << "\t"
		<< "gene.name" << "\t"
		<< "exon.name" << "\t"
		<< "exon.start" << "\t"
		<< "exon.end" << "\t"
		<< "gc.percent"
		;
	    for(size_t bamIdx=0;bamIdx< bamFiles.size();++bamIdx)
	    	{
		os << "\t"
		 << "coverage(" << bamFiles.at(bamIdx)->filename << ")"
		 << "\t"
		 << "loess.coverage(" << bamFiles.at(bamIdx)->filename << ")"
		 ;
	    	}
	   os << "\tFLAG"<< endl;

	    for(set<string>::const_iterator r=chromosomes.begin(); r!=chromosomes.end();++r)
		{
		 DepthShuttle param;
		memset((void*)&param,0,sizeof(DepthShuttle));
		param.chrom=r->c_str();
		param.owner=this;

		for(size_t i=0;i< knownGenes.size();++i)
		    {
		    KnownGene* gene=knownGenes.at(i);
		    if((*r).compare(gene->chrom)!=0) continue;


		    for(int32_t e=0;e < gene->countExons();++e)
			{
			const Exon* exon= gene->exon(e);
			param.start=exon->start;
			param.end=exon->end;

			WHERE(gene->name<< " " << exon->name() << "/"<< gene->countExons());
			double gc_percent=0.0;
			if(!gcPercentAt(r->c_str(),exon->start,exon->end,&(gc_percent))) continue;

			 os << gene->chrom << "\t"
			    << gene->name << "\t"
			    << exon->name() << "\t"
			    << exon->start << "\t"
			    << exon->end << "\t"
			    << gc_percent
			    ;

			vector<double> all_loess_cov;
			all_loess_cov.reserve(bamFiles.size());
			for(size_t bamIdx=0;bamIdx< bamFiles.size();++bamIdx)
			    {
			    param.bamFile=bamFiles.at(bamIdx);
			    param.coverage=0;
			    os << "\t";
			    if(!param.bamFile->pileup(&param))
				{
				os << ".\t.";
				continue;
				}

			    param.coverage/=(double)(exon->end-exon->start);

			    vector<double>::iterator rgc=lower_bound(x.begin(),x.end(),gc_percent);
			    if(rgc==x.end())
				{
				os << ".\t.";
				continue;
				}
			    size_t coverage_index=std::distance(x.begin(),rgc);


			    os << param.coverage << "\t"
				<< y_prime->at(coverage_index)
				;

			    all_loess_cov.push_back(y_prime->at(coverage_index));
			    }
			double total_cov=0.0;
			for(size_t k=0;k< all_loess_cov.size();++k)
			    {
			    total_cov+=all_loess_cov.at(k);
			    }
			string flag(".");
			if(!all_loess_cov.empty() && total_cov>0)
			    {
			    double mean_cov=total_cov/(double)all_loess_cov.size();

			    for(size_t k=0;k< all_loess_cov.size();++k)
				{
				if(all_loess_cov.at(k)< mean_cov/10.0)
				    {
				    flag.assign("CNV");
				    break;
				    }
				}
			    }

			os << "\t" << flag;
			os << endl;
			}
		    }
		}
	    os.flush();
	    os.close();
	    }

	void readKnownGenes(std::istream& in)
	    {
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		auto_ptr<KnownGene> g=KnownGene::parse(line);
		if(g.get()==0) continue;
		knownGenes.push_back(g.release());
		}
	    }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] (knownGene stdin| knownGene files)"<< endl;
	    out << "\nOptions:\n";
	    out << "  -B (/path/to/file.bam) Add a bam to the list." << endl;
	    out << "  -b (file) read a list containing the path to the BAM." << endl;
	    out << "  -f (file.fa) genome reference file indexed with samtools faidx." << endl;
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    int optind=1;
	    char* faidxfile=NULL;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    this->usage(cerr,argc,argv);
		    return(EXIT_FAILURE);
		    }
		else if(strcmp(argv[optind],"-f")==0 && optind+1< argc)
		    {
		    faidxfile = argv[++optind];
		    }
		else if(strcmp(argv[optind],"-B")==0 && optind+1< argc)
		    {
		    BamFile* bf=new BamFile;
		    bf->filename.assign(argv[++optind]);
		    this->bamFiles.push_back(bf);
		    }
		else if(strcmp(argv[optind],"-b")==0 && optind+1< argc)
		    {
		    char* f=argv[++optind];
		    fstream in(f);
		    if(!in.is_open())
			{
			cerr << "Cannot open "<< f << " " << strerror(errno)<< endl;
			this->usage(cerr,argc,argv);
			return EXIT_FAILURE;
			}
		    string line;
		    while(getline(in,line,'\n'))
			{
			if(line.empty() || line[0]=='#') continue;
			BamFile* bf=new BamFile;
			bf->filename.assign(line.c_str());
			this->bamFiles.push_back(bf);
			}
		    in.close();
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
	    if(faidxfile==0)
		{
		cerr << "no fasta genome defined." << endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	    this->faidx.reset(new IndexedFasta(faidxfile));

	    if(this->bamFiles.empty())
		{
		cerr << "no BAM file defined." << endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}

	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->open();
		}

	    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		this->readKnownGenes(in);
		}
	    else
		{
		while(optind< argc)
		    {
		    igzstreambuf buf(argv[optind++]);
		    istream in(&buf);
		    this->readKnownGenes(in);
		    buf.close();
		    }
		}
	    run();
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}
	    cerr << "Done." << endl;
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    RedonStructVar app;
    return app.main(argc,argv);
    }
