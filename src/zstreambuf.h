#ifndef GZSTREAMBUF_H
#define GZSTREAMBUF_H

#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <zlib.h>
#include "throw.h"

#define GZBUFSIZ BUFSIZ

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

class deflatestreambuf:public std::streambuf
	{
	private:
		std::istream* in;
		unsigned char buffin[GZBUFSIZ];
		unsigned char buffout[GZBUFSIZ];
		char* buffer;
		z_stream strm;
		int status_flag;
	public:
		deflatestreambuf(std::istream* in):in(in),status_flag(0)
			{
			if(in==NULL) THROW("Null pointer");
			std::memset((void*)&strm,0,sizeof(z_stream));
			strm.zalloc = Z_NULL;
			strm.zfree = Z_NULL;
			strm.opaque = Z_NULL;
			strm.avail_in = 0;
			strm.next_in = Z_NULL;

			buffer=(char*)std::malloc(GZBUFSIZ*sizeof(char));
			if(buffer==NULL)
			    {
			    THROW("out of memory");
			    }
			if ( inflateInit2(&strm, 16+MAX_WBITS) != Z_OK)
			    {

			    THROW("::inflateInit failed");
			    }
			setg(   &buffer[0],
				&buffer[GZBUFSIZ],
				&buffer[GZBUFSIZ]
				);

			}


		virtual void close()
			{
			if(in!=NULL)
				{
				in=NULL;
				(void)inflateEnd(&strm);
				free(buffer);
				buffer=NULL;
				}
			in=NULL;
			}

		virtual ~deflatestreambuf()
			{
			close();
			}
	
	    virtual int underflow ( )
		{
		if(in==NULL) return EOF;
		if(status_flag==Z_STREAM_END)
		    {
		    close();
		    return EOF;
		    }
		in->read((char*)buffin,GZBUFSIZ);
		strm.avail_in = in->gcount();

		if(strm.avail_in == 0)
		    {
		    close();
		    return EOF;
		    }
		strm.next_in=buffin;
	       /* run inflate() on input until output buffer not full */
		unsigned int total=0;
		do {
			strm.avail_out = GZBUFSIZ;
			strm.next_out = buffout;
			status_flag = ::inflate(&strm, Z_NO_FLUSH);

			switch (status_flag)
			    {
			    case 0:break;
			    case Z_STREAM_END:break;
			    case Z_NEED_DICT:
				status_flag=Z_DATA_ERROR;
			    case Z_DATA_ERROR:
			    case Z_MEM_ERROR:

				close();
				THROW("Error "<< status_flag);
				break;
			    default:
				{
				close();
				THROW("Error "<< status_flag);
				break;
				}
			    }

		    unsigned int have = GZBUFSIZ - strm.avail_out;
		    buffer=(char*)std::realloc(buffer,sizeof(char)*(total+have));
		    if(buffer==NULL)
			{
			THROW("out of memory");
			}

		    std::memcpy((void*)&buffer[total],
			&buffout[0],
			sizeof(char)*have
			);
		    total+=have;
		    std::cerr.write(buffer,total);

		    } while (strm.avail_out == 0);


		setg(	buffer,
			buffer,
			&buffer[total]
			);

		return total==0?EOF:this->buffer[0];
		}
	};


#undef GZBUFSIZ

#endif
