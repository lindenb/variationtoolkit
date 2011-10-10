#ifndef GZSTREAMBUF_H
#define GZSTREAMBUF_H

#include <iostream>
#include <cstdio>
#include <zlib.h>
#include "throw.h"

#define GZBUFSIZ 2
//BUFSIZ
class igzstreambuf:public std::streambuf
	{
	private:
		gzFile in;
		char buffer[GZBUFSIZ];
		unsigned int buffer_size;
		bool owner;
	public:
		igzstreambuf(gzFile in):in(in),buffer_size(0),owner(false)
			{
			setg( &this->buffer[0],
				&this->buffer[GZBUFSIZ],
				&this->buffer[GZBUFSIZ]
				);
			}
		
		igzstreambuf():in(NULL),buffer_size(0),owner(false)
			{
			errno=0;
			in=::gzdopen(fileno(stdin),"r");
			if(in==NULL)
			    {
			    THROW("Cannot open gz file (stdin) " << strerror(errno));
			    }
			setg( &this->buffer[0],
				&this->buffer[GZBUFSIZ],
				&this->buffer[GZBUFSIZ]
				);
			}	
		
		
		igzstreambuf(const char* f):in(NULL),buffer_size(0),owner(true)
			{
			errno=0;
			in=::gzopen(f,"rb");
			if(in==NULL)
			    {
			    THROW("Cannot open gz file \""<< f << "\" " << strerror(errno));
			    }
			setg( &this->buffer[0],
				&this->buffer[GZBUFSIZ],
				&this->buffer[GZBUFSIZ]
				);
			}	
		
		virtual void close()
			{
			if(in!=NULL && owner)
				{
				::gzclose(in);
				}
			in=NULL;
			}
		
		virtual ~igzstreambuf()
			{
			if(in!=NULL && owner)
				{
				::gzclose(in);
				}
			}
	
	    virtual int underflow ( )
			{
			if(in==NULL) return EOF;
			int nRead= ::gzread(this->in,(void*)this->buffer,GZBUFSIZ);
			if(nRead<0)
				{
				THROW("I/O error");
				}
			else if(nRead==0)
				{
				if(owner) ::gzclose(in);
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
	
#undef GZBUFSIZ

#endif
