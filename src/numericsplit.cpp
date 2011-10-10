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
#include "zstreambuf.h"
#include "tokenizer.h"
using namespace std;

class NumericSplit
	{
	public:
		Tokenizer tokenizer;
		int column;
		double* minVal;
		double* maxVal;
		bool inverse;
		NumericSplit():column(-1),minVal(NULL),maxVal(NULL),inverse(false)
			{
			tokenizer.delim='\t';
			}

		void usage(int argc,char** argv)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Options:\n";
			cerr << "  -c <column-infex> ("<<  column << ")" << endl;
			cerr << "  --delim <column-delimiter> (default:tab)" << endl;
			cerr << "  -m <min-value>" << endl;
			cerr << "  -M <max-value>" << endl;
			cerr << "  -v Inverse" << endl;
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
					cout << line << endl;
					continue;
					}
				tokenizer.split(line,tokens);
				if(column>= (int)tokens.size())
					{
					cerr << "Cannot find column "<< (column+1) << " in " << line << endl;
					continue;
					}
				char* p2;
				double v=strtod(tokens[column].c_str(),&p2);
				if(*p2!=0)
					{
					cerr << "bad number in " << line << endl;
					continue;
					}
				bool keep=true;
				if(minVal!=NULL && v<*minVal)
					{
					keep=false;
					}
				if(maxVal!=NULL && v>*maxVal)
					{
					keep=false;
					}
				if(keep==!inverse)
					{
					cout << line << endl;
					}
				}
			}
	};

int main(int argc,char** argv)
    {
    NumericSplit app;
    int optind=1;
    double minVal,maxVal;
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
		     app.column=(int)strtol(argv[++optind],&p2,10)-1;
		     if(app.column<0 || *p2!=0)
				{
				cerr << "Illegal number for column" << endl;
				return EXIT_FAILURE;
				}
		    }
   		else if(std::strcmp(argv[optind],"-m")==0)
		    {
		    char* p2;
		    minVal=(double)strtod(argv[++optind],&p2);
		    if(*p2!=0)
				{
				cerr << "Illegal number for min" << endl;
				return EXIT_FAILURE;
				}
			app.minVal=&minVal;
		    }
		else if(std::strcmp(argv[optind],"-M")==0)
		    {
		    char* p2;
		    maxVal=(double)strtod(argv[++optind],&p2);
		    if(*p2!=0)
				{
				cerr << "Illegal number for max" << endl;
				return EXIT_FAILURE;
				}
			app.maxVal=&maxVal;
		    }
		else if(std::strcmp(argv[optind],"-v")==0)
			{
			app.inverse=true;
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
   			fprintf(stderr,"unknown option '%s'\n",argv[optind]);
   			app.usage(argc,argv);
   			return (EXIT_FAILURE);
   			}
   		else
   			{
   			break;
   			}
   		++optind;
                }
    if(app.column<0)
	{
	cerr << "Undefined column."<< endl;
	app.usage(argc,argv);
	return (EXIT_FAILURE);
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
		char* filename=argv[optind++];
		igzstreambuf buf(filename);
		istream in(&buf);
		app.run(in);
		buf.close();
		}
	    }
    return EXIT_SUCCESS;
    }
