/*
 * xbam.h
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */
#include <cerrno>
#include <cassert>
#include "where.h"
#include "bam.h"
#include "throw.h"
#include "xbam.h"

using namespace std;

#define CASTBAM(a) ((bamFile)a)
#define CASTIDX(a) ((bam_index_t*)a)
#define CASTHEAD(a) ((bam_header_t*)a)

extern "C" {
void bam_init_header_hash(bam_header_t *header);
}

BamFile::BamFile(const char* file):fp(NULL),header(NULL),index(NULL)
    {
    bamFile f=::bam_open(file, "r");
    if(f==NULL) THROW("Cannot open bam file \""<< file << "\" : " << strerror(errno));
    fp=f;

    bam_header_t *h= ::bam_header_read(f);
    if(h==NULL)
	{
	::bam_close(f);
	THROW("Cannot read header for "<< file);
	}
    ::bam_init_header_hash(h);
    this->header=h;

    bam_index_t *i= ::bam_index_load(file);
    if(i==NULL)
	{
	::bam_close(f);
	::bam_header_destroy(h);
	THROW("Cannot read index for "<< file);
	}
    this->index=i;
    }
BamFile::~BamFile()
    {
    if(this->index!=NULL) bam_index_destroy(CASTIDX(index));
    if(this->header!=NULL) bam_header_destroy(CASTHEAD(header));
    if(this->fp!=NULL) bam_close(CASTBAM(fp));
    }



const char* BamFile::findNameByTid(int32_t tid)
    {
    assert(header!=NULL);
    assert(tid>=0);
    if(tid<0 || tid>= CASTHEAD(header)->n_targets) return NULL;
    return CASTHEAD(header)->target_name[tid];
    }
int32_t BamFile::findTidByName(const char* seq_name)
    {
    assert(header!=NULL);
    assert(seq_name!=NULL);
    assert(CASTHEAD(header)->hash!=NULL);
    return ::bam_get_tid(CASTHEAD(header), seq_name);
    }

