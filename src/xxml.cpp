
#include "xxml.h"
#include "throw.h"
#include "where.h"
using namespace std;

static int _xmlOutputWriteCallbackIostream(void * context,
     const char * buffer,
     int len)
    {
    if(context==0) return  len;
    ((std::ostream*)context)->write(buffer,len);
    return len;
    }

static int _xmlOutputCloseCallbackIostream(void * context)
    {
    if(context!=0)
	{
	((std::ostream*)context)->flush();
	}
    return 0;
    }
xmlOutputBufferPtr xmlOutputBufferCreateIOStream(
	std::ostream* out,
	xmlCharEncodingHandlerPtr encoder
	)
    {
    xmlOutputBufferPtr outbuf=::xmlOutputBufferCreateIO(
	_xmlOutputWriteCallbackIostream,
	_xmlOutputCloseCallbackIostream,
	 (void*)out,
	 encoder);
    return outbuf;
    }

static int _xmlOutputWriteCallbackString(void * context,
						 const char * buffer,
						 int len)
    {
    if(context==0) return len;
    ((std::string*)context)->append(buffer,len);
    return len;
    }
static int _xmlOutputCloseCallbackString(void * context)
    {
    return 0;
    }


xmlOutputBufferPtr xmlOutputBufferCreateString(
	std::string* out,
	xmlCharEncodingHandlerPtr encoder)
    {
    xmlOutputBufferPtr outbuf=::xmlOutputBufferCreateIO(
    	 		_xmlOutputWriteCallbackString,
    	 		_xmlOutputCloseCallbackString,
    			 (void*)out,
    			 0);
    return outbuf;
    }



XmlStreamWriter::XmlStreamWriter(xmlTextWriterPtr delegate):delegate(delegate)
    {
    }
XmlStreamWriter::~XmlStreamWriter()
    {

    }
xmlTextWriterPtr XmlStreamWriter::getDelegate()
    {
    return delegate;
    }

void XmlStreamWriter::writeStartDocument()
    {
    ::xmlTextWriterStartDocument(getDelegate(),0,0,0);
    }

void XmlStreamWriter::writeEndDocument()
    {
    ::xmlTextWriterEndDocument(getDelegate());
    }

void XmlStreamWriter::writeAttribute(const char* name,const char* value)
    {
    ::xmlTextWriterWriteAttribute(
	    getDelegate(),
	    BAD_CAST name,
	    BAD_CAST value
	    );
    }


void XmlStreamWriter::writeStartElementNS(const char* prefix,const char* name,const char* ns)
    {
    ::xmlTextWriterStartElementNS(
	    getDelegate(),
	    BAD_CAST prefix,
	    BAD_CAST name,
	    BAD_CAST ns);
    }

void XmlStreamWriter::writeStartElement(const char* name)
    {
    ::xmlTextWriterStartElement(
    	    getDelegate(),
    	    BAD_CAST name
    	    );
    }

void XmlStreamWriter::writeEndElement()
    {
    ::xmlTextWriterEndElement(getDelegate());
    }

void XmlStreamWriter::writeText(const char* s)
    {
    if(s==0) return;
    ::xmlTextWriterWriteString(getDelegate(),BAD_CAST s);
    }

void XmlStreamWriter::writeComment(const char* s)
    {
    if(s==0) return;
    ::xmlTextWriterWriteComment(getDelegate(),BAD_CAST s);
    }


