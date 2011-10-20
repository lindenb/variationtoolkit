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
 *	verticalize a table
 * Compilation:
 *	 g++ -o verticalize -Wall -O3 verticalize.cpp -lz
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
#include "application.h"

using namespace std;


class Verticalize:public AbstractApplication
    {
    public:
	   bool first_line_is_header;
	Verticalize():first_line_is_header(true)
	    {

	    }
	~Verticalize()
	    {
	    }




	void run(std::istream& in)
	    {
	    size_t nLine=0UL;
	    vector<string> header;
	    vector<string> tokens;
	    string line;
	    size_t len_word=0UL;
	    if(first_line_is_header)
		{
		while(getline(in,line,'\n'))
		    {
		    if(AbstractApplication::stopping()) break;
		    ++nLine;
		    if(line.empty()) continue;
		    if(line.size()>1 && line[0]=='#' && line[1]=='#')
			{
			cout << line << endl;
			continue;
			}
		    tokenizer.split(line,header);
		    for(size_t i=0;i< header.size();++i) len_word=max(len_word,header[i].size());
		    break;
		    }
		}

	    while(getline(in,line,'\n'))
		{
		if(AbstractApplication::stopping()) break;
		++nLine;
		cout << ">>>"<< tokenizer.delim << (nLine)<< endl;
		tokenizer.split(line,tokens);
		if(first_line_is_header)
		    {
		    for(size_t i=0;i< header.size();++i)
			{
			cout << "$"<<(i+1)<< tokenizer.delim << header[i];
			for(size_t j=header[i].size();j< len_word;++j)
			    {
			    cout << " ";
			    }
			cout <<tokenizer.delim;
			if(i<tokens.size())
			    {
			    cout << tokens[i];
			    }
			else
			    {
			    cout << "???";
			    }
			cout <<endl;
			}
		    for(size_t i=header.size();i< tokens.size();++i)
			{
			cout << "$"<<(i+1);
			if(i+1<100) cout << " ";
			if(i+1<10) cout << " ";
			cout <<tokenizer.delim << "???";
			for(size_t j=3;j< len_word;++j)
			    {
			    cout << " ";
			    }
			cout << tokenizer.delim << tokens[i] << endl;
			}
		    }
		else
		    {
		    for(size_t i=header.size();i< tokens.size();++i)
			{
			cout << "$"<<(i+1);
			if(i+1<100) cout << " ";
			if(i+1<10) cout << " ";
			cout <<tokenizer.delim << tokens[i] << endl;
			}
		    }
		cout << "<<<"<< tokenizer.delim << (nLine)<< "\n\n";
		}
	    }
	void usage(int argc,char** argv)
		{
		cerr << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
		cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		cerr << "Options:\n";
		cerr << "  -d or --delim (char) delimiter default:tab\n";
		cerr << "  -n first line is NOT the header.\n";
		cerr << "(stdin|file|file.gz)\n";
		}

    };


int main(int argc,char** argv)
    {
    Verticalize app;
    int optind=1;
    while(optind < argc)
   		{
   		if(std::strcmp(argv[optind],"-h")==0)
   			{
   			app.usage(argc,argv);
   			return EXIT_FAILURE;
   			}

   		else if(std::strcmp(argv[optind],"-n")==0)
   			{
   			app.first_line_is_header =false;
   			}
   		else if((std::strcmp(argv[optind],"-d")==0 ||
   			 std::strcmp(argv[optind],"--delim")==0)
   			&& optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    cerr << "Bad delimiter \""<< p << "\"\n";
			    app.usage(argc,argv);
			    exit(EXIT_FAILURE);
			    }
			app.tokenizer.delim=p[0];
			}
   		else if(argv[optind][0]=='-')
   			{
   			cerr << "unknown option '"<< argv[optind] <<"'"<< endl;
   			app.usage(argc,argv);
   			exit(EXIT_FAILURE);
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
	    app.run(in);
	    }
    else
	    {
	    while(optind< argc)
		{
		if(AbstractApplication::stopping()) break;
		igzstreambuf buf(argv[optind++]);
		istream in(&buf);
		app.run(in);
		buf.close();
		++optind;
		}
	    }
    return EXIT_SUCCESS;
    }
