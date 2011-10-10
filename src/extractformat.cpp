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
 *	 g++ -o extractinfo -Wall -O3 extractinfo.cpp -lz
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

class ExtractFormat
	{
	public:
		Tokenizer tokenizer;
		Tokenizer colon;
		int formatcolumn;
		int callcolumn;
		string tag;
		string notFound;
		
		ExtractFormat():formatcolumn(8),callcolumn(9),notFound("N/A")
			{
			colon.delim=':';
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
					cout << line << tokenizer.delim << tag << endl;
					continue;
					}
				tokenizer.split(line,tokens);
				if(formatcolumn>=(int)tokens.size())
					{
					cerr << "Column FORMAT out of range in " << line << endl;
					continue;
					}
				if(callcolumn>=(int)tokens.size())
					{
					cerr << "Column CALL out of range in " << line << endl;
					continue;
					}
					
				for(size_t i=0;i< tokens.size();++i)
					{
					if(i>0) cout << tokenizer.delim;
					cout << tokens[i];
					}
				string content(notFound);
				int tagIndex=-1;
				colon.split(tokens[formatcolumn],hash);
				for(size_t i=0;i< hash.size();++i)
					{
					if(hash[i].compare(tag)==0)
						{
						tagIndex=(int)i;
						break;
						}
					}
				if(tagIndex!=-1)
					{
					colon.split(tokens[callcolumn],hash);
					if(tagIndex<(int)hash.size())
						{
						content.assign(hash[tagIndex]);
						}
					}
				cout << tokenizer.delim << content << endl;
				}
			}
			
		void usage(int argc,char** argv)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Options:\n";
			cerr << "  -f <format-column-infex> ("<<  formatcolumn << ")" << endl;
			cerr << "  -c <call-column-infex> ("<<  callcolumn << ")" << endl;
			cerr << "  --delim <column-delimiter> (default:tab)" << endl;
			cerr << "  -t <tag> (required)" << endl;
			cerr << "  -N <string> symbol for NOT-FOUND. default:"<< notFound << endl;
			}
	};

int main(int argc,char** argv)
	{
	ExtractFormat app;


	int optind=1;
	while(optind < argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		    {
		   	app.usage(argc,argv);
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
			app.tokenizer.delim=p[0];
			}
		else if(strcmp(argv[optind],"-N")==0 && optind+1<argc)
			{
			app.notFound.assign(argv[++optind]);
			}
	  	else if(strcmp(argv[optind],"-t")==0 && optind+1<argc)
			{
			app.tag.assign(argv[++optind]);
			}
	    else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			char* p2;
			app.formatcolumn=(int)strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.formatcolumn<1)
			    {
			    fprintf(stderr,"Bad FORMAT column:%s\n",argv[optind]);
			    return EXIT_FAILURE;
			    }
			app.formatcolumn--;/* to 0-based */
			}
		else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			char* p2;
			app.callcolumn=(int)strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.callcolumn<1)
			    {
			    fprintf(stderr,"Bad CALL column:%s\n",argv[optind]);
			    return EXIT_FAILURE;
			    }
			app.callcolumn--;/* to 0-based */
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
	if(app.callcolumn==app.formatcolumn)
		{
		cerr << "FORMAT column=CALL column" << endl;
		return EXIT_FAILURE;
		}
	if(app.tag.empty())
		{
		cerr << "undefined tag" << endl;
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
	return EXIT_SUCCESS;
	}

