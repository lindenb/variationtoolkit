/*
 * xbgzf.cpp
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */
#include <sstream>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <bgzf.h>
#include "xbgzf.h"
#include "throw.h"
using namespace std;

#define CASTPTR ((BGZF *)(this->_ptr))


BgzFile::BgzFile(const char* filename,const char* mode):_ptr(NULL)
    {
    _ptr=::bgzf_open(filename,mode);
    if(_ptr==NULL)
		{
		THROW("Cannot open "<< filename << " " << strerror(errno));
		}
    }
BgzFile::BgzFile(int fd,const char* mode):_ptr(NULL)
    {
    _ptr=::bgzf_fdopen(fd,mode);
    if(_ptr==NULL)
		{
    	THROW("Cannot open file descriptor " << strerror(errno));
		}
    }
BgzFile::~BgzFile()
    {
    close();
    }
void BgzFile::close()
    {
    if(_ptr!=NULL) ::bgzf_close(CASTPTR);
    _ptr=NULL;
    }
int BgzFile::read(void* data,int length)
    {
    return _ptr==NULL? -1:bgzf_read(CASTPTR,data,length);
    }

int BgzFile::write(void* data,int length)
    {
    return _ptr==NULL? -1:bgzf_write(CASTPTR,data,length);
    }
int64_t BgzFile::tell()
    {
    return _ptr==NULL? -1:bgzf_tell(CASTPTR);
    }
int64_t BgzFile::seek(int64_t pos, int where)
    {
    return _ptr==NULL? -1:bgzf_seek(CASTPTR,pos,where);
    }
void BgzFile::set_cache_size(int cache_size)
    {
    if( _ptr!=NULL) ::bgzf_set_cache_size(CASTPTR,cache_size);
    }
int BgzFile::getc()
    {
    return _ptr==NULL? -1:bgzf_getc(CASTPTR);
    }

int BgzFile::flush()
    {
    return _ptr==NULL? -1: ::bgzf_flush(CASTPTR);
    }
    