/*
 * xtabix.cpp
 *
 *  Created on: Oct 11, 2011
 *      Author: lindenb
 */
#include "bgzf.h"
#include "tabix.h"
#include "throw.h"
#include "xtabix.h"
using namespace std;

#define CASTPTR ((tabix_t*)this->ptr)

Tabix::Tabix(const char* tabixfile,bool loadIndex)
    {
    tabix_t* t= ti_open(tabixfile, 0);
    if(t==NULL) THROW("Cannot open tabix file: "<< tabixfile);
    ptr=t;
    if(loadIndex && ::ti_lazy_index_load(t) < 0)
      	 {
	 ::ti_close(t);
         THROW( "Cannot open index for file \""<< tabixfile << "\".");
         }
    }

Tabix::~Tabix()
    {
    if(ptr!=NULL) ::ti_close(CASTPTR);
    }

std::auto_ptr<Tabix::Cursor> Tabix::cursor(const char* chrom,int32_t chromStart,int32_t chromEnd)
    {
    int tid=::ti_get_tid(CASTPTR->idx,chrom);
    if(tid<0) return std::auto_ptr<Tabix::Cursor>(new Tabix::Cursor(CASTPTR,NULL));
    ti_iter_t iter = ti_queryi(CASTPTR, tid, chromStart, chromEnd);
    return std::auto_ptr<Tabix::Cursor>(new Tabix::Cursor(CASTPTR,iter));
    }


Tabix::Cursor::Cursor(void* p1,void* p2):ptr(p1),iter(p2)
    {
    }

Tabix::Cursor::~Cursor()
    {
    if(iter!=NULL) ti_iter_destroy((ti_iter_t)iter);
    }

const char*
Tabix::Cursor::next(int* len)
    {
    if(iter==NULL) return NULL;
    int len2;
    const char* s=ti_read(CASTPTR,(ti_iter_t)iter, &len2);
    if(len!=NULL) *len=len2;
    return s;
    }
