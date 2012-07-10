#include <iostream>
#include "numeric_cast.h"
#include "knowngene.h"
using namespace std;

static void usage(std::ostream& out,int argc,char** argv)
	{
	out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	out << "Usage:\n\t"<< argv[0] << " [options] stdin (stream of ucsc knownGene table.\n";
	out << "Options:\n";
	out << " -x <INTEGER> extends bounds default:0.\n";
	out << " -i print Introns.\n";
	out << " -e do not print Exons.\n";
	}

int main(int argc,char** argv)
	{
	bool print_introns=false;
	bool print_exons=true;
	int32_t extend=0;
	int optind=1;
	while(optind < argc)
		{
		if(strcmp(argv[optind],"-h")==0)
		        {
		        usage(cout,argc,argv);
		        return EXIT_FAILURE;
		        }
		else if(strcmp(argv[optind],"-x")==0 && optind+1<argc)
			{
			if(!numeric_cast<int32_t>(argv[++optind],&extend) || extend<0)
				{
				cerr << "Bad extends: " << argv[optind] << endl;
				return EXIT_FAILURE;
				}
			}
		else if(strcmp(argv[optind],"-i")==0)
			{
			print_introns=true;
			}
		else if(strcmp(argv[optind],"-e")==0)
			{
			print_exons=false;
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
		return EXIT_FAILURE;
		}

	string line;
	while(getline(cin,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		std::auto_ptr<KnownGene> kg=KnownGene::parse(line);
		for(int32_t i=0;i< kg->countExons();++i)
			{
			if(print_exons)
				{
				const Exon* exon=kg->exon(i);
				cout << kg->chrom << "\t"
					<< std::max(exon->start-extend,0) << "\t"
					<< (exon->end+extend) << "\t"
					<< kg->strand << "\t"
					<< kg->name << "\t"
					<< exon->name()
					<< std::endl
					;
				} 
			if(print_introns && i+1< kg->countExons())
				{
				std::auto_ptr<Intron> intron =kg->intron(i);
				cout << kg->chrom << "\t"
					<< std::max(intron->start-extend,0) << "\t"
					<< (intron->end+extend) << "\t"
					<< kg->strand << "\t"
					<< kg->name << "\t"
					<< intron->name()
					<< std::endl
					;
				}
			}
		}
	return 0;
	}
