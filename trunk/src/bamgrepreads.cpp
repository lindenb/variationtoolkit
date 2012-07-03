#include <set>
#include <string>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cassert>
#include "bam.h"
#include "sam.h"

using namespace std;

class BamGrepReads
{
public:
        bool erase_if_find;
	int flag_on;
	int flag_off;
	set<string> reads;
	
	BamGrepReads():erase_if_find(false),flag_on(0),flag_off(0)
		{
		}

	int grep(const char* filename)
	 {
	
	 samfile_t *fp_in= NULL;
	 bam1_t *b=NULL;
	
	 fp_in = samopen(filename==NULL?"-":filename, "rb", 0);
	 if(NULL == fp_in)
		  {
		  cerr << "Could not open file. "<<(filename==NULL?"-":filename)
		  	<< " "
		  	<< strerror(errno)
		  	<< endl  ;
		  return EXIT_FAILURE;
		  }
         string name;
         set<string>::iterator r_end=this->reads.end();
         set<string>::iterator r;
	 b = bam_init1();
	 while(samread(fp_in, b) > 0)
	      {
	      const bam1_core_t *c = &b->core;
	      if(((b->core.flag & flag_on) != flag_on) || (b->core.flag & flag_off))
	      	{

	      	continue;
	      	}
	      name.assign(bam1_qname(b),c->l_qname-1);
		

	      if( (r=this->reads.find(name))==r_end)
	      	{
	      	continue;
	      	}
	      char* s= bam_format1( fp_in->header,b);
	      cout << s << endl;
	      free(s);
	      if(erase_if_find)
	      	{
	      	this->reads.erase(r);
	      	if(reads.empty()) break;
	      	}
	      }
	 bam_destroy1(b);
	 samclose(fp_in);
	 return EXIT_SUCCESS;
	 }
	int loadReads(const char* filename)
		{
		string line;
		ifstream in(filename,ios::in);
		if(!in.is_open())
			  {
			  cerr << "Could not open file. "<<(filename)
			  	<< " "
			  	<< strerror(errno)
			  	<< endl  ;
			  return EXIT_FAILURE;
			  }
		while(getline(in,line,'\n'))
			{
			if(line.empty() || line[0]=='#') continue;
			this->reads.insert(line);

			}
	        in.close();
		return 0;
		}
		
int main(int argc, char ** argv)
  {
  int optind=1;
  /* loop over the arguments */
  while(optind<argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		    {
		    cerr << "Author: Pierre Lindenbaum PHD. 2012.\n";
		    cerr << "Last compilation:"<<__DATE__<<" " <<__TIME__ << endl;
		    cerr << argv[0] << " -R reads.txt [-e] [-f flag_on] [-F flag_off](stdin| bam1 bam2....)" << endl;
		    return EXIT_SUCCESS;
		    }
	     else if(strcmp(argv[optind],"-e")==0)
	    	{
	    	this->erase_if_find=true;
	    	}
	    else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
	    	{
	    	this->flag_on = strtol(argv[++optind], 0, 0);
	    	}
	    else if(strcmp(argv[optind],"-F")==0 && optind+1<argc)
	    	{
	    	this->flag_off = strtol(argv[++optind], 0, 0);
	    	}
	    else if(strcmp(argv[optind],"-R")==0 && optind+1<argc)
		    {
		    if(this->loadReads(argv[++optind])!=EXIT_SUCCESS)
				{
				return EXIT_FAILURE;
				}
		    }
	    else if(strcmp(argv[optind],"--")==0)
		    {
		    optind++;
		    break;
		    }
	    else if(argv[optind][0]=='-')
		    {
		     cerr << "Unnown option:"<<argv[optind] << endl;
		    return EXIT_FAILURE;
		    }
	    else
		    {
		    break;
		    }
	    ++optind;
	    }
  if(this->reads.empty())
  	{
  	cerr << "No read loaded" << endl;
  	return EXIT_FAILURE;
  	}
  if(optind==argc)
      {
	
      return this->grep(NULL);
      }
  /* loop over the files */
  while(optind<argc)
	{
	if(this->grep(argv[optind++])!=EXIT_SUCCESS)
		{
		return EXIT_FAILURE;
		}
	 if(erase_if_find && reads.empty()) break;
	}
  return EXIT_SUCCESS;
  }
};


int main(int argc, char** argv)
  {
  BamGrepReads app;
  return app.main(argc,argv);
  }

