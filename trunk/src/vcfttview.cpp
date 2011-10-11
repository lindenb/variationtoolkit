/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Orginal source code:
 *	bam_tview.c in http://samtools.sourceforge.net/
 *	Authors: Heng Li, Bob Handsaker, Jue Ruan, Colin Hercus, Petr Danecek
 * Contact:
 *	plindenbaum@yahoo.fr
 * Reference:
 *	http://plindenbaum.blogspot.com/2011/07/text-alignment-viewer-using-samtools.html
 * WWW:
 *	http://plindenbaum.blogspot.com
 *	http://samtools.sourceforge.net/
 * Motivation:
 *	Text alignment viewer using the samtools API
 */
#include <cctype>
#include <cassert>
#include <cstring>
#include <cmath>
#include <map>
#include <cstdarg>
#include <cerrno>
#include <limits>
#include <iostream>
#include <cstdlib>
#include "throw.h"
#include "ttview.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "smartcmp.h"
using namespace std;




class VCFTTView
    {
    public:
	Tokenizer tokenizer;
	TTView tv;
	int chromColumn;
	int posColumn;
	int sampleColumn;
	int shift;
	typedef map<std::string,BamFile*,SmartComparator> sample2bam_map;
	typedef map<std::string,std::string,SmartComparator> sample2file_map;
	sample2bam_map sample2bam;
	sample2file_map sample2file;
	BamFile* mainBam;
	IndexedFasta* faidx;
	bool print_all_bam;

	VCFTTView():chromColumn(0),posColumn(1),sampleColumn(-1),
		    shift(10),
		    mainBam(NULL),
		    faidx(NULL),
		    print_all_bam(false)
	    {
	    tokenizer.delim='\t';
	    }

	~VCFTTView()
	    {
	    if(mainBam!=NULL) delete mainBam;
	    if(faidx!=NULL) delete faidx;
	    for(sample2bam_map::iterator r=sample2bam.begin();
		r!=sample2bam.end();
		++r)
		{
		delete r->second;
		}
	    }
	void loadSample2Bam(const char* fname)
	    {
	    Tokenizer tab;
	    vector<string> tokens;
	    igzstreambuf buf(fname);
	    istream in(&buf);
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		tab.split(line,tokens);
		if(tokens.size()<2) THROW("column missing in "<< line);
		BamFile* bam=new BamFile(tokens[1].c_str());

		sample2bam.insert(make_pair(tokens[0],bam));
		sample2file.insert(make_pair(tokens[0],tokens[1]));
		}
	    buf.close();
	    }
	void run(std::istream& in)
	    {
	    string line;
	    vector<string> tokens;
	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(line.at(0)=='#') continue;
		tokenizer.split(line,tokens);
		int32_t pos;


		if(mainBam!=NULL && sampleColumn==-1)
		    {
		    tv.print(
			   cout,
			   tokens[chromColumn].c_str(),
			   pos+shift,
			   mainBam,
			   faidx
			   );
		    }
		else if(print_all_bam && !sample2bam.empty())
		    {
		    for(sample2bam_map::iterator r=sample2bam.begin();
		    		r!=sample2bam.end();
		    		++r)
			{
			tv.print(
			   cout,
			   tokens[chromColumn].c_str(),
			   pos+shift,
			   r->second,
			   faidx
			   );
			}
		    }
		else if(!sample2bam.empty() && sampleColumn!=-1)
		    {
		    sample2bam_map::iterator r=sample2bam.find(tokens[sampleColumn]);
		    if(r==sample2bam.end())
			{
			cerr << "No BAM fdefined for sample "<<tokens[sampleColumn] << endl;
			continue;
			}
		    tv.print(
		       cout,
		       tokens[chromColumn].c_str(),
		       pos+shift,
		       r->second,
		       faidx
		       );
		    }
		else
		    {
		    THROW("Runtime Error: Illegal parameters.");
		    }
		}
	    }

	void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Usage" << endl
		    << "   "<< argv[0]<< " [options] -v val (file|stdin)"<< endl;
	    cerr << "Options:\n";
	    cerr << "  -c <chrom Column> ("<<  (chromColumn+1) << ")" << endl;
	    cerr << "  -p <pos Column> ("<<  (posColumn+1) << ")" << endl;
	    cerr << "  -s <sample Column> ("<<  (sampleColumn+1) << ") [optional]" << endl;
	    cerr << "  -d <column-delimiter> (default:tab)" << endl;
	    cerr << endl;
	    }
    };


int main(int argc, char **argv)
	{
	VCFTTView app;

	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		    {
		    app.usage(argc,argv);
		    exit(EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
		    {
		    char* p2;
		    app.chromColumn=(int)strtol(argv[++optind],&p2,10)-1;
		    if(app.chromColumn<0 || *p2!=0)
			{
			cerr << "Illegal number for CHROM" << endl;
			return EXIT_FAILURE;
			}
		    }
		else if(std::strcmp(argv[optind],"-p")==0 && optind+1<argc)
		    {
		    char* p2;
		    app.posColumn=(int)strtol(argv[++optind],&p2,10)-1;
		    if(app.posColumn<0 || *p2!=0)
			{
			cerr << "Illegal number for POS" << endl;
			return EXIT_FAILURE;
			}
		    }
		else if(std::strcmp(argv[optind],"-s")==0 && optind+1<argc)
		    {
		    char* p2;
		    app.sampleColumn=(int)strtol(argv[++optind],&p2,10)-1;
		    if(app.sampleColumn<0 || *p2!=0)
			{
			cerr << "Illegal number for SAMPLE" << endl;
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
		        fprintf(stderr,"%s: unknown option '%s'\n",argv[0],argv[optind]);
		        app.usage(argc,argv);
		        return (EXIT_FAILURE);
		        }
		else
		        {
		        break;
		        }
		++optind;
		}
	


	return 0;
	}

