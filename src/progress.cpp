#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>
using namespace std;
/*
 * progress.cpp
 *
 *  Created on: Feb 21, 2012
 *      Author: lindenb
 */
int main(int argc,char** argv)
    {
    string prefix;
    long every=1000;
    int optind=1;
    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
			{
			cerr << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
			cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			cerr << "Usage" << endl
				<< "   "<< argv[0]<< " [otions] stdin"<< endl;
			cerr << "Options:\n";
			cerr << "  -e <int> print every 'x' lines default:"<< every << endl;
			cerr << "  -p <string> prefix (optional)\n";
			cerr << endl;
			return (EXIT_FAILURE);
			}
		else if(std::strcmp(argv[optind],"-e")==0 && optind+1<argc)
			{
			every=atol(argv[++optind]);
			}
		else if(std::strcmp(argv[optind],"-p")==0 && optind+1<argc)
			{
			prefix.assign(argv[++optind]);
			}
		else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
			return (EXIT_FAILURE);
			}
		else
			{
			break;
			}
		++optind;
		}
    FILE* log=fopen("/dev/tty","w");
    string msg;
    long nLines=0;
    int c;
    while((c=fgetc(stdin))!=EOF)
	{
	fputc(c,stdout);
	if(log!=0 && c=='\n')
	    {
	    ++nLines;
	    if(nLines%every==0)
		{
		for(size_t i=0;i< msg.size();++i)
		    {
		    fputc('\b',log);
		    }
		ostringstream os;
		if(!prefix.empty())
		    {
		    os << prefix << " : ";
		    }
		os << nLines << " lines.";
		msg.assign(os.str());
		for(size_t i=0;i< msg.size();++i)
		    {
		    fputc(msg[i],log);
		    }
		}
	    }
	}
    if(log!=0)
	{
	 fputc('\n',log);
	fclose(log);
	}
    return EXIT_SUCCESS;
    }
