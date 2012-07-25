/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * Date:
 *	July 2012
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	http://www.biostars.org/post/show/48211
 */
#include <string>
#include "zstreambuf.h"
#ifdef __GNUC__
#include <ext/hash_set>
namespace __gnu_cxx
{
     /* hack from http://www.moosechips.com/2008/10/using-gcc-c-hash-classes-with-strings/ */
template<> struct hash< std::string >
        {
                size_t operator()( const std::string& x ) const
                {
                        return hash< const char* >()( x.c_str() );
                }
        };
}

typedef __gnu_cxx::hash_set<std::string, __gnu_cxx::hash<std::string> > set_of_string;
#else
#include <set>
typedef std::set<std::string> set_of_string;
#endif

#include <fstream>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cassert>
#include "zstreambuf.h"

using namespace std;

class FastQGrep
{
private:
        bool erase_if_find;
	set_of_string reads;
	ostream* out[2];
public:	
	FastQGrep():erase_if_find(true)
		{
		out[0]=&cout;
		out[1]=&cout;
		}
	
	bool readFastQ(std::istream& in,
		std::string& name,
	 	std::string& seq,
	 	std::string& qual,
	 	std::string& delim
		)
		{
		if(!getline(in,name,'\n')) return false;
		if(!getline(in,seq,'\n')) return false;
		if(!getline(in,delim,'\n')) return false;
		if(!getline(in,qual,'\n')) return false;
		return true;
		}
	
	set_of_string::iterator find(const string& name)
		{
		set_of_string::iterator r_end=this->reads.end();
		set_of_string::iterator r;
		if( (r=this->reads.find(name))!=r_end) return r;
		string::size_type n= name.find_first_of(" \t");
		if(n!=string::npos)
			{
			
			return find(name.substr(0,n));
			}
		return r_end;
		}
		
	
	int grep1(std::istream& in)
		{
		std::string name;
		std::string seq;
		std::string qual;
		std::string delim;
		set_of_string::iterator r_end=this->reads.end();
		set_of_string::iterator r;
		while(readFastQ(in,name,seq,qual,delim))
		{
		if( (r=this->find(name))==r_end)
			{
			continue;
			}
		*(out[0]) << name << "\n"
				<< seq <<  "\n"
				<< delim << "\n"
				<< qual << "\n"
				;
		
		
		if(erase_if_find)
			{
			this->reads.erase(r);
			if(reads.empty()) break;
			}
		}
		return EXIT_SUCCESS;
		}
	 
	 int grep2(std::istream& in1,std::istream& in2)
		{
		std::string name[2];
		std::string seq[2];
		std::string qual[2];
		std::string delim[2];
		set_of_string::iterator r_end=this->reads.end();
		set_of_string::iterator r;
		while(
			readFastQ(in1,name[0],seq[0],qual[0],delim[0]) &&
			readFastQ(in2,name[1],seq[1],qual[1],delim[1]) 
			)
			{
			r=this->find(name[0]);
			if(r==r_end)
				{
				r = this->find(name[1]);
				if(r==r_end) continue;
				}
			if(out[0]==out[1])
				{
				*(out[0]) << name[0] << "\t"<<  name[1]<< "\n"
					<< seq[0] << "\t"<< seq[1]<<  "\n"
					<< delim[0] << "\t"<< delim[1]<< "\n"
					<< qual[0] << "\t"<< qual[1]<< "\n"
					;
				}
			else
				{
				for(int i=0;i< 2;++i)
					{
					*(out[i]) << name[i] << "\n"
							<< seq[i] <<  "\n"
							<< delim[i] << "\n"
							<< qual[i] << "\n"
							;
					}
				}
			if(erase_if_find)
				{
				this->reads.erase(r);
				if(reads.empty()) break;
				}
			}
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
			if(line[0]!='@') line.insert(0,"@");
			this->reads.insert(line);
			}
	        in.close();
		return 0;
		}

void usage(int argc, char ** argv)
	{
	cerr << "Author: Pierre Lindenbaum PHD. 2012.\n";
	cerr << "Last compilation:"<<__DATE__<<" " <<__TIME__ << endl;
	cerr << "Usage:\n";
	cerr << "  " << argv[0] << " -R file_of_names.txt [options] (stdin)|(fastq)|(fastq1 fastq2)" << endl;
	cerr  << "Options:\n";
	cerr  << " -R file_of_names.txt REQUIRED." << endl;
	cerr  << " -m allow multiple hits per read." << endl;
	cerr  << " -o1 (filename) file out1 OPTIONAL" << endl;
	cerr  << " -o2 (filename) file out2 OPTIONAL" << endl;
	}

int main(int argc, char ** argv)
  {
  char* filename[2]={0,0};
  int optind=1;
  /* loop over the arguments */
  while(optind<argc)
	    {
	    if(strcmp(argv[optind],"-h")==0)
		    {
		    usage(argc,argv);
		    return EXIT_SUCCESS;
		    }
	     else if(strcmp(argv[optind],"-m")==0)
	    	{
	    	this->erase_if_find=false;
	    	}
	    else if(strcmp(argv[optind],"-o1")==0 && optind+1<argc)
	    	{
	    	filename[0]=argv[++optind];
	    	}
	    else if(strcmp(argv[optind],"-o2")==0 && optind+1<argc)
	    	{
	    	filename[1]=argv[++optind];
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
		     usage(argc,argv);
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
  for(int i=0;i< 2;++i)
  	{
  	if(filename[i]!=0)
  		{
  		out[i]=new fstream(filename[i],ios::out);
  		if(!((fstream*)out[i])->is_open())
  			{
  			cerr << "Cannot open " << filename[i] << " " << strerror(errno) << endl;
  			return EXIT_FAILURE;
  			}
  		}
  	}
  if(optind==argc)
      {
      igzstreambuf buf;
      istream in(&buf);
      grep1(in);
      buf.close();
      }
  else if(optind+1==argc)
      	{
	igzstreambuf buf(argv[optind++]);
	istream in(&buf);
	grep1(in);
	buf.close();
      	}
  else if(optind+2==argc)
	{
	igzstreambuf buf1(argv[optind++]);
	igzstreambuf buf2(argv[optind++]);
	istream in1(&buf1);
	istream in2(&buf2);
	 this->grep2(in1,in2);
	 buf1.close();
	 buf2.close();
	}
  else
  	{
  	cerr << "Illegal number of arguments.\n";
  	return EXIT_FAILURE;
  	}
  	
  for(int i=0;i< 2;++i)
  	{
  	if(filename[i]!=0)
  		{
  		if(out[i]==0 || out[i]==&cout) continue;
  		((fstream*)out[i])->close();
  		}
  	}
  return EXIT_SUCCESS;
  }
};


int main(int argc, char** argv)
  {
  FastQGrep app;
  return app.main(argc,argv);
  }

