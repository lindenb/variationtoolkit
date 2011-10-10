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

class ExtractInfo
	{
	public:
		Tokenizer tokenizer;
		Tokenizer semicolon;
		int column;
		string tag;
		string notFound;
		bool ignore_if_not_found;
		
		ExtractInfo():column(7),notFound("N/A"),ignore_if_not_found(false)
			{
			semicolon.delim=';';
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
				if(column>=(int)tokens.size())
						{
						cerr << "Column out of range in " << line << endl;
						continue;
						}
				
				bool found=false;
				string content(notFound);
				semicolon.split(tokens[column],hash);
				for(size_t i=0;i< hash.size();++i)
					{
					string::size_type n=hash[i].find('=');
					if(n==string::npos)
						{
						if(hash[i].compare(tag)==0)
							{
							found=true;
							content.assign("true");
							break;
							}
						}
					else if(hash[i].substr(0,n).compare(tag)==0)
						{
						found=true;
						content.assign(hash[i].substr(n+1));
						break;
						}
					}
				if(!found && ignore_if_not_found) continue;
				for(size_t i=0;i< tokens.size();++i)
					{
					if(i>0) cout << tokenizer.delim;
					cout << tokens[i];
					}
				cout << tokenizer.delim << content << endl;
				}
			}
			
		void usage(int argc,char** argv)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Options:\n";
			cerr << "  -c <info-column-infex> ("<<  column << ")" << endl;
			cerr << "  --delim <column-delimiter> (default:tab)" << endl;
			cerr << "  -t <tag> (required)" << endl;
			cerr << "  -N <string> symbol for NOT-FOUND. default:"<< notFound << endl;
			cerr << "  -i ignore line if tag was not found" << endl;
			}
	};

int main(int argc,char** argv)
	{
	ExtractInfo app;


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
	    else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			char* p2;
			app.column=(int)strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.column<1)
			    {
			    fprintf(stderr,"Bad INFO column:%s\n",argv[optind]);
			    return EXIT_FAILURE;
			    }
			app.column--;/* to 0-based */
			}
		else if(strcmp(argv[optind],"-i")==0)
			{
			app.ignore_if_not_found=true;
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

