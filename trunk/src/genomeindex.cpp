#include <zlib.h>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <limits>
#include "xstdio.h"
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

GenomeIndex::GenomeIndex():short_read_max_size(100)
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
	GenomeIndex* genome;

	Sorter():n_called(0)
	    {
	    }
	bool lt( const GenomeIndex::Reference& o1,
			 const GenomeIndex::Reference& o2
			 ) const

	    {
	    if((++n_called)%1000000==0)
		{
		cerr << ".";
		}
	    const GenomeIndex::Chromosome* c1= this->genome->chromosomes.at(o1.chrom_id);
	    const GenomeIndex::Chromosome* c2= this->genome->chromosomes.at(o2.chrom_id);
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
		if( n> this->genome->short_read_max_size ) return false;
		}
	    return false;
	    }

	bool operator()(
		const GenomeIndex::Reference& o1,
		const GenomeIndex::Reference & o2
		) const
	    {
	    return lt(o1,o2);
	    }
    };

struct Merger
    {
    FILE* io;
    Sorter* sorter;
    void merge(std::vector<GenomeIndex::Reference>& gIndex)
	{
	std::sort(gIndex.begin(),gIndex.end(),*sorter);
	if(io==NULL)
	    {
	    io=safeTmpFile();
	    safeFWrite((void*)&gIndex.front(),gIndex.size(),sizeof(GenomeIndex::Reference),io);
	    safeFFlush(io);
	    return;
	    }
	FILE* out=safeTmpFile();
	::safeRewind(io);
	bool need_reload_file=true;
	GenomeIndex::Reference fReference;
	size_t array_index=0;
	while(array_index<gIndex.size())
	    {
	    if(need_reload_file)
		{
		need_reload_file=false;
		if(fread((void*)&fReference,sizeof(GenomeIndex::Reference),1,io)!=1)
		    {
		    break;
		    }
		}
	    if(sorter->lt(gIndex[array_index],fReference))
		{
		safeFWrite((void*)&gIndex[array_index],sizeof(GenomeIndex::Reference),1,out);
		array_index++;
		}
	    else
		{
		safeFWrite((void*)&fReference,sizeof(GenomeIndex::Reference),1,out);
		need_reload_file=true;
		}
	    }
	while(array_index<gIndex.size())
	    {
	    safeFWrite((void*)&gIndex[array_index],sizeof(GenomeIndex::Reference),1,out);
	    array_index++;
	    }

	while(fread((void*)&fReference,sizeof(GenomeIndex::Reference),1,io)==1)
	    {
	    safeFWrite((void*)&fReference,sizeof(GenomeIndex::Reference),1,out);
	    }

	fclose(io);
	io=out;
	gIndex.clear();
	}
    };

#define GINDEX_BUFFER_SIZE 100000
void GenomeIndex::_createindex()
    {
    gIndex.clear();
    gIndex.reserve(GINDEX_BUFFER_SIZE);

    Sorter sorter;
    sorter.genome=this;
    Merger merger;
    merger.io=NULL;
    merger.sorter=&sorter;

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
    	    if(gIndex.size()>=GINDEX_BUFFER_SIZE)
    		{
    		merger.merge(gIndex);
    		gIndex.clear();
    		gIndex.reserve(GINDEX_BUFFER_SIZE);
    		}
    	    }
    	if(!gIndex.empty())
    	    {
    	    merger.merge(gIndex);
    	    gIndex.clear();
    	    }
    	if(merger.io!=NULL)
    	    {
    	    ::safeRewind(merger.io);
    	    GenomeIndex::Reference fReference;
    	    while(fread((void*)&fReference,sizeof(GenomeIndex::Reference),1,merger.io)==1)
    		    {
    		    gIndex.push_back(fReference);
    		    }
    	    fclose(merger.io);
    	    }
    	}

void GenomeIndex::writeIndex(const char* filename)
    {
    gzFile* out=gzopen(filename,"wb");
    if(out==NULL) THROW("Cannot open "<< filename << " "<< strerror(errno));


    uint8_t n_chrom=(uint8_t)chromosomes.size();
    ::gzwrite(out,(void*)&n_chrom,sizeof(uint8_t)*1);

    for(size_t i=0;i< chromosomes.size();++i)
	{
	const Chromosome* chrom= chromosomes.at(i);
	size_t n_len_name=chrom->name.size();
	::gzwrite(out,(void*)&n_len_name,sizeof(size_t)*1);
	::gzwrite(out,(void*)chrom->name.data(),sizeof(char)*n_len_name);
	int32_t chrom_len=chrom->size();
	::gzwrite(out,(void*)&chrom_len,sizeof(int32_t));
	}

    size_t n_gindex=gIndex.size();
    ::gzwrite(out,(void*)&n_gindex,sizeof(size_t));
    ::gzwrite(out,(void*)&gIndex.front(),sizeof(GenomeIndex::Reference)*n_gindex);

    gzclose(out);
    }

 void GenomeIndex::readIndex(const char* filename)
    {
    gzFile* in=gzopen(filename,"rb");
    if(in==NULL) THROW("Cannot open "<< filename << " "<< strerror(errno));


    uint8_t n_chrom;
    ::gzread(in,(void*)&n_chrom,sizeof(uint8_t)*1);


    for(int32_t i=0;i< (int32_t)n_chrom;++i)
	{
	Chromosome* chrom= new Chromosome;
	chrom->id= (uint8_t)chromosomes.size();
	chromosomes.push_back(chrom);

	size_t n_len_name;
	::gzread(in,(void*)&n_len_name,sizeof(size_t)*1);
	chrom->name.resize(n_len_name,'\0');
	::gzread(in,(void*)chrom->name.data(),sizeof(char)*n_len_name);
	::gzread(in,(void*)&(chrom->length),sizeof(int32_t));
	}

    size_t n_gindex;
    ::gzread(in,(void*)&n_gindex,sizeof(size_t));
    gIndex.resize(n_gindex);
    ::gzread(in,(void*)&gIndex.front(),sizeof(GenomeIndex::Reference)*n_gindex);

    gzclose(in);
    }
    }


int main(int argc,char** argv)
    {
    GenomeIndex test;
    test.readGenome(argv[1]);
    cerr << "sorting" << endl;
    test._createindex();
    return 0;
    }
