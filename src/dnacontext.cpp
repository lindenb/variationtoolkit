/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	Oct 2011
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	DNA context and %GC
 */
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cctype>
#include <iostream>
#include <stdint.h>
#include "zstreambuf.h"
#include "tokenizer.h"
#include "xfaidx.h"
#include "numeric_cast.h"
using namespace std;

class DNAContext
	{
	public:
		struct GCPercent
		    {
		    double gc;
		    double total;
		    };
		IndexedFasta* faidx;
		Tokenizer tokenizer;
		int32_t chromColumn;
		int32_t posColumn;//or end column
		int extend;
		int32_t endColumn;
		bool print_gc;
		bool print_seq;
		DNAContext():faidx(NULL),chromColumn(0),
			posColumn(1),
			extend(10),
			endColumn(-1),
			print_gc(true),
			print_seq(true)
			{
			tokenizer.delim='\t';
			}
		~DNAContext()
		    {
		    if(faidx!=NULL) delete faidx;
		    }

		void fillGC(GCPercent* gc,const std::string* dna)
		    {
		    for(string::size_type i=0;i< dna->size();++i)
			{
			switch(toupper(dna->at(i)))
			    {
			    case 'G':
			    case 'C':
			    case 'W': gc->gc+=1.0; break;
			    default:break;
			    }
			}
		    gc->total+= dna->size();
		    }

		void run(std::istream& in)
		    {
		    string line;
		    vector<string> tokens;
		    while(getline(in,line,'\n'))
			{
			if(line.empty()) continue;
			if(line[0]=='#')
				{
				if(line.size()>1 && line[1]=='#') continue;
				if(endColumn==-1)
				    {
				    cout << line;
				    if(print_seq)
					{
					cout << tokenizer.delim
					    << "LEFT(DNA)" << tokenizer.delim
					    << "CONTEXT(DNA)" << tokenizer.delim
					    << "RIGHT(DNA)";
					}
				    if(print_gc) cout << tokenizer.delim << "%GC";
				    cout << endl;
				    }
				else
				    {
				    cout << line;
				    if(print_seq ) cout << tokenizer.delim << "DNA";
				    if(print_gc ) cout << tokenizer.delim << "%GC";
				    cout << endl;
				    }
				cout.flush();
				continue;
				}
			tokenizer.split(line,tokens);
			if(chromColumn>= (int)tokens.size())
			    {
			    cerr << "[error] CHROM column "<< (chromColumn+1) << " for " << line << endl;
			    continue;
			    }
			if(posColumn>= (int)tokens.size())
			    {
			    cerr << "[error] POS/START column "<< (posColumn+1) << " for " << line << endl;
			    continue;
			    }


			int32_t pos=-1;


			if(!numeric_cast<int32_t>(tokens[posColumn].c_str(),&pos) || pos<0)
			    {
			    cerr << "bad pos/start in " << line << endl;
			    continue;
			    }


			if(endColumn==-1)
			    {
			    pos-=1;//0 based
			    GCPercent gcPercent;
			    gcPercent.gc=0;
			    gcPercent.total=0;
			    cout << line;
			    const char* chrom=tokens[chromColumn].c_str();
			    auto_ptr<string> dna=faidx->fetch(chrom,max(0,pos-extend),max(pos-1,0));
			    if(dna.get()==NULL)
				{
				if(print_seq) cout << tokenizer.delim << "#ERR";
				}
			    else
				{
				fillGC(&gcPercent,dna.get());
				if(print_seq) cout << tokenizer.delim << *(dna);
				}
			    dna=faidx->fetch(chrom,pos,pos);
			    if(dna.get()==NULL)
				{
				if(print_seq) cout << tokenizer.delim << "#ERR";
				}
			    else
				{
				fillGC(&gcPercent,dna.get());
				if(print_seq) cout << tokenizer.delim << *(dna);
				}
			    dna=faidx->fetch(chrom,pos+1,pos+extend);
			    if(dna.get()==NULL)
				{
				if(print_seq)  cout<< tokenizer.delim << "#ERR";
				}
			    else
				{
				fillGC(&gcPercent,dna.get());
				if(print_seq)  cout<< tokenizer.delim << *(dna);
				}
			    if(print_gc)
				{
				cout << tokenizer.delim;
				if(gcPercent.total<=0)
				    {
				    cout << ".";
				    }
				else
				    {
				    cout << (gcPercent.gc/gcPercent.total);
				    }
				}
			    cout << endl;
			    }
			else
			    {
			    if(endColumn>=(int32_t)tokens.size())
				{
				cerr << "[error] bad END column "<< (endColumn+1) << " for " << line << endl;
				continue;
				}
			    int32_t chromEnd;
			    if(!numeric_cast<int32_t>(tokens[endColumn].c_str(),&chromEnd))
				{
				cerr << "bad end Column in " << line << endl;
				continue;
				}
			    if(pos>chromEnd)
				{
				cerr << "start>end in "<< line << endl;
				continue;
				}
			    cout << line;
			    const char* chrom=tokens[chromColumn].c_str();
			    auto_ptr<string> dna=faidx->fetch(chrom,pos,chromEnd);

			    GCPercent gcPercent;
			    gcPercent.gc=0;
			    gcPercent.total=0;
			    if(dna.get()==NULL)
				{
				if(print_seq)  cout << tokenizer.delim << "#ERR";
				}
			    else
				{
				fillGC(&gcPercent,dna.get());
				if(print_seq)  cout << tokenizer.delim << *(dna);
				}
			    if(print_gc)
				{
				cout << tokenizer.delim;
				if(gcPercent.total<=0)
				    {
				    cout << ".";
				    }
				else
				    {
				    cout << (gcPercent.gc/gcPercent.total);
				    }
				}
			    cout << endl;
			    }
			}
		    }


	void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Extracts DNA context from indexed Faidx genome and prints the %GC.\n";
	    cerr << "Usage" << endl
		    << "   "<< argv[0]<< " [options] -f genome.fa (files|stdin)"<< endl;
	    cerr << "Options:\n";
	    cerr << "  -c <chrom Column> ("<<  (chromColumn+1) << ")" << endl;
	    cerr << "  -p|-s <pos or start Column> ("<<  (posColumn+1) << ")" << endl;
	    cerr << "  -d <column-delimiter> (default:tab)" << endl;
	    cerr << "  -x <segment-size> (default:"<< extend <<"). Assumes VCF data (first base=1)" << endl;
	    cerr << "  -e <end-column> . exclusive of -x . Assumes BED data (first base=0)"  << endl;
	    cerr << "  -f <genome file indexed with tabix." << endl;
	    cerr << "  --no-gc don't print gc%" << endl;
	    cerr << "  --no-seq don't print DNA sequence" << endl;
	    cerr << endl;
	    }


    int main(int argc,char** argv)
	{
	int optind=1;
	const char* faidxfile=NULL;
	while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			    {
			    this->usage(argc,argv);
			    return (EXIT_FAILURE);
			    }
		    else if(std::strcmp(argv[optind],"--no-gc")==0)
			{
			print_gc=false;
			}
		    else if(std::strcmp(argv[optind],"--no-seq")==0)
			{
			print_seq=false;
			}
		    else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
			    {
			    if(!numeric_cast<int32_t>(argv[++optind],&chromColumn) || this->chromColumn<1)
				{
				cerr << "Illegal number for CHROM." << endl;
				return EXIT_FAILURE;
				}
			    chromColumn--;
			    }
		    else if((std::strcmp(argv[optind],"-p")==0 ||
			    std::strcmp(argv[optind],"-s")==0) && optind+1<argc)
			{
			if(!numeric_cast<int32_t>(argv[++optind],&posColumn) || this->posColumn<1)
			    {
			    cerr << "Illegal number for POS/START" << endl;
			    usage(argc,argv);
			    return EXIT_FAILURE;
			    }
			posColumn--;
			}
		    else if(std::strcmp(argv[optind],"-e")==0 && optind+1<argc)
			{
			if(!numeric_cast<int32_t>(argv[++optind],&endColumn) || this->endColumn<1)
			    {
			    cerr << "Illegal number for END" << endl;
			    usage(argc,argv);
			    return EXIT_FAILURE;
			    }
			endColumn--;
			}
		    else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			faidxfile=argv[++optind];
			}
		    else if(std::strcmp(argv[optind],"-x")==0 && optind+1<argc)
			{
			if(endColumn!=-1)
			    {
			    cerr << "-x and -e exclusive options.\n";
			    usage(argc,argv);
			    return EXIT_FAILURE;
			    }
			if(!numeric_cast<int32_t>(argv[++optind],&extend) || this->extend<0)
			    {
			    cerr << "Illegal number for 'extend'" << endl;
			    usage(argc,argv);
			    return EXIT_FAILURE;
			    }
			}
		    else if(std::strcmp(argv[optind],"--delim")==0 && optind+1< argc)
			    {
			    char* p=argv[++optind];
			    if(strlen(p)!=1)
				    {
				    cerr << "Bad delimiter \""<< p << "\"\n";
				    this->usage(argc,argv);
				    return(EXIT_FAILURE);
				    }
			    this->tokenizer.delim=p[0];
			    }
		    else if(argv[optind][0]=='-')
			    {
			    cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
			    this->usage(argc,argv);
			    return (EXIT_FAILURE);
			    }
		    else
			    {
			    break;
			    }
		    ++optind;
		    }

	if(faidxfile==NULL)
	    {
	    cerr << "Undefined genome file."<< endl;
	    this->usage(argc,argv);
	    return (EXIT_FAILURE);
	    }
	this->faidx=new IndexedFasta(faidxfile);

	if(optind==argc)
		    {
		    igzstreambuf buf;
		    istream in(&buf);
		    this->run(in);
		    }
	else
		    {
		    while(optind<argc)
			    {
			    igzstreambuf buf(argv[optind++]);
			    istream in(&buf);
			    this->run(in);
			    buf.close();
			    }
		    }
	return EXIT_SUCCESS;
	}
    };

int main(int argc,char** argv)
    {
    DNAContext app;
    return app.main(argc,argv);
    }
