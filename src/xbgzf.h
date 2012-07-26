/*
 * xbgzf.h
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */

#ifndef XBGZF_H_
#define XBGZF_H_

#include <stdint.h>
class BgzFile
    {
    private:
	void* _ptr;
    public:
	BgzFile(int fd,const char* m);
	BgzFile(const char* filename,const char* m);
	virtual ~BgzFile();
	virtual void close();
	virtual int flush();
	virtual int read(void* data,int length);
	virtual int write(void* data,int length);
	virtual int64_t tell();
	virtual int64_t seek(int64_t pos, int where);
	virtual void set_cache_size(int cache_size);
	virtual int getc();
    };

#endif /* XBGZF_H_ */
