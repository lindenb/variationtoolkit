#include <iostream>
#include <cstring>
#include <map>
#include "numeric_cast.h"
#include "tokenizer.h"
using namespace std;
static void usage(std::ostream& out,int argc,char** argv)
	{
		    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage:\n\t"<< argv[0] << " [options] file.bam [chr:start-end]\n";
	    out << "Options:\n";
	    out << " -d <CHAR> delimiter (default:tab ).:\n";
	    out << " -L <CHAR> begin head line with... (default:# ).:\n";
	    out << " -c <INTEGER> <STRING> custom column def.\n";

		}
int main(int argc,char** argv)
	{
	map<size_t,string> usercol;
	char delim='\t';
	char left='#';
	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		        {
		        usage(cout,argc,argv);
		        return EXIT_FAILURE;
		        }
		else if(strcmp(argv[optind],"-c")==0 && optind+2<argc)
			{
			size_t i=0;
			if(!numeric_cast<size_t>(argv[++optind],&i) || i==0)
				{
				cerr << "Bad column index " << argv[optind] << endl;
				return EXIT_FAILURE;
				}
			i--;
			usercol.insert(make_pair<size_t,string>(i,argv[++optind]));
			}
		else if(strcmp(argv[optind],"-d")==0 &&
			optind+1<argc &&
			strlen(argv[optind+1])==1)
		        {
		        ++optind;
		        delim=argv[optind][0];
		        }
		else if(strcmp(argv[optind],"-L")==0 &&
			optind+1<argc &&
			strlen(argv[optind+1])==1)
		        {
		         ++optind;
		        left=argv[optind][0];
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
        
        if(optind!=argc)
		{
		cerr << "Can only read from stdin" << endl;
		}

	string  line;
	bool first=true;
	while(getline(cin,line,'\n'))
		{
		if(first)
			{
			Tokenizer tokenizer(delim);
			std::vector<std::string> tokens;
			tokenizer.split(line,tokens);
			while(!tokens.empty() && tokens.back().size()==0) tokens.pop_back();
			for(size_t i=0;i<tokens.size();++i)
				{
				cout << (i==0?left:delim) << "$";
				map<size_t,string>::iterator r=usercol.find(i);
				if(r==usercol.end())
					{
					cout << (i+1);
					}
				else
					{
					cout << (*r).second;
					}
				}
			cout << endl;
			first=false;
			}
		cout << line << endl;
		}
	return 0;
	}
