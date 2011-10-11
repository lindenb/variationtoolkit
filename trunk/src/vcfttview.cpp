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
#include "where.h"
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
	void pushSample2Bam(string sample,string path)
	    {
	    BamFile* bam=new BamFile(path.c_str());
	    sample2bam.insert(make_pair(sample,bam));
	    sample2file.insert(make_pair(sample,path));
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
		pushSample2Bam(tokens[0],tokens[1]);
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

		if(chromColumn>= (int)tokens.size())
		    {
		    cerr << "[error] CHROM column "<< (chromColumn+1) << " for " << line << endl;
		    continue;
		    }
		if(posColumn>= (int)tokens.size())
		    {
		    cerr << "[error] POS column "<< (posColumn+1) << " for " << line << endl;
		    continue;
		    }
		if(sampleColumn!=-1 && sampleColumn>= (int)tokens.size())
		    {
		    cerr << "[error] SAMPLE column "<< (sampleColumn+1) << " for " << line << endl;
		    continue;
		    }
		char* p2;

		int32_t pos=(int32_t)strtod(tokens[posColumn].c_str(),&p2);
		if(*p2!=0 || pos<1)
		    {
		    cerr << "bad pos (" << pos << ") in line \"" << line << "\"" << endl;
		    continue;
		    }



		if(print_all_bam && !sample2bam.empty())
		    {
		    cout << ">>" << tokens[0] << ":" << pos << endl;
		    for(sample2bam_map::iterator r=sample2bam.begin();
		    		r!=sample2bam.end();
		    		++r)
			{
			cout << "> " << tokens[0] << ":" << pos << "\t" << sample2file[r->first]<< "\n\n";
			tv.print(
			   cout,
			   tokens[chromColumn].c_str(),
			   max(0,(pos-1)-shift),
			   r->second,
			   faidx
			   );
			cout << endl;
			}
		    cout << "\n\n";
		    }
		else if(mainBam!=NULL && sampleColumn==-1)
		    {
		    cout << ">" << tokens[0] << ":" << pos << "\n\n";
		    tv.print(
			   cout,
			   tokens[chromColumn].c_str(),
			   max(0,(pos-1)-shift),
			   mainBam,
			   faidx
			   );
		    cout << "\n\n";
		    }
		else if(!sample2bam.empty() && sampleColumn!=-1)
		    {
		    sample2bam_map::iterator r=sample2bam.find(tokens[sampleColumn]);
		    if(r==sample2bam.end())
			{
			cerr << "No BAM fdefined for sample "<<tokens[sampleColumn] << endl;
			continue;
			}
		    cout << ">" << tokens[0] << ":" << pos << "\t"  << sample2file[r->first] << "\n\n";
		    tv.print(
		       cout,
		       tokens[chromColumn].c_str(),
		       max(0,(pos-1)-shift),
		       r->second,
		       faidx
		       );
		    cout << endl;
		    }
		else
		    {
		    THROW("Runtime Error: Illegal parameters.");
		    }
		cout << endl;
		}
	    }

	void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    cerr << "Original code from samtools ttview : Heng Li, Bob Handsaker, Jue Ruan, Colin Hercus, Petr Danecek\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Usage" << endl
		    << "   "<< argv[0]<< " [options] (file|stdin)"<< endl;
	    cerr << "Options:\n";
	    cerr << "  -c <chrom Column> ("<<  (chromColumn+1) << ")" << endl;
	    cerr << "  -p <pos Column> ("<<  (posColumn+1) << ")" << endl;
	    cerr << "  -s <sample Column> ("<<  (sampleColumn+1) << ") [optional]" << endl;
	    cerr << "  -d <column-delimiter> (default:tab)" << endl;
	    cerr << "  -B <bam-file> [defines one main bam for all data]" << endl;
	    cerr << "  -f <file> loads a file tab delimited with SAMPLE-NAME\\tPATH-TO-BAM" << endl;
	    cerr << "  -F <SAMPLE> <FILE>  push a SAMPLE-NAME/PATH-TO-BAM in the current list" << endl;
	    cerr << "  -a for one position, print all BAM" << endl;
	    cerr << "  -x <int> shift x bases to the right: default" << shift << endl;
	    cerr << "  -w <int> screen width default:" << tv.mcol << endl;
	    cerr << "  -R <fasta> reference file indexed with samtools faidx "<< endl;
	    cerr << endl;
	    }
    };


int main(int argc, char **argv)
	{
	VCFTTView app;
	char* mainBamFile=NULL;
	char* faidFile=NULL;
	while(optind < argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		{
		app.usage(argc,argv);
		exit(EXIT_FAILURE);
		}
	    else if(std::strcmp(argv[optind],"-a")==0)
		{
		app.print_all_bam=true;
		}
	    else if(std::strcmp(argv[optind],"-R")==0 && optind+1<argc)
		{
		faidFile=(argv[++optind]);
		}
	    else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
		{
		app.loadSample2Bam(argv[++optind]);
		}
	    else if(std::strcmp(argv[optind],"-F")==0 && optind+2<argc)
		{
		string sample1(argv[++optind]);
		string bam1(argv[++optind]);
		app.pushSample2Bam(sample1,bam1);
		}
	    else if(std::strcmp(argv[optind],"-B")==0 && optind+1<argc)
		{
		mainBamFile=argv[++optind];
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
	    else if(std::strcmp(argv[optind],"-x")==0 && optind+1<argc)
		{
		char* p2;
		app.shift=(int)strtol(argv[++optind],&p2,10);
		if(app.shift<0 || *p2!=0)
		    {
		    cerr << "Illegal value for shift" << endl;
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"-w")==0 && optind+1<argc)
		{
		char* p2;
		app.tv.mcol=(int)strtol(argv[++optind],&p2,10);
		if(app.tv.mcol<1 || *p2!=0)
		    {
		    cerr << "Illegal value for shift" << endl;
		    return EXIT_FAILURE;
		    }
		app.tv.mcol=min(app.tv.mcol,10);
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
	if(faidFile!=NULL)
	    {
	    app.faidx=new IndexedFasta(faidFile);
	    }
	if(mainBamFile!=NULL)
	    {
	    app.mainBam=new BamFile(mainBamFile);
	    }
	if(app.sample2bam.empty() && app.mainBam==NULL)
	    {
	    cerr << "No main bam or sample associated to bam" << endl;
	    app.usage(argc,argv);
	    return EXIT_FAILURE;
	    }
	if(optind==argc)
	    {
	    igzstreambuf buf;
	    istream in(&buf);
	    app.run(in);
	    }
	else
	    {
	    while(optind< argc)
		    {
		    igzstreambuf buf(argv[optind++]);
		    istream in(&buf);
		    app.run(in);
		    buf.close();
		    ++optind;
		    }
	    }
	return 0;
	}

