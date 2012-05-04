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
 *	 Extract info field
 * Compilation:
 *	 g++ -o extractsnpeff -Wall -O3 extractsnpeff.cpp -lz
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <string>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <stdint.h>
#include "zstreambuf.h"
#include "tokenizer.h"

using namespace std;

class ExtractSnpEff
	{
	public:
		Tokenizer tokenizer;
		int column;
		string notFound;
		
		ExtractSnpEff():column(7),notFound("N/A")
			{
			}
		
		void run(std::istream& in)
			{
			string line;
			vector<string> tokens;
			vector<string> hash;
			while(getline(in,line,'\n'))
				{
				if(line.empty()) continue;
				if(line[0]=='#')
					{
					if(line.size()>1 && line[1]=='#')
					    {
					    cout << line << endl;
					    continue;
					    }
					cout << line << tokenizer.delim <<
						"Effect"<< tokenizer.delim <<
						"Effefct_Impact"<< tokenizer.delim <<
						"Functional_Class"<< tokenizer.delim <<
						"Codon_Change"<< tokenizer.delim <<
						"Amino_Acid_change"<< tokenizer.delim <<
						"Gene_Name"<< tokenizer.delim <<
						"Gene_BioType"<< tokenizer.delim <<
						"Coding"<< tokenizer.delim <<
						"Transcript"<< tokenizer.delim <<
						"Exon"<< tokenizer.delim <<
						"ERRORS_WARNINGS"
						<< endl;
					continue;
					}

				tokenizer.split(line,tokens);
				if(column>=(int)tokens.size())
					{
					cerr << "Column out of range in " << line << endl;
					continue;
					}

				const size_t NUM_COLS=11;
				vector<string> appendcells;
				appendcells.resize(NUM_COLS,this->notFound);

				for(;;)
				    {
				    string infoContent(tokens[column]);
				    string::size_type effBeg=0;
				    if(infoContent.compare(0,4,"EFF=")==0)
					    {
					    effBeg=0;
					    }
				    else if((effBeg=infoContent.find(";EFF="))==string::npos)
					    {
					    break;
					    }
				    while(infoContent.at(effBeg)!='=') effBeg++;
				    effBeg++;
				    string::size_type opar=infoContent.find('(',effBeg);
				    if(opar==string::npos) break;
				    string effect=infoContent.substr(effBeg,opar-effBeg);
				    if(effect.empty()) break;
				    appendcells[0].assign(effect);
				    string::size_type i=opar+1;
				    string::size_type prev=i;
				    string::size_type fieldIdx=1;
				    bool eof=false;
				    for(;;)
					    {
					    if(i==infoContent.size() || infoContent.at(i)==')')
						    {
						    eof=true;
						    }
					    if(eof || infoContent.at(i)=='|')
						    {
						    if(fieldIdx>=NUM_COLS) break;
						    string s2=infoContent.substr(prev, i-prev);
						    if(!s2.empty())
							    {
							    appendcells[fieldIdx].assign(s2);
							    }
						    if(eof) break;
						    fieldIdx++;
						    prev=i+1;
						    }
					    ++i;
					    }
				    break;
				    }

				
				for(size_t i=0;i< tokens.size();++i)
					{
					if(i>0) cout << tokenizer.delim;
					cout << tokens[i];
					}
				for(size_t i=0;i< appendcells.size();++i)
					{
					cout << tokenizer.delim;
					cout << appendcells[i];
					}

				cout << endl;
				}
			}
			
    void usage(int argc,char** argv)
	    {
	    cerr << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    cerr << "Options:\n";
	    cerr << "  -c <info-column-infex> ("<<  (column+1) << ")" << endl;
	    cerr << "  --delim <column-delimiter> (default:tab)" << endl;
	    cerr << "  -N <string> symbol for NOT-FOUND. default:"<< notFound << endl;
	    cerr << endl;
	    }

    int main(int argc,char** argv)
	    {

	    int optind=1;
	    while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
			{
			usage(argc,argv);
			return(EXIT_FAILURE);
			}
		else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
			    {
			    char* p=argv[++optind];
			    if(strlen(p)!=1)
				{
				fprintf(stderr,"Bad delimiter \"%s\"\n",p);
				return (EXIT_FAILURE);
				}
			    this->tokenizer.delim=p[0];
			    }
		    else if(strcmp(argv[optind],"-N")==0 && optind+1<argc)
			    {
			    this->notFound.assign(argv[++optind]);
			    }
		else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
			    {
			    char* p2;
			    this->column=(int)strtol(argv[++optind],&p2,10);
			    if(*p2!=0 || this->column<1)
				{
				fprintf(stderr,"Bad INFO column:%s\n",argv[optind]);
				return EXIT_FAILURE;
				}
			    this->column--;/* to 0-based */
			    }
		else if(strcmp(argv[optind],"--")==0)
			    {
			    ++optind;
			    break;
			    }
		 else if(argv[optind][0]=='-')
			    {
			    cerr << "unknown option '" << argv[optind]<< "'" << endl;
			    return EXIT_FAILURE;
			    }
		else
			    {
			    break;
			    }
		++optind;
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
			    ++optind;
			    }
		    }
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
	{
	ExtractSnpEff app;
	return app.main(argc,argv);
	}
