/*
 * xtabix.cpp
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */

#ifndef XTABIX_H
#define XTABIX_H
#include <memory>
#include <stdint.h>
class Tabix
    {
    private:
	void* ptr;
    public:
	class Cursor
	    {
	    private:
		void* ptr;
		void* iter;
	    public:
		Cursor(void*,void*);
		virtual ~Cursor();
		const char* next(int *len=NULL);
	    };
	Tabix(const char* filename,bool loadIndex=true);
	virtual ~Tabix();
	std::auto_ptr<Tabix::Cursor> cursor(const char* chrom,int32_t chromStart,int32_t chromEnd);
    };

#endif
