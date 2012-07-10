#include <iostream>
#include "knowngene.h"
using namespace std;

int main(int argc,char** argv)
	{
	string line;
	while(getline(cin,line,'\n'))
		{
		if(line.empty() || line[0]=='#') continue;
		std::auto_ptr<KnownGene> kg=KnownGene::parse(line);
		for(int32_t i=0;i< kg->countExons();++i)
			{
			const Exon* exon=kg->exon(i);
			cout << kg->chrom << "\t"
				<< exon->start << "\t"
				<< exon->end << "\t"
				<< kg->strand << "\t"
				<< kg->name << "\t"
				<< exon->name()
				<< std::endl
				; 
			}
		}
	return 0;
	}
