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
#include "bam.h"
#include "throw.h"
#include "auto_vector.h"
#include "xfaidx.h"
#include "knowngene.h"
#include "application.h"
#include "zstreambuf.h"
using namespace std;

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}

class RedonStructVar:public AbstractApplication
    {
    public:


	class BamFile;

	struct Param
	    {
	    RedonStructVar* owner;
	    Exon* exon;
	    BamFile* bamFile;
	    vector<int>* coverage;
	    };

	struct GCPercent
	    {
	    double gc;
	    double total;
	    };

	struct Point
	    {
	    double gc;
	    int32_t coverage;
	    };


	// callback for bam_fetch()
	static int fetch_func(const bam1_t *b, void *data)
	    {
		bam_plbuf_t *buf = (bam_plbuf_t*)data;
		bam_plbuf_push(b, buf);
		return 0;
	    }

	static int  scan_all_genome_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    Param* param=( Param*)data;
	    if(pos<param->exon->start) return 0;
	    if(pos>=param->exon->end) return 0;
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
		 /*
		for(int n=0;n< core.l_qseq;++n)
		    {
		    cerr << bam_nt16_rev_table[bam1_seqi(seq,n)];
		    }
		cerr << " ";*/
		/*
		cerr << " qpos:" <<  p->qpos;
		cerr << " indel:" <<  p->indel;
		cerr << " is_del :" <<  p->is_del;
		cerr << " strand:" <<  bam1_strand(aln);
		*/
		cerr << " cigar:" << core.n_cigar;


		cerr << endl;
		}
	    cerr << endl;
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

		void pileup(Param* param)
		    {
		    const char* chrom=param->exon->gene->chrom.c_str();
		    int32_t tid=getTidByName(chrom);
		    if(tid<0) THROW("Cannot find chromosome \""<< chrom << "\" in "<< filename);

		    ::bam_plbuf_t *buf= ::bam_plbuf_init( scan_all_genome_func,param);
		    ::bam_fetch(in, idx, tid, param->exon->start, param->exon->end, buf, fetch_func);
		    ::bam_plbuf_push(0, buf); // finalize pileup
		    ::bam_plbuf_destroy(buf);
		    }

	    };

	std::string genomePath;
	auto_vector<BamFile> bamFiles;
	std::auto_ptr<IndexedFasta> faidx;
	double min_qual;
	RedonStructVar():min_qual(0.0)
	    {
	    }


	GCPercent getGC(const Exon* exon)
	    {
	    GCPercent g;
	    std::memset((void*)&g,0,sizeof(GCPercent));
	    std::auto_ptr<string> seq=faidx->fetch(exon->gene->chrom.c_str(),exon->start,exon->end);
	    if(seq.get()!=0)
		{
		for(size_t i=0;i<seq->size();++i)
		    {
		    switch(toupper(seq->at(i)))
			{
			case 'A':case 'T': case 'W': g.total++;break;
			case 'G':case 'C': case 'S': g.gc++;g.total++;break;
			default:break;
			}
		    }
		}
	    return g;
	    }


	void run(const KnownGene* gene)
	    {
	    if(gene->countExons()==0) return;
	    vector<Point> points;
	    Param param;
	    param.owner=this;
	    //loop over exons
	    for(int32_t i=0;i< gene->countExons();++i)
		{
		param.exon=(Exon*)gene->exon(i);
		param.coverage=new vector<int>(param.exon->end - param.exon->start,0);

		for(size_t b=0;b < bamFiles.size();++b)
		    {
		    BamFile* bf=bamFiles.at(b);
		    param.bamFile=bf;
		    bf->pileup(&param);
		    }
		delete param.coverage;
		}
	    sort(points.begin(),points.end());
	    }

	void run(std::istream& in)
	    {
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		auto_ptr<KnownGene> g=KnownGene::parse(line);
		if(g.get()==0) continue;
		run(g.get());
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
		this->run(in);
		}
	    else
		{
		while(optind< argc)
		    {
		    igzstreambuf buf(argv[optind++]);
		    istream in(&buf);
		    this->run(in);
		    buf.close();
		    }
		}

	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}

	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    RedonStructVar app;
    return app.main(argc,argv);
    }
