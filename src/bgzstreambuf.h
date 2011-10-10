#ifndef GZSTREAMBUF_H
#define GZSTREAMBUF_H

#include <iostream>
#include <cstdio>
#include <bgzf.h>
#include <cerrno>
#include <cstring>
#include "throw.h"

#define BGZBUFSIZ BUFSIZ

class ibgzstreambuf:public std::streambuf
	{
	private:
		BGZF* in;
		char buffer[BGZBUFSIZ];
		unsigned int buffer_size;
		bool owner;
	public:
		ibgzstreambuf(BGZF* in):in(in),buffer_size(0),owner(false)
			{
			setg( &this->buffer[0],
				&this->buffer[BGZBUFSIZ],
				&this->buffer[BGZBUFSIZ]
				);
			}
		
		ibgzstreambuf():in(NULL),buffer_size(0),owner(false)
			{
			errno=0;
			in=::bgzf_fdopen(fileno(stdin),"r");
			if(in==NULL)
			    {
			    THROW("Cannot open gz file (stdin) " << strerror(errno));
			    }
			setg( &this->buffer[0],
				&this->buffer[BGZBUFSIZ],
				&this->buffer[BGZBUFSIZ]
				);
			}	
		
		
		ibgzstreambuf(const char* f):in(NULL),buffer_size(0),owner(true)
			{
			errno=0;
			in=::bgzf_open(f,"r");
			if(in==NULL)
			    {
			    THROW("Cannot open gz file \""<< f << "\" " << strerror(errno));
			    }
			setg( &this->buffer[0],
				&this->buffer[BGZBUFSIZ],
				&this->buffer[BGZBUFSIZ]
				);
			}	
		
		virtual void close()
			{
			if(in!=NULL && owner)
				{
				::bgzf_close(in);
				}
			in=NULL;
			}
		
		virtual ~ibgzstreambuf()
			{
			if(in!=NULL && owner)
				{
				::bgzf_close(in);
				}
			}
	
	    virtual int underflow ( )
			{
			if(in==NULL) return EOF;
			int nRead= ::bgzf_read(this->in,(void*)this->buffer,BGZBUFSIZ);
			if(nRead<0)
				{
				THROW("I/O error");
				}
			else if(nRead==0)
				{
				if(owner) ::bgzf_close(in);
				in=NULL;
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
