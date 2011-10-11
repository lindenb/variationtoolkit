/*
 * xfaidx.cpp
 *
 *  Created on: Oct 10, 2011
 *      Author: lindenb
 */
#ifndef XFAIDX_H
#define XFAIDX_H
#include <string>
#include <memory>
#include <stdint.h>


class IndexedFasta
    {
    private:
	void* ptr;
    public:
	IndexedFasta(const char* fasta);
	virtual ~IndexedFasta();
	std::auto_ptr<std::string> fetch(const char* chrom,int32_t start0,int32_t end0);
	std::auto_ptr<std::string> fetch(int32_t tid,int32_t start0,int32_t end0);
	int32_t size();

    };

#endif

