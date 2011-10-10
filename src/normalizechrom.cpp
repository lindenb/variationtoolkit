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
 *	normlize chrom column to/from UCSC/ENSEMBL
 * Compilation:
 *	 g++ -o normalizechrom -Wall -O3 normalizechrom.cpp -lz
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

class NormalizeChrom
	    {
	    public:
		    char delim;
		    char* ignore;
		    int chrom_col;
		    bool to_ucsc;
		    
		    NormalizeChrom()
		    	{
		    	delim='\t';
				chrom_col=0;
				to_ucsc=true;
		    	}
		    	
		  void normalize(std::istream& in)
		    {
		    string line;
		   

		    Tokenizer tokenizer;
		    tokenizer.delim=delim;
		    vector<string> tokens;
		    while(getline(in,line,'\n'))
				{
				if(!line.empty() && line[0]=='#')
				    {
				    cout << line << endl;
				    continue;
				    }
				tokenizer.split(line,tokens);
				for(int column=0;column<(int)tokens.size();++column)
				    {
				    if(column>0) cout << delim;
				    if(column==chrom_col)
						{
						char* p=(char*)tokens[chrom_col].c_str();
						char *p2=p;
						while(*p2!=0) {*p2=tolower(*p2);++p2;}
						while(*p!=0 && isspace(*p)) p++;
						if(strncmp(p,"chrom",5)==0)
						    {
						    p+=5;
						    }
						else if(strncmp(p,"chr",3)==0)
						    {
						    p+=3;
						    }
						else if(strncmp(p,"k",1)==0)
						    {
						    p++;
						    }
						while(*p!=0 && isspace(*p)) p++;
						if(to_ucsc)
						    {
						    if(strcmp(p,"x")==0)
								{
								cout << "chrY";
								}
						    else if(strcmp(p,"y")==0)
								{
								cout << "chrY";
								}
						    else if(strcmp(p,"m")==0 || strcmp(p,"mt")==0)
								{
								cout << "chrM";
								}
						    else
								{
								cout << "chr" << p;
								}
					    }
					else /* to ENSEMBL */
					    {
					    if(strcmp(p,"x")==0)
							{
							cout << "X";
							}
					    else if(strcmp(p,"y")==0)
							{
							cout << "Y";
							}
					    else if(strcmp(p,"m")==0 || strcmp(p,"mt")==0)
							{
							cout << "MT";
							}
					    else
							{
							cout << p;
							}
					    }
					}
				    else
						{
						cout << tokens[column];
						}
				    }
				cout << endl;
				}
		    }
	    };

int main(int argc,char** argv)
	{
	NormalizeChrom app;


	int optind=1;
	while(optind < argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		    {
		    fprintf(stderr,"%s: Pierre Lindenbaum PHD. October 2011.\nNormalize a chromosome name to/from UCSC/Ensembl\n",argv[0]);
		    fprintf(stderr,"Compilation: %s at %s.\n",__DATE__,__TIME__);
		    fprintf(stderr,"Options:\n");
		    fprintf(stderr," -i <string> ignore lines starting with this string.\n");
		    fprintf(stderr," -d <string> column delimiter (default:tab).\n");
		    fprintf(stderr," -c <int> column index (+1) (default:%d).\n",app.chrom_col+1);
		    fprintf(stderr," -E convert to ENSEMBL syntax (default is UCSC).\n");
		    fprintf(stderr,"\n(stdin|files)\n\n");
		    exit(EXIT_FAILURE);
		    }
	    else if(strcmp(argv[optind],"-d")==0 && optind+1< argc)
			{
			char* p=argv[++optind];
			if(strlen(p)!=1)
			    {
			    fprintf(stderr,"Bad delimiter \"%s\"\n",p);
			    return (EXIT_FAILURE);
			    }
			app.delim=p[0];
			}
	    else if(strcmp(argv[optind],"-i")==0 && optind+1<argc)
			{
			app.ignore=argv[++optind];
			}
	    else if(strcmp(argv[optind],"-E")==0 )
			{
			app.to_ucsc=0;
			}
	    else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			char* p2;
			app.chrom_col=(int)strtol(argv[++optind],&p2,10);
			if(*p2!=0 || app.chrom_col<1)
			    {
			    fprintf(stderr,"Bad chromosome column:%s\n",argv[optind]);
			    return EXIT_FAILURE;
			    }
			app.chrom_col--;/* to 0-based */
			}
	    else if(strcmp(argv[optind],"--")==0)
			{
			++optind;
			break;
			}
	     else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '" << argv[optind]<< "'" << endl;
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
		app.normalize(in);
		}
	else
		{
		while(optind< argc)
			{
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			app.normalize(in);
			buf.close();
			++optind;
			}
		}
	return EXIT_SUCCESS;
	}
