
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
