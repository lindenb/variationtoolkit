/*
 * xbam.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */
#include <cerrno>
#include <cassert>
#include "where.h"

#include "throw.h"
#include "xbam2.h"

using namespace std;

#define CASTBAM(a) (a)
#define CASTIDX(a) (a)
#define CASTHEAD(a) (a)

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}

BamFile2::BamFile2(const char* file) :fp(NULL),header(NULL),index(NULL),filename(file)
    {

    }

// callback for bam_fetch()
int BamFile2::fetch_func(const bam1_t *b, void *data)
	{
	bam_plbuf_t *buf = (bam_plbuf_t*)data;
	bam_plbuf_push(b, buf);
	return 0;
	}

bool BamFile2::is_open() const
    {
    return fp!=0;
    }

void BamFile2::open(bool load_index)
    {
    close();
    bamFile f=::bam_open(filename.c_str(), "r");
    if(f==NULL) THROW("Cannot open bam file \""<< filename << "\" : " << strerror(errno));
    fp=f;

    bam_header_t *h= ::bam_header_read(f);
    if(h==NULL)
	{
	close();
	THROW("Cannot read header for "<< filename);
	}
    ::bam_init_header_hash(h);
    this->header=h;

    if(load_index)
	{
	bam_index_t *i= ::bam_index_load(filename.c_str());
	if(i==NULL)
	    {
	    close();
	    THROW("Cannot read index for "<< filename);
	    }
	this->index=i;
	}
    }
BamFile2::~BamFile2()
    {
    close();
    }

void BamFile2::close()
    {
    if(this->index!=NULL) ::bam_index_destroy(CASTIDX(index));
    this->index=0;
    if(this->header!=NULL) ::bam_header_destroy(CASTHEAD(header));
    this->header=0;
    if(this->fp!=NULL) ::bam_close(CASTBAM(fp));
    this->fp=0;
    }

const char* BamFile2::findNameByTid(int32_t tid)
    {
    assert(header!=NULL);
    assert(tid>=0);
    if(tid<0 || tid>= CASTHEAD(header)->n_targets) return NULL;
    return CASTHEAD(header)->target_name[tid];
    }

int32_t BamFile2::findTidByName(const char* seq_name)
    {
    assert(header!=NULL);
    assert(seq_name!=NULL);
    assert(CASTHEAD(header)->hash!=NULL);
    return ::bam_get_tid(CASTHEAD(header), seq_name);
    }

bamFile BamFile2::bamPtr() { return this->fp;}
bam_index_t* BamFile2::bamIndex() { return this->index;}
bam_header_t* BamFile2::bamHeader() { return this->header;}

int32_t BamFile2::count_targets() const
    {
    return header->n_targets;
    }
const char* BamFile2::target_name(int32_t n) const
    {
    assert(n< header->n_targets);
    return header->target_name[n];
    }
int32_t BamFile2::target_length(int32_t n) const
    {
    assert(n< header->n_targets);
    return (int32_t) header->target_len[n];
    }

const char* BamFile2::path() const
    {
    return this->filename.c_str();
    }

std::auto_ptr<vector<BamFile2::Target> > BamFile2::targets()
    {
    std::vector<BamFile2::Target>* v=new vector<BamFile2::Target>;
    v->reserve(header->n_targets);
    for(int32_t n=0;n< count_targets();++n)
	{
	v->push_back(Target(n,target_name(n),target_length(n)));
	}
    return std::auto_ptr<vector<BamFile2::Target> >(v);
    }

BamFile2::Target::Target(int32_t tid,const char* name,uint32_t length):
	_tid(-1),_name(name),_length(length)
    {

    }

BamFile2::Target::Target(const Target& cp):
	_tid(cp._tid),_name(cp._name),_length(cp._length)
    {

    }
BamFile2::Target::~Target()
    {

    }
BamFile2::Target& BamFile2::Target::operator=(const BamFile2::Target& cp)
    {
    if(this!=&cp)
	{
	_tid=cp._tid;
	_name.assign(cp._name);
	_length=cp._length;
	}
    return *this;
    }

int32_t BamFile2::Target::tid() const
    {
    return _tid;
    }
const std::string& BamFile2::Target::name() const
    {
    return _name;
    }
uint32_t BamFile2::Target::length() const
    {
    return _length;
    }
