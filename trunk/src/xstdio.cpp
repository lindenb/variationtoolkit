/*
 * xstdio.cpp
 *
 *  Created on: Aug 8, 2011
 *      Author: Pierre Lindenbaum PhD
 *              plindenbaum@yahoo.fr
 *              http://plindenbaum.blogspot.com
 *              
 */
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include "xstdio.h"
#include "xstdlib.h"
#include "throw.h"
using namespace std;

FILE *safeFOpen (const char *filename,const char *modes)
    {
    FILE* f=std::fopen(filename,modes);
    if(f==NULL)
	{
	THROW( "[I/O] Cannot open " << filename << " " << strerror(errno)<< ".\n");
    	}
    return f;
    }

FILE *safeTmpFile()
    {
    FILE* f=std::tmpfile();
    if(f==NULL)
	{
	THROW("[I/O] Cannot open temporary file " << strerror(errno)<< ".\n");
    	}
    return f;
    }

char* readLine(std::FILE* in,std::size_t* userlen)
    {
    size_t buffer_len=BUFSIZ;
    size_t len=0;
    if(feof(in))
	{
	if(userlen!=NULL) *userlen=0UL;
	return NULL;
	}

    char* p=(char*)::safeMalloc(buffer_len);
    int c;
    while((c=fgetc(in))!=EOF && c!='\n')
	{
	if(len+2>=buffer_len)
	    {
	    buffer_len+=BUFSIZ;
	    p=(char*)::safeRealloc(p,buffer_len*sizeof(char));
	    }
	p[len++]=c;
	}
    p[len]='\0';
    if(userlen!=NULL) *userlen=len;
    return p;
    }


std::size_t safeFRead(void *ptr, std::size_t size, std::size_t nmemb, std::FILE *in)
    {
    size_t n=  fread(ptr,size,nmemb,in);
    if(n!=nmemb)
    	{
    	THROW("[I/O] Wanted to read " <<nmemb << " items but got " << n <<".\n");
        }
    return n;
    }

std::size_t safeFWrite(const void *ptr, std::size_t size, std::size_t nmemb, std::FILE *out)
    {
    size_t n=fwrite(ptr,size,nmemb,out);
    if(n!=nmemb)
	{
	THROW("[I/O] Wanted to write " <<nmemb << " items but got " << n <<".\n");
    	}
    return n;
    }

int safeFSeek(FILE * stream, long int offset, int origin)
    {
    int n= fseek(stream,offset,origin);
    if(n!=0)
   	{
   	THROW("[I/O] fseek failed " <<offset << "/"+origin << "  got error= " << n <<".\n");
       	}
    return n;
    }

int safeFFlush(std::FILE * stream)
    {
    int n= fflush(stream);
    if(n!=0)
       	{
       	THROW("[I/O] fflush failed got error= " << n <<".\n");
        }
    return n;
    }

void safeRewind(std::FILE *stream)
    {
    errno=0;
    rewind(stream);
    if(errno!=0) THROW("Cannot rewind");
    }
