/*
 * shortreadswords.cpp
 *
 *  Created on: Dec 2012
 *      Author: lindenb
 */
#include <map>
#include <vector>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <cerrno>
#include <fstream>
#include <algorithm>
#include <zlib.h>
#include <stdint.h>
#include "zstreambuf.h"
#include "throw.h"
#define NOWHERE
#include "where.h"
#include "numeric_cast.h"

using namespace std;



class ShortReaWords
    {
    public:
	std::map<string,uint64_t > words;
	uint16_t word_size;
	bool sortocc;
	bool printnotfound;

	ShortReaWords():word_size(5),sortocc(false),printnotfound(false)
	    {

	    }

	~ShortReaWords()
	    {

	    }

	
	void missings(char* seq,int size)
		{
		if(size==this->word_size)
			{
			if(words.find(seq)==words.end())
				{
				cout.write(seq,this->word_size);
				cout << "\t0\n";
				}
			return;
			}
		seq[size]='A';missings(seq,size+1);
		seq[size]='T';missings(seq,size+1);
		seq[size]='G';missings(seq,size+1);
		seq[size]='C';missings(seq,size+1);
		}
	
	void read(std::istream& in)
	    {
	    string line;
	    uint64_t n=0;
	   
	    char* seq=new char[this->word_size+1];
	    seq[this->word_size]=0;
	    while(getline(in,line,'\n'))
	    	{
	    	++n;
	    	if(n%4!=2) continue;
	    	
	    	std::transform(line.begin(), line.end(),line.begin(), ::toupper);
	    	for(size_t i=0;i+this->word_size <= line.size();++i)
	    		{
	    		
	    		line.copy(seq,this->word_size,i);
	    		if(strspn(seq,"ATGC")!=this->word_size) continue;
	    		std::map<string,uint64_t >::iterator r=words.find(seq);

	    		if(r==words.end())
	    			{
	    			words.insert(make_pair<string,uint64_t>(seq,01L));
	    			}
	    		else
	    			{
	    			r->second++;
	    			}
	    		
	    		}
	    	}
	    
	    delete[] seq;

	    }
	    
	void dump()
		{
		
		
		if(sortocc)
			{
			std::multimap<uint64_t,string> occmap;
			
			std::map<string,uint64_t >::iterator r=words.begin();
			while(r!=words.end())
				{
				occmap.insert(make_pair<uint64_t,string>(r->second,r->first));
				++r;
				}

			
			std::multimap<uint64_t,string>::reverse_iterator r2=occmap.rbegin();
			while(r2!=occmap.rend())
				{
				cout << r2->second << "\t" << r2->first << endl;
				++r2;
				}
			}
		else
			{
			std::map<string,uint64_t >::iterator r=words.begin();
			while(r!=words.end())
				{
				cout << r->first << "\t" << r->second << endl;
				++r;
				}
			}
			
	    	if(printnotfound)
	    		{
	    		char* seq=new char[this->word_size+1];
	   		seq[this->word_size]=0;
	    		missings(seq,0);
	    		delete[] seq;
	    		}
		}
		
	void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Usage" << endl
		    << "   "<< argv[0]<< " [options] (file1.fastq file2.fastq ... | stdin )"<< endl;
	    out << "Options:\n";
	    out << "  -N (word_size) default:" << word_size << endl;
	    out << "  -S sort on frequency"<< endl;
	    out << "  -F print not found."<< endl;
	    out << endl;
	    }

	int main(int argc,char** argv)
	    {
	    int optind=1;
	   

	    while(optind < argc)
		{
		if(std::strcmp(argv[optind],"-h")==0)
		    {
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		else if(std::strcmp(argv[optind],"-N")==0 && optind+1< argc)
		    {
		     if(!numeric_cast<uint16_t>(argv[++optind],&word_size) || word_size==0)
		     	{
		     	cerr << "bad value for option -N " << argv[optind] << endl;
		     	exit(EXIT_FAILURE);
		     	}
		    }
		else if(std::strcmp(argv[optind],"-S")==0 )
		    {
		    this->sortocc=true;
		    }
		else if(std::strcmp(argv[optind],"-F")==0 )
		    {   
		    this-> printnotfound=true;
		    }
		else if(argv[optind][0]=='-')
		    {
		    cerr << "unknown option '"<< argv[optind]<< "'"<< endl;
		    usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
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
		    this->read(in);
		    }
	    else
		    {
		    while(optind< argc)
			{
			igzstreambuf buf(argv[optind++]);
			istream in(&buf);
			this->read(in);
			buf.close();
			++optind;
			}
		    }
	    dump();
	    return EXIT_SUCCESS;
	    }
    };


int main(int argc, char** argv)
    {
    ShortReaWords app;
    return app.main(argc,argv);
    }
