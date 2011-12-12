#include <zlib.h>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include "throw.h"
#include "genomeindex.h"

using namespace std;

GenomeIndex::Chromosome::Chromosome():length(0),sequence(NULL)
    {
    }

GenomeIndex::Chromosome::~Chromosome()
    {
    if(sequence!=NULL) free(sequence);
    }

char GenomeIndex::Chromosome::at(int32_t index) const
    {
    return sequence[index];
    }

int32_t GenomeIndex::Chromosome::size() const
    {
    return length;
    }

GenomeIndex::GenomeIndex()
    {
    }

GenomeIndex::~GenomeIndex()
    {
    }

const GenomeIndex::Chromosome* GenomeIndex::getChromsomeByIndex(uint8_t id) const
    {
    return this->chromosomes.at(id);
    }

void GenomeIndex::readGenome(const char* fasta)
    {
    gzFile in=gzopen(fasta,"rb");
    if(in==NULL) THROW("Cannot open "<< fasta << " "<< strerror(errno));
    int c;
    GenomeIndex::Chromosome* curr=NULL;
   while((c=gzgetc(in))!=-1)
	{

	if(c=='>')
	    {
	    if(this->chromosomes.size()== std::numeric_limits<uint8_t>::max())
		{
		THROW("too many sequences in "<< fasta);
		}
	    curr=new GenomeIndex::Chromosome;
	    curr->id=(uint8_t)this->chromosomes.size();
	    curr->length=0;
	    while((c=gzgetc(in))!=-1 && c!='\n')
		{
		if(c=='\r') continue;
		curr->name+=(char)c;
		}
	    this->chromosomes.push_back(curr);
	    continue;
	    }
	if(!isalpha(c)) continue;

	if(curr==NULL) THROW("Header missing for "<< (char)c);
	if(curr->length== std::numeric_limits<int32_t>::max())
		{
		THROW("sequence too large: "<< curr->name << " in "<< fasta);
		}
	curr->length++;
	}
    if(gzrewind(in)!=0)
	{
	THROW("Cannot rewind "<< fasta << " "<< strerror(errno));
	}
    uint8_t seqidx=0;
    curr=NULL;
    int32_t curr_length=0;
    while((c=gzgetc(in))!=-1)
    	{
    	if(c=='>')
    	    {
    	    while((c=gzgetc(in))!=-1 && c!='\n')
    	     {
    	     }
    	    curr= this->chromosomes.at(seqidx);
    	    seqidx++;
    	    curr->sequence=(char*)::malloc(sizeof(char)*(curr->length+1));
    	    if(curr->sequence==NULL) THROW("CAnnot alloc "<< curr->length);
    	    curr->sequence[curr->length]=0;
    	    curr_length=0;
    	    continue;
    	    }
    	if(!isalpha(c)) continue;
    	if(curr==NULL) THROW("Header missing");
    	curr->sequence[curr_length]=toupper(c);
    	curr_length++;
    	}
    gzclose(in);
    }

class Sorter
    {
	private:
	mutable size_t n_called;
    public:
	auto_vector<GenomeIndex::Chromosome>* chromosomes;
	int32_t max_lookahead;
	Sorter():n_called(0),max_lookahead(50)
	    {
	    }

	bool operator()(
		const GenomeIndex::Reference& o1,
		const GenomeIndex::Reference & o2
		) const
	    {
	    if((++n_called)%1000000==0)
		{
		cerr << ".";
		}
	    const GenomeIndex::Chromosome* c1= this->chromosomes->at(o1.chrom_id);
	    const GenomeIndex::Chromosome* c2= this->chromosomes->at(o2.chrom_id);
	    int32_t i1=o1.pos;
	    int32_t i2=o2.pos;
	    int32_t n=0;
	    for(;;)
		{
		if(i1>= c1->size())
		    {
		    if(i2>=c2->size()) return false;
		    return true;
		    }
		if(i2>=c2->size())
		    {
		    return false;
		    }
		char b1= c1->at(i1);
		char b2= c2->at(i2);
		if(b1!=b2) return b1<b2;
		++i1;
		++i2;
		++n;
		if(n> max_lookahead) return false;
		}
	    return false;
	    }
    };

void GenomeIndex::_createindex()
    {
    uint64_t genome_size=0;
    for(size_t i=0;i< chromosomes.size();++i)
	{
	genome_size+= chromosomes[i]->size();
	}
    gIndex.clear();
    gIndex.reserve(genome_size);
    Reference reference;
    for(size_t i=0;i< chromosomes.size();++i)
    	{
	reference.chrom_id=(uint8_t)i;
	const Chromosome* curr=chromosomes[i];
    	for(int32_t j=0;j< curr->size();++j)
    	    {
    	    if(curr->at(j)=='N') continue;
    	    reference.pos=j;
    	    gIndex.push_back(reference);
    	    }
    	}
    Sorter sorter;
    sorter.chromosomes=&(this->chromosomes);
    std::random_shuffle(gIndex.begin(),gIndex.end());
    std::sort(gIndex.begin(),gIndex.end(),sorter);
    }


int main(int argc,char** argv)
    {
    GenomeIndex test;
    test.readGenome(argv[1]);
    cerr << "sorting" << endl;
    test._createindex();
    return 0;
    }
