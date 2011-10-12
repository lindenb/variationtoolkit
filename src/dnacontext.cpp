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
 *	stupid split data for a given numeric value
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
using namespace std;

class DNAContext
	{
	public:

		IndexedFasta* faidx;
		Tokenizer tokenizer;
		int32_t chromColumn;
		int32_t posColumn;
		int extend;
		DNAContext():faidx(NULL),chromColumn(0),
			posColumn(1),
			extend(10)
			{
			tokenizer.delim='\t';
			}
		~DNAContext()
		    {
		    if(faidx!=NULL) delete faidx;
		    }
		void usage(int argc,char** argv)
		    {
		    cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
		    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    cerr << "Usage" << endl
			    << "   "<< argv[0]<< " [options] -f genome.fa (files|stdin)"<< endl;
		    cerr << "Options:\n";
		    cerr << "  -c <chrom Column> ("<<  (chromColumn+1) << ")" << endl;
		    cerr << "  -p <pos Column> ("<<  (posColumn+1) << ")" << endl;
		    cerr << "  -d <column-delimiter> (default:tab)" << endl;
		    cerr << "  -x <segment-size> (default:"<< extend <<")" << endl;
		    cerr << "  -f <genome fle indexed with tabix." << endl;
		    cerr << endl;
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
				cout << line << tokenizer.delim
					<< "LEFT(DNA)" << tokenizer.delim
					<< "CONTEXT(DNA)" << tokenizer.delim
					<< "RIGHT(DNA)"
					<< endl;
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
			    cerr << "[error] POS column "<< (posColumn+1) << " for " << line << endl;
			    continue;
			    }

			char* p2;

			int32_t pos=(int32_t)strtod(tokens[posColumn].c_str(),&p2);
			if(*p2!=0 )
				{
				cerr << "bad pos in " << line << endl;
				continue;
				}
			pos-=1;//0 based
			cout << line << tokenizer.delim;
			const char* chrom=tokens[chromColumn].c_str();
			auto_ptr<string> dna=faidx->fetch(chrom,max(0,pos-extend),max(pos-1,0));
			if(dna.get()==NULL)
			    {
			    cout << "#ERR";
			    }
			else
			    {
			    cout << *(dna);
			    }
			cout << tokenizer.delim;
			dna=faidx->fetch(chrom,pos,pos);
			if(dna.get()==NULL)
			    {
			    cout << "#ERR";
			    }
			else
			    {
			    cout << *(dna);
			    }
			cout << tokenizer.delim;
			dna=faidx->fetch(chrom,pos+1,pos+extend);
			if(dna.get()==NULL)
			    {
			    cout << "#ERR";
			    }
			else
			    {
			    cout << *(dna);
			    }
			cout << endl;
			}
		    }
	};

int main(int argc,char** argv)
    {
    DNAContext app;
    int optind=1;
    const char* faidxfile=NULL;
    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
			{
			app.usage(argc,argv);
			return (EXIT_FAILURE);
			}
		else if(std::strcmp(argv[optind],"-c")==0)
			{
			char* p2;
			app.chromColumn=(int)strtol(argv[++optind],&p2,10)-1;
			if(app.chromColumn<0 || *p2!=0)
				{
				cerr << "Illegal number for CHROM" << endl;
				return EXIT_FAILURE;
				}
			}
		else if(std::strcmp(argv[optind],"-p")==0)
			{
			char* p2;
			app.posColumn=(int)strtol(argv[++optind],&p2,10)-1;
			if(app.posColumn<0 || *p2!=0)
			{
			cerr << "Illegal number for POS" << endl;
			return EXIT_FAILURE;
			}
			}
		else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			faidxfile=argv[++optind];
			}
		else if(std::strcmp(argv[optind],"-x")==0 && optind+1<argc)
			{
			char* p2;
			app.extend=(int)strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.extend <0)
				{
				cerr << "Illegal number for 'extend'" << endl;
				return EXIT_FAILURE;
				}
			}
		else if(std::strcmp(argv[optind],"--delim")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
				{
				cerr << "Bad delimiter \""<< p << "\"\n";
				app.usage(argc,argv);
				return(EXIT_FAILURE);
				}
			app.tokenizer.delim=p[0];
			}
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
			app.usage(argc,argv);
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
		app.usage(argc,argv);
		return (EXIT_FAILURE);
		}
    app.faidx=new IndexedFasta(faidxfile);

    if(optind==argc)
		{
		igzstreambuf buf;
		istream in(&buf);
		app.run(in);
		}
    else
		{
		while(optind<argc)
			{
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			app.run(in);
			buf.close();
			}
		}
    return EXIT_SUCCESS;
    }
