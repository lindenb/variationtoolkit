/*
 * md5cols.cpp
 *
 *  Created on: Feb 22, 2012
 *      Author: lindenb
 */
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <openssl/evp.h>

#define NOWHERE
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "bin.h"
#include "where.h"
#include "numeric_cast.h"

using namespace std;

class MD5Cols
    {
    public:
	Tokenizer tokenizer;
	vector<int> columns;
	static void print_as_hex (const unsigned char *digest, int len)
	    {
	    for(int i = 0; i < len; i++)
		{
		fprintf(stdout,"%02x", digest[i]);
		}
	    }

	 void run(std::istream& in)
	     {
	     string line;
	     vector<int> binList;
	     vector<string> tokens;
	     ostringstream os;
	     while(getline(in,line,'\n'))
		 {
		 if(line.empty()) continue;
		 if(line[0]=='#')
		     {
		     fputs("#MD5\t",stdout);
		     fputs(&line.c_str()[1],stdout);
		     fputc('\n',stdout);
		     }
		 tokenizer.split(line,tokens);
		 EVP_MD_CTX mdctx;
	         unsigned char md_value[EVP_MAX_MD_SIZE];
	         unsigned int md_len;

	         EVP_DigestInit(&mdctx, EVP_md5());

		    if(columns.empty())
			{
			for(size_t i=0;i< tokens.size();++i)
			    {
			    EVP_DigestUpdate(&mdctx, tokens[i].c_str(),tokens[i].size());
			    }
			}
		    else
			{
			for(size_t i=0;i< columns.size();++i)
			    {
			    size_t idx=columns[i];
			    if(idx>=tokens.size())
				{
				char nil='\0';
				EVP_DigestUpdate(&mdctx, &nil,1);
				}
			    else
				{
				EVP_DigestUpdate(&mdctx, tokens[idx].c_str(),tokens[idx].size());
				}
			    }
			}


	         EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
	         EVP_MD_CTX_cleanup(&mdctx);
	         print_as_hex(md_value,md_len);
	         for(size_t i=0;i< tokens.size();++i)
		    {
	            fputc(tokenizer.delim,stdout);
	            fputs(tokens[i].c_str(),stdout);
		    }
		 fputc('\n',stdout);
		 }
	     }

	int main(int argc,char** argv)
	    {
	    int optind=1;
	    while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			    {
			    cerr << argv[0] << "Pierre Lindenbaum PHD. 2012.\n";
			    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			    cerr << "Usage:\n\t"<< argv[0]<< " [options] (files|stdin)*\n\n";
			    cerr << "Options:\n";
			    cerr << "  -c columns\n";
			    cerr << "(stdin|files)\n\n";
			    exit(EXIT_FAILURE);
			    }
		    else if(std::strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			int c;
			if(!numeric_cast(argv[++optind],&c) || c<1)
			    {
			    cerr << "Bad column " << argv[optind]<< endl;
			    return EXIT_FAILURE;
			    }
			this->columns.push_back(c-1);
			}
		    else if(argv[optind][0]=='-')
			{
			cerr<<"unknown option '"<<argv[optind]<<"'"<< endl;
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
		    this->run(in);
		    }
	    else
		    {
		    while(optind< argc)
				{
				char* filename=argv[optind++];
				WHERE(filename);
				igzstreambuf buf(filename);
				istream in(&buf);
				this->run(in);
				buf.close();
				}
		    }
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    MD5Cols app;
    return app.main(argc,argv);
    }
