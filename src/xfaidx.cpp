/*
 * xfaidx.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
 */
#include "faidx.h"
#include "xfaidx.h"
#include "throw.h"

using namespace std;

#define CASTPTR ((faidx_t*)ptr)
IndexedFasta::IndexedFasta(const char* fasta)
    {
    faidx_t *idx=fai_load(fasta);
    if(idx==NULL) THROW("Cannot load indexed fasta "<< fasta);
    ptr=idx;
    }

IndexedFasta::~IndexedFasta()
    {
    if(ptr!=NULL) fai_destroy(CASTPTR);
    }

int32_t IndexedFasta::size()
    {
    return ::faidx_fetch_nseq(CASTPTR);
    }

std::auto_ptr<std::string>
IndexedFasta::fetch(const char* chrom,int32_t start0,int32_t end0)
    {
    int len;
    char *s=::faidx_fetch_seq(CASTPTR,(char*)chrom,start0, end0, &len);
    if(s==NULL) return std::auto_ptr<std::string>();
    return auto_ptr<std::string>(new string(s,len));
    }
