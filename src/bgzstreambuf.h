#ifndef GZSTREAMBUF_H
#define GZSTREAMBUF_H

#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include "xbgzf.h"
#include "throw.h"

#define BGZBUFSIZ BUFSIZ

class ibgzstreambuf:public std::streambuf
	{
	private:
		BgzFile* in;
		char buffer[BGZBUFSIZ];
		unsigned int buffer_size;
		bool owner;
	public:
		ibgzstreambuf(BgzFile* in):in(in),buffer_size(0),owner(false)
			{
			setg( &this->buffer[0],
				&this->buffer[BGZBUFSIZ],
				&this->buffer[BGZBUFSIZ]
				);
			}
		
		ibgzstreambuf():in(NULL),buffer_size(0),owner(false)
			{
			in=new BgzFile(fileno(stdin),"r");
			setg( &this->buffer[0],
				&this->buffer[BGZBUFSIZ],
				&this->buffer[BGZBUFSIZ]
				);
			}	
		
		
		ibgzstreambuf(const char* f):in(NULL),buffer_size(0),owner(true)
			{
			errno=0;
			in=new BgzFile(f,"r");
			setg( &this->buffer[0],
				&this->buffer[BGZBUFSIZ],
				&this->buffer[BGZBUFSIZ]
				);
			}	
		
		virtual void close()
			{
			if(in!=NULL && owner)
				{
				delete in;
				}
			in=NULL;
			}
		
		virtual ~ibgzstreambuf()
			{
			if(in!=NULL && owner)
				{
				delete in;
				}
			}
	
	    virtual int underflow ( )
			{
			if(in==NULL) return EOF;
			int nRead=  this->in->read((void*)this->buffer,BGZBUFSIZ);
			if(nRead<0)
				{
				THROW("I/O error");
				}
			else if(nRead==0)
				{
				close();
				return EOF;
				}
			this->buffer_size=(unsigned int)nRead;
			setg( (char*)this->buffer,
				(char*)&this->buffer[0],
				(char*)&this->buffer[this->buffer_size]
				);
			return this->buffer_size==0?EOF:this->buffer[0];
			}
	};
	
#undef BGZBUFSIZ

#endif
