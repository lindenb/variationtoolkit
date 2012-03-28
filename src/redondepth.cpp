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
#include <cmath>
#include "xbam2.h"
#include "segments.h"
#include "throw.h"
#include "auto_vector.h"
#include "xfaidx.h"
#include "application.h"
#include "zstreambuf.h"
#include "loess.h"
#include "numeric_cast.h"
#include "where.h"
#include "tarball.h"

using namespace std;


class RedonDepth:public AbstractApplication
    {
    public:

	/* data per exon */
	class Exon
	    {
	    public:
		ChromStartEnd segment;
		double gc_percent;
		std::vector<double> coverage;
		std::vector<double> normalized_coverage;

		Exon(const char* c,int32_t s,int32_t e):segment(c,s,e)
		    {

		    }

		int32_t size() const
		    {
		    return segment.end-segment.start;
		    }
		double median_depth() const
		    {
		    std::vector<double> cp(coverage.begin(),coverage.end());
		    sort(cp.begin(),cp.end());
		    return cp[cp.size()/2];
		    }

		double median_normalized_coverage() const
		    {
		    std::vector<double> cp(normalized_coverage.begin(),normalized_coverage.end());
		    sort(cp.begin(),cp.end());
		    return cp[cp.size()/2];
		    }

		double min_depth() const
		    {
		    double d=numeric_limits<double>::max();
		    for(size_t i=0;i< coverage.size();++i)
			{
			d=std::min(d,coverage[i]);
			}
		    return d;
		    }

	    };

	struct PileupShuttle
	    {
		RedonDepth* owner;
		BamFile2* bamFile;
		Exon* exon;
		size_t bam_index;
	    };



	static int  scan_depth_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	    {
	    PileupShuttle* param=( PileupShuttle*)data;
	    //WHERE("ici!"<< pos << " " << param->chrom << ":" << param->position);
	    if((int32_t)pos<param->exon->segment.start) return 0;
	    if((int32_t)pos>=param->exon->segment.end) return 0;
	    int32_t count=0;
	    for( int i=0;i<depth ;++i)
		{
		count++;
		}
	    param->exon->coverage[param->bam_index]+=count;
	    return 0;
	    }

	Loess loessAlgo;
	/** the BAM file input */
	auto_vector<BamFile2> bamFiles;
	/** reference genome */
	std::auto_ptr<IndexedFasta> faidx;
	/** list of exons */
	auto_vector<Exon> exons;
	/** minimum quality */
	double min_qual;
	/* minimum exon size */
	int32_t minimum_exon_size;



	RedonDepth():min_qual(0.0),
		minimum_exon_size(25)
	    {
	    }


	/** returns the GC% for a given genomic segment */
	float gcPercentAt(const char* chrom,int32_t exon_start,int32_t exon_end)
	    {
	    int32_t g_gc=0;
	    int32_t g_total=0;
	    std::auto_ptr<string> seq=faidx->fetch(chrom,exon_start,exon_end);
	    if(seq.get()==0) THROW("BOUM");
	    //if(seq->size()!=1) cerr << "???GC%:"<< chrom<<":" << pos << endl;
	    for(size_t i=0;i<seq->size();++i)
		{
		 g_total++;
		switch(toupper(seq->at(i)))
		    {
		    case 'G':case 'C': case 'S': g_gc++;break;
		    default:break;
		    }
		}
	    return g_gc/g_total;
	    }

	/** comparator for exons on GC% */
	static bool cmp_on_gc(const Exon* g1,const Exon* g2)
	    {
	    return g1->gc_percent< g2->gc_percent;
	    }
	static FILE* safe_tmpfile()
	    {
	    FILE* o=::tmpfile();
	    if(o==NULL) THROW("Cannot open file");
	    return o;
	    }

	/** main loop */
	void run()
	    {
	    set<string> all_chromosomes;
	    /** data shuttle, will be used by the pileup function */
	    PileupShuttle shuttle;
	    shuttle.owner=this;

	    /* loop over the exon */
	    size_t exon_index=0;
	    while( exon_index< this->exons.size())
		{
		WHERE(exon_index << "/"<<this->exons.size() );
		shuttle.exon = this->exons.at(exon_index);

		/* delete exon size<=25 */
		if(shuttle.exon->size()<= this->minimum_exon_size)
		    {
		    this->exons.erase(exon_index);
		    continue;
		    }


		//get gc% for this segement
		shuttle.exon->gc_percent=gcPercentAt(
			shuttle.exon->segment.chrom.c_str(),
			shuttle.exon->segment.start,
			shuttle.exon->segment.end
			);




		shuttle.exon->coverage.resize(this->bamFiles.size(),0);

		 /** loop over each sample */
		for(shuttle.bam_index=0;
			shuttle.bam_index< this->bamFiles.size();
			++shuttle.bam_index
			)
		    {
		    shuttle.bamFile = this->bamFiles.at(shuttle.bam_index);
		    int32_t tid=shuttle.bamFile->findTidByName(shuttle.exon->segment.chrom.c_str());
		    if(tid<0) THROW("chromosome name not indexed in tid.");
		    bam_plbuf_t *buf= ::bam_plbuf_init(scan_depth_func, &shuttle);
		    ::bam_fetch(
			    shuttle.bamFile->bamPtr(),
			    shuttle.bamFile->bamIndex(),
			    tid,
			    shuttle.exon->segment.start,
			    shuttle.exon->segment.end,
			    buf,
			    BamFile2::fetch_func
			    );
		    bam_plbuf_push(0, buf);
		    bam_plbuf_destroy(buf);

		    //normalize to exon length
		    shuttle.exon->coverage[shuttle.bam_index]/=(double)( shuttle.exon->size()  );
		    }

		/* min-depth */
		if(shuttle.exon->min_depth()<=0)
		    {
		    this->exons.erase(exon_index);
		    continue;
		    }

		/* mediane_depthh */
		if(shuttle.exon->median_depth()<10)
		    {
		    this->exons.erase(exon_index);
		    continue;
		    }

		all_chromosomes.insert(shuttle.exon->segment.chrom);
		++exon_index;
		}/* end of loop over exons */

	   vector<Exon*> sort_on_gc;
	   for(size_t i=0;i< this->exons.size();++i)
	       {
	       sort_on_gc.push_back(this->exons.at(i));
	       }
	   /* sort on GC% */
	   std::sort(sort_on_gc.begin(),sort_on_gc.end(),cmp_on_gc);


	   fstream os("jeter.tar",ios::out);
	   Tar tarball(os);
	   std::string prefix("redondepth");
	   std::string filename;
	   FILE* gnuplotout= safe_tmpfile();


	       /* reset gnuplot */
	       fprintf(gnuplotout,
		       "set term postscript\n"
		       "set output \"redondepth.ps\"\n"
		       );


	   /* loop over each individual */
	   for(size_t sampleidx1=0;sampleidx1< this->bamFiles.size();++sampleidx1)
	       {
	       char folder_name[50];
	       sprintf(folder_name,"SAMPLE%d",(int)(1+sampleidx1));
	       string prefix2(prefix);
	       prefix2.append("/").append(folder_name);



	       /* initialize */
	       for(size_t i=0;i< sort_on_gc.size();++i)
		   {
		   Exon* exon=sort_on_gc.at(i);
		   exon->normalized_coverage.clear();
		   }

	       /* loop over the other individuals */
	       for(size_t sampleidx2=0;sampleidx2< this->bamFiles.size();++sampleidx2)
	       	       {
		       /* skip if same individual */
		       if(sampleidx1==sampleidx2) continue;
		       vector<double> x;
		       vector<double> y;
		       /** loop over each exon and build lowess X/Y */
		       for(size_t i=0;i< sort_on_gc.size();++i)
			   {
			   Exon* exon=sort_on_gc.at(i);
			   x.push_back(exon->gc_percent);
			   y.push_back(log2(exon->coverage[sampleidx1]/exon->coverage[sampleidx2]));
			   }
		       auto_ptr<vector<double> > y_prime=loessAlgo.lowess(&(x.front()),&(y.front()),x.size());

		       /* loop over all exons and push normalized ratio*/
		       for(size_t i=0;i< sort_on_gc.size();++i)
			   {
			   Exon* exon=sort_on_gc.at(i);
			   double l=log2(exon->coverage[sampleidx1]/exon->coverage[sampleidx2]);
			   exon->normalized_coverage.push_back(l-y_prime->at(i));
			   }
	       	       }


	       for(std::set<string>::iterator r=all_chromosomes.begin();
		       r!=all_chromosomes.end();
		       ++r)
		   {
		   FILE* outdat = safe_tmpfile();

		   for(size_t i=0;i< this->exons.size();++i)
		       {
		       Exon* exon=this->exons.at(i);
		       if(exon->segment.chrom.compare(*r)!=0) continue;

		       fprintf(
			       outdat,
			       "%d\t%f\n",
			       exon->segment.start,
			       exon->median_normalized_coverage()
			   );
		       }

		   string filename_dat(prefix2);
		   filename_dat.append("/").append(*r).append(".dat");
		   tarball.putFile(outdat,filename_dat.c_str());
		   fclose(outdat);
		   fprintf(gnuplotout,"set title \"SAMPLE-%d %s\"\n",(1+sampleidx1),r->c_str());
		   fprintf(gnuplotout,"set xlabel \"Position\"\n");
		   fprintf(gnuplotout,"set ylabel \"Median depth\"\n");
		   fprintf(gnuplotout,"set yrange [-2:2]\n");
		   fprintf(gnuplotout,"plot \"%s/%s.dat\" using 1:2 notitle\n",folder_name,r->c_str());
		   }
	       }
	    filename.assign(prefix).append("/gnuplot.txt");
	    tarball.putFile(gnuplotout,filename.c_str());
	    tarball.finish();
	    fclose(gnuplotout);
	    os.flush();
	    os.close();
	    }

	void readExons(std::istream& in)
	    {
	    vector<string> tokens;
	    Tokenizer tab;
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		tab.split(line,tokens);
		if(tokens.size()<3)
		    {
		    THROW("illegal number of tokens in "<< line);
		    }
		Exon* exon=0;
		int32_t chromStart=0;
		int32_t chromEnd=0;
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&chromStart) || chromStart<0)
		    {
		    THROW("bad chromStart in  "<< line);
		    }
		if(!numeric_cast<int32_t>(tokens[2].c_str(),&chromEnd) || chromStart >= chromEnd)
		    {
		    THROW("bad chromEnd in  "<< line);
		    }
		exon=new Exon(tokens[0].c_str(),chromStart,chromEnd);



		this->exons.push_back(exon);
		}
	    }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\n";
	    out << "   "<<  argv[0] << " [options] bam1.bam bam2.bam bam3.bam ..."<< endl;
	    out << "\nOptions:\n";
	    out << "  -r (/path/to/file.bam) Add a bam to the list." << endl;
	    out << "  -g (file) bed list of exon (chrom/start/end)." << endl;
	    out << "  -f (file.fa) genome reference file indexed with samtools faidx." << endl;
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    const char* exonFile=0;
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
		else if(strcmp(argv[optind],"-g")==0 && optind+1< argc)
		    {
		    exonFile=argv[++optind];
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





	    if(exonFile==0)
		{
		cerr << "no bed for exons defined." << endl;
		this->usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}



	    if(optind==argc)
		{
		cerr << "BAM files missing"<< endl;
		return EXIT_FAILURE;
		}
	    else
		{
		while(optind< argc)
		    {
		    char* bamfile=argv[optind++];
		    WHERE("Open "<< bamfile);
		    BamFile2* bam=new BamFile2(bamfile);
		    this->bamFiles.push_back(bam);
		    bam->open(true);
		    }
		}

	   //open exon file
	    igzstreambuf buf(exonFile);
	    istream in(&buf);
	    this->readExons(in);
	    buf.close();
	    if(exons.empty())
		{
		cerr << "No exon.\n";
		return EXIT_FAILURE;
		}

	    run();
	    for(size_t i=0;i< bamFiles.size();++i)
		{
		bamFiles[i]->close();
		}
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    RedonDepth app;
    return app.main(argc,argv);
    }
