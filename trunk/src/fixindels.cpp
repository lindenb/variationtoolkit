/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	For Solena:
 *	"Il me les faut au format VCF 'classique' par exemple pas GCC -> GCCC mais C -> CC"
 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <set>
#include <cerrno>
#include <iostream>
#include <limits>
#include <fstream>
#include <string>
#include <stdint.h>
#include "throw.h"
#include "tokenizer.h"
#include "numeric_cast.h"



using namespace std;

class FixIndels
    {
    private:
	Tokenizer tokenizer;

    public:
	FixIndels()
	    {

	    }

	void run(std::istream& in)
	    {
	    Tokenizer comma(',');
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

		this->tokenizer.split(line,tokens);
		if(tokens.size()<5)
		    {
		    cerr << "Illegal number of tokens in "<< line << endl;
		    continue;
		    }

		string& ref=tokens[3];
		string& alt=tokens[4];
		if( ref.size()==1 &&
		    alt.size()==1 &&
		    isalpha(ref[0]) &&
		    isalpha(alt[0])
		    )
		    {
		    cout << line << endl;
		    continue;
		    }

		int32_t pos;
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&pos))
		    {
		    THROW("Bad position in "<< line);
		    continue;
		    }
		/* CCTTCT	CCT */
		if( alt.size()>1  &&
		    alt.size() < ref.size() &&
		    ref.compare(0,alt.size(),alt)==0
		    )
		    {
		    /* TTCT	T */
		    size_t rm = alt.size()-1;
		    ref.erase(0,rm);
		    alt.erase(0,rm);
		    pos+=(int32_t)rm;
		    }
		/* TGG	TGGG */
		else if( ref.size()>1  &&
		    ref.size() < alt.size() &&
		    alt.compare(0,ref.size(),ref)==0
		    )
		    {
		    /* G	GG */
		    size_t rm = ref.size()-1;
		    ref.erase(0,rm);
		    alt.erase(0,rm);
		    pos+=(int32_t)rm;
		    }
		else
		    {
		    cout << line << endl;
		    cerr << "How should I handle : " << line << endl;
		    continue;
		    }

		 for(size_t i=0;i< tokens.size();++i)
		     {
		     if(i>0) cout << "\t";
		     switch(i)
			 {
			 case 1: cout << pos ; break;
			 case 3: cout << ref ; break;
			 case 4: cout << alt ; break;
			 default: cout << tokens[i] << endl;
			 }
		     }
		cout << endl;
		}
	    }
	void usage(ostream& out,int argc,char **argv)
		{
		out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
		out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		out << "Usage:\n\t"<< argv[0] << " [options] (vcf|stdin)\n";
		}

	int main(int argc,char** argv)
	    {
	    int optind=1;


	    while(optind < argc)
		    {
		    if(strcmp(argv[optind],"-h")==0)
			    {
			    usage(cout,argc,argv);
			    return EXIT_FAILURE;
			    }
		    else if(strcmp(argv[optind],"--")==0)
			    {
			    ++optind;
			    break;
			    }
		    else if(argv[optind][0]=='-')
			    {
			    cerr << "unknown option '" << argv[optind]<< endl;
			    usage(cerr,argc,argv);
			    return(EXIT_FAILURE);
			    }
		    else
			    {
			    break;
			    }
		    ++optind;
		    }

	    if(optind==argc)
		    {
		    run(cin);
		    }
	    else if(optind+1==argc)
		    {
		    ifstream in(argv[optind++],ios::in);
		    if(!in.is_open())
			{
			cerr << "Cannot open input file. "<< strerror(errno) << endl;
			return EXIT_FAILURE;
			}
		    this->run(in);
		    in.close();
		    }
	    else
		{
		cerr << "Illegal number of arguments.\n";
		return EXIT_FAILURE;
		}
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc,char** argv)
    {
    FixIndels app;
    return app.main(argc,argv);
    }
