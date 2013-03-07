/**
 
Motivation: recursively count the number of reads in a hierachy containing some FASTQ

*/
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <zlib.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>
using namespace std;



class FastqCount
	{
	public:
	char* regex_expr;
	regex_t regex;
	bool print_per_fastq;
	bool log;
	FastqCount():regex_expr(0),print_per_fastq(false),log(false)
		{
		}
	~FastqCount()
		{
		if(regex_expr!=0)
			{
			regfree(&regex);
			}
		}
	bool ends_with(const char* s,const char* suffix)
		{
		size_t L1=strlen(s);
		size_t L2=strlen(suffix);
		if(L2>L1) return false;
		return strncmp(&s[L1-L2],suffix,L2)==0;
		}
	
	long recursive(const char* filename)
		 {
		 if(log) clog << "Scanning " << filename<< endl;
		 struct stat st;
		 if(stat(filename, &st) != 0)
		 	{
		    	cerr << "unable to stat " << filename  << endl;
		    	return 0L;
		 	 }
		if(S_ISDIR(st.st_mode))
			{
			long N=0L;
			DIR *D = opendir(filename);
      			if(D==NULL)
      				{
       				cerr << "unable to opendir " << filename  << endl;
		    		return 0L;
		  		}
		  	struct dirent *de;
     			while((de = readdir(D)) != NULL)
     				{
     				if(strcmp(de->d_name,".")==0) continue;
     				if(strcmp(de->d_name,"..")==0) continue;
  				char *fname = (char*)malloc(strlen(filename) + strlen(de->d_name) + 2);
  				if(ends_with(filename,"/"))
  					{
  					sprintf(fname, "%s%s", filename, de->d_name);
  					}
  				else
  					{
  					sprintf(fname, "%s/%s", filename, de->d_name);
  					}
  				N+=recursive(fname);
  				free(fname); 
      				}
      			(void)closedir(D);
      			if(N>0L)
      				{
      				cout << N << "\t" << filename << endl;
      				}
      			return N;
    			}
		else
		     {
		     if(!(ends_with(filename, ".fastq.gz") || ends_with(filename, ".fastq"))) return 0L;
		     if(regex_expr!=0)
		     	{
		     	int reti = regexec(&regex,filename, 0, NULL, REG_NOTBOL);
		     	if(reti == REG_NOMATCH) return 0;
		     	}
		     long N=0L;
		     if(log) clog << "Reading " << filename<< endl;
		     gzFile in=gzopen(filename,"rb");
		     if(in==0)
		     	{
		     	cerr << "Cannot open " << filename << endl;
		     	return 0;
		     	}
		     
		     char buffer[BUFSIZ];
		     for(;;)
		     	{
		     	int nRead= ::gzread(in,(void*)buffer,BUFSIZ);
		     	if(nRead<=0) break;
		     	for(int i=0;i< nRead;++i) if(buffer[i]=='\n') nRead++;
		     	}
		     gzclose(in);
		     N=N/4;
		     if(print_per_fastq)
		     	{
		     	cout << N << "\t" << filename << endl;
		     	}
		     return N;
		     }
		}
	
	void usage(int argc, char ** argv)
		{
		cerr << "Author: Pierre Lindenbaum PHD. 2013.\n";
		cerr << "Last compilation:"<<__DATE__<<" " <<__TIME__ << endl;
		cerr << "Usage:\n";
		cerr << "  " << argv[0] << " [options] path1 path2 path3 ..." << endl;
		cerr  << "Options:\n";
		cerr  << " -r (regex) filter FASTQ on those names. (OPTIONAL)." << endl;
		cerr  << " -p count for each Fastq file (OPTIONAL)." << endl;
		cerr  << " -L log (OPTIONAL)." << endl;
		}

	int run(int argc,char** argv)
		{
		  int optind=1;
		  /* loop over the arguments */
		  while(optind<argc)
			    {
			    if(strcmp(argv[optind],"-h")==0)
				    {
				    usage(argc,argv);
				    return EXIT_SUCCESS;
				    }
			   else  if(strcmp(argv[optind],"-p")==0)
				    {
				    print_per_fastq=true;
				    }
			else  if(strcmp(argv[optind],"-L")==0)
				    {
				    log=true;
				    }
			   else if(strcmp(argv[optind],"-r")==0 && optind+1<argc)
			    	{
			    	this->regex_expr=argv[++optind];
			    	if(regcomp(&regex, this->regex_expr, 0)!=0)
			    		{
			    		cerr << "Cannot compile regex "<< this->regex_expr << endl;
			    		regex_expr=0;
			    		return -1;
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
				     usage(argc,argv);
				    return EXIT_FAILURE;
				    }
			    else
				    {
				    break;
				    }
			    ++optind;
			    }
		if(optind==argc)
			{
			recursive(".");
			}
		else while(optind<argc)
			{
			recursive(argv[optind++]);
			}
		return 0;
		}

	};

int main(int argc,char** argv)
	{
        FastqCount app;
	return app.run(argc,argv);       
	}
