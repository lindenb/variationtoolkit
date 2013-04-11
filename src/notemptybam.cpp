/**
 * Author:
 *	Pierre Lindenbaum PhD
 * Contact:
 *	plindenbaum@yahoo.fr
 * WWW:
 *	http://plindenbaum.blogspot.com
 * Motivation:
 *	select non-empty BAMs. Samtools (or other) might throw an error
 	if a BAM contains NO read
 * Usage:
 *	notemptybam bam1 bam2 ...
 */
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cerrno>
#include <iostream>
#include <limits>
#include <stdint.h>
#include "sam.h"
#include "bam.h"


using namespace std;


void usage(ostream& out,int argc,char** argv)
    {
    out << argv[0] << " Pierre Lindenbaum PHD. 2013.\n";
    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
    out << "Usage:\n\n";
    out << "   "<<  argv[0] << " [options] bam1.bam bam2.bam bam3.bam ..."<< endl;
    out << "Options:\n"
    	<< " -v inverse \n"
    	<< " -s skip BAM file if it cannot be opened, but no error. \n"
    	;
    out << endl;
    }


int main(int argc, char *argv[])
	{
	int optind=1;
	bool inverse=false;
	bool skip_if_file_not_exist=false;
	
	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		        {
		        usage(cout,argc,argv);
		        return EXIT_FAILURE;
		        }
		else if(strcmp(argv[optind],"-s")==0)
			{
			skip_if_file_not_exist=true;
			}
		else if(strcmp(argv[optind],"-v")==0)
			{
			inverse=true;
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
		return 0;
		}

	while(optind<argc)
		{
		char* filename=argv[optind++];
		bam1_t* b= bam_init1();
		if(b==NULL)
			{
			cerr << "Out of memory\n";
			return(EXIT_FAILURE);
			}
		samfile_t* in = samopen(filename, "rb", 0);
		if(in==NULL)
			{
			cerr << "Cannot open "<< filename << " " << strerror(errno) << "\n";
			if(skip_if_file_not_exist) continue;
			return(EXIT_FAILURE);	
			}
		bool has_read=false;
		while(samread(in, b) > 0)
			{
			has_read=true;
			break;
			}
		bam_destroy1(b);
		samclose(in);
		if(has_read == !inverse)
			{
			cout << filename << "\n";
			}
		}
	

	
	
	return EXIT_SUCCESS;
	}
    

