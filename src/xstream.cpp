#include "xstream.h"
#include "throw.h"

using namespace std;

XmlStream::XmlStream(std::istream& in):
	in(&in),reader(0),dom(0)
    {
    const char* url=0;
    int options=0;
    reader=(void*)::xmlReaderForIO(
	    XmlStream::_xmlInputReadCallback,
	    XmlStream::_xmlInputCloseCallback,
	     this,
	     url,
	     "UTF-8",
	     options
	     );
    }
XmlStream::~XmlStream()
    {
    close();
    }

void XmlStream::close()
    {
    if(reader!=0)
	{
	::xmlFreeTextReader(reader);
	reader=0;
	}
    if(dom!=0)
	{
	::xmlDocFree(dom);
	dom=0;
	}
    in=0;
    }

int StreamXmlReader::_xmlInputReadCallback(
		 void * context,
		 char * buffer,
		 int len)
    {
    if( ((XmlStream*)context)->in==0) return 0;
    ((XmlStream*)context)->in->read(buffer,len);
    int n= (int) ((XmlStream*)context)->in->gcount();
    return n;
    }
int StreamXmlReader::_xmlInputCloseCallback(void * context)
    {
    ((StreamXmlReader*)context)->in=0;
    return 0;
    }


xmlDocPtr XmlStream::next()
    {
    for(;;)
	{
	int ret = ::xmlTextReaderRead(reader);
	if(ret==-1 || ret==0)
	    {
	    close();
	    return 0;
	    }
	int nodeType= ::xmlTextReaderNodeType(reader);
	switch(nodeType)
	     {

	     case XML_READER_TYPE_ELEMENT:
		 {


		 xmlNodePtr node=::xmlNewNode(0,0);
		 if(dom==0)
		     {
		     dom = ::xmlNewDoc("1.0");
		     ::xmlDocSetRootElement(dom, node);
		     }


		 if(xmlTextReaderHasAttributes(CAST_READER(this)))
		     {
		     int n_att=xmlTextReaderAttributeCount(CAST_READER(this));
		     for(int i=0;i< n_att;++i)
			 {
			 xmlTextReaderMoveToAttributeNo(reader,i);

			 xmlChar* v = xmlTextReaderValue(reader);
			 xmlNewProp(node, BAD_CAST "attribute", v);
			 xmlFree(v);
			 }
		     xmlTextReaderMoveToElement(CAST_READER(this));
		     }
		 if(xmlTextReaderIsEmptyElement(CAST_READER(this)))
		     {
		     EndElement* c=new EndElement;
		     c->_name=new QName(*(e->_name));
		     queue.push_back(c);
		     }
		 break;
		 }
	     case XML_READER_TYPE_END_ELEMENT:
		 {
		 EndElement* c=new EndElement;
		 c->_name=new QName(makeQName());
		 queue.push_back(c);
		 break;
		 }
	     case XML_READER_TYPE_TEXT:
		 {
		 Text* t=new Text();
		xmlChar* v = xmlTextReaderValue(CAST_READER(this));
		 t->_data.assign((const char*)v);
		 xmlFree(v);
		queue.push_back(t);
		 break;
		 }
	     case XML_READER_TYPE_COMMENT:
		 {
		WHERE(nodeType);
		 break;
		 }
	     default:
		 {
		WHERE(nodeType);
		 break;
		 }
	     }
    return dom;
    }
