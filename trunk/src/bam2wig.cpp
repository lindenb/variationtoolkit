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
#include <stdint.h>
#include "sam.h"
#include "throw.h"
#include "numeric_cast.h"

using namespace std;

class Bam2Wig
    {
    public:
	static const int NUM_ZERO_ACCEPTED_DEFAULT=10;

	/** output stream */
	FILE* out;
	/** previous chromosome seen */
	int32_t prev_tid;
	/** previous genomic position seen */
	int32_t prev_pos;
	/** user's start position */
	int32_t beg;
	/** user's end position */
	int32_t end;
	/** number of depth=0 seen */
	int32_t count_zero;
	/** max number of depth=0 allowed */
	int32_t pref_zero;
	/** input BAM */
	samfile_t *in;

	Bam2Wig():out(stdout),prev_tid(-1),
		prev_pos(-1),
		beg(0),
		end(numeric_limits<int32_t>::max()),
		count_zero(0),
		pref_zero(NUM_ZERO_ACCEPTED_DEFAULT),
		in(NULL)
	    {

	    }

	virtual ~Bam2Wig()
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
    static int  scan_all_genome_func(uint32_t tid, uint32_t pos, int depth, const bam_pileup1_t *pl, void *data)
	{
	Bam2Wig* param = (Bam2Wig*)data;
	if ((int)pos >= param->beg && (int)pos < param->end)
		{
		if(depth==0) /* no coverage */
			{
			param->count_zero++;
			}
		else
			{

			if(param->prev_tid!=(int32_t)tid || /* not the same chromosome */
			   param->prev_pos+1+param->count_zero!=(int)pos || /* not the expected index  */
			   param->count_zero > param->pref_zero /* too many depth=0 */
			   )
				{
				param->count_zero=0;/* reset count depth=0 */
				/* print WIGGLE header .
				 *
				 * "Wiggle track data values can be integer or real, positive or negative values.
				 *  Chromosome positions are specified as 1-relative.
				 *  For a chromosome of length N, the first position is 1 and the last position is N.
				 *  Only positions specified have data.
				 *  Positions not specified do not have data and will not be graphed.
				 *  All positions specified in the input data must be in numerical order.
				 * */
				fprintf(param->out,"fixedStep chrom=%s start=%d step=1 span=1\n",
					param->in->header->target_name[tid],
					pos+1 //wig first base is 1
					);
				}
			while(param->count_zero >0)
				{
				fputs("0\n",param->out);
				param->count_zero--;
				}
			fprintf(param->out,"%d\n",depth);
			param->prev_pos=(int)pos;
			}
		param->prev_tid=(int)tid;
		}

	return 0;
	}
    void usage(ostream& out,int argc,char **argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\t"<< argv[0] << " [otions] file.bam [chr:start-end]\n";
	    out << "Options:\n";
	    out << " -z <int> number of depth=0 accepted before starting a new WIG file (default:"<< this-> pref_zero <<" ).:\n";
	    out << " -o <filename-out> save as... (default:stdout).\n";
	    out << " -t print a ucsc custom track header.\n";
	    }
	
int main(int argc, char *argv[])
	{
	int optind=1;
	bool header=false;
	char* fileout=NULL;
	
	this->beg = 0;
	this->end = numeric_limits<int32_t>::max();
	this->pref_zero=NUM_ZERO_ACCEPTED_DEFAULT;
	this->count_zero=0;
	this->in=NULL;
	this->out=stdout;
	
	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		        {
		        usage(cout,argc,argv);
		        return EXIT_FAILURE;
		        }
		else if(strcmp(argv[optind],"-o")==0 && optind+1<argc)
			{
			fileout=argv[++optind];
			}
		else if(strcmp(argv[optind],"-t")==0)
			{
			header=true;
			}
		else if(strcmp(argv[optind],"-z")==0 && optind+1<argc)
		        {
			if(!numeric_cast<int32_t>(argv[++optind],&pref_zero))
			    {
			    cerr << "Bad value for option -z " << argv[optind] << endl;
			    usage(cerr,argc,argv);
			    return EXIT_FAILURE;
			    }

		        if(this->pref_zero<0)
		        	{
		        	this->pref_zero=0;
		        	}
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
        
        if(optind==argc)
		{
		usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	this->in = samopen(argv[optind], "rb", 0);
	if (this->in == 0)
		{
		cerr<<  "Cannot open BAM file \""<< argv[optind] << "\". "<< strerror(errno) << endl;
		return EXIT_FAILURE;
		}
	
	if(fileout!=NULL)
		{
		errno=0;
		this->out=fopen(fileout,"w");
		if(this->out==NULL)
			{
			cerr<< "Cannot open \""<< fileout<< "\" "
			    << strerror(errno)
			    << endl;
			return EXIT_FAILURE;
			}
		}
	if(header!=0)
		{
		fputs( "track name=\"__TRACK_NAME__\" description=\"__TRACK_DESC__\" type=\"wiggle_0\"\n",this->out);
		}
	if (optind+1 == argc)
		{
		::sampileup(in, -1, scan_all_genome_func,this);
		}
	else  if (optind+2 == argc)
	        {
		int ref;
		bam_index_t *idx;
		bam_plbuf_t *buf;
		idx = bam_index_load(argv[optind]); // load BAM index
		if (idx == 0)
			{
			cerr <<  "BAM indexed file is not available for "<< argv[optind] << endl;
			return EXIT_FAILURE;
			}
		::bam_parse_region(
			in->header,
			argv[optind+1],
			&ref,
		        &beg, &end); // parse the region
		if (ref < 0)
			{
			cerr<<  "Chromosome is not indexed in BAM:" <<  argv[optind+1] << endl;
			return EXIT_FAILURE;
			}
		buf = bam_plbuf_init( scan_all_genome_func,this); // initialize pileup
		bam_fetch(in->x.bam, idx, ref, this->beg, this->end, buf, fetch_func);
		bam_plbuf_push(0, buf); // finalize pileup
		bam_index_destroy(idx);
		bam_plbuf_destroy(buf);
		}
	else
		{
		cerr<< "illegal number of arguments." << endl;
		usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	::samclose(this->in);
	if(fileout!=NULL)
		{
		fflush(this->out);
		fclose(this->out);
		}
	return EXIT_SUCCESS;
	}
    };

int main(int argc, char *argv[])
    {
    Bam2Wig app;
    return app.main(argc,argv);
    }


